/*
* Ser25lcxxx library
* Copyright (c) 2010 Hendrik Lipka
* 
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
* 
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
* 
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
* THE SOFTWARE.
*/

#ifndef __SER25LCXXX_H__
#define __SER25LCXXX_H__

#include "stm32f4xx.h"
#include "config.h"

#define _size       128   // EEPEOMµÄÈÝÁ¿£¨byte)
#define _pageSize   16

#define HIGH(x) ((x&0xff00)>>8)
#define LOW(x) (x&0xff)

#define SPI_CS_H             GPIO_SetBits(SPI_FLASH_PORT ,SPI_FLASH_CS_PIN)
#define SPI_CS_L             GPIO_ResetBits(SPI_FLASH_PORT ,SPI_FLASH_CS_PIN)

void SPI_FLASH_Init(void);
u8 SpiWrByte(u8 data);
u8 SpiRdByte(u8 data);
void EEPROM25LC010WaitForWrite(void);
void EEPROM25LC010EnableWrite(void);
/**
    read a part of the eeproms memory. The buffer will be allocated here, and must be freed by the user
    @param startAdr the adress where to start reading. Doesn't need to match a page boundary
    @param len the number of bytes to read (must not exceed the end of memory)
    @return NULL if the adresses are out of range, the pointer to the data otherwise
*/
u8 EEPROM25LC010Read(unsigned int startAdr,unsigned int len,char* data ) ;

/**
    writes the give buffer into the memory. This function handles dividing the write into 
    pages, and waites until the phyiscal write has finished
    @param startAdr the adress where to start writing. Doesn't need to match a page boundary
    @param len the number of bytes to read (must not exceed the end of memory)
    @return false if the adresses are out of range
*/
u8 EEPROM25LC010Write(unsigned int startAdr, unsigned int len, char* data);

/**
    fills the given page with 0xFF
    @param pageNum the page number to clear
    @return if the pageNum is out of range        
*/
u8 EEPROM25LC010ClearPage(unsigned int pageNum);

/**
    fills the while eeprom with 0xFF
*/
void EEPROM25LC010ClearMem();

#endif

