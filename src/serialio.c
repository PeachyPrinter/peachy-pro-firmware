#include "serialio.h"
#include "iolib.h"
#include "pb_decode.h"
#include "pb_encode.h"
#include "messages.pb.h"
#include <usb_cdc.h>
#include <i2c.h>
#include <clock.h>
#include "pwmout.h"
#include "hwaccess.h"
#include "version.h"
#include "reprog.h"

extern volatile uint32_t g_dripcount;
extern volatile uint8_t move_start;
extern volatile uint8_t move_count;
extern Move move_buffer[MOVE_SIZE];
extern bool g_debug;

extern volatile uint16_t g_adcVals[ADC_CHANS];

#define HEADER 0x40
#define FOOTER 0x41
#define ESCAPE_CHAR 0x42

void handle_move(unsigned char* buffer, int len);
void handle_nack(unsigned char* buffer, int len);
void handle_ack(unsigned char* buffer, int len);
void handle_set_drip_count(unsigned char* buffer, int len);
void handle_identify(unsigned char* buffer, int len);
void handle_debug(unsigned char* buffer, int len);
void handle_reboot_bootloader(unsigned char* buffer, int len);
void handle_get_adc_val(unsigned char* buffer, int len);

static type_callback_map_t callbacks[] = {
  { NACK, &handle_nack },
  { ACK, &handle_ack },
  { MOVE, &handle_move }, 
  { SET_DRIP_COUNT, &handle_set_drip_count },
  { IDENTIFY, &handle_identify },
	{ DEBUG, &handle_debug },
	{ ENTER_BOOTLOADER, &handle_reboot_bootloader },
	{ GET_ADC_VAL, &handle_get_adc_val },
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
    GPIO_WriteBit(GPIOB, GPIO_Pin_12, 1); //Enable USB LED
    serial_done(read_buffer, count); 
  }
}

/*****************************************/
/* Callbacks for handling messages */

void handle_get_adc_val(unsigned char* buffer, int len) {
  uint32_t adcNumber;
  pb_istream_t stream = pb_istream_from_buffer(buffer, len);
  bool status;
  GetAdcVal message;
  status = pb_decode(&stream, GetAdcVal_fields, &message);
  if (status) {
    adcNumber = message.adcNum;
    //Works up to here, I get adcNum

    pb_ostream_t ostream;
    uint8_t outbuf[64]; //max size
    ostream = pb_ostream_from_buffer(outbuf, 64);
    uint8_t type = RETURN_ADC_VAL;
    pb_write(&ostream, &type, 1);

    ReturnAdcVal message;

    //As stuff gets added or switched out, software can keep the same numbers
    switch(adcNumber){
      case 0:
        message.adcVal=*VREFINT_CAL;
        break;
      case 1:
        message.adcVal=*TEMP30_CAL;
        break;
      case 2:
        message.adcVal=*TEMP110_CAL;
        break;
      case 3:
        message.adcVal=g_adcVals[0]; //PA2
        break;
      case 4:
        message.adcVal=g_adcVals[1]; //PA3
        break;
      case 5:
        message.adcVal=g_adcVals[2]; //Temperature
        break;
      case 6:
        message.adcVal=g_adcVals[3]; //Vref
        break;
      default:
        message.adcVal=0;
        break;
    }

    /*
    if (adcNumber < ADC_CHANS+1){
      if (adcNumber==0)
        message.adcVal=g_adcCal;
      else
        message.adcVal = g_adcVals[adcNumber-1];
    }
    else
      message.adcVal = 0; //just in case they ask for outside numbers, deals with stack overflow
    */

    if(!pb_encode(&ostream, ReturnAdcVal_fields, &message)) {
      return;
    }
    serialio_write(outbuf, ostream.bytes_written);
	}
}


void handle_debug(unsigned char* buffer, int len) {
  pb_istream_t stream = pb_istream_from_buffer(buffer, len);
  bool status;
  SetDebug message;
  status = pb_decode(&stream, SetDebug_fields, &message);
  if (status) {
    g_debug = message.debug;
  }
}

void handle_reboot_bootloader(unsigned char* buffer, int len) {
  pb_istream_t stream = pb_istream_from_buffer(buffer, len);
  bool status;
  EnterBootloader message;
  status = pb_decode(&stream, SetDebug_fields, &message);
  if (status) {
		RebootToBootloader();
  }
}

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

static char g_serial_number[9];

/** Writes two ascii characters into buf to represent b in hex */
void hex_byte_to_chars(uint8_t b, char* buf) {
  uint8_t upper = (b & 0xf0) >> 4;
  uint8_t lower = (b & 0x0f);

  if (upper < 10) {
    buf[0] = 0x30 + upper;
  } else {
    buf[0] = 0x41 + (upper - 10);
  }

  if (lower < 10) {
    buf[1] = 0x30 + lower;
  } else {
    buf[1] = 0x41 + (lower - 10);
  }
}

void set_identify_serial_number(uint32_t serial_number) {
  hex_byte_to_chars((serial_number & 0xff000000) >> 24, &g_serial_number[0]);
  hex_byte_to_chars((serial_number & 0x00ff0000) >> 16, &g_serial_number[2]);
  hex_byte_to_chars((serial_number & 0x0000ff00) >> 8, &g_serial_number[4]);
  hex_byte_to_chars((serial_number & 0x000000ff), &g_serial_number[6]);  
  g_serial_number[8] = 0;
}

void handle_identify(unsigned char* buffer, int len) {
  /* Identify message has nothing interesting in it to read */
  pb_ostream_t stream;
  uint8_t outbuf[64];
  stream = pb_ostream_from_buffer(outbuf, 64);
  char* swrev = VERSION;
  char* hwrev = "1.14";
  char* sn = g_serial_number;

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

