#include <sysinit.h>
#include <string.h>

#include "basic/basic.h"
#include "basic/config.h"

#include "lcd/render.h"
#include "lcd/print.h"

#include "usetable.h"

/**************************************************************************/

void ram(void) {
	unsigned char v;
	unsigned long int cur_mv;
	unsigned int mv[32];
	unsigned char mv_idx = 0;
	unsigned char i;
	char not_charging;

	for (i = 0; i < 32; i++)
		mv[i] = GetVoltage();

	do {
		lcdClear();
		lcdPrintln("Battery status:");

		not_charging = gpioGetValue(RB_PWR_CHRG);
		cur_mv = GetVoltage();

		if (!not_charging)
			cur_mv -= 500;

		mv[++mv_idx % 32] = cur_mv;
		cur_mv = 0;

		for (i = 0; i < 32; i++)
			cur_mv += mv[i];
		cur_mv /= 32;

		v = cur_mv/1000;

		lcdNl();

		if (!not_charging)
			lcdPrintln("   CHARGING");
		else if (cur_mv < 3550)
			lcdPrintln(" Charge NOW!");
		else if (cur_mv < 3650)
			lcdPrintln(" Charge soon");
		else if (cur_mv < 4000)
			lcdPrintln("      OK");
		else if (cur_mv < 4200)
			lcdPrintln("     Good");
		else
			lcdPrintln("     Full");

		lcdNl();
		lcdPrint(" ");
		lcdPrint(IntToStr(v,2,0));
		lcdPrint(".");
		lcdPrint(IntToStr(cur_mv % 1000, 3, F_ZEROS | F_LONG));
		lcdPrint("V  ~");

		/* battery full */
		if (cur_mv > 4200)
			cur_mv = 4200;

		lcdPrint(IntToStr((cur_mv - 3450) * 100 / (4100 - 3450), 2, F_ZEROS | F_LONG));
		lcdPrintln("%");
		lcdRefresh();
	} while (getInputWaitTimeout(242) == BTN_NONE);
}
