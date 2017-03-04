################################################################################
#
# Filename:	Makefile
#
# Project:	Zip CPU -- a small, lightweight, RISC CPU soft core
#
# Purpose:	This makefile builds the final verilator simulation of the
#		zipsystem.  Specifically, it builds the final C++ portion
#	of the simulator, and thus the final simulator executable.
#
#	This simulator depends upon the libelf library.
#
# Creator:	Dan Gisselquist, Ph.D.
#		Gisselquist Technology, LLC
#
################################################################################
#
# Copyright (C) 2015,2017, Gisselquist Technology, LLC
#
# This program is free software (firmware): you can redistribute it and/or
# modify it under the terms of  the GNU General Public License as published
# by the Free Software Foundation, either version 3 of the License, or (at
# your option) any later version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTIBILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
# for more details.
#
# License:	GPL, v3, as defined and found on www.gnu.org,
#		http://www.gnu.org/licenses/gpl.html
#
#
################################################################################
#
all: zip_sim

CXX	:= g++
CFLAGS	:= -Wall -Og -g
OBJDIR  := obj-pc
SWHOST	:= ../../sw/host
RTLD	:= ../../rtl
RTLOBJD := $(RTLD)/obj_dir
VERILATOR_ROOT ?= $(shell bash -c 'verilator -V|grep VERILATOR_ROOT | head -1 | sed -e " s/^.*=\s*//"')
VROOT   := $(VERILATOR_ROOT)
VINCS	:= -I$(VROOT)/include -I$(VROOT)/include/vltstd
INCS	:= -I$(RTLOBJD) -I$(RTLD) -I$(VROOT)/include -I$(SWHOST) $(VINCS)
SOURCES := zip_sim.cpp twoc.cpp qspiflashsim.cpp uartsim.cpp
VLSRCS	:= verilated.cpp verilated_vcd_c.cpp
VLOBJS := $(OBJDIR)/verilated.o $(OBJDIR)/verilated_vcd_c.o
VLIB	:= $(addprefix $(VROOT)/include/,$(VLSRCS))
RAWLIB	:= $(RTLOBJD)/Vbusmaster__ALL.a
OBJECTS := $(addprefix $(OBJDIR)/,$(subst .cpp,.o,$(SOURCES) $(VLSRCS)))
LIBS	:= $(RAWLIB) -lelf
TESTF	:= $(ZASM)/z.out
DHRYSTONEF := ../asm/zipdhry.z

$(OBJDIR)/%.o: %.cpp
	$(CXX) $(CFLAGS) $(INCS) -c $< -o $@

$(OBJDIR)/%.o: $(VROOT)/include/%.cpp
	$(CXX) $(CFLAGS) $(INCS) -c $< -o $@

zip_sim: $(OBJECTS)
	$(CXX) $(CFLAGS) $(INCS) $(OBJECTS) $(LIBS) -o $@




define	build-depends
	@echo "Building dependencies"
	@$(CXX) $(CFLAGS) $(INCS) -MM $(VLIB) $(SOURCES) > $(OBJDIR)/xdepends.txt
	@sed -e 's/^.*.o: /$(OBJDIR)\/&/' < $(OBJDIR)/xdepends.txt > $(OBJDIR)/depends.txt
	@rm $(OBJDIR)/xdepends.txt
endef

tags: $(VLIB) $(SOURCES)
	@ctags $(SOURCES) $(VLIB)

.PHONY: depends
depends: tags $(OBJDIR)/
	$(build-depends)

$(OBJDIR)/:
	@bash -c "if [ ! -e $(OBJDIR) ]; then mkdir -p $(OBJDIR)/; fi"

$(OBJDIR)/depends.txt: $(OBJDIR)/ depends

.PHONY: clean
clean:
	rm -rf $(OBJDIR)/
	rm -f ./zip_sim

-include $(OBJDIR)/depends.txt
