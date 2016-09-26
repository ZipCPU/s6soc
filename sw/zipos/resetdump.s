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
;;			variables and restart.
;;		sometime before the desired clock, set the watchdog timer
;;			(currently TIMER_B).  When the watchdog expires,
;;			the CPU will restart.  Adjusting the watchdog will
;;			adjust how long the CPU waits before restarting, and
;;			may also adjust what instructions you find going on
;;
;;	- In hardware, you can set the scope.  On boot up, this resetdump
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
;; Copyright (C) 2015-2016, Gisselquist Technology, LLC
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
	STO	R0,(DBG)
	MOV	PC+1,R0
	BRA	internal_kpanic
	LDI	_top_of_stack,SP
	LDI	kernel_entry,R0
	BRA	bootloader

	.global		kpanic
	.type		kpanic,@function
kpanic:
	STO	R0,(DBG)
	STO	R1,1(DBG)
	STO	R2,2(DBG)
	LDI	'P',R1
	MOV	PC+1,R0
	JMP	raw_put_uart
	LDI	'a',R1
	MOV	PC+1,R0
	JMP	raw_put_uart
	LDI	'n',R1
	MOV	PC+1,R0
	JMP	raw_put_uart
	LDI	'i',R1
	MOV	PC+1,R0
	JMP	raw_put_uart
	LDI	'c',R1
	MOV	PC+1,R0
	JMP	raw_put_uart
	LDI	':',R1
	MOV	PC+1,R0
	JMP	raw_put_uart
	LDI	' ',R1
	MOV	PC+1,R0
	JMP	raw_put_uart
	LDI	'\r',R1
	MOV	PC+1,R0
	JMP	raw_put_uart
	LDI	'\n',R1
	MOV	PC+1,R0
	JMP	raw_put_uart
	LOD	1(DBG),R1
	LOD	2(DBG),R2
	MOV	PC+1,R0
	JMP	internal_kpanic
kpanic_wait_for_button_release:
	LOD	(SPIO),R0
	TEST	0x010,R0
	BNZ	kpanic_wait_for_button_release
kpanic_wait_for_button:
	LOD	(SPIO),R0
	TEST	0x010,R0
	BZ	kpanic_wait_for_button
	BRA	_start
	HALT
	
internal_kpanic:
	STO	R1,1(DBG)
	STO	R2,2(DBG)
	STO	R0,3(DBG)	; Our return address

	; R0
	LDI	0,R1
	LOD	(DBG),R2
	MOV	.Lcall0(PC),R0
	JMP	uart_put_reg_value
.Lcall0:

	; R1
	LDI	1,R1
	LOD	1(DBG),R2
	MOV	.Lcall1(PC),R0
	JMP	uart_put_reg_value
.Lcall1:
	; R2
	LDI	2,R1
	LOD	2(DBG),R2
	MOV	PC+1,R0
	JMP	uart_put_reg_value
	; R3
	LDI	3,R1
	MOV	R3,R2
	MOV	PC+1,R0
	JMP	uart_put_reg_value

	; R4
	LDI	4,R1
	MOV	R4,R2
	MOV	PC+1,R0
	JMP	uart_put_reg_value

	; R5
	LDI	5,R1
	MOV	R5,R2
	MOV	PC+1,R0
	JMP	uart_put_reg_value

	; R6
	LDI	6,R1
	MOV	R6,R2
	MOV	PC+1,R0
	JMP	uart_put_reg_value

	; R7
	LDI	7,R1
	MOV	R7,R2
	MOV	PC+1,R0
	JMP	uart_put_reg_value

	; R8
	LDI	8,R1
	MOV	R8,R2
	MOV	PC+1,R0
	JMP	uart_put_reg_value

	; R9
	LDI	9,R1
	MOV	R9,R2
	MOV	PC+1,R0
	JMP	uart_put_reg_value

	; R10
	LDI	10,R1
	MOV	R10,R2
	MOV	PC+1,R0
	JMP	uart_put_reg_value

	; R11
	LDI	11,R1
	MOV	R11,R2
	MOV	PC+1,R0
	JMP	uart_put_reg_value

	; R12
	LDI	12,R1
	MOV	R12,R2
	MOV	PC+1,R0
	JMP	uart_put_reg_value

	; SP
	LDI	13,R1
	MOV	R13,R2
	MOV	PC+1,R0
	JMP	uart_put_reg_value

	; uR0
	LDI	16,R1
	MOV	uR0,R2
	MOV	PC+1,R0
	JMP	uart_put_reg_value

	; uR1
	LDI	17,R1
	MOV	uR1,R2
	MOV	PC+1,R0
	JMP	uart_put_reg_value

	LDI	18,R1
	MOV	uR2,R2
	MOV	PC+1,R0
	JMP	uart_put_reg_value

	LDI	19,R1
	MOV	uR3,R2
	MOV	PC+1,R0
	JMP	uart_put_reg_value

	LDI	20,R1
	MOV	uR4,R2
	MOV	PC+1,R0
	JMP	uart_put_reg_value

	LDI	21,R1
	MOV	uR5,R2
	MOV	PC+1,R0
	JMP	uart_put_reg_value

	LDI	22,R1
	MOV	uR6,R2
	MOV	PC+1,R0
	JMP	uart_put_reg_value

	LDI	23,R1
	MOV	uR7,R2
	MOV	PC+1,R0
	JMP	uart_put_reg_value

	LDI	24,R1
	MOV	uR8,R2
	MOV	PC+1,R0
	JMP	uart_put_reg_value

	LDI	25,R1
	MOV	uR9,R2
	MOV	PC+1,R0
	JMP	uart_put_reg_value

	LDI	26,R1
	MOV	uR10,R2
	MOV	PC+1,R0
	JMP	uart_put_reg_value

	LDI	27,R1
	MOV	uR11,R2
	MOV	PC+1,R0
	JMP	uart_put_reg_value

	LDI	28,R1
	MOV	uR12,R2
	MOV	PC+1,R0
	JMP	uart_put_reg_value

	; uSP
	LDI	29,R1
	MOV	uSP,R2
	MOV	PC+1,R0
	JMP	uart_put_reg_value

	; uCC
	LDI	30,R1
	MOV	uCC,R2
	MOV	PC+1,R0
	JMP	uart_put_reg_value

	; uPC
	LDI	31,R1
	MOV	uPC,R2
	MOV	PC+1,R0
	JMP	uart_put_reg_value

;stack_mem_dump:
	;LDI	0,R4
	;LDI	_top_of_stack,R5
;stack_mem_dump_loop:
	;MOV	R4,R1
	;LOD	(R5),R2
	;MOV	PC+1,R0
	;JMP	uart_put_stack_value
	;ADD	1,R4
	;SUB	1,R5
	;CMP	64,R4
	;BLT	stack_mem_dump_loop

; Get prepared for a proper start by setting our stack register
	LDI	_top_of_stack,SP

	BRA	dump_scope
	; BRA	end_internal_panic

; Now, do a full dump of all memory--all registers are available to us
dump_memory:
	LDI	RAM,R5
	LDI	0x1000,R6
	LDI	0x0f8,R7
	STO	R7,(SPIO)
full_mem_dump_loop:
	MOV	R5,R1
	LOD	(R5),R2
	MOV	PC+1,R0
	JMP	uart_dump_mem_value
	LDI	0x0f2,R7
	STO	R7,(SPIO)

	ADD	1,R5
	SUB	1,R6
	BGT	full_mem_dump_loop

	LDI	0x0f5,R7
	STO	R7,(SPIO)

dump_scope:
; Finally, do a full dump of the scope--if it had triggered
;   First, dump the scope control word
	LDI	SCOPE,R7	; R7 = Debugging scope address
	MOV	R7,R1
	LOD	(R7),R2
	MOV	R2,R5		; R5 will not be changed by a subroutine
	MOV	PC+1,R0
	BRA	uart_dump_mem_value
;   Then test whether or not the scope has stopped
	LDI	0x40000000,R1
	TEST	R1,R5
;   If not, start our kernel.
	BZ	dump_buserr
;   Otherwise, calculate the size of the scope
	LSR	20,R5
	AND	0x1f,R5
	LDI	1,R6
	LSL	R5,R6
;   And start dumping
	ADD	1,R7	; Get the scope data address
dump_scope_loop:
	MOV	R7,R1
	LOD	(R7),R2
	MOV	PC+1,R0
	BRA	uart_dump_mem_value
	SUB	1,R6
	BGT	dump_scope_loop

dump_buserr:
; Dump a bus error address, if used
	LDI	'B',R1
	MOV	PC+1,R0
	BRA	raw_put_uart
	LDI	'u',R1
	MOV	PC+1,R0
	BRA	raw_put_uart
	LDI	's',R1
	MOV	PC+1,R0
	BRA	raw_put_uart
	LDI	'E',R1
	MOV	PC+1,R0
	BRA	raw_put_uart
	LDI	'r',R1
	MOV	PC+1,R0
	BRA	raw_put_uart
	LDI	'r',R1
	MOV	PC+1,R0
	BRA	raw_put_uart
	LDI	':',R1
	MOV	PC+1,R0
	BRA	raw_put_uart
	LDI	' ',R1
	MOV	PC+1,R0
	BRA	raw_put_uart
	LDI	BUSERR,R1
	LOD	(R1),R2
	MOV	PC+1,R0
	BRA	uart_dump_mem_value

end_internal_panic:
	LDI	'\r',R1
	MOV	PC+1,R0
	BRA	raw_put_uart
	LDI	'\n',R1
	MOV	PC+1,R0
	BRA	raw_put_uart
	LDI	'\r',R1
	MOV	PC+1,R0
	BRA	raw_put_uart
	LDI	'\n',R1
	MOV	PC+1,R0
	BRA	raw_put_uart
	LDI	0x0ff,R7
	STO	R7,(SPIO)
	LOD	3(DBG),PC
	JMP	R0

; R0 is return address
; R1 is register ID
; R2 is register to output
uart_put_reg_value:
	STO	R0,4(DBG)
	STO	R2,5(DBG)
	STO	R3,6(DBG)
	MOV	R1,R2
	LDI	'u',R1
	CMP	16,R2
	LDILO.LT 's',R1
	SUB.GE	16,R2
	MOV	PC+1,R0
	JMP	raw_put_uart
	LDI	'0',R1
	CMP	10,R2
	LDILO.GE '1',R1
	SUB.GE	10,R2
	MOV	PC+1,R0
	JMP	raw_put_uart
	MOV	R2,R1
	AND	15,R1
	MOV	PC+1,R0
	JMP	get_hex
	MOV	PC+1,R0
	JMP	raw_put_uart
	LDI	58,R1	; A ':'
	MOV	PC+1,R0
	JMP	raw_put_uart
	LOD	5(DBG),R2
	LDI	8,R3
uart_put_loop:
	ROL	4,R2
	MOV	R2,R1
	AND	15,R1
	MOV	PC+1,R0
	JMP	get_hex
	MOV	PC+1,R0
	JMP	raw_put_uart
	SUB	1,R3
	BNZ	uart_put_loop
	LDI	'\r',R1
	MOV	PC+1,R0
	JMP	raw_put_uart
	LDI	'\n',R1
	MOV	PC+1,R0
	JMP	raw_put_uart
	LOD	4(DBG),R0
	LOD	5(DBG),R2
	LOD	6(DBG),R3
	JMP	R0

uart_dump_mem_value:
;	R0 = return address
;	R1 = Memory address
;	R2 = Memory Value
; Local: R3 = working value
	STO	R0,7(DBG)
	MOV	R1,R3		; R3 = Memory address
	MOV	R2,R4		; R4 = Memory Value
	LDI	77,R1		; 77 = 'M'
	MOV	PC+1,R0
	JMP	raw_put_uart
	LDI	91,R1		; 91 = '['
	MOV	PC+1,R0
	JMP	raw_put_uart
	LDI	48,R1		; A '0'
	MOV	PC+1,R0
	JMP	raw_put_uart
	LDI	120,R1		; An 'x'
	MOV	PC+1,R0
	JMP	raw_put_uart
	; Set up a loop to dump things
	ROL	16,R3		; Ignore the first 16 bits
	LDI	4,R2		; We're going to do four hex digits here
	;
uart_put_mem_address_loop:
	ROL	4,R3
	MOV	R3,R1
	AND	15,R1
	MOV	PC+1,R0
	JMP	get_hex
	MOV	PC+1,R0
	JMP	raw_put_uart
	SUB	1,R2
	BNZ	uart_put_mem_address_loop
	; Put some transition characters
	LDI	93,R1		; 93 = ']'
	MOV	PC+1,R0
	JMP	raw_put_uart
	LDI	58, R1		; A semicolon
	MOV	PC+1,R0
	JMP	raw_put_uart
	LDI	32, R1		; A space
	MOV	PC+1,R0
	JMP	raw_put_uart

	; Set up a loop to dump the memory value now
	LDI	8,R2
uart_put_mem_value_loop:
	ROL	4,R4
	MOV	R4,R1
	AND	15,R1
	MOV	PC+1,R0
	JMP	get_hex
	MOV	PC+1,R0
	JMP	raw_put_uart
	SUB	1,R2
	BNZ	uart_put_mem_value_loop
	; Clear the line
	LDI	'\r', R1
	MOV	PC+1,R0
	JMP	raw_put_uart
	LDI	'\n', R1
	MOV	PC+1,R0
	JMP	raw_put_uart
	; And return from our subroutine
	LOD	7(DBG),R0
	JMP	R0

get_hex:
	ADD	0x30,R1
	CMP	0x39,R1
	ADD.GT	7,R1	; Add 'A'-'0'-10
	JMP	R0

raw_put_uart:	; R0 is return address, R1 is value to transmit
	STO	R2,8(DBG)
	LDI	INT_UARTTX,R2
	STO	R2,(PIC)	; Clear the PIC, turn off interrupts, etc.
raw_put_uart_retest:
	LOD	(PIC),R2
	TEST	INT_UARTTX,R2
	BZ	raw_put_uart_retest
	STO	R1,(UART)
	LOD	8(DBG),R2
	JMP	R0

	.section	.fixdata
DBG:
.byte	0,0,0,0,0,0,0,0,0

.set	INT_UARTRX, 0x040
.set	INT_UARTTX, 0x080
.set	PIC,     0x0100
.set	BUSERR,  0x0101
.set	TMRA,    0x0102
.set	TMRB,    0x0103
.set	PWM,     0x0104
.set	SPIO,    0x0105
.set	GPIO,    0x0106
.set	UART,    0x0107
.set	VERSION, 0x0108
.set	SCOPE,   0x0200
.set	SCOPED,  0x0201
.set	CLOCK,   0x0800
.set	CONFIG,  0x0400
.set	TIMER,   0x0801
.set	STOPWATCH,0x802
.set	ALARM,   0x0803
.set	RAM,     0x2000
