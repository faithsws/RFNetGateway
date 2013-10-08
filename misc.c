#include "stm8s.h"
#include "string.h"

__IO uint32_t TimingDelay = 0;
void lcd_backlight_init()
{}

void lcd_backlight_on(unsigned char on)
{}

void timer1_init()
{}
void led_init()
{}

void led_r_on(unsigned char on)
{}
void led_g_on(unsigned char on)
{}
void led_y_on(unsigned char on)
{}

void delay1ms(__IO uint32_t nTime)
{
    TimingDelay = nTime;

  while (TimingDelay != 0);
}
void TimingDelay_Decrement(void)
{
  if (TimingDelay != 0x00)
  {
    TimingDelay--;
  }
}

void delay1us(__IO uint32_t us)
{
  uint32_t i = us*16;
  while(i-- > 1);
}
unsigned char get_r_led()
{
  return 1;
}
unsigned char get_g_led()
{
  return 1;
}
unsigned char get_y_led()
{
  return 1;
}
///////////////////////////////
void lcd_print(char * str)
{

}
void eeprom_read_block ( uint8_t * val, uint8_t * def, uint8_t len )
{
	memcpy(val,def,len);
}