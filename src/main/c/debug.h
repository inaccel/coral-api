#ifndef DEBUG_H
#define DEBUG_H

#ifdef DEBUG
#include <stdio.h>

#define PERROR(...) perror(__VA_ARGS__)

#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PERROR(...) {}

#define PRINTF(...) {}
#endif

#endif // DEBUG_H
