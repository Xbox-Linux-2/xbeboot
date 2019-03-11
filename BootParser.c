#include "xbox.h"
#include <xboxrt/debug.h>
#include <pbkit/pbkit.h>
#include <hal/xbox.h>
#include "stdio.h"
#include <xboxkrnl/xboxkrnl.h>
#include "BootString.h"
#include "BootParser.h"
#include "BootEEPROM.h"
#include "boot.h"
#include "BootMemory.h"
#include <xboxrt/string.h>
#include <xboxrt/stdlib.h>
#include <xboxrt/ctype.h>
#include <hal/xbox.h>

char * strpbrk(const char * cs,const char * ct)
{
        const char *sc1,*sc2;

        for( sc1 = cs; *sc1 != '\0'; ++sc1) {
		for( sc2 = ct; *sc2 != '\0'; ++sc2) {
			if (*sc1 == *sc2) return (char *) sc1;
		}
	}
	return NULL;
}

char * strsep(char **s, const char *ct)
{
        char *sbegin = *s, *end;

       	if (sbegin == NULL) return NULL;

	end = strpbrk(sbegin, ct);
	if (end) *end++ = '\0';
	*s = end;
	return sbegin;
}

int sprintf(char * buf, const char *fmt, ...);

void chrreplace(char *string, char search, char ch) {
	char *ptr = string;
	while(*ptr != 0) {
		if(*ptr == search) {
			*ptr = ch;
		} else {
			ptr++;
		}
	}
}


int ParseXBEConfig(char *szPath,char *szBuffer, CONFIGENTRY *entry) {
	char *szLine;
	char *szTmp;
	char *szNorm;
        int nXboxFB = 0;
        int nVesaFB = 0;
	char *ptr;
	//,ptr1;
	int i;
	
        szLine = (char *)MmAllocateContiguousMemoryEx(MAX_LINE,MIN_KERNEL,
	                        MAX_KERNEL, 0, PAGE_READWRITE);
        szTmp = (char *)MmAllocateContiguousMemoryEx(MAX_LINE,MIN_KERNEL,
	                        MAX_KERNEL, 0, PAGE_READWRITE);
        szNorm = (char *)MmAllocateContiguousMemoryEx(MAX_LINE,MIN_KERNEL,
	                        MAX_KERNEL, 0, PAGE_READWRITE);

	xbememset(entry,0,sizeof(CONFIGENTRY));
	
	ptr = szBuffer;
	ptr = HelpGetToken(szBuffer,10);
	int configValid = 1;
	HelpCopyUntil(entry->szPath,szPath,MAX_LINE);
	while(1) {
		xbememcpy(szLine,ptr,HelpStrlen(ptr));
		if(HelpStrlen(ptr) < MAX_LINE) {
			if(HelpStrncmp(ptr,"kernel",HelpStrlen("kernel")) == 0)  {
				HelpGetParm(szTmp, ptr);
				HelpCopyUntil(entry->szKernel,szPath,MAX_LINE);
				HelpCopyUntil(HelpScan0(entry->szKernel),szTmp,MAX_LINE);
			}
			if(HelpStrncmp(ptr,"initrd",HelpStrlen("initrd")) == 0) {
				HelpGetParm(szTmp, ptr);
				HelpCopyUntil(entry->szInitrd,szPath,MAX_LINE);
				HelpCopyUntil(HelpScan0(entry->szInitrd),szTmp,MAX_LINE);
			}
			if(HelpStrncmp(ptr,"xboxfb",HelpStrlen("xboxfb")) == 0) {
				nXboxFB = 1;
			}
			if(HelpStrncmp(ptr,"vesafb",HelpStrlen("vesafb")) == 0) {
				nVesaFB = 1;
			}
			if(HelpStrncmp(ptr,"append",HelpStrlen("append")) == 0)
				HelpGetParm(entry->szAppend, ptr);
		} else {
			configValid = 0;
		}
		ptr = HelpGetToken(0,10);
		if(*ptr == 0) break;
	}

	if(nXboxFB == 1) {
		strcpy(szNorm," video=xbox:640x480,nohwcursor ");
	}

	if(nVesaFB == 1) {
		strcpy(szNorm," video=vesa:640x480 ");
	}
	if(szNorm[0] != 0) {
		debugPrint(entry->szAppend,"%s%s",entry->szAppend,szNorm);
	}

	MmFreeContiguousMemory(szLine);
	MmFreeContiguousMemory(szTmp);
	MmFreeContiguousMemory(szNorm);

	chrreplace(entry->szInitrd, '/', '\\');
	chrreplace(entry->szKernel, '/', '\\');

	return configValid;
}

void ParseConfig(char *szPath, char *szBuffer, CONFIGENTRY *entry) {
	char *szLine;
	char *szTmp;
	char *szNorm;
    int nXboxFB = 0;
    int nVesaFB = 0;
	char *ptr;
	int i;
    szLine = (char *)MmAllocateContiguousMemoryEx(MAX_LINE, MIN_KERNEL, MAX_KERNEL, 0, PAGE_READWRITE);
    szTmp = (char *)MmAllocateContiguousMemoryEx(MAX_LINE, MIN_KERNEL, MAX_KERNEL, 0, PAGE_READWRITE);
    szNorm = (char *)MmAllocateContiguousMemoryEx(MAX_LINE, MIN_KERNEL, MAX_KERNEL, 0, PAGE_READWRITE);

	ptr = szBuffer;
	ptr = HelpGetToken(szBuffer,10);
	entry->nValid = 1;
	HelpCopyUntil(entry->szPath,szPath,MAX_LINE);
	while(1) {
		xbememcpy(szLine,ptr,HelpStrlen(ptr));
		if(HelpStrlen(ptr) < MAX_LINE) {
			if(HelpStrncmp(ptr,"kernel",HelpStrlen("kernel")) == 0)  {
				HelpGetParm(szTmp, ptr);
				HelpCopyUntil(HelpScan0(entry->szKernel),szTmp,MAX_LINE);
			}
			if(HelpStrncmp(ptr,"initrd",HelpStrlen("initrd")) == 0) {
				HelpGetParm(szTmp, ptr);
				HelpCopyUntil(HelpScan0(entry->szInitrd),szTmp,MAX_LINE);
			}
			if(HelpStrncmp(ptr,"xboxfb",HelpStrlen("xboxfb")) == 0) {
				nXboxFB = 1;
			}
			if(HelpStrncmp(ptr,"vesafb",HelpStrlen("vesafb")) == 0) {
				nVesaFB = 1;
			}
			if(HelpStrncmp(ptr,"append",HelpStrlen("append")) == 0)
				HelpGetParm(entry->szAppend, ptr);
		} else {
			entry->nValid = 0;
		}
		ptr = HelpGetToken(0,10);
		if(*ptr == 0) break;
	}

	if(nXboxFB == 1) {
		strcpy(szNorm," video=xbox:640x480,nohwcursor ");
	}

	if(nVesaFB == 1) {
		strcpy(szNorm," video=vesa:640x480 ");
	}

	MmFreeContiguousMemory(szLine);
	MmFreeContiguousMemory(szTmp);
	MmFreeContiguousMemory(szNorm);

	chrreplace(entry->szInitrd, '/', '\\');
	chrreplace(entry->szKernel, '/', '\\');
}

void PrintConfig(CONFIGENTRY* entry) {
        debugPrint("path \"%s\"\n", entry->szPath);
        debugPrint("kernel \"%s\"\n", entry->szKernel);
        debugPrint("initrd \"%s\"\n", entry->szInitrd);
        debugPrint("vmode \"%d\"\n", entry->vmode);
        debugPrint("command line: \"%s\"\n", entry->szAppend);
}