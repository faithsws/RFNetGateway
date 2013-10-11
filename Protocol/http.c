//********************************************************************************************
//
// File : http.c implement for Hyper Text transfer Protocol
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
#include "fatfs/ff.h"
#include "tcp.h"
//********************************************************************************************
//
// Global variable for http.c
//
//********************************************************************************************
prog_int8_t web_title[] = "faithsws的网站-李鸿宇是笨蛋";
prog_int8_t tag_br[] = "<br>";
prog_int8_t tag_hr[] = "<hr width=\"100%\" size=\"1\"><br>";
prog_int8_t tag_form[] = "<form action=\"./?\" method=\"get\">";
//********************************************************************************************
//
// Function : http_webserver_process
// Description : Initial connection to web server
//
//********************************************************************************************
void http_webserver_process ( BYTE *rxtx_buffer, BYTE *dest_mac, BYTE *dest_ip )
{
	WORD dlength, dest_port;
	
	dest_port = (rxtx_buffer[TCP_SRC_PORT_H_P]<<8)|rxtx_buffer[TCP_SRC_PORT_L_P];
	// tcp port 80 start for web server
	if ( rxtx_buffer [ IP_PROTO_P ] == IP_PROTO_TCP_V && rxtx_buffer[ TCP_DST_PORT_H_P ] == 0 && rxtx_buffer[ TCP_DST_PORT_L_P ] == 80 )
	{
		// received packet with flags "SYN", let's send "SYNACK"
		if ( (rxtx_buffer[ TCP_FLAGS_P ] & TCP_FLAG_SYN_V) )
		{
//			tcp_send_synack ( rxtx_buffer, dest_mac, dest_ip );
			tcp_send_packet (
				rxtx_buffer,
				(WORD_BYTES){dest_port},
				(WORD_BYTES){80},					// source port
				TCP_FLAG_SYN_V|TCP_FLAG_ACK_V,			// flag
				1,						// (bool)maximum segment size
				0,						// (bool)clear sequence ack number
				1,						// (bool)calculate new seq and seqack number
				0,						// tcp data length
				dest_mac,		// server mac address
				dest_ip );		// server ip address
			flag1.bits.syn_is_received = 1;
			return;
		}

		if ( (rxtx_buffer [ TCP_FLAGS_P ] & TCP_FLAG_ACK_V) )
		{
			// get tcp data length
			dlength = tcp_get_dlength( rxtx_buffer );
			if ( dlength == 0 )
			{
				// finack, answer with ack
				if ( (rxtx_buffer[TCP_FLAGS_P] & TCP_FLAG_FIN_V) )
				{
//					tcp_send_ack ( rxtx_buffer, dest_mac, dest_ip );
					tcp_send_packet (
						rxtx_buffer,
						(WORD_BYTES){dest_port},
						(WORD_BYTES){80},						// source port
						TCP_FLAG_ACK_V,			// flag
						0,						// (bool)maximum segment size
						0,						// (bool)clear sequence ack number
						1,						// (bool)calculate new seq and seqack number
						0,						// tcp data length
						dest_mac,		// server mac address
						dest_ip );		// server ip address
				}
				return;
			}
#if 0			
			// get avr ip address from request and set to new avr ip address
			if ( http_get_variable ( rxtx_buffer, dlength, PSTR( "aip" ), generic_buf ) )
			{
			#if 0
				if ( http_get_ip ( generic_buf, (BYTE*)&avr_ip ) == 4 )
					eeprom_write_block ( &avr_ip, ee_avr_ip, 4 );
				eeprom_read_block ( &avr_ip, ee_avr_ip, 4 );
			#endif
			}
			// get server ip address from request and set to new server ip address
			if ( http_get_variable ( rxtx_buffer, dlength, PSTR( "sip" ), generic_buf ) )
			{
			#if 0
				if ( http_get_ip ( generic_buf, (BYTE*)&server_ip ) == 4 )
					eeprom_write_block ( &server_ip, ee_server_ip, 4 );
				eeprom_read_block ( &server_ip, ee_server_ip, 4 );
			#endif
			}
			// get LED1 on/of command
			if ( http_get_variable ( rxtx_buffer, dlength, PSTR( "l1" ), generic_buf ) )
			{
				if ( generic_buf[0] == '0' )
					led_g_on(0);
				else
					led_g_on(1);
			}
			// get LED1 on/of command
			if ( http_get_variable ( rxtx_buffer, dlength, PSTR( "l2" ), generic_buf ) )
			{
				if ( generic_buf[0] == '0' )
					led_r_on(0);
				else
					led_r_on(1);
			}
			// get LCD string and show on first line
			if ( http_get_variable ( rxtx_buffer, dlength, PSTR( "lcd1" ), generic_buf ) )
			{
				urldecode ( generic_buf );
				lcd_putc ( '\f' );
				lcd_print ( generic_buf );
				flag1.bits.lcd_busy = 1;
			}
			// get LCD string and show on second line
			if ( http_get_variable ( rxtx_buffer, dlength, PSTR( "lcd2" ), generic_buf ) )
			{
				urldecode ( generic_buf );
				lcd_putc ( '\n' );
				lcd_print ( generic_buf );
				flag1.bits.lcd_busy = 1;
			}
			// get send temparature to server configuration
			if ( http_get_variable ( rxtx_buffer, dlength, PSTR( "tc" ), generic_buf ) )
			{
				// enable or disable send temparature
				if ( http_get_variable ( rxtx_buffer, dlength, PSTR( "en" ), generic_buf ) )
					count_time_temp[0] = 1;
				else
					count_time_temp[0] = 0;
				// get hour
				if ( http_get_variable ( rxtx_buffer, dlength, PSTR( "h" ), generic_buf ) )
				{
					count_time_temp[1] = (generic_buf[0] - '0') * 10;
					count_time_temp[1] = count_time_temp[1] + (generic_buf[1] - '0');
				}
				// get minute
				if ( http_get_variable ( rxtx_buffer, dlength, PSTR( "m" ), generic_buf ) )
				{
					count_time_temp[2] = (generic_buf[0] - '0') * 10;
					count_time_temp[2] = count_time_temp[2] + (generic_buf[1] - '0');
				}
				// write config to eeprom
				//eeprom_write_block ( count_time_temp, ee_count_time, 3 );
				//eeprom_read_block ( count_time, ee_count_time, 3 );
				//count_time[3] = 0;
			}
#endif
			// print webpage
			dlength = http_home( rxtx_buffer );
			// send ack before send data
//			tcp_send_ack ( rxtx_buffer, dest_mac, dest_ip );
			tcp_send_packet (
						rxtx_buffer,
						(WORD_BYTES){dest_port},
						(WORD_BYTES){80},						// source port
						TCP_FLAG_ACK_V,			// flag
						0,						// (bool)maximum segment size
						0,						// (bool)clear sequence ack number
						1,						// (bool)calculate new seq and seqack number
						0,						// tcp data length
						dest_mac,		// server mac address
						dest_ip );		// server ip address
			// send tcp data
//			tcp_send_data ( rxtx_buffer, dest_mac, dest_ip, dlength );
			tcp_send_packet (
						rxtx_buffer,
						(WORD_BYTES){dest_port},
						(WORD_BYTES){80},						// source port
						TCP_FLAG_ACK_V | TCP_FLAG_PSH_V | TCP_FLAG_FIN_V,			// flag
						0,						// (bool)maximum segment size
						0,						// (bool)clear sequence ack number
						0,						// (bool)calculate new seq and seqack number
						dlength,				// tcp data length
						dest_mac,		// server mac address
						dest_ip );		// server ip address
			flag1.bits.syn_is_received = 0;
		
		}	

	}
}
//********************************************************************************************
//
// Function : http_get_ip
// Description : Get IP address from buffer (stored after call http_get_variable function)
// example after call http_get_variable function ip address (ascii) has been stored in buffer
// 10.1.1.1 (ascii), http_get_ip function convert ip address in ascii to binary and stored
// in BYTE *dest
//
//********************************************************************************************
unsigned char http_get_ip ( unsigned char *buf, BYTE *dest )
{
	unsigned char i, ch, digit, temp;

	i = 0;
	digit = 1;
	temp = 0;

	while ( 1 )
	{
		ch = *buf++;

		if ( ch >= '0' && ch <= '9' )
		{
			ch = ch - '0';
			temp = (temp * digit) + ch;
			digit *= 10;
		}
		else if ( ch == '.' || ch == '\0' )
		{
			dest[ i ] = temp;
			i++;
			digit = 1;
			temp = 0;
		}
		else
		{
			return 0;
		}
		if ( i == 4 )
			return i;
	}
}

//********************************************************************************************
//
// Function : hex2int
// Description : convert a single hex digit character to its integer value
//
//********************************************************************************************
unsigned char hex2int(char c)
{
	if (c >= '0' && c <='9')
		return((unsigned char)c - '0');

	if (c >= 'a' && c <='f')
		return((unsigned char)c - 'a' + 10);
	
	if (c >= 'A' && c <='F')
		return((unsigned char)c - 'A' + 10);

	return 0;
}
//********************************************************************************************
//
// Function : urldecode
// Description : decode a url string e.g "hello%20joe" or "hello+joe" becomes "hello joe"
//
//********************************************************************************************
void urldecode(unsigned char *urlbuf)
{
	unsigned char c;
	unsigned char *dst;

	dst=urlbuf;
	while ((c = *urlbuf))
	{
		if (c == '+') c = ' ';
		if (c == '%')
		{
			urlbuf++;
			c = *urlbuf;
			urlbuf++;
			c = (hex2int(c) << 4) | hex2int(*urlbuf);
		}
		*dst = c;
		dst++;
		urlbuf++;
	}
	*dst = '\0';
}
//由于堆栈限制，使用SD卡和Fatfs，每个html必须小于800字节
//800 字节以下的网页
WORD http_home( BYTE *rxtx_buffer )
{
	WORD dlen;
#if 1
        FIL fp;
        FRESULT fr;
        UINT br;
        fr = f_open (&fp, "0:baidu.htm", FA_READ);
        fr = f_read (&fp, rxtx_buffer+TCP_DATA_P, fp.fsize, &br);
        f_close(&fp);
	//dlen = tcp_puts_data_p ( rxtx_buffer, PSTR ( "HTTP/1.0 200 OK\r\nContent-Type: text/html\r\n\r\n" ), 0 );
	dlen = br;
#endif
        
        
	return(dlen);
}
