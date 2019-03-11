/*
   Xbox XBE bootloader

    by Michael Steil & anonymous
    VESA Framebuffer code by Milosch Meriac
*/

#include "consts.h"
#include "xbox.h"
#include "boot.h"
#include "BootVideo.h"
#include "BootString.h"
#include "BootParser.h"
#include "BootEEPROM.h"
#include "BootMemory.h"
#include "config.h"
#include <xboxrt/debug.h>
#include <xboxrt/string.h>
#include <pbkit/pbkit.h>
#include <hal/xbox.h>
#include <hal/fileio.h>
#include <xboxkrnl/xboxkrnl.h>
#include <stdlib.h>

int NewFramebuffer = 0;
long KernelSize = 0;
PHYSICAL_ADDRESS PhysKernelPos, PhysEscapeCodePos;
PVOID EscapeCodePos;
EEPROMDATA eeprom;
static CONFIGENTRY entry;

int ReadFile(HANDLE Handle, PVOID Buffer, ULONG Size, BOOL quiet);

int WriteFile(HANDLE Handle, PVOID Buffer, ULONG Size);
int SaveFile(char *szFileName, BYTE *Buffer, ULONG Size);
void DismountFileSystems(void);
//int RemapDrive(char *szDrive);
HANDLE OpenFile(HANDLE Root, LPCSTR Filename, LONG Length, ULONG Mode);
BOOL GetFileSize(HANDLE File, LONGLONG *Size);

void GetConfig(void);
NTSTATUS GetConfigXBE(CONFIGENTRY *entry);

void die() {
	while(1);
}

#ifdef LOADHDD
/* Loads the kernel image file into contiguous physical memory */
long LoadFile(PVOID Filename, long *lFileSize) {

	HANDLE hFile;
	BYTE *Buffer = 0;
	ULONGLONG FileSize;

    if (!(hFile = OpenFile(NULL, Filename, -1, FILE_NON_DIRECTORY_FILE))) {
		debugPrint("Error opening file %s\n",Filename);
	}

	if(!GetFileSize(hFile,(long long *)&FileSize)) {
		debugPrint("Error getting file size %s\n",Filename);
	}

	Buffer = MmAllocateContiguousMemoryEx(FileSize + 0x1000, MIN_KERNEL, MAX_KERNEL, 0, PAGE_READWRITE);
	if (!Buffer) {
		debugPrint("Error allocating memory for file %s\n",Filename);
	}

	xbememset(Buffer,0xff,FileSize + 0x1000);

	if (!ReadFile(hFile, Buffer, FileSize, TRUE)) {
		debugPrint("Error loading file %s\n",Filename);
	}

	NtClose(hFile);

	*lFileSize = FileSize + 0x1000;

	return (long)Buffer;
}
#endif

#ifdef LOADXBE
long LoadKernelXBE(long *FileSize) {

	PVOID Buffer;
	/* Size of the kernel file */
	ULONGLONG TempKernelStart;
	ULONGLONG TempKernelSize;
	ULONGLONG TempKernelSizev;
	TempKernelSize = *FileSize;

	// This is where the real kernel starts in the XBE
	xbememcpy(&TempKernelStart,(void*)0x011080,4);
	// This is the real kernel Size
	xbememcpy(&TempKernelSizev,(void*)0x011080+0x04,4);
	// this is the kernel size we pass to the kernel loader
	xbememcpy(&TempKernelSize,(void*)0x011080+0x08,4);

	*FileSize = TempKernelSize;

	Buffer = MmAllocateContiguousMemoryEx((ULONG) TempKernelSize, MIN_KERNEL, MAX_KERNEL, 0, PAGE_READWRITE);
	if (!Buffer) return 0;

	// We fill the complete space with 0xff
	xbememcpy(Buffer,(void*)0x010000+TempKernelStart,TempKernelSizev);
	xbememset(Buffer+TempKernelSizev,0xff,TempKernelSize-TempKernelSizev);

	// We force the cache to write back the changes to RAM
	asm volatile ("wbinvd\n");

    return (long)Buffer;
}

long LoadIinitrdXBE(long *FileSize) {

	PVOID Buffer;
	/* Size of the initrd file */
	ULONGLONG TempInitrdStart;
	ULONGLONG TempInitrdSize;

	xbememcpy(&TempInitrdStart,(void*)0x011080+0xC,4);	// This is the where the real kernel starts in the XBE
	xbememcpy(&TempInitrdSize,(void*)0x011080+0x10,4);	// This is the real kernel Size

	*FileSize= TempInitrdSize;

	Buffer = MmAllocateContiguousMemoryEx((ULONG) TempInitrdSize,
		MIN_KERNEL, MAX_KERNEL, 0, PAGE_READWRITE);

	if (!Buffer) return 0;

	xbememcpy(Buffer,(void*)0x010000+TempInitrdStart,TempInitrdSize);
	// We force the Cache to write back the changes to RAM
	asm volatile ("wbinvd\n");

    return (long)Buffer;
}

#endif

void main(void) {
	HalWriteSMBusValue(0x20, 0x08, FALSE, 0x08);
	HalWriteSMBusValue(0x20, 0x07, FALSE, 0x01);
	long KernelPos;
	long InitrdSize, InitrdPos;
	PHYSICAL_ADDRESS PhysInitrdPos;
	int data_PAGE_SIZE;
	extern int EscapeCode;

	/*framebuffer = (unsigned int*)(0xF0000000+*(unsigned int*)0xFD600800);
	xbememset(framebuffer,0,SCREEN_WIDTH*SCREEN_HEIGHT*4);
	BootVgaInitializationKernelNG((CURRENT_VIDEO_MODE_DETAILS *)&vmode);*/

	I2CTransmitWord(0x10, 0x1a01);
	I2CTransmitWord(0x10, 0x1901); //definitely don't want the xbox to reset on eject

	switch(pb_init())
    {
        case 0: break;
        default:
            XSleep(2000);
            XReboot();
            return;
	}
	pb_show_debug_screen();

	debugPrint("Xbox Linux 2 XBEBOOT\n");

	debugPrint("%s - https://github.com/Xbox-Linux-2/xbeboot\n",__DATE__);
	debugPrint("(C)2002,2018-2019 Xbox Linux Team, Xbox Linux 2 Team\nLicensed under the GPLv2\n\n");

	xbememset(&eeprom, 0, sizeof(EEPROMDATA));
	BootEepromReadEntireEEPROM(&eeprom);
	volatile BYTE * pb=(BYTE *)0xfef000a8;  // Ethernet MMIO base + MAC register offset (<--thanks to Anders Gustafsson)
	int n;
	for(n=5;n>=0;n--) { 
		*pb++=	eeprom.MACAddress[n];  // send it in backwards, its reversed by the driver
	}

	GetConfig();

	PrintConfig(&entry);
	debugPrint("\n\n");

	// Load the kernel image into RAM
	KernelPos = LoadFile(entry.szKernel, &KernelSize);

	/* get physical addresses */
	PhysKernelPos = MmGetPhysicalAddress((PVOID)KernelPos);

	if (KernelPos == 0) {
		debugPrint("Error Loading Kernel\n");
	}

	// ED : only if initrd
	if(entry.szInitrd[0]) {
		InitrdPos = LoadFile(entry.szInitrd, &InitrdSize);
		if (InitrdPos == 0) {
		        debugPrint("Error Loading initrd\n");
		}
		PhysInitrdPos = MmGetPhysicalAddress((PVOID)InitrdPos);
	} else {
		InitrdSize = 0;
		PhysInitrdPos = 0;
	}

	/* allocate memory for EscapeCode */
	EscapeCodePos = MmAllocateContiguousMemoryEx(PAGE_SIZE, RAMSIZE /4, RAMSIZE / 2, 16, PAGE_READWRITE);
	PhysEscapeCodePos = MmGetPhysicalAddress(EscapeCodePos);

	data_PAGE_SIZE = PAGE_SIZE;
	NtAllocateVirtualMemory((PVOID*)&PhysEscapeCodePos, 0, (PSIZE_T) &data_PAGE_SIZE, MEM_RESERVE | MEM_COMMIT, PAGE_EXECUTE_READWRITE);

	/* copy EscapeCode to EscapeCodePos & PhysEscapeCodePos */
	xbememcpy(EscapeCodePos, &EscapeCode, PAGE_SIZE);
	xbememcpy((void*)PhysEscapeCodePos, &EscapeCode, PAGE_SIZE);

	XSleep(5000);

	setup((void*)KernelPos, (void*)PhysInitrdPos, (void*)InitrdSize, entry.szAppend);

	/* orange LED */
	HalWriteSMBusValue(0x20, 0x08, FALSE, 0xff);
	HalWriteSMBusValue(0x20, 0x07, FALSE, 0x01);
	NewFramebuffer = NEW_FRAMEBUFFER + 0xF0000000;

	__asm(
		"mov	_PhysEscapeCodePos, %edx\n"

		/* construct jmp to new CS */
		"mov	%edx, %eax\n"
		"sub	$_EscapeCode, %eax\n"
		"add	$newloc, %eax\n"
		"mov	_EscapeCodePos, %ebx\n"
		"sub	$_EscapeCode, %ebx\n"
		"add	$ptr_newloc, %ebx\n"
		"mov	%eax, (%ebx)\n"

		"mov	_NewFramebuffer, %edi\n"
		"mov	_PhysKernelPos, %ebp\n"
		"mov	_KernelSize, %esp\n"

		"cli\n"
		"jmp	*%edx\n"
	);
}

int WriteFile(HANDLE Handle, PVOID Buffer, ULONG Size)
{
        IO_STATUS_BLOCK IoStatus;

        // Try to write the buffer
        if (!NT_SUCCESS(NtWriteFile(Handle, NULL, NULL, NULL, &IoStatus,
                Buffer, Size, NULL)))
                return 0;

        // Verify that the amount written is the correct size
        if (IoStatus.Information != Size)
                return 0;

        return 1;
}

int ReadFile(HANDLE Handle, PVOID Buffer, ULONG Size, BOOL quiet)
{

    IO_STATUS_BLOCK IoStatus;
    // Try to write the buffer
    if (!NT_SUCCESS(NtReadFile(Handle, NULL, NULL, NULL, &IoStatus,
        Buffer, Size, NULL)))
        return 0;
	// Verify that the amount read is the correct size

	if (!quiet) {
		debugPrint("\nFILE SIZE: %d bytes\n\nCONTENTS:\n", (int)Size);
		for (int i = 0; i < Size; i++) {
			if (*((char*)Buffer + i) == '\n') {
				debugPrint("\n");
			} else {
				debugPrint("%c", *((char*)Buffer + i));
			}
		}
		debugPrint("\n\n");
	}

    if (IoStatus.Information != Size) {
        return 0;
	}

    return 1;
}

int SaveFile(char *szFileName,BYTE* Buffer, ULONG Size) {
	ANSI_STRING DestFileName;
	IO_STATUS_BLOCK IoStatus;
    OBJECT_ATTRIBUTES Attributes;
    HANDLE DestHandle = NULL;

	RtlInitAnsiString(&DestFileName,szFileName);
    Attributes.RootDirectory = NULL;
    Attributes.ObjectName = &DestFileName;
    Attributes.Attributes = OBJ_CASE_INSENSITIVE;

	if (!NT_SUCCESS(NtCreateFile(&DestHandle,
		GENERIC_WRITE  | GENERIC_READ | SYNCHRONIZE,
		&Attributes, &IoStatus,
		NULL, FILE_RANDOM_ACCESS,
		FILE_SHARE_READ, FILE_CREATE,
		FILE_SYNCHRONOUS_IO_NONALERT | FILE_NON_DIRECTORY_FILE))) {
			debugPrint("Error saving File\n");
			return 0;
	}

	if(!WriteFile(DestHandle, Buffer, Size)) {
		debugPrint("Error saving File\n");
		return 0;
	}

	NtClose(DestHandle);

	return 1;
}

// Dismount all file systems
void DismountFileSystems(void) {
        ANSI_STRING String;

        RtlInitAnsiString(&String, "\\Device\\Harddisk0\\Partition1");
        IoDismountVolumeByName(&String);
        RtlInitAnsiString(&String, "\\Device\\Harddisk0\\Partition2");
        IoDismountVolumeByName(&String);
        RtlInitAnsiString(&String, "\\Device\\Harddisk0\\Partition3");
        IoDismountVolumeByName(&String);
        RtlInitAnsiString(&String, "\\Device\\Harddisk0\\Partition4");
        IoDismountVolumeByName(&String);
        RtlInitAnsiString(&String, "\\Device\\Harddisk0\\Partition5");
        IoDismountVolumeByName(&String);
        RtlInitAnsiString(&String, "\\Device\\Harddisk0\\Partition6");
        IoDismountVolumeByName(&String);
}

void GetConfig(void) {
	ULONGLONG FileSize;
	BYTE* Buffer;
	HANDLE hFile;

	char linuxbootFilename[] = "\\Device\\Harddisk0\\Partition1\\linuxboot.cfg"; 

    if (!(hFile = OpenFile(NULL, (LPCSTR)linuxbootFilename, -1, FILE_NON_DIRECTORY_FILE))) {
		die();
	}

	if(!GetFileSize(hFile,(LONGLONG*)&FileSize)) {
		debugPrint("Error getting file size of linuxboot.cfg\n");
		die();
	}

	Buffer = MmAllocateContiguousMemoryEx(FileSize, MIN_KERNEL, MAX_KERNEL, 0, PAGE_READWRITE);

	if (!Buffer) {
		debugPrint("Error alloc memory for linuxboot.cfg\n");
		die();
	}

	debugPrint("linuxboot.cfg ");
	if (!ReadFile(hFile, Buffer, FileSize, FALSE)) {
		debugPrint("Error loading linuxboot.cfg\n");
		die();
	}

	ParseConfig(linuxbootFilename,(char*)Buffer, &entry);

	NtClose(hFile);
}

NTSTATUS GetConfigXBE(CONFIGENTRY *entry) {
	BYTE* Buffer;
    unsigned int TempConfigStart;
    unsigned int TempConfigSize;

	// This is the Real kernel Size
	xbememcpy(&TempConfigStart,(void*)0x011080+0x14,4);
	// This is the kernel size we pass to the Kernel loader
	xbememcpy(&TempConfigSize, (void*)0x011080+0x18,4);

	Buffer = MmAllocateContiguousMemoryEx(CONFIG_BUFFERSIZE, MIN_KERNEL, MAX_KERNEL, 0, PAGE_READWRITE);

    xbememset(Buffer,0x00,CONFIG_BUFFERSIZE);
    xbememcpy(Buffer,(void*)0x010000+TempConfigStart,TempConfigSize);

	return STATUS_SUCCESS;
}

// Opens a file or directory for read-only access
// Length parameter is negative means use strlen()
// This was originally designed to open directories, but it turned out to be
// too much of a hassle and was scrapped.  Use only for files with the
// FILE_NON_DIRECTORY_FILE mode.
HANDLE OpenFile(HANDLE Root, LPCSTR Filename, LONG Length, ULONG Mode)
{
        ANSI_STRING FilenameString;
        OBJECT_ATTRIBUTES Attributes;
        IO_STATUS_BLOCK IoStatus;
        HANDLE Handle;

		debugPrint("Opening %s\n", Filename);
        RtlInitAnsiString(&FilenameString, Filename);

        // Initialize the object attributes
        Attributes.Attributes = OBJ_CASE_INSENSITIVE;
        Attributes.RootDirectory = Root;
        Attributes.ObjectName = &FilenameString;

        // Try to open the file or directory
        if (!NT_SUCCESS(NtCreateFile(&Handle, GENERIC_READ | SYNCHRONIZE,
                &Attributes, &IoStatus, NULL, 0, FILE_SHARE_READ | FILE_SHARE_WRITE
                | FILE_SHARE_DELETE, FILE_OPEN_IF, FILE_SYNCHRONOUS_IO_NONALERT |
                Mode))) {
					debugPrint("Couldn't open file %s\n", Attributes.ObjectName->Buffer);
					if (IoStatus.Status == FILE_CREATED) {
						debugPrint("ERROR: FILE_CREATED\n");
					} else if (IoStatus.Status == FILE_OPENED) {
						debugPrint("ERROR: FILE_OPENED\n");
					} else if (IoStatus.Status == FILE_OVERWRITTEN) {
						debugPrint("ERROR: FILE_OVERWRITTEN\n");
					} else if (IoStatus.Status == FILE_SUPERSEDED) {
						debugPrint("ERROR: FILE_SUPERSEDED\n");
					} else if (IoStatus.Status == FILE_EXISTS) {
						debugPrint("ERROR: FILE_EXISTS\n");
					} else if (IoStatus.Status == FILE_DOES_NOT_EXIST) {
						debugPrint("ERROR: FILE_DOES_NOT_EXIST\n");
					}
                	return NULL;
				}
        return Handle;
}

// Gets the size of a file
BOOL GetFileSize(HANDLE File, LONGLONG *Size)
{
        FILE_NETWORK_OPEN_INFORMATION SizeInformation;
        IO_STATUS_BLOCK IoStatus;

        // Try to retrieve the file size
        if (!NT_SUCCESS(NtQueryInformationFile(File, &IoStatus,
                &SizeInformation, sizeof(SizeInformation),
                FileNetworkOpenInformation)))
                return FALSE;

        *Size = SizeInformation.EndOfFile.QuadPart;
        return TRUE;
}