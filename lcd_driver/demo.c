#include "lcd_driver.h"
#include "lcd_protocol.h"
#include "font.h"

#include "at91sam9260.h"
#include "epson.h"



void animated_ball(void){
	int x=0,y=0;
	int dx=1,dy=2;

	while(1){
		if(bmp_x+x>130||x<0)
			dx=-dx;
		if(bmp_y+y>130||y<0)
			dy=-dy;
		x+=dx;
		y+=dy;
		lcd_set_bmp(x,y);
		lcd_delay(20);
	}
}

void all_on_screen(void){
	lcd_set_pixel(rand(132), rand(132), GREEN);
	lcd_set_rect(rand(132), rand(132), rand(30), rand(30), FILL, YELLOW);
	lcd_set_rect(rand(132), rand(132), rand(30), rand(30), 0, BROWN);
	lcd_set_line (rand(132), rand(132),rand(132), rand(132), RED);
	lcd_put_str("FTW", 50, 50, MEDIUM, BLUE);
}


void print_string(char *pString){
	lcd_put_str(pString, 50, 50, MEDIUM, BLUE);
}

