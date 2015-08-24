//
//  VMInputWindow.m
//  cocoa-easy6502
//
//  Created by Daniel Loffgren on H25/06/24.
//  Copyright (c) 平成25年 Hello-Channel, LLC. All rights reserved.
//

#import "VMInputWindow.h"

@implementation VMInputWindow

- (void) keyDown:(NSEvent *)theEvent {
	if (self.delegate && [(NSObject *)self.delegate respondsToSelector:@selector(keyDown:)]) {
		[self.delegate performSelector:@selector(keyDown:) withObject:theEvent];
	}
	[super keyDown:theEvent];
}

@end
