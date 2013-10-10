//********************************************************************************************
//
// File : enc28j60.c Microchip ENC28J60 Ethernet Interface Driver
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
#include "enc28j60.h"
//
extern void delay1ms(__IO uint32_t nTime);
extern void delay1us(__IO uint32_t us);


static uint8_t Enc28j60Bank;
static uint16_t NextPacketPtr;

#define SD_DUMMY_BYTE   0x00
uint8_t SPISendByte(uint8_t Data)
{

#ifdef USE_SPI_MODULE
  /*!< Wait until the transmit buffer is empty */
  while (SPI_GetFlagStatus(SPI_FLAG_TXE) == RESET)
  {}

  /*!< Send the byte */
  SPI_SendData(Data);

  /*!< Wait to receive a byte*/
  while (SPI_GetFlagStatus(SPI_FLAG_RXNE) == RESET)
  {}

  /*!< Return the byte read from the SPI bus */
  return SPI_ReceiveData();
#else
  uint8_t i;
  for(i=0;i<8;i++)
  {
	GPIO_WriteLow(ENC_SCK_PORT,ENC_SCK_PIN);
        if(Data & 0x80)
          GPIO_WriteHigh(ENC_MOSI_PORT,ENC_MOSI_PIN);
        else
          GPIO_WriteLow(ENC_MOSI_PORT,ENC_MOSI_PIN);
	GPIO_WriteHigh(ENC_SCK_PORT,ENC_SCK_PIN);
	Data <<=1;
  }
  GPIO_WriteLow(ENC_SCK_PORT,ENC_SCK_PIN);
  
  return 0;
#endif
  
}
uint8_t SPIReadByte(void)
{
  uint8_t Data = 0;
#ifdef USE_SPI_MODULE
  

  /*!< Wait until the transmit buffer is empty */
  while (SPI_GetFlagStatus(SPI_FLAG_TXE) == RESET)
  {}
  /*!< Send the byte */
  SPI_SendData(SD_DUMMY_BYTE);

  /*!< Wait until a data is received */
  while (SPI_GetFlagStatus(SPI_FLAG_RXNE) == RESET)
  {}
  /*!< Get the received data */
  Data = SPI_ReceiveData();
#else
  
  uint8_t i;
  GPIO_WriteLow(ENC_SCK_PORT,ENC_SCK_PIN);
  for(i=0;i<8;i++)
  {	
	GPIO_WriteHigh(ENC_SCK_PORT,ENC_SCK_PIN);
	Data <<=1;
        
        if(0 != GPIO_ReadInputPin(ENC_MISO_PORT,ENC_MISO_PIN)) 
          Data |= 0x01;
	GPIO_WriteLow(ENC_SCK_PORT,ENC_SCK_PIN);
  }
  
  
#endif 
  /*!< Return the shifted data */
  return Data;
  
}
//*******************************************************************************************
//
// Function : icmp_send_request
// Description : Send ARP request packet to destination.
//
//*******************************************************************************************
BYTE enc28j60ReadOp(BYTE op, BYTE address)
{
        BYTE data;
	// activate CS
	CSACTIVE;
        
	// issue read command
	SPISendByte(op | (address & ADDR_MASK));
	// read data
	data = SPIReadByte();
	// do dummy read if needed (for mac and mii, see datasheet page 29)
	if(address & 0x80)
	{
		data = SPIReadByte();
	}

	CSPASSIVE;
	return data;
}
//*******************************************************************************************
//
// Function : icmp_send_request
// Description : Send ARP request packet to destination.
//
//*******************************************************************************************
void enc28j60WriteOp(BYTE op, BYTE address, BYTE data)
{
	CSACTIVE;

	SPISendByte(op | (address & ADDR_MASK));
	
	SPISendByte( data);
	CSPASSIVE;
}
void enc28j60ReadBuffer(uint16_t len, uint8_t* data)
{
        CSACTIVE;

        SPISendByte(ENC28J60_READ_BUF_MEM);
        while(len)
        {
                len--;

                *data = SPIReadByte();
                data++;
        }
        *data='\0';
        CSPASSIVE;
}
void enc28j60WriteBuffer(uint16_t len, uint8_t* data)
{
        CSACTIVE;
        
        SPISendByte(ENC28J60_WRITE_BUF_MEM);
        
        while(len)
        {
                len--;
                
                SPISendByte(*data);
                data++;
                
        }
        CSPASSIVE;
}

//*******************************************************************************************
//
// Function : icmp_send_request
// Description : Send ARP request packet to destination.
//
//*******************************************************************************************
void enc28j60SetBank(BYTE address)
{
	// set the bank (if needed)
	if((address & BANK_MASK) != Enc28j60Bank)
	{
		// set the bank
		enc28j60WriteOp(ENC28J60_BIT_FIELD_CLR, ECON1, (ECON1_BSEL1|ECON1_BSEL0));
		enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, ECON1, (address & BANK_MASK)>>5);
		Enc28j60Bank = (address & BANK_MASK);
	}
}
//*******************************************************************************************
//
// Function : icmp_send_request
// Description : Send ARP request packet to destination.
//
//*******************************************************************************************
BYTE enc28j60Read(BYTE address)
{
	// select bank to read
	enc28j60SetBank(address);
	
	// do the read
	return enc28j60ReadOp(ENC28J60_READ_CTRL_REG, address);
}
//*******************************************************************************************
//
// Function : icmp_send_request
// Description : Send ARP request packet to destination.
//
//*******************************************************************************************
void enc28j60Write(BYTE address, BYTE data)
{
	// select bank to write
	enc28j60SetBank(address);

	// do the write
	enc28j60WriteOp(ENC28J60_WRITE_CTRL_REG, address, data);
}
//*******************************************************************************************
//
// Function : icmp_send_request
// Description : Send ARP request packet to destination.
//
//*******************************************************************************************
WORD enc28j60_read_phyreg(BYTE address)
{
	WORD data;
	
	// set the PHY register address
	enc28j60Write(MIREGADR, address);
	enc28j60Write(MICMD, MICMD_MIIRD);
        delay1us(15);
	
	// Loop to wait until the PHY register has been read through the MII
	// This requires 10.24us
	while( (enc28j60Read(MISTAT) & MISTAT_BUSY) );
	
	// Stop reading
	enc28j60Write(MICMD, MICMD_MIIRD);
	
	// Obtain results and return
	data = enc28j60Read ( MIRDL );
	data |= enc28j60Read ( MIRDH );

	return data;
}
//*******************************************************************************************
//
// Function : icmp_send_request
// Description : Send ARP request packet to destination.
//
//*******************************************************************************************
void enc28j60PhyWrite(BYTE address, uint16_t data)
{
	// set the PHY register address
	enc28j60Write(MIREGADR, address);
	// write the PHY data
	enc28j60Write(MIWRL, data);
        enc28j60Write(MIWRH, data>>8);
	// wait until the PHY write completes
	while(enc28j60Read(MISTAT) & MISTAT_BUSY)
	{
		delay1us(15);
	}
}


void enc28j60_init( BYTE *avr_mac)
{
  BYTE val;     
  
#ifdef USE_SPI_MODULE  
  GPIO_Init(ENC28J60_PORT, (GPIO_Pin_TypeDef)(ENC28J60_RST |ENC28J60_CS) ,GPIO_MODE_OUT_PP_HIGH_SLOW);
  GPIO_ExternalPullUpConfig(SD_SPI_SCK_GPIO_PORT, (GPIO_Pin_TypeDef)(SD_SPI_MISO_PIN | SD_SPI_MOSI_PIN | \
                            SD_SPI_SCK_PIN), DISABLE);

        CLK_PeripheralClockConfig(SD_SPI_CLK, ENABLE);
        SPI_Init( SPI_FIRSTBIT_MSB, SPI_BAUDRATEPRESCALER_4, SPI_MODE_MASTER,SPI_CLOCKPOLARITY_LOW, SPI_CLOCKPHASE_1EDGE, SPI_DATADIRECTION_2LINES_FULLDUPLEX,
           SPI_NSS_SOFT, 0x07);
        SPI_Cmd( ENABLE);
        
        CSPASSIVE;
        
        GPIO_WriteLow(ENC28J60_PORT,ENC28J60_RST);
        delay1ms(10);
        GPIO_WriteHigh(ENC28J60_PORT,ENC28J60_RST);
        delay1ms(200);
        CSPASSIVE;
#else
    GPIO_Init(ENC_RST_PORT, (GPIO_Pin_TypeDef)(ENC_RST_PIN) ,GPIO_MODE_OUT_PP_HIGH_SLOW);     
    GPIO_Init(ENC_SCK_PORT, (GPIO_Pin_TypeDef)( ENC_SCK_PIN),GPIO_MODE_OUT_PP_LOW_SLOW);
    
    GPIO_Init(ENC_MISO_PORT, (GPIO_Pin_TypeDef)( ENC_MISO_PIN),GPIO_MODE_IN_PU_NO_IT);
    GPIO_Init(ENC_CS_PORT, (GPIO_Pin_TypeDef)(ENC_CS_PIN) ,GPIO_MODE_OUT_PP_HIGH_SLOW);
    GPIO_Init(ENC_MOSI_PORT, (GPIO_Pin_TypeDef)(ENC_MOSI_PIN) ,GPIO_MODE_OUT_PP_HIGH_SLOW);
    CSACTIVE;
    CSPASSIVE;
        
    
    GPIO_WriteLow(ENC_RST_PORT,ENC_RST_PIN);
    delay1ms(10);
    GPIO_WriteHigh(ENC_RST_PORT,ENC_RST_PIN);
    delay1ms(200);
    CSPASSIVE;
#endif
        

	// perform system reset
	enc28j60WriteOp(ENC28J60_SOFT_RESET, 0, ENC28J60_SOFT_RESET);
	delay1ms(50);

	// check CLKRDY bit to see if reset is complete
	// The CLKRDY does not work. See Rev. B4 Silicon Errata point. Just wait.
	//while(!(enc28j60Read(ESTAT) & ESTAT_CLKRDY));
	// do bank 0 stuff
	// initialize receive buffer
	// 16-bit transfers, must write low byte first
	// set receive buffer start address
	NextPacketPtr = RXSTART_INIT;
        // Rx start
	enc28j60Write(ERXSTL, RXSTART_INIT&0xFF);
	enc28j60Write(ERXSTH, RXSTART_INIT>>8);
	// set receive pointer address
	enc28j60Write(ERXRDPTL, RXSTART_INIT&0xFF);
	enc28j60Write(ERXRDPTH, RXSTART_INIT>>8);
	// RX end
	enc28j60Write(ERXNDL, RXSTOP_INIT&0xFF);
	enc28j60Write(ERXNDH, RXSTOP_INIT>>8);
	// TX start
	enc28j60Write(ETXSTL, TXSTART_INIT&0xFF);
	enc28j60Write(ETXSTH, TXSTART_INIT>>8);
	// TX end
	enc28j60Write(ETXNDL, TXSTOP_INIT&0xFF);
	enc28j60Write(ETXNDH, TXSTOP_INIT>>8);
	// do bank 1 stuff, packet filter:
        // For broadcast packets we allow only ARP packtets
        // All other packets should be unicast only for our mac (MAADR)
        //
        // The pattern to match on is therefore
        // Type     ETH.DST
        // ARP      BROADCAST
        // 06 08 -- ff ff ff ff ff ff -> ip checksum for theses bytes=f7f9
        // in binary these poitions are:11 0000 0011 1111
        // This is hex 303F->EPMM0=0x3f,EPMM1=0x30
	enc28j60Write(ERXFCON, ERXFCON_UCEN|ERXFCON_CRCEN|ERXFCON_PMEN);
	enc28j60Write(EPMM0, 0x3f);
	enc28j60Write(EPMM1, 0x30);
	enc28j60Write(EPMCSL, 0xf9);
	enc28j60Write(EPMCSH, 0xf7);
        //
        //
	// do bank 2 stuff
	// enable MAC receive
	enc28j60Write(MACON1, MACON1_MARXEN|MACON1_TXPAUS|MACON1_RXPAUS);
	// bring MAC out of reset
	enc28j60Write(MACON2, 0x00);
	// enable automatic padding to 60bytes and CRC operations
	enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, MACON3, MACON3_PADCFG0|MACON3_TXCRCEN|MACON3_FRMLNEN);
	// set inter-frame gap (non-back-to-back)
	enc28j60Write(MAIPGL, 0x12);
	enc28j60Write(MAIPGH, 0x0C);
	// set inter-frame gap (back-to-back)
	enc28j60Write(MABBIPG, 0x12);
	// Set the maximum packet size which the controller will accept
    // Do not send packets longer than MAX_FRAMELEN:
	enc28j60Write(MAMXFLL, MAX_FRAMELEN&0xFF);	
	enc28j60Write(MAMXFLH, MAX_FRAMELEN>>8);
	
	// do bank 3 stuff
    // write MAC address
	// NOTE: MAC address in ENC28J60 is byte-backward
	// ENC28J60 is big-endian avr gcc is little-endian
	enc28j60Write(MAADR5, avr_mac[0]);
	enc28j60Write(MAADR4, avr_mac[1]);
	enc28j60Write(MAADR3, avr_mac[2]);
	enc28j60Write(MAADR2, avr_mac[3]);
	enc28j60Write(MAADR1, avr_mac[4]);
	enc28j60Write(MAADR0, avr_mac[5]);
	
        
        
       
	// no loopback of transmitted frames
	enc28j60PhyWrite(PHCON2, PHCON2_HDLDIS);
	// switch to bank 0
	enc28j60SetBank(ECON1);

	// enable interrutps
	enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, EIE, EIE_INTIE|EIE_PKTIE);

	// enable packet reception
	enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_RXEN);

	delay1ms(20);
}
//*******************************************************************************************
//
// Function : enc28j60getrev
// Description : read the revision of the chip.
//
//*******************************************************************************************
BYTE enc28j60getrev(void)
{
	return(enc28j60Read(EREVID));
}

// link status
uint8_t enc28j60linkup(void)
{
        // bit 10 (= bit 3 in upper reg)
	return(enc28j60PhyReadH(PHSTAT2) && 4);
}

void enc28j60_packet_send(uint8_t* packet,uint16_t len)
{
	// Set the write pointer to start of transmit buffer area
	enc28j60Write(EWRPTL, TXSTART_INIT&0xFF);
	enc28j60Write(EWRPTH, TXSTART_INIT>>8);
	// Set the TXND pointer to correspond to the packet size given
	enc28j60Write(ETXNDL, (TXSTART_INIT+len)&0xFF);
	enc28j60Write(ETXNDH, (TXSTART_INIT+len)>>8);
	// write per-packet control byte (0x00 means use macon3 settings)
	enc28j60WriteOp(ENC28J60_WRITE_BUF_MEM, 0, 0x00);
	// copy the packet into the transmit buffer
	enc28j60WriteBuffer(len, packet);
	// send the contents of the transmit buffer onto the network
	enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, ECON1, ECON1_TXRTS);

	// Reset the transmit logic problem. See Rev. B4 Silicon Errata point 12.
	if( (enc28j60Read(EIR) & EIR_TXERIF) )
	{
		enc28j60WriteOp(ENC28J60_BIT_FIELD_CLR, ECON1, ECON1_TXRTS);
	}
}
//*******************************************************************************************
//
// Function : enc28j60_mac_is_linked
// Description : return MAC link status.
//
//*******************************************************************************************
/*
BYTE enc28j60_mac_is_linked(void)
{
	if ( (enc28j60_read_phyreg(PHSTAT1) & PHSTAT1_LLSTAT ) )
		return 1;
	else
		return 0;
}
*/
//*******************************************************************************************
//
// Function : enc28j60_packet_receive
// Description : check received packet and return length of data
//
//*******************************************************************************************
//WORD data_length;
WORD enc28j60_packet_receive ( BYTE *packet, uint16_t max_length )
{
	uint16_t rxstat;
	uint16_t len;
	// check if a packet has been received and buffered
	// if( !(enc28j60Read(EIR) & EIR_PKTIF) ){
	// The above does not work. See Rev. B4 Silicon Errata point 6.
	if( enc28j60Read(EPKTCNT) == 0 )
	{
		return 0;
	}

	// Set the read pointer to the start of the received packet
	enc28j60Write(ERDPTL, (NextPacketPtr));
	enc28j60Write(ERDPTH, (NextPacketPtr)>>8);
	// read the next packet pointer
	NextPacketPtr  = enc28j60ReadOp(ENC28J60_READ_BUF_MEM, 0);
	NextPacketPtr |= enc28j60ReadOp(ENC28J60_READ_BUF_MEM, 0)<<8;
	// read the packet length (see datasheet page 43)
	len  = enc28j60ReadOp(ENC28J60_READ_BUF_MEM, 0);
	len |= enc28j60ReadOp(ENC28J60_READ_BUF_MEM, 0)<<8;
        len-=4; //remove the CRC count
	// read the receive status (see datasheet page 43)
	rxstat  = enc28j60ReadOp(ENC28J60_READ_BUF_MEM, 0);
	rxstat |= ((uint16_t)enc28j60ReadOp(ENC28J60_READ_BUF_MEM, 0))<<8;
	// limit retrieve length
        if (len>max_length-1){
                len=max_length-1;
        }
        // check CRC and symbol errors (see datasheet page 44, table 7-3):
        // The ERXFCON.CRCEN is set by default. Normally we should not
        // need to check this.
        if ((rxstat & 0x80)==0){
                // invalid
                len=0;
        }else{
                // copy the packet from the receive buffer
                enc28j60ReadBuffer(len, packet);
        }
	// Move the RX read pointer to the start of the next received packet
	// This frees the memory we just read out
	enc28j60Write(ERXRDPTL, (NextPacketPtr));
	enc28j60Write(ERXRDPTH, (NextPacketPtr)>>8);
	// decrement the packet counter indicate we are done with this packet
	enc28j60WriteOp(ENC28J60_BIT_FIELD_SET, ECON2, ECON2_PKTDEC);
	return(len);
}

