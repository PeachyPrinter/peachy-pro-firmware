#include "serialio.h"
#include "iolib.h"
#include "pb_decode.h"
#include "pb_encode.h"
#include "messages.pb.h"
#include <usb_cdc.h>
#include <i2c.h>
#include <clock.h>
#include "pwmout.h"
#include "version.h"

/**
 * Serial protocol
 *
 * Header - 0x12
 * Type 
 * Payload
 * Checksum
 * Footer - 0x13
 *
 * Everything between header and footer is escaped to ensure that the
 header and footer are only ever transmitted as headers and footers. Escaping happens by sending 0x14 followed by ^(byte). Have to escape 0x12, 0x13, 0x14. 

 * State machine for reading:
 * SEARCHING - looking for 0x12
 * READING - filling buffer
 * READING_GOT_ESCAPE - skip writing to buffer, write ^(next) to buffer
 * DONE - read 0x13
*/

extern volatile uint32_t g_dripcount;
extern volatile uint8_t move_start;
extern volatile uint8_t move_count;
extern Move move_buffer[MOVE_SIZE];

#define HEADER 0x40
#define FOOTER 0x41
#define ESCAPE_CHAR 0x42

void handle_move(unsigned char* buffer, int len);
void handle_nack(unsigned char* buffer, int len);
void handle_ack(unsigned char* buffer, int len);
void handle_measure(unsigned char* buffer, int len);
void handle_set_drip_count(unsigned char* buffer, int len);
void handle_identify(unsigned char* buffer, int len);

static type_callback_map_t callbacks[] = {
  { NACK, &handle_nack },
  { ACK, &handle_ack },
  { MOVE, &handle_move }, 
  { MEASURE, &handle_measure },
  { SET_DRIP_COUNT, &handle_set_drip_count },
  { IDENTIFY, &handle_identify },
  { 0, 0 }
};

void serialio_write(unsigned char* buffer, uint8_t len) {
  unsigned char to_send[len*2+2]; // worst case, every single byte gets escaped, plus header and footer
  int out_idx = 0;
  int in_idx;

  to_send[out_idx++] = HEADER;
  for(in_idx = 0; in_idx < len; in_idx++) {
    if (buffer[in_idx] == HEADER || buffer[in_idx] == FOOTER || buffer[in_idx] == ESCAPE_CHAR) {
      to_send[out_idx++] = ESCAPE_CHAR;
      to_send[out_idx++] = ~buffer[in_idx];
    } else {
      to_send[out_idx++] = buffer[in_idx];
    }
  }
  to_send[out_idx++] = FOOTER;
  QueueTx(to_send, out_idx);
}

serial_state_t serial_searching(uint8_t* idx, unsigned char* buffer, unsigned char input) {
  if(input != HEADER) {
    return SEARCHING;
  }
  *idx = 0;
  buffer[*idx] = 0;
  return READING;
}

serial_state_t serial_reading(uint8_t* idx, unsigned char* buffer, unsigned char input) {
  if(input == ESCAPE_CHAR) {
    return READING_ESCAPED;
  }
  if(input == FOOTER) {
    return DONE;
  }
  buffer[*idx] = input;
  (*idx) += 1;
  return READING;
}

serial_state_t serial_reading_esc(uint8_t* idx, unsigned char* buffer, unsigned char input) {
  buffer[*idx] = ~input;
  (*idx) += 1;
  return READING;
}

serial_state_t serial_done(uint8_t* idx, unsigned char* buffer) {
  type_callback_map_t* cur = callbacks;
  while(cur->callback != 0) {
    if (cur->message_type == buffer[0]) {
      buffer[*idx] = 0;
      cur->callback(&buffer[1], (*idx)-1);
      break;
    }
    cur++;
  }
  return SEARCHING;
}

void serialio_feed() {
  static unsigned char read_buffer[64] = {0};
  static unsigned char out_buffer[32] = {0};
  static uint8_t out_idx = 0;
  static serial_state_t state = SEARCHING;

  int count = 0;
  int i = 0;

  if((count = CDC_ReadBytes(read_buffer)) != 0) {
    for(i = 0; i < count; i++) {
      switch(state) {
      case SEARCHING: state = serial_searching(&out_idx, out_buffer, read_buffer[i]); break;
      case READING: state = serial_reading(&out_idx, out_buffer, read_buffer[i]); break;
      case READING_ESCAPED: state = serial_reading_esc(&out_idx, out_buffer, read_buffer[i]); break;
      default:
        break;
      }
      if (state == DONE) { // Special case DONE because it doesn't need any input
        state = serial_done(&out_idx, out_buffer); 
      }
    }
  }
}

/*****************************************/
/* Callbacks for handling messages */

void handle_move(unsigned char* buffer, int len) {
  pb_istream_t stream = pb_istream_from_buffer(buffer, len);
  bool status;
  Move message;

  status = pb_decode(&stream, Move_fields, &message);
  if(status) {
    // wait until there's room in the move buffer
    while(move_count == MOVE_SIZE) ;

    int write_idx = (move_start + move_count) % MOVE_SIZE;
    move_buffer[write_idx] = message;
    move_count++;
  }
}

typedef struct {
  uint32_t id;
  uint16_t val;
} measure_output_t;


void handle_measure(unsigned char* buffer, int len) {
  pb_istream_t stream = pb_istream_from_buffer(buffer, len);
  bool status;
  Measure message;
  measure_output_t out;

  status = pb_decode(&stream, Measure_fields, &message);
  if(status) {
    out.id = message.id;
    i2c_trigger_capture(message.channel);
    delay_ms(10);
    out.val = i2c_read_values();
// TODO replace this with a real encoded thing
    QueueTx((unsigned char*)&out, sizeof(out));
  }
}

void handle_set_drip_count(unsigned char* buffer, int len) {
  pb_istream_t stream = pb_istream_from_buffer(buffer, len);
  bool status;
  SetDripCount message;
  status = pb_decode(&stream, SetDripCount_fields, &message);
  if (status) {
    g_dripcount = message.drips;
  }
}

static bool constant_string(pb_ostream_t* stream, const pb_field_t* field,
                            void* const *arg) {
  char* str = *(char**)arg;
  if (!pb_encode_tag_for_field(stream, field)) {
    return false;
  }
  return pb_encode_string(stream, (uint8_t*)str, strlen(str));
}

void handle_identify(unsigned char* buffer, int len) {
  /* Identify message has nothing interesting in it to read */
  pb_ostream_t stream;
  uint8_t outbuf[64];
  stream = pb_ostream_from_buffer(outbuf, 64);
  char* swrev = VERSION;
  char* hwrev = "hwrev 3456";
  char* sn = "unodos";

  IAm message;
  message.swrev.funcs.encode = &constant_string;
  message.swrev.arg = swrev;
  message.hwrev.funcs.encode = &constant_string;
  message.hwrev.arg = hwrev;
  message.sn.funcs.encode = &constant_string;
  message.sn.arg = sn;
  message.dataRate = 2000;
  
  if(!pb_encode(&stream, IAm_fields, &message)) {
    return;
  }
  serialio_write(outbuf, stream.bytes_written);
}

void handle_nack(unsigned char* buffer, int len) {

}
void handle_ack(unsigned char* buffer, int len) {

}

