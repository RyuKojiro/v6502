//
//  linectl.h
//  v6502
//
//  Created by Daniel Loffgren on H.25/05/05.
//  Copyright (c) 平成25年 Hello-Channel, LLC. All rights reserved.
//

#ifndef v6502_linectl_h
#define v6502_linectl_h

// Destructively trim trailing whitespace with NUL
void trimtaild(char *str);

// Destructively tail trim at first encounter of token char from the tail end
void trimtailchard(char *str, char token);

// Destructively tail trim at first encounter of token char from the head end
void trimgreedytailchard(char *str, char token);

// Safely trim leading whitespace by pushing pointer
char *trimhead(char *str);

#endif
