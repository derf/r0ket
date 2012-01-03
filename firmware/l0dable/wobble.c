#include <sysinit.h>

#include "basic/basic.h"
#include "basic/config.h"
#include "lcd/lcd.h"
#include "lcd/render.h"
#include "filesystem/ff.h"

#include "usetable.h"

#define MAX_BRIGHTNESS 255;

const unsigned char pwm[32] = {0, 2, 3, 4, 6, 8, 11, 16, 23, 32, 45, 64,
	90, 128, 181, 255,
181, 128, 90, 64, 45, 32, 16, 11, 8, 6, 4, 3, 2, 0, 0};

const unsigned char pwm_bl[32] = { 0, 1, 1, 2, 3, 4, 5, 8, 11, 16, 22, 32,
	45, 64, 90, 100,
	90, 64, 45, 32, 22, 16, 11, 8, 5, 4, 3, 2, 1, 1, 0, 0};

/*
 * Designed for a two-pin red/green LED connected to SS0 / SS1
 */

void ram(void)
{
	lcdLoadImage("sr.lcd");
	lcdRefresh();

	char key;
	unsigned int i;
	unsigned int cnt = 0;
	unsigned char x = 0;
	unsigned const int cnt_max = 2560;
	unsigned const char x_max = 32;
	unsigned char step;

	unsigned int con_2_4 = IOCON_PIO2_4;
	unsigned int con_2_5 = IOCON_PIO2_5;

	unsigned char led[5];
	IOCON_PIO1_11 = 0x00;

	IOCON_SWDIO_PIO1_3 = IOCON_SWDIO_PIO1_3_FUNC_GPIO;
	IOCON_PIO2_4 = IOCON_PIO2_4_FUNC_GPIO;
	IOCON_PIO2_5 = IOCON_PIO2_5_FUNC_GPIO;

	gpioSetDir(RB_LED3, gpioDirection_Output);
	gpioSetDir(RB_SPI_SS0, gpioDirection_Output);
	gpioSetDir(RB_SPI_SS1, gpioDirection_Output);

	gpioSetDir(RB_HB0, gpioDirection_Output);

	for (;;) {

		if (++cnt >= cnt_max) {
			cnt = 0;
			if (++x == x_max) {
				x = 0;
				/* set next mode here */
			}
			for (i = 0; i < 4; i++) {
				led[0] = pwm[x];
				led[1] = pwm[x];
				led[2] = pwm[(x+16) % 32];
				led[3] = pwm[(x+16) % 32];
				led[4] = (x < 16) ? (x * 17) : (527 - x * 17);
			}
//			TMR_TMR16B1MR0 = 0xFFFF * (100 - pwm_bl[x]) / 100;
		}

		step = cnt % MAX_BRIGHTNESS;

		if (step == 0) {
			gpioSetValue(RB_LED0, 1);
			gpioSetValue(RB_LED1, 1);
			gpioSetValue(RB_LED2, 1);
			gpioSetValue(RB_LED3, 1);
			gpioSetValue(RB_SPI_SS0, 1);
			gpioSetValue(RB_SPI_SS1, 0);
		}

		if (step == led[0])
			gpioSetValue(RB_LED0, 0);
		if (step == led[1])
			gpioSetValue(RB_LED1, 0);
		if (step == led[3])
			gpioSetValue(RB_LED2, 0);
		if (step == led[3])
			gpioSetValue(RB_LED3, 0);
		if (step == led[4]) {
			gpioSetValue(RB_SPI_SS0, 0);
			gpioSetValue(RB_SPI_SS1, 1);
		}

		for (i = 0; i < 100; i++)
			__asm volatile ("nop");

		key = getInputRaw();
		if (key != BTN_NONE) {
			gpioSetValue(RB_LED0, 0);
			gpioSetValue(RB_LED1, 0);
			gpioSetValue(RB_LED2, 0);
			gpioSetValue(RB_LED3, 0);
			IOCON_PIO2_4 = con_2_4;
			IOCON_PIO2_5 = con_2_5;
			return;
		}
	}
};
