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

void textMode_updateOffset(v6502_textmode_video *vid, uint16_t address) {
	if (!vid->screen) {
		vid->screen = initscr();
	}
	
	char ch = vid->memory->bytes[address];
	address -= textMode_characterMemoryStart;
	
	int x = address % 80;
	int y = (address - x) / 80;
	
	if (ch) {
		wmove(vid->screen, y, x);
		waddch(vid->screen, ch);
	}
}

void textMode_write(v6502_memory *memory, uint16_t offset, uint8_t value, void *context) {
	v6502_textmode_video *vid = context;
	vid->memory->bytes[offset] = value;
	textMode_updateOffset(vid, offset);
	wrefresh(vid->screen);
}

v6502_textmode_video *textMode_create(v6502_memory *mem) {
	v6502_textmode_video *vid = malloc(sizeof(v6502_textmode_video));
	vid->screen = NULL;
	vid->memory = mem;
	v6502_map(mem, textMode_characterMemoryStart, textMode_memoryCeiling - textMode_characterMemoryStart, NULL, textMode_write, vid);
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
	char ch = vid->memory->bytes[address];
	
	if (ch) {
		wmove(vid->screen, y, x);
		waddch(vid->screen, ch);
	}
}


uint16_t textMode_addressForLocation(int x, int y) {
	return textMode_characterMemoryStart + ((y * 80) + x);
}
