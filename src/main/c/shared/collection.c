#include <stdlib.h>

#include "collection.h"

void __clear(void ***collection) {
	if (*collection != NULL) {
		free(*collection);
	}
	*collection = NULL;
}

int __set(void ***collection, void *item) {
	void **__collection = *collection;
	size_t size = 0;
	if (__collection != NULL) {
		for (; *__collection != NULL; __collection++) {
			size++;
		}
	}
	__collection = (void **) realloc(*collection, (size + 2) * sizeof(void *));
	if (__collection == NULL) {
		return -1;
	}
	__collection[size] = item;
	__collection[size + 1] = NULL;
	*collection = __collection;
	return 0;
}

void __unset(void ***collection, void *item) {
	void **__collection = *collection;
	if (__collection != NULL) {
		while (*__collection != NULL) {
			if (*__collection == item) {
				void **shift = __collection;
				do {
					shift[0] = shift[1];
				} while (*shift++);
			} else {
				__collection++;
			}
		}
	}
}
