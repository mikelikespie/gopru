package pruss

import (
	"fmt"
	"io"
	"io/ioutil"
	"log"
	"os"
	"strconv"
	"strings"
	"syscall"
	"unsafe"
)

type PruRamId int
type EvtOut int
type PruNum int

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

	Pru0     PruNum = 0
	Pru1     PruNum = 1
	PruCount PruNum = 2
)

type PruRamType int

const (
	DATARAM PruRamType = 0
	IRAM    PruRamType = iota
	// This is shared between both PRUs
	SHARED PruRamType = iota
)

type Channel uint16
type Host uint16
type EventNum uint16
type SysevtToChannelMap map[EventNum]Channel
type ChannelToHostMap map[Channel]Host

const enableL3ram = false

var PruEvts = []EvtOut{
	PRU_EVTOUT_0,
	PRU_EVTOUT_1,
}

type Pru interface {
	Reset() error
	Enable() error
	Disable() error
	WriteMemory(ramType PruRamType, writeOffset uint, sourceData []byte) error
	ExecProgram(filename string) error
	ExecImage(image []byte) error

	// The default evt out to listen on
	DefaultEvtOut() EvtOut

	DataramMem() []byte
}

type PrussDrv interface {
	OpenInterrupt(evtOut EvtOut) (l EventListener, err error)

	Pru(pruNum PruNum) (pru Pru)
	InitInterrupts() (err error)

	Close() (err error)

	// Returns a reference to the DDR memory
	ExtRamMem() []byte
	ExtRamPhys() uintptr
	SharedRamMem() []byte

	SendEvent(eventNum EventNum) error

	GetPhysicalAddress(ptr unsafe.Pointer) uintptr
}

func InitDrv() (drv PrussDrv, err error) {
	drv = &prussDrv{
		initData: DefaultInitData,
		prus: []prussPru{
			prussPru{pruNum: Pru0},
			prussPru{pruNum: Pru1},
		},
		eventListeners: make(map[EvtOut]*eventListener),
	}

	return
}

type prussPru struct {
	drv        *prussDrv
	pruNum     PruNum
	dataramMem []byte
	controlMem []byte
	// Remapped offset 0 of controlMem
	controlMemPtr *uint32
	debugMem      []byte
	iramMem       []byte
}

// Outer is for requests, inner is chan for responses

type EventListener interface {
	// Returns a channel that will yield true when we receive an event
	Wait() uint32

	// Blocks on enquing a wait task, but will return a channel that will receive an element after the wait is finished
	WaitC() <-chan uint32

	ClearEvent(eventNum EventNum) error
}

type eventListener struct {
	f *os.File

	drv *prussDrv

	// Incoming wait requests
	in chan bool

	// Outgoing wait requests
	out chan uint32

	// Event out number
	evt EvtOut
}

type prussDrv struct {
	initData PruIntcInitData
	prus     []prussPru

	eventListeners map[EvtOut]*eventListener

	mmapFdFile *os.File

	physMem        uintptr
	l3RamPhysBase  uintptr
	extRamPhysBase uintptr

	// mmapped files
	intcMem      []byte
	l3RamMem     []byte
	extRamMem    []byte
	sharedramMem []byte
}

func readIntFromFile(fileName string) (num uintptr, err error) {
	c, err := ioutil.ReadFile(fileName)
	if err != nil {
		return 0, err
	}

	i, err := strconv.ParseUint(strings.TrimSpace(string(c[2:])), 16, 32)
	if err == nil {
		num = uintptr(i)

	}
	return
}

func (d *prussDrv) rambaseOffset(physMem int) []byte {
	offset := physMem - DATARAM0_PHYS_BASE
	ramMem := d.prus[0].dataramMem
	return ramMem[offset:]
}

func (d *prussDrv) memmapInit() (err error) {

	// Get the first one
	for _, l := range d.eventListeners {
		d.mmapFdFile = l.f
		break
	}

	if d.mmapFdFile == nil {
		panic("We should have an fd by now")
	}

	d.physMem, err = readIntFromFile(PRUSS_UIO_DRV_PRUSS_BASE)
	if err != nil {
		return err
	}

	prussMapSize, err := readIntFromFile(PRUSS_UIO_DRV_PRUSS_SIZE)
	if err != nil {
		return err
	}

	d.prus[0].dataramMem, err = syscall.Mmap(
		int(d.mmapFdFile.Fd()),
		int64(PRUSS_UIO_MAP_OFFSET_PRUSS),
		int(prussMapSize),
		syscall.PROT_READ|syscall.PROT_WRITE,
		syscall.MAP_SHARED,
	)
	if err != nil {
		return err
	}

	d.prus[1].dataramMem = d.rambaseOffset(DATARAM1_PHYS_BASE)

	d.intcMem = d.rambaseOffset(INTC_PHYS_BASE)

	if len(d.intcMem) < PRU_INTC_HIER_REG {
		return fmt.Errorf(
			"Expected d.intcMem to be at least %d big, but it is only of length %d",
			PRU_INTC_HIER_REG,
			len(d.intcMem),
		)
	}

	d.sharedramMem = d.rambaseOffset(PRUSS_SHAREDRAM_BASE)

	d.prus[0].controlMem = d.rambaseOffset(PRU0CONTROL_PHYS_BASE)
	d.prus[0].controlMemPtr = (*uint32)(unsafe.Pointer(&d.prus[0].controlMem[0]))

	d.prus[1].controlMem = d.rambaseOffset(PRU1CONTROL_PHYS_BASE)
	d.prus[1].controlMemPtr = (*uint32)(unsafe.Pointer(&d.prus[1].controlMem[0]))

	d.prus[0].debugMem = d.rambaseOffset(PRU0DEBUG_PHYS_BASE)
	d.prus[1].debugMem = d.rambaseOffset(PRU1DEBUG_PHYS_BASE)

	d.prus[0].iramMem = d.rambaseOffset(PRU0IRAM_PHYS_BASE)
	d.prus[1].iramMem = d.rambaseOffset(PRU1IRAM_PHYS_BASE)

	if enableL3ram {
		d.l3RamPhysBase, err = readIntFromFile(PRUSS_UIO_DRV_L3RAM_BASE)
		if err != nil {
			return err
		}
		l3RamMapSize, err := readIntFromFile(PRUSS_UIO_DRV_L3RAM_SIZE)
		if err != nil {
			return err
		}
		d.l3RamMem, err = syscall.Mmap(
			int(d.mmapFdFile.Fd()),
			int64(PRUSS_UIO_MAP_OFFSET_L3RAM),
			int(l3RamMapSize),
			syscall.PROT_READ|syscall.PROT_WRITE,
			syscall.MAP_SHARED,
		)
	}

	d.extRamPhysBase, err = readIntFromFile(PRUSS_UIO_DRV_EXTRAM_BASE)
	if err != nil {
		return err
	}
	extRamMapSize, err := readIntFromFile(PRUSS_UIO_DRV_EXTRAM_SIZE)
	if err != nil {
		return err
	}
	d.extRamMem, err = syscall.Mmap(
		int(d.mmapFdFile.Fd()),
		int64(PRUSS_UIO_MAP_OFFSET_EXTRAM),
		int(extRamMapSize),
		syscall.PROT_READ|syscall.PROT_WRITE,
		syscall.MAP_SHARED,
	)

	return
}

func (d *prussDrv) OpenInterrupt(evtOut EvtOut) (l EventListener, err error) {
	if l, found := d.eventListeners[evtOut]; found {
		return l, nil
	}

	f, err := os.OpenFile(
		"/dev/uio"+strconv.Itoa(int(evtOut)),
		os.O_SYNC|os.O_RDWR,
		0600,
	)

	if err != nil {
		return
	}

	ret := &eventListener{
		f:   f,
		drv: d,
		in:  make(chan bool),
		out: make(chan uint32),
		evt: evtOut,
	}

	d.eventListeners[evtOut] = ret
	go ret.consumeEvents()

	// We initialize memmap stuff here since it uses it
	if d.mmapFdFile == nil {
		if err = d.memmapInit(); err != nil {
			return nil, err
		}
	}

	return ret, nil
}

func (d *prussDrv) Pru(pruNum PruNum) (pru Pru) {
	return &d.prus[pruNum]
}

// Returns an int pointer into the interrupt register offset by bytes
func (d *prussDrv) intcOffsetInt(offsetBytes uint32, offsetInt int) *uint32 {
	offset := int(offsetBytes) + offsetInt*int(unsafe.Sizeof(uint32(0)))
	if offset+int(unsafe.Sizeof(uint32(0)))-1 > len(d.intcMem) {
		log.Panicf("Invalid offset %d. Max size of intc is %d", offset, len(d.intcMem))
	}
	sPtr := &d.intcMem[offset]
	return (*uint32)(unsafe.Pointer(sPtr))
}

func (d *prussDrv) intcSetCmr(sysevt EventNum, channel Channel) {
	offset := PRU_INTC_CMR1_REG + uint32(uint16(sysevt) & ^(uint16(0x3)))
	*d.intcOffsetInt(offset, 0) |= ((uint32(channel) & 0xF) << ((uint32(sysevt) & 0x3) << 3))
}

func (d *prussDrv) intcSetHmr(channel Channel, host Host) {
	offset := PRU_INTC_HMR1_REG + uint32(uint16(channel) & ^(uint16(0x3)))
	*d.intcOffsetInt(offset, 0) |= ((uint32(host) & 0xF) << ((uint32(channel) & 0x3) << 3))
}

func (d *prussDrv) getHardwareVersion() (v uint32) {
	return *d.intcOffsetInt(0, 0)
}

// This is a transliteration of pruss's prussdrv_pruintc_init.
func (d *prussDrv) InitInterrupts() (err error) {
	*d.intcOffsetInt(PRU_INTC_SIPR1_REG, 0) = 0xFFFFFFFF
	*d.intcOffsetInt(PRU_INTC_SIPR2_REG, 0) = 0xFFFFFFFF

	for i := 0; i < NUM_PRU_SYS_EVTS; i++ {
		*d.intcOffsetInt(PRU_INTC_CMR1_REG, i) = 0x0
	}

	for evt, channel := range d.initData.SysevtToChannelMap {
		d.intcSetCmr(evt, channel)
	}

	for i := 0; i < NUM_PRU_HOSTS; i++ {
		*d.intcOffsetInt(PRU_INTC_HMR1_REG, i) = 0x0
	}

	for channel, host := range d.initData.ChannelToHostMap {
		d.intcSetHmr(channel, host)
	}

	*d.intcOffsetInt(PRU_INTC_SITR1_REG, 0) = 0x0
	*d.intcOffsetInt(PRU_INTC_SITR2_REG, 0) = 0x0

	var mask1, mask2 uint32

	for _, sysevt := range d.initData.SysevtsEnabled {
		switch {
		case sysevt < 32:
			mask1 |= 1 << sysevt
		case sysevt < 64:
			mask2 |= 1 << (sysevt - 32)
		default:
			return fmt.Errorf("SYS_EVT%d out of range", sysevt)
		}
	}

	*d.intcOffsetInt(PRU_INTC_ESR1_REG, 0) = mask1
	*d.intcOffsetInt(PRU_INTC_SECR1_REG, 0) = mask1
	*d.intcOffsetInt(PRU_INTC_ESR2_REG, 0) = mask2
	*d.intcOffsetInt(PRU_INTC_SECR2_REG, 0) = mask2

	for _, h := range d.initData.HostEnabled {
		// We set this one at a time to enable the interrupts
		// see http://elinux.org/PRUSSv2_Interrupt_Controller#Enabling_the_Interrupt_Controller
		if h < MAX_HOSTS_SUPPORTED {
			*d.intcOffsetInt(PRU_INTC_HIEISR_REG, 0) = uint32(h)
		}
	}

	*d.intcOffsetInt(PRU_INTC_GER_REG, 0) = 0x1

	if d.getHardwareVersion() != 0x4E82A900 {
		return fmt.Errorf("Unexpected hardware version")
	}

	return
}

func clearAndMunmap(v *[]byte) {
	if *v != nil {
		syscall.Munmap(*v)
		*v = nil
	}
}

func (d *prussDrv) Close() (err error) {
	for _, l := range d.eventListeners {
		l.Close()
	}

	clearAndMunmap(&d.prus[0].dataramMem)
	d.intcMem = nil
	d.sharedramMem = nil
	if enableL3ram {
		clearAndMunmap(&d.l3RamMem)
	}
	clearAndMunmap(&d.extRamMem)

	for _, pru := range d.prus {
		pru.controlMem = nil
		pru.controlMemPtr = nil
		pru.dataramMem = nil
		pru.debugMem = nil
		pru.iramMem = nil
	}

	d.mmapFdFile = nil

	return
}

func (p *prussPru) Enable() (err error) {
	*p.controlMemPtr = 2
	return
}

func (p *prussPru) Reset() (err error) {
	*p.controlMemPtr = 0
	return
}
func (p *prussPru) Disable() (err error) {
	*p.controlMemPtr = 1
	return
}

func (p *prussPru) WriteMemory(ramType PruRamType, writeOffset uint, sourceData []byte) (err error) {
	var memory []byte
	switch ramType {
	case IRAM:
		memory = p.iramMem
	case DATARAM:
		memory = p.dataramMem
	case SHARED:
		memory = p.drv.sharedramMem
	}

	copy(memory[writeOffset:], sourceData)

	return
}

func (p *prussPru) ExecImage(image []byte) (err error) {

	if err = p.Disable(); err != nil {
		return
	}

	if err = p.WriteMemory(IRAM, 0, image); err != nil {
		return
	}

	if err = p.Enable(); err != nil {
		return
	}
	return
}

func (p *prussPru) ExecProgram(filename string) (err error) {
	b, err := ioutil.ReadFile(filename)

	if err != nil {
		return
	}

	if len(b) >= PRUSS_IRAM_SIZE {
		err = fmt.Errorf(
			"Image size (%d) from (%s) exceeds max iram size (%d)",
			len(b),
			filename,
			PRUSS_IRAM_SIZE,
		)
		return
	}

	return p.ExecImage(b)
}

func (p *prussPru) DefaultEvtOut() EvtOut {
	return PruEvts[p.pruNum]
}

func (p *prussPru) DataramMem() []byte {
	return p.dataramMem
}

func (d *prussDrv) ExtRamMem() []byte {
	return d.extRamMem
}
func (d *prussDrv) ExtRamPhys() uintptr {
	return uintptr(d.extRamPhysBase)
}

func (d *prussDrv) SharedRamMem() []byte {
	return d.sharedramMem
}

func (d *prussDrv) SendEvent(eventNum EventNum) (err error) {

	switch {
	case eventNum < 32:
		*d.intcOffsetInt(PRU_INTC_SRSR1_REG, 0) = uint32(eventNum)
	case eventNum < 64:
		*d.intcOffsetInt(PRU_INTC_SRSR2_REG, 0) = uint32(eventNum) - 32
	default:
		return fmt.Errorf("Invalid EventNum: %d", eventNum)
	}

	return
}

func (d *prussDrv) GetPhysicalAddress(address unsafe.Pointer) (physAddr uintptr) {

	addr := uintptr(address)
	dataRamBase := uintptr(unsafe.Pointer(&d.prus[0].dataramMem[0]))

	var l3base, l3end uintptr

	if enableL3ram {
		l3base = uintptr(unsafe.Pointer(&d.l3RamMem[0]))
		l3end = l3base + uintptr(len(d.l3RamMem))
	}

	extBase := uintptr(unsafe.Pointer(&d.extRamMem[0]))
	extEnd := extBase + uintptr(len(d.extRamMem))

	switch {
	case addr >= dataRamBase && addr < dataRamBase+uintptr(len(d.prus[0].dataramMem)):
		physAddr = addr - dataRamBase + d.extRamPhysBase

	case enableL3ram && addr >= l3base && addr < l3end:
		physAddr = addr - l3base + d.l3RamPhysBase

	case addr >= extBase && addr < extEnd:
		physAddr = addr - extBase + d.extRamPhysBase
	}
	return
}

/*
   unsigned int retaddr = 0;
   if ((address >= prussdrv.pru0_dataram_base)
       && (address <
           prussdrv.pru0_dataram_base + prussdrv.pruss_map_size)) {
       retaddr =
           ((unsigned int) (address - prussdrv.pru0_dataram_base) +
            prussdrv.pru0_dataram_phy_base);
   } else if ((address >= prussdrv.l3ram_base)
              && (address <
                  prussdrv.l3ram_base + prussdrv.l3ram_map_size)) {
       retaddr =
           ((unsigned int) (address - prussdrv.l3ram_base) +
            prussdrv.l3ram_phys_base);
   } else if ((address >= prussdrv.extram_base)
              && (address <
                  prussdrv.extram_base + prussdrv.extram_map_size)) {
       retaddr =
           ((unsigned int) (address - prussdrv.extram_base) +
            prussdrv.extram_phys_base);
   }
   return retaddr;
*/

func (el *eventListener) ClearEvent(eventNum EventNum) (err error) {
	switch {
	case eventNum < 32:
		*el.drv.intcOffsetInt(PRU_INTC_SECR1_REG, 0) = uint32(eventNum)
	case eventNum < 64:
		*el.drv.intcOffsetInt(PRU_INTC_SECR2_REG, 0) = uint32(eventNum) - 32
	default:
		return fmt.Errorf("Invalid event num: %d", eventNum)
	}

	*el.drv.intcOffsetInt(PRU_INTC_HIEISR_REG, 0) = uint32(el.evt) + 2

	return
}

func (el *eventListener) consumeEvents() {
	defer close(el.out)

	buf := make([]byte, 4)

	numEventsBuff := (*uint32)(unsafe.Pointer(&buf[0]))

	for _ = range el.in {
		n, err := el.f.Read(buf)
		switch {
		case err == nil && n == len(buf):
			el.out <- *numEventsBuff
			*el.drv.intcOffsetInt(PRU_INTC_HIEISR_REG, 0) = uint32(el.evt) + 2
		case err == io.EOF:
			return
		case err == nil && n != len(buf):
			log.Panicln("read bytes not as long as buff.  don't know what to do :(")
		default:
			log.Panicln("error reading stream. error: ", err)
		}
	}
}

func (el *eventListener) Wait() uint32 {
	return <-el.WaitC()
}

func (el *eventListener) WaitC() <-chan uint32 {
	el.in <- true
	return el.out
}

func (el *eventListener) Close() {
	close(el.in)
	for _ = range el.out {
	}
	el.f.Close()
}
