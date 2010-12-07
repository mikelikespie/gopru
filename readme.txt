The pru_sw package is organized as follows:  

|--\
    |
    |--\app_loader	Contains user space library (PRUSSDRV) for loading and interacting 
    |			with PRU in user space.
    |
    |--\example_apps	Contains example PRU assembly code and associated code to build, 
    |			load, and test the PRU code.
    |
    |--\kernel_loader	Contains kernel drivers and kernel-level code used to interact with the PRU.
    |
        |--\can		Contains out-of-kernel CAN module and associated kernel patches, 
        |			hal include files, and PRU firmware.
        |
        |--\pru_lib	Contains API's for loading and interacting with PRU at kernel-level.
        |
        |--\uio		Contains kernel patch for uio_pru module that sets up resources 
        |			for the app_loader.
        |
        |--\suart	Contains out-of-kernel soft-UART module and associated kernel 
        |			patches, hal include files, and PRU firmware.
        |
    |--\peripheral_lib	Contains peripheral libraries needed by example_apps.
    |
    |--\utils		Contains PRU assembler for Linux system.
    |    

The CAN and soft-UART source code and documentation (L13x-daughter-card-documentation-software)
can be downloaded from Mistral's website (http://mistralsolutions.com/).  The patches and source code 
hosted on the Mistral website configures the CAN and soft-UART modules as in-kernel modules.  
The patches and source code in the pru_sw package configure the CAN and soft-UART modules as 
out-of-kernel modules. For more information on these modules, visit Mistral's website 
(http://mistralsolutions.com/).
 
For addition details about the pru_sw package, refer to 
http://processors.wiki.ti.com/index.php/PRU_Linux_Loader.
