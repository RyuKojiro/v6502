;
; Copyright (c) 2015 Daniel Loffgren
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
; This file is a work in progress line printing implementation.
;


	lda #$1f
	ldy #$06
	jsr PRINT
end:				; This loop is effectively a WAI instruction
	jmp end
	jmp string			; This only exists to emit the string label address

;; PRINT ;; line print the string located at $YYAA
PRINT:
	sta *$00			; create a pointer in *$01,00 that contains the string start
	sty *$01			; the lower byte of the target string
	ldy #$00			; Reset the Y register to zero, since it is next used ¬
							; as the iterator over the strings
nextChar:			; This is basically a strcpy loop
	lda ($00),Y			; load the byte (Y is required for this kind of indirection)
	sta $2000,Y			; store the byte in the same offset at the target location
	iny					; increment the byte offset
	beq donePrinting	; If the next byte is null, we're done
	jmp nextChar		; otherwise, rinse and repeat
donePrinting:		; we hit a null byte
	rts					; return

string:
.asciiz "This is a line test"