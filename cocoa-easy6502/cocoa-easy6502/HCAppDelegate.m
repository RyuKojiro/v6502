//
//  HCAppDelegate.m
//  cocoa-easy6502
//
//  Created by Daniel Loffgren on H.25/06/04.
//  Copyright (c) 平成25年 Hello-Channel, LLC. All rights reserved.
//

#import "HCAppDelegate.h"
#import "VMWindowController.h"

@implementation HCAppDelegate {
	VMWindowController *vmwc;
}

- (void)dealloc
{
	[vmwc release];
    [super dealloc];
}

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
	// Insert code here to initialize your application
	vmwc = [[VMWindowController alloc] initWithWindowNibName:@"VMWindowController"];
	[[vmwc window] display];
}

@end
