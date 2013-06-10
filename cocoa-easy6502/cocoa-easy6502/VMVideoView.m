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
		case 0: {
			[[NSColor blackColor] setFill];
		} return;
		case 1: {
			[[NSColor whiteColor] setFill];
		} return;
		case 2: {
			[[NSColor redColor] setFill];
		} return;
		case 3: {
			[[NSColor cyanColor] setFill];
		} return;
		case 4: {
			[[NSColor purpleColor] setFill];
		} return;
		case 5: {
			[[NSColor greenColor] setFill];
		} return;
		case 6: {
			[[NSColor blueColor] setFill];
		} return;
		case 7: {
			[[NSColor yellowColor] setFill];
		} return;
		case 8: {
			[[NSColor orangeColor] setFill];
		} return;
		case 9: {
			[[NSColor brownColor] setFill];
		} return;
		default:
			break;
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
	
	for (int x = 0; x < dirtyRect.size.width; x++) {
		for (int y = 0; y < dirtyRect.size.height; y++) {
			byte = mem->bytes[x * (int)dirtyRect.size.width + y + VIDEO_OFFSET];
			if (byte) {
				[VMVideoView setColorForByte:byte];
				NSRectFill(NSMakeRect(x * scale, y * scale, scale, scale));
			}
		}
	}

	
    [super drawRect:dirtyRect];
}
@end
