;
;  instructions.s
;  v6502
;
;  Created by Daniel Loffgren on H25/06/29.
;

; NOTE: If LDA, JMP, or JSR do not properly work, then the environment is not
; sane enough to even test.

; On failure, brk, so that the PC is intact, if there is a branch required for
; failure, then have a unique label with a brk.

; First, sanity check flags and branches
; SEC	....	set carry
	lda #$38
	sec
	bcc fail1
; BCC	....	branch on carry clear
	bcs testCLC
	brk
fail1:
	brk
; CLC	....	clear carry
testCLC:
	lda #$18
	clc
	bcs fail2
; BCS	....	branch on carry set
	bcc testSED
	brk
fail2:
	brk
;;;;;;;;;;;;;;;;;;;;;;;;;;;;; GOOD UNTIL HERE ;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; SED	....	set decimal
testSED:
	lda #$F8
	sed
	bcs fail3
; CLD	....	clear decimal
	lda #$D8
	cld
	bcs fail4

; SEI	....	set interrupt disable
; CLI	....	clear interrupt disable

; BEQ	....	branch on equal (zero set)
	lda #00
	ldx #00
	bne eqFail
	ldx #01
	beq testBMI

eqFail:
	lda #F0
	jmp fail

; BMI	....	branch on minus (negative set)
testBMI:
	lda #FF
	beq negFail

negFail:
	

; BNE	....	branch on not equal (zero clear)
; BPL	....	branch on plus (negative clear)


; LDA	....	load accumulator
; LDY	....	load X
; LDY	....	load Y

; ADC	....	add with carry
; AND	....	and (with accumulator)
; ASL	....	arithmetic shift left
; BIT	....	bit test
; BRK	....	interrupt
; BVC	....	branch on overflow clear
; BVS	....	branch on overflow set
; CLV	....	clear overflow
; CMP	....	compare (with accumulator)
; CPX	....	compare with X
; CPY	....	compare with Y
; DEC	....	decrement
; DEX	....	decrement X
; DEY	....	decrement Y
; EOR	....	exclusive or (with accumulator)
; INC	....	increment
; INX	....	increment X
; INY	....	increment Y
; JMP	....	jump
; JSR	....	jump subroutine
; LSR	....	logical shift right
; NOP	....	no operation
; ORA	....	or with accumulator
; PHA	....	push accumulator
; PHP	....	push processor status (SR)
; PLA	....	pull accumulator
; PLP	....	pull processor status (SR)
; ROL	....	rotate left
; ROR	....	rotate right
; RTI	....	return from interrupt
; RTS	....	return from subroutine
; SBC	....	subtract with carry
; STA	....	store accumulator
; STX	....	store X
; STY	....	store Y
; TAX	....	transfer accumulator to X
; TAY	....	transfer accumulator to Y
; TSX	....	transfer stack pointer to X
; TXA	....	transfer X to accumulator
; TXS	....	transfer X to stack pointer
; TYA	....	transfer Y to accumulator

success:
	lda #$FF
	brk
