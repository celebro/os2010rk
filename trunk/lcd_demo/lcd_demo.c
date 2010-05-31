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

int colors[10] = {
	WHITE,
	RED,
	GREEN,
	BLUE,
	CYAN,
	MAGENTA,
	YELLOW,
	BROWN,
	ORANGE,
	PINK
};


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
	menu_item[0][1] = "Text & bmp";
	menu_item[0][2] = "Ioctl";
	menu_item[0][3] = "Pong";

	menu_item[1][0] = "Pixel";
	menu_item[1][1] = "Line";
	menu_item[1][2] = "Rect";
	menu_item[1][3] = "";

	menu_item[2][0] = "Strings";
	menu_item[2][1] = "Bitmap";
	menu_item[2][2] = "Combined";
	menu_item[2][3] = "";

	menu_item[3][0] = "Backlight";
	menu_item[3][1] = "Sleep";
	menu_item[3][2] = "Off";
	menu_item[3][3] = "";
}

void clear_screen() {
	lcd_set_rect(0, 0, 129, 129, FILL, BLACK, display);
}

void demo_pixel() {
	printf("Demo pixel\n");
	char key;

	clear_screen();
	while (1) {
		lcd_set_pixel(rand()%130, rand()%130, colors[rand()%10], display);
		lcd_set_pixel(rand()%130, rand()%130, colors[rand()%10], display);
		lcd_set_pixel(rand()%130, rand()%130, colors[rand()%10], display);
		lcd_set_pixel(rand()%130, rand()%130, colors[rand()%10], display);
		key = get_key();
		if (key == QUIT)
			break;
	}
}

void demo_line() {
	printf("Demo line\n");

	int i;
	char key;

	int coord[15][5] =
	{
		{ 20, 110, 110, 110, GREEN},
		{ 20, 110, 40, 90, GREEN},
		{ 110, 110, 90, 90, GREEN},
		{ 40, 90, 90, 90, GREEN},
		{ 80, 90, 80, 20, BLUE},
		{ 80, 20, 50, 20, BLUE},
		{ 50, 20, 50, 43, BLUE},
		{ 47, 44, 53, 44, YELLOW},
		{ 47, 45, 53, 45, YELLOW},
		{ 47, 46, 53, 46, YELLOW},
		{ 47, 47, 53, 47, YELLOW},
		{ 50, 48, 50, 66, RED},
		{ 40, 56, 60, 56, RED},
		{ 50, 66, 40, 75, RED},
		{ 50, 66, 60, 75, RED}
	};

	clear_screen();
	for (i = 0; i < 15; i++) {
		lcd_set_line(coord[i][1], coord[i][0], coord[i][3], coord[i][2], coord[i][4], display);
		key = get_key();
		if (key == QUIT)
			return;
	}

}

void demo_rect() {
	printf("Demo rect\n");
	int i;
	char key;

	int coord[11][6] =
	{
		{ 20, 20, 60, 60, WHITE, NOFILL},
		{ 20, 70, 60, 110, RED, NOFILL},
		{ 70, 20, 110, 60, BLUE, NOFILL},
		{ 70, 70, 110, 110, ORANGE, NOFILL},

		{ 20, 20, 60, 60, WHITE, FILL},
		{ 20, 70, 60, 110, RED, FILL},
		{ 70, 20, 110, 60, BLUE, FILL},
		{ 70, 70, 110, 110, ORANGE, FILL},

		{ 40, 40, 90, 90, GREEN, FILL},
		{ 50, 50, 80, 80, BLACK, FILL},
		{ 60, 60, 70, 70, PINK, FILL}
	};

	clear_screen();
	for (i = 0; i < 11; i++) {
		lcd_set_rect(coord[i][0], coord[i][1], coord[i][2], coord[i][3], coord[i][5], coord[i][4], display);
		key = get_key();
		if (key == QUIT)
			return;
	}

}

void demo_strings() {
	printf("Demo strings\n");

	clear_screen();
	lcd_put_str("Small font", 20, 20, SMALL, YELLOW, display);
	get_key();
	lcd_put_str("Medium font", 45, 25, MEDIUM, CYAN, display);
	get_key();
	lcd_put_str("Large font", 70, 30, LARGE, MAGENTA, display);
	get_key();
	lcd_put_str("!@#$%^&*()_+", 90, 20, LARGE, GREEN, display);
	get_key();

}

void demo_bitmap() {
	printf("Demo bitmap.\n");

	lcd_load_bmp("pic1.bmp", display);
	lcd_set_bmp(0,0,display);

	if (get_key() == QUIT)
		return;

	lcd_load_bmp("pic2.bmp", display);
	lcd_set_bmp(0,0,display);

	if (get_key() == QUIT)
		return;

	lcd_load_bmp("pic4.bmp", display);
	clear_screen();
	lcd_set_bmp(0,0,display);

	if (get_key() == QUIT)
		return;

	lcd_set_bmp(40,0,display);

	if (get_key() == QUIT)
		return;

	lcd_set_bmp(80,0,display);

	if (get_key() == QUIT)
		return;

	lcd_load_bmp("pic3.bmp", display);
	lcd_set_bmp(10,10,display);

	if (get_key() == QUIT)
		return;

	lcd_set_bmp(70,70,display);

	if (get_key() == QUIT)
		return;
}


void demo_combined() {
	printf("Demo combined\n");

	clear_screen();
	lcd_load_bmp("pic1.bmp", display);
	lcd_set_bmp(0,0,display);

	if (get_key() == QUIT)
		return;

	lcd_put_str("ARM/Linux", 30, 30, LARGE, RED, display);
	lcd_put_str("LCD gonilnik", 50, 15, LARGE, RED, display);
	//lcd_set_rect(0, 0, 129, 129, FILL, BLACK, display);
	lcd_put_str("DEMO", 70, 50, LARGE, RED, display);

	if (get_key() == QUIT)
		return;
}

void demo_backlight() {

}

void demo_sleep() {

}

void demo_off() {

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

    int maxd = 4;

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

		if (x+size >= padx) {
			if ((y+size < pady) || (y > pady+paddy)) {
				//die
				alive = 0;
				active = 0;
			}
			else  {
				if (dy <= 0) {
					dy = -(rand() % 5);
				}
				else {
					dy = rand() % 5;
				}
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

		x+=dx;
		y+=dy;

		/* Draw new */
		lcd_set_rect(x, y, x+size, y+size, FILL, GREEN, display);

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

    /* Player pressed quit */
    if (alive) {
    	return;
    }

    lcd_put_str("YOU LOST", 10, 10, LARGE, YELLOW, display);

    active = 1;
    while (active) {
		/* Clear previous */
		lcd_set_rect(x, y, x+size, y+size, FILL, BLACK, display);

		if (x+size >= 129+size) {
			active = 0;
		}

		x+=dx;
		y+=dy;

		/* Draw new */
		lcd_set_rect(x, y, x+size, y+size, FILL, RED, display);
	}

    get_key();

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
		case RIGHT:
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
						menu = 2;
						position = menu_position[menu];
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
				/* Text and bitmap */
				case 2:
					if (position == 0) {
						demo_strings();
						intro_animation();
					}
					else if (position == 1) {
						demo_bitmap();
						intro_animation();
					}
					else if (position == 2) {
						demo_combined();
						intro_animation();
					}
					break;
				/* Bitmap*/
				case 3:
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
		case LEFT:
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
