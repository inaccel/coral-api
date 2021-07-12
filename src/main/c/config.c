#include <stdlib.h>

#include "config.h"

char *INACCEL;

__attribute__ ((constructor (101)))
static void __init(void);

static void __init(void) {
	if (!(INACCEL = getenv("INACCEL"))) {
		INACCEL = "/var/lib/inaccel";
	}
}
