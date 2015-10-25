#ifndef __SERIALIO__H__
#define __SERIALIO__H__

typedef enum {
  SEARCHING = 1,
  READING = 2,
  READING_ESCAPED = 3,
  DONE = 4
} serial_state_t;

typedef struct {
  uint8_t message_type;
  void (*callback)(unsigned char* buffer, int len);
} type_callback_map_t;

typedef enum {
  NACK = 0,
  ACK = 1,
  MOVE = 2,
  DRIP_RECORDED = 3,
  SET_DRIP_COUNT = 4,
  MOVE_TO_DRIP_COUNT = 5,
  MEASURE = 6,
  IDENTIFY = 7,
  IAM = 8,
	DEBUG = 9,
	REBOOT = 10
} message_types_t;

void serialio_feed(void);
void serialio_write(unsigned char* buffer, uint8_t len);
void set_identify_serial_number(uint32_t serial_number);

#endif
