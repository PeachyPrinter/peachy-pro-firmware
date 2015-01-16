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
  void (*callback)(char* buffer, int len);
} type_callback_map_t;

void serialio_feed(void);

#endif
