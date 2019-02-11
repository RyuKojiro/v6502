/*
 * Copyright (c) 2018 Daniel Loffgren
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

#include "time.h"

#include <assert.h>

bool timespec_compare(const struct timespec *a, const struct timespec *b) {
	return (a->tv_sec > b->tv_sec) ||
	       (a->tv_sec == b->tv_sec && a->tv_nsec > b->tv_nsec);
}

struct timespec timespec_subtract(const struct timespec *minuend,
                                  const struct timespec *subtrahend) {
	/*
	 * This has to be arranged in reverse, since timespec_compare only operates
	 * as a greater-than comparison, and subtracting two equal timespecs is
	 * allowed.
	 */
	assert(!timespec_compare(subtrahend, minuend));

	/*
	 * Borrow from the seconds place.
	 *
	 * tv_nsec is a long, and so should be capable of holding a spare
	 * second (in nanoseconds) on all platforms.
	 */
	const int borrow = minuend->tv_nsec < subtrahend->tv_nsec;

	const struct timespec result = {
		.tv_sec = minuend->tv_sec - borrow - subtrahend->tv_sec,
		.tv_nsec = minuend->tv_nsec + (borrow * NSEC_PER_SEC) - subtrahend->tv_nsec,
	};

	return result;
}
