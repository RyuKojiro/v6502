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

#ifndef v6502_textmode_h
#define v6502_textmode_h

#include <curses.h>

#include "mem.h"

#define textMode_memoryStart	0x2000

typedef struct {
	WINDOW *screen;
	v6502_memory *memory;
} v6502_textmode_video;

v6502_textmode_video *textMode_create(v6502_memory *mem);
void textMode_destroy(v6502_textmode_video *vid);
void textMode_rest(v6502_textmode_video *vid);
void textMode_refreshVideo(v6502_textmode_video *vid);
void textMode_updateCharacter(v6502_textmode_video *vid, int x, int y);
uint16_t textMode_addressForLocation(int x, int y);

#endif