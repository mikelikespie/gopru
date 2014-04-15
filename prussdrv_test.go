package pruss

import (
	"testing"
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
