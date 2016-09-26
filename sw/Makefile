##############################################################################//
##
## Filename: 	Makefile
##
## Project:	CMod S6 System on a Chip, ZipCPU demonstration project
##
## Purpose:	A speedbump on the way to building the software for the various
##		support directories--simply coordinating that.  This makefile
##	allows the prior directory to issue a make in this directory, while
##	not needing to worry about subdirectories--which this file handles.
##
## Creator:	Dan Gisselquist, Ph.D.
##		Gisselquist Technology, LLC
##
##############################################################################//
##
## Copyright (C) 2015-2016, Gisselquist Technology, LLC
##
## This program is free software (firmware): you can redistribute it and/or
## modify it under the terms of  the GNU General Public License as published
## by the Free Software Foundation, either version 3 of the License, or (at
## your option) any later version.
##
## This program is distributed in the hope that it will be useful, but WITHOUT
## ANY WARRANTY; without even the implied warranty of MERCHANTIBILITY or
## FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
## for more details.
##
## You should have received a copy of the GNU General Public License along
## with this program.  (It's in the $(ROOT)/doc directory, run make with no
## target there if the PDF file isn't present.)  If not, see
## <http://www.gnu.org/licenses/> for a copy.
##
## License:	GPL, v3, as defined and found on www.gnu.org,
##		http://www.gnu.org/licenses/gpl.html
##
##
##############################################################################//
##
##
.PHONY: all
all:	host dev zipos

.PHONY: host dev zipos clean depends

host:
	@bash -c 'if [ -e host/Makefile ]; then make --no-print-directory -C host; fi'
dev:
	@bash -c 'if [ -e dev/Makefile ]; then make --no-print-directory -C dev; fi'
zipos:
	@bash -c 'if [ -e zipos/Makefile ]; then make --no-print-directory -C zipos; fi'

clean:
	@bash -c 'if [ -e host/Makefile ]; then make --no-print-directory -C host clean; fi'
	@bash -c 'if [ -e dev/Makefile ]; then make --no-print-directory -C dev clean; fi'
	@bash -c 'if [ -e zipos/Makefile ]; then make --no-print-directory -C zipos clean; fi'

depends:
	@bash -c 'if [ -e host/Makefile ]; then make --no-print-directory -C host depends; fi'
	@bash -c 'if [ -e dev/Makefile ]; then make --no-print-directory -C dev depends; fi'
	@bash -c 'if [ -e zipos/Makefile ]; then make --no-print-directory -C zipos depends; fi'

SNDFILE=$(HOME)/sounds/doorbell_x.wav

.PHONY: doorbell
doorbell:
	@bash -c "if [ ! -e doorbell.wav ]; then mpg321 $(SNDFILE) -w doorbell.wav; fi"
	sox doorbell.wav -t raw -e signed -b 16 -c 1 -r 8k doorbell.16t 

doorbell.16t: doorbell
dev/samples.c: doorbell.16t host/buildsamples
	host/buildsamples doorbell.16t > dev/samples.c

play:
	play -q -t raw -e signed -b 16 -c 1 -r 8k doorbell.16t

