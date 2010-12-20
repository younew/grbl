// A module to poll AVR analog inputs

#ifndef analog_h
#define analog_h
                    
void analog_init();

uint16_t analog_read(uint8_t channel);

#endif