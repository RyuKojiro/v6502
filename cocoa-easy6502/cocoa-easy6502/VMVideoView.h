//
//  VMVideoView.h
//  cocoa-easy6502
//
//  Created by Daniel Loffgren on H.25/06/04.
//  Copyright (c) 平成25年 Hello-Channel, LLC. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "mem.h"

@interface VMVideoView : NSView {
	v6502_memory *mem;
}

@property (readwrite) v6502_memory *mem;

@end
