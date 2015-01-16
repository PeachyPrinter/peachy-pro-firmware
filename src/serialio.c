#include "serialio.h"
#include "iolib.h"

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


#define HEADER 0x12
#define FOOTER 0x13
#define ESCAPE_CHAR 0x14

void handle_type_1(char* buffer, int len);
void handle_type_2(char* buffer, int len);

serial_state_t serial_searching(uint8_t* idx, char* buffer, char input) {
  if(input != HEADER) {
    return SEARCHING;
  }
  *idx = 0;
  buffer[*idx] = 0;
  return READING;
}

serial_state_t serial_reading(uint8_t* idx, char* buffer, char input) {
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

serial_state_t serial_reading_esc(uint8_t* idx, char* buffer, char input) {
  buffer[*idx] = ~input;
  (*idx) += 1;
  return READING;
}

serial_state_t serial_done(uint8_t* idx, char* buffer) {
  return SEARCHING;
}

void serialio_feed() {
  static char buffer[32] = {0};
  static uint8_t idx = 0;
  static serial_state_t state = SEARCHING;

  int input = 0;

  while((input = GetCharnw()) != -1) {
    switch(state) {
    case SEARCHING: state = serial_searching(&idx, buffer, (char)input); break;
    case READING: state = serial_reading(&idx, buffer, (char)input); break;
    case READING_ESCAPED: state = serial_reading_esc(&idx, buffer, (char)input); break;
    default:
      break;
    }
    if (state == DONE) { // Special case DONE because it doesn't need any input
      state = serial_done(&idx, buffer); 
    }
  }
}

