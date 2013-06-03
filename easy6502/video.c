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
	return initscr();
}

void updateVideo(v6502_memory *mem, WINDOW *win) {
	int w = MAX(getmaxx(win), 32);
	int h = MAX(getmaxy(win), 32);
	uint8_t byte;

	for (int x = 0; x < w; x++) {
		for (int y = 0; y < h; y++) {
			byte = mem->bytes[x * w + y + VIDEO_OFFSET];
			if (byte) {
				mvwaddch(win, x, y, byte);
			}
		}
	}
	
	refresh();
}