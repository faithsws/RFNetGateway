#ifndef __MAIN_H__
#define __MAIN_H__
#include "stm8s.h"

#define BUZZER_GPIO_PORT  (GPIOA)
#define BUZZER_PIN GPIO_PIN_3
extern void buzzer_init();
extern void buzzer(char on);


#define TEST  GPIO_PIN_2
#define HALL GPIO_PIN_3

#define DIGTRON_PORT GPIOC
#define SI  GPIO_PIN_4
#define SCK GPIO_PIN_3
#define RCK GPIO_PIN_2
//#define EN  GPIO_PIN_5
#define DIGTRON_GPIO_PINS (SI | SCK | RCK )
extern void digtron_init();
extern void digtron_display(char l);
extern void digtron_display_none();

#define RELAY_GPIO_PORT   GPIOB
#define RELAY_PIN         GPIO_PIN_1
extern void relay_init();
extern void relay(char on);


#define LED_GPIO_PORT   GPIOB
#define LED_PIN          GPIO_PIN_2
extern void led_init();
extern void led_y(char on);
extern void led_y_blink();

#define TEMP_18B20_PORT   GPIOB
#define TEMP_18B20_PIN         GPIO_PIN_0
extern void temp18b20_init();
extern short readTemp(void) ;
void test_time();

extern void uart_init();
extern void uart_send_string(char * str);

#define SCAN_KEY
#define KEY_GPIO_PORT   GPIOB
#define KEY1_PIN        GPIO_PIN_3
// AIN12_PF4
#define AD_KEY_PORT     GPIOF
#define AD_KEY_PIN      GPIO_PIN_4

typedef enum{
  KEY_CENTER = 0,
  KEY_LEFT,
  KEY_TOP,
  KEY_RIGHT,
  KEY_BOTTOM,
  KEY_NONE,
}KEY_CODE;
extern void key_init();
extern KEY_CODE  get_key();


#define LCDDIG_PORT  GPIOC
#define LCDDIG_DATA_PIN   GPIO_PIN_4
#define LCDDIG_WR_PIN   GPIO_PIN_3
#define LCDDIG_RD_PIN   GPIO_PIN_2
#define LCDDIG_CS_PORT  GPIOE
#define LCDDIG_CS_PIN   GPIO_PIN_5

extern void lcddig_init();
extern void lcddig_write(char addr,char buf);
extern void lcddig_write_same(char seg,char data);
extern unsigned char lcddig_read(char addr);
extern void lcddig_poweron(char on);
extern void lcddig_display_int(unsigned short i);
extern void lcddig_display_hex(unsigned short data);
extern void lcddig_display_time(unsigned char time[7]);
void Delay (uint16_t nCount);

#define sEE_I2C                          I2C  
#define sEE_I2C_CLK                      CLK_PERIPHERAL_I2C
#define sEE_I2C_SCL_PIN                  GPIO_PIN_4                  /* PC.01 */
#define sEE_I2C_SCL_GPIO_PORT            GPIOB                       /* GPIOE */
#define sEE_I2C_SDA_PIN                  GPIO_PIN_5                  /* PC.00 */
#define sEE_I2C_SDA_GPIO_PORT            GPIOB                       /* GPIOE */
//#define sEE_M24C64_32


#define sEE_DIRECTION_TX                 0
#define sEE_DIRECTION_RX       1


#define INFR_BUF_SIZE 50
extern void infr_init();
extern char get_infr_code();


#define DS1302_RST    GPIO_PIN_6
#define DS1302_RST_PORT   GPIOB
#define DS1302_SCLK   GPIO_PIN_3
#define DS1302_SCLK_PORT   GPIOD
#define DS1302_DATA   GPIO_PIN_7
#define DS1302_DATA_PORT  GPIOB

extern void ds1302_init();
extern void set_time(unsigned char time[]);
extern void read_time(unsigned char time[7]);


#define SD_SPI_CLK                       CLK_PERIPHERAL_SPI
#define SD_SPI_SCK_PIN                   GPIO_PIN_5                  /* PC.05 */
#define SD_SPI_SCK_GPIO_PORT             GPIOC                       /* GPIOC */
#define SD_SPI_MISO_PIN                  GPIO_PIN_7                  /* PC.05 */
#define SD_SPI_MISO_GPIO_PORT            GPIOC                       /* GPIOC */
#define SD_SPI_MOSI_PIN                  GPIO_PIN_6                  /* PC.06 */
#define SD_SPI_MOSI_GPIO_PORT            GPIOC                       /* GPIOC */
#define SD_CS_PIN                        GPIO_PIN_0                  /* PE.05 */
#define SD_CS_GPIO_PORT                  GPIOD                       /* GPIOE */
//#define SD_DETECT_PIN                                      /* PE.04 */
//#define SD_DETECT_GPIO_PORT                                     /* GPIOE */


extern void tmp75_init();
extern uint16_t tmp75_read_temp();
extern uint8_t tmp75_read_config();
extern int16_t tmp75_get_temp();
extern void tmp75_set_config(uint8_t config);
#endif

