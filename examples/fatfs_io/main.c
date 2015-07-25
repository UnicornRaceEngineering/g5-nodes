/*
The MIT License (MIT)

Copyright (c) 2014 UnicornRaceEngineering

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
the Software, and to permit persons to whom the Software is furnished to do so,
subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


#include <avr/interrupt.h>
#include <fatfs/ff.h>  // for f_open, f_close, f_write, FA_CREATE_ALWAYS, etc
#include <stdint.h>    // for uint8_t
#include <stdio.h>     // for printf, putchar, scanf, getchar
#include <string.h>    // for strlen, memset
#include <usart.h>     // for usart1_init

#include "utils.h"     // for ARR_LEN

static uint8_t buf_in[64];
static uint8_t buf_out[64];

static FATFS fs;
static FIL file;

static FRESULT fr;

void print_fr(FRESULT fr) {
	switch (fr) {
		case FR_OK: 			printf("OK\n"); break;
		case FR_DISK_ERR: 		printf("Disk error\n"); break;
		case FR_INT_ERR:		printf("Assertion failed\n"); break;
		case FR_NOT_READY:		printf("Disk not ready\n"); break;
		case FR_NO_FILE:		printf("No such file\n"); break;
		case FR_NO_PATH:		printf("No such path\n"); break;
		case FR_INVALID_NAME: 	printf("Invalid name\n"); break;
		case FR_DENIED:			printf("Access denied\n"); break;
		case FR_EXIST:			printf("File exists\n"); break;
		case FR_INVALID_OBJECT: printf("Invalid structure\n"); break;
		case FR_WRITE_PROTECTED:printf("Write protected\n"); break;
		case FR_INVALID_DRIVE:	printf("Invalid drive\n"); break;
		case FR_NOT_ENABLED:	printf("Not enabled\n"); break;
		case FR_NO_FILESYSTEM:  printf("Disk does not contain a FAT fs\n"); break;
		case FR_MKFS_ABORTED:	printf("mkfs aborted\n"); break;
		case FR_TIMEOUT: 		printf("Timeout\n"); break;
		case FR_LOCKED:         printf("Object locked\n"); break;
		case FR_NOT_ENOUGH_CORE:printf("Not enough memory\n"); break;
		case FR_TOO_MANY_OPEN_FILES: printf("Too many open files \n"); break;
		case FR_INVALID_PARAMETER: printf("Invalid parameter\n"); break;

		default: printf("Unknown error\n");
	}
}

static void init(void) {
	usart1_init(115200, buf_in, ARR_LEN(buf_in), buf_out, ARR_LEN(buf_out));
	sei();
}

void open(void) {
	printf("Enter file to open\n");
	char file_name[32] = {'\0'};
	scanf("%s", file_name);

	fr = f_open(&file, file_name, FA_WRITE | FA_READ | FA_CREATE_ALWAYS);
	printf("Opened \"%s\" ", file_name);
	print_fr(fr);
}

void write(void) {
	char buf[1024] = {'\0'};
	scanf("%s", buf);
	unsigned len = strlen(buf);
	unsigned bw;

	fr = f_write(&file, buf, len, &bw);
	printf("Wrote %u/%u bytes", bw, len);
	print_fr(fr);
}

void read(void) {
	printf("Enter how much to read");
	unsigned len;
	scanf("%u", &len);

	char buf[len+1];
	memset(buf, 0, len+1);

	unsigned br;

	f_lseek(&file, 0);
	fr = f_read(&file, buf, len, &br);
	printf("\n%s\nRead %u/%u ", buf, br, len);
	print_fr(fr);
}


void test(void) {
	printf("Opening file\n");
	fr = f_open(&file, "newfile.txt", FA_WRITE | FA_CREATE_ALWAYS);
	printf("Opened file ");
	print_fr(fr);

	unsigned bw;
	fr = f_write(&file, "It works!\r\n", 11, &bw);
	printf("write %u/%u ", bw, 11);
	print_fr(fr);

	f_close(&file);
}

void mount(void) {
	fr = f_mount(&fs, "", 1);
	printf("Mounted ");
	print_fr(fr);
}
int main(void) {
	init();
	printf("\n\n\nInit finished\n");

	mount();

	char *file_name = "test.txt";
	printf("Opening %s\n", file_name);
	fr = f_open(&file, file_name, FA_READ | FA_WRITE | FA_CREATE_ALWAYS);
	printf("Opened \"%s\" ", file_name);
	print_fr(fr);

	while (1) {
		char c = getchar();
		if (c == '\r') putchar('\n');
		putchar(c);
		putchar('\n');

		switch (c) {
			case 'o': open(); break;
			case 'w': write(); break;
			case 'r': read(); break;
			case 'm': mount(); break;

			case 't': test(); break;

			case 's':
				printf("Syncing\n");
				fr = f_sync(&file);
				printf("Done syncing");
				print_fr(fr);
				break;

			case 'c':
				printf("Closing file\n");
				fr = f_close(&file);
				print_fr(fr);
				break;
		}

	}

	return 0;
}
