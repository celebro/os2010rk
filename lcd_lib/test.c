/*
 * test.c
 *
 *  Created on: 7.5.2010
 *      Author: Jure
 */

#include <stdio.h>
#include "display_lib.h"
#include "lcd.h"

struct display* display;

int main(){

	display = get_display();

	if (display == NULL) {
		printf("Failed to get display\n");
		return -1;
	}

	LCDSetRectangle(1,5,3,9,FILL,GREEN,display);

	release_display(display);
}

