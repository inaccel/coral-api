package com.inaccel.coral;

import com.sun.jna.Library;
import com.sun.jna.Native;

interface C extends Library {

	C library = (C) Native.load("c", C.class);

	String strerror(int errnum);

}
