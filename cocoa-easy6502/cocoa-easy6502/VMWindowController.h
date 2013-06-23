//
//  VMWindowController.h
//  cocoa-easy6502
//
//  Created by Daniel Loffgren on H.25/06/04.
//  Copyright (c) 平成25年 Hello-Channel, LLC. All rights reserved.
//

#import <Cocoa/Cocoa.h>
#import "VMVideoView.h"
#import "cpu.h"

@interface VMWindowController : NSWindowController {
	v6502_cpu *cpu;
}

@property (nonatomic, retain) IBOutlet VMVideoView *video;

@property (nonatomic, retain) IBOutlet NSTextField *pcField;
@property (nonatomic, retain) IBOutlet NSTextField *acField;
@property (nonatomic, retain) IBOutlet NSTextField *xField;
@property (nonatomic, retain) IBOutlet NSTextField *yField;
@property (nonatomic, retain) IBOutlet NSTextField *spField;
@property (nonatomic, retain) IBOutlet NSTextField *srField;
@property (nonatomic, retain) IBOutlet NSTextField *instructionField;

@property (nonatomic, retain) IBOutlet NSButton *logCheckBox;

@property (nonatomic, retain) IBOutlet NSButton *toggleButton;

- (IBAction)reset:(id)sender;
- (IBAction)toggleRunning:(id)sender;
- (IBAction)step:(id)sender;
- (IBAction)testPattern:(id)sender;
- (IBAction)dumpMemory:(id)sender;

@end
