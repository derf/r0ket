#include <sysinit.h>
#include <string.h>

#include "basic/basic.h"
#include "basic/config.h"

#include "lcd/render.h"
#include "lcd/print.h"

#include "filesystem/ff.h"

#include "usetable.h"

/**************************************************************************/

void ram(void) {
	unsigned char v;
	unsigned long int cur_mv;
	unsigned int mv[32];
	unsigned char mv_idx = 0;
	unsigned char i;
	unsigned long uptime = 0;
	char not_charging;
	unsigned char write_out = 0;

	FIL log;
	UINT wb;
	if (f_open(&log, "volt.log", FA_CREATE_ALWAYS | FA_WRITE))
		return;

	for (i = 0; i < 32; i++)
		mv[i] = GetVoltage();

	do {
		lcdClear();
		lcdPrintln("Battery status:");

		uptime = (_timectr)/(1000/SYSTICKSPEED);

		not_charging = gpioGetValue(RB_PWR_CHRG);
		cur_mv = GetVoltage();

		mv[++mv_idx % 32] = cur_mv;
		cur_mv = 0;

		for (i = 0; i < 32; i++)
			cur_mv += mv[i];
		cur_mv /= 32;

		v = cur_mv/1000;

		if (write_out++ == 100) {
			f_write(&log, IntToStr(uptime, 8, F_ZEROS | F_LONG), 8, &wb);
			f_write(&log, " ", 1, &wb);
			f_write(&log, IntToStr(cur_mv, 4, F_ZEROS | F_LONG), 4, &wb);
			f_write(&log, "\n", 1, &wb);
			write_out = 0;
		}

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

		lcdPrint(IntToStr((cur_mv - 3550) * 100 / (4100 - 3550), 2, F_ZEROS | F_LONG));
		lcdPrintln("%");
		lcdPrintln(IntToStr(uptime, 5, 0));
		lcdRefresh();
	} while (getInputWaitTimeout(242) == BTN_NONE);

	f_close(&log);
}
