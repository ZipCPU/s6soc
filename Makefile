##############################################################################//
##
## Filename: 	Makefile
##
## Project:	CMod S6 System on a Chip, ZipCPU demonstration project
##
## Purpose:	An initial attempt at a master project makefile.  Does not
##		yet support subdirectory recursion, so it currently does
##	little more than make a tar file or a date stamp.
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
all:	datestamp archive rtl sw
# BENCH := `find bench -name Makefile` `find bench -name "*.cpp"` `find bench -name "*.h"`
BENCH :=
RTL   := `find rtl -name "*.v"` `find rtl -name Makefile`
NOTES := `find doc -name "*.txt"` `find doc -name "*.html"` `ls *.txt`
SW    := `find sw -name "*.cpp"` `find sw -name "*.h"`	\
	`find sw -name "*.c"` `find sw -name "*.sh"`	\
	`find sw -name "*.pl"` `find sw -name Makefile`
# PROJ  := xilinx/xula.prj xilinx/xula.xise xilinx/xula.xst	\
#	xilinx/xula.ut xilinx/Makefile
PROJ	:=
BIN	:= `find xilinx -name "*.bit"`
CONSTRAINTS := cmod.ucf
YYMMDD	:= `date +%Y%m%d`

.PHONY: datestamp
datestamp:
	@bash -c 'if [ ! -e $(YYMMDD)-build.v ]; then rm -f 20??????-build.v; perl mkdatev.pl > $(YYMMDD)-build.v; rm -f rtl/builddate.v; fi'
	@bash -c 'if [ ! -e rtl/builddate.v ]; then cd rtl; cp ../$(YYMMDD)-build.v builddate.v; fi'

.PHONY: rtl
rtl:
	@make --no-print-directory -C rtl

.PHONY: sw
sw:
	@make --no-print-directory -C sw

.PHONY: doc
doc:
	@make --no-print-directory -C doc

.PHONY: bench
bench: rtl
	@make --no-print-directory -C bench/cpp

.PHONY: list-archive-rtl
list-archive-rtl:
	echo $(RTL)

.PHONY: list-archive-sw
list-archive-sw:
	echo $(SW)

.PHONY: list-archive-bin
list-archive-bin:
	echo $(BIN)

.PHONY: list-archive-notes
list-archive-notes:
	echo $(NOTES)

.PHONY: list-archive-proj
list-archive-proj:
	echo $(PROJ)

.PHONY: list-archive
list-archive: list-archive-sw list-archive-rtl list-archive-notes list-archive-proj list-archive-bin

.PHONY: archive
archive:
	tar --transform s,^,$(YYMMDD)-s6/, -chjf $(YYMMDD)-s6.tjz $(BENCH) $(SW) $(RTL) $(NOTES) $(PROJ) $(BIN) $(CONSTRAINTS)

# .PHONY: bit
# bit:
#	make --no-print-directory -C xilinx toplevel.bit

axload:
	djtgcfg enum
	djtgcfg init -d CmodS6
	djtgcfg prog  -d CmodS6 -i 0 -f xilinx/alttop.bit
# Might also be able to do a ...
#   djtgcfg erase -d CmodS6 -i 0
# but I can't speak to whether it would be useful or not.

xload:
	djtgcfg init -d CmodS6
	djtgcfg prog -d CmodS6 -i 0 -f xilinx/toplevel.bit

# Fload really depends upon axload, but we'll ignore that here.
fload:
	sw/host/zipload xilinx/toplevel.bit sw/zipos/doorbell
