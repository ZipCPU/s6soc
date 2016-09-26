################################################################################
##
## Filename: 	Makefile	(sw/host)
##
## Project:	CMod S6 System on a Chip, ZipCPU demonstration project
##
## Purpose:	Supports the building of the various host (i.e. PC) software
##		that will be used to support the Cmod S6 board.  None of this
##	software will actually run on the board, but it may be used for 
##	communicating with and configuring the board.
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
PROGRAMS := wbregs readflash zipload buildsamples dumpuart
all: $(PROGRAMS)

CXX := g++
OBJDIR := obj-pc
# BUSSRCS:= deppbus.cpp regdefs.cpp
BUSSRCS:= deppi.cpp regdefs.cpp llcomms.cpp ttybus.cpp
SOURCES:= wbregs.cpp readflash.cpp zipload.cpp flashdrvr.cpp # flashload.cpp/ziprun.cpp
ADEPT := ../../../refs/digilent.adept.sdk_2.3.1
HEADERS:= deppbus.h devbus.h flashdrvr.h regdefs.h twoc.h
OBJECTS:= $(addprefix $(OBJDIR)/,$(subst .cpp,.o,$(SOURCES)))
BUSOBJS:= $(addprefix $(OBJDIR)/,$(subst .cpp,.o,$(BUSSRCS)))
CFLAGS := -g -Wall -I$(ADEPT)/inc -I.
LIBS := -L /usr/lib64/digilent/adept -ldmgr -ldepp

%.o: $(OBJDIR)/ $(OBJDIR)/%.o
$(OBJDIR)/%.o: %.cpp
	$(CXX) $(CFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -rf $(OBJDIR)/ $(PROGRAMS)

wbregs: $(OBJDIR)/wbregs.o $(BUSOBJS)
	$(CXX) $(CFLAGS) $^ $(LIBS) -o $@

scope: $(OBJDIR)/scope.o $(BUSOBJS)
	$(CXX) $(CFLAGS) $^ $(LIBS) -o $@

readflash: $(OBJDIR)/readflash.o $(BUSOBJS)
	$(CXX) $(CFLAGS) $^ $(LIBS) -o $@

zipload: $(OBJDIR)/zipload.o $(OBJDIR)/flashdrvr.o $(BUSOBJS)
	$(CXX) $(CFLAGS) $^ $(LIBS) -lelf -o $@

buildsamples: buildsamples.cpp
	$(CXX) $(CFLAGS) $^ -o $@

dumpuart: dumpuart.cpp
	$(CXX) $(CFLAGS) $^ -o $@

define	build-depends
	@echo "Building dependency file(s)"
	@$(CXX) $(CFLAGS) -MM $(SOURCES) $(BUSSRCS) > $(OBJDIR)/xdepends.txt
	@sed -e 's/^.*.o: /$(OBJDIR)\/&/' < $(OBJDIR)/xdepends.txt > $(OBJDIR)/depends.txt
	@rm $(OBJDIR)/xdepends.txt
endef

tags: $(SOURCES) $(HEADERS)
	@ctags $(SOURCES) $(HEADERS)

.PHONY: depends
depends: tags
	$(build-depends)

$(OBJDIR)/depends.txt: $(OBJDIR)/ $(SOURCES) $(HEADERS)
	$(build-depends)

$(OBJDIR)/:
	@bash -c "if [ ! -e $(OBJDIR) ]; then mkdir -p $(OBJDIR); fi"

-include $(OBJDIR)/depends.txt
