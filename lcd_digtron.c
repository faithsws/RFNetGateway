#include "main.h"
#include "stm8s.h"

#define BIAS_DUTY   
#define RC_256    0x18
#define LCD_ON    0x03
#define SYS_EN    0x01
#define TONE_OFF  0x08



static void cs(char h)
{
  if(h)
     GPIO_WriteHigh(LCDDIG_CS_PORT,LCDDIG_CS_PIN);
  else
     GPIO_WriteLow(LCDDIG_CS_PORT,LCDDIG_CS_PIN);
}
static void wr(char h)
{
  if(h)
     GPIO_WriteHigh(LCDDIG_PORT,LCDDIG_WR_PIN);
  else
     GPIO_WriteLow(LCDDIG_PORT,LCDDIG_WR_PIN);
}
static void rd(char h)
{
  if(h)
     GPIO_WriteHigh(LCDDIG_PORT,LCDDIG_RD_PIN);
  else
     GPIO_WriteLow(LCDDIG_PORT,LCDDIG_RD_PIN);
}
static void data(char h)
{
  GPIO_Init(LCDDIG_PORT, (GPIO_Pin_TypeDef)(LCDDIG_DATA_PIN ), GPIO_MODE_OUT_PP_LOW_FAST);
  if(h)
     GPIO_WriteHigh(LCDDIG_PORT,LCDDIG_DATA_PIN);
  else
     GPIO_WriteLow(LCDDIG_PORT,LCDDIG_DATA_PIN);
}
static BitStatus data_in()
{
  GPIO_Init(LCDDIG_PORT, (GPIO_Pin_TypeDef)(LCDDIG_DATA_PIN ), GPIO_MODE_IN_FL_NO_IT);
  return GPIO_ReadInputPin(LCDDIG_PORT,(GPIO_Pin_TypeDef)(LCDDIG_DATA_PIN ));
}

static void delay()
{
  unsigned char i = 10;
  while(i --);
}
static void start()
{
  cs(1);
  wr(1);
  rd(1);
  data(1);
  
  delay();
  delay();
  cs(0);
  delay();
}
static void stop()
{
  cs(1);
  wr(1);
  rd(1);
  data(1);
  
  delay();

}
static void send(char buf,char len)
{
  unsigned char i;
  for(i = 0;i < len;i ++)
  {
    if(buf & 0x80)data(1);
    else data(0);
    wr(0);
    delay();
    wr(1);
    delay();
    buf <<= 1;
  }
}
static unsigned char  receive()
{
  BitStatus bit;
  unsigned char data = 0;
  char i;
  for(i = 0;i<4;i++)
  {
    rd(0);
    delay();
    rd(1);
    bit = data_in();
    if(bit == RESET)
    {
      
    }
    else
    {
      data |= 0x01;
    }
    data <<= 1;
    delay();
  }
  return data >> 1;

}

unsigned char lcddig_read(char addr)
{
  unsigned char i;
  start();
  send(0xc0,3);
  send(addr <<2,6);
  
  i = receive();

  stop();
  
  return i;
}
void lcddig_write(char addr,char buf)
{
  start();
  
  send(0xa0,3);
  send(addr << 2,6); 
  send(buf<<4,4);
  stop();
}

void lcddig_command(char buf)
{
  unsigned char i;
  unsigned len = 1;
  start();
  send(0x80,3);
  for(i = 0;i<len;i ++)
  {
    send(buf,8);
    send(0x00,1); 
  }
  stop();
}
void lcddig_write_same(char seg,char data)
{
  lcddig_write(seg*2,data);
  lcddig_write(seg*2+1,data);
}
char lcddig_read_same(char seg)
{
  return lcddig_read(seg*2);
}
void lcddig_poweron(char on)
{
  char data = lcddig_read_same(17);
  if(on)
    lcddig_write_same(17,data | 0x08);
  else
    lcddig_write_same(17,data &(~ 0x08));
}
unsigned mod[16] = {0x5f,0x06,0x3d,0x2f,0x66,0x6b,0x7b,0x0e,0x7f,0x6f,/*A*/ 0x7e,0x73,0x59,0x37,0x79,0x78};
static void display_digital(unsigned char bit,unsigned char d)
{
  if(bit == 0)
  {
    lcddig_write_same(6,mod[d] & 0x0f);
    lcddig_write_same(11,(mod[d] >> 4) & 0x7);
  }
  else if(bit == 1)
  {
    lcddig_write_same(5,mod[d] & 0x0f);
    lcddig_write_same(12,(mod[d] >> 4) & 0x7);
  }
  else if(bit == 2)
  {
    lcddig_write_same(4,mod[d] & 0x0f);
    lcddig_write_same(13,(mod[d] >> 4) & 0x7);
  }
  else if(bit == 3)
  {
    lcddig_write_same(3,mod[d] & 0x0f);
    lcddig_write_same(14,(mod[d] >> 4) & 0x7);
  }
  else if(bit == 4)
  {
    lcddig_write_same(2,mod[d] & 0x0f);
    lcddig_write_same(15,(mod[d] >> 4) & 0x7);
  }
  else if(bit == 5)
  {
    lcddig_write_same(1,mod[d] & 0x0f);
    lcddig_write_same(16,(mod[d] >> 4) & 0x7);
  }
  else if(bit == 6)
  {
    lcddig_write_same(0,mod[d] & 0x0f);
    lcddig_write_same(17,(mod[d] >> 4) & 0x7);
  }
  else if(bit == 7)
  {
    lcddig_write_same(18,mod[d] & 0x0f);
    lcddig_write_same(19,(mod[d] >> 4) & 0x7);
  }
}
void lcddig_clear()
{
  char i;
  for(i = 0;i< 20;i++)
  {
    lcddig_write_same(i,0);
  }
}
void lcddig_display_int(unsigned short data)
{
  unsigned char i;
  lcddig_clear();
  
   i = data / 10000;
  display_digital(7,i);
  data = data % 10000;
   i = data / 1000;
  display_digital(6,i);
  data = data % 1000;
   i = data / 100;
  display_digital(5,i);
  data = data % 100;
   i = data / 10;
  display_digital(4,i);
  data = data % 10;
   
  display_digital(3,data);
  
}
void lcddig_display_hex(unsigned short data)
{
  
  unsigned char i  = 0;
  lcddig_clear();
  i = (unsigned char)(data>>12)&0x0f;
  display_digital(7,i);
  i = (unsigned char)(data>>8)&0x0f;
  display_digital(6,i);
  i = (unsigned char)(data>>4)&0x0f;
  display_digital(5,i);
  i = (unsigned char)(data>>0)&0x0f;
  display_digital(4,i);
}
void lcddig_display_time(unsigned char time[7])
{
  display_digital(7,time[4]/10);
  display_digital(6,time[4]%10);
  display_digital(5,time[3]/10);
  display_digital(4,time[3]%10);
  display_digital(3,time[2]/10);
  display_digital(2,time[2]%10);
  display_digital(1,time[1]/10);
  display_digital(0,time[1]%10);
}
void lcddig_init()
{
  GPIO_Init(LCDDIG_PORT, (GPIO_Pin_TypeDef)(LCDDIG_DATA_PIN |LCDDIG_WR_PIN|LCDDIG_RD_PIN), GPIO_MODE_OUT_PP_LOW_FAST);
  GPIO_Init(LCDDIG_CS_PORT, (GPIO_Pin_TypeDef)(LCDDIG_CS_PIN), GPIO_MODE_OUT_PP_LOW_FAST);  
  
  lcddig_command(RC_256);
  lcddig_command(SYS_EN);
  lcddig_command(LCD_ON);
  lcddig_command(TONE_OFF);
  

}