#include <sysinit.h>

#include "basic/basic.h"
#include "basic/config.h"
#include "lcd/lcd.h"
#include "lcd/render.h"
#include "filesystem/ff.h"

#include "usetable.h"

void ledwobble(char mode);

const unsigned char pwm[32] = {
	0, 2, 3, 4, 6, 8, 11, 16, 23, 32, 45, 64,  90, 128, 181, 255,
	181, 128, 90, 64, 45, 32, 16, 11, 8, 6, 4, 3, 2, 0, 0
};

void ram(void)
{
	unsigned char t = 0;
	unsigned int raw_mv = 0, mv = 0;
	int not_charging;
	unsigned int percent;
	char key;
	lcdLoadImage("sr.lcd");
	lcdRefresh();

	IOCON_PIO1_11 = 0x00;
	IOCON_SWDIO_PIO1_3 = IOCON_SWDIO_PIO1_3_FUNC_GPIO;
	gpioSetDir(RB_LED3, gpioDirection_Output);
	gpioSetDir(1, 3, gpioDirection_Output);

	mv = GetVoltage();

	for (;;) {
		if (++t >= 230) {
			raw_mv = GetVoltage();
			not_charging = gpioGetValue(RB_PWR_CHRG);

			if (!not_charging)
				raw_mv -= 500;

			mv = ((mv * 3) + raw_mv) / 4;

			percent = (mv - 3450) * 100 / (4100 - 3450);
			setExtFont(GLOBAL(nickfont));
			lcdClear();
			DoString(0,0, GLOBAL(nickname));

			if (!not_charging)
				DoString(0, 40, "+");
			else if (not_charging && (raw_mv > 4200)) {
				DoString(0, 40, "=");
			}

			DoString(16, 40, (percent > 100 ? "1" : ""));
			DoString(32, 40, IntToStr(percent % 100, 2, F_ZEROS | F_LONG));
			DoString(64, 40, "%");
			lcdRefresh();

			ledwobble((not_charging && (raw_mv < 4200)));

			if (not_charging && (raw_mv < 4200)) {
				t = 0;
				lcdLoadImage("sr.lcd");
				lcdRefresh();
			}
			else
				t = 233;
		}

		delayms(80);

		if (t%2)
			gpioSetValue(1,3,1);
		else
			gpioSetValue(1,3,0);

		if ((t % 20) == 0) {
			gpioSetValue(RB_LED2, 1);
			gpioSetValue(RB_LED0, 1);
		}
		if ((t % 20) == 1) {
			gpioSetValue(RB_LED2, 0);
			gpioSetValue(RB_LED0, 0);
			gpioSetValue(RB_LED1, 1);
			gpioSetValue(RB_LED3, 1);
		}
		if ((t % 20) == 2) {
			gpioSetValue(RB_LED1, 0);
			gpioSetValue(RB_LED3, 0);
		}
		key = getInputRaw();
		if (key == BTN_ENTER)
			return;
	}
};

void ledwobble(char mode)
{
	char key;
	unsigned char i;
	unsigned int cnt = 0;
	unsigned char x = 0;
	unsigned char fin = 0;
	unsigned const int cnt_max = 2560;
	unsigned const char x_max[2] = {32, 48};
	unsigned char step;

	unsigned char led[4] = {0, 0, 0, 0};

	for (;;) {
		if (++cnt >= cnt_max) {
			cnt = 0;
			if (++x == x_max[mode]) {
				x = 0;
				delayms(500);

				key = getInputRaw();
				if (key == BTN_ENTER)
					return;

				if (++fin == 5)
					return;
			}

			if (mode == 1) {
				if (x < 32)
					led[0] = led[2] = pwm[x];
				if (x > 15)
					led[1] = led[3] = pwm[x - 16];
			}
			else {
				led[0] = led[1] = led[2] = led[3] = pwm[x];
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
			if (led[3])
				gpioSetValue(RB_LED3, 1);
		}

		if (step == led[0])
			gpioSetValue(RB_LED0, 0);
		if (step == led[1])
			gpioSetValue(RB_LED1, 0);
		if (step == led[2])
			gpioSetValue(RB_LED2, 0);
		if (step == led[3])
			gpioSetValue(RB_LED3, 0);

		for (i = 0; i < 100; i++)
			__asm volatile ("nop");
	}
}
