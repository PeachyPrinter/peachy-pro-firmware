#include "stm32f0xx_conf.h"
#include "stm32f0xx_tim.h"
#include "stm32f0xx_gpio.h"

#include "iolib.h"
#include "serialio.h"
#include "pwmout.h"
#include "dripper.h"
#include "reprog.h"
#include "hwaccess.h"
#include "clock.h"
#include "keycard.h"
#include "ADClockout.h"
#include "led_override.h"
#include "overrides.h"

#include <usb_core.h>
#include <usb_cdc.h>
#include <i2c.h>

extern volatile uint32_t g_dripcount;
extern volatile uint32_t g_dripghosts;
extern uint32_t g_key_beeps;
extern uint32_t g_key_beep_counter;

volatile uint32_t tick = 0;
bool g_debug=0;
bool g_checkcoils=1;
bool g_twig_coils=1;
int g_key_coil_gate=1;
uint8_t move_start = 0;
uint8_t move_count = 0;
Move move_buffer[MOVE_SIZE];


void delay_ms(int ms) {
  uint32_t end = tick + (ms*2);
  while(tick < end);
}

void coilBuzzer(void){
  if((g_key_coil_gate==1) & (g_key_beeps>0)){

    if ((g_key_beeps&0x1)==1){ //beep on odd counts
      buzzCoilStep();
    }
    g_key_beep_counter=g_key_beep_counter-1;
    if (g_key_beep_counter==0){
      g_key_beeps=g_key_beeps-1;
      g_key_beep_counter=KEY_TONE_LENGTH;
    }
  }
  else if(g_twig_coils){
    twigCoils();
  }
}

void toggle_dripper(){
  uint8_t ndripper_toggle_bit;
	ndripper_toggle_bit = !(GPIO_ReadOutputDataBit(GPIOA,GPIO_Pin_0));
	GPIO_WriteBit(GPIOA, GPIO_Pin_0,ndripper_toggle_bit);
}

void SysTick_Handler(void) {
  tick += 1;
  if (tick & 0x1)
    set_pwm(0,0,255);
  else
    set_pwm(0,0,0);
}

void init_serial_number() {
  uint32_t* serial_base = (uint32_t*)0x1FFFF7AC;
  // 96-bit serial number = 4 words
  uint32_t part1 = *serial_base;
  uint32_t part2 = *(serial_base+1);
  uint32_t part3 = *(serial_base+2);
  uint32_t part4 = *(serial_base+3);
  
  uint32_t hashed = part1 ^ part2 ^ part3 ^ part4;
  USB_SetSerial(hashed);
  set_identify_serial_number(hashed);
}

int main(void)
{
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOA, ENABLE);
  RCC_AHBPeriphClockCmd(RCC_AHBPeriph_GPIOB, ENABLE);

	setupJP5();
	setupJP6();
	setupLeds();

	initialize_led_override();
	LED_OVERRIDES_EN){
	play_long_spin(); //Spin the led's while we load the rest of this stuff
	
  	initialize_pwm();
 	 initialize_dripper();

	setCornerLed(0);
	setInLed(0);
	setCoilLed(0);
	setUSBLed(0);

  SysTick_Config(SystemCoreClock / 2000); //48MHz/2000 gives us 24KHz, so a count of 24000 should be 1 second?

  int last_drip_count = g_dripcount;

  while(1) {
    serialio_feed();
    updateADC();
    if (move_count!=0){
      g_twig_coils=0;
      g_key_coil_gate=0;
    }
    if (g_dripcount != last_drip_count) {
      last_drip_count = g_dripcount;
      send_updated_drip_count();
    }
  }
}
