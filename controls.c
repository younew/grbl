/*
  controls.c - support for local controls like speed dial and emergency stop
  Part of Grbl

  Copyright (c) 2009 Simen Svale Skogsrud

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
#include <avr/io.h>
#include <util/delay.h>
#include "analog.h"
#include "config.h"


void controls_init() {
#ifdef SPEED_CONTROL_PIN
  analog_init();        
#endif

#ifdef FULL_SPEED_LED_DDR
  FULL_SPEED_LED_DDR |= (1<<FULL_SPEED_LED_PIN);
#endif

#ifdef SIGNAL_LED_DDR
  SIGNAL_LED_DDR |= (1<<SIGNAL_LED_PIN);
#endif

#ifdef EMERGENCY_STOP_BUTTON_DDR
  EMERGENCY_STOP_BUTTON_DDR &= ~(1<<EMERGENCY_STOP_BUTTON_PIN);
#endif
}

#ifdef FULL_SPEED_LED_DDR
void fullSpeedLedOn() {
  FULL_SPEED_LED_PORT |= (1<<FULL_SPEED_LED_PIN);
}

void fullSpeedLedOff() {
  FULL_SPEED_LED_PORT &= ~(1<<FULL_SPEED_LED_PIN);
}
#endif

#ifdef SIGNAL_LED_DDR
void signalLedOn() {
  SIGNAL_LED_PORT |= (1<<SIGNAL_LED_PIN);
}

void signalLedOff() {
  SIGNAL_LED_PORT &= ~(1<<SIGNAL_LED_PIN);
}
#endif

#ifdef EMERGENCY_STOP_BUTTON_DDR
uint8_t pollEmergencyStopButton() {
  if (EMERGENCY_STOP_BUTTON_PORT & (1<<EMERGENCY_STOP_BUTTON_PIN)) {
    _delay_ms(10);
    return(1);
  } else {
    return(0);
  }
}
#endif 

#ifdef SPEED_CONTROL_PIN

// Defines the maximum value where the pot is considered to be
// in speed reduction mode
#define SPEED_REDUCTION_MAX 450
// Defines the minimum value where the pot is considered to be in 
// speed multiplication mode
#define SPEED_MULTIPLICATION_MIN 550
// Implies the size of the multiplication range
#define SPEED_POT_MULTIPLICATION_RANGE (1023-SPEED_MULTIPLICATION_MIN)
// Defines the multiplication at maximum multiplication (must be >= 1.0)
#define MAX_SPEED_MULTIPLICATION 1.25
 
// Reads the speed control pot and returns a speed multiplier
float readSpeedControlPot() {
  uint16_t value = analog_read(SPEED_CONTROL_PIN);
  if (value<=SPEED_REDUCTION_MAX) {
    #ifdef FULL_SPEED_LED_DDR
    fullSpeedLedOff();
    #endif
    // Speed reduction from 1.0 to 0.0
    return((value*1.0)/SPEED_REDUCTION_MAX);
  } else if (value>=SPEED_MULTIPLICATION_MIN) {
    #ifdef FULL_SPEED_LED_DDR
    fullSpeedLedOff();
    #endif
    // Speed multiplication from 1.0 to MAX_SPEED_MULTIPLICATION
    return(((value-SPEED_MULTIPLICATION_MIN)*1.0/SPEED_POT_MULTIPLICATION_RANGE)*(MAX_SPEED_MULTIPLICATION-1.0)+1.0);
  } else {      
    #ifdef FULL_SPEED_LED_DDR
    fullSpeedLedOn();
    #endif
    return(1.0);
  }  
}
#endif