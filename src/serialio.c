#include "serialio.h"
#include "iolib.h"

void serialio_feed() {
  static char buffer[32] = {0};
  static uint8_t idx = 0;

  int input = 0;

  while((input = GetCharnw()) != -1) {
    buffer[idx] = (char)(input & 0xFF);
    PutChar(buffer[idx]);
    idx++;
    buffer[idx] = '\0';
    
    if (idx >= 32) {
      Puts("overflow\r\n");
      idx = 0;
      buffer[0] = '\0';
    }

    if (idx && ((buffer[idx-1] == '\r') || (buffer[idx-1] == '\n'))) {
      idx = 0;
      buffer[0] = '\0';
      Puts("reset!\r\n");
    }
  }
}

