package pruss

import "C"
import "os"
import "syscall"

type PruRamId int
type EvtOut int

const (
	NUM_PRU_HOSTIRQS = 8
	NUM_PRU_HOSTS    = 10
	NUM_PRU_CHANNELS = 10
	NUM_PRU_SYS_EVTS = 64

	PRUSS0_PRU0_DATARAM PruRamId = 0
	PRUSS0_PRU1_DATARAM PruRamId = 1
	PRUSS0_PRU0_IRAM    PruRamId = 2
	PRUSS0_PRU1_IRAM    PruRamId = 3

	PRU_EVTOUT_0 EvtOut = 0
	PRU_EVTOUT_1 EvtOut = 1
	PRU_EVTOUT_2 EvtOut = 2
	PRU_EVTOUT_3 EvtOut = 3
	PRU_EVTOUT_4 EvtOut = 4
	PRU_EVTOUT_5 EvtOut = 5
	PRU_EVTOUT_6 EvtOut = 6
	PRU_EVTOUT_7 EvtOut = 7
)

type PruRamType int

const (
	DATARAM PruRamType = 0
	IRAM    PruRamType = iota
)

type SysevtToChannelMap map[int16]int16
type ChannelToHostMap map[int16]int16

type Pru interface {
	Reset() (err error)
	Enable() (err error)
	Disable() (err error)
	WriteMemory(ramType PruRamType, writeOffset uint, sourceData []byte) (err error)
	ExecProgram(filename string) (err error)

	SendEvent(evtOutNum EvtOut)
	WaitEvent(uint eventNum)
	ClearEvent(uint eventNum)
	SendWaitClearEvent(sendEventNum uint, evtOutNum EvtOut, ackEventNum uint)
}

type PrussDrv interface {
	Open(pruEvtoutNum int) (err error)
	GetPru(pruNum int) (pru Pru)
	InitInterrupts() (err error)
	Close() (err error)
}

func InitDrv() (drv PrussDrv, err error) {

}

type prussPru struct {
	drv *prussDrv

	dataramBase []byte
	controlBase []byte
	debugBase   []byte
	iramBase    []byte

}

type prussDrv struct {
	prus []prussPru

	// Files for interrups
	evtFiles map[EvtOut]*os.File

	mmapFdFile *os.File

	// mmapped files
	intcBase   []byte
	l3RamBase  []byte
	extRamBase []byte
}

func (d *prussDrv) memmapInit() (err error) {
}

func (d *prussDrv) Open(pruEvtoutNum int) (err error) {
	return
}

func (d *prussDrv) GetPru(pruNum int) (pru Pru) {
	return d.prus[0]
}

func (d *prussDrv) InitInterrupts() (err error) {
	return
}

func clearAndMunmap(v *[]byte) {
	if *v != nil {
		syscall.Munmap(*v)
		*v = nil
	}
}

func (d *prussDrv) Close() (err error) {
	clearAndMunmap(&d.intcBase)
	clearAndMunmap(&d.l3RamBase)
	clearAndMunmap(&d.extRamBase)

	for _, pru := range d.prus {
		clearAndMunmap(&pru.controlBase)
		clearAndMunmap(&pru.dataramBase)
		clearAndMunmap(&pru.debugBase)
		clearAndMunmap(&pru.iramBase)
	}

	for i, f := range d.evtFiles {
		f.Close()
		delete(d.evtFiles, i)
	}

	if (d.mmapFdFile != nil) {
		d.mmapFdFile.Close()
		d.mmapFdFile = nil
	}

	return
}

func (p *prussPru) Reset() (err error)   { return }
func (p *prussPru) Enable() (err error)  { return }
func (p *prussPru) Disable() (err error) { return }
func (p *prussPru) WriteMemory(ramType PruRamType, writeOffset uint, sourceData []byte) (err error) {
	return
}
func (p *prussPru) ExecProgram(filename string) (err error) { return }

func (p *prussPru) SendEvent(evtOutNum EvtOut)                                               { return }
func (p *prussPru) WaitEvent(uint eventNum)                                                  { return }
func (p *prussPru) ClearEvent(uint eventNum)                                                 { return }
func (p *prussPru) SendWaitClearEvent(sendEventNum uint, evtOutNum EvtOut, ackEventNum uint) { return }

/*

   int prussdrv_open(unsigned int pru_evtout_num);

   int prussdrv_pru_reset(unsigned int prunum);

   int prussdrv_pru_disable(unsigned int prunum);

   int prussdrv_pru_enable(unsigned int prunum);

   int prussdrv_pru_write_memory(unsigned int pru_ram_id,
                                 unsigned int wordoffset,
                                 unsigned int *memarea,
                                 unsigned int bytelength);

   int prussdrv_pruintc_init(tpruss_intc_initdata * prussintc_init_data);

   int prussdrv_map_l3mem(void **address);

   int prussdrv_map_extmem(void **address);

   int prussdrv_map_prumem(unsigned int pru_ram_id, void **address);

   unsigned int prussdrv_get_phys_addr(void *address);

   void *prussdrv_get_virt_addr(unsigned int phyaddr);

   int prussdrv_pru_wait_event(unsigned int pru_evtout_num);

   int prussdrv_pru_send_event(unsigned int eventnum);

   int prussdrv_pru_clear_event(unsigned int eventnum);

   int prussdrv_pru_send_wait_clear_event(unsigned int send_eventnum,
                                          unsigned int pru_evtout_num,
                                          unsigned int ack_eventnum);

   int prussdrv_exit(void);

   int prussdrv_exec_program(int prunum, char *filename);
*/
