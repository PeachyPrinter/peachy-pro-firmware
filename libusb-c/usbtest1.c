#include <stdio.h>
#include "libusb.h"

int main(int argc, char* argv[]) {
  int r;
  struct libusb_device_handle *devh;
  char out[64] = { 0x40, 0x07, 0x41 };
  char in[64] = { 0 };

  printf("libusb test program\n");

  r = libusb_init(NULL);
  if (r < 0) {
    printf("Failed to initialize libusb\n");
    return 1;
  }

  libusb_set_debug(NULL, 3);

  devh = libusb_open_device_with_vid_pid(NULL, 0x16d0, 0x0af3);
  if(!devh) {
    printf("Failed to get device\n");
    return 2;
  }

  printf("Claiming interface\n");
  r = libusb_claim_interface(devh, 0);
  if (r < 0) {
    printf("Failed to claim device\n");
    return 3;
  }
  
  printf("Sending message\n");
  int count = 0;
  r = libusb_bulk_transfer(devh, 0x02, out, 3, &count, 1000);
  if (r != 0) {
    printf("Sending failed: %d\n", r);
    return 4;
  }
  
  count = 0;
  r = libusb_bulk_transfer(devh, 0x83, in, 64, &count, 1000);
  if (r != 0) {
    printf("Receiving failed: %d\n", r);
    return 5;
  }
  printf("Received %d bytes\n", count);

  return 0;
}
