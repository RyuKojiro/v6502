//
//  VMWindowController.m
//  cocoa-easy6502
//
//  Created by Daniel Loffgren on H.25/06/04.
//  Copyright (c) 平成25年 Hello-Channel, LLC. All rights reserved.
//

#import <HIToolbox/Events.h>
#import <v6502/cpu.h>
#import <v6502/mem.h>
#import <dis6502/reverse.h>

#import "VMWindowController.h"

#define DEFAULT_RESET_VECTOR	0x0600

#define MEM_RANDOM_BYTE			0x00FE
#define MEM_KEYBOARD_BYTE		0x00FF

#define kCEZFreezeFileType		@"v6freeze"

volatile static int faulted;

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
		mem->bytes[DEFAULT_RESET_VECTOR + (offset++)] = byte;
	}
		
	fclose(f);
	return YES;
}

BOOL saveFreeze(v6502_cpu *cpu, const char *fname) {
	FILE *f = fopen(fname, "w");
	if (!f) {
		return NO;
	}
	
	fwrite(&cpu->pc, sizeof(uint16_t), 1, f);
	fwrite(&cpu->sp, sizeof(uint8_t), 1, f);
	fwrite(&cpu->sr, sizeof(uint8_t), 1, f);
	fwrite(&cpu->x, sizeof(uint8_t), 1, f);
	fwrite(&cpu->y, sizeof(uint8_t), 1, f);
	fwrite(&cpu->ac, sizeof(uint8_t), 1, f);

	for (uint16_t offset = 0; offset < v6502_memoryStartCeiling; offset++) {
		uint8_t byte = v6502_read(cpu->memory, offset, NO);
		fwrite(&byte, sizeof(uint8_t), 1, f);
	}

	fclose(f);
	return YES;
}

BOOL loadFreeze(v6502_cpu *cpu, const char *fname) {
	FILE *f = fopen(fname, "r");
	if (!f) {
		return NO;
	}
	
	fread(&cpu->pc, sizeof(uint16_t), 1, f);
	fread(&cpu->sp, sizeof(uint8_t), 1, f);
	fread(&cpu->sr, sizeof(uint8_t), 1, f);
	fread(&cpu->x, sizeof(uint8_t), 1, f);
	fread(&cpu->y, sizeof(uint8_t), 1, f);
	fread(&cpu->ac, sizeof(uint8_t), 1, f);
	
	for (uint16_t offset = 0; offset < v6502_memoryStartCeiling; offset++) {
		uint8_t byte;
		fread(&byte, sizeof(uint8_t), 1, f);
		v6502_write(cpu->memory, offset, byte);
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
	if (![NSThread isMainThread]) {
		[[NSThread currentThread] setName:@"v6502 CPU"];
	}
	
	do {
		// Processor time
		v6502_step(cpu);
		
		// Exit on break
		if (cpu->sr & v6502_cpu_status_break) {
			return;
		}
		
		// Update fields
		//[self update];
		
		// Throttle CPU Frequency
		int t = 200 - _speedSlider.intValue;
		usleep(t);
	} while (!faulted);
}

- (IBAction)reset:(id)sender {
	v6502_reset(cpu);
	[video resetVideoMemory];
	[self update];
}

- (IBAction)freeze:(id)sender {
	BOOL wasRunning = !faulted;
	[self stop:sender];
	
	NSSavePanel *p = [NSSavePanel savePanel];
	p.title = @"Freeze State";
	p.allowedFileTypes = @[kCEZFreezeFileType];
	[p beginSheetModalForWindow:self.window completionHandler:^(NSInteger result) {
		if (result == NSFileHandlingPanelOKButton) {
			if (!saveFreeze(cpu, [p.URL.path cStringUsingEncoding:NSUTF8StringEncoding])) {
				NSAlert *alert = [NSAlert alertWithMessageText:@"Failed to save" defaultButton:@"OK" alternateButton:nil otherButton:nil informativeTextWithFormat:@"There was a problem trying to open the freeze file for writing."];
				[alert beginSheetModalForWindow:self.window completionHandler:nil];
			}
		}
		
		if (wasRunning) {
			[self start:sender];
		}
	}];
}

- (IBAction)loadFreeze:(id)sender {
	[self stop:sender];
	
	NSOpenPanel *p = [NSOpenPanel openPanel];
	p.title = @"Load Freeze State";
	p.allowedFileTypes = @[kCEZFreezeFileType];
	[p beginSheetModalForWindow:self.window completionHandler:^(NSInteger result) {
		if (result == NSFileHandlingPanelOKButton) {
			if (!loadFreeze(cpu, [p.URL.path cStringUsingEncoding:NSUTF8StringEncoding])) {
				NSAlert *alert = [NSAlert alertWithMessageText:@"Failed to open" defaultButton:@"OK" alternateButton:nil otherButton:nil informativeTextWithFormat:@"There was a problem trying to open the freeze file for reading."];
				[alert beginSheetModalForWindow:self.window completionHandler:nil];
			}
		}
	}];
}

- (IBAction)videoReset:(id)sender {
	[video resetVideoMemory];
	[video setNeedsDisplay:YES];
}

- (IBAction)testPattern:(id)sender {
	[video testPattern];
	[video setNeedsDisplay:YES];
}

- (IBAction)start:(id)sender {
	[toggleButton setTitle:@"Halt"];
	faulted = 0;
	[self performSelectorInBackground:@selector(cycle) withObject:nil];
}

- (IBAction)stop:(id)sender {
	[toggleButton setTitle:@"Run"];
	faulted++;
}

- (IBAction)toggleRunning:(id)sender {
	if (faulted) {
		[self start:sender];
	}
	else {
		[self stop:sender];
	}
}

- (IBAction)step:(id)sender {
	[self cycle];
	[self update];
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

uint8_t randomByteCallback(struct _v6502_memory *memory, uint16_t offset, int trap, void *context) {
	return (uint8_t)arc4random();
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

	// Wire the random byte to the randomizer
	v6502_map(cpu->memory, MEM_RANDOM_BYTE, 1, randomByteCallback, NULL, NULL);
	
	// Reset the cpu
	v6502_reset(cpu);
	
	// Wait for user to start it
	faulted = 1;
}

- (void) keyDown:(NSEvent *)theEvent {
	switch ([theEvent keyCode]) {
		case kVK_ANSI_W: {
			cpu->memory->bytes[MEM_KEYBOARD_BYTE] = 'w';
		} break;
		case kVK_ANSI_A: {
			cpu->memory->bytes[MEM_KEYBOARD_BYTE] = 'a';
		} break;
		case kVK_ANSI_S: {
			cpu->memory->bytes[MEM_KEYBOARD_BYTE] = 's';
		} break;
		case kVK_ANSI_D: {
			cpu->memory->bytes[MEM_KEYBOARD_BYTE] = 'd';
		} break;
		default:
			break;
	}
}

@end
