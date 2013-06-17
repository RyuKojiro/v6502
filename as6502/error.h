//
//  error.h
//  v6502
//
//  Created by Daniel Loffgren on H.25/05/05.
//  Copyright (c) 平成25年 Hello-Channel, LLC. All rights reserved.
//

#ifndef v6502_error_h
#define v6502_error_h

__attribute((noreturn)) void as6502_fatal(const char *reason);

#endif
