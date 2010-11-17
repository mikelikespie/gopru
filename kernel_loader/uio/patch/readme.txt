Linux patch to add UIO driver on PSP Linux release 03.20.00.12

Linux kernel available at - http://software-dl.ti.com/dsps/dsps_public_sw/psp/LinuxPSP/DaVinci_03_20/03_20_00_12/index_FDS.html

Steps: patch -p1 < uio_pruss.patch

Link for the latest patch & driver is - http://www.india.ti.com/~pspcm/armmpufee/OMAP-L138_SW/PRU/

The procedure to build (as built-in)
make ARCH=arm CROSS_COMPILE=arm-none-linux-gnueabi- da850_omapl138_defconfig
make ARCH=arm CROSS_COMPILE=arm-none-linux-gnueabi- menuconfig -> Device Drivers ---> Userspace I/O drivers  ---> <*>   Texas Instruments PRUSS driver
make ARCH=arm CROSS_COMPILE=arm-none-linux-gnueabi- uImage; copy arch/arm/boot/uImage to your tftp path

Power on the board & ensure that the latest built image is selected.
Check at file system the following should be visible - cat /sys/class/uio/uio0/maps/map0/addr

The procedure to build (as module)
make ARCH=arm CROSS_COMPILE=arm-none-linux-gnueabi- da850_omapl138_defconfig make ARCH=arm CROSS_COMPILE=arm-none-linux-gnueabi- uImage; copy arch/arm/boot/uImage to your tftp path make ARCH=arm CROSS_COMPILE=arm-none-linux-gnueabi- modules make ARCH=arm CROSS_COMPILE=arm-none-linux-gnueabi- INSTALL_MOD_PATH=<nfs path> modules_install

Power on the board & ensure that the latest built image is selected.
Load the module via
modprobe uio_pru		-> This will work only if you had done modules install above. Else use insmod <lib path>

Check at file system the following should be visible - cat /sys/class/uio/uio0/maps/map0/addr