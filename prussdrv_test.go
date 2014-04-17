package pruss

import (
	"testing"
	"time"
)

func TestPruss(t *testing.T) {

	drv, err := InitDrv()
	if err != nil {
		t.Fatal("Error initializing:", err)
	}

	// Open up
	if _, err = drv.OpenInterrupt(PRU_EVTOUT_0); err != nil {
		t.Fatal("Error opening interrupt:", err)
	}

	if _, err = drv.OpenInterrupt(PRU_EVTOUT_1); err != nil {
		t.Fatal("Error opening interrupt:", err)
	}

	if err = drv.InitInterrupts(); err != nil {
		t.Fatal("Error initializing interrupts:", err)
	}

	// These take a small amount of time to start reading
	time.Sleep(time.Millisecond)

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

	defer func() {
		err := drv.Close()
		if err != nil {
			t.Fatal("Error closing:", err)
		}
	}()
}
