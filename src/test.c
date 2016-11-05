#include "shell2.h"

#ifdef TEST

#include <stdlib.h>
static int ok = 1;

#define BCHECK(x) printf(#x); \
	if(x) { \
		printf(": OK\n"); \
	} else { \
		ok = 0; \
		printf(": FAILED\n"); \
	}; \

#define SCHECK(a, b) printf(#a " = " #b); \
	if(!strcmp(a, b)) { \
		printf(": OK\n"); \
	} else { \
		ok = 0; \
		printf(": FAILED: '%s' /= '%s'\n", a, b); \
	}; \









int main(void) {





	return EXIT_SUCCESS;
}
#endif
