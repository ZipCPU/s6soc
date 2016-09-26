################################################################################
##
## Filename: 	Makefile
##
## Project:	CMod S6 System on a Chip, ZipCPU demonstration project
##
## Purpose:	To coordinate the build of a small ZipOS.
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

CROSS   := zip-
CC      := $(CROSS)gcc
AS	:= $(CROSS)as
OBJDUMP := $(CROSS)objdump
OBJDIR  := obj-zip
DEVSRCSR:= display.c keypad.c rtcsim.c
DEVSRCS := $(addprefix ../dev/,$(DEVSRCSR))
DEVOBJS := $(addprefix $(OBJDIR)/,$(subst .c,.o,$(DEVSRCSR)))
SOURCES := bootloader.c kernel.c ksetup.c syspipe.c pipesetup.c taskp.c doorbell.c zipsys.c # ziplib.c
OBJECTS := $(addprefix $(OBJDIR)/,$(subst .c,.o,$(SOURCES))) $(DEVOBJS) $(OBJDIR)/resetdump.o
HEADERS := $(wildcard *.h) $(subst .c,.h,$(DEVSRCS))
# CFLAGS  := -O3 -fdump-tree-all -Wall -Wextra -nostdlib -fno-builtin
# CFLAGS  := -O3 -fdump-rtl-all -DZIPOS -Wall -Wextra -nostdlib -fno-builtin
CFLAGS  := -O3 -DZIPOS -Wall -Wextra -nostdlib -fno-builtin
# CFLAGS  := -Wall -Wextra -nostdlib -fno-builtin
LDFLAGS := -T cmodram.ld -Wl,-Map,$(OBJDIR)/doorbell.map

all: $(OBJDIR)/ doorbell
$(OBJDIR)/:
	@bash -c "if [ ! -e $(OBJDIR) ]; then mkdir -p $(OBJDIR); fi"

%.o: $(OBJDIR)/%.o
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/%.o: ../dev/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/%.txt: $(OBJDIR)/%.o
	$(OBJDUMP) -d $^ -o $@

$(OBJDIR)/%.s: %.c
	$(CC) -S $(CFLAGS) -c $< -o $@

$(OBJDIR)/%.s: ../dev/%.c
	$(CC) -S $(CFLAGS) -c $< -o $@

$(OBJDIR)/resetdump.o: resetdump.s
	$(AS) $^ -o $@

doorbell: $(OBJECTS) cmodram.ld
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

$(OBJDIR)/doorbell.txt: doorbell
	$(OBJDUMP) -d $^ > $@

define	build-depends
	@echo "Building dependency file(s)"
	@$(CC) $(CPPFLAGS) -MM $(SOURCES) $(DEVSRCS) > $(OBJDIR)/xdepends.txt
	@sed -e 's/^.*.o: /$(OBJDIR)\/&/' < $(OBJDIR)/xdepends.txt > $(OBJDIR)/depends.txt
	@rm $(OBJDIR)/xdepends.txt
endef

.PHONY: depends
depends: $(OBJDIR)/ tags
	$(build-depends)

tags: $(SOURCES) $(HEADERS)
	@echo "Generating tags"
	@ctags $(SOURCES) $(DEVSRCS) $(HEADERS)

.PHONY: clean
clean:
	rm -rf $(OBJDIR)/ doorbell

-include $(OBJDIR)/depends.txt
