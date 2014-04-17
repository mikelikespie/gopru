package pruss

const (
	PRU0_PRU1_INTERRUPT EventNum = 17
	PRU1_PRU0_INTERRUPT EventNum = 18
	PRU0_ARM_INTERRUPT  EventNum = 19
	PRU1_ARM_INTERRUPT  EventNum = 20
	ARM_PRU0_INTERRUPT  EventNum = 21
	ARM_PRU1_INTERRUPT  EventNum = 22

	CHANNEL0 Channel = 0
	CHANNEL1 Channel = 1
	CHANNEL2 Channel = 2
	CHANNEL3 Channel = 3
	CHANNEL4 Channel = 4
	CHANNEL5 Channel = 5
	CHANNEL6 Channel = 6
	CHANNEL7 Channel = 7
	CHANNEL8 Channel = 8
	CHANNEL9 Channel = 9

	PRU0        Host = 0
	PRU1        Host = 1
	PRU_EVTOUT0 Host = 2
	PRU_EVTOUT1 Host = 3
	PRU_EVTOUT2 Host = 4
	PRU_EVTOUT3 Host = 5
	PRU_EVTOUT4 Host = 6
	PRU_EVTOUT5 Host = 7
	PRU_EVTOUT6 Host = 8
	PRU_EVTOUT7 Host = 9
)

type PruIntcInitData struct {
	SysevtsEnabled     []EventNum
	SysevtToChannelMap SysevtToChannelMap
	ChannelToHostMap   ChannelToHostMap
	HostEnabled        []Host
}

var DefaultInitData = PruIntcInitData{
	SysevtsEnabled: []EventNum{
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
	HostEnabled: []Host{
		PRU0,
		PRU1,
		PRU_EVTOUT0,
		PRU_EVTOUT1,
	},
}
