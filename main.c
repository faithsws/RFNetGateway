//********************************************************************************************
//
// AVRnet firmware Version 1.1
//
// MCU : ATMEGA32 @ 16MHz
// Ethernet controller : ENC28J60
// IDE & Compiler : AVR Studio version 4.13.528 & WINAVR version 20070525
// Author : Jirawat Kongkaen
// Website : http://www.avrportal.com/
//
//********************************************************************************************
//
// File : main.c main program for AVRnet development board.
//
//********************************************************************************************
//
// Copyright (C) 2007
//
// This program is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License as published by the Free Software
// Foundation; either version 2 of the License, or (at your option) any later
// version.
// This program is distributed in the hope that it will be useful, but
//
// WITHOUT ANY WARRANTY;
//
// without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
// PURPOSE. See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License along with
// this program; if not, write to the Free Software Foundation, Inc., 51
// Franklin St, Fifth Floor, Boston, MA 02110, USA
//
// http://www.gnu.de/gpl-ger.html
//
//********************************************************************************************
#include "includes.h"
#include "main.h"
union flag1 flag1;
union flag2 flag2;
// Global variables
MAC_ADDR avr_mac;
IP_ADDR avr_ip;

MAC_ADDR server_mac;
IP_ADDR server_ip;

//BYTE generic_buf[128];

// added in V1.1
prog_int8_t version[] = "AVRnet V1.1";
// end added in V1.1

// Change your avr and server ip address here
// avr and server ip address are stored in eeprom
BYTE ee_avr_ip[4]  = { 192, 168, 1, 5 };
BYTE ee_server_ip[4]  = { 192, 168, 1, 104 };
// added in V1.1 ==========================================================================
//*****************************************************************************************
//
// Function : software_reset
// Description : reset mcu by enable watchdog
//
//*****************************************************************************************
extern void delay1ms(__IO uint32_t nTime);
extern void eeprom_read_block ( uint8_t * val, uint8_t * def, uint8_t len );
#define REMOVE_BOOTLOADER_SUPPORT
#ifndef REMOVE_BOOTLOADER_SUPPORT
void software_reset(void) __attribute__ ((naked));
void software_reset(void)
{
	wdt_enable(WDTO_15MS);
	for(;;);
}
#endif
// end added in V1.1 ======================================================================
//*****************************************************************************************
//
// Function : server_process
// Description : Run web server and listen on port 80
//
//*****************************************************************************************
void server_process ( void )
{
	MAC_ADDR client_mac;
	IP_ADDR client_ip;
	// you can change rx,tx buffer size in includes.h
	BYTE rxtx_buffer[MAX_RXTX_BUFFER];
	WORD plen;
	
	if ( flag1.bits.syn_is_sent )
		return;
	// get new packet
	plen = enc28j60_packet_receive( (BYTE*)&rxtx_buffer, MAX_RXTX_BUFFER );
	
	//plen will ne unequal to zero if there is a valid packet (without crc error)
	if(plen==0)
		return;

	// copy client mac address from buffer to client mac variable
	memcpy ( (BYTE*)&client_mac, &rxtx_buffer[ ETH_SRC_MAC_P ], sizeof(MAC_ADDR) );
	
	// check arp packet if match with avr ip let's send reply
	if ( arp_packet_is_arp( rxtx_buffer, (WORD_BYTES){ARP_OPCODE_REQUEST_V} ) )
	{
		arp_send_reply ( (BYTE*)&rxtx_buffer, (BYTE*)&client_mac );
		return;
	}

	// get client ip address
	memcpy ( (BYTE*)&client_ip, &rxtx_buffer[ IP_SRC_IP_P ], sizeof(IP_ADDR) );
	// check ip packet send to avr or not?
	if ( ip_packet_is_ip ( (BYTE*)&rxtx_buffer ) == 0 )
	{
		return;
	}

	// check ICMP packet, if packet is icmp packet let's send icmp echo reply
	if ( icmp_send_reply ( (BYTE*)&rxtx_buffer, (BYTE*)&client_mac, (BYTE*)&client_ip ) )
	{
		return;
	}
#if 0
	// check UDP packet
	if (udp_receive ( (BYTE *)&rxtx_buffer, (BYTE *)&client_mac, (BYTE *)&client_ip ))
	{
// added in V1.1 ***********************************
#ifndef REMOVE_BOOTLOADER_SUPPORT
		if( flag2.bits.software_reset )
			software_reset();
#endif
// end added in V1.1 *******************************
		return;
	}
#endif
	// tcp start here
	// start web server at port 80, see http.c
	http_webserver_process ( (BYTE*)rxtx_buffer, (BYTE*)&client_mac, (BYTE*)&client_ip );
}
//*****************************************************************************************
//
// Function : client_process
// Description : send temparature to web server, this option is disabled by default.
// YOU MUST install webserver and server script before enable this option, 
// I recommented Apache webserver and PHP script. 
// More detail about Apache and PHP installation please visit http://www.avrportal.com/
//
//*****************************************************************************************
void client_process ( void )
{
	WORD dlength;
	// you can change rx,tx buffer size in includes.h
	BYTE rxtx_buffer[MAX_RXTX_BUFFER];

	// wait for send temparature flag is set, this flag set by time_base function (menu.c)
	if ( flag1.bits.send_temp == 0 )
		return;	
	// AVR busy now and wait untill transfer data to web browser completed.
	if ( flag1.bits.syn_is_received )
		return;
	// AVR sent temparature to web server but not found web server on port 80
	//if ( flag1.bits.not_found_server )
	//	return;
	// send SYN to initial connection
	if ( flag1.bits.syn_is_sent == 0 )
	{
		// start arp 
		// server ip was not found on network
		if ( arp_who_is ( rxtx_buffer, (BYTE*)&server_mac, (BYTE*)&server_ip ) == 0 )
		{
			flag1.bits.send_temp = 0;
			return;
		}
	
		// send SYN packet to initial connection
		tcp_send_packet (
			rxtx_buffer,
			(WORD_BYTES){80},						// destination port
			(WORD_BYTES){1200},					// source port
			TCP_FLAG_SYN_V,			// flag
			1,						// (bool)maximum segment size
			1,						// (bool)clear sequence ack number
			0,						// 0=use old seq, seqack : 1=new seq,seqack no data : new seq,seqack with data
			0,						// tcp data length
			(BYTE*)&server_mac,		// server mac address
			(BYTE*)&server_ip );	// server ip address
		flag1.bits.syn_is_sent = 1;
	}
	// get new packet
	dlength = enc28j60_packet_receive( (BYTE*)&rxtx_buffer, MAX_RXTX_BUFFER );
	
	// no new packet incoming
	if ( dlength == 0 )
	{
		// timeout occured, when SYN has been sent but no response from web server
		// reset send_temp and syn_is_sent flags
		if ( flag1.bits.send_temp_timeout )
		{
			flag1.bits.send_temp_timeout = 0;
			flag1.bits.send_temp = 0;
			flag1.bits.syn_is_sent = 0;
		}
		return;
	}
	
	// check ip packet send to avr or not?
	// accept ip packet only
	if ( ip_packet_is_ip ( (BYTE*)&rxtx_buffer ) == 0 )
	{
		return;
	}

	// check SYNACK flag, after AVR send SYN server response by send SYNACK to AVR
	if ( rxtx_buffer [ TCP_FLAGS_P ] == ( TCP_FLAG_SYN_V | TCP_FLAG_ACK_V ) )
	{
		// send ACK to answer SYNACK
		tcp_send_packet (
					(BYTE*)&rxtx_buffer,
					(WORD_BYTES){80},						// destination port
					(WORD_BYTES){1200},					// source port
					TCP_FLAG_ACK_V,			// flag
					0,						// (bool)maximum segment size
					0,						// (bool)clear sequence ack number
					1,						// 0=use old seq, seqack : 1=new seq,seqack no data : >1 new seq,seqack with data
					0,						// tcp data length
					(BYTE*)&server_mac,		// server mac address
					(BYTE*)&server_ip );	// server ip address
		// setup http request to server
		dlength = http_put_request( (BYTE*)&rxtx_buffer );
		// send http request packet
		// send packet with PSHACK
		tcp_send_packet (
					(BYTE*)&rxtx_buffer,
					(WORD_BYTES){80},						// destination port
					(WORD_BYTES){1200},					// source port
					TCP_FLAG_ACK_V | TCP_FLAG_PSH_V,			// flag
					0,						// (bool)maximum segment size
					0,						// (bool)clear sequence ack number
					0,						// 0=use old seq, seqack : 1=new seq,seqack no data : >1 new seq,seqack with data
					dlength,				// tcp data length
					(BYTE*)&server_mac,		// server mac address
					(BYTE*)&server_ip );	// server ip address
		return;
	}
	// after AVR send http request to server, server response by send data with PSHACK to AVR
	// AVR answer by send ACK and FINACK to server
	if ( rxtx_buffer [ TCP_FLAGS_P ] == (TCP_FLAG_ACK_V|TCP_FLAG_PSH_V) )
	{
		dlength = tcp_get_dlength( (BYTE*)&rxtx_buffer );

		// send ACK to answer PSHACK from server
		tcp_send_packet (
					(BYTE*)&rxtx_buffer,
					(WORD_BYTES){80},						// destination port
					(WORD_BYTES){1200},					// source port
					TCP_FLAG_ACK_V,			// flag
					0,						// (bool)maximum segment size
					0,						// (bool)clear sequence ack number
					dlength,						// 0=use old seq, seqack : 1=new seq,seqack no data : >1 new seq,seqack with data
					0,				// tcp data length
					(BYTE*)&server_mac,		// server mac address
					(BYTE*)&server_ip );	// server ip address
		// send finack to disconnect from web server
		
		tcp_send_packet (
					(BYTE*)&rxtx_buffer,
					(WORD_BYTES){80},						// destination port
					(WORD_BYTES){1200},					// source port
					TCP_FLAG_FIN_V|TCP_FLAG_ACK_V,			// flag
					0,						// (bool)maximum segment size
					0,						// (bool)clear sequence ack number
					0,						// (bool)calculate new seq and seqack number
					0,						// tcp data length
					(BYTE*)&server_mac,		// server mac address
					(BYTE*)&server_ip );	// server ip address
		return;
		//menu_flag.bits.send_temp = 0;
		//send_syn = 0;
	}
	// answer FINACK from web server by send ACK to web server
	if ( rxtx_buffer [ TCP_FLAGS_P ] == (TCP_FLAG_FIN_V|TCP_FLAG_ACK_V) )
	{
		// send ACK with seqack = 1
		tcp_send_packet (
					(BYTE*)&rxtx_buffer,
					(WORD_BYTES){80},						// destination port
					(WORD_BYTES){1200},					// source port
					TCP_FLAG_ACK_V,			// flag
					0,						// (bool)maximum segment size
					0,						// (bool)clear sequence ack number
					1,						// 0=use old seq, seqack : 1=new seq,seqack no data : >1 new seq,seqack with data
					0,				// tcp data length
					(BYTE*)&server_mac,		// server mac address
					(BYTE*)&server_ip );	// server ip address
		// temparature has been sent
		// and wait for next schedule to send temparature
		flag1.bits.send_temp = 0;
		flag1.bits.syn_is_sent = 0;
	}
}
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
static void TIM4_Config(void)
{
  /* TIM4 configuration:
   - TIM4CLK is set to 16 MHz, the TIM4 Prescaler is equal to 128 so the TIM1 counter
   clock used is 16 MHz / 128 = 125 000 Hz
  - With 125 000 Hz we can generate time base:
      max time base is 2.048 ms if TIM4_PERIOD = 255 --> (255 + 1) / 125000 = 2.048 ms
      min time base is 0.016 ms if TIM4_PERIOD = 1   --> (  1 + 1) / 125000 = 0.016 ms
  - In this example we need to generate a time base equal to 1 ms
   so TIM4_PERIOD = (0.001 * 125000 - 1) = 124 */

  /* Time base configuration */
  TIM4_TimeBaseInit(TIM4_PRESCALER_128, 124);
  /* Clear TIM4 update flag */
  TIM4_ClearFlag(TIM4_FLAG_UPDATE);
  /* Enable update interrupt */
  TIM4_ITConfig(TIM4_IT_UPDATE, ENABLE);
  
  /* enable interrupts */
  enableInterrupts();

  /* Enable TIM4 */
  TIM4_Cmd(ENABLE);
}

//*****************************************************************************************
//
// Function : main
// Description : main program, 
//
//*****************************************************************************************
int main (void)
{
        BYTE vr;
       
	TIM4_Config();
  // change your mac address here

	avr_mac.byte[0] = 0x12;
	avr_mac.byte[1] = 0x34;
	avr_mac.byte[2] = 0x56;
	avr_mac.byte[3] = 0x78;
	avr_mac.byte[4] = 0x90;
	avr_mac.byte[5] = 0xab;

	// read avr and server ip from eeprom
	eeprom_read_block ( (uint8_t*)&avr_ip, ee_avr_ip, 4 );
	eeprom_read_block ( (uint8_t*)&server_ip, ee_server_ip, 4 );

	// setup port as input and enable pull-up


	// initial enc28j60
	enc28j60_init( (BYTE*)&avr_mac );
	vr = enc28j60getrev();
// added in V1.1
	//lcd_print_p( (PGM_P)version );
// end added in V1.1

	// loop forever
	for(;;)
	{
		// wait until timer1 overflow
                //delay1ms(4);
#if 0

		// general time base, generate by timer1
		// overflow every 1/250 seconds
		time_base ();
	
		// read temparature
		adc_read_temp();
		
		

		// send temparature to web server unsing http protocol
		// disable by default.
		client_process ();
		
		// lcd user interface menu
		// setup IP address, countdown timer
		menu_process ();

		// display AVR ethernet status
		// temparature, AVR ip, server ip, countdown time
		standby_display ();
#endif


		// server process response for arp, icmp, http
		server_process ();
	}

	return 0;
}
