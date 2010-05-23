#include "lcd_driver.h"
#include "lcd_ioctl.h"
#include "lcd_protocol.h"
#include "font.h"

#include "at91sam9260.h"
#include "epson.h"

MODULE_LICENSE("Dual BSD/GPL");
MODULE_DESCRIPTION("Driver for Nokia 6610 LCD on ARM board");

/* Pointers to controllers on ARM board*/
volatile AT91PS_PIO		pPIOB;
volatile AT91PS_SPI		pSPI;
volatile AT91PS_PMC		pPMC;

/* For the device, we will create */
static struct char_device *lcd_dev;
//static unsigned char *buf;
static char str[LCD_MAX_STR_LEN];

/* File operations for that device */
static struct file_operations lcd_fops = {
  .read 	= lcd_read,
  .write 	= lcd_write,
  .open 	= lcd_open,
  .release 	= lcd_release,
  .ioctl	= lcd_ioctl
};

/* Testing parameters */
unsigned int vol = 50;
unsigned int res = 3;
unsigned int col = YELLOW;
unsigned int temp = 0;
module_param(vol, uint, S_IRUGO);
module_param(res, uint, S_IRUGO);
module_param(col, uint, S_IRUGO);
module_param(temp, uint, S_IRUGO);

unsigned int image[132][132];
unsigned int *bmp;
int bmp_x;
int bmp_y;


////TODO
//#include <mach/at91_spi.h>
//static void __iomem *spi_base;
//#define at91_spi_read(reg)		__raw_readl(spi_base + (reg))
//#define at91_spi_write(reg, val)	__raw_writel((val), spi_base + (reg))

/* when inserting module into kernel */
static int __init lcd_driver_init (void) {
	printk(KERN_ALERT PREFIX "Module init.\n");

	/* Physical to virtual translation of controller addresses */
	//pPIOB = (AT91PS_PIO) AT91_IO_P2V((unsigned long) AT91C_BASE_PIOB)
	//pSPI = (AT91PS_SPI) AT91_IO_P2V((unsigned long) AT91C_BASE_SPI1);
	//pPMC = (AT91PS_PMC) AT91_IO_P2V((unsigned long) AT91C_BASE_PMC);
	pPIOB 	= ioremap((unsigned long)AT91C_BASE_PIOB, sizeof(AT91S_PIO));
	pSPI 	= ioremap((unsigned long)AT91C_BASE_SPI1, sizeof(AT91S_SPI));
	pPMC 	= ioremap((unsigned long)AT91C_BASE_PMC, sizeof(AT91S_PMC));

	//	at91_set_gpio_output( AT91_PIN_PB0, 1);
	//	a = at91_spi_read(AT91_SPI_IMR);
	//	at91_spi_write(AT91_SPI_IER, 1);
	//	at91_set_gpio_output( AT91_PIN_PB0, 0);

	printk(KERN_ALERT PREFIX "Col = %u\n", col);
	printk(KERN_ALERT PREFIX "Res = %u\n", res);
	printk(KERN_ALERT PREFIX "Vol = %u\n", vol);

	init_controllers();
	init_lcd();
	lcd_dev = char_device_create("lcd", &lcd_fops);

	if (lcd_dev == NULL) {
		printk(KERN_ALERT PREFIX "Failed to initialize device\n");
		lcd_driver_exit();
		return -1;
	}

	bmp = NULL;
//	buf = kmalloc(sizeof(unsigned char)*BUF_SIZE, GFP_KERNEL);
//	if (buf == NULL) {
//		printk(KERN_ALERT PREFIX "Failed allocate memmory for buffer.\n");
//		return -1;
//	}

	printk(KERN_ALERT PREFIX "Module loaded v10.\n");

	return 0;
}

/* when removing module from kernel */
static void __exit lcd_driver_exit (void) {
	char_device_release(lcd_dev);
	sleep_lcd();
	release_controllers();

	iounmap(pPIOB);
	iounmap(pSPI);
	iounmap(pPMC);

	kfree(bmp);

	printk(KERN_ALERT PREFIX "Module unloaded.\n");
}

module_init(lcd_driver_init)
module_exit(lcd_driver_exit)

static struct char_device* char_device_create(char *name, struct file_operations *fops) {
	struct char_device *device = kmalloc(sizeof(struct char_device), GFP_KERNEL);

	int err;
	int devno;
	int major;
	dev_t dev = 0;

	if (device == NULL) {
		printk(KERN_ALERT PREFIX "Failed allocate memmory for device.\n");
		return NULL;
	}

	device->name = name;

	err = alloc_chrdev_region(&dev, 0, 1, "lcd");
	if (err < 0) {
		printk(KERN_ALERT PREFIX "Failed to get major with error %d\n", err);
		return NULL;
	}
	major = MAJOR(dev);
	device->major = major;

	devno = MKDEV(major, 0);
	cdev_init(&device->cdev, fops);
	device->cdev.owner = THIS_MODULE;
	device->cdev.ops = fops;
	err = cdev_add(&device->cdev, devno, 1);

	if (err) {
		printk(KERN_ALERT PREFIX "Failed to register device %s with error %d\n", name, err);\
		unregister_chrdev_region(dev,1);
		kfree(device);
		return NULL;
	}

	printk(KERN_ALERT PREFIX "Register device %s with major %d\n", name, major);
	printk(KERN_ALERT PREFIX "Use: mknod /dev/%s c %d 0\n", name, major);

	return device;
}

static void char_device_release(struct char_device *device) {
	if (device != NULL) {
		dev_t dev = MKDEV(device->major,0);
		cdev_del(&device->cdev);
		unregister_chrdev_region(dev,1);

		printk(KERN_ALERT PREFIX "Unregister device %s\n", device->name);
		kfree(device);
		device = NULL;
	}
}

static int lcd_open(struct inode *inode, struct file *file) {
	printk(KERN_ALERT PREFIX "Open.\n");

	//TODO lock device using spinlock

	try_module_get(THIS_MODULE);
	return 0;
}

static int lcd_release(struct inode *inode, struct file *file) {
	printk(KERN_ALERT PREFIX "Release.\n");

	module_put(THIS_MODULE);
	return 0;
}

static ssize_t lcd_read(struct file *file, char *buffer, size_t length, loff_t *offset) {
	printk(KERN_ALERT PREFIX "Read.\n");
	return 0;
}

static ssize_t lcd_write(struct file *file, const char __user *buffer, size_t length, loff_t *offset) {
	int type;
	struct lcd_func_params param;
	int str_len;

	printk(KERN_ALERT PREFIX "Write.\n");

	if (length == 0)
		return 0;

	get_user(type, buffer);
	printk(KERN_ALERT PREFIX "Type - %d.\n", type);

	switch (type) {

		case LCD_SET_RECT:
			printk(KERN_ALERT PREFIX "Set rect.\n");
			if (length-1 >= sizeof(struct lcd_func_params)) {
				copy_from_user(&param, buffer+1, sizeof(struct lcd_func_params));
				lcd_set_rect(param.x1+2, param.y1, param.x2, param.y2, param.fill, param.color);
			}
			break;

		case LCD_SET_LINE:
			printk(KERN_ALERT PREFIX "Set line.\n");
			if (length-1 >= sizeof(struct lcd_func_params)) {
				copy_from_user(&param, buffer+1, sizeof(struct lcd_func_params));
				lcd_set_line(param.x1+2, param.y1, param.x2, param.y2, param.color);
			}
			break;

		case LCD_PUT_STR:
			printk(KERN_ALERT PREFIX "Put string.\n");
			if (length-1 >= sizeof(struct lcd_func_params)) {
				copy_from_user(&param, buffer+1, sizeof(struct lcd_func_params));
				str_len = length - 1 - sizeof(struct lcd_func_params);
				if (str_len > param.t_len)
					str_len = param.t_len;
				copy_from_user(&str, buffer+1+sizeof(struct lcd_func_params), param.t_len);
				str[LCD_MAX_STR_LEN-1] = '\n';
				lcd_put_str(str, param.x1+2, param.y1, param.t_size, param.color);
			}
			break;

		case LCD_LOAD_BMP:
			printk(KERN_ALERT PREFIX "Put bmp.\n");
			if (length-1 >= sizeof(struct lcd_func_params)) {
				copy_from_user(&param, buffer+1, sizeof(struct lcd_func_params));

				if (bmp != NULL)
					kfree(bmp);
				bmp = NULL;
				bmp = kmalloc(sizeof(unsigned int)*param.x1*param.y1, GFP_KERNEL);
				if (bmp == NULL) {
					printk(KERN_ALERT PREFIX "Failed to allocate bmp memmory.\n");
					break;
				}

				copy_from_user(bmp, param.bmp, sizeof(unsigned int)*param.x1*param.y1);
				bmp_x = param.x1;
				bmp_y = param.y1;
			}
			break;

		case LCD_SET_BMP:
			printk(KERN_ALERT PREFIX "Set bmp.\n");
			if (length-1 >= sizeof(struct lcd_func_params)) {
				if (length-1 >= sizeof(struct lcd_func_params)) {
					copy_from_user(&param, buffer+1, sizeof(struct lcd_func_params));
					lcd_set_bmp(param.x1+2, param.y1);
				}
			}
			break;

		case '0': lcd_set_rect(50, 50, 100, 100, FILL, WHITE); break;
		case '1': lcd_set_rect(50, 50, 100, 100, FILL, BLACK); break;
		case '2': lcd_set_rect(50, 50, 100, 100, FILL, RED); break;
		case '3': lcd_set_rect(50, 50, 100, 100, FILL, GREEN); break;
		case '4': lcd_set_rect(50, 50, 100, 100, FILL, BLUE); break;
		case '5': lcd_set_rect(50, 50, 100, 100, FILL, CYAN); break;
		case '6': lcd_set_rect(50, 50, 100, 100, FILL, BROWN); break;
		case '7': lcd_set_rect(70, 70, 100, 100, FILL, ORANGE); break;
		case '8': //lcd_set_rect(50, 50, 100, 100, FILL, PINK); break;
			lcd_redraw();
			lcd_set_rect(30, 0, 65, 100, FILL, YELLOW);
			break;
		case '9':
			lcd_set_rect(2, 0, 100, 100, FILL, PINK);
			lcd_set_rect(10, 0, 20, 23, FILL, YELLOW);
			lcd_put_char('E', 10, 10, 2, RED);
			lcd_put_char('B', 10, 20, 2, RED);
			lcd_put_char('M', 10, 30, 2, RED);
			//lcd_put_str("Test string\n", 30, 10, SMALL, YELLOW, BLACK);
			//lcd_put_str("Test string\n", 45, 10, MEDIUM, YELLOW, BLACK);
			//lcd_put_str("Test string\n", 60, 10, LARGE, YELLOW, BLACK);
			break;

	}

	return length;
}

static int lcd_ioctl(struct inode *inode,
					 struct file *file,
					 unsigned int ioctl_num,
					 unsigned long ioctl_param)
{

	printk(KERN_ALERT PREFIX "Ioctl (%d, %lu).\n", ioctl_num, ioctl_param);

	switch(ioctl_num) {
		/* Turn LCD on or off */
		case LCD_IOCTL_ONOFF:
			if (ioctl_param == LCD_IOCTL_ON) {
				/* Enable power */
				pPIOB->PIO_SODR = LCD_PWR | LCD_RESET;
				init_lcd();
				/* Enable backlight (looks better) */
				pPIOB->PIO_SODR	= LCD_BL;
				printk(KERN_ALERT PREFIX IOCTL_PREFIX "lcd on.\n");
			}
			else if (ioctl_param == LCD_IOCTL_OFF) {
				/* Disable backlight */
				pPIOB->PIO_CODR	= LCD_BL;
				sleep_lcd();
				/* Disable power */
				pPIOB->PIO_CODR	= LCD_BL | LCD_PWR | LCD_RESET;
				printk(KERN_ALERT PREFIX IOCTL_PREFIX "lcd off.\n");
			}
			break;

		/* Put LCD to sleep or wake up */
		case LCD_IOCTL_SLEEP:
			if (ioctl_param == LCD_IOCTL_ON) {
				pPIOB->PIO_CODR	= LCD_BL;
				sleep_lcd();
				printk(KERN_ALERT PREFIX IOCTL_PREFIX "sleep on.\n");
			}
			else if (ioctl_param == LCD_IOCTL_OFF) {
				wake_lcd();
				pPIOB->PIO_SODR	= LCD_BL;
				printk(KERN_ALERT PREFIX IOCTL_PREFIX "sleep off.\n");
			}
			break;

		/* LCD backlight on/off */
		case LCD_IOCTL_BACKLIGHT:
			if (ioctl_param == LCD_IOCTL_ON) {
				pPIOB->PIO_SODR	= LCD_BL;
				printk(KERN_ALERT PREFIX IOCTL_PREFIX "backlight on.\n");
			}
			else if (ioctl_param == LCD_IOCTL_OFF) {
				pPIOB->PIO_CODR	= LCD_BL;
				printk(KERN_ALERT PREFIX IOCTL_PREFIX "backlight off.\n");
			}
			break;

	}
	return 0;
}

/* Initialize PIO and SPI controllers */
static void init_controllers(void) {
	// Lcd backlight pin as output, set to high
	pPIOB->PIO_PER		= LCD_BL;
	pPIOB->PIO_OER		= LCD_BL;
	pPIOB->PIO_SODR		= LCD_BL;
	// Lcd reset pin, as output, set to high (reset not active)
	pPIOB->PIO_PER		= LCD_RESET;
	pPIOB->PIO_OER		= LCD_RESET;                // Configure PA2 as output
	pPIOB->PIO_SODR		= LCD_RESET;                // Set PA2 to HIGH   (assert LCD Reset low then high to reset the controller)
	// SPI chip select initialy high (not selected)
	pPIOB->PIO_PER		= LCD_CS;
	pPIOB->PIO_OER		= LCD_CS;               // Configure PA12 as output
	pPIOB->PIO_SODR		= LCD_CS;               // Set PA12 to HIGH (assert CS_LCD low to enable transmission)
	// Lcd controller power as output, set to high
	pPIOB->PIO_PER		= LCD_PWR;
	pPIOB->PIO_OER		= LCD_PWR;
	pPIOB->PIO_SODR		= LCD_PWR;

	// Disable the following pins from PIO control (will be used instead by the SPI0 peripheral)
	pPIOB->PIO_PDR = LCD_CS | LCD_MISO | LCD_MOSI | LCD_CLOCK; // Peripheral A Disable Register (Disable PIO control of these 4 bits)
	pPIOB->PIO_ASR = LCD_CS | LCD_MISO | LCD_MOSI | LCD_CLOCK; // Peripheral A Select Register (all 4 bits are in PIOA)

	//enable the SPI0 Peripheral clock
	pPMC->PMC_PCER = 1 << AT91C_ID_SPI1;

	// SPI Control Register SPI_CR
	pSPI->SPI_CR = AT91C_SPI_SWRST | AT91C_SPI_SPIEN;           //Software reset, SPI Enable  (0x81)
	pSPI->SPI_CR = AT91C_SPI_SPIEN;                             //SPI Enable (0x01)

	// SPI Mode Register SPI_MR = 0xE0011
	pSPI->SPI_MR =
		 (AT91C_SPI_DLYBCS & (0 << 24)) |            // Delay between chip selects (take default: 6 MCK periods)
		 (AT91C_SPI_PCS & (0xE << 16)) |             // Peripheral Chip Select (selects SPI_NPCS0 or PA12)
		 (AT91C_SPI_LLB & (0 << 7))      |           // Local Loopback Enabled (disabled)
		 (AT91C_SPI_MODFDIS & (1 << 4))  |           // Mode Fault Detection (disabled)
		 (AT91C_SPI_PCSDEC & (0 << 2))   |           // Chip Select Decode (chip selects connected directly to peripheral)
		 (AT91C_SPI_PS & (0 << 1))       |           // Peripheral Select (fixed)
		 (AT91C_SPI_MSTR & (1 << 0));                // Master/Slave Mode (Master)

	// SPI Chip Select Register SPI_CSR[0] = 0x01010311
	pSPI->SPI_CSR[0] =
	(AT91C_SPI_DLYBCT & (0x01 << 24))         |      // Delay between Consecutive Transfers (32 MCK periods)
	(AT91C_SPI_DLYBS & (0x01 << 16))          |      // Delay Before SPCK (1 MCK period)
	(AT91C_SPI_SCBR & (0x10 << 8))            |      // Serial Clock Baud Rate (baudrate = MCK/8 = 48054841/8 = 6006855 baud
	(AT91C_SPI_BITS & (AT91C_SPI_BITS_9))     |      // Bits per Transfer (9 bits)
	(AT91C_SPI_CSAAT & (0x0 << 3))            |      // Chip Select Active After Transfer (is active after xfer)/
	(AT91C_SPI_NCPHA & (0x0 << 1))            |      // Clock Phase (data captured on falling edge)
	(AT91C_SPI_CPOL & (0x01 << 0));                  // Clock Polarity (inactive state is logic one)
}

static void release_controllers(void) {
	pPIOB->PIO_CODR		= LCD_BL | LCD_PWR | LCD_RESET;

	//pPIOB->PIO_PER = LCD_CS | LCD_MISO | LCD_MOSI | LCD_CLOCK;
	//pPIOB->PIO_ODR = LCD_CS | LCD_MISO | LCD_MOSI | LCD_CLOCK;

}

static void write_spi_command(volatile unsigned int command) {
	// wait for the previous transfer to complete
	while((pSPI->SPI_SR & AT91C_SPI_TXEMPTY) == 0);
	// clear bit 8 - indicates a "command"
	command = (command & ~0x0100);
	// send the command
	pSPI->SPI_TDR = command;
}

static void write_spi_data(volatile unsigned int data) {
	// wait for the transfer to complete
	while((pSPI->SPI_SR & AT91C_SPI_TXEMPTY) == 0);
	// set bit 8, indicates "data"
	data = (data | 0x0100);
	// send the data
	pSPI->SPI_TDR = data;
}

static void lcd_delay(int n)
{
	int time_to_stop = get_jiffies_64() + n;
	int time_now;

	do
	{
		time_now = get_jiffies_64();
		cpu_relax();
	} while(time_now < time_to_stop);
}

//static void init_lcd2(void) {
//	// Hardware reset
//	pPIOB->PIO_CODR = LCD_RESET;
//	lcd_delay(10);
//	pPIOB->PIO_SODR = LCD_RESET;
//	lcd_delay(10);
//
//	// Display control
//	write_spi_command(DISCTL);
//	write_spi_data(0x00); // P1: 0x00 = 2 divisions, switching period=8 (default)
//	write_spi_data(0x20); // P2: 0x20 = nlines/4 - 1 = 132/4 - 1 = 32)
//	write_spi_data(0x00); // P3: 0x00 = no inversely highlighted lines
//
//	// COM scan
//	write_spi_command(COMSCN);
//	write_spi_data(1);           // P1: 0x01 = Scan 1->80, 160<-81
//
//	// Internal oscilator ON
//	write_spi_command(OSCON);
//
//	// Sleep out
//	write_spi_command(SLPOUT);
//	lcd_delay(10);
//
//	// Power control
//	write_spi_command(PWRCTR);
//	write_spi_data(0x0f);    // reference voltage regulator on, circuit voltage follower on, BOOST ON
//	lcd_delay(10);
//
//	// Inverse display
//	write_spi_command(DISNOR);
//
//	// Data control
//	write_spi_command(DATCTL);
//	write_spi_data(0x00); // P1: 0x01 = page address inverted, column address normal, address scan in column direction
//	write_spi_data(0x00); // P2: 0x00 = RGB sequence (default value)
//	write_spi_data(0x02); // P3: 0x02 = Grayscale -> 16 (selects 12-bit color, type A)
//
//	// Voltage control (contrast setting)
//	write_spi_command(VOLCTR);
//	write_spi_data(vol); // P1 = 32 volume value (experiment with this value to get the best contrast)
//	write_spi_data(res);    // P2 = 3    resistance ratio (only value that works)
//
//	// allow power supply to stabilize
//	lcd_delay(100);
//
//	// turn on the display
//	write_spi_command(DISON);
//}

static void init_lcd(void) {
	// Hardware reset
	pPIOB->PIO_CODR = LCD_RESET;
	lcd_delay(10);
	pPIOB->PIO_SODR = LCD_RESET;
	lcd_delay(10);

	// Display control
	write_spi_command(DISCTL);
	write_spi_data(0x00); // P1: 0x00 = 2 divisions, switching period=8 (default)
	write_spi_data(0x20); // P2: 0x20 = nlines/4 - 1 = 132/4 - 1 = 32)
	write_spi_data(0x00); // P3: 0x00 = no inversely highlighted lines

	// COM scan
	write_spi_command(COMSCN);
	write_spi_data(1);           // P1: 0x01 = Scan 1->80, 160<-81

	// Internal oscilator ON
	write_spi_command(OSCON);

	// Sleep out
	write_spi_command(SLPOUT);
	lcd_delay(10);

	// Voltage control (contrast setting)
	write_spi_command(VOLCTR);
	write_spi_data(vol); // P1 = 32 volume value (experiment with this value to get the best contrast)
	write_spi_data(res);    // P2 = 3    resistance ratio (only value that works)

	//TODO temperature
	write_spi_command(TMPGRD);
	write_spi_data(temp);

	// Power control
	write_spi_command(PWRCTR);
	write_spi_data(0x0f);    // reference voltage regulator on, circuit voltage follower on, BOOST ON
	lcd_delay(10);

	// Inverse display
	write_spi_command(DISINV);

	// Data control
	write_spi_command(DATCTL);
	write_spi_data(0x00); // P1: 0x01 = page address inverted, column address normal, address scan in column direction
	write_spi_data(0x00); // P2: 0x00 = RGB sequence (default value)
	write_spi_data(0x02); // P3: 0x02 = Grayscale -> 16 (selects 12-bit color, type A)

	// allow power supply to stabilize
	lcd_delay(100);

	// Turn on the display
	write_spi_command(DISON);

	lcd_delay(20);

	/* Single color backfill */
	lcd_set_rect(2,0,131,129,FILL,BLACK);
}

static void sleep_lcd() {
	// Turn off the display
	write_spi_command(DISOFF);

	// Power circuit off
	write_spi_command(PWRCTR);
	write_spi_data(0x00);    // reference voltage regulator on, circuit voltage follower on, BOOST ON
	lcd_delay(10);

	// Internal oscilator OFF
	write_spi_command(OSCOFF);

	// Sleep
	write_spi_command(SLPOUT);

	// Wait for capacitors to discharge
	lcd_delay(100);
}

static void wake_lcd() {\
	// Sleep out
	write_spi_command(SLPOUT);
	lcd_delay(10);

	// Internal oscilator on
	write_spi_command(OSCON);
	lcd_delay(10);

	// Power circuit on
	write_spi_command(PWRCTR);
	write_spi_data(0x0f);    // reference voltage regulator on, circuit voltage follower on, BOOST ON
	lcd_delay(100);

	// Turn on the display
	write_spi_command(DISON);
}

static void lcd_redraw(void) {
	int x,y;
	int c1, c2;

	write_spi_command(PASET);
	write_spi_data(2);
	write_spi_data(131);

	write_spi_command(CASET);
	write_spi_data(0);
	write_spi_data(129);

	write_spi_command(RAMWR);

	for (x = 2; x < 132; x++) {
		for (y = 0; y < 130; y=y+2) {
			c1 = image[x][y];
			write_spi_data((c1 >> 4) & 0xFF);
			c2 = image[x][y+1];
			write_spi_data(((c1 & 0xF) << 4) | ((c2 >> 8) & 0xF));
			write_spi_data(c2 & 0xFF);
		}
	}

	write_spi_command(NOP);
}

static void lcd_set_pixel(int x, int y, int color) {
	if ((x < 0) || (y < 0) || (x > 131) || (y > 131)) {
		printk(KERN_ALERT PREFIX "Bad coordinates");
		return;
	}

	write_spi_command(PASET);
	write_spi_data(x);
	write_spi_data(x);

	write_spi_command(CASET);
	write_spi_data(y);
	write_spi_data(y);

	write_spi_command(RAMWR);

	write_spi_data((color >> 4) & 0xFF);
	write_spi_data(((color & 0xF) << 4) | ((color >> 8) & 0xF));
	write_spi_data(color & 0xFF);

	write_spi_command(NOP);

	image[x][y] = color;
}

static void lcd_set_rect(int x0, int y0, int x1, int y1, int fill, int color) {
	int xmin, xmax, ymin, ymax;
	//int i;
	int x,y;
	if (fill == FILL) {
		// best way to create a filled rectangle is to define a drawing box
		// and loop two pixels at a time
		// calculate the min and max for x and y directions
		xmin = (x0 <= x1) ? x0 : x1;
		xmax = (x0 > x1) ? x0 : x1;
		ymin = (y0 <= y1) ? y0 : y1;
		ymax = (y0 > y1) ? y0 : y1;

		if ((xmin < 0) || (ymin < 0) || (xmax > 131) || (ymax > 131)) {
			printk(KERN_ALERT PREFIX "Bad coordinates");
			return;
		}

		// specify the controller drawing box according to those limits
		// Row address set (command 0x2B)
		write_spi_command(PASET);
		write_spi_data(xmin);
		write_spi_data(xmax);
		// Column address set (command 0x2A)
		write_spi_command(CASET);
		write_spi_data(ymin);
		write_spi_data(ymax);
		// WRITE MEMORY
		write_spi_command(RAMWR);
		// loop on total number of pixels / 2
//		for (i = 0; i < ((((xmax - xmin + 1) * (ymax - ymin + 1)) / 2) + 1); i++) {
//			// use the color value to output three data bytes covering two pixels
//			write_spi_data((color >> 4) & 0xFF);
//			write_spi_data(((color & 0xF) << 4) | ((color >> 8) & 0xF));
//			write_spi_data(color & 0xFF);
//		}
		if ((ymax-ymin)%2 == 1) {
			for (x = xmin; x <= xmax; x++) {
				for (y = ymin; y <= ymax; y=y+2) {
					write_spi_data((color >> 4) & 0xFF);
					image[x][y] = color;
					write_spi_data(((color & 0xF) << 4) | ((color >> 8) & 0xF));
					image[x][y + 1] = color;
					write_spi_data(color & 0xFF);
				}
			}
		}
		else {
			for (x = xmin; x <= xmax; x++) {
				for (y = ymin; y < ymax; y=y+2) {
					write_spi_data((color >> 4) & 0xFF);
					image[x][y] = color;
					write_spi_data(((color & 0xF) << 4) | ((color >> 8) & 0xF));
					image[x][y + 1] = color;
					write_spi_data(color & 0xFF);
				}
				if ((x-xmin)%2 == 0){
					write_spi_data((color >> 4) & 0xFF);
					write_spi_data(((color & 0xF) << 4) | ((color >> 8) & 0xF));
					write_spi_data(color & 0xFF);
				}
				image[x][ymax] = color;
			}
		}


	} else {
		lcd_set_line(x0, y0, x1, y0, color);
		lcd_set_line(x0, y1, x1, y1, color);
		lcd_set_line(x0, y0, x0, y1, color);
		lcd_set_line(x1, y0, x1, y1, color);
	}

	write_spi_command(NOP);
}

static void lcd_set_line (int x0, int y0, int x1, int y1, int color) {
	int dy = y1 - y0;
	int dx = x1 - x0;
	int stepx, stepy;
	if (dy < 0) {
		dy = -dy;
		stepy = -1;
	} else {
		stepy = 1;
	}
	if (dx < 0) {
		dx = -dx;
		stepx = -1;
	} else {
		stepx = 1;
	}
	dy <<= 1; // dy is now 2*dy
	dx <<= 1; // dx is now 2*dx
	lcd_set_pixel(x0, y0, color);
	if (dx > dy) {
		int fraction = dy - (dx >> 1); // same as 2*dy - dx
		while (x0 != x1) {
			if (fraction >= 0) {
				y0 += stepy;
				fraction -= dx; // same as fraction -= 2*dx
			}
			x0 += stepx;
			fraction += dy; // same as fraction -= 2*dy
			lcd_set_pixel(x0, y0, color);
		}
	} else {
		int fraction = dx - (dy >> 1);
		while (y0 != y1) {
			if (fraction >= 0) {
				x0 += stepx;
				fraction -= dy;
			}
			y0 += stepy;
			fraction += dx;
			lcd_set_pixel(x0, y0, color);
		}
	}
}

static void lcd_put_char(char c, int x, int y, int size, int fColor) {
	//extern const unsigned char FONT6x8[97][8];
	//extern const unsigned char FONT8x8[97][8];
	//extern const unsigned char FONT8x16[97][16];
	int i, j;
	unsigned int nCols;
	unsigned int nRows;
	unsigned int nBytes;
	unsigned char PixelRow;
	unsigned char Mask;
	unsigned int Word0;
	unsigned int Word1;
	unsigned char *pFont;
	unsigned char *pChar;
	unsigned char *FontTable[] = { (unsigned char *) FONT6x8,
			(unsigned char *) FONT8x8, (unsigned char *) FONT8x16 };

	// get pointer to the beginning of the selected font table
	pFont = (unsigned char *) FontTable[size];
	//pFont = (unsigned char *) FONT6x8;
	// get the nColumns, nRows and nBytes
	nCols = *pFont;
	nRows = *(pFont + 1);
	nBytes = *(pFont + 2);
	// get pointer to the last byte of the desired character
	///pChar = pFont + (nBytes * (c - 0x1F)) + nBytes - 1;
	pChar = pFont + (nBytes * (c - 0x1F));

	if ((x < 0) || (y < 0) || (x + nRows - 1 > 131) || (y + nCols - 1 > 131)) {
		printk(KERN_ALERT PREFIX "Bad coordinates");
		return;
	}

	// Row address set (command 0x2B)
	write_spi_command(PASET);
	write_spi_data(x);
	write_spi_data(x + nRows - 1);
	// Column address set (command 0x2A)
	write_spi_command(CASET);
	write_spi_data(y);
	write_spi_data(y + nCols - 1);
	// WRITE MEMORY
	write_spi_command(RAMWR);
	// loop on each row, working backwards from the bottom to the top
	//for (i = nRows - 1; i >= 0; i--) {

	for (i = x; i < nRows + x; i++) {
		// copy pixel row from font table and then decrement row
		PixelRow = *pChar++;
		// loop on each pixel in the row (left to right)
		// Note: we do two pixels each loop
		Mask = 0x80;
		//Mask = 0x01;
		for (j = y; j < nCols+y; j += 2) {
		//for (j = nCols-1; j >= 0; j -= 2) {
			// if pixel bit set, use foreground color; else use the background color
			// now get the pixel color for two successive pixels
			if ((PixelRow & Mask) == 0)
				Word0 = image[i][j];
			else
				Word0 = fColor;
			Mask = Mask >> 1;
			//Mask = Mask << 1;
			if ((PixelRow & Mask) == 0)
				Word1 = image[i][j+1];
			else
				Word1 = fColor;
			Mask = Mask >> 1;
			//Mask = Mask << 1;
			// use this information to output three data bytes
			write_spi_data((Word0 >> 4) & 0xFF);
			image[i][j] = Word0;
			write_spi_data(((Word0 & 0xF) << 4) | ((Word1 >> 8) & 0xF));
			image[i][j+1] = Word1;
			write_spi_data(Word1 & 0xFF);
		}
	}

	// terminate the Write Memory command
	write_spi_command(NOP);
}

static void lcd_put_str(char *pString, int x, int y, int Size, int fColor) {
         // loop until null-terminator is seen
         while (*pString != '\n') {
                  // draw the character
                  lcd_put_char(*pString++, x, y, Size, fColor);
                  // advance the y position
                  if (Size == SMALL)
                           y = y + 6;
                  else if (Size == MEDIUM)
                           y = y + 8;
                  else
                           y = y + 8;
                  // bail out if y exceeds 131
                  if (y > 131) break;
         }
}

static void lcd_set_bmp(int xmin, int ymin) {
	int color1, color2;
	int x, y;
	int i;
	int xmax = xmin + bmp_x -1;
	int ymax = ymin + bmp_y -1;

	if ((xmin < 0) || (ymin < 0) || (xmax > 131) || (ymax > 131)) {
		printk(KERN_ALERT PREFIX "Bad coordinates: %d %d %d %d", xmin, xmax, ymin, ymax);
		return;
	}

	printk(KERN_ALERT PREFIX "BMP height: %d, width: %d, len: %d\n", bmp_x, bmp_y, bmp_x*bmp_y);

	if (bmp == NULL) {
		printk(KERN_ALERT PREFIX "Attempt wo put null BMP.\n");
		return;
	}

	write_spi_command(PASET);
	write_spi_data(xmin);
	write_spi_data(xmax);

	write_spi_command(CASET);
	write_spi_data(ymin);
	write_spi_data(ymax);

	write_spi_command(RAMWR);

	i = 0;
	if ((ymax-ymin)%2 == 1) {
		for (x = xmin; x <= xmax; x++) {
			for (y = ymin; y <= ymax; y=y+2) {
				color1 = bmp[i++];
				color2 = bmp[i++];
				write_spi_data((color1 >> 4) & 0xFF);
				image[x][y] = color1;
				write_spi_data(((color1 & 0xF) << 4) | ((color2 >> 8) & 0xF));
				image[x][y + 1] = color2;
				write_spi_data(color2 & 0xFF);
			}
		}
	}
	else {
//		for (x = xmin; x <= xmax; x++) {
//			for (y = ymin; y < ymax; y=y+2) {
//				color1 = bmp[i++];
//				color2 = bmp[i++];
//				write_spi_data((color1 >> 4) & 0xFF);
//				image[x][y] = color1;
//				write_spi_data(((color1 & 0xF) << 4) | ((color2 >> 8) & 0xF));
//				image[x][y + 1] = color2;
//				write_spi_data(color2 & 0xFF);
//			}
//			if ((x-xmin)%2 == 0){
//				color1 = bmp[i++];
//				color2 = bmp[i++];
//				write_spi_data((color1 >> 4) & 0xFF);
//				write_spi_data(((color1 & 0xF) << 4) | ((color2 >> 8) & 0xF));
//				write_spi_data(color2 & 0xFF);
//			}
//			image[x][ymax] = color;
//		}
	}












//
//	for (x = xmin; x < xmin+bmp_x; x++) {
//		for (y = ymin; y < ymax; y=y+2) {
//			color1 = bmp[i++];
//			color2 = bmp[i++];
//			write_spi_data((color >> 4) & 0xFF);
//			image[x][y] = color;
//			write_spi_data(((color & 0xF) << 4) | ((color >> 8) & 0xF));
//			image[x][y + 1] = color;
//			write_spi_data(color & 0xFF);
//		}
//	}
//
//	for (i = 0; i < bmp_x*bmp_y; i=i+2) {
//		color1 = bmp[i];
//		color2 = bmp[i+1];
//		write_spi_data((color1 >> 4) & 0xFF);
//		write_spi_data(((color1 & 0xF) << 4) | ((color2 >> 8) & 0xF));
//		write_spi_data(color2 & 0xFF);
//	}

	write_spi_command(NOP);
}
