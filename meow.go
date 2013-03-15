package main

/*
#include <stdio.h>
#include "meow.h"
#cgo LDFLAGS: -L./ -lmeow
*/
import "C"

import (
	"fmt"
)

func main() {
	initConverter()
	for i := 0; i<20; i++ {
		simple("sample.jpg")
	}
	fmt.Scanf("asdf")
	destroyConverter()
}

func initConverter() {
	C.start()
}

func destroyConverter() {
	C.finish()
}

func simple(filename string) {
	cfilename := C.CString(filename)
	result := C.convert_teh_stuff(cfilename)
	output := C.GoString(result)
	// TODO free the c string and the result
	// C.free(unsafe.Pointer(cfilename))
	// C.free(unsafe.Pointer(result))
	fmt.Println(output)
}
