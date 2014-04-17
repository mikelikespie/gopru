package pasm

import (
	"testing"
)


func TestPasm(t *testing.T) {
	bytes, err := Assemble(`
.origin 0
.entrypoint START

START:

    HALT

	`)

	if err != nil {
		t.Fatal("Error assembling", err)
	}

	if bytes == nil {
		t.Fatal("Bytes is nil")
	}

	if len(bytes) == 0 {
		t.Fatal("Image should not be of length 0")
	}
}
