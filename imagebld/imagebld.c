#include <errno.h>
#include <stdio.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>

#include <string.h>

#include <stdarg.h>
#include <stdlib.h>
#include "sha1.h"
#include "xbe-header.h"
#include "../config.h"

#define debug

#define ValueDumps	0x1000

struct Checksumstruct {
	unsigned char Checksum[20];	
	unsigned int Size_ramcopy;
	unsigned int compressed_image_start;
	unsigned int compressed_image_size;
	unsigned int Biossize_type;
}Checksumstruct;

void shax(unsigned char *result, unsigned char *data, unsigned int len)
{
	struct SHA1Context context;
	SHA1Reset(&context);
	SHA1Input(&context, (unsigned char *)&len, 4);
	SHA1Input(&context, data, len);
	SHA1Result(&context,result);	
}



int xbebuild (	unsigned char * xbeimage,
		unsigned char * vmlinuzname,
		unsigned char * initrdname,
		unsigned char * configname
		)
{
	FILE *f;
	
    int a;
	unsigned char sha_Message_Digest[SHA1HashSize];

    unsigned char *xbe;
    unsigned int xbesize = 0;

    unsigned char *vmlinuz;
    unsigned int vmlinux_size = 0;
    unsigned int vmlinux_start=0;
        
    unsigned char *initrd;         
    unsigned int initrd_size = 0;
	unsigned int initrd_start = 0;

    unsigned char *config;         
    unsigned int config_size = 0;
	unsigned int config_start = 0;

	unsigned int FileSize = 0;

    unsigned int xbeloader_size=0;
	
	unsigned int temp;
		 
	XBE_HEADER *header;
 	XBE_SECTION *sechdr;

    printf("ImageBLD Hasher by XBL Project (c) hamtitampti\n");
    printf("XBEBOOT Modus\n\n");

#ifdef LOADXBE	
	f = fopen((const char *)vmlinuzname, "rb");
	if (f!=NULL) 
    	{    
 		fseek(f, 0, SEEK_END); 
         	vmlinux_size	 = ftell(f);        
         	fseek(f, 0, SEEK_SET);
    		vmlinuz = malloc(vmlinux_size);
    		memset(vmlinuz,0xff,vmlinux_size);
    		fread(vmlinuz, 1, vmlinux_size, f);
    		fclose(f);
    		printf("vmlinuz found, linking it in\n");
    	} else  {
    		printf("vmlinuz not found ----> ERROR \n");
    		return 1;
    		}
#endif
#ifdef LOADHDD_CFGFALLBACK
	f = fopen((const char *)configname, "rb");
	if (f!=NULL) 
    	{    
 		fseek(f, 0, SEEK_END); 
         	config_size	 = ftell(f);        
         	fseek(f, 0, SEEK_SET);
    		config = malloc(config_size);
    		fread(config, 1, config_size, f);
    		fclose(f);
		printf("linuxboot.cfg found, linking it in\n");
    	} else  {
    		printf("linuxboot.cfg not found ---> ERROR \n");
    		return 1;
    		}
#endif
#ifdef LOADXBE
	f = fopen((const char *)initrdname, "rb");
	if (f!=NULL) 
    	{    
 		fseek(f, 0, SEEK_END); 
         	initrd_size	 = ftell(f);        
         	fseek(f, 0, SEEK_SET);
    		initrd = malloc(initrd_size);
    		fread(initrd, 1, initrd_size, f);
   		printf("initrd found, linking it in\n");
    	} else  {
    		printf("initrd not found ---> ERROR \n");
    		return 1;
    		}
#endif    	
	f = fopen((const char *)xbeimage, "rb");
    	if (f!=NULL) 
    	{   
  		fseek(f, 0, SEEK_END); 
         	xbesize	 = ftell(f); 
		FileSize = xbesize;
         	fseek(f, 0, SEEK_SET);
           	  
           	
//           	xbe = malloc(xbesize+vmlinux_size+1024*1024);
           	xbe = malloc(xbesize+vmlinux_size+3*1024*1024+initrd_size+config_size);
           	    		
    		memset(xbe,0x00,sizeof(xbesize+vmlinux_size+3*1024*1024+initrd_size+config_size));
    		fread(xbe, 1, xbesize, f);
    		fclose(f);
	      
	      	//printf("xxx:     : 0x%08X\n", (unsigned int)xbesize);
	        
	       	// We make some Allignment
	       	xbesize = (xbesize & 0xffffff00) + 0x100;
	        	      
	        //vmlinux_size = (vmlinux_size & 0xfffff800 ) + 0x400;
	        
#ifdef LOADXBE
	        vmlinux_start = xbesize;
	        memcpy(&xbe[ValueDumps + 0x80],&vmlinux_start,4);
		memcpy(&xbe[ValueDumps + 0x84],&vmlinux_size,4);

	        memcpy(&xbe[vmlinux_start],vmlinuz,vmlinux_size);
	        		
		// We tell the XBEBOOT loader, that the Paramter he should pass to the Kernel = 2MB for the Size
		
		//temp= 2*1024*1024;
		
		temp = vmlinux_size;
		temp = (temp & 0xffff0000) + 0xffff + 0xffff;
		memcpy(&xbe[ValueDumps + 0x88],&temp,4);		
		
		xbesize = xbesize + vmlinux_size;
		FileSize += vmlinux_size;
		// Ok, we allign again
		xbesize = (xbesize & 0xffffff00) + 0x100;

#endif
		
#ifdef LOADXBE		
		initrd_start = xbesize;
		memcpy(&xbe[ValueDumps + 0x8C],&initrd_start,4);
		memcpy(&xbe[ValueDumps + 0x90],&initrd_size,4);
		
		memcpy(&xbe[initrd_start],initrd,initrd_size);
		
		xbesize = xbesize + initrd_size;	
		FileSize += initrd_size;
        xbesize = (xbesize & 0xffffff00) + 0x100;
#endif                

#ifdef LOADHDD_CFGFALLBACK
        config_start = xbesize;
		memcpy(&xbe[ValueDumps + 0x94],&config_start,4);
		memcpy(&xbe[ValueDumps + 0x98],&config_size,4);               	
		
		memcpy(&xbe[config_start],config,config_size);               	

		xbesize = xbesize + config_size;	
		FileSize += config_size;
                xbesize = (xbesize & 0xffffff00) + 0x100;
#endif
                			        
		#ifdef debug
	 	printf("Linking Section\n");
	 	#ifdef LOADXBE	
	 	printf("Start of Linux Kernel    : 0x%08X\n", vmlinux_start);
	 	printf("Size of Linux Kernel     : 0x%08X\n", vmlinux_size);
		printf("Start of InitRD          : 0x%08X\n", initrd_start);
	 	printf("Size of Initrd           : 0x%08X\n", initrd_size);
		#endif		
		#ifdef LOADHDD_CFGFALLBACK
		printf("Start of Config          : 0x%08X\n", config_start);
	 	printf("Size of config           : 0x%08X\n", config_size);
	 	#endif
		printf("----------------\n");
		#endif	      

		header = (XBE_HEADER*) xbe;
	    
		// We calculate a new Size of the overall XBE, we allign too		
		
		xbeloader_size = xbesize - 0x1000;
		
		xbesize = (xbesize & 0xffffff00) + 0x100;
		
	    header->ImageSize = FileSize; 
		
		//printf("%08x",sechdr->FileSize);                    
		
	        
	  	#ifdef debug
	 	printf("Size of all headers:     : 0x%08X\n", (unsigned int)header->HeaderSize);
        printf("Size of entire image     : 0x%08X\n", (unsigned int)header->ImageSize);
		#endif

		// This selects the first section, we only have one
		sechdr = (XBE_SECTION *)(((char *)xbe) + (int)header->Sections - (int)header->BaseAddress);
		#ifdef debug
		printf("Location of section header: %p\n", (void *)&sechdr);
		#endif

		sechdr->FileSize = xbeloader_size;
		sechdr->VirtualSize = xbeloader_size;
			        
        shax(&sha_Message_Digest[0], ((unsigned char *)xbe)+(int)sechdr->FileAddress ,sechdr->FileSize);
	  	memcpy(&sechdr->ShaHash[0],&sha_Message_Digest[0],20);
	  	
	  	#ifdef debug
		
		printf("S0: Virtual address      : 0x%08X\n", (unsigned int)sechdr->VirtualAddress);
        printf("S0: Virtual size         : 0x%08X\n", (unsigned int)sechdr->VirtualSize);
        printf("S0: File address         : 0x%08X\n", (unsigned int)sechdr->FileAddress);
        printf("S0: File size            : 0x%08X\n", (unsigned int)sechdr->FileSize);

		printf("Section 0 Hash XBE       : ");
		for(a=0; a<SHA1HashSize; a++) {
			printf("%02x",sha_Message_Digest[a]);
		}
	      	printf("\n");
	      	#endif




	      	
		// Write back the Image to Disk
		f = fopen((const char *)xbeimage, "wb");
    		if (f!=NULL) 
    		{   
		 fwrite(xbe, 1, xbesize, f);
        	 fclose(f);			
		}	  	
	        
	        printf("\nXbeboot.xbe Created    : %s\n",xbeimage);
	      	
		#ifdef LOADXBE
		free(initrd);         
		#endif
        	
        	#ifdef LOADHDD_CFGFALLBACK
        	free(config);   
        	#endif
           	
           	#ifdef LOADXBE	
                free(vmlinuz);  
           	#endif
                free(xbe);
              
	} else return 1;


        
	return 0;	
}


int xbeextract (	unsigned char * xbeimage ) 
{
	FILE *f;

        unsigned char *xbe;
        unsigned int xbesize = 0;
    	
    	unsigned char *vmlinuz;
    	unsigned int vmlinux_size = 0;
        unsigned int vmlinux_start=0;
        
        unsigned char *initrd;         
        unsigned int initrd_size = 0;
	unsigned int initrd_start = 0;

        unsigned char *config;         
        unsigned int config_size = 0;
	unsigned int config_start = 0;



	f = fopen((const char *)xbeimage, "rb");
    	if (f!=NULL) 
    	{   
  		fseek(f, 0, SEEK_END); 
         	xbesize	 = ftell(f); 
         	fseek(f, 0, SEEK_SET);
           	xbe = malloc(xbesize);
    		fread(xbe, 1, xbesize, f);
    		fclose(f);

		memcpy(&initrd_start, &xbe[ValueDumps + 0x8C],4);
		memcpy(&initrd_size,  &xbe[ValueDumps + 0x90],4);
	    memcpy(&vmlinux_start,&xbe[ValueDumps + 0x80],4);
		memcpy(&vmlinux_size, &xbe[ValueDumps + 0x84],4);
		memcpy(&config_start, &xbe[ValueDumps + 0x94],4);
		memcpy(&config_size,  &xbe[ValueDumps + 0x98],4);         		

	 	printf("Linked Sections\n");
	 	printf("Start of Linux Kernel    : 0x%08X\n", vmlinux_start);
	 	printf("Size of Linux Kernel     : 0x%08X\n", vmlinux_size);
		printf("Start of InitRD          : 0x%08X\n", initrd_start);
	 	printf("Size of Initrd           : 0x%08X\n", initrd_size);
		printf("Start of Config          : 0x%08X\n", config_start);
	 	printf("Size of config           : 0x%08X\n", config_size);
		printf("----------------\n");


		printf("Extracting Kernel");
		f = fopen("kernel", "wb");
    		if (f!=NULL) 
    		{   
		 fwrite(&xbe[vmlinux_start], 1, vmlinux_size, f);
        	 fclose(f);			
		}	  
		printf(" .. Done \n");


		printf("Extracting Ramdisk");
		f = fopen("ramdisk", "wb");
    		if (f!=NULL) 
    		{   
		 fwrite(&xbe[initrd_start], 1, initrd_size, f);
        	 fclose(f);			
		}	  
		printf(" .. Done \n");

		printf("Extracting Config");
		f = fopen("config.cfg", "wb");
    		if (f!=NULL) 
    		{   
		 fwrite(&xbe[config_start], 1, config_size, f);
        	 fclose(f);			
		}	  
		printf(" .. Done \n");
    		
    		free(xbe);
    	}        
	return 0;	
}

int main (int argc, const char * argv[])
{
	int error=0;
	
	if (strcmp(argv[1],"-build")==0) {
	error = xbebuild((unsigned char*)argv[2],(unsigned char*)argv[3],(unsigned char*)argv[4],(unsigned char*)argv[5]);
	}

	if (strcmp(argv[1],"-extract")==0) {
	error = xbeextract((unsigned char*)argv[2]);
	}

	return error;	
}
