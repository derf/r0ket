#include <sysinit.h>
#include <string.h>

#include "basic/basic.h"
#include "basic/config.h"

#include "lcd/render.h"
#include "lcd/print.h"

#include "usetable.h"

/**************************************************************************/

void ram(void) {
	unsigned int v, mv, raw_mv;
	char not_charging;

	mv = GetVoltage();

	do {
		lcdClear();
		lcdPrintln("Battery status:");

		not_charging = gpioGetValue(RB_PWR_CHRG);
		raw_mv = GetVoltage();

		if (!not_charging)
			raw_mv -= 500;

		mv = ((mv * 5) + raw_mv) / 6;
		v = mv/1000;

		lcdNl();

		if (!not_charging)
			lcdPrintln("   CHARGING");
		else if (mv < 3550)
			lcdPrintln(" Charge NOW!");
		else if (mv < 3650)
			lcdPrintln(" Charge soon");
		else if (mv < 4000)
			lcdPrintln("      OK");
		else if (mv < 4200)
			lcdPrintln("     Good");
		else
			lcdPrintln("     Full");

		lcdNl();
		lcdPrint(" ");
		lcdPrint(IntToStr(v,2,0));
		lcdPrint(".");
		lcdPrint(IntToStr(mv % 1000, 3, F_ZEROS | F_LONG));
		lcdPrint("V  ~");
		lcdPrint(IntToStr((mv - 3450) * 100 / (4100 - 3450), 2, F_ZEROS | F_LONG));
		lcdPrintln("%");
		lcdRefresh();
	} while (getInputWaitTimeout(242) == BTN_NONE);
}
