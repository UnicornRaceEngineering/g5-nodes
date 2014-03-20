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

#include <stdlib.h> // size_t
#include <avr/interrupt.h> // sei()
#include <usart.h>

char buf[256] = {'\0'};
size_t bufIndex = 0;

int main(void) {
	usart1_init(115200);						//Serial communication

	sei();										//Enable interrupt

	usart1_printf("\n\n\nSTARTING\n");

	while(1){
		// Main work loop
		if (usart1_hasData()) {
			char c = usart1_getc();
			buf[bufIndex] = c;
			if (c == '\n') {
				buf[bufIndex+1] = '\0';
				usart1_printf("s", buf); // Echo the string back
				bufIndex = 0;
			}
		}
	}

    return 0;
}
