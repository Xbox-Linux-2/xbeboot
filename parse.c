#include "xbox.h"
#include "xboxkrnl.h"
#include "BootString.h"
#include "BootParser.h"

#define NULL 0

NTSTATUS GetConfig(CONFIGENTRY *entry) {
	char path[BUFFERSIZE];
	char filename[BUFFERSIZE];
	char config[CONFIG_BUFFERSIZE];
	ANSI_STRING ConfigFileString;
	HANDLE ConfigFile;
	OBJECT_ATTRIBUTES ConfigFileAttributes;
	IO_STATUS_BLOCK IoStatusBlock;
	NTSTATUS Error;

	memset(path,0,sizeof(path));
	/* get the directory of the bootloader executable */
	HelpCopyUntil(path, XeImageFileName->Buffer, XeImageFileName->Length);
	HelpStrrchr(path, '\\')[1] = 0;
	/* read the config file from there */
	HelpCopyUntil(filename, path, BUFFERSIZE);
	HelpCopyUntil(HelpScan0(filename), CONFIG_FILE, BUFFERSIZE);

//	dprintf("Path: %s\n", path);
//	dprintf("Filename: %s\n", filename);

	RtlInitAnsiString(&ConfigFileString, filename);

	ConfigFileAttributes.Attributes = OBJ_CASE_INSENSITIVE;
	ConfigFileAttributes.ObjectName = &ConfigFileString;
	ConfigFileAttributes.RootDirectory = NULL;

	Error = NtCreateFile(&ConfigFile, 0x80100080 /* GENERIC_READ |
		SYNCHRONIZE | FILE_READ_ATTRIBUTES */, &ConfigFileAttributes,
		&IoStatusBlock, NULL, 0, 7 /* FILE_SHARE_READ | FILE_SHARE_WRITE |
		FILE_SHARE_DELETE*/, 1 /* FILE_OPEN */, 0x60 /* FILE_NON_DIRECTORY_FILE |
		FILE_SYNCHRONOUS_IO_NONALERT */);
	if (!NT_SUCCESS(Error)) return Error;
//	dprintf("NtCreateFile() = %08x\n", Error);
//	dprintf("HANDLE ConfigFile = %08x\n", ConfigFile);

	Error = NtReadFile(ConfigFile, NULL, NULL, NULL, &IoStatusBlock,
			config, CONFIG_BUFFERSIZE, NULL);
	if (!NT_SUCCESS(Error)) return Error;
//	dprintf("Read.\n");

	ParseConfig(path,config,entry);
//	PrintConfig(entry);

	return STATUS_SUCCESS;
}

