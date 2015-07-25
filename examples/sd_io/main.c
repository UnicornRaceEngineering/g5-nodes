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
#include <avr/pgmspace.h>
#include <mmc_sdcard.h>  // for SD_BLOCKSIZE, get_memory_capacity, sd_init, etc
#include <stdint.h>      // for uint8_t, uint32_t
#include <stdio.h>       // for printf, putchar, getchar, scanf
#include <usart.h>       // for usart1_init
#include <util/delay.h>
#include <utils.h>       // for ARR_LEN

static uint8_t buf_in[64];
static uint8_t buf_out[64];

uint32_t sector = 0;

#define uart_raw(c) do { \
	loop_until_bit_is_set(UCSR1A, UDRE0); \
    UDR1 = c; \
} while (0)

static void init(void) {
	usart1_init(115200, buf_in, ARR_LEN(buf_in), buf_out, ARR_LEN(buf_out));
	sei();
	printf("Running init\n");
	int rc = sd_init();


	if (rc == 0) {
		puts_P(PSTR("Init complete\n\n"));
	} else {
		puts_P(PSTR("\n\n\nInit failed\n\n"));
		// Hard_reset();
	}
}

void read(void) {
	printf("\nrecv:\n");
	uint8_t buf[SD_BLOCKSIZE*2] = {'\0'};
	if (sd_read(buf, sector, ARR_LEN(buf)/SD_BLOCKSIZE) != 0) printf("ERROR: read error\n");
	printf("%s\n", buf);
	printf("\nend recv\n");
}

void write(void) {
	printf("\nwriting\n");
	uint8_t buf[SD_BLOCKSIZE*2] = {'\0'};
	{
		int i = 0;
		char c;
		do {
			c = getchar();
			putchar(c);
			buf[i++] = c;
		} while (c != '\r');
	}
	if (sd_write(buf, sector, ARR_LEN(buf)/SD_BLOCKSIZE) != 0) printf("ERROR: write error\n");
	printf("\nend write\n");
}

void capacity(void) {
	putchar('\n');
	uint32_t cap = 0;
	if (get_memory_capacity(&cap) != 0) {
		printf("ERROR reading Card capacity\n");
	} else {
		printf("Card capacity: %u MiB\n", (unsigned)(cap / 1024));
	}
}

void change_sector(void) {
	scanf("%lu", &sector);
	printf("\nchanged to sector %lu\n", sector);
}

int main(void) {
	// _delay_ms(1500);
	init();

	// uint8_t i = 0;
	while (1) {
		char c = getchar();
		if (c == '\r') putchar('\n');
		putchar(c);

		switch (c) {
		case 'R':
		case 'r': read(); 	break;

		case 'W':
		case 'w': write(); 	break;

		case 'C':
		case 'c': capacity(); break;

		case 'I':
		case 'i': init(); break;

		case 'S':
		case 's': change_sector();

		// case '?': print_help(); break;
		}

	}

	return 0;
}
