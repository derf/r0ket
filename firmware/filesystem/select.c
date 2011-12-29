#include <string.h>
#include "sysdefs.h"
#include "lcd/lcd.h"
#include "lcd/fonts/smallfonts.h"
#include "lcd/print.h"
#include "filesystem/ff.h"
#include "basic/basic.h"

/* XXX executeSelect has a 15 byte buffer. 0: + filename + \0 */
#define FLEN 12

/* retrieve filelist.
 * not (yet) sorted.
 * flen+2 because optional slash and nullbyte
 */
static int retrieve_files(char files[][FLEN+2],
                          uint8_t count, uint16_t skip,
                          char *ext,
                          char *cwd,
                          int *total) {
	if (strlen(cwd) > FLEN) {
		lcdPrintln("cwd too long");
		lcdRefresh();
		return 0;
	}

	char path[FLEN+3];
	path[0] = '0';
	path[1] = ':';
	strcpy(path + 2, cwd);
	path[2 + strlen(cwd)] = 0;

	/* remove trailing slash, otherwise INVALID_NAME */
	if (path[2 + strlen(cwd) - 1] == '/') {
		path[2 + strlen(cwd) - 1] = 0;
	}

	DIR dir;
	FILINFO fileinfo;
	FRESULT res;

	res = f_opendir(&dir, path);
	if (res) {
		lcdPrintln("E[f_opendir]");
		lcdPrintln(path);
		lcdPrintln(f_get_rc_string(res));
		lcdRefresh();
		return 0;
	}

	int pos = 0;
	int extlen = strlen(ext);
	if (total) {
		(*total) = 0;
	}
	while (f_readdir(&dir, &fileinfo) == FR_OK && fileinfo.fname[0]) {
		int len = strlen(fileinfo.fname);

		if (extlen && !(fileinfo.fattrib & AM_DIR)) {
			if (len < extlen) {
				continue;
			}

			/* fixme, proper ext check */
			if (strcmp(fileinfo.fname + len - extlen, ext) != 0) {
				continue;
			}
		}

		if (total) {
			(*total)++;
		}

		if (skip > 0) {
			skip--;
			continue;
		}

		if (pos < count) {
			strcpy(files[pos], fileinfo.fname);
			if (fileinfo.fattrib & AM_DIR) {
				files[pos][len]     = '/';
				files[pos][len + 1] = 0;
			}
			pos++;
		}
	}

	return pos;
}

#define PERPAGE 7
int selectFile(char *filename, char *extension)
{
    int skip = 0;
    int selected = 0;
    char pwd[FLEN];
    memset(pwd, 0, FLEN);
    font=&Font_7x8;

	while(1) {
		int total;
		char files[PERPAGE][FLEN+2];
		int count = retrieve_files(files, PERPAGE, skip, extension, pwd, &total);
		if (selected >= count) {
			selected = count - 1;
		}

		/* optimization: don't reload filelist if only redraw needed */
		redraw:

		lcdClear();
		lcdPrint("[");
		lcdPrint(pwd);
		lcdPrintln("]");

		for (int i = 0; i < 98; i++) {
			lcdSetPixel(i, 9, 1);
		}

		lcdSetCrsr(0, 12);

		if (!count) {
			lcdPrintln("- empty");
		} else {
			for (int i = 0; i < count; i++) {
				if (selected == i) {
					lcdPrint("*");
				} else {
					lcdPrint(" ");
				}
				lcdSetCrsrX(14);

				lcdPrintln(files[i]);
			}
		}
		lcdRefresh();

		char key = getInputWaitRepeat();
		switch (key) {
		case BTN_DOWN:
			if (selected < count - 1) {
				selected++;
				goto redraw;
			} else {
				if (skip < total - PERPAGE) {
					skip++;
				} else {
					skip = 0;
					selected = 0;
					continue;
				}
			}
			break;
		case BTN_UP:
			if (selected > 0) {
				selected--;
				goto redraw;
			} else {
				if (skip > 0) {
					skip--;
				} else {
					skip = total - PERPAGE;
					if (skip < 0) {
						skip = 0;
					}
					selected = PERPAGE - 1;
					continue;
				}
			}
			break;
		case BTN_LEFT:
			if (pwd[0] == 0) {
				return -1;
			} else {
				pwd[0] = 0;
				continue;
			}
		case BTN_ENTER:
		case BTN_RIGHT:
			if (count) {
				if (files[selected][strlen(files[selected]) - 1] == '/') {
					/* directory selected */
					strcpy(pwd, files[selected]);
					continue;
				} else {
					strcpy(filename, pwd);
					strcpy(filename + strlen(pwd), files[selected]);
					getInputWaitRelease(); /* ?! */
					return 0;
				}
			}
		}
	}
}
