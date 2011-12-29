#include <sysinit.h>

#include "basic/basic.h"
#include "basic/config.h"
#include "lcd/lcd.h"
#include "lcd/render.h"
#include "filesystem/ff.h"

#include "usetable.h"

void ram(void)
{
	unsigned char t = 0;
	int mv;
	unsigned char percent;
	char key;
	lcdLoadImage("sr.lcd");
	lcdRefresh();

	IOCON_PIO1_11 = 0x00;
	IOCON_SWDIO_PIO1_3 = IOCON_SWDIO_PIO1_3_FUNC_GPIO;
	gpioSetDir(RB_LED3, gpioDirection_Output);
	gpioSetDir(1, 3, gpioDirection_Output);

	for (;;) {
		if (++t >= 110) {
			if (t == 110) {
				mv = GetVoltage();
				percent = (mv - 3450) * 100 / (4100 - 3450);
				setExtFont(GLOBAL(nickfont));
				lcdClear();
				DoString(0,0, GLOBAL(nickname));
				DoString(0, 40, (percent > 100 ? "1" : ""));
				DoString(16, 40, IntToStr(percent % 100, 2, F_ZEROS));
				DoString(50, 40, "%");
				lcdRefresh();
			}
			if (t == 160) {
				t = 0;
				lcdLoadImage("sr.lcd");
				lcdRefresh();
			}
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
