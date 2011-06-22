ROOTDIR = ..
TARGET = libedmautils

include $(ROOTDIR)/Rules.make

CC = $(CCTOOL_PREFIX)gcc
AR = $(CCTOOL_PREFIX)ar

INCLUDEDIR = ../include
LIBDIR	   = ../lib

C_FLAGS += -I. -Wall -I$(INCLUDEDIR) -I$(KERNEL_DIR)/include 

COMPILE.c = $(CC) $(C_FLAGS) $(CPP_FLAGS) -c
AR.c	  = $(AR) rc

DBGTARGET = $(LIBDIR)/$(TARGET)d.a
RELTARGET = $(LIBDIR)/$(TARGET).a

DBGCFLAGS = -g -O0 -D__DEBUG
RELCFLAGS = -O3  -mtune=arm926ej-s -march=armv5te

SOURCES = $(wildcard *.c)
HEADERS = $(wildcard *.h)

TARGETHEADERS = $(addprefix $(INCLUDEDIR)/, $(HEADERS))

DBGOBJFILES = $(SOURCES:%.c=debug/%.o) 
RELOBJFILES = $(SOURCES:%.c=release/%.o) 

.PHONY: clean debug release install

all:	debug release

install:

release:	$(RELTARGET) 
#release:	$(RELTARGET) $(TARGETHEADERS)

debug:		$(DBGTARGET)
#debug:		$(DBGTARGET) $(TARGETHEADERS)


$(RELTARGET):	$(RELOBJFILES)
	@mkdir -p $(ROOTDIR)/lib
	$(AR.c) $@ $(RELOBJFILES)

$(DBGTARGET):	$(DBGOBJFILES) 
	@mkdir -p $(ROOTDIR)/lib
	$(AR.c) $@ $(DBGOBJFILES)

$(RELOBJFILES):	release/%.o: %.c $(HEADERS)
	@mkdir -p release
	$(COMPILE.c) $(RELCFLAGS) -o $@ $<

$(DBGOBJFILES):	debug/%.o: %.c $(HEADERS)
	@mkdir -p debug
	$(COMPILE.c) $(DBGCFLAGS) -o $@ $<

$(TARGETHEADERS): $(HEADERS)
	@echo Installing headers...
	@install -d $(INCLUDEDIR)
	@install -c $< $@

clean:
	-rm -rf release debug *~ ../lib/*

