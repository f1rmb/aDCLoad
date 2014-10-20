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
* size of the hex should be
*     text	   data	    bss	    dec	    hex	filename
*     30792	    206	    506	  31504	   7b10	output/sketch.elf

To - Dos
* Display in engineering units, ie. mA and mW.

*/

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
