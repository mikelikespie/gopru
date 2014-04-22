package pruss

import (
	"github.com/mikelikespie/gopru/pasm"
	"testing"
	"unsafe"
)

// one for pru1 one for pru2
var testImages = [][]byte{
	pasm.ForceAssemble(`
#define PRU0_ARM_INTERRUPT      19

.origin 0
.entrypoint START

START:
    LBCO	r0, C4, 4, 4
    CLR		r0, r0, 4
    SBCO	r0, C4, 4, 4

    // Send notification to host to say hi
    MOV R31.b0, PRU0_ARM_INTERRUPT+16

    HALT
`),
	pasm.ForceAssemble(`
#define PRU1_ARM_INTERRUPT      20

.origin 0
.entrypoint START

START:
    LBCO	r0, C4, 4, 4
    CLR		r0, r0, 4
    SBCO	r0, C4, 4, 4

    // Send notification to host to say hi
    MOV R31.b0, PRU1_ARM_INTERRUPT+16

    HALT
`),
}

func TestPrusses(t *testing.T) {
	drv, err := InitDrv()
	if err != nil {
		t.Fatal("Error initializing:", err)
	}

	defer func() {
		err := drv.Close()
		if err != nil {
			t.Fatal("Error closing:", err)
		}
	}()

	listeners := make([]EventListener, PruCount)

	for i := Pru0; i < PruCount; i++ {
		pru := drv.Pru(i)
		if listeners[i], err = drv.OpenInterrupt(pru.DefaultEvtOut()); err != nil {
			t.Fatal("Error opening interrupt:", err)
		}
	}

	if err = drv.InitInterrupts(); err != nil {
		t.Fatal("Error initializing interrupts:", err)
	}

	for i := Pru0; i < PruCount; i++ {
		pru := drv.Pru(i)
		if err = pru.Enable(); err != nil {
			t.Fatal("Error enabling pru", i, ":", err)
		}
		if err = pru.Disable(); err != nil {
			t.Fatal("Error disabling pru", i, ":", err)
		}
		if err = pru.Reset(); err != nil {
			t.Fatal("Error resetting pru", i, ":", err)
		}
	}

	for i := Pru0; i < PruCount; i++ {
		pru := drv.Pru(i)
		image := testImages[i]

		if err = pru.ExecImage(image); err != nil {
			t.Fatal("Error executing image", i, ":", err)
		}

		listeners[i].Wait()
		listeners[i].ClearEvent([]EventNum{PRU0_ARM_INTERRUPT, PRU1_ARM_INTERRUPT}[i])
	}
}

// This adds 0x01010101 to the beginning of ddr memory

var extMemImage = pasm.ForceAssemble(`
.origin 0
.entrypoint START

#define PRU0_ARM_INTERRUPT 19
#define ADD_NUMBER         0x01010101

// See http://processors.wiki.ti.com/index.php/Programmable_Realtime_Unit#PRU_Constant_Table_Programmable_Pointer_Register_0_.280x0028.29
#define CONTROL_BASE 0x22000
// Address for the Constant table Programmable Pointer Register 0(CTPPR_0)
#define CTPPR_0         (CONTROL_BASE + 0x0028)

#define CONST_PRUCFG	C4
#define CONST_PRUDRAM   C24
#define CONST_SHAREDRAM C28

#define SHARED_RAM_ADDR 0x00010000

START:
	// Configure the programmable pointer register for PRU0 by setting
	// c28_pointer[15:0] field to 0x0120.  This will make C28 point to
	// 0x00012000 (PRU shared RAM).
	// Docs say: 0x00nn_nn00, nnnn = c28_pointer[15:0]
	/// This value is shifted left 8 
	MOV	r0, SHARED_RAM_ADDR >> 8
	MOV	r1, CTPPR_0
	SBBO    r0, r1, 0, 4

	MOV  r3, ADD_NUMBER
	
	// r2 is now the pointer to the DMA segment in DDR
	// Our app will put the pointer in it for us
	LBCO r2, CONST_PRUDRAM, 0, 4
	LBBO r1, r2, 0,  4
	ADD  r1, r1, r3
	SBBO r1, r2, 0,  4

	LBCO r1, CONST_SHAREDRAM, 0,  4
	ADD  r1, r1, r3
	SBCO r1, CONST_SHAREDRAM, 0,  4

	// We're done 
	MOV R31.b0, PRU0_ARM_INTERRUPT+16
	
HALT
`)

func TestExtMem(t *testing.T) {
	drv, err := InitDrv()
	if err != nil {
		t.Fatal("Error initializing:", err)
	}

	defer func() {
		err := drv.Close()
		if err != nil {
			t.Fatal("Error closing:", err)
		}
	}()

	pru := drv.Pru(Pru0)

	listener, err := drv.OpenInterrupt(pru.DefaultEvtOut())
	if err != nil {
		t.Fatal("Error opening interrupt:", err)
	}

	if err = drv.InitInterrupts(); err != nil {
		t.Fatal("Error initializing interrupts:", err)
	}

	extRam := drv.ExtRamMem()

	if len(extRam) < 0x40000 {
		t.Fatal("extRam smaller than expected size")
	}

	start := uint32(0xAABBCCDD)
	expected := start + 0x01010101

	extPtr := (*uint32)(unsafe.Pointer(&extRam[0]))
	*extPtr = start

	// Pointer to shared memory. Not 100% sure why 2000 offset maps to
	sharedPtr := (*uint32)(unsafe.Pointer(&drv.SharedRamMem()[0x0]))
	*sharedPtr = start

	// Write to the dataram so the code can know where the extmem is
	*(*uint32)(unsafe.Pointer(&pru.DataramMem()[0])) = uint32(drv.ExtRamPhys())

	if err = pru.ExecImage(extMemImage); err != nil {
		t.Fatal("Error executing image")
	}

	defer pru.Disable()

	listener.Wait()

	if *sharedPtr != expected {
		t.Fatalf("PRU did not manipulate shared memory as expected. expected %X, got %X", expected, *extPtr)
	}

	if *extPtr != expected {
		t.Fatalf("PRU did not manipulate memory as expected. expected %X, got %X", expected, *extPtr)
	}
}
