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
				NSRectFill(NSMakeRect(x * scale, y * scale, scale, scale));
			}
		}
	}

	
    [super drawRect:dirtyRect];
}
@end
