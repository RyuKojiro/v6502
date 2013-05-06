//
//  labels.h
//  v6502
//
//  Created by Daniel Loffgren on H.25/05/05.
//  Copyright (c) 平成25年 Hello-Channel, LLC. All rights reserved.
//

#ifndef v6502_labels_h
#define v6502_labels_h

#include <stdint.h>
#include <stdio.h>

// Types
typedef struct {
	int size;
	uint16_t addresses[];
} as6502_label_table;

// Address Table Lifecycle Functions
as6502_label_table *as6502_createLabelTable();
void as6502_destroyLabelTable(as6502_label_table *table);
void as6502_populateLabelTableWithFile(as6502_label_table *table, FILE *file);
void as6502_printLabelTable(as6502_label_table *table);

// Address Table Accessors
void as6502_addLabelToTable(as6502_label_table *table, const char labelName, uint16_t address);
uint16_t as6502_addressForLabel(as6502_label_table *table, const char labelName);

#endif
