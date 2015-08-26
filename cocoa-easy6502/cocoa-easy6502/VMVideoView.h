//
//  VMVideoView.h
//  cocoa-easy6502
//
//  Created by Daniel Loffgren on H.25/06/04.
//  Copyright (c) 平成25年 Hello-Channel, LLC. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import <v6502/mem.h>

@protocol VMVideoViewDelegate <NSObject>

- (void) update;

@end

@interface VMVideoView : NSView {
	v6502_memory *mem;
	uint16_t selectedPixel;
	id <VMVideoViewDelegate> delegate;
}

@property (readwrite) uint64_t lastRefresh;
@property (readwrite) v6502_memory *mem;
@property (readonly) uint16_t selectedPixel;
@property (nonatomic, assign) id <VMVideoViewDelegate> delegate;

- (void) testPattern;
- (void) resetVideoMemory;

@end
