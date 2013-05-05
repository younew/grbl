/*
  spindle_control.c - spindle control methods
  Part of Grbl

  Copyright (c) 2009-2011 Simen Svale Skogsrud
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

#include "settings.h"
#include "spindle_control.h"
#include "planner.h"

static uint8_t current_direction;

void spindle_init()
{
  GPIO_InitTypeDef  GPIO_InitStructure;
  
  current_direction = 0;
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_25MHz;
  
  GPIO_InitStructure.GPIO_Pin = SPINDLE_ENABLE_BIT;
  GPIO_Init(SPINDLE_ENABLE_PORT, &GPIO_InitStructure);
  GPIO_InitStructure.GPIO_Pin = SPINDLE_DIRECTION_BIT;
  GPIO_Init(SPINDLE_DIRECTION_PORT, &GPIO_InitStructure);
  spindle_stop();
}

void spindle_stop()
{
  //SPINDLE_ENABLE_PORT &= ~(1<<SPINDLE_ENABLE_BIT);
  SPINDLE_STOP;
}

void spindle_run(int8_t direction) //, uint16_t rpm) 
{
  if (direction != current_direction) {
    plan_synchronize();
    if (direction) {
      if(direction > 0) {
        SPINDLE_CW;//SPINDLE_DIRECTION_PORT &= ~(1<<SPINDLE_DIRECTION_BIT);
      } else {
        SPINDLE_CCW;//SPINDLE_DIRECTION_PORT |= 1<<SPINDLE_DIRECTION_BIT;
      }
      SPINDLE_RUN;//SPINDLE_ENABLE_PORT |= 1<<SPINDLE_ENABLE_BIT;
    } else {
      spindle_stop();     
    }
    current_direction = direction;
  }
}
