package pruss

const (
	PAGE_SIZE           = 4096
	PRUSS_MAX_IRAM_SIZE = 8192
	PRUSS_IRAM_SIZE     = 8192
	PRUSS_DATARAM_SIZE  = 512

	PRUSS_MMAP_SIZE = 0x40000

	DATARAM0_PHYS_BASE    = 0x4a300000
	DATARAM1_PHYS_BASE    = 0x4a302000
	INTC_PHYS_BASE        = 0x4a320000
	PRU0CONTROL_PHYS_BASE = 0x4a322000
	PRU0DEBUG_PHYS_BASE   = 0x4a322400
	PRU1CONTROL_PHYS_BASE = 0x4a324000
	PRU1DEBUG_PHYS_BASE   = 0x4a324400
	PRU0IRAM_PHYS_BASE    = 0x4a334000
	PRU1IRAM_PHYS_BASE    = 0x4a338000
	PRUSS_SHAREDRAM_BASE  = 0x4a310000
	PRUSS_CFG_BASE        = 0x4a326000
	PRUSS_UART_BASE       = 0x4a328000
	PRUSS_IEP_BASE        = 0x4a32e000
	PRUSS_ECAP_BASE       = 0x4a330000
	PRUSS_MIIRT_BASE      = 0x4a332000
	PRUSS_MDIO_BASE       = 0x4a332400

	//NOTE: Above defines are SOC specific

	PRU_INTC_REVID_REG  = 0x000
	PRU_INTC_CR_REG     = 0x004
	PRU_INTC_HCR_REG    = 0x00C
	PRU_INTC_GER_REG    = 0x010
	PRU_INTC_GNLR_REG   = 0x01C
	PRU_INTC_SISR_REG   = 0x020
	PRU_INTC_SICR_REG   = 0x024
	PRU_INTC_EISR_REG   = 0x028
	PRU_INTC_EICR_REG   = 0x02C
	PRU_INTC_HIEISR_REG = 0x034
	PRU_INTC_HIDISR_REG = 0x038
	PRU_INTC_GPIR_REG   = 0x080

	PRU_INTC_SRSR1_REG = 0x200
	PRU_INTC_SRSR2_REG = 0x204

	PRU_INTC_SECR1_REG = 0x280
	PRU_INTC_SECR2_REG = 0x284

	PRU_INTC_ESR1_REG = 0x300
	PRU_INTC_ESR2_REG = 0x304

	PRU_INTC_ECR1_REG = 0x380
	PRU_INTC_ECR2_REG = 0x384

	PRU_INTC_CMR1_REG  = 0x400
	PRU_INTC_CMR2_REG  = 0x404
	PRU_INTC_CMR3_REG  = 0x408
	PRU_INTC_CMR4_REG  = 0x40C
	PRU_INTC_CMR5_REG  = 0x410
	PRU_INTC_CMR6_REG  = 0x414
	PRU_INTC_CMR7_REG  = 0x418
	PRU_INTC_CMR8_REG  = 0x41C
	PRU_INTC_CMR9_REG  = 0x420
	PRU_INTC_CMR10_REG = 0x424
	PRU_INTC_CMR11_REG = 0x428
	PRU_INTC_CMR12_REG = 0x42C
	PRU_INTC_CMR13_REG = 0x430
	PRU_INTC_CMR14_REG = 0x434
	PRU_INTC_CMR15_REG = 0x438
	PRU_INTC_CMR16_REG = 0x43C

	PRU_INTC_HMR1_REG = 0x800
	PRU_INTC_HMR2_REG = 0x804
	PRU_INTC_HMR3_REG = 0x808

	PRU_INTC_SIPR1_REG = 0xD00
	PRU_INTC_SIPR2_REG = 0xD04

	PRU_INTC_SITR1_REG = 0xD80
	PRU_INTC_SITR2_REG = 0xD84

	PRU_INTC_HIER_REG = 0x1500

	MAX_HOSTS_SUPPORTED = 10

	//UIO driver expects user space to map PRUSS_UIO_MAP_OFFSET_XXX to
	//access corresponding memory regions - region offset is N*PAGE_SIZE

	PRUSS_UIO_MAP_OFFSET_PRUSS = 0 * PAGE_SIZE
	PRUSS_UIO_DRV_PRUSS_BASE   = "/sys/class/uio/uio0/maps/map0/addr"
	PRUSS_UIO_DRV_PRUSS_SIZE   = "/sys/class/uio/uio0/maps/map0/size"

	PRUSS_UIO_MAP_OFFSET_EXTRAM = 1 * PAGE_SIZE
	PRUSS_UIO_DRV_EXTRAM_BASE   = "/sys/class/uio/uio0/maps/map1/addr"
	PRUSS_UIO_DRV_EXTRAM_SIZE   = "/sys/class/uio/uio0/maps/map1/size"

	// Unused. Also if we're ging to use this, the path for the EXTRAM needs to change
	PRUSS_UIO_MAP_OFFSET_L3RAM = 1 * PAGE_SIZE
	PRUSS_UIO_DRV_L3RAM_BASE   = "/sys/class/uio/uio0/maps/map1/addr"
	PRUSS_UIO_DRV_L3RAM_SIZE   = "/sys/class/uio/uio0/maps/map1/size"
)
