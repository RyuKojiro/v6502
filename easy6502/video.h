//
//  video.h
//  v6502
//
//  Created by Daniel Loffgren on H.25/06/03.
//  Copyright (c) 平成25年 Hello-Channel, LLC. All rights reserved.
//

#include <curses.h>

#include "mem.h"

#ifndef easy6502_video_h
#define easy6502_video_h

WINDOW *initVideo();
void updateVideo(v6502_memory *mem, WINDOW *win);

#endif
