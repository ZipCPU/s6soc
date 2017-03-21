;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;//
;;
;; Filename: 	resetdump.s
;;
;; Project:	CMod S6 System on a Chip, ZipCPU demonstration project
;;
;; Purpose:	While most of my ZipCPU programs to date have started with a
;;		simple assembly script to load the stack and call the program
;;	entry point, this is different.  This is a very complicated startup
;;	script designed to dump all of the internal information to the CPU
;;	to a UART output port.  This is on purpose.  Indeed, this may be my
;;	only means of debugging the device once it goes bad:
;;
;;	- To set a breakpoint
;;		at the location desired call kpanic(), the CPU will dump its
;;			variables and wait for a button press to restart.
;;		sometime before the desired clock, set the watchdog timer.
;;			When the watchdog expires, the CPU will restart. 
;;			Adjusting the watchdog will adjust how long the CPU
;;			waits before restarting, and may also adjust what
;;			instructions you find going on
;;
;;	- In hardware, you can also set the scope.  On boot up, this resetdump
;;		startup will dump the value of the scope to the UART.
;;
;;	Of course, this all depends upon someone listening on the uart.  That's
;;	the purpose of the dumpuart.cpp program in the sw/host directory.
;;	That file will capture the dump so it can be examined later.
;;
;; Creator:	Dan Gisselquist, Ph.D.
;;		Gisselquist Technology, LLC
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;//
;;
;; Copyright (C) 2015-2017, Gisselquist Technology, LLC
;;
;; This program is free software (firmware): you can redistribute it and/or
;; modify it under the terms of  the GNU General Public License as published
;; by the Free Software Foundation, either version 3 of the License, or (at
;; your option) any later version.
;;
;; This program is distributed in the hope that it will be useful, but WITHOUT
;; ANY WARRANTY; without even the implied warranty of MERCHANTIBILITY or
;; FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
;; for more details.
;;
;; You should have received a copy of the GNU General Public License along
;; with this program.  (It's in the $(ROOT)/doc directory, run make with no
;; target there if the PDF file isn't present.)  If not, see
;; <http://www.gnu.org/licenses/> for a copy.
;;
;; License:	GPL, v3, as defined and found on www.gnu.org,
;;		http://www.gnu.org/licenses/gpl.html
;;
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;//
;;
;;
	.section	.start
	.global		_start
	.type		_start,@function
_start:
; Upon reset, we must output our registers to the UART, lest we reset because
; of a crash
	SW	R0,(DBG)
	LDI	0x00000002,R0
	SW	R0,(SCOPE)
	JSR	internal_kpanic
	LDI	_top_of_stack,SP
	LDI	gostr,R1
	JSR	raw_put_string
	LDI	0x7fffffff,R1
	SW	R1,(PIC)
;	; Turn on the button interrupt--so we can stop on interrupt
;	LDI	0x80010001,R1
;	SW	R1,(PIC)
	;LDI	0xff000002,R1
	;SW	R1,(SCOPE)
; For running in supervisor mode:
;	JSR	entry
; otherwise ..
	MOV	entry(PC),uPC
	MOV	SP,uSP
	RTU
	JMP	kpanic

	.global		kpanic
	.type		kpanic,@function
kpanic:
	SW	R0,(DBG)
	SW	R1,4(DBG)
	SW	R2,8(DBG)
	LDI	panicstr, R1
	JSR	raw_put_string
	LW	4(DBG),R1
	LW	8(DBG),R2
	JSR	internal_kpanic
kpanic_wait_for_button_release:
	LW	(SPIO),R0
	TEST	0x010,R0
	BNZ	kpanic_wait_for_button_release
kpanic_wait_for_button:
	LW	(SPIO),R0
	TEST	0x010,R0
	BZ	kpanic_wait_for_button
	BRA	_start
	HALT
	
internal_kpanic:
	; The original R0 is stored in (DBG)
	SW	R1,4(DBG)
	SW	R2,8(DBG)
	SW	R0,12(DBG)	; Our return address

	; Make sure the watchdog is off
	LDI	0,R1
	SW	R1,0x40c

	; R0
	LDI	0,R1
	LW	(DBG),R2
	JSR	uart_put_reg_value

	; R1
	LDI	1,R1
	LW	4(DBG),R2
	JSR	uart_put_reg_value
	; R2
	LDI	2,R1
	LW	8(DBG),R2
	JSR	uart_put_reg_value
	; R3
	LDI	3,R1
	MOV	R3,R2
	JSR	uart_put_reg_value

	; R4
	LDI	4,R1
	MOV	R4,R2
	JSR	uart_put_reg_value

	; R5
	LDI	5,R1
	MOV	R5,R2
	JSR	uart_put_reg_value

	; R6
	LDI	6,R1
	MOV	R6,R2
	JSR	uart_put_reg_value

	; R7
	LDI	7,R1
	MOV	R7,R2
	JSR	uart_put_reg_value

	; R8
	LDI	8,R1
	MOV	R8,R2
	JSR	uart_put_reg_value

	; R9
	LDI	9,R1
	MOV	R9,R2
	JSR	uart_put_reg_value

	; R10
	LDI	10,R1
	MOV	R10,R2
	JSR	uart_put_reg_value

	; R11
	LDI	11,R1
	MOV	R11,R2
	JSR	uart_put_reg_value

	; R12
	LDI	12,R1
	MOV	R12,R2
	JSR	uart_put_reg_value

	; SP
	LDI	13,R1
	MOV	R13,R2
	JSR	uart_put_reg_value

	; uR0
	LDI	16,R1
	MOV	uR0,R2
	JSR	uart_put_reg_value

	; uR1
	LDI	17,R1
	MOV	uR1,R2
	JSR	uart_put_reg_value

	LDI	18,R1
	MOV	uR2,R2
	JSR	uart_put_reg_value

	LDI	19,R1
	MOV	uR3,R2
	JSR	uart_put_reg_value

	LDI	20,R1
	MOV	uR4,R2
	JSR	uart_put_reg_value

	LDI	21,R1
	MOV	uR5,R2
	JSR	uart_put_reg_value

	LDI	22,R1
	MOV	uR6,R2
	JSR	uart_put_reg_value

	LDI	23,R1
	MOV	uR7,R2
	JSR	uart_put_reg_value

	LDI	24,R1
	MOV	uR8,R2
	JSR	uart_put_reg_value

	LDI	25,R1
	MOV	uR9,R2
	JSR	uart_put_reg_value

	LDI	26,R1
	MOV	uR10,R2
	JSR	uart_put_reg_value

	LDI	27,R1
	MOV	uR11,R2
	JSR	uart_put_reg_value

	LDI	28,R1
	MOV	uR12,R2
	JSR	uart_put_reg_value

	; uSP
	LDI	29,R1
	MOV	uSP,R2
	JSR	uart_put_reg_value

	; uCC
	LDI	30,R1
	MOV	uCC,R2
	JSR	uart_put_reg_value

	; uPC
	LDI	31,R1
	MOV	uPC,R2
	JSR	uart_put_reg_value

;stack_mem_dump:
;	LDI	0,R4
;	LDI	_top_of_stack,R5
;stack_mem_dump_loop:
;	MOV	R4,R1
;	LW	(R5),R2
;	JSR	uart_put_stack_value
;	ADD	4,R4
;	SUB	4,R5
;	CMP	256,R4
;	BLT	stack_mem_dump_loop

; Get prepared for a proper start by setting our stack register
	LDI	_top_of_stack,SP

	BRA	dump_scope
	; BRA	end_internal_panic

; Now, do a full dump of all memory--all registers are available to us
dump_memory:
	LDI	RAM,R5
	LDI	0x0fff,R6
	LDI	0x0f8,R7
	SW	R7,(SPIO)
full_mem_dump_loop:
	MOV	R5,R1
	LW	(R5),R2
	JSR	uart_dump_mem_value
	LDI	0x0f2,R7
	SW	R7,(SPIO)

	ADD	4,R5
	SUB	1,R6
	BGE	full_mem_dump_loop

	LDI	0x0f5,R7
	SW	R7,(SPIO)

dump_scope:
; Finally, do a full dump of the scope--if it had triggered
;   First, dump the scope control word
	LDI	SCOPE,R7	; R7 = Debugging scope address
	MOV	R7,R1
	LW	(R7),R2
	MOV	R2,R5		; R5 will not be changed by a subroutine
	JSR	uart_dump_mem_value
;   Then test whether or not the scope has stopped
	LDI	0x40000000,R1
	TEST	R1,R5
;   If not, start our kernel.
	BZ	dump_buserr
;   Otherwise, calculate the size of the scope
	LSR	20,R5
	AND	0x1f,R5
	LDI	1,R6
	LSL	R5,R6	; R6 is now the size (number of words) of the scope
	SUB	1,R6	; Now it is one less than the size,2 support the BGE l8r
;   And start dumping
	ADD	4,R7	; Get the scope data address
dump_scope_loop:
	MOV	R7,R1
	LW	(R7),R2
	JSR	uart_dump_mem_value
	SUB	1,R6
	BGE	dump_scope_loop

dump_buserr:
; Dump a bus error address, if used
	LDI	buserr_header, R1
	JSR	raw_put_string
	LDI	BUSERR,R1
	LW	(R1),R2
	JSR	uart_dump_mem_value

end_internal_panic:
	LDI	doublenewline,R1
	JSR	raw_put_string
	LDI	0x0ff,R7
	SW	R7,(SPIO)
	LW	12(DBG),PC
	RTN

; R0 is return address
; R1 is register ID
; R2 is register value to output
uart_put_reg_value:
	SW	R0,16(DBG)
	SW	R2,20(DBG)
	SW	R3,24(DBG)
	MOV	R1,R2
	LDI	'u',R1
	CMP	16,R2
	LDILO.LT 's',R1
	SUB.GE	16,R2
	JSR	raw_put_uart
	LDI	'0',R1
	CMP	10,R2
	LDILO.GE '1',R1
	SUB.GE	10,R2
	JSR	raw_put_uart
	MOV	R2,R1
	AND	15,R1
	JSR	get_hex
	JSR	raw_put_uart
	LDI	':',R1
	JSR	raw_put_uart
	LW	20(DBG),R2
	LDI	8,R3
uart_put_loop:
	MOV	R2,R1
	LSR	28,R1
	LSL	4,R2
	JSR	get_hex
	JSR	raw_put_uart
	SUB	1,R3
	BNZ	uart_put_loop
	LDI	newline, R1
	JSR	raw_put_string
	LW	16(DBG),R0
	LW	20(DBG),R2
	LW	24(DBG),R3
	RTN

uart_dump_mem_value:
;	R0 = return address
;	R1 = Memory address
;	R2 = Memory Value
; Local: R3 = working value
	SW	R0,28(DBG)
	MOV	R1,R3		; R3 = Memory address
	MOV	R2,R4		; R4 = Memory Value
	LDI	memopenstr,R1
	JSR	raw_put_string
	; Set up a loop to dump things
	LSL	16,R3		; Ignore the first 16 bits
	LDI	4,R2		; We're going to do four hex digits here
	;
uart_put_mem_address_loop:
	MOV	R3,R1
	LSR	28,R1
	LSL	4,R3
	JSR	get_hex
	JSR	raw_put_uart
	SUB	1,R2
	BNZ	uart_put_mem_address_loop
	; Put some transition characters
	LDI	memmidstr,R1
	JSR	raw_put_string

	; Set up a loop to dump the memory value now
	LDI	8,R2
uart_put_mem_value_loop:
	MOV	R4,R1
	LSR	28,R1
	LSL	4,R4
	JSR	get_hex
	JSR	raw_put_uart
	SUB	1,R2
	BNZ	uart_put_mem_value_loop
	; Clear the line
	LDI	newline,R1
	JSR	raw_put_string
	; And return from our subroutine
	LW	28(DBG),R0
	RTN

get_hex:
	ADD	0x30,R1
	CMP	0x3a,R1
	ADD.GE	7,R1	; Add 'A'-'0'-10
	JMP	R0

; R0 is the return address
; R1 is the string address
raw_put_string:
	SW	R0,36(DBG)
	SW	R2,40(DBG)
	MOV	R1,R2
raw_put_string_next:
	LB	(R2),R1
	CMP	0,R1
	LW.Z	36(DBG),R0
	LW.Z	40(DBG),R2
	RTN.Z
	ADD	1,R2
	MOV	raw_put_string_next(PC),R0
	BRA	raw_put_uart
	
; R0 is return address,
; R1 is value to transmit
raw_put_uart:
	SW	R2,32(DBG)
	LDI	INT_UARTTX,R2
	SW	R2,(PIC)	; Clear the PIC, turn off interrupts, etc.
raw_put_uart_retest:
	LW	(PIC),R2
	TEST	INT_UARTTX,R2
	BZ	raw_put_uart_retest
	SW	R1,(UART)
	LW	32(DBG),R2
	JMP	R0

	.section	.fixdata
DBG:
.int	0,0,0,0,0,0,0,0,0,0

.set	INT_UARTRX, 0x040
.set	INT_UARTTX, 0x080
.set	PIC,     0x0400
.set	BUSERR,  0x0404
.set	TMRA,    0x0408
.set	TMRB,    0x040c
.set	PWM,     0x0410
.set	SPIO,    0x0414
.set	GPIO,    0x0418
.set	UART,    0x041c
.set	VERSION, 0x0420
.set	SCOPE,   0x0800
.set	SCOPED,  0x0804
.set	RAM,     0x8000
	.section	.rodata
doublenewline:
	.ascii	"\r\n"
newline:
	.asciz	"\r\n"
buserr_header:
	.string	"BusErr: "
panicstr:
	.string	"Panic: \r\n"
memopenstr:
	.string	"M[0x"
memmidstr:
	.string	"]: "
gostr:
	.string	"Go!\r\n"

