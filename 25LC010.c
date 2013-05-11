/*
* Ser25lcxxx library
*/

#include "25LC010.h"
#include "nuts_bolts.h"


static u8 EEPROM25LC010WritePage(unsigned int startAdr, unsigned int len, const char* data);
static int EEPROM25LC010ReadStatus(void);


#define USER_PARAMENT_BYTES         4//一个参数占用的字节数
#define USER_PARAMENT_COUNT         7
#define EEPROM_INIT_OK_ADDRESS      USER_PARAMENT_BYTES * USER_PARAMENT_COUNT


void SPI_FLASH_Init(void)
{
  SPI_InitTypeDef  SPI_InitStructure;
  GPIO_InitTypeDef GPIO_InitStructure;
  
  //Enable SPI_FLASH and GPIOA clocks
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOD, ENABLE);
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI3, ENABLE);
  SPI_I2S_DeInit(SPI_FLASH);
  //GPIO_PinAFConfig(GPIOD, GPIO_PinSource0, GPIO_AF_SPI3);
  GPIO_PinAFConfig(GPIOC, GPIO_PinSource10, GPIO_AF_SPI3);
  GPIO_PinAFConfig(GPIOC, GPIO_PinSource11, GPIO_AF_SPI3);
  GPIO_PinAFConfig(GPIOC, GPIO_PinSource12, GPIO_AF_SPI3);
  
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_DOWN;

  GPIO_InitStructure.GPIO_Pin = SPI_FLASH_CLK_PIN | SPI_FLASH_MOSI_PIN;
  GPIO_Init(SPI_FLASH_PORT, &GPIO_InitStructure);
  
  GPIO_InitStructure.GPIO_Pin = SPI_FLASH_MISO_PIN;
  GPIO_Init(SPI_FLASH_PORT, &GPIO_InitStructure);

  GPIO_InitStructure.GPIO_Pin = SPI_FLASH_CS_PIN;	 
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_Init(SPI_FLASH_CS_PORT, &GPIO_InitStructure);
  SPI_CS_H;
  //SPI_FLASH configuration
  SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;
  SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
  SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
  SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
  SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
  SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
  SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_4;  //8分频，如果是36MHz时钟，则SPI串行时钟的频率是4.5MHz
  SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
  SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_Init(SPI_FLASH, &SPI_InitStructure);
	SPI_Cmd(SPI_FLASH, ENABLE); // Enable SPI_FLASH
}

u8 SpiWrByte(u8 data)
{
  vu8 temp;
  //SPI_CS_L;
  //delay_us(1);
  while ((SPI_FLASH->SR & SPI_I2S_FLAG_TXE) == RESET);
  while ((SPI_FLASH->SR & SPI_I2S_FLAG_BSY) == SET);
  SPI_FLASH->DR = data;
  while((SPI_FLASH->SR & SPI_I2S_FLAG_RXNE) == RESET);
  while((SPI_FLASH->SR & SPI_I2S_FLAG_BSY) == SET);
  temp = SPI_FLASH->DR;
  //delay_us(10);
  //SPI_CS_H;
  //delay_us(1);
  return(temp);
}
u8 SpiRdByte(u8 data)
{
  vu8 temp;
  //SPI_CS_L;
  //delay_us(1);
  while ((SPI_FLASH->SR & SPI_I2S_FLAG_TXE) == RESET);
  while ((SPI_FLASH->SR & SPI_I2S_FLAG_BSY) == SET);
  SPI_FLASH->DR = 0x00;
  while((SPI_FLASH->SR & SPI_I2S_FLAG_RXNE) == RESET);
  while((SPI_FLASH->SR & SPI_I2S_FLAG_BSY) == SET);
  temp = SPI_FLASH->DR;
  //delay_us(10);
  //SPI_CS_H;
  return(temp);
}
u8 EEPROM25LC010Read(unsigned int startAdr,unsigned int len,char* data ) 
{
  // assertion
  if (startAdr+len > _size)
    return 0;
  SPI_CS_L;
  delay_us(1);
  // send address
  if (_size<512) { // 256 and 128 bytes
    SpiWrByte(0x03);
    SpiWrByte(LOW(startAdr));
  } else if (512==_size) { // 4k variant adds 9th address bit to command
    SpiWrByte(startAdr>255?0xb:0x3);
    SpiWrByte(LOW(startAdr));
  } else if (_size<131072) { // everything up to 512k
    SpiWrByte(0x03);
    SpiWrByte(HIGH(startAdr));
    SpiWrByte(LOW(startAdr));
  } else { // 25xx1024, needs 3 byte address
    SpiWrByte(0x03);
    SpiWrByte(startAdr>>16);
    SpiWrByte(HIGH(startAdr));
    SpiWrByte(LOW(startAdr));
  }
  // read data into buffer
  for (int i=0;i<len;i++) {
    data[i]=SpiWrByte(0);
  }
  delay_us(1);
  SPI_CS_H;
  return 1;
}

u8 EEPROM25LC010Write(unsigned int startAdr, unsigned int len, char* data) {
  u8 b;
  if (startAdr+len>_size)
    return 0;

  int ofs=0;
  while (ofs<len) {
    // calculate amount of data to write into current page
    int pageLen=_pageSize-((startAdr+ofs)%_pageSize);
    if (ofs+pageLen>len)
        pageLen=len-ofs;
    // write single page
    b=EEPROM25LC010WritePage(startAdr+ofs,pageLen,data+ofs);
    if (!b)
        return 0;
    // and switch to next page
    ofs+=pageLen;
  }
  return 1;
}

static u8 EEPROM25LC010WritePage(unsigned int startAdr, unsigned int len, const char* data) 
{
  EEPROM25LC010EnableWrite();
  SPI_CS_L;
  delay_us(1);
  if (_size<512) { // 256 and 128 bytes
    SpiWrByte(0x02);
    SpiWrByte(LOW(startAdr));
  } else if (512==_size) { // 4k variant adds 9th address bit to command
    SpiWrByte(startAdr>255?0xa:0x2);
    SpiWrByte(LOW(startAdr));
  } else if (_size<131072) { // everything up to 512k
    SpiWrByte(0x02);
    SpiWrByte(HIGH(startAdr));
    SpiWrByte(LOW(startAdr));
  } else { // 25xx1024, needs 3 byte address
    SpiWrByte(0x02);
    SpiWrByte(startAdr>>16);
    SpiWrByte(HIGH(startAdr));
    SpiWrByte(LOW(startAdr));
  }

  // do real write
  for (int i=0;i<len;i++) {
    SpiWrByte(data[i]);
  }
  SPI_CS_H;
  delay_us(1);
  // disable to start physical write

  EEPROM25LC010WaitForWrite();

  return 1;
}

u8 EEPROM25LC010ClearPage(unsigned int pageNum) 
{
  char  s[_pageSize];
  u8 b;
  EEPROM25LC010EnableWrite();
  if (_size<65535) 
  {
    for (int i=0;i<_pageSize;i++) 
    {
        s[i]=0xff;
    }
    b=EEPROM25LC010WritePage(_pageSize*pageNum,_pageSize,s);
    return b;
  } 
  else 
  {
    delay_us(1);
    SpiWrByte(0x42);
    SpiWrByte(HIGH(_pageSize*pageNum));
    SpiWrByte(LOW(_pageSize*pageNum));
    delay_us(1);
    EEPROM25LC010WaitForWrite();
  }
  return 1;
}

void EEPROM25LC010ClearMem() 
{
    EEPROM25LC010EnableWrite();
    if (_size<65535) 
    {
        for (int i=0;i<_size/_pageSize;i++) 
        {
            if (!clearPage(i))
                break;
        }
    }
    else
    {
        delay_us(1);
        SpiWrByte(0xc7);
        delay_us(1);
        EEPROM25LC010WaitForWrite();
    }
}

static int EEPROM25LC010ReadStatus() 
{
  SPI_CS_L;
  delay_us(1);
  SpiWrByte(0x5);
  int status=SpiWrByte(0x00);
  delay_us(1);
  SPI_CS_H;
  return status;
}

void EEPROM25LC010WaitForWrite() 
{
  while (1) 
  {
    if (0==EEPROM25LC010ReadStatus()&1)
        break;
    delay_us(10);
  }
}

void EEPROM25LC010EnableWrite()
{
  SPI_CS_L;
  delay_us(1);
  SpiWrByte(0x06);
  delay_us(1);
  SPI_CS_H;
}
