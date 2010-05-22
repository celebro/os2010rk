#include "lcd_driver.h"
#include "lcd_ioctl.h"
#include "lcd_protocol.h"

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
static unsigned char *buf;

/* File operations for that device */
static struct file_operations lcd_fops = {
  .read 	= lcd_read,
  .write 	= lcd_write,
  .open 	= lcd_open,
  .release 	= lcd_release,
  .ioctl	= lcd_ioctl
};

/* Testing parameters */
unsigned int vol = 32;
unsigned int res = 3;
unsigned int col = YELLOW;
unsigned int temp = 0;
module_param(vol, uint, S_IRUGO);
module_param(res, uint, S_IRUGO);
module_param(col, uint, S_IRUGO);
module_param(temp, uint, S_IRUGO);


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

	buf = kmalloc(sizeof(unsigned char)*BUF_SIZE, GFP_KERNEL);
	if (buf == NULL) {
		printk(KERN_ALERT PREFIX "Failed allocate memmory for buffer.\n");
		return -1;
	}

	printk(KERN_ALERT PREFIX "Module loaded v09.\n");

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

	kfree(buf);

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
//				printk(KERN_ALERT PREFIX "x1: %d, y1: %d, x2:%d, y2: %d; fill: %d, color: %d\n",param.x1, param.y1, param.x2, param.y2, param.fill, RED);
				lcd_set_rect(param.x1, param.y1, param.x2, param.y2, param.fill, param.color);
			}
			break;
		case LCD_SET_LINE:
			printk(KERN_ALERT PREFIX "Set line.\n");
			if (length-1 >= sizeof(struct lcd_func_params)) {
				copy_from_user(&param, buffer+1, sizeof(struct lcd_func_params));
				lcd_set_line(param.x1, param.y1, param.x2, param.y2, param.color);
			}
			break;
			break;
		case '0': lcd_set_rect(2, 0, 131, 129, FILL, WHITE); break;
		case '1': lcd_set_rect(2, 0, 131, 129, FILL, BLACK); break;
		case '2': lcd_set_rect(2, 0, 131, 129, FILL, RED); break;
		case '3': lcd_set_rect(2, 0, 131, 129, FILL, GREEN); break;
		case '4': lcd_set_rect(2, 0, 131, 129, FILL, BLUE); break;
		case '5': lcd_set_rect(2, 0, 131, 129, FILL, CYAN); break;
		case '6': lcd_set_rect(2, 0, 131, 129, FILL, BROWN); break;
		case '7': lcd_set_rect(2, 0, 131, 129, FILL, ORANGE); break;
		case '8': lcd_set_rect(2, 0, 131, 129, FILL, PINK); break;
		case '9': lcd_set_line(2, 0, 100, 100, YELLOW); break;

	}

	return length;

//	char type = 0;
//	char data = 0;
//	int i;
//
//	printk(KERN_ALERT PREFIX "Write, length=%d.\n",length);
//
//	if (length > BUF_SIZE) {
//		length = BUF_SIZE;
//	}
//
//	if (length % 2 == 1)
//		length--;
//
//	if (copy_from_user(buf, buffer, length)) {
//		return -EFAULT;
//	}
//
//	for (i = 0; i < length; ) {
////		get_user(type, buffer+i);
////		i++;
////		get_user(data,buffer+i);
//
//		type = buf[i++];
//		data = buf[i++];
//
//		//printk(KERN_ALERT PREFIX "Type = %d; data = %d\n", type, data);
//
//		if (type == 1)
//			write_spi_data(data);
//		else if (type == 0) {
//			if ((data == PASET) || (data == CASET) || (data == RAMWR)) {
//				write_spi_command(data);
//			}
//		}
//	}
//
//	return length;

//	unsigned char input;
//	int i;
//
//	for (i = 0; i < length; i++) {
//		get_user(input, buffer+i);
//		printk(KERN_ALERT PREFIX "Write: %i\n", (int)input);
//	}
//
//	if (length > 0) {
//		get_user(input, buffer);
//	}
//	else {
//		return 0;
//	}
//
//	switch (input) {
//		case '0': LCDSetRect(2, 0, 131, 129, BLACK); break;
//		case '1': LCDSetRect(2, 0, 131, 129, BLACK); break;
//		case '2': LCDSetRect(2, 0, 131, 129, RED); break;
//		case '3': LCDSetRect(2, 0, 131, 129, GREEN); break;
//		case '4': LCDSetRect(2, 0, 131, 129, BLUE); break;
//		case '5': LCDSetRect(2, 0, 131, 129, CYAN); break;
//		case '6': LCDSetRect(2, 0, 131, 129, BROWN); break;
//		case '7': LCDSetRect(2, 0, 131, 129, ORANGE); break;
//		case '8': LCDSetRect(2, 0, 131, 129, PINK); break;
//		case '9': LCDSetRect(2, 0, 131, 129, YELLOW); break;
//		case '0': LCDSetRect(3, 1, 20, 4, RED); break;
//		case '1': LCDSetRect(2, 0, 20, 4, BLUE); break;
//		case '2': LCDSetRect(3, 1, 130, 128, RED); break;
//		case '3': LCDSetRect(2, 0, 131, 129, BLUE); break;
//	}
//
//	return length;
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
	int i;
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
	write_spi_data(62); // P1 = 32 volume value (experiment with this value to get the best contrast)
	write_spi_data(3);    // P2 = 3    resistance ratio (only value that works)

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
	lcd_delay(200);

	/* Single color backfill */
	write_spi_command(PASET);
	write_spi_data(2);
	write_spi_data(131);

	write_spi_command(CASET);
	write_spi_data(0);
	write_spi_data(129);

	write_spi_command(RAMWR);

	for (i = 0; i < 130*130/2; i++) {
		write_spi_data(0x00);
		write_spi_data(0x00);
		write_spi_data(0x00);
	}
	write_spi_command(NOP);

	// Turn on the display
	write_spi_command(DISON);
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

static void lcd_set_pixel(int x, int y, int color) {
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
}

static void lcd_set_rect(int x0, int y0, int x1, int y1, int fill, int color) {
	int      xmin, xmax, ymin, ymax;
	int               i;
	// best way to create a filled rectangle is to define a drawing box
	// and loop two pixels at a time
	// calculate the min and max for x and y directions
	xmin = (x0 <= x1) ? x0 : x1;
	xmax = (x0 > x1) ? x0 : x1;
	ymin = (y0 <= y1) ? y0 : y1;
	ymax = (y0 > y1) ? y0 : y1;
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
	for (i = 0; i < ((((xmax - xmin + 1) * (ymax - ymin + 1)) / 2)+1); i++) {
		   // use the color value to output three data bytes covering two pixels
		   write_spi_data((color >> 4) & 0xFF);
		   write_spi_data(((color & 0xF) << 4) | ((color >> 8) & 0xF));
		   write_spi_data(color & 0xFF);
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
