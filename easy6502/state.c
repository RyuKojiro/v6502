//
//  state.c
//  v6502
//
//  Created by Daniel Loffgren on H.25/06/03.
//  Copyright (c) 平成25年 Hello-Channel, LLC. All rights reserved.
//

#include "state.h"

#include <curses.h>
#include <stdlib.h>

/*
 Memory location $fe contains a new random byte on every instruction.
 Memory location $ff contains the ascii code of the last key pressed.
*/

void initState() {
	timeout(100);
	keypad(stdscr, TRUE);
}

void stateCycle(v6502_memory *mem) {
	// Key
	mem->bytes[0xff] = (uint8_t)getch();
	
	// Random
	mem->bytes[0xfe] = (uint8_t)arc4random();
}