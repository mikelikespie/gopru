package pasm

/*
#cgo CFLAGS: -D_UNIX_
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "pasm.h"
*/
import "C"
import (
	"fmt"
	"io/ioutil"
	"os"
	"path"
	"sync"
)

const (
	OPTION_BINARY        = C.OPTION_BINARY
	OPTION_BINARYBIG     = C.OPTION_BINARYBIG
	OPTION_CARRAY        = C.OPTION_CARRAY
	OPTION_IMGFILE       = C.OPTION_IMGFILE
	OPTION_DBGFILE       = C.OPTION_DBGFILE
	OPTION_LISTING       = C.OPTION_LISTING
	OPTION_DEBUG         = C.OPTION_DEBUG
	OPTION_BIGENDIAN     = C.OPTION_BIGENDIAN
	OPTION_RETREGSET     = C.OPTION_RETREGSET
	OPTION_SOURCELISTING = C.OPTION_SOURCELISTING

	CORE_NONE = C.CORE_NONE
	CORE_V0   = C.CORE_V0
	CORE_V1   = C.CORE_V1
	CORE_V2   = C.CORE_V2
	CORE_V3   = C.CORE_V3
)

func setOptions(options uint) {
	C.Options = C.uint(options)
}

func setCore(core uint) {
	C.Core = C.uint(core)
}

// Pasm uses globals.  We have to have a mutex on this
var lock sync.Mutex

// Panics if there's an error.  Useful for consts, etc
func ForceAssemble(assembly string) (image []byte) {
	image, err := Assemble(assembly)
	if err != nil {
		panic(err)
	}
	return image
}

func Assemble(assembly string) (image []byte, err error) {
	lock.Lock()
	defer lock.Unlock()
	setOptions(OPTION_BINARY)
	setCore(CORE_V3)

	dir, err := ioutil.TempDir("", "pasm-compilation")
	if err != nil {
		panic("Could not create temp directory")
	}
	defer os.RemoveAll(dir)

	baseFile := path.Join(dir, "assembly")

	outFile := baseFile + ".p"

	assembledFile := baseFile + ".bin"

	if err = ioutil.WriteFile(outFile, []byte(assembly), 0600); err != nil {
		panic("could not write assembly file")
	}

	ret := int(C.runMain(C.CString(outFile), C.CString(assembledFile)))

	if ret != 0 {
		err = fmt.Errorf("Error assembling file with return code %d. See standard out", ret)
		return
	}

	image, err = ioutil.ReadFile(assembledFile)

	return
}
