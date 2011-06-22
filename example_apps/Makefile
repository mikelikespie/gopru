-include DIRS

CCTOOLS?=arm-arago-linux-gnueabi-gcc
LIBDIR_APP_LOADER?=../../app_loader/lib
LIBDIR_EDMA_DRIVER?=../../peripheral_lib/edma_driver/lib/
INCDIR_APP_LOADER?=../../app_loader/include
INCDIR_EDMA_DRIVER?=../../peripheral_lib/edma_driver/include
BINDIR_APPLICATIONS?=../bin
BINDIR_FW?=bin
UTILS_DIR?=../utils

all:
# Make PRU example applications
	mkdir -p bin

	for dir in $(APP_DIRS); do make -C $$dir CCTOOLS="${CCTOOLS}" LIBDIR_APP_LOADER="${LIBDIR_APP_LOADER}" LIBDIR_EDMA_DRIVER="${LIBDIR_EDMA_DRIVER}" INCDIR_APP_LOADER="${INCDIR_APP_LOADER}" INCDIR_EDMA_DRIVER="${INCDIR_EDMA_DRIVER}" BINDIR="${BINDIR_APPLICATIONS}"; done

# Pass PRU assembly code for each example through assembler
	for a_file in ${ASSEM_FILES} ; \
	do \
          ${UTILS_DIR}/pasm -b $$a_file ; \
	done ; \
	mv *.bin ${BINDIR_FW}

clean:
	for dir in $(APP_DIRS); do make -C $$dir clean LIBDIR_APP_LOADER="${LIBDIR_APP_LOADER}" LIBDIR_EDMA_DRIVER="${LIBDIR_EDMA_DRIVER}" INCDIR_APP_LOADER="${INCDIR_APP_LOADER}" INCDIR_EDMA_DRIVER="${INCDIR_EDMA_DRIVER}" BINDIR="${BINDIR_APPLICATIONS}"; done

	for bin_file in ${BIN_FILES}; do rm -fr ${BINDIR_FW}/$$bin_file; done

