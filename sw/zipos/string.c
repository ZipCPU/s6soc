////////////////////////////////////////////////////////////////////////////////
//
// Filename: 	string.c
//
// Project:	CMod S6 System on a Chip, ZipCPU demonstration project
//
// Purpose:	To provide *some* of the C-library's capabilities, without
//		using perfectly optimal functions--but rather simple things that
//	can be easily tested and debugged.
//
// Creator:	Dan Gisselquist, Ph.D.
//		Gisselquist Technology, LLC
//
////////////////////////////////////////////////////////////////////////////////
//
// Copyright (C) 2017, Gisselquist Technology, LLC
//
// This program is free software (firmware): you can redistribute it and/or
// modify it under the terms of  the GNU General Public License as published
// by the Free Software Foundation, either version 3 of the License, or (at
// your option) any later version.
//
// This program is distributed in the hope that it will be useful, but WITHOUT
// ANY WARRANTY; without even the implied warranty of MERCHANTIBILITY or
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
// for more details.
//
// You should have received a copy of the GNU General Public License along
// with this program.  (It's in the $(ROOT)/doc directory.  Run make with no
// target there if the PDF file isn't present.)  If not, see
// <http://www.gnu.org/licenses/> for a copy.
//
// License:	GPL, v3, as defined and found on www.gnu.org,
//		http://www.gnu.org/licenses/gpl.html
//
//
////////////////////////////////////////////////////////////////////////////////
//
//
#include "string.h"
#include "txfns.h"

char *
strcpy(char *dst, const char *src) {
	char	v, *d = dst;
	do {
		*dst++ = (v = *src++);
	} while(v);

	return d;
}

char *
strcat(char *dst, const char *src) {
	char	v, *d = dst;
	do {
		v = *dst++;
	} while(v);

	dst--;
	strcpy(dst, src);

	return d;
}

#ifdef	C_STRING_FNS
size_t
strlen(const char *str) {
	size_t	ln = 0;
	while(*str++)
		ln++;
	return ln;
}
#else
asm("\t.section\t.text\n"
	"\t.global\tstrlen\n"
"strlen:\n"
	"\tMOV\tR1,R2\n"
	"\tCLR\tR1\n"
".Lstrlen_loop:\n"
	"\tLB\t(R2),R3\n"
	"\tCMP\t0,R3\n"
	"\tRTN.Z\n"
	"\tADD\t1,R2\n"
	"\tADD\t1,R1\n"
	"\tBRA\t.Lstrlen_loop\n");
#endif

#ifdef	C_STRING_FNS
void	*memcpy(void *dest, const void *src, size_t ln) {
	char	*d = dest; const char *s = src;

/*
	if (((((unsigned)d ^ (unsigned)s)&3)==0)&&(ln>4)) {
		// Source and destination are aligned with each other

		// Align them to a word boundary
		int n = (3-(((unsigned)d)&3));
		ln -= n;
		while(n>0) {
			*d++ = *s++; n--;
		}

		int *id = (int *)d;
		const int *is = (const int *)s;
		while(ln >= 4) {
			*id++ = *is++; ln-=4;
		} d = (char *)id; s = (const char *)is;
	}
*/

	while(ln > 0) {
		*d++ = *s++; ln--;
	} return dest;
}
#else
asm("\t.section\t.text\n"
	"\t.global\tmemcpy\n"
"memcpy:\n"
	"\tCMP	0,R3\n"
	"\tRTN.Z\n"
	"\tMOV\tR1,R4\n"
	"\tSUB	4,SP\n"
	"\tSW	R5,(SP)\n"
	//
	"\tTST\t3,R2\n"
	"\tBZ\t.Lpre_aligned\n"
	"\tLB\t(R2),R5\n"
	"\tSB\tR5,(R4),R5\n"
	"\tADD\t1,R2\n"
	"\tADD\t1,R4\n"
	"\tSUB\t1,R3\n"
	"\tBZ\t.Lmemcpy_epilogue\n"
	//
	"\tTST\t3,R2\n"
	"\tBZ\t.Lpre_aligned\n"
	"\tLB\t(R2),R5\n"
	"\tSB\tR5,(R4),R5\n"
	"\tADD\t1,R2\n"
	"\tADD\t1,R4\n"
	"\tSUB\t1,R3\n"
	"\tBZ\t.Lmemcpy_epilogue\n"
	//
	"\tTST\t3,R2\n"
	"\tBZ\t.Lpre_aligned\n"
	"\tLB\t(R2),R5\n"
	"\tSB\tR5,(R4),R5\n"
	"\tADD\t1,R2\n"
	"\tADD\t1,R4\n"
	"\tSUB\t1,R3\n"
	"\tBZ\t.Lmemcpy_epilogue\n"
	//
".Lpre_aligned:\n"
	"\tTST\t1,R4\n"
	"\tBNZ\t.Lmemcpy_unaligned\n"
	"\tTST\t2,R4\n"
	"\tBNZ\t.Lmemcpy_half_aligned\n"
".Lmemcpy_highspeed:\n"
	"\tSUB\t32,R3\n"
	"\tBLT\t.Lend_of_high_speed\n"
	//
	"\tLW\t(R2),R5\n"
	"\tSW\tR5,(R4)\n"
	"\tLW\t4(R2),R5\n"
	"\tSW\tR5,4(R4)\n"
	"\tLW\t8(R2),R5\n"
	"\tSW\tR5,8(R4)\n"
	"\tLW\t12(R2),R5\n"
	"\tSW\tR5,12(R4)\n"
	//
	"\tLW\t16(R2),R5\n"
	"\tSW\tR5,16(R4)\n"
	"\tLW\t20(R2),R5\n"
	"\tSW\tR5,20(R4)\n"
	"\tLW\t24(R2),R5\n"
	"\tSW\tR5,24(R4)\n"
	"\tLW\t28(R2),R5\n"
	"\tSW\tR5,28(R4)\n"
	//
	"\tADD\t32,R2\n"
	"\tADD\t32,R4\n"
	"\tBRA\t.Lmemcpy_highspeed\n"
".Lend_of_high_speed:\n"
	"\tADD\t32,R3\n"
".Lmemcpy_half_aligned:\n"
	"\tSUB\t4,R3\n"
	"\tBLT\t.Lend_of_short_speed\n"
	"\tLH\t(R2),R5\n"
	"\tSH\tR5,(R4)\n"
	"\tLH\t2(R2),R5\n"
	"\tSH\tR5,2(R4)\n"
	"\tADD\t4,R2\n"
	"\tADD\t4,R4\n"
	"\tBRA\t.Lmemcpy_half_aligned\n"
".Lend_of_short_speed:\n"
	"\tADD\t4,R3\n"
".Lmemcpy_unaligned:\n"
	"\tSUB\t1,R3\n"
	"\tBLT\t.Lmemcpy_epilogue\n"
	"\tLB\t(R2),R5\n"
	"\tSB\tR5,(R4)\n"
	"\tADD\t1,R2\n"
	"\tADD\t1,R4\n"
	"\tBRA\t.Lmemcpy_unaligned\n"
".Lmemcpy_epilogue:\n"
	"\tLW\t(SP),R5\n"
	"\tADD\t4,SP\n"
	"\tRETN\n"
);
/*
asm("\t.section\t.text\n"
	"\t.global\tmemcpy\n"
"memcpy:\n"
	"\tCMP	0,R3\n"
	"\tRTN.Z\n"
	"\tSUB	8,SP\n"
	"\tSW	R5,(SP)\n"
#define	HIGH_SPEED_MEMCPY
#ifdef	HIGH_SPEED_MEMCPY
	"\tSW	R6,4(SP)\n"
	"\tMOV	R1,R4\n"
	"\tXOR	R2,R4\n"
	"\tTEST	3,R4\n"
	"\tBNZ	.Lmemcpy_unaligned\n"
	"\tCMP	8,R3\n"
	"\tBLT	.Lmemcpy_unaligned\n"

	// n = 3+ ~((unsigned)d&3)+1
	"\tMOV\tR1,R4\n"
	"\tAND\t3,R4\n"
	"\tLDI\t3,R5\n"
	"\tSUB\tR4,R5"	"\t; R5 = n\n"
	"\tMOV\tR1,R4"	"\t; R4 = d\n"
	"\tBZ\t.Lmemcpy_n_is_zero\n"

	"\tSUB\tR5,R3"	"\t; ln -= n\n"
	"\tTEST\t1,R5\n"
	"\tLB.NZ\t(R2),R6\n"
	"\tSB.NZ\tR6,(R4)\n"
	"\tADD.NZ\t1,R4\n"
	"\tADD.NZ\t1,R2\n"
	"\tTEST\t2,R5\n"
	"\tLH.NZ\t(R2),R6\n"
	"\tSH.NZ\tR6,(R4)\n"
	"\tADD.NZ\t2,R4\n"
	"\tADD.NZ\t2,R2\n"

".Lmemcpy_n_is_zero:\n"
".Lmemcpy_word_loop:\n"
	"\tSUB\t4,R3\n"
	"\tBLT\t.Lmemcpy_pretail\n"
	"\tLW\t(R4),R5\n"
	"\tSW\tR5,(R2)\n"
	"\tADD\t4,R4\n"
	"\tADD\t4,R2\n"
	"\tBRA\t.Lmemcpy_word_loop\n"
".Lmemcpy_pretail:\n"
	"\tADD\t4,R3\n"
	"\tBZ\t.Lmemcpy_epilogue\n"
	"\tBRA\t.Lmemcpy_unaligned_loop\n"

".Lmemcpy_unaligned:\n"
	"\tCMP	0,R3\n"
	"\tBZ	.Lmemcpy_epilogue\n"
#endif
	"\tMOV\tR1,R4\n"
".Lmemcpy_unaligned_loop:\n"
	"\tLB\t(R2),R5\n"
	"\tSB\tR5,(R4)\n"
	"\tADD\t1,R4\n"
	"\tADD\t1,R2\n"
	"\tSUB\t1,R3\n"
	"\tBNZ\t.Lmemcpy_unaligned_loop\n"
".Lmemcpy_epilogue:\n"
	"\tLW\t(SP),R5\n"
#ifdef	HIGH_SPEED_MEMCPY
	"\tLW\t4(SP),R6\n"
#endif
	"\tADD\t8,SP\n"
	"\tRTN\n"
);
*/
#endif


