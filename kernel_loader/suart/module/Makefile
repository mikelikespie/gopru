ROOTDIR = ../
-include $(ROOTDIR)/Rules.make

INCDIR ?= <full-path-to suart/module/hal/uart/include> -I<full-path-to suart/pru/hal/common/include> -I<full-path-to suart/module>


EXTRA_CFLAGS += -I${INCDIR}

suart_emu-objs := 	ti_omapl_pru_suart.o \
				hal/uart/src/suart_api.o \
				hal/uart/src/suart_utils.o \
				../../pru_lib/hal/common/src/pru.o

obj-m	+= suart_emu.o

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
