/*
  serial.c - Low level functions for sending and recieving bytes via the serial port
  Part of Grbl

  Copyright (c) 2009-2011 Simen Svale Skogsrud
  Copyright (c) 2011-2012 Sungeun K. Jeon

  Grbl is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Grbl is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Grbl.  If not, see <http://www.gnu.org/licenses/>.
*/

/* This code was initially inspired by the wiring_serial module by David A. Mellis which
   used to be a part of the Arduino project. */ 

#include "serial.h"
#include "config.h"
#include "motion_control.h"
#include "protocol.h"

uint8_t rx_buffer[RX_BUFFER_SIZE];
volatile uint32_t rx_buffer_head = 0;
volatile uint32_t rx_buffer_tail = 0;

uint8_t tx_buffer[TX_BUFFER_SIZE];
uint32_t tx_buffer_head = 0;
volatile uint16_t tx_buffer_tail = 0;

#ifdef ENABLE_XONXOFF
  volatile uint8_t flow_ctrl = XON_SENT; // Flow control state variable
  
  // Returns the number of bytes in the RX buffer. This replaces a typical byte counter to prevent
  // the interrupt and main programs from writing to the counter at the same time.
  static uint8_t get_rx_buffer_count()
  {
    if (rx_buffer_head == rx_buffer_tail) { return(0); }
    if (rx_buffer_head < rx_buffer_tail) { return(rx_buffer_tail-rx_buffer_head); }
    return (RX_BUFFER_SIZE - (rx_buffer_head-rx_buffer_tail));
  }
#endif
//-------------------------------配置刻串口 IO口--------------------------------
//  
static void serial_io_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
  NVIC_InitTypeDef NVIC_InitStructure;
  
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
  
  GPIO_PinAFConfig(MY_USART_PC_GPIO_PORT, MY_USART_PC_TX_SOURCE, MY_USART_PC_TX_AF);
  GPIO_PinAFConfig(MY_USART_PC_GPIO_PORT, MY_USART_PC_RX_SOURCE, MY_USART_PC_RX_AF);
  
  /* Configure USART Tx as alternate function  */
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;

  GPIO_InitStructure.GPIO_Pin = MY_USART_PC_GPIO_TxPin;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(MY_USART_PC_GPIO_PORT, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_Pin = MY_USART_PC_GPIO_RxPin;
  GPIO_Init(MY_USART_PC_GPIO_PORT, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = MY_USART_PC_GPIO_CTSPin;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;//GPIO_Mode_IN;
	GPIO_Init(MY_USART_PC_GPIO_CTS_GPIO_PORT, &GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin = MY_USART_PC_GPIO_RTSPin;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_Init(MY_USART_PC_GPIO_RTS_GPIO_PORT, &GPIO_InitStructure);
	//取消阻塞接收,可以接收
	ResetUSART_RTS;
  NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure); 
}
void serial_init(void)
{
	USART_InitTypeDef USART_InitStructure;
  
  serial_io_init();
  
  USART_InitStructure.USART_BaudRate = BAUD_RATE;//19200;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//USART_HardwareFlowControl_RTS_CTS;//
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

  USART_Init(MY_USART_PC, &USART_InitStructure);
  USART_ITConfig(MY_USART_PC, USART_IT_TXE, DISABLE);
	USART_ITConfig(MY_USART_PC, USART_IT_RXNE, ENABLE);
	USART_Cmd(MY_USART_PC, ENABLE);
  
  USART_InitStructure.USART_BaudRate = 9600;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

	USART_Init(USART_RS485, &USART_InitStructure);
  USART_ITConfig(USART_RS485, USART_IT_TXE, DISABLE);
	USART_ITConfig(USART_RS485, USART_IT_RXNE, ENABLE);
	USART_Cmd(USART_RS485, ENABLE);
}
void serial_write(uint8_t data) {
  // Calculate next head
  uint8_t next_head = tx_buffer_head + 1;
  if (next_head == TX_BUFFER_SIZE) { next_head = 0; }

  // Wait until there is space in the buffer
  while (next_head == tx_buffer_tail) { 
    if (sys.execute & EXEC_RESET) { return; } // Only check for abort to avoid an endless loop.
  }

  // Store data and advance head
  tx_buffer[tx_buffer_head] = data;
  tx_buffer_head = next_head;
  
  // Enable Data Register Empty Interrupt to make sure tx-streaming is running
  //UCSR0B |=  (1 << UDRIE0); 
  USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
}
void USART1_IRQHandler(void)
{
	u8 data;
	static u32 next_head;
  static u32 data_count = 0;
	//接收中断
	if (USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
	{
		data = USART1->DR;//USART_ReceiveData(MY_USART);
		  // Pick off runtime command characters directly from the serial stream. These characters are
		// not passed into the buffer, but these set system state flag bits for runtime execution.
		switch (data) {
			case CMD_STATUS_REPORT: sys.execute |= EXEC_STATUS_REPORT; break; // Set as true
			case CMD_CYCLE_START:   sys.execute |= EXEC_CYCLE_START; break; // Set as true
			case CMD_FEED_HOLD:     sys.execute |= EXEC_FEED_HOLD; break; // Set as true
			case CMD_RESET:         mc_reset(); break; // Call motion control reset routine.
			default: // Write character to buffer   
        data_count ++;
			  next_head = rx_buffer_head + 1;
			  if (next_head == RX_BUFFER_SIZE) { next_head = 0; }

			  // Write data to buffer unless it is full.
			  if (next_head != rx_buffer_tail) {
			    rx_buffer[rx_buffer_head] = data;
			    rx_buffer_head = next_head;    
			    
			    #ifdef ENABLE_XONXOFF
			      if ((get_rx_buffer_count() >= RX_BUFFER_FULL) && flow_ctrl == XON_SENT) {
			        flow_ctrl = SEND_XOFF;
			        //UCSR0B |=  (1 << UDRIE0); // Force TX
			        USART_ITConfig(USART1, USART_IT_TXE, ENABLE);
			      } 
			    #endif
			    
		  	}
        else
        {
          next_head = 0;
        }
		}
	}
	//发送中断
	if (USART_GetITStatus(USART1, USART_IT_TXE) == SET)
	{
		// Temporary tx_buffer_tail (to optimize for volatile)
		uint8_t tail = tx_buffer_tail;

#ifdef ENABLE_XONXOFF
		if (flow_ctrl == SEND_XOFF) { 
		  USART1->DR = XOFF_CHAR; 
		  flow_ctrl = XOFF_SENT; 
		} else if (flow_ctrl == SEND_XON) { 
		  USART1->DR = XON_CHAR; 
		  flow_ctrl = XON_SENT; 
		} else
#endif
		{ 
			// Send a byte from the buffer	
			USART1->DR = tx_buffer[tail];

			// Update tail position
			tail++;
			if (tail == TX_BUFFER_SIZE) { tail = 0; }

			tx_buffer_tail = tail;
		}
  
	  	// Turn off Data Register Empty Interrupt to stop tx-streaming if this concludes the transfer
	  	if (tail == tx_buffer_head) 
			{ USART_ITConfig(USART1, USART_IT_TXE, DISABLE);}
	}
	//溢出-如果发生溢出需要先读SR,再读DR寄存器 则可清除不断入中断的问题
	if(USART_GetFlagStatus(USART1,USART_FLAG_ORE)==SET)
	{
		USART_ClearFlag(USART1, USART_FLAG_ORE);	//读SR
		USART_ReceiveData(USART1);				//读DR
	}
}
uint8_t serial_read()
{
  if (rx_buffer_head == rx_buffer_tail) {
    return SERIAL_NO_DATA;
  } else {
    uint8_t data = rx_buffer[rx_buffer_tail];
    rx_buffer_tail++;
    if (rx_buffer_tail == RX_BUFFER_SIZE) { rx_buffer_tail = 0; }

    #ifdef ENABLE_XONXOFF
      if ((get_rx_buffer_count() < RX_BUFFER_LOW) && flow_ctrl == XOFF_SENT) { 
        flow_ctrl = SEND_XON;
        UCSR0B |=  (1 << UDRIE0); // Force TX
      }
    #endif
    
    return data;
  }
}

void serial_reset_read_buffer() 
{
  rx_buffer_tail = rx_buffer_head;

  #ifdef ENABLE_XONXOFF
    flow_ctrl = XON_SENT;
  #endif
}
