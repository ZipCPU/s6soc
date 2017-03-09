################################################################################
##
## Filename: 	Makefile	(sw/dev)
##
## Project:	CMod S6 System on a Chip, ZipCPU demonstration project
##
## Purpose:	This makefile supports (directs) the building of the various
##		software for the S6 Cmod board.
##
##	Targets:
##
##	(all)		Builds all of the program files
##
##	clean		Removes all object files, the dependency file,
##				and any programs that have been built.
##
##	depends		Builds a master dependency file
##
## Creator:	Dan Gisselquist, Ph.D.
##		Gisselquist Technology, LLC
##
################################################################################
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
## <http:##www.gnu.org/licenses/> for a copy.
##
## License:	GPL, v3, as defined and found on www.gnu.org,
##		http://www.gnu.org/licenses/gpl.html
##
##
################################################################################
##
##
all:
PROGRAMS := helloworld doorbell doorbell2 kptest blinky
all: $(OBJDIR)/ $(PROGRAMS)


OBJDIR := obj-zip
CROSS := zip
CC    := $(CROSS)-gcc
AS    := $(CROSS)-as
LD    := $(CROSS)-ld
SED   := sed
OBJDUMP := $(CROSS)-objdump

# Not for build, for for building tags and dependency files, we need to know
# what the sources and headers are
DEVDRVR:= keypad.c display.c rtcsim.c txfns.c
SOURCES:= helloworld.c doorbell.c doorbell2.c kptest.c $(DEVDRVR)
HEADERS:= board.h
OBJDRVR := $(addprefix $(OBJDIR)/,$(subst .c,.o,$(DEVDRVR)))

CPPFLAGS := -I../zipos -I.
DUMPFLAGS := # -fdump-rtl-all
CFLAGS   := $(CPPFLAGS) $(DUMPFLAGS) -O3 -Wall -Wextra -nostdlib -fno-builtin -Wa,-nocis
LDFLAGS   = -T cmod.ld -Wl,-Map,$(OBJDIR)/$@.map -Wl,--unresolved-symbols=report-all -nostdlib

$(OBJDIR)/:
	$(mk-objdir)

%.o: $(OBJDIR)/%.o

$(OBJDIR)/%.o: %.c
	$(mk-objdir)
	$(CC) $(CFLAGS) -c $< -o $@
$(OBJDIR)/%.o: ../zipos/%.c
	$(mk-objdir)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/%.s: %.c
	$(mk-objdir)
	$(CC) -S $(CFLAGS) -c $< -o $@

$(OBJDIR)/%.txt: $(OBJDIR)/%.o
	$(mk-objdir)
	$(OBJDUMP) -dr $^ > $@
%.txt: %
	$(OBJDUMP) -Dr $< > $@


helloworld: $(OBJDIR)/helloworld.o cmod.ld
	$(CC) $(LDFLAGS) $(OBJDIR)/helloworld.o -o $@
$(OBJDIR)/helloworld.txt: helloworld
	$(OBJDUMP) -dr $^ > $@

doorbell: $(OBJDIR)/doorbell.o cmod.ld
	$(CC) $(LDFLAGS) $(OBJDIR)/doorbell.o -o $@

doorbell2: $(OBJDIR)/doorbell2.o $(OBJDRVR) $(OBJDIR)/string.o cmod.ld
	$(CC) $(LDFLAGS) $(OBJDIR)/doorbell2.o $(OBJDRVR) $(OBJDIR)/string.o -o $@
$(OBJDIR)/doorbell2.txt: doorbell2
	$(OBJDUMP) -dr $^ > $@

KPSRCS := kptest.c keypad.c txfns.c
KPOBJS := $(addprefix $(OBJDIR)/,$(subst .c,.o,$(KPSRCS)))
kptest: $(KPOBJS) cmod.ld
	$(CC) $(LDFLAGS) $(KPOBJS) -o $@
$(OBJDIR)/kptest.txt: kptest
	$(OBJDUMP) -dr $^ > $@

blinky: $(OBJDIR)/blinky.o cmod.ld
	$(CC) $(LDFLAGS) $(OBJDIR)/blinky.o -o $@

define	mk-objdir
	@bash -c "if [ ! -e $(OBJDIR) ]; then mkdir -p $(OBJDIR); fi"
endef

define	build-depends
	@echo "Building dependency file(s)"
	$(CC) $(CPPFLAGS) -MM $(SOURCES) > $(OBJDIR)/xdep.txt
	$(SED) -e 's/^.*.o: /$(OBJDIR)\/&/' < $(OBJDIR)/xdep.txt > $(OBJDIR)/depends.txt
	@rm $(OBJDIR)/xdep.txt
endef

.PHONY: depends
depends: $(OBJDIR)/ tags
	$(build-depends)

tags: $(SOURCES) $(HEADERS)
	@echo "Generating tags"
	@ctags $(SOURCES) $(HEADERS)

.PHONY: clean
clean:
	rm -rf $(OBJDIR)/ $(PROGRAMS)

-include $(OBJDIR)/depends.txt
