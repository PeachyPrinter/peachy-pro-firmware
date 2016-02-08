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

uint8_t g_laser_state=0;
uint8_t g_laser_leds=0;
uint16_t g_laser_button_debounce=0;

void delay_ms(int ms) {
  uint32_t end = tick + (ms*2);
  while(tick < end);
}

void laser_handler(){
  g_laser_button_debounce = g_laser_button_debounce<<1;
  g_laser_button_debounce += getDebugSwitch();
  if ((g_laser_state == 0 ) & (g_laser_button_debounce==0xFFFF)) {
    g_laser_state = 1;
    g_laser_leds=(g_laser_leds+1)%5;
  }else if (g_laser_button_debounce==0){
    g_laser_state = 0;
  }
}

void led_handler(){
  switch (g_laser_leds){
    case 0:
      setCornerLed(0);
      setCoilLed(0);
      setInLed(0);
      setUSBLed(0);
      set_pwm(0,0,0);
      break;
    case 1:
      setCornerLed(1);
      set_pwm(0,0,128);
      break;
    case 2:
      setCoilLed(1);
      set_pwm(0,0,170);
      break;
    case 3:
      setInLed(1);
      set_pwm(0,0,212);
      break;
    case 4:
      setUSBLed(1);
      set_pwm(0,0,255);
      break;
    default:
      setCornerLed(0);
      setCoilLed(0);
      setInLed(0);
      setUSBLed(0);
      break;
  }
}

void SysTick_Handler(void) {
  tick += 1;
  laser_handler();
  led_handler();
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
	play_long_spin(); //Spin the led's while we load the rest of this stuff

  //setup_keycard();
  initialize_pwm();
  initialize_dripper();

	setCornerLed(0);
	setInLed(0);
	setCoilLed(0);
	setUSBLed(0);

  SysTick_Config(SystemCoreClock / 2000); //48MHz/2000 gives us 24KHz, so a count of 24000 should be 1 second?

  int last_drip_count = g_dripcount;

  while(1) {
      //setUSBLed(getDebugSwitch());
  }
}
