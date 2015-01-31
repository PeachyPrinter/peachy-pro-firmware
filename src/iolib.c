#include "iolib.h"


int usbrxheadptr=0, usbrxtailptr=0;
unsigned char usbrxbuffer[64];

// USB putchar function
void PutChar(unsigned char outchar)
{
}

// USB get char no wait
int GetCharnw(void)
{
  return -1;
}

// USB get char wait
unsigned char GetChar(void)
{
	int intchar;

	while((intchar=GetCharnw())==-1) ;
	return (unsigned int) intchar;
}

void Puts(const char *outstrg)
{
}
