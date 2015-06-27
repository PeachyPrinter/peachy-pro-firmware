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
  QueueTx(buffer, len);
}

static void serial_done(unsigned char* buffer, uint8_t len) {
  type_callback_map_t* cur = callbacks;
  while(cur->callback != 0) {
    if (cur->message_type == buffer[0]) {
      cur->callback(&buffer[1], len - 1);
      break;
    }
    cur++;
  }
}

void serialio_feed() {
  int count;
  uint8_t read_buffer[64];
  if((count = CDC_ReadBytes(read_buffer)) != 0) {
    GPIO_WriteBit(GPIOB, GPIO_Pin_12, 1);
    serial_done(read_buffer, count); 
  } else {
    GPIO_WriteBit(GPIOB, GPIO_Pin_12, 0);
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

  uint8_t type = IAM;
  pb_write(&stream, &type, 1);

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

