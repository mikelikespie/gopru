package pruss

import (
	"github.com/mikelikespie/gopru/pasm"
	"testing"
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

func TestPruss(t *testing.T) {

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

	evts := []EvtOut{PRU_EVTOUT_0, PRU_EVTOUT_1}

	listeners := make([]EventListener, len(evts))

	// Open up
	for i, evt := range evts {
		if listeners[i], err = drv.OpenInterrupt(evt); err != nil {
			t.Fatal("Error opening interrupt:", err)
		}
	}

	if err = drv.InitInterrupts(); err != nil {
		t.Fatal("Error initializing interrupts:", err)
	}


	for i := 0; i < 2; i++ {
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


	for i := 0; i < 2; i++ {
		pru := drv.Pru(i)
		image := testImages[i]

		if err = pru.ExecImage(image); err != nil {
			t.Fatal("Error executing image", i, ":", err)
		}

		listeners[i].Wait()
		listeners[i].ClearEvent([]EventNum{PRU0_ARM_INTERRUPT, PRU1_ARM_INTERRUPT}[i])
	}
}
