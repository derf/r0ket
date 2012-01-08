#include <sysinit.h>

#include "basic/basic.h"
#include "basic/config.h"
#include "lcd/lcd.h"
#include "lcd/render.h"
#include "filesystem/ff.h"

#include "usetable.h"

void next_image(unsigned char img);
void ledwobble(signed char mode);

const unsigned char pwm[32] = {
	0, 2, 3, 4, 6, 8, 11, 16, 23, 32, 45, 64,  90, 128, 181, 255,
	181, 128, 90, 64, 45, 32, 16, 11, 8, 6, 4, 3, 2, 0, 0
};

void ram(void)
{
	unsigned char i;
	unsigned int t = 0;
	unsigned char img = 0;
	char key;

	unsigned char mv_idx = 0;
	unsigned long int cur_mv = GetVoltage();
	unsigned int mv[32];
	unsigned int percent;
	int not_charging;

	FIL afile;
	UINT readbytes;

	lcdLoadImage("sr.lcd");
	lcdRefresh();


	IOCON_PIO1_11 = 0x00;
	IOCON_SWDIO_PIO1_3 = IOCON_SWDIO_PIO1_3_FUNC_GPIO;
	gpioSetDir(RB_LED3, gpioDirection_Output);
	gpioSetDir(1, 3, gpioDirection_Output);

	for (i = 0; i < 32; i++)
		mv[i] = GetVoltage();

	f_open(&afile, "nyan.lcd", FA_OPEN_EXISTING | FA_READ);


	for (;;) {

		if (!(t % 5)) {
			cur_mv = GetVoltage();
			not_charging = gpioGetValue(RB_PWR_CHRG);
			if (!not_charging)
				cur_mv -= 500;

			mv[++mv_idx % 32] = cur_mv;
			cur_mv = 0;

			for (i = 0; i < 32; i++)
				cur_mv += mv[i];
			cur_mv /= 32;
		}

		if (++t >= 250) {

			percent = (cur_mv - 3550) * 100 / (4100 - 3550);
			setExtFont(GLOBAL(nickfont));
			lcdClear();
			DoString(0,0, GLOBAL(nickname));

			if (percent > 100)
				percent = 100;

			if (!not_charging)
				DoString(0, 40, "+");
			else if (not_charging && (mv[mv_idx % 32] > 4200)) {
				DoString(0, 40, "=");
			}

			DoString(16, 40, (percent == 100 ? "1" : ""));
			DoString(32, 40, IntToStr(percent % 100, 2, F_ZEROS | F_LONG));
			DoString(64, 40, "%");
			lcdRefresh();

			if (not_charging && (mv[mv_idx % 32] < 4200))
				ledwobble(-1);
			else if (not_charging)
				ledwobble(0);
			else
				ledwobble(1);

			if (not_charging && (mv[mv_idx % 32] < 4200)) {
				t = 0;
				img = (img + 1) % 5;
				next_image(img);
				if (img == 2) {
					f_close(&afile);
					f_open(&afile, "nyan.lcd", FA_OPEN_EXISTING | FA_READ);
				}
				else if (img == 4) {
					f_close(&afile);
					f_open(&afile, "pinky.lcd", FA_OPEN_EXISTING | FA_READ);
				}
			}
			else
				t = 255;
		}

		if ((img == 2) && (t % 2)) {
			f_read(&afile, (char *)lcdBuffer, RESX * RESY_B, &readbytes);
			if (readbytes < (RESX * RESY_B)) {
				f_lseek(&afile, 0);
				continue;
			}
			lcdDisplay();
		}
		if (img == 4) {
			f_read(&afile, (char *)lcdBuffer, RESX * RESY_B, &readbytes);
			if (readbytes < (RESX * RESY_B)) {
				f_lseek(&afile, 0);
				continue;
			}
			lcdDisplay();
		}

		delayms_queue(100);

		if (t%2)
			gpioSetValue(1,3,1);
		else
			gpioSetValue(1,3,0);

		if (cur_mv < 3650) {
			if (t % 2)
				gpioSetValue(RB_LED0, 0);
			else
				gpioSetValue(RB_LED0, 1);
		}

		if ((t % 40) == 0) {
			gpioSetDir(RB_LED3, gpioDirection_Output);
			gpioSetValue(RB_LED2, 1);
			gpioSetValue(RB_LED0, 1);
		}
		if ((t % 40) == 1) {
			gpioSetValue(RB_LED2, 0);
			gpioSetValue(RB_LED0, 0);
			gpioSetValue(RB_LED1, 1);
			gpioSetValue(RB_LED3, 1);
		}
		if ((t % 40) == 2) {
			gpioSetValue(RB_LED1, 0);
			gpioSetValue(RB_LED3, 0);
			gpioSetDir(RB_LED3, gpioDirection_Input);
		}
		key = getInputRaw();
		if (key == BTN_ENTER)
			return;
	}
};

void next_image(unsigned char img)
{
	switch (img) {
	case 0:
		lcdLoadImage("sr.lcd");
		break;
	case 1:
		lcdLoadImage("cccd.lcd");
		break;
	case 2:
		break;
	case 3:
		lcdLoadImage("caek.lcd");
		break;
	case 4:
		break;
	}
	lcdRefresh();
}

void ledwobble(signed char mode)
{
	char key;
	unsigned char i;
	unsigned int cnt = 0;
	unsigned char x = 0;
	unsigned char fin = 0;
	unsigned const int cnt_max = 2560;
	unsigned const char x_max[3] = {64, 32, 48};
	unsigned char step;

	unsigned char led[4] = {0, 0, 0, 0};

	for (;;) {
		if (++cnt >= cnt_max) {
			cnt = 0;
			if (++x == x_max[mode + 1]) {
				x = 0;
				getInputWaitTimeout(500);

				key = getInputRaw();
				if (key == BTN_ENTER)
					return;

				if (++fin == 2)
					return;
			}

			if (mode == 1) {
				if (x < 32)
					led[0] = led[2] = pwm[x];
				if (x > 15)
					led[1] = led[3] = pwm[x - 16];
			}
			else if (mode == 0)
				led[0] = led[1] = led[2] = led[3] = pwm[x];
			else {
				if (x < 32)
					led[0] = led[1] = pwm[x];
				if (x > 31)
					led[2] = led[3] = pwm[x % 32];
				if (x == 32)
					getInputWaitTimeout(500);
			}

		}

		step = cnt % 256;

		if (step == 0) {
			if (led[0])
				gpioSetValue(RB_LED0, 1);
			if (led[1])
				gpioSetValue(RB_LED1, 1);
			if (led[2])
				gpioSetValue(RB_LED2, 1);
			if (led[3]) {
				gpioSetDir(RB_LED3, gpioDirection_Output);
				gpioSetValue(RB_LED3, 1);
			}
		}

		if (step == led[0])
			gpioSetValue(RB_LED0, 0);
		if (step == led[1])
			gpioSetValue(RB_LED1, 0);
		if (step == led[2])
			gpioSetValue(RB_LED2, 0);
		if (step == led[3]) {
			gpioSetValue(RB_LED3, 0);
			gpioSetDir(RB_LED3, gpioDirection_Input);
		}

		for (i = 0; i < 100; i++)
			__asm volatile ("nop");
	}
}
