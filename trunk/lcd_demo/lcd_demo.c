#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <termios.h>
#include <fcntl.h>

#include "lcd_demo.h"
#include "display_lib.h"

/* Key processing */
#define ENTER	0
#define UP		1
#define DOWN	2
#define LEFT	3
#define RIGHT	4
#define QUIT	5

#define NOKEY	100


/* For storing terminal setting */
static struct termios oldt;

struct display* display;

int menu_position[4] = {0,0,0,0};
int menu_size[4];
int menu_x[4];
int menu_dy[4];
char *menu_item[4][4];
int menu_dx = 18;
int menu_spacing = 15;
int menu_color = ORANGE;
int str_vspace = 2;
int str_hspace = 4;
int str_color = RED;
int str_size = LARGE;

int menu = 0;

int running = 1;

void restore_terminal_settings(void) {
	tcsetattr(0, TCSANOW, &oldt); /* Apply saved settings */
}

void disable_waiting_for_enter(void) {
	struct termios newt;

	/* Make terminal read 1 char at a time */
	tcgetattr(0, &oldt); /* Save terminal settings */
	newt = oldt; /* Init new settings */
	newt.c_lflag &= ~(ICANON | ECHO); /* Change settings */
	tcsetattr(0, TCSANOW, &newt); /* Apply settings */
	atexit(restore_terminal_settings); /* Make sure settings will be restored when program ends  */
}

char get_key() {
	int ch;

	ch = getchar();
	if (ch == 27) {
		ch = getchar();
		if (ch == 91) {
			ch = getchar();
			if (ch == 65)
				return UP;
			if (ch == 66)
				return DOWN;
			if (ch == 67)
				return RIGHT;
			if (ch == 68)
				return LEFT;
		}
	}
	else if (ch == 10)
		return ENTER;
	else if (ch == 'Q' || ch == 'q') {
		return QUIT;
	}

	return NOKEY;
}

void build_vars() {
	int i;

	menu_size[0] = 3;
	menu_size[1] = 2;
	menu_size[2] = 2;
	menu_size[3] = 2;

	menu_x[0] = 7;
	menu_dy[0] = 110;
	for (i = 1; i<4; i++) {
		menu_x[i] = menu_x[i-1] + menu_dx + menu_spacing;
		menu_dy[i] = menu_dy[i-1] - 15;
	}

	menu_item[0][0] = "Basic shapes";
	menu_item[0][1] = "Strings";
	menu_item[0][2] = "Bitmap";
	menu_item[0][3] = "Pong";

	menu_item[1][0] = "Pixel";
	menu_item[1][1] = "Line";
	menu_item[1][2] = "Rect";
	menu_item[1][3] = "";

	menu_item[2][0] = "Large";
	menu_item[2][1] = "Medium";
	menu_item[2][2] = "Small";
	menu_item[2][3] = "";

	menu_item[3][0] = "Full screen";
	menu_item[3][1] = "Small";
	menu_item[3][2] = "Combined";
	menu_item[3][3] = "";
}

void clear_screen() {
	lcd_set_rect(0, 0, 129, 129, FILL, BLACK, display);
}

void demo_pixel() {
	printf("Demo pixel\n");
}

void demo_line() {
	printf("Demo line\n");
}

void demo_rect() {
	printf("Demo rect\n");
}

void demo_string() {
	printf("Demo string\n");
}

void demo_bmpfull() {
	printf("Demo bmpfull\n");
}

void demo_bmpsmall() {
	printf("Demo bmpsmall\n");
}

void demo_integration() {
	printf("Demo intergration\n");
}

void demo_pong() {
	printf("Demo pong\n");

	int term_setting;

	int size = 20;
    int x=0, y=0;
    int dx=3, dy=1;

    int pady = 30;
    int padx = 100;
    int paddy = 30;
    int paddx = 10;

    char c;
    int active = 1;
    int alive = 1;

	term_setting = fcntl(0, F_GETFL, 0);
	fcntl (0, F_SETFL, (term_setting | O_NDELAY));

	clear_screen();
	lcd_set_rect(padx, pady, padx+paddx, pady+paddy, FILL, YELLOW, display);

    while (active) {
		/* Clear previous */
		lcd_set_rect(x, y, x+size, y+size, FILL, BLACK, display);

		//if (alive) {

			if (x+size >= padx) {
				if ((y+size < pady) || (y > pady+paddy)) {
					//die
					//usleep(1000000);
					alive = 0;
				}
				else  {
					dx = -dx;
					x = padx - size - 1;
					lcd_set_rect(padx, pady, padx+paddx, pady+paddy, FILL, YELLOW, display);
				}
			}
			if (y+size >= 129) {
				dy = -dy;
				y = 129 - size;
			}
			if (x <= 0) {
				dx = -dx;
				x = 0;
			}
			if (y <= 0) {
				dy = -dy;
				y = 0;
			}
		//}

		x+=dx;
		y+=dy;

		/* Draw new */
		lcd_set_rect(x, y, x+size, y+size, FILL, RED, display);

		c = get_key();
		switch (c) {
		case LEFT:
			lcd_set_rect(padx, pady, padx+paddx, pady+paddy, FILL, BLACK, display);
			pady -= 10;
			lcd_set_rect(padx, pady, padx+paddx, pady+paddy, FILL, YELLOW, display);
			break;
		case RIGHT:
			lcd_set_rect(padx, pady, padx+paddx, pady+paddy, FILL, BLACK, display);
			pady += 10;
			lcd_set_rect(padx, pady, padx+paddx, pady+paddy, FILL, YELLOW, display);
			break;
		case QUIT:
		case ENTER:
			active = 0;
			break;
		}

	}

	fcntl(0, F_SETFL, term_setting);
}

/* Intro animation */
void intro_animation() {
	int i;
	int m;

	int step = 12;

	/* TODO a nice bmp */

	clear_screen();
	for (m = 0; m < 4; m++) {
		if (m > menu_size[menu]) {
			break;
		}
		for (i = -menu_dy[m]; i <= 0; i = i+step) {
			lcd_set_rect(menu_x[m], i, menu_x[m] + menu_dx, i+menu_dy[m], FILL, menu_color, display);
			lcd_put_str(menu_item[menu][m], menu_x[m]+str_vspace, i+str_hspace, str_size, str_color, display);
		}
		lcd_set_rect(menu_x[m], 0, menu_x[m] + menu_dx, menu_dy[m], FILL, menu_color, display);
		lcd_put_str(menu_item[menu][m], menu_x[m]+str_vspace, str_hspace, str_size, str_color, display);
	}

	switch_tab(menu_position[menu], menu_position[menu]);
}

void switch_tab(int previous, int current){
	lcd_set_rect(menu_x[previous], 0, menu_x[previous] + menu_dx, menu_dy[previous], FILL, menu_color, display);
	lcd_put_str(menu_item[menu][previous], menu_x[previous]+str_vspace, str_hspace, str_size, str_color, display);

	lcd_set_rect(menu_x[current], 0, menu_x[current] + menu_dx, menu_dy[current], FILL, str_color, display);
	lcd_put_str(menu_item[menu][current], menu_x[current]+str_vspace, str_hspace, str_size, menu_color, display);

}

int menu_selection(int position){
	char key = get_key();

	if((key == DOWN) && position==menu_size[menu])	//end is reached, return current position
		return position;
	if((key == UP)&& position == 0)
		return position;

	switch (key){
		case ENTER:
			menu_position[menu] = position;
			switch (menu) {
				/* Main menu*/
				case 0:
					printf("Menu 0 selection %i\n", position);
					if (position == 0) {
						menu = 1;
						position = menu_position[menu];
						intro_animation();
					}
					else if (position == 1) {
						demo_string();
						intro_animation();
					}
					else if (position == 2) {
						menu = 3;
						position = menu_position[menu];
						intro_animation();
					}
					else if (position == 3) {
						demo_pong();
						intro_animation();
					}
					break;
				/* Basic shapes */
				case 1:
					printf("Strings selection %i\n", position);
					if (position == 0) {
						demo_pixel();
						intro_animation();
					}
					else if (position == 1) {
						demo_line();
						intro_animation();
					}
					else if (position == 2) {
						demo_rect();
						intro_animation();
					}
					break;
				/* Strings */
				case 2:
					break;
				/* Bitmap*/
				case 3:
					if (position == 0) {
						demo_bmpfull();
						intro_animation();
					}
					else if (position == 1) {
						demo_bmpsmall();
						intro_animation();
					}
					else if (position == 2) {
						demo_integration();
						intro_animation();
					}
					break;
			}
			break;
		case DOWN:
			switch_tab(position,position+1);
			position++;
			break;
		case UP:
			switch_tab(position,position-1);
			position--;
			break;
		case QUIT:
			if (menu == 0)
				running = 0;
			else {
				menu_position[menu] = position;
				menu = 0;
				position = menu_position[menu];
				intro_animation();
			}
			break;
		default:
			break;
	}

	return position;
}

int main(){

	printf("Lcd driver and lib demo\n");

	display = get_display();
	if (display == NULL) {
		printf("Failed to get display\n");
		return -1;
	}

	disable_waiting_for_enter();

	build_vars();
	intro_animation();

	char key = 0;
	int position = 0;

	while(running){
		position = menu_selection(position);
	}
	clear_screen();

	restore_terminal_settings();
	release_display(display);
}
