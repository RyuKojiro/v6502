//
//  core.c
//  v6502
//
//  Created by Daniel Loffgren on H.25/03/28.
//  Copyright (c) 平成25年 Hello-Channel, LLC. All rights reserved.
//

#include <stdio.h>

void v6502_faultExternal(const char *error) {
	sprintf(stderr, "External fault: ");
	sprintf(stderr, error);
	if (error[strlen(error)] != '\n') {
		sprintf(stderr, "\n");
	}
}