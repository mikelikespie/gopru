gopru
=====

Pure go implementation of pru app loader for beaglebone black. 
[C source of Pruss app loader](https://github.com/beagleboard/am335x_pru_package/tree/master/pru_sw/app_loader) (the inspiration for this)

The quality of this code is hacked at best (at the moment).  See `prussdrv_test.go` for usage.


pasm wrapper
------------
the `pasm` directory contains a port of the pasm assembler (from [https://github.com/beagleboard/am335x_pru_package/tree/master/pru_sw/utils/pasm_source](here)).  It is modified to be callable via cgo.  This allows for testing to be much easier for gopru.  (see `prussdrv_test.go` for example)
