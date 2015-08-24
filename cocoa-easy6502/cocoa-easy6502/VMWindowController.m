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
#import "reverse.h"

#import <HIToolbox/Events.h>

#define DEFAULT_RESET_VECTOR	0x0600

volatile static int faulted;

@interface VMWindowController ()

@end

@implementation VMWindowController
@synthesize video;
@synthesize pcField, acField, xField, yField, spField, srField, instructionField, logCheckBox, toggleButton, pixelField;

- (id)initWithWindow:(NSWindow *)window
{
    self = [super initWithWindow:window];
    if (self) {
        // Initialization code here.
    }
    
    return self;
}

BOOL loadProgram(v6502_memory *mem, const char *fname) {
	FILE *f = fopen(fname, "r");
	uint8_t byte;
	uint16_t offset = 0;
	
	if (!f) {
		return NO;
	}
	
	while (fread(&byte, 1, 1, f)) {
		mem->bytes[0x600 + (offset++)] = byte;
	}
		
	fclose(f);
	return YES;
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

	[pixelField setStringValue:[NSString stringWithFormat:@"0x%02x", video.selectedPixel]];
	
	char instruction[32];
	dis6502_stringForInstruction(instruction, 32, cpu->memory->bytes[cpu->pc], cpu->memory->bytes[cpu->pc + 2], cpu->memory->bytes[cpu->pc + 1]);
	[instructionField setStringValue:[NSString stringWithCString:instruction encoding:NSASCIIStringEncoding]];
	
	if ([logCheckBox state] == NSOnState) {
		NSLog(@"0x%04x: %s", cpu->pc, instruction);
	}
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
			
	if (!faulted) {
		[self performSelector:_cmd withObject:nil afterDelay:0.0001f];
	}
	else {
		[toggleButton setTitle:@"Run"];
	}
}

- (void) refresh {
	// Refresh Video
	[video setNeedsDisplay:YES];
	
	// Update fields
	[self update];

	if (!faulted && !(cpu->sr & v6502_cpu_status_break)) {
		[self performSelector:_cmd withObject:nil afterDelay:0.02f];
	}
}

- (IBAction)reset:(id)sender {
	v6502_reset(cpu);
	[self update];
}

- (IBAction)testPattern:(id)sender {
	[video testPattern];
	[video setNeedsDisplay:YES];
}

- (IBAction)toggleRunning:(id)sender {
	if (faulted) {
		[toggleButton setTitle:@"Halt"];
		faulted = 0;
		[self cycle];
		[self refresh];
	}
	else {
		[toggleButton setTitle:@"Run"];
		faulted++;
	}
}

- (IBAction)step:(id)sender {
	[self cycle];
	[self refresh];
}

- (IBAction)dumpMemory:(id)sender {
	uint16_t end = 0x200;
		
	printf("      0  1  2  3  4  5  6  7  8  9  A  B  C  D  E  F\n");
	for (uint16_t y = 0; y < end; y += 0x10) {
		printf("%04x ", y);
		for (uint16_t x = y; x <= y + 0x0F; x++) {
			printf("%02x ", cpu->memory->bytes[x]);
		}
		printf("\n");
	}
}

- (void)windowDidLoad
{
    [super windowDidLoad];
    
    // Implement this method to handle any initialization after your window controller's window has been loaded from its nib file.
	cpu = v6502_createCPU();
	cpu->memory = v6502_createMemory(v6502_memoryStartCeiling);
	[video setMem:cpu->memory];
	[video setDelegate:self];
	
	// Load program code into memory
	if (!loadProgram(cpu->memory, "/Users/kojiro/Code/v6502/tests/snake.o")) {
		NSAlert *alert = [NSAlert alertWithMessageText:@"Failed to open" defaultButton:@"OK" alternateButton:nil otherButton:nil informativeTextWithFormat:@"There was a problem trying to open the file."];
		[alert beginSheetModalForWindow:self.window completionHandler:nil];
	}
	
	// Set the reset vector
	v6502_write(cpu->memory, v6502_memoryVectorResetLow, DEFAULT_RESET_VECTOR & 0xFF);
	v6502_write(cpu->memory, v6502_memoryVectorResetHigh, DEFAULT_RESET_VECTOR >> 8);

	// Reset the cpu
	v6502_reset(cpu);
	
	// Wait for user to start it
	faulted = 1;
}

- (void) keyDown:(NSEvent *)theEvent {
	switch ([theEvent keyCode]) {
		case kVK_ANSI_W: {
			cpu->memory->bytes[0xFF] = 'w';
		} break;
		case kVK_ANSI_A: {
			cpu->memory->bytes[0xFF] = 'a';
		} break;
		case kVK_ANSI_S: {
			cpu->memory->bytes[0xFF] = 's';
		} break;
		case kVK_ANSI_D: {
			cpu->memory->bytes[0xFF] = 'd';
		} break;
		default:
			break;
	}
}

@end
