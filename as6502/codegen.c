/*
 * Copyright (c) 2013 Daniel Loffgren
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */

#include <string.h>
#include <strings.h>
#include <stdlib.h>
#include <assert.h>

#include "codegen.h"
#include "parser.h"
#include "linectl.h"
#include "error.h"
#include "token.h"

#define MAX_HEX_LEN 8

as6502_token *as6502_resolveArithmeticInExpression(as6502_token *head) {
	// There can be multiple arithmetical operations per line, so just loop until all are solved.
	
	as6502_token *lhs, *op, *rhs;
	while (as6502_tokenListFindTokenLiteral(head, "+") ||
			as6502_tokenListFindTokenLiteral(head, "-") ) {
		lhs = as6502_firstTokenOfTypeInList(head, as6502_token_type_value);
		op = lhs->next;

		// actually detect an operator
		if (!op || op->len != 1 || !(op->text[0] == '+' || op->text[0] == '-')) {
			continue; 
		}
		rhs = op->next;

		// Calculate value, variables are initialized for lint
		uint16_t result = 0;
		int lwide = NO;
		int rwide = NO;
		switch (op->text[0]) {
			case '+': {
				result = as6502_valueForString(&lwide, lhs->text) + as6502_valueForString(&rwide, rhs->text);
			} break;
			case '-': {
				result = as6502_valueForString(&lwide, lhs->text) - as6502_valueForString(&rwide, rhs->text);
			} break;
			default:
				assert(op);
		}

		// replace lhs value with result
		char *newValue = malloc(MAX_HEX_LEN);
		if (lwide || rwide) {
			snprintf(newValue, MAX_HEX_LEN, "$%4x", result);
		}
		else {
			snprintf(newValue, MAX_HEX_LEN, "$%2x", result);
		}
		free(lhs->text);
		lhs->text = newValue;
		lhs->len = strlen(newValue);

		// remove op and rhs from list
		lhs->next = rhs->next;

		// free op and rhs
		as6502_tokenDestroy(op);
		as6502_tokenDestroy(rhs);
	}

	return head;
}

void as6502_processObjectDirectiveInExpression(ld6502_object *obj, int *currentBlob, as6502_token *head) {
	assert(obj);

	if (as6502_tokenIsEqualToStringLiteral(head, ".org")) {
		if (!head->next) {
			as6502_error(head->loc, head->len, "Encountered .org directive without an address afterwards.");
			return;
		}
		
		// start new blob
		ld6502_addBlobToObject(obj, as6502_valueForString(NULL, head->next->text));
		*currentBlob = obj->count - 1;
	}
	else if (as6502_tokenIsEqualToStringLiteral(head, ".end")) {
		// revert to top blob
		*currentBlob = 0;
	}
	else if (as6502_tokenIsEqualToStringLiteral(head, ".asciiz")) {
		if (!head->next || head->next->text[0] != '"') {
			as6502_error(head->loc, head->len, "Encountered .asciiz directive without a string afterwards.");
			return;
		}

		const char *string = head->next->text;
		size_t len = head->next->len;
		ld6502_object_blob *blob = &obj->blobs[*currentBlob];
		for (const char *cur = string + 1; *cur && (cur < string + len) && *cur != '"'; cur++) {
			ld6502_appendByteToBlob(blob, *cur);
		}
		ld6502_appendByteToBlob(blob, '\0'); // the 'z' means null terminate
	}
	else if (as6502_tokenIsEqualToStringLiteral(head, ".byte")) {
		int wide;
		uint8_t low, high;
		
		as6502_byteValuesForString(&high, &low, &wide, head->next->text);

		ld6502_object_blob *blob = &obj->blobs[*currentBlob];
		ld6502_appendByteToBlob(blob, low);
		if (wide) {
			ld6502_appendByteToBlob(blob, high);
		}
	}
	else {
		as6502_warn(head->loc, head->len, "Unknown assembler directive");
	}
}
