#ifndef BOOT_MEMORY_H
#define BOOT_MEMORY_H

#include "boot.h"
#include <xboxkrnl/xboxkrnl.h>

void __inline * xbememcpy(void *dest, const void *src, SIZE_T size) {

	__asm__  (
		"    push %%esi    \n"
		"    push %%edi    \n"
		"    push %%ecx    \n"
		"    mov %0, %%esi \n"
		"    mov %1, %%edi \n"
		"    mov %2, %%ecx \n"
		"    push %%ecx    \n"
		"    shr $2, %%ecx \n"
		"    rep movsl     \n"
		"    pop %%ecx     \n"
		"    and $3, %%ecx \n"
		"    rep movsb     \n"
		"    pop %%ecx     \n"
		"    pop %%edi     \n"
		"    pop %%esi     \n"
		: : "S" (src), "D" (dest), "c" (size)
	);
	return dest;
}
void * xbestrcpy(void *, char *);
void xbememset(void *, int,  SIZE_T);

#endif