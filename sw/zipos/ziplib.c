#include "ziplib.h"

void *memset(void *s, int c, unsigned n) {
	int	*p = s;
	do {
		*p++ = c;
	} while(n-- > 0);
}

