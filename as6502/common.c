//
//  common.c
//  v6502
//
//  Created by Daniel Loffgren on H.25/05/05.
//  Copyright (c) 平成25年 Hello-Channel, LLC. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>

#include "common.h"

void die(const char *reason) {
	fprintf(stderr, "as6502: fatal: %s\n", reason);
	exit(EXIT_FAILURE);
}
