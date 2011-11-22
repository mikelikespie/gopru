-include ../../Rules.make

# export variables for compatibility with lower-level makefiles
export CCTOOL_PREFIX=$(CROSS_COMPILE)
export KERNEL_DIR=$(LINUXKERNEL_INSTALL_DIR)
export CCTOOLS=$(CROSS_COMPILE)gcc

# set a default DESTDIR if one was not provided
DESTDIR ?= ./temp

all: libprussdrv edma pru_examples
clean: libprussdrv_clean edma_clean pru_examples_clean

libprussdrv:
	cd app_loader/interface; make release

libprussdrv_clean:
	cd app_loader/interface; make clean

edma:
	cd peripheral_lib/edma_driver/module; make release
	cd peripheral_lib/edma_driver/interface; make release

edma_clean:
	cd peripheral_lib/edma_driver/module; make clean
	cd peripheral_lib/edma_driver/interface; make clean

pru_examples: libprussdrv
	cd example_apps; make

pru_examples_clean:
	cd example_apps; make clean

install: pru_examples edma
	@install -d $(DESTDIR)/usr/share/ti/ti-pru-eg
	cp -f example_apps/bin/* $(DESTDIR)/usr/share/ti/ti-pru-eg/
	@install -d $(DESTDIR)/lib/modules/2.6.37/kernel/drivers/pru/
	cp -f peripheral_lib/edma_driver/module/edmautils.ko $(DESTDIR)/lib/modules/2.6.37/kernel/drivers/pru/
