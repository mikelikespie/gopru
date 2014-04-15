package pruss

import (
	"fmt"
	"io"
	"io/ioutil"
	"os"
	"strconv"
	"strings"
	"syscall"
	"unsafe"
)

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
	// This is shared between both PRUs
	SHARED PruRamType = iota
)

type SysevtToChannelMap map[int16]int16
type ChannelToHostMap map[int16]int16

type Pru interface {
	Reset() error
	Enable() error
	Disable() error
	WriteMemory(ramType PruRamType, writeOffset uint, sourceData []byte) error
	ExecProgram(filename string) error
}

type PrussDrv interface {
	OpenInterrupt(evtOut EvtOut) (intcChan chan int, err error)

	Pru(pruNum int) (pru Pru)
	InitInterrupts() (err error)

	Close() (err error)

	SendEvent(eventNum uint) error
	ClearEvent(eventNum uint) error
}

func InitDrv() (drv PrussDrv, err error) {
	drv = &prussDrv{
		initData: DefaultInitData,
		prus: []prussPru{
			prussPru{pruNum: 0},
			prussPru{pruNum: 1},
		},
		evtFiles: make(map[EvtOut]*os.File),
		evtChans: make(map[EvtOut] chan int),
	}

	return
}

type prussPru struct {
	drv           *prussDrv
	pruNum        int
	dataramBase   []byte
	controlBase   []byte
	debugBase     []byte
	iramBase      []byte
	sharedramBase []byte
}

type prussDrv struct {
	initData PruIntcInitData
	prus     []prussPru

	// Files for interrups
	evtFiles map[EvtOut]*os.File
	evtChans map[EvtOut]chan int

	mmapFdFile *os.File

	physBase       uint64
	l3RamPhysBase  uint64
	extRamPhysBase uint64

	// mmapped files
	intcBase   []byte
	l3RamBase  []byte
	extRamBase []byte
}

func readIntFromFile(fileName string) (num uint64, err error) {
	c, err := ioutil.ReadFile(fileName)
	if err != nil {
		return 0, err
	}

	return strconv.ParseUint(strings.TrimSpace(string(c[2:])), 16, 32)
}

func (d *prussDrv) rambaseOffset(physBase int) []byte {
	offset := physBase - DATARAM0_PHYS_BASE
	ramBase := d.prus[0].dataramBase
	return ramBase[offset : len(ramBase)-offset]
}

func (d *prussDrv) memmapInit() (err error) {

	// Get the first one
	for _, d.mmapFdFile = range d.evtFiles {
		break
	}

	if d.mmapFdFile == nil {
		panic("We should have an fd by now")
	}

	d.physBase, err = readIntFromFile(PRUSS_UIO_DRV_PRUSS_BASE)
	if err != nil {
		return err
	}

	prussMapSize, err := readIntFromFile(PRUSS_UIO_DRV_PRUSS_SIZE)
	if err != nil {
		return err
	}
	return

	d.prus[0].dataramBase, err = syscall.Mmap(
		int(d.mmapFdFile.Fd()),
		int64(PRUSS_UIO_MAP_OFFSET_PRUSS),
		int(prussMapSize),
		syscall.PROT_READ|syscall.PROT_WRITE,
		syscall.MAP_SHARED,
	)
	if err != nil {
		return err
	}

	d.prus[1].dataramBase = d.rambaseOffset(DATARAM1_PHYS_BASE)

	d.intcBase = d.rambaseOffset(INTC_PHYS_BASE)

	sharedBase := d.rambaseOffset(PRUSS_SHAREDRAM_BASE)
	d.prus[0].sharedramBase = sharedBase
	d.prus[1].sharedramBase = sharedBase

	d.prus[0].controlBase = d.rambaseOffset(PRU0CONTROL_PHYS_BASE)
	d.prus[1].controlBase = d.rambaseOffset(PRU1CONTROL_PHYS_BASE)

	d.prus[0].debugBase = d.rambaseOffset(PRU0DEBUG_PHYS_BASE)
	d.prus[1].debugBase = d.rambaseOffset(PRU1DEBUG_PHYS_BASE)

	d.prus[0].iramBase = d.rambaseOffset(PRU0IRAM_PHYS_BASE)
	d.prus[1].iramBase = d.rambaseOffset(PRU1IRAM_PHYS_BASE)

	d.l3RamPhysBase, err = readIntFromFile(PRUSS_UIO_DRV_L3RAM_BASE)
	if err != nil {
		return err
	}
	l3RamMapSize, err := readIntFromFile(PRUSS_UIO_DRV_L3RAM_SIZE)
	if err != nil {
		return err
	}
	d.l3RamBase, err = syscall.Mmap(
		int(d.mmapFdFile.Fd()),
		int64(PRUSS_UIO_MAP_OFFSET_L3RAM),
		int(l3RamMapSize),
		syscall.PROT_READ|syscall.PROT_WRITE,
		syscall.MAP_SHARED,
	)

	d.extRamPhysBase, err = readIntFromFile(PRUSS_UIO_DRV_EXTRAM_BASE)
	if err != nil {
		return err
	}
	extRamMapSize, err := readIntFromFile(PRUSS_UIO_DRV_EXTRAM_SIZE)
	if err != nil {
		return err
	}
	d.extRamBase, err = syscall.Mmap(
		int(d.mmapFdFile.Fd()),
		int64(PRUSS_UIO_MAP_OFFSET_EXTRAM),
		int(extRamMapSize),
		syscall.PROT_READ|syscall.PROT_WRITE,
		syscall.MAP_SHARED,
	)

	return
}

func (d *prussDrv) consumeEvents(evtOut EvtOut, c chan int, f *os.File) {
	buf := make([]byte, 4)

	numEventsBuff := (*uint32)(unsafe.Pointer(&buf[0]))

	for {
		n, err := f.Read(buf)
		switch {
		case err == nil && n == len(buf):
			c <- int(*numEventsBuff)
			*(*uint32)(unsafe.Pointer(&d.intcBase[PRU_INTC_HIEISR_REG])) = uint32(evtOut) + 2
		case err == io.EOF:
			break
		case n != len(buf):
			panic("read bytes not as long as buff.  don't know what to do :(")

		default:
			panic("error closing stream")
		}
	}

	close(c)
}

func (d *prussDrv) OpenInterrupt(evtOut EvtOut) (intcChan chan int, err error) {
	if chn, found := d.evtChans[evtOut]; found {
		return chn, nil
	}

	d.evtFiles[evtOut], err = os.OpenFile(
		"/dev/uio"+strconv.Itoa(int(evtOut)),
		os.O_SYNC|os.O_RDWR,
		0600,
	)


	if err != nil {
		return
	}

	d.evtChans[evtOut] = make(chan int)

	go d.consumeEvents(evtOut, d.evtChans[evtOut], d.evtFiles[evtOut])

	// We initialize memmap stuff here since it uses it
	if d.mmapFdFile == nil {
		if err = d.memmapInit(); err != nil {
			return nil, err
		}
	}
	return
}

func (d *prussDrv) Pru(pruNum int) (pru Pru) {
	return &d.prus[0]
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
	clearAndMunmap(&d.prus[0].dataramBase)
	d.intcBase = nil
	clearAndMunmap(&d.l3RamBase)
	clearAndMunmap(&d.extRamBase)

	for _, pru := range d.prus {
		pru.controlBase = nil
		pru.dataramBase = nil
		pru.debugBase = nil
		pru.iramBase = nil
	}

	for k, f := range d.evtFiles {
		f.Close()
		for _ = range d.evtChans[k] {
		}
		delete(d.evtFiles, k)
		delete(d.evtChans, k)
	}

	d.mmapFdFile = nil

	return
}

func (p *prussPru) Enable() (err error) {
	*(*uint32)(unsafe.Pointer(&p.controlBase[0])) = 2
	return
}

func (p *prussPru) Reset() (err error) {
	*(*uint32)(unsafe.Pointer(&p.controlBase[0])) = 0
	return
}
func (p *prussPru) Disable() (err error) {
	*(*uint32)(unsafe.Pointer(&p.controlBase[0])) = 1
	return
}

func (p *prussPru) WriteMemory(ramType PruRamType, writeOffset uint, sourceData []byte) (err error) {
	var memory []byte
	switch ramType {
	case IRAM:
		memory = p.iramBase
	case DATARAM:
		memory = p.dataramBase
	case SHARED:
		memory = p.sharedramBase
	}

	copy(memory[writeOffset:], sourceData)

	return
}

func (p *prussPru) ExecProgram(filename string) (err error) {
	b, err := ioutil.ReadFile(filename)
	if err != nil {
		return
	}

	if len(b) >= PRUSS_IRAM_SIZE {
		err = fmt.Errorf("Image size (%d) from (%s) exceeds max iram size (%d)", len(b), filename, PRUSS_IRAM_SIZE)
		return
	}

	if err = p.Disable(); err != nil {
		return
	}

	if err = p.WriteMemory(IRAM, 0, b); err != nil {
		return
	}

	return
}

func (d *prussDrv) SendEvent(eventNum uint) (err error) {

	var reg int
	switch {
	case eventNum < 32:
		reg = PRU_INTC_SRSR1_REG
	case eventNum < 64:
		reg = PRU_INTC_SRSR2_REG
		eventNum -= 32
	default:
		return fmt.Errorf("Invalid event num: %d", eventNum)
	}

	*(*uint32)(unsafe.Pointer(&d.intcBase[reg])) = uint32(eventNum)

	return
}

func (d *prussDrv) ClearEvent(eventNum uint) (err error) {
	var reg int
	switch {
	case eventNum < 32:
		reg = PRU_INTC_SECR1_REG
	case eventNum < 64:
		reg = PRU_INTC_SECR2_REG
		eventNum -= 32
	default:
		return fmt.Errorf("Invalid event num: %d", eventNum)
	}

	*(*uint32)(unsafe.Pointer(&d.intcBase[reg])) = uint32(eventNum)

	return
}
