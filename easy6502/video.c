//
//  video.c
//  v6502
//
//  Created by Daniel Loffgren on H.25/06/03.
//  Copyright (c) 平成25年 Hello-Channel, LLC. All rights reserved.
//

#include "video.h"

#include <stdint.h>

#define MAX(a, b)		(a > b ? a : b)
#define VIDEO_OFFSET	0x200

/*
 Memory locations $200 to $5ff map to the screen pixels. Different values will
 draw different colour pixels. The colours are:

 $0: Black
 $1: White
 $2: Red
 $3: Cyan
 $4: Purple
 $5: Green
 $6: Blue
 $7: Yellow
 $8: Orange
 $9: Brown
 $a: Light red
 $b: Dark grey
 $c: Grey
 $d: Light green
 $e: Light blue
 $f: Light grey
*/

WINDOW *initVideo() {
	WINDOW *win = initscr();
	noecho();
	return win;
}

char hex(uint8_t b) {
	b &= 0x0F;
	
	if (b < 0x0A) {
		return b + '0';
	}
	return b + 'A';
}

void updateVideo(v6502_memory *mem, WINDOW *win) {
	int w = MAX(getmaxx(win), 32);
	int h = MAX(getmaxy(win), 32);
	uint8_t byte;

	for (int x = 0; x < w; x++) {
		for (int y = 0; y < h; y++) {
			byte = mem->bytes[x * w + y + VIDEO_OFFSET];
			if (byte) {
				move(x, y);
				addch(hex(byte));
			}
		}
	}
	
	refresh();
}