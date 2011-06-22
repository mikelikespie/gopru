ROOTDIR = ../
-include $(ROOTDIR)/Rules.make

obj-m += edmautils.o

EXTRA_CFLAGS +=	-DUSE_UDEV=$(USE_UDEV) 

MAKE_ENV = ARCH=arm CROSS_COMPILE=$(CCTOOL_PREFIX)


debug: EXTRA_CFLAGS += -D__DEBUG

.PHONY: clean debug release install default

default: release

docs:

release:
	make -C $(KERNEL_DIR) M=`pwd` $(MAKE_ENV) \
		EXTRA_CFLAGS="$(EXTRA_CFLAGS)" modules

debug:
	make $(INCLUDES) -C $(KERNEL_DIR) M=`pwd` $(MAKE_ENV) \
		EXTRA_CFLAGS="$(EXTRA_CFLAGS)" modules

install:
	install -d $(EXEC_DIR)
	install -c $(TARGET) $(EXEC_DIR)

clean:
	rm -rf *.o *mod* *.ko .edma* .tmp*  Module.symvers
