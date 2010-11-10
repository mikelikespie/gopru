/*
 * prussdrv.c
 *
 * ============================================================================
 * Copyright (c) Texas Instruments Inc 2010-11
 *
 * Use of this software is controlled by the terms and conditions found in the
 * license agreement under which this software has been supplied or provided.
 * ============================================================================
 */


#include <prussdrv.h>
#include "__prussdrv.h"
#include <pthread.h>


static tprussdrv prussdrv;
static pthread_mutex_t prussdrv_send_event_mutex =
    PTHREAD_MUTEX_INITIALIZER;
//static pthread_mutex_t prussdrv_clear_event_mutex = PTHREAD_MUTEX_INITIALIZER;

int __prussdrv_memmap_init(void)
{
    int i, fd;
    char hexstring[20];

    if (prussdrv.mmap_fd == 0) {
        for (i = 0; i < NUM_PRU_HOSTIRQS; i++) {
            if (prussdrv.fd[i])
                break;
        }
        if (i == NUM_PRU_HOSTIRQS)
            return -1;
        else
            prussdrv.mmap_fd = prussdrv.fd[i];
    }
//	printf("No error.\n");
    fd = open(PRUSS_UIO_DRV_PRUSS_BASE, O_RDONLY);
    if (fd >= 0) {
        read(fd, hexstring, 20);
        pruss_phys_base = strtoul(hexstring, NULL, 16);
//		printf("Base Address: %x\n", &pruss_phys_base);
//		printf("Base Value: %x\n", *(unsigned int*) pruss_phys_base);
        close(fd);
    } else
        return -1;
 //printf("No error.\n");
    fd = open(PRUSS_UIO_DRV_PRUSS_SIZE, O_RDONLY);
    if (fd >= 0) {
        read(fd, hexstring, 20);
        pruss_map_size = strtoul(hexstring, NULL, 16);
//		printf("Size Addr: %x\n", &pruss_phys_size);
//		printf("Size Value: %x\n", *(unsigned int*) pruss_map_size);
        close(fd);
    } else
        return -1;

//printf("No error.\n");
    prussdrv.pru0dataram_base =
        mmap(0, pruss_map_size, PROT_READ | PROT_WRITE, MAP_SHARED,
             prussdrv.mmap_fd, pruss_phys_base);

    prussdrv.pru1dataram_base =
        prussdrv.pru0dataram_base + (DATARAM1_PHYS_BASE -
                                     DATARAM0_PHYS_BASE);
    prussdrv.intc_base =
        prussdrv.pru0dataram_base + (INTC_PHYS_BASE - DATARAM0_PHYS_BASE);
    prussdrv.pru0control_base =
        prussdrv.pru0dataram_base + (PRU0CONTROL_PHYS_BASE -
                                     DATARAM0_PHYS_BASE);
    prussdrv.pru0debug_base =
        prussdrv.pru0dataram_base + (PRU0DEBUG_PHYS_BASE -
                                     DATARAM0_PHYS_BASE);
    prussdrv.pru1control_base =
        prussdrv.pru0dataram_base + (PRU1CONTROL_PHYS_BASE -
                                     DATARAM0_PHYS_BASE);
    prussdrv.pru1debug_base =
        prussdrv.pru0dataram_base + (PRU1DEBUG_PHYS_BASE -
                                     DATARAM0_PHYS_BASE);
    prussdrv.pru0iram_base =
        prussdrv.pru0dataram_base + (PRU0IRAM_PHYS_BASE -
                                     DATARAM0_PHYS_BASE);
    prussdrv.pru1iram_base =
        prussdrv.pru0dataram_base + (PRU1IRAM_PHYS_BASE -
                                     DATARAM0_PHYS_BASE);

    fd = open(PRUSS_UIO_DRV_L3RAM_BASE, O_RDONLY);
    if (fd >= 0) {
        read(fd, hexstring, 20);
        l3ram_phys_base = strtoul(hexstring, NULL, 16);
        close(fd);
    } else
        return -1;


    fd = open(PRUSS_UIO_DRV_L3RAM_SIZE, O_RDONLY);
    if (fd >= 0) {
        read(fd, hexstring, 20);
        l3ram_map_size = strtoul(hexstring, NULL, 16);
        close(fd);
    } else
        return -1;

    prussdrv.l3ram_base =
        mmap(0, l3ram_map_size, PROT_READ | PROT_WRITE, MAP_SHARED,
             prussdrv.mmap_fd, l3ram_phys_base);

    fd = open(PRUSS_UIO_DRV_EXTRAM_BASE, O_RDONLY);
    if (fd >= 0) {
        read(fd, hexstring, 20);
        extram_phys_base = strtoul(hexstring, NULL, 16);
        close(fd);
    } else
        return -1;


    fd = open(PRUSS_UIO_DRV_EXTRAM_SIZE, O_RDONLY);
    if (fd >= 0) {
        read(fd, hexstring, 20);
        extram_map_size = strtoul(hexstring, NULL, 16);
        close(fd);
    } else
        return -1;


    prussdrv.extram_base =
        mmap(0, extram_map_size, PROT_READ | PROT_WRITE, MAP_SHARED,
             prussdrv.mmap_fd, extram_phys_base);

    return 0;

}

int prussdrv_init(void)
{
    memset(&prussdrv, 0, sizeof(prussdrv));
    return 0;

}

int prussdrv_open(unsigned int pru_evtout_num)
{
    char name[128];
    sprintf(name, "/dev/uio%d", pru_evtout_num);
    prussdrv.fd[pru_evtout_num] = open(name, O_RDWR | O_SYNC);
    return __prussdrv_memmap_init();

}




int prussdrv_pru_reset(unsigned int prunum)
{
    unsigned int *prucontrolregs;
    if (prunum == 0)
        prucontrolregs = (unsigned int *) prussdrv.pru0control_base;
    else if (prunum == 1)
        prucontrolregs = (unsigned int *) prussdrv.pru1control_base;
    else
        return -1;
    *prucontrolregs = 0;
    return 0;
}

int prussdrv_pru_enable(unsigned int prunum)
{
    unsigned int *prucontrolregs;
    if (prunum == 0)
        prucontrolregs = (unsigned int *) prussdrv.pru0control_base;
    else if (prunum == 1)
        prucontrolregs = (unsigned int *) prussdrv.pru1control_base;
    else
        return -1;

    *prucontrolregs = 2;
    return 0;

}

int prussdrv_pru_disable(unsigned int prunum)
{
    unsigned int *prucontrolregs;
    if (prunum == 0)
        prucontrolregs = (unsigned int *) prussdrv.pru0control_base;
    else if (prunum == 1)
        prucontrolregs = (unsigned int *) prussdrv.pru1control_base;
    else
        return -1;
//	printf("regs = %x\n", *(unsigned int*) prucontrolregs);
    *prucontrolregs = 1;
    return 0;

}

int prussdrv_pru_write_memory(unsigned int pru_ram_id,
                              unsigned int wordoffset,
                              unsigned int *memarea,
                              unsigned int bytelength)
{
    unsigned int *pruramarea, i, wordlength;
//	printf("No error.\n");
    switch (pru_ram_id) {
//	printf("No error.\n");
    case PRUSS0_PRU0_IRAM:
        pruramarea = (unsigned int *) prussdrv.pru0iram_base;
//		printf("No error.\n");
        break;
    case PRUSS0_PRU1_IRAM:
        pruramarea = (unsigned int *) prussdrv.pru1iram_base;
//		printf("No error.\n");
        break;
    case PRUSS0_PRU0_DATARAM:
        pruramarea = (unsigned int *) prussdrv.pru0dataram_base;
//		printf("No error.\n");
        break;
    case PRUSS0_PRU1_DATARAM:
        pruramarea = (unsigned int *) prussdrv.pru1dataram_base;
//		printf("No error.\n");
        break;
    default:
        return -1;
    }


    wordlength = (bytelength + 3) >> 2; //Adjust length as multiple of 4 bytes
//	printf("No error.\n");
    for (i = 0; i < wordlength; i++) {
        *(pruramarea + i + wordoffset) = *(memarea + i);
//		printf("No error.\n");
    }
    return wordlength;

}


int prussdrv_pruintc_init(tpruss_intc_initdata * prussintc_init_data)
{
    unsigned int *pruintc_io = (unsigned int *) prussdrv.intc_base;
    unsigned int i, mask1, mask2;

    pruintc_io[PRU_INTC_SIPR1_REG >> 2] = 0xFFFFFFFF;
    pruintc_io[PRU_INTC_SIPR2_REG >> 2] = 0xFFFFFFFF;

    for (i = 0; i < (NUM_PRU_SYS_EVTS + 3) >> 2; i++)
        pruintc_io[(PRU_INTC_CMR1_REG >> 2) + i] = 0;
    for (i = 0;
         ((prussintc_init_data->sysevt_to_channel_map[i].sysevt != -1)
          && (prussintc_init_data->sysevt_to_channel_map[i].channel !=
              -1)); i++) {
        __prussintc_set_cmr(pruintc_io,
                            prussintc_init_data->
                            sysevt_to_channel_map[i].sysevt,
                            prussintc_init_data->
                            sysevt_to_channel_map[i].channel);
    }
    for (i = 0; i < (NUM_PRU_HOSTS + 3) >> 2; i++)
        pruintc_io[(PRU_INTC_HMR1_REG >> 2) + i] = 0;
    for (i = 0;
         ((prussintc_init_data->channel_to_host_map[i].channel != -1)
          && (prussintc_init_data->channel_to_host_map[i].host != -1));
         i++) {

        __prussintc_set_hmr(pruintc_io,
                            prussintc_init_data->
                            channel_to_host_map[i].channel,
                            prussintc_init_data->
                            channel_to_host_map[i].host);
    }

    pruintc_io[PRU_INTC_SITR1_REG >> 2] = 0x0;
    pruintc_io[PRU_INTC_SITR2_REG >> 2] = 0x0;


    mask1 = mask2 = 0;
    for (i = 0; prussintc_init_data->sysevts_enabled[i] != 255; i++) {
        if (prussintc_init_data->sysevts_enabled[i] < 32) {
            mask1 =
                mask1 + (1 << (prussintc_init_data->sysevts_enabled[i]));
        } else if (prussintc_init_data->sysevts_enabled[i] < 64) {
            mask2 =
                mask2 +
                (1 << (prussintc_init_data->sysevts_enabled[i] - 32));
        } else {
            printf("Error: SYS_EVT%d out of range\n",
                   prussintc_init_data->sysevts_enabled[i]);
            return -1;
        }
    }
    pruintc_io[PRU_INTC_ESR1_REG >> 2] = mask1;
    pruintc_io[PRU_INTC_SECR1_REG >> 2] = mask1;
    pruintc_io[PRU_INTC_ESR2_REG >> 2] = mask2;
    pruintc_io[PRU_INTC_SECR2_REG >> 2] = mask2;

    pruintc_io[PRU_INTC_HIER_REG >> 2] =
        (prussintc_init_data->host_enable_bitmask) & 0x3FF;

    pruintc_io[PRU_INTC_GER_REG >> 2] = 0x1;

    return 0;
}


int prussdrv_pru_send_event(unsigned int eventnum)
{
    unsigned int *pruintc_io = (unsigned int *) prussdrv.intc_base;
    // pthread_mutex_lock (&prussdrv_send_event_mutex);
    pruintc_io[PRU_INTC_SRSR2_REG >> 2] = 1 << (eventnum - 32);
    //pthread_mutex_unlock (&prussdrv_send_event_mutex);
    return 0;
}

int prussdrv_pru_wait_event(unsigned int pru_evtout_num)
{
    int event_count;
    return (read(prussdrv.fd[pru_evtout_num], &event_count, sizeof(int)));

}

int prussdrv_pru_clear_event(unsigned int eventnum)
{
    unsigned int *pruintc_io = (unsigned int *) prussdrv.intc_base;
    //pthread_mutex_lock (&prussdrv_clear_event_mutex);
    pruintc_io[PRU_INTC_SECR2_REG >> 2] = 1 << (eventnum - 32);
    //pthread_mutex_unlock (&prussdrv_clear_event_mutex);
    return 0;
}

int prussdrv_pru_send_wait_clear_event(unsigned int send_eventnum,
                                       unsigned int pru_evtout_num,
                                       unsigned int ack_eventnum)
{
    pthread_mutex_lock(&prussdrv_send_event_mutex);
    prussdrv_pru_send_event(send_eventnum);
    prussdrv_pru_wait_event(pru_evtout_num);
    prussdrv_pru_clear_event(ack_eventnum);
    pthread_mutex_unlock(&prussdrv_send_event_mutex);
    return 0;

}


int prussdrv_map_l3mem(void **address)
{
    *address = prussdrv.l3ram_base;
    return 0;
}



int prussdrv_map_extmem(void **address)
{

    *address = prussdrv.extram_base;
    return 0;

}


int prussdrv_map_prumem(unsigned int pru_ram_id, void **address)
{
    switch (pru_ram_id) {
    case PRUSS0_PRU0_DATARAM:
        *address = prussdrv.pru0dataram_base;
        break;
    case PRUSS0_PRU1_DATARAM:
        *address = prussdrv.pru1dataram_base;
    default:
        return -1;
    }
    return 0;
}


unsigned int prussdrv_get_phys_addr(void *address)
{
    unsigned int retaddr = 0;
    if ((address >= prussdrv.pru0dataram_base)
        && (address < prussdrv.pru0dataram_base + PRUSS_DATARAM_SIZE)) {
        retaddr =
            ((unsigned int) (address - prussdrv.pru0dataram_base) +
             DATARAM0_PHYS_BASE);
    } else if ((address >= prussdrv.pru1dataram_base)
               && (address <
                   prussdrv.pru1dataram_base + PRUSS_DATARAM_SIZE)) {
        retaddr =
            ((unsigned int) (address - prussdrv.pru1dataram_base) +
             DATARAM1_PHYS_BASE);
    } else if ((address >= prussdrv.l3ram_base)
               && (address < prussdrv.l3ram_base + l3ram_map_size)) {
        retaddr =
            ((unsigned int) (address - prussdrv.l3ram_base) +
             l3ram_phys_base);
    } else if ((address >= prussdrv.extram_base)
               && (address < prussdrv.extram_base + extram_map_size)) {
        retaddr =
            ((unsigned int) (address - prussdrv.extram_base) +
             extram_phys_base);
    }
    //printf ("virt_addr: %X phys_addr: %X\n",address,  retaddr);       
    return retaddr;

}

void *prussdrv_get_virt_addr(unsigned int phyaddr)
{
    void *address = 0;
    if ((phyaddr >= DATARAM0_PHYS_BASE)
        && (phyaddr < DATARAM0_PHYS_BASE + PRUSS_DATARAM_SIZE)) {
        address =
            (void *) ((unsigned int) prussdrv.pru0dataram_base +
                      (phyaddr - DATARAM0_PHYS_BASE));
    } else if ((phyaddr >= DATARAM1_PHYS_BASE)
               && (phyaddr < DATARAM1_PHYS_BASE + PRUSS_DATARAM_SIZE)) {
        address =
            (void *) ((unsigned int) prussdrv.pru1dataram_base +
                      (phyaddr - DATARAM1_PHYS_BASE));
    } else if ((phyaddr >= l3ram_phys_base)
               && (phyaddr < l3ram_phys_base + l3ram_map_size)) {
        address =
            (void *) ((unsigned int) prussdrv.l3ram_base +
                      (phyaddr - l3ram_phys_base));
    } else if ((phyaddr >= extram_phys_base)
               && (phyaddr < extram_phys_base + extram_map_size)) {
        address =
            (void *) ((unsigned int) prussdrv.extram_base +
                      (phyaddr - extram_phys_base));
    }
    //printf ("phys_addr: %X virt_addr: %X\n",  phyaddr, address);      
    return address;

}


int prussdrv_exit()
{
    int i;
    munmap(prussdrv.pru0dataram_base, pruss_map_size);
    munmap(prussdrv.l3ram_base, l3ram_map_size);
    munmap(prussdrv.extram_base, extram_map_size);
    for (i = 0; i < NUM_PRU_HOSTIRQS; i++) {
        if (prussdrv.fd[i])
            close(prussdrv.fd[i]);
        if (prussdrv.irq_thread[i])
            pthread_join(prussdrv.irq_thread[i], NULL);
    }
    return 0;
}

int prussdrv_exec_program(int prunum, char *filename)
{
    FILE *fPtr;
    unsigned char fileDataArray[PRUSS_IRAM_SIZE];
    int fileSize = 0;
    unsigned int pru_ram_id;

    if (prunum == 0)
        pru_ram_id = PRUSS0_PRU0_IRAM;
    else if (prunum == 1)
        pru_ram_id = PRUSS0_PRU1_IRAM;
    else
        return -1;

    // Open an File from the hard drive
    fPtr = fopen(filename, "rb");
    if (fPtr == NULL) {
        printf("File %s open failed\n", filename);
    } else {
        printf("File %s open passed\n", filename);
    }
    // Read file size
    fseek(fPtr, 0, SEEK_END);
    fileSize = ftell(fPtr);

    if (fileSize == 0) {
        printf("File read failed.. Closing program\n");
        fclose(fPtr);
        return -1;
    }

    fseek(fPtr, 0, SEEK_SET);

    if (fileSize !=
        fread((unsigned char *) fileDataArray, 1, fileSize, fPtr)) {
        printf("WARNING: File Size mismatch\n");
    }

    fclose(fPtr);

    // Make sure PRU sub system is first disabled/reset  
//	printf("Disable\n");
    prussdrv_pru_disable(prunum);
//	printf("Write\n");
    prussdrv_pru_write_memory(pru_ram_id, 0,
                              (unsigned int *) fileDataArray, fileSize);
//	printf("Enable\n");
    prussdrv_pru_enable(prunum);

    return 0;
}

int prussdrv_start_irqthread(unsigned int pru_evtout_num, int priority,
                             prussdrv_function_handler irqhandler)
{
    pthread_attr_t pthread_attr;
    struct sched_param sched_param;
    pthread_attr_init(&pthread_attr);
    if (priority != 0) {
        pthread_attr_setinheritsched(&pthread_attr,
                                     PTHREAD_EXPLICIT_SCHED);
        pthread_attr_setschedpolicy(&pthread_attr, SCHED_FIFO);
        sched_param.sched_priority = priority;
        pthread_attr_setschedparam(&pthread_attr, &sched_param);
    }

    pthread_create(&prussdrv.irq_thread[pru_evtout_num], &pthread_attr,
                   irqhandler, NULL);

    pthread_attr_destroy(&pthread_attr);

    return 0;

}
