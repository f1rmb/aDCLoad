/*

  Copyright (C) 2014  Lee Wiggins <lee@wigweb.com.au>
                      F1RMB, Daniel Caujolle-Bert <f1rmb.daniel@gmail.com>

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include <Arduino.h>

#include "aDCLoad.h"

/*
Version Control

1.00
Initial upload

1.10
* Fixed an error in the set current function of the DAC
* Added improved current calculation to allow for correction in OP-AMP offset, uses the measured value to adjust the set current.
* General code tidy up

2.0
* full rewrite, using C++. Lot of bug fixes, lot of new features.
* size of the hex should be:
*      text	   data	    bss	    dec	    hex	filename
*     30884	    206	    506	  31596	   7b6c	output/sketch.elf

To - Dos
* Display in engineering units, ie. mA and mW.

*/

/**
 *                            !!!!! BIG FAT WARNING !!!!!
 *                        Should be compiled with "-Os" flag.
 *                Bootloader couldn't be flashed, ISP programming ONLY
 *            -=- use Code::Blocks or the included Makefile to compile -=-
 *
**/

aDCEngine engine(LCD_RS_PIN, LCD_ENABLE_PIN, LCD_D0_PIN, LCD_D1_PIN, LCD_D2_PIN, LCD_D3_PIN, LCD_COLS_NUM, LCD_ROWS_NUM,
                 ENCODER_A_PIN, ENCODER_B_PIN, ENCODER_PB_PIN, ENCODER_STEPS_PER_NOTCH);


void isr()
{
    engine.service();
}

void setup()
{
    engine.setup(isr);
}

void loop()
{
    engine.run();
}
