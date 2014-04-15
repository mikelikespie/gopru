package pruss

const (
	PRU0_PRU1_INTERRUPT = 32
	PRU1_PRU0_INTERRUPT = 33
	PRU0_ARM_INTERRUPT  = 34
	PRU1_ARM_INTERRUPT  = 35
	ARM_PRU0_INTERRUPT  = 36
	ARM_PRU1_INTERRUPT  = 37

	CHANNEL0 = 0
	CHANNEL1 = 1
	CHANNEL2 = 2
	CHANNEL3 = 3
	CHANNEL4 = 4
	CHANNEL5 = 5
	CHANNEL6 = 6
	CHANNEL7 = 7
	CHANNEL8 = 8
	CHANNEL9 = 9

	PRU0        = 0
	PRU1        = 1
	PRU_EVTOUT0 = 2
	PRU_EVTOUT1 = 3
	PRU_EVTOUT2 = 4
	PRU_EVTOUT3 = 5
	PRU_EVTOUT4 = 6
	PRU_EVTOUT5 = 7
	PRU_EVTOUT6 = 8
	PRU_EVTOUT7 = 9

	PRU0_HOSTEN_MASK        = 0x0001
	PRU1_HOSTEN_MASK        = 0x0002
	PRU_EVTOUT0_HOSTEN_MASK = 0x0004
	PRU_EVTOUT1_HOSTEN_MASK = 0x0008
	PRU_EVTOUT2_HOSTEN_MASK = 0x0010
	PRU_EVTOUT3_HOSTEN_MASK = 0x0020
	PRU_EVTOUT4_HOSTEN_MASK = 0x0040
	PRU_EVTOUT5_HOSTEN_MASK = 0x0080
	PRU_EVTOUT6_HOSTEN_MASK = 0x0100
	PRU_EVTOUT7_HOSTEN_MASK = 0x0200
)

type PruIntcInitData struct {
	SysevtsEnabled     []uint8
	SysevtToChannelMap SysevtToChannelMap
	ChannelToHostMap   ChannelToHostMap
	HostEnabledBitmask uint
}

var DefaultInitData = PruIntcInitData{
	SysevtsEnabled: []uint8{
		PRU0_PRU1_INTERRUPT,
		PRU1_PRU0_INTERRUPT,
		PRU0_ARM_INTERRUPT,
		PRU1_ARM_INTERRUPT,
		ARM_PRU0_INTERRUPT,
		ARM_PRU1_INTERRUPT,
	},
	SysevtToChannelMap: SysevtToChannelMap{
		PRU0_PRU1_INTERRUPT: CHANNEL1,
		PRU1_PRU0_INTERRUPT: CHANNEL0,
		PRU0_ARM_INTERRUPT:  CHANNEL2,
		PRU1_ARM_INTERRUPT:  CHANNEL3,
		ARM_PRU0_INTERRUPT:  CHANNEL0,
		ARM_PRU1_INTERRUPT:  CHANNEL1,
	},
	ChannelToHostMap: ChannelToHostMap{
		CHANNEL0: PRU0,
		CHANNEL1: PRU1,
		CHANNEL2: PRU_EVTOUT0,
		CHANNEL3: PRU_EVTOUT1,
	},

	HostEnabledBitmask: (PRU0_HOSTEN_MASK | PRU1_HOSTEN_MASK | PRU_EVTOUT0_HOSTEN_MASK | PRU_EVTOUT1_HOSTEN_MASK),
}