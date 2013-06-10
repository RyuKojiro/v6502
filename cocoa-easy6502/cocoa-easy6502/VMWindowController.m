//
//  VMWindowController.m
//  cocoa-easy6502
//
//  Created by Daniel Loffgren on H.25/06/04.
//  Copyright (c) 平成25年 Hello-Channel, LLC. All rights reserved.
//

#import "VMWindowController.h"
#import "cpu.h"
#import "mem.h"

volatile static int faulted;

@interface VMWindowController ()

@end

@implementation VMWindowController
@synthesize video;
@synthesize pcField, acField, xField, yField, spField, srField, toggleButton;

- (id)initWithWindow:(NSWindow *)window
{
    self = [super initWithWindow:window];
    if (self) {
        // Initialization code here.
    }
    
    return self;
}

static void v6502_fault(const char *reason) {
	faulted++;
}

void loadProgram(v6502_memory *mem, const char *fname) {
	FILE *f = fopen(fname, "r");
	uint8_t byte;
	uint8_t offset = 0;
	
	while (fread(&byte, 1, 1, f)) {
		mem->bytes[0x600 + (offset++)] = byte;
	}
		
	fclose(f);
}

- (void) update {
	[pcField setStringValue:[NSString stringWithFormat:@"0x%04x", cpu->pc]];
	[acField setStringValue:[NSString stringWithFormat:@"0x%02x", cpu->ac]];
	[xField setStringValue:[NSString stringWithFormat:@"0x%02x", cpu->x]];
	[yField setStringValue:[NSString stringWithFormat:@"0x%02x", cpu->y]];
	[spField setStringValue:[NSString stringWithFormat:@"0x%02x", cpu->sp]];
	[srField setStringValue:[NSString stringWithFormat:@"%c%c%c%c%c%c%c%c",
							 cpu->sr & v6502_cpu_status_negative ? 'N' : '-',
							 cpu->sr & v6502_cpu_status_overflow ? 'V' : '-',
							 cpu->sr & v6502_cpu_status_ignored ? 'X' : '-',
							 cpu->sr & v6502_cpu_status_break ? 'B' : '-',
							 cpu->sr & v6502_cpu_status_decimal ? 'D' : '-',
							 cpu->sr & v6502_cpu_status_interrupt ? 'I' : '-',
							 cpu->sr & v6502_cpu_status_zero ? 'Z' : '-',
							 cpu->sr & v6502_cpu_status_carry ? 'C' : '-']];
}

- (void) cycle {
	// Update keypress byte, hold for clock cycle, and refresh random byte
	//mem->bytes[0xff] = (uint8_t)getch();
	cpu->memory->bytes[0xfe] = (uint8_t)arc4random();

	// Processor time
	v6502_step(cpu);
	
	// Exit on break
	if (cpu->sr & v6502_cpu_status_break) {
		return;
	}
	
	// Update fields
	[self update];
	
	// Refresh Video
	[video setNeedsDisplay:YES];
	
	if (!faulted) {
		[self performSelector:_cmd withObject:nil afterDelay:0.1f];
	}
}

- (IBAction)reset:(id)sender {
	v6502_reset(cpu);
}

- (IBAction)toggleRunning:(id)sender {
	if (faulted) {
		[toggleButton setTitle:@"Halt"];
		faulted = 0;
		[self cycle];
	}
	else {
		[toggleButton setTitle:@"Run"];
		faulted++;
	}
}

- (IBAction)step:(id)sender {
	[self cycle];
}

- (void)windowDidLoad
{
    [super windowDidLoad];
    
    // Implement this method to handle any initialization after your window controller's window has been loaded from its nib file.
	cpu = v6502_createCPU();
	cpu->memory = v6502_createMemory(2048);
	[video setMem:cpu->memory];
	
	// Load program code into memory
	loadProgram(cpu->memory, "/Users/kojiro/Code/v6502/easy6502/easy_test.o");
	
	// Reset the cpu, wait for user to start it
	v6502_reset(cpu);
}

@end
