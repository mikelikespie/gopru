ROOTDIR = ../
-include $(ROOTDIR)/Rules.make

INCDIR ?= <full-path-to can/module/hal/can/include> -I<full-path-to pru_lib/hal/common/include -I<full-path-to can/module/>

EXTRA_CFLAGS += -I${INCDIR}

can_emu-objs := ti_omapl_pru_can.o \
		  ../../pru_lib/hal/common/src/pru.o \
                  hal/can/src/pru_can_emulation_api.o
                  
obj-m	+= can_emu.o

MAKE_ENV = ARCH=arm CROSS_COMPILE=$(CSTOOL_PREFIX)


debug: EXTRA_CFLAGS += -D__DEBUG

.PHONY: clean debug release install default

default: release

docs:

release:
	make -C $(KERNEL_DIR) M=`pwd` $(MAKE_ENV) EXTRA_CFLAGS="$(EXTRA_CFLAGS)" modules

debug:
	make $(INCLUDES) -C $(KERNEL_DIR) M=`pwd` $(MAKE_ENV) \
		EXTRA_CFLAGS="$(EXTRA_CFLAGS)" modules

install:
	install -d $(EXEC_DIR)
	install -c $(TARGET) $(EXEC_DIR)

clean:
	rm -rf *.o *mod* *.ko .edma* .tmp*  Module.symvers
