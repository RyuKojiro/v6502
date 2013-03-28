//
//  mem.h
//  v6502
//
//  Created by Daniel Loffgren on H.25/03/28.
//  Copyright (c) 平成25年 Hello-Channel, LLC. All rights reserved.
//

#ifndef v6502_mem_h
#define v6502_mem_h

// Memory Object
typedef struct {
	void *bytes;
	size_t size;
} v6502_memory;

// Memory Lifecycle
v6502_memory *v6502_createMemory(size_t size);
void v6502_destroyMemory(v6502_memory *memory);

#endif
