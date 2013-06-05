//
/** @brief Instruction Disassembly */
/** @file reverse.h */
//
//  Created by Daniel Loffgren on H.25/06/04.
//  Copyright (c) 平成25年 Hello-Channel, LLC. All rights reserved.
//

#ifndef v6502_reverse_h
#define v6502_reverse_h

#include <stdio.h>

#include "cpu.h"
#include "parser.h"

/** @defgroup rev Instruction Disassembly */
/**@{*/
/** @brief Get the string representation of an opcode */
void as6502_stringForOpcode(char *string, size_t len, v6502_opcode opcode);
/** @brief Get the string representation of the operands in a given address mode */
void as6502_stringForOperand(char *string, size_t len, v6502_address_mode opcode, uint8_t high, uint8_t low);
/** @brief Get the complete string representation of an instruction line */
void as6502_stringForInstruction(char *string, size_t len, v6502_opcode opcode, uint8_t high, uint8_t low);
/**@}*/

#endif
