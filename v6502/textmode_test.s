;
; Copyright (c) 2013 Daniel Loffgren
;
; Permission is hereby granted, free of charge, to any person obtaining a copy
; of this software and associated documentation files (the "Software"), to
; deal in the Software without restriction, including without limitation the
; rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
; sell copies of the Software, and to permit persons to whom the Software is
; furnished to do so, subject to the following conditions:
;
; The above copyright notice and this permission notice shall be included in
; all copies or substantial portions of the Software.
;
; THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
; IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
; FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
; AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
; LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
; FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
; IN THE SOFTWARE.
; 
; This file is a sample of some assembly to interact with text mode video.
;

	ldx #$00
nextChar:
	lda string,X
	sta $2000,X
	inx
	beq end
	jmp nextChar
end:			; This does the same in v6502 as a wai instruction would
	jmp end
string:
.asciiz "This is a test"