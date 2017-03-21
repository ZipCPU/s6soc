;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Filename: 	asmstartup.s
;;
;; Project:	CMod S6 System on a Chip, ZipCPU demonstration project
;;
;; Purpose:	A small assembly routine, designed to place the startup code
;;		into the very beginning of the program space (i.e. crt0.s). 
;;	This startup code *must* start at the RESET_ADDRESS of the ZipCPU.  It
;;	does two things: 1) loads a valid stack pointer, and 2) jumps to the
;;	entry point in the program which (as a result of this startup code)
;;	may be anywhere in the address space.
;;
;; Creator:	Dan Gisselquist, Ph.D.
;;		Gisselquist Technology, LLC
;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
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
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;;
	.section .start,"ax",@progbits
	.global	_start
	.type	_start,@function
_start:
	NDUMP
	LDI	255,R0		; Turn all the LEDs on
	SW	R0,(SPIO)
	MOV	kernel_exit(PC),uPC
	LDI	_top_of_stack,SP
	JSR	entry
	NEXIT	R0		; Exit on return if in a simulator
kernel_exit:
	HALT			; Otherwise just halt the CPU
	BRA	kernel_exit	; In case were called from user mode

.set	SPIO, 0x0414
