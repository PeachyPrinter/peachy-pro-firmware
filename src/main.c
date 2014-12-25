#include "stm32f0xx_conf.h"

void SysTick_Handler(void) {
  static uint16_t tick = 0;
  static char val = 0;

  switch (tick++) {
  	case 100:
  		tick = 0;
		if (val) { val = 0; } else { val = 1; }
  		GPIOA->ODR = (GPIOA->ODR & 0xFFFFFE) | val;
  		break;
  }
}

int main(void)
{
	volatile int x = 0;

	RCC->AHBENR |= RCC_AHBENR_GPIOAEN; 	// enable the clock to GPIOC
						//(RM0091 lists this as IOPCEN, not GPIOCEN)
	__asm("dsb");

	GPIOA->MODER = 0x00000015;

	for(x = 0; x < 100; x++);
	
	SysTick_Config(SystemCoreClock/100);

	while(1);
}
