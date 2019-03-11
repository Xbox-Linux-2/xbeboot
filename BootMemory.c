#include "BootMemory.h"
#include <xboxrt/debug.h>

void * xbestrcpy(void *dest, char *data)
{
   char *saved = dest;
   while ((*(char *)dest++ = *data++) != '\0');

   return saved;
}

void xbememset(void *dest, int data,  SIZE_T size)
{
	char *loc = dest;
	int p;
	if (size != 0) {
		for (int i = 0; i < (int)size; i++) {
			*(loc + i) = data;
			p = i;
		}
	}
}