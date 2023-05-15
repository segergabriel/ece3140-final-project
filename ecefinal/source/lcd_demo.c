/*
 * lcd_demo.c
 *
 *  Created on: May 6, 2022
 *      Author: Aaron Wilhelm
 */

#include <stdint.h>
#include "lcd.h"


int main(void)
{
    /* Starts LCD display. */
    init_lcd();

    // display 3140
    displayDigit(1, 3);
    displayDigit(2, 1);
    displayDigit(3, 4);
    displayDigit(4, 0);
    LCD_TimeDelay(0xFFFFFFU);

    // display 3.14
    clearDigit(1);
    displayDigit(2, 3);
    displayDigit(3, 1);
    displayDigit(4, 4);
    displayDecimalPoint(2);
    LCD_TimeDelay(0xFFFFFFU);
    clearDisplay();

    // you can also write to the segments directly for more than just digits
    turnOnSegment(2, 'A');
    turnOnSegment(2, 'D');
    turnOnSegment(2, 'E');
    turnOnSegment(2, 'F');
    turnOnSegment(2, 'G');
    turnOnSegment(3, 'A');
    turnOnSegment(3, 'D');
    turnOnSegment(3, 'E');
    turnOnSegment(3, 'F');
    turnOnSegment(4, 'A');
    turnOnSegment(4, 'D');
    turnOnSegment(4, 'E');
    turnOnSegment(4, 'F');
    turnOnSegment(4, 'G');
    LCD_TimeDelay(0xFFFFFFU);
    clearDisplay();

    // an infinite loop that first displays all the digits, then all of the punctuation
    while(1) {
    	// display digits
		for(int iteration = 0; iteration <= 9; iteration++) {
			for(int disp_num = 1; disp_num <= 4; disp_num++) {
				displayDigit(disp_num, iteration);
			}
			LCD_TimeDelay(0xFFFFFFU);
		}
		clearDisplay();

    	// display decimal points and colon
    	for(int dp_num = 1; dp_num <=3; dp_num++) {
    		displayDecimalPoint(dp_num);
    	}
    	displayColon();
    	LCD_TimeDelay(0xFFFFFFU);
    	// clear decimal points and colon
    	for(int dp_num = 1; dp_num <=3; dp_num++) {
    		clearDecimalPoint(dp_num);
    	}
    	clearColon();
    }

}
