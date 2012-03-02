#include <sysinit.h>

#include "basic/basic.h"
#include "basic/config.h"
#include "lcd/lcd.h"
#include "lcd/render.h"
#include "filesystem/ff.h"

#include "usetable.h"

void ram(void)
{
	unsigned int i;
	unsigned char cur = 0;

	FIL snd;
	UINT readbytes;
	unsigned char buf[2048];
	unsigned int bufin = 0;
	unsigned int fpos = sizeof(buf);
	unsigned int delay = 0;

	gpioSetDir(RB_SPI_SS0, gpioDirection_Output);
	gpioSetDir(RB_SPI_SS1, gpioDirection_Output);
	gpioSetDir(RB_SPI_SS2, gpioDirection_Output);
	gpioSetDir(RB_SPI_SS3, gpioDirection_Output);
	gpioSetDir(RB_SPI_SS4, gpioDirection_Output);
	gpioSetDir(RB_SPI_SS5, gpioDirection_Output);

	gpioSetValue(RB_SPI_SS0, 0);
	gpioSetValue(RB_SPI_SS1, 0);
	gpioSetValue(RB_SPI_SS2, 0);
	gpioSetValue(RB_SPI_SS3, 0);
	gpioSetValue(RB_SPI_SS4, 0);
	gpioSetValue(RB_SPI_SS5, 0);

	f_open(&snd, "nyan.5nd", FA_OPEN_EXISTING | FA_READ);
	f_read(&snd, buf, sizeof(buf), &readbytes);

	while (getInputRaw() == BTN_NONE) {

		if (++bufin == sizeof(buf)) {
			bufin = 0;
			f_read(&snd, buf, sizeof(buf), &readbytes);
			fpos += readbytes;
			if (readbytes < sizeof(buf))
				break;

			lcdClear();
			lcdPrint("fpos ");
			lcdPrintln(IntToStr(fpos, 8, F_ZEROS | F_LONG));
			lcdPrint(IntToStr(fpos / 4, 5, F_ZEROS | F_LONG));
			lcdPrintln("ms");
			lcdRefresh();

		}

		cur = buf[bufin];

		gpioSetValue(RB_SPI_SS5, (cur & 0x80) ? 1 : 0);
		gpioSetValue(RB_SPI_SS4, (cur & 0x40) ? 1 : 0);
		gpioSetValue(RB_SPI_SS3, (cur & 0x20) ? 1 : 0);
		gpioSetValue(RB_SPI_SS2, (cur & 0x10) ? 1 : 0);
		gpioSetValue(RB_SPI_SS1, (cur & 0x08) ? 1 : 0);
		gpioSetValue(RB_SPI_SS0, (cur & 0x06) ? 1 : 0);

		delayms(1);
	}

	gpioSetValue(RB_SPI_SS0, 0);
	gpioSetValue(RB_SPI_SS1, 0);
	gpioSetValue(RB_SPI_SS2, 0);
	gpioSetValue(RB_SPI_SS3, 0);
	gpioSetValue(RB_SPI_SS4, 0);
	gpioSetValue(RB_SPI_SS5, 0);

};
