/*
 * Copyright (c) 2013 Daniel Loffgren
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <stdlib.h>

#include "textmode.h"

v6502_textmode_video *textMode_create(v6502_memory *mem) {
	v6502_textmode_video *vid = malloc(sizeof(v6502_textmode_video));
	vid->memory = mem;
	return vid;
}

void textMode_destroy(v6502_textmode_video *vid) {
	endwin();
	free(vid);
}

void textMode_rest(v6502_textmode_video *vid) {
	vid->screen = NULL;
	endwin();
}


void textMode_refreshVideo(v6502_textmode_video *vid) {
	if (!vid->screen) {
		vid->screen = initscr();
	}
	
	for (int y = 0; y < 24; y++) {
		for (int x = 0; x < 80; x++) {
			textMode_updateCharacter(vid, x, y);
			wrefresh(vid->screen);
		}
	}
}

void textMode_updateCharacter(v6502_textmode_video *vid, int x, int y) {
	uint16_t address = textMode_addressForLocation(x, y);
	char ch = *v6502_map(vid->memory, address);
	
	if (ch) {
		wmove(vid->screen, y, x);
		waddch(vid->screen, ch);
	}
}


uint16_t textMode_addressForLocation(int x, int y) {
	return textMode_memoryStart + ((y * 80) + x);
}
