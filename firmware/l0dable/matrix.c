#include <sysinit.h>

#include "basic/basic.h"
#include "basic/config.h"
#include "lcd/lcd.h"
#include "lcd/render.h"
#include "filesystem/ff.h"

#include "usetable.h"

#define MV_WARN 3600

#define MAX_BRIGHTNESS 255;
#define MAX_ANIM 1

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
	unsigned const char x_max = 255;
	unsigned char step;
	unsigned char anim = 0;
	unsigned char next_anim = 0;

	unsigned char led[3][3];

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

	gpioSetValue(RB_SPI_SS0, 1);
	gpioSetValue(RB_SPI_SS1, 1);
	gpioSetValue(RB_SPI_SS2, 1);

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
				led[0][0] = (x < 64) ? pwm[x] : 0;
				led[1][0] = ((x >=  13) && (x <  77)) ? pwm[x -  13] : 0;
				led[2][0] = ((x >=  26) && (x <  90)) ? pwm[x -  26] : 0;
				led[2][1] = ((x >=  39) && (x < 103)) ? pwm[x -  39] : (
				            ((x >= 117) && (x < 181)) ? pwm[x - 117] : 0);
				led[2][2] = ((x >=  52) && (x < 116)) ? pwm[x -  52] : (
				            ((x >= 130) && (x < 194)) ? pwm[x - 130] : 0);
				led[1][2] = ((x >=  65) && (x < 129)) ? pwm[x -  65] : (
				            ((x >= 143) && (x < 207)) ? pwm[x - 143] : 0);
				led[0][2] = ((x >=  78) && (x < 142)) ? pwm[x -  78] : (
				            ((x >= 156) && (x < 220)) ? pwm[x - 156] : 0);
				led[0][1] = ((x >=  91) && (x < 155)) ? pwm[x -  91] : (
				            ((x >= 169) && (x < 233)) ? pwm[x - 169] : 0);
				led[1][1] = ((x >= 104) && (x < 168)) ? pwm[x - 104] : (
				            ((x >= 182) && (x < 246)) ? pwm[x - 182] : 0);
			}
		}

		step = cnt % MAX_BRIGHTNESS;

		gpioSetValue(RB_SPI_SS3, (step <= led[0][0]));
		gpioSetValue(RB_SPI_SS4, (step <= led[0][1]));
		gpioSetValue(RB_SPI_SS5, (step <= led[0][2]));

		gpioSetValue(RB_SPI_SS0, 0);
		gpioSetValue(RB_SPI_SS0, 1);

		gpioSetValue(RB_SPI_SS3, (step <= led[1][0]));
		gpioSetValue(RB_SPI_SS4, (step <= led[1][1]));
		gpioSetValue(RB_SPI_SS5, (step <= led[1][2]));

		gpioSetValue(RB_SPI_SS1, 0);
		gpioSetValue(RB_SPI_SS1, 1);

		gpioSetValue(RB_SPI_SS3, (step <= led[2][0]));
		gpioSetValue(RB_SPI_SS4, (step <= led[2][1]));
		gpioSetValue(RB_SPI_SS5, (step <= led[2][2]));

		gpioSetValue(RB_SPI_SS2, 0);
		gpioSetValue(RB_SPI_SS2, 1);

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
