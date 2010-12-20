/*
  controls.h - support for local controls like speed dial and emergency stop
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
#ifndef controls_h
#define controls_h

#include "config.h"

void controls_init();

#ifdef FULL_SPEED_LED_DDR
void fullSpeedLedOn();
void fullSpeedLedOff();
#endif

#ifdef SIGNAL_LED_DDR
void signalLedOn();
void signalLedOff();
#endif

#ifdef SPEED_CONTROL_PIN
// Reads the speed control pot and returns a speed multiplier
float readSpeedControlPot();
#endif      
      
#ifdef EMERGENCY_STOP_BUTTON_DDR
uint8_t pollEmergencyStopButton();
#endif

#endif