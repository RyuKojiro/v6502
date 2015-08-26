//
//  VMVideoView.m
//  cocoa-easy6502
//
//  Created by Daniel Loffgren on H.25/06/04.
//  Copyright (c) 平成25年 Hello-Channel, LLC. All rights reserved.
//

#include <mach/mach_time.h>
#include <CoreServices/CoreServices.h>

#import "VMVideoView.h"

#define VIDEO_OFFSET			0x0200
#define VIDEO_WIDTH				32
#define VIDEO_HEIGHT			32
#define VIDEO_PIXELCOUNT		(VIDEO_WIDTH * VIDEO_HEIGHT)
#define VIDEO_REFRESH_PERIOD	3.333E7 // ns (30Hz)

@implementation VMVideoView
@synthesize mem, selectedPixel, delegate;

#pragma mark - Callback Support and Memory Mapping

void video_writeCallback(v6502_memory *memory, uint16_t offset, uint8_t value, void *context) {
	// TODO: In the future this should only update the single pixel that changed.
	memory->bytes[offset] = value;
	
	VMVideoView *self = context;
	
	// This throttles -setNeedsDisplay:, since it's the real CPU hog
	uint64_t t = mach_absolute_time();
	uint64_t diff = t - self.lastRefresh;
	Nanoseconds elapsed = AbsoluteToNanoseconds(*(AbsoluteTime *)&diff);
	if ((*(uint64_t *)&elapsed) > VIDEO_REFRESH_PERIOD) {
		self.lastRefresh = t;
		[self setNeedsDisplay:YES];
	}
}

- (void) setMem:(v6502_memory *)m {
	mem = m;
	
	// Enable map caching
	mem->mapCacheEnabled = YES;

	v6502_map(mem, VIDEO_OFFSET, VIDEO_PIXELCOUNT, NULL, video_writeCallback, self);
}

- (v6502_memory *)mem {
	return mem;
}

#pragma mark - Object Lifecycle

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
	for (uint16_t addr = VIDEO_OFFSET; addr < VIDEO_OFFSET + VIDEO_PIXELCOUNT; addr++) {
		mem->bytes[addr] = v++;
	}
}

- (void) resetVideoMemory {
	for (uint16_t addr = VIDEO_OFFSET; addr < VIDEO_OFFSET + VIDEO_PIXELCOUNT; addr++) {
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
		
	NSPoint point;
	for (point.x = 0; point.x < VIDEO_WIDTH; point.x++) {
		for (point.y = 0; point.y < VIDEO_HEIGHT; point.y++) {
			byte = mem->bytes[addressForRawPoint(point)];
			if (byte) {
				[VMVideoView setColorForByte:byte];
				NSRectFill([self rectForRawPoint:point]);
			}
		}
	}
	
	if (selectedPixel) {
		NSPoint selectedPoint = rawPointForAddress(selectedPixel);
		NSRect rect = [self rectForRawPoint:selectedPoint];
		[[NSColor magentaColor] setStroke];
		[NSBezierPath setDefaultLineWidth:2.0f];
		[NSBezierPath setDefaultLineJoinStyle:NSMiterLineJoinStyle];
		[NSBezierPath strokeRect:rect];
	}
	
    [super drawRect:dirtyRect];
}

static NSPoint rawPointForAddress(uint16_t address) {
	NSPoint rawPoint;
	address -= VIDEO_OFFSET;
	
	rawPoint.x = address % 32;
	rawPoint.y = (31 - (address / 32));
	
	return rawPoint;
}

static uint16_t addressForRawPoint(NSPoint rawPoint) {
	return (VIDEO_OFFSET + (rawPoint.y * 32) + rawPoint.x);
}

- (NSPoint) rawPointForPointInView:(NSPoint)pointInView {
	NSPoint rawPoint;
	CGFloat scale = self.frame.size.width / 32.0f;

	rawPoint.x = floor(pointInView.x / scale);
	rawPoint.y = floor(pointInView.y / scale);
	
	return rawPoint;
}

- (NSRect) rectForRawPoint:(NSPoint)rawPoint {
	CGFloat scale = self.frame.size.width / 32.0f;
	return NSMakeRect(floor(rawPoint.x * scale), floor((31 - rawPoint.y) * scale), ceil(scale), ceil(scale));
}

- (void) updateSelectedPixelWithPoint:(NSPoint)loc {
	selectedPixel = addressForRawPoint([self rawPointForPointInView:loc]);
	[self setNeedsDisplay:YES];
	
	if (delegate && [delegate respondsToSelector:@selector(update)]) {
		[delegate update];
	}
}

- (void) mouseDragged:(NSEvent *)theEvent {
	NSPoint loc = [self convertPoint:[theEvent locationInWindow] fromView:nil];
	if (NSPointInRect(loc, self.bounds)) {
		[self updateSelectedPixelWithPoint:loc];
	}
}

- (void)mouseDown:(NSEvent *)theEvent {
	NSPoint loc = [self convertPoint:[theEvent locationInWindow] fromView:nil];
	[self updateSelectedPixelWithPoint:loc];
}

@end
