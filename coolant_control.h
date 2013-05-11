/*
  coolant_control.h - spindle control methods
  Part of Grbl

  Copyright (c) 2012 Sungeun K. Jeon

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

#ifndef coolant_control_h
#define coolant_control_h 

#include "stm32f4xx.h"

#define COOLANT_MIST_ENABLE 2
#define COOLANT_FLOOD_ENABLE 1
#define COOLANT_DISABLE 0 // Must be zero.

#define COOLANT_FLOOD_STOP           SPINDLE_ENABLE_PORT->BSRRH = SPINDLE_ENABLE_BIT
#define COOLANT_FLOOD_RUN             SPINDLE_ENABLE_PORT->BSRRL = SPINDLE_ENABLE_BIT//高电平

#define COOLANT_MIST_STOP           SPINDLE_ENABLE_PORT->BSRRH = SPINDLE_ENABLE_BIT
#define COOLANT_MIST_RUN             SPINDLE_ENABLE_PORT->BSRRL = SPINDLE_ENABLE_BIT//高电平

void coolant_init();
void coolant_stop();
void coolant_run(u8 mode);

#endif