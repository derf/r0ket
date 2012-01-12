#include <sysinit.h>

#include "basic/basic.h"
#include "basic/config.h"
#include "lcd/lcd.h"
#include "lcd/render.h"
#include "filesystem/ff.h"

#include "usetable.h"

#define MV_WARN 3600

#define MAX_BRIGHTNESS 255;
#define MAX_ANIM 7

const unsigned char pwm[64] = {0, 1, 2, 2, 2, 3, 3, 4, 5, 6, 7, 8, 10, 11,
	13, 16, 19, 23, 27, 32, 38, 45, 54, 64, 76,
	91, 108, 128, 152, 181, 215, 255,
	215, 181, 152, 128, 108, 91,
	76, 64, 54, 45, 38, 32, 27, 23, 19, 16, 13,
	11, 10, 8, 7, 6, 5, 4, 3, 3, 2, 2, 2, 1, 0, 0};

void ram(void)
{
	char key;
	unsigned int i;
	unsigned int cnt = 0;
	unsigned char x = 0;
	unsigned const int cnt_max = 2560;
	unsigned const char x_max = 64;
	unsigned char step;
	unsigned char anim = 0;
	unsigned char next_anim = 0;

	unsigned char led[6];

	unsigned char mv_idx = 0;
	unsigned long int cur_mv = GetVoltage();
	unsigned int mv[32];
	unsigned char not_charging;

	gpioSetDir(RB_SPI_SS0, gpioDirection_Output);
	gpioSetDir(RB_SPI_SS1, gpioDirection_Output);
	gpioSetDir(RB_SPI_SS2, gpioDirection_Output);
	gpioSetDir(RB_SPI_SS3, gpioDirection_Output);
	gpioSetDir(RB_SPI_SS4, gpioDirection_Output);
	gpioSetDir(RB_SPI_SS5, gpioDirection_Output);

	for (i = 0; i < 32; i++)
		mv[i] = GetVoltage();

	for (;;) {

		if (++cnt >= cnt_max) {
			cnt = 0;
			if (++x == x_max) {
				x = 0;

				cur_mv = GetVoltage();
				not_charging = gpioGetValue(RB_PWR_CHRG);
				if (!not_charging)
					cur_mv -= 500;

				mv[++mv_idx % 32] = cur_mv;
				cur_mv = 0;

				for (i = 0; i < 32; i++)
					cur_mv += mv[i];
				cur_mv /= 32;

				if ((mv[mv_idx % 32] < MV_WARN) && not_charging)
					gpioSetValue(RB_LED0, 1);
				else
					gpioSetValue(RB_LED0, 0);

				if (++next_anim == 16) {
					next_anim = 0;
					anim = (anim + 1) % MAX_ANIM;

					lcdClear();
					lcdPrint("anim ");
					lcdPrintln(IntToStr(anim, 1, 0));
					lcdPrint(IntToStr(cur_mv, 4, F_LONG | F_ZEROS));
					lcdPrintln("mv");
					lcdDisplay();
				}
			}
			if (anim == 0) {
				led[0] = pwm[x];
				led[1] = pwm[(x + 10) % 64];
				led[2] = pwm[(x + 20) % 64];
				led[3] = pwm[(x + 30) % 64];
				led[4] = pwm[(x + 40) % 64];
				led[5] = pwm[(x + 50) % 64];
			}
			else if (anim == 1) {
				led[0] = pwm[(x + 50) % 64];
				led[1] = pwm[(x + 40) % 64];
				led[2] = pwm[(x + 30) % 64];
				led[3] = pwm[(x + 20) % 64];
				led[4] = pwm[(x + 10) % 64];
				led[5] = pwm[x];
			}
			else if (anim == 2) {
				led[0] = led[1] = led[2] = led[3] = led[4] = led[5]
					= pwm[x];
			}
			else if (anim == 3) {
				led[0] = ((x % 2) && (x < 6)) * 255;
				led[1] = ((x % 2) && (((x >=  6) && (x < 12)) || (x >= 56))) * 255;
				led[2] = ((x % 2) && (((x >= 12) && (x < 18)) || ((x >= 50) && (x < 56)))) * 255;
				led[3] = ((x % 2) && (((x >= 16) && (x < 24)) || ((x >= 44) && (x < 50)))) * 255;
				led[4] = ((x % 2) && (((x >= 24) && (x < 30)) || ((x >= 38) && (x < 44)))) * 255;
				led[5] = ((x % 2) && ((x >= 30) && (x < 38))) * 255;
			}
			else if (anim == 4) {
				for (i = 0; i < 6; i++)
					led[i] = ~pwm[(x + (10 * i)) % 64];
			}
			else if (anim == 5) {
				led[0] = led[1] = led[2] = led[3] = led[4] = led[5]
					= (x % 2) * MAX_BRIGHTNESS;
			}
			else if (anim == 6) {
				for (i = 0; i < 6; i++)
					led[i] = (x == i) * MAX_BRIGHTNESS;
			}
		}

		step = cnt % MAX_BRIGHTNESS;

		if (step == 0) {
			if (led[0])
				gpioSetValue(RB_SPI_SS5, 1);
			if (led[1])
				gpioSetValue(RB_SPI_SS4, 1);
			if (led[2])
				gpioSetValue(RB_SPI_SS3, 1);
			if (led[3])
				gpioSetValue(RB_SPI_SS2, 1);
			if (led[4])
				gpioSetValue(RB_SPI_SS1, 1);
			if (led[5])
				gpioSetValue(RB_SPI_SS0, 1);
		}

		if (step == led[0])
			gpioSetValue(RB_SPI_SS5, 0);
		if (step == led[1])
			gpioSetValue(RB_SPI_SS4, 0);
		if (step == led[2])
			gpioSetValue(RB_SPI_SS3, 0);
		if (step == led[3])
			gpioSetValue(RB_SPI_SS2, 0);
		if (step == led[4])
			gpioSetValue(RB_SPI_SS1, 0);
		if (step == led[5])
			gpioSetValue(RB_SPI_SS0, 0);

		for (i = 0; i < 60; i++)
			__asm volatile ("nop");

		key = getInputRaw();
		if (key != BTN_NONE) {
			gpioSetValue(RB_LED0, 0);
			gpioSetValue(RB_SPI_SS0, 0);
			gpioSetValue(RB_SPI_SS1, 0);
			gpioSetValue(RB_SPI_SS2, 0);
			gpioSetValue(RB_SPI_SS3, 0);
			gpioSetValue(RB_SPI_SS4, 0);
			gpioSetValue(RB_SPI_SS5, 0);
			return;
		}
	}
};
