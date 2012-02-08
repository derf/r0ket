#include <sysinit.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>
#include "basic/basic.h"
#include "lcd/render.h"
#include "lcd/allfonts.h"
#include "basic/config.h"
#include "basic/byteorder.h"
#include "lcd/lcd.h"
#include "lcd/print.h"
#include "funk/nrf24l01p.h"
#include "usbcdc/usb.h"
#include "usbcdc/usbcore.h"
#include "usbcdc/usbhw.h"
#include "usbcdc/cdcuser.h"
#include "usbcdc/cdc_buf.h"
#include "usbcdc/util.h"
#include "core/ssp/ssp.h"

#if CFG_USBMSC
#error "MSC is defined"
#endif

#if !CFG_USBCDC
#error "CDC is not defined"
#endif


void main_bridge(void)
{
    GLOBAL(privacy) = 3;
    char input[128];
    unsigned int i;
    unsigned char offs;

    usbCDCInit();
    delayms(500);
    while(1){
        int l, i, status;
        CDC_OutBufAvailChar (&l);
	   gpioSetValue(RB_LED2, 0);
        if(l>0){
		gpioSetValue(RB_LED2, 1);
            CDC_RdOutBuf (input, &l);
		  /*
		  if (input[0] == '\r')
			puts("\r\nderf@r0ket > ");
		else
		  puts(input);
*/
		if (input[0] == 'l') {
			switch (input[1]) {
				case '0': gpioSetValue(RB_LED0, (input[2] == '1') ? 1 : 0); break;
				case '1': gpioSetValue(RB_LED1, (input[2] == '1') ? 1 : 0); break;
				case '2': gpioSetValue(RB_LED2, (input[2] == '1') ? 1 : 0); break;
			}
		}
		else if ((input[0] == 'D') || (input[0] == 'd')) {
			offs = (signed char)input[1] - 32;
			if (offs <= 26)
				for (i = 0; (i < 32) && ((offs * 32 + i) < (RESX*RESY_B)); i++)
					lcdBuffer[offs * 32 + i] = input[i + 2];
			if (input[0] == 'd')
				lcdRefresh();
		}
		else if (input[0] == 'p') {
			input[l-1] = '\0';
			for (i = 0; (i+2) < l; i += 14)
				lcdPrintln(input + i + 1);
			lcdRefresh();
		}
		else if (input[0] == 'P') {
			input[l-1] = '\0';
			for (i = 0; (i+2) < l; i += 14)
				lcdPrintln(input + i + 1);
		}
		else if (input[0] == 'c') {
			lcdClear();
			lcdRefresh();
		}
		else if (input[0] == 'C') {
			lcdRefresh();
			lcdClear();
		}
        }
    }
}

void tick_bridge(void){
    return;
};
