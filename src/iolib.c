#include "iolib.h"

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
