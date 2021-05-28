#include <stdlib.h>

#include "config.h"

char *INACCEL;

__attribute__ ((constructor (101)))
static void __init();

static void __init() {
	if (!(INACCEL = getenv("INACCEL"))) {
		INACCEL = "/var/lib/inaccel";
	}
}
