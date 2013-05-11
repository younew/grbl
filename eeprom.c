// This file has been prepared for Doxygen automatic documentation generation.
/*! \file ********************************************************************
*
* Atmel Corporation
*
* \li File:               eeprom.c
* \li Compiler:           IAR EWAAVR 3.10c
* \li Support mail:       avr@atmel.com
*
* \li Supported devices:  All devices with split EEPROM erase/write
*                         capabilities can be used.
*                         The example is written for ATmega48.
*
* \li AppNote:            AVR103 - Using the EEPROM Programming Modes.
*
* \li Description:        Example on how to use the split EEPROM erase/write
*                         capabilities in e.g. ATmega48. All EEPROM
*                         programming modes are tested, i.e. Erase+Write,
*                         Erase-only and Write-only.
*
*                         $Revision: 1.6 $
*                         $Date: Friday, February 11, 2005 07:16:44 UTC $
****************************************************************************/
#include "nuts_bolts.h"
#include "25LC010.h"

/*! \brief  Read byte from EEPROM.
 *
 *  This function reads one byte from a given EEPROM address.
 *
 *  \note  The CPU is halted for 4 clock cycles during EEPROM read.
 *
 *  \param  addr  EEPROM address to read from.
 *  \return  The byte read from the EEPROM address.
 */
unsigned char eeprom_get_char( unsigned int addr )
{
unsigned char data;
    if (addr >= _size)
        return 0;
    SPI_CS_L;
    delay_us(1);
    // send address
    if (_size<512) { // 256 and 128 bytes
        SpiWrByte(0x03);
        SpiWrByte(LOW(addr));
    } else if (512==_size) { // 4k variant adds 9th address bit to command
        SpiWrByte(addr>255?0xb:0x3);
        SpiWrByte(LOW(addr));
    } else if (_size<131072) { // everything up to 512k
        SpiWrByte(0x03);
        SpiWrByte(HIGH(addr));
        SpiWrByte(LOW(addr));
    } else { // 25xx1024, needs 3 byte address
        SpiWrByte(0x03);
        SpiWrByte(addr>>16);
        SpiWrByte(HIGH(addr));
        SpiWrByte(LOW(addr));
    }
    // read data into buffer
    data=SpiWrByte(0);
    delay_us(1);
    SPI_CS_H;
    return(data);
}

/*! \brief  Write byte to EEPROM.
 *
 *  This function writes one byte to a given EEPROM address.
 *  The differences between the existing byte and the new value is used
 *  to select the most efficient EEPROM programming mode.
 *
 *  \note  The CPU is halted for 2 clock cycles during EEPROM programming.
 *
 *  \note  When this function returns, the new EEPROM value is not available
 *         until the EEPROM programming time has passed. The EEPE bit in EECR
 *         should be polled to check whether the programming is finished.
 *
 *  \note  The EEPROM_GetChar() function checks the EEPE bit automatically.
 *
 *  \param  addr  EEPROM address to write to.
 *  \param  new_value  New EEPROM value.
 */
void eeprom_put_char( unsigned int addr, unsigned char new_value )
{
	char old_value; // Old EEPROM value.
	char diff_mask; // Difference mask, i.e. old value XOR new value.

	old_value = eeprom_get_char(addr);
	diff_mask = old_value ^ new_value; // Get bit differences.
	// Check if any bits are changed to '1' in the new value.
	if( diff_mask) 
	{
	    EEPROM25LC010EnableWrite();
	    SPI_CS_L;
	    delay_us(1);
	    if (_size<512) { // 256 and 128 bytes
	        SpiWrByte(0x02);
	        SpiWrByte(LOW(addr));
	    } else if (512==_size) { // 4k variant adds 9th address bit to command
	        SpiWrByte(addr>255?0xa:0x2);
	        SpiWrByte(LOW(addr));
	    } else if (_size<131072) { // everything up to 512k
	        SpiWrByte(0x02);
	        SpiWrByte(HIGH(addr));
	        SpiWrByte(LOW(addr));
	    } else { // 25xx1024, needs 3 byte address
	        SpiWrByte(0x02);
	        SpiWrByte(addr>>16);
	        SpiWrByte(HIGH(addr));
	        SpiWrByte(LOW(addr));
	    }
	        SpiWrByte(new_value);
	    SPI_CS_H;
	    delay_us(1);
	    EEPROM25LC010WaitForWrite();
	}
	//sei(); // Restore interrupt flag state.
}

// Extensions added as part of Grbl 


void memcpy_to_eeprom_with_checksum(unsigned int destination, char *source, unsigned int size) {
  unsigned char checksum = 0;
  for(; size > 0; size--) { 
    checksum = (checksum << 1) || (checksum >> 7);
    checksum += *source;
    eeprom_put_char(destination++, *(source++)); 
  }
  eeprom_put_char(destination, checksum);
}

int memcpy_from_eeprom_with_checksum(char *destination, unsigned int source, unsigned int size) {
  unsigned char data, checksum = 0;
  for(; size > 0; size--) { 
    data = eeprom_get_char(source++);
    checksum = (checksum << 1) || (checksum >> 7);
    checksum += data;    
    *(destination++) = data; 
  }
  return(checksum == eeprom_get_char(source));
}

// end of file
