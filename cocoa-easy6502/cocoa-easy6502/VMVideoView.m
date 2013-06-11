//
//  VMVideoView.m
//  cocoa-easy6502
//
//  Created by Daniel Loffgren on H.25/06/04.
//  Copyright (c) 平成25年 Hello-Channel, LLC. All rights reserved.
//

#import "VMVideoView.h"

#define VIDEO_OFFSET	0x0200

@implementation VMVideoView
@synthesize mem;

- (id)initWithFrame:(NSRect)frame
{
    self = [super initWithFrame:frame];
    if (self) {
        // Initialization code here.
		NSLayoutConstraint *square = [NSLayoutConstraint constraintWithItem:self attribute:NSLayoutAttributeHeight relatedBy:0 toItem:self attribute:NSLayoutAttributeWidth multiplier:1 constant:0];
		[self addConstraint:square];
    }
    
    return self;
}

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

+ (void)setColorForByte:(uint8_t)byte {
	byte &= 0xf;
	switch (byte) {
		case 0x00: {
			[[NSColor blackColor] setFill];
		} return;
		case 0x01: {
			[[NSColor whiteColor] setFill];
		} return;
		case 0x02: {
			[[NSColor redColor] setFill];
		} return;
		case 0x03: {
			[[NSColor cyanColor] setFill];
		} return;
		case 0x04: {
			[[NSColor purpleColor] setFill];
		} return;
		case 0x05: {
			[[NSColor greenColor] setFill];
		} return;
		case 0x06: {
			[[NSColor blueColor] setFill];
		} return;
		case 0x07: {
			[[NSColor yellowColor] setFill];
		} return;
		case 0x08: {
			[[NSColor orangeColor] setFill];
		} return;
		case 0x09: {
			[[NSColor brownColor] setFill];
		} return;
		case 0x0a: {
			[[NSColor colorWithDeviceRed:1.0f green:0.5f blue:0.5f alpha:1.0f] setFill];
		} return;
		case 0x0b: {
			[[NSColor darkGrayColor] setFill];
		} return;
		case 0x0c: {
			[[NSColor grayColor] setFill];
		} return;
		case 0x0d: {
			[[NSColor colorWithDeviceRed:0.5f green:1.0f blue:0.5f alpha:1.0f] setFill];
		} return;
		case 0x0e: {
			[[NSColor colorWithDeviceRed:0.5f green:0.5f blue:1.0f alpha:1.0f] setFill];
		} return;
		case 0x0f: {
			[[NSColor lightGrayColor] setFill];
		} return;
		default:
			break;
	}
}

- (void) testPattern {
	char v = 0;
	for (uint16_t addr = VIDEO_OFFSET; addr < VIDEO_OFFSET + (32 * 32); addr++) {
		mem->bytes[addr] = v++;
	}
}

- (void) resetVideoMemory {
	for (uint16_t addr = VIDEO_OFFSET; addr < VIDEO_OFFSET + (32 * 32); addr++) {
		mem->bytes[addr] = 0;
	}
}

- (void)drawRect:(NSRect)dirtyRect {
    // set any NSColor for filling, say white:
    [[NSColor blackColor] setFill];
    NSRectFill(dirtyRect);
	
	if (!mem) {
		return;
	}
	
	uint8_t byte;
	CGFloat scale = dirtyRect.size.width / 32.0f;
	
	for (int x = 0; x < 32; x++) {
		for (int y = 0; y < 32; y++) {
			byte = mem->bytes[VIDEO_OFFSET + (y * 32) + x];
			if (byte) {
				[VMVideoView setColorForByte:byte];
				NSRectFill(NSMakeRect(floor(x * scale), floor((31 - y) * scale), ceil(scale), ceil(scale)));
			}
		}
	}

	
    [super drawRect:dirtyRect];
}
@end
