#ifndef _BootParser_H_
#define _BootParser_H_

#define MAX_LINE 1024

#include <stdint.h>

struct CONFIGENTRY;

enum BootTypes {
	BOOT_CDROM,
	BOOT_FATX,
	BOOT_NATIVE
};

typedef struct _CONFIGENTRY {
    int  nValid;
	char szPath[MAX_LINE];
    char szKernel[MAX_LINE];
    char szInitrd[MAX_LINE];
    char szAppend[MAX_LINE];
	int vmode;
} CONFIGENTRY, *LPCONFIGENTRY;

char * strpbrk(const char * cs,const char * ct);
char * strsep(char **s, const char *ct);
int isspace (int c);
int ParseXBEConfig(char *szPath,char *szBuffer, CONFIGENTRY *entry);
void ParseConfig(char *szPath,char *szBuffer, CONFIGENTRY *entry);
void PrintConfig(CONFIGENTRY* entry);

#endif // _BootParser_H_