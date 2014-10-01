/** @brief 6502 Reference Platform Virtual Terminal */
/** @file textmode.h */

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

/** @brief The start address of character data in memory */
#define textMode_characterMemoryStart	0x2000
/** @brief The start address of attribute data in memory */
#define textMode_attributeMemoryStart	0x3000
/** @brief The upper bounds of memory reserved for terminal hardware */
#define textMode_memoryCeiling			0x4000

/** The first two kilobytes of memory (starting at textMode_characterMemoryStart or 0x2000) are character data, starting with the top right, ending with the bottom left, one row at a time, with no interruptions. After a short break for video hardware registers, another 2 kilobytes (starting at textMode_attributeMemoryStart or 0x3000) are attribute data, which correspond to each byte of character data + 0x1000.
 
	Attribute data byte anatomy:
	B = Bright C = Color data
	<- fg -><- bg ->
	B C C C  B C C C
 */

/** @defgroup textmode Reference Platform Video/Keyboard */
/**@{*/
/** @struct */
/** @brief Virtual Text-Mode Hardware Object */
typedef struct {
	/** @brief Curses output object */
	WINDOW *screen;
	/** @brief Hardwired memory used to trap video activity and report keyboard input */
	v6502_memory *memory;
} v6502_textmode_video;

/** @brief Create v6502_textmode_video */
v6502_textmode_video *textMode_create(v6502_memory *mem);
/** @brief Destroy v6502_textmode_video */
void textMode_destroy(v6502_textmode_video *vid);
/** @brief Put virtual video into "rest" mode, where the hosting terminal is restored, but the video data is preserved and will be redisplayed on next access */
void textMode_rest(v6502_textmode_video *vid);
/** @brief Force a fullscreen refresh */
void textMode_refreshVideo(v6502_textmode_video *vid);
/** @brief Refresh a single character of the output (When memory is trapped, this is called automatically on memory interaction) */
void textMode_updateCharacter(v6502_textmode_video *vid, int x, int y);
/** @brief Convert x, y coordinates to the address in memory that is expected to hold the character */
uint16_t textMode_addressForLocation(int x, int y);

#endif
