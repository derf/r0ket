#include <sysinit.h>
#include "basic/basic.h"
#include "basic/config.h"

#include "lcd/lcd.h"
#include "lcd/fonts/smallfonts.h"
#include "lcd/print.h"
#include "lcd/image.h"
#include "filesystem/ff.h"
#include "usb/usbmsc.h"
#include "basic/random.h"
#include "funk/nrf24l01p.h"

/**************************************************************************/

void main_default(void) {
    systickInit(SYSTICKSPEED);
    
	//show bootscreen
	lcdClear();
	lcdLoadImage("r0ket.lcd");
	lcdRefresh();
	lcdClear();
    
    readConfig();
	if(getInputRaw()==BTN_RIGHT){
            GLOBAL(develmode)=1;
            applyConfig();
    };
    randomInit();

    return;
};



static void queue_setinvert(void){
    lcdSetInvert(1);
};
static void queue_unsetinvert(void){
    lcdSetInvert(0);
};

#define EVERY(x,y) if((ctr+y)%(x/SYSTICKSPEED)==0)

// every SYSTICKSPEED ms
void tick_default(void) {
    static int ctr;
    ctr++;
    incTimer();

    EVERY(1024,0){
        if(!adcMutex){
            VoltageCheck();
            LightCheck();
        }else{
            ctr--;
        };
    };

	if (GLOBAL(positionleds)) {
		int f = isNight();
		EVERY(2000, 0) {
			gpioSetValue(RB_LED0, f);
			gpioSetValue(RB_LED2, f);
		}
		EVERY(2000, 10) {
			gpioSetValue(RB_LED0, 1 - f);
			gpioSetValue(RB_LED2, 1 - f);
		}
		EVERY(2000, 20) {
			gpioSetValue(RB_LED0, f);
			gpioSetValue(RB_LED2, f);
		}
		EVERY(2000, 30) {
			gpioSetValue(RB_LED0, 1 - f);
			gpioSetValue(RB_LED2, 1 - f);
		}
	}

    static char night=0;
    EVERY(128,2){
        if(night!=isNight()){
            night=isNight();
            if(night){
                backlightSetBrightness(GLOBAL(lcdbacklight));
                push_queue(queue_unsetinvert);
            }else{
                backlightSetBrightness(0);
                push_queue(queue_setinvert);
           };
        };
    };


    EVERY(50,0){
        if(GLOBAL(chargeled)){
            char iodir= (GPIO_GPIO1DIR & (1 << (11) ))?1:0;
            if(GetChrgStat()) {
                if (iodir == gpioDirection_Input){
                    IOCON_PIO1_11 = 0x0;
                    gpioSetDir(RB_LED3, gpioDirection_Output);
                    gpioSetValue (RB_LED3, 1);
                    LightCheck();
                }
            } else {
                if (iodir != gpioDirection_Input){
                    gpioSetValue (RB_LED3, 0);
                    gpioSetDir(RB_LED3, gpioDirection_Input);
                    IOCON_PIO1_11 = 0x41;
                    LightCheck();
                }
            }
        };

        if(GetVoltage()<3600){
            IOCON_PIO1_11 = 0x0;
            gpioSetDir(RB_LED3, gpioDirection_Output);
            if( (ctr/(50/SYSTICKSPEED))%10 == 1 )
                gpioSetValue (RB_LED3, 1);
            else
                gpioSetValue (RB_LED3, 0);
        };
    };

    EVERY(4096,17){
        push_queue(nrf_check_reset);
    };
    return;
};
