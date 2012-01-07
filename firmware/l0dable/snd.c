#include <sysinit.h>

#include "basic/basic.h"
#include "basic/config.h"
#include "lcd/lcd.h"
#include "lcd/render.h"
#include "filesystem/ff.h"

#include "usetable.h"

const unsigned char pwm[16] = {0, 2, 3, 4, 6, 8, 11, 16, 23, 32, 45, 64,
	90, 128, 181, 255};

void ram(void)
{
	unsigned int i;
	unsigned int cnt = 0;
	unsigned const int cnt_max = 256;
	unsigned int step;
	unsigned char cur = 0;

	FIL snd;
	UINT readbytes;
	unsigned char buf[2048];
	unsigned int bufin = 0;
	unsigned int fpos = sizeof(buf);

	unsigned int con_2_4 = IOCON_PIO2_4;
	unsigned int con_2_5 = IOCON_PIO2_5;
	unsigned int con_3_2 = IOCON_PIO3_2;

	IOCON_PIO2_4 = IOCON_PIO2_4_FUNC_GPIO;
	IOCON_PIO2_5 = IOCON_PIO2_5_FUNC_GPIO;
	IOCON_PIO3_2 = IOCON_PIO3_2_FUNC_GPIO;

	gpioSetDir(RB_SPI_SS0, gpioDirection_Output);
	gpioSetDir(RB_SPI_SS1, gpioDirection_Output);
	gpioSetDir(RB_SPI_SS3, gpioDirection_Output);

	f_open(&snd, "nyan.5nd", FA_OPEN_EXISTING | FA_READ);
	f_read(&snd, buf, sizeof(buf), &readbytes);

	while (getInputRaw() == BTN_NONE) {

		if (++cnt >= cnt_max) {
			cnt = 0;

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
		}

		step = cnt % 256;

		if (step == 0) {
			gpioSetValue(RB_SPI_SS0, 0);
			gpioSetValue(RB_SPI_SS1, 1);
		}

		if (step == cur) {
			gpioSetValue(RB_SPI_SS0, 1);
			gpioSetValue(RB_SPI_SS1, 0);
		}

	}

	gpioSetValue(RB_SPI_SS0, 0);
	gpioSetValue(RB_SPI_SS1, 0);

	IOCON_PIO2_4 = con_2_4;
	IOCON_PIO2_5 = con_2_5;
	IOCON_PIO3_2 = con_3_2;
};
