/*
 * video-related stuff
 * 2003-02-02  andy@warmcat.com  Major reshuffle, threw out tons of unnecessary init
                                 Made a good start on separating the video mode from the AV cable
																 Consolidated init tables into a big struct (see boot.h)
 * 2002-12-04  andy@warmcat.com  Video now working :-)
 */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "boot.h"

void IoOutputByte(WORD wAds, BYTE bValue);
void BootPciInterruptEnable(void);
int I2CTransmitWord(BYTE bPicAddressI2cFormat, WORD wDataToWrite);
int I2CTransmitByteGetReturn(BYTE bPicAddressI2cFormat, BYTE bDataToWrite);
#include "BootVideo.h"

const BYTE baGraInit[] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x40, 0x05, 0x0f, 0xff };
const BYTE baSequencerInit[] = { 0x03, 0x21, 0x0f, 0x00, 0x06 };
const BYTE baAttrInit[] = {
	0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
	0x08, 0x09, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x0f,
	0x01, 0x4a, 0x0f, 0x00, 0x00
 };


#define NVCRTC 0x6013D4

// definition of typedef'd struct in boot.h
// this struct defines the different register contents per video encoding, per mode
// the data here should be independent of the AV cable in use
// (unfortunately the conexant tables fully reprogram the device including AV right now)

const VIDEO_MODE_TABLES videomodetables = {

	{ // DWORD m_dwAddressesNv[0x3c]; -- these are the registers updated by the data in dwaVideoModeNv
			0x680800, 0x680804, 0x680808, 0x68080c,
			0x680810, 0x680814, 0x680818, 0x68081c,
			0x680820, 0x680824, 0x680828, 0x68082c,
			0x680830, 0x680834, 0x680838, 0x68083c,
			0x680840, 0x680844, 0x680848, 0x68084c,
			0x680880, 0x680884, 0x680888, 0x68088c,
			0x680890, 0x680894, 0x680898, 0x68089c,

			0x680324, 0x680328, 0x68032c, 0x680330,
			0x680504, 0x680508, 0x680518, 0x680520,
			0x680544, 0x680548, 0x680558, 0x680560,
			0x680584, 0x680588, 0x680598, 0x6805a0,
			0x6805c4, 0x6805c8, 0x6805d8, 0x6805e0,
			0x680604, 0x68060c, 0x680624, 0x680840,

			0x6808C0, 0x6808C4, 0x680630,	0x680680,
			0x680684, 0x680688,	0x68068C, 0x680690,
	},

	{	// DWORD dwaVideoModeNv[TV_ENCODING_COUNT][VIDEO_MODE_COUNT][0x3c];

		{ // PAL VIDEO ENCODING

			{ // 640x480
				0x1df, 0x270, 0x1df, 0x218,
				0x21b, 0x0, 0x1df, 0x0,
				0x27f, 0x3af, 0x257, 0x29e,
				0x2be, 0, 0x27f, 0,
				0x1dbcad, 0, 0x10100111, 0x00801080,
					//+880->
				0x21101100, 0x0, 0x0, 0x10001000,
				0x10000000, 0x10000000, 0x10000000, 0x10000000,
				0x8b363bd7, 0xbcb86d9d, 0xcb0f6aec, 0xeb39e7a5, 0x00007702, 0x0003C20D, 0x0014180b, 0x0003C20D,
				0x00007702, 0x0003C20D, 0x00301834, 0x0003C20D, 0x00007702, 0x0003C20D, 0x00301834, 0x0003C20D,
				0x00007702, 0x0003C20D, 0x00301834, 0x0003C20D, 0xe0700105, 0x007fffad, 0x2ddf77ed, 0x001Dd414,

				0x0, 0x40801080, 0x2, 0x10E62C, 0x271, 0x0B1,	0x2C4, 0x0,

			}, { // 640x576
				0x23f, 0x270, 0x23f, 0x249,
				0x24c, 0x0, 0x23f, 0x0,
				0x27f, 0x313, 0x257, 0x28f,
				0x2af, 0, 0x27f, 0,
				0xffffff, 0, 0x10100111, 0x00801080,
					//+880->
				0x21101100, 0x0, 0x0, 0x10001000,
				0x10000000, 0x10000000, 0x10000000, 0x10000000,
				0x8b363bd7, 0xbcb86d9d, 0xcb0f6aec, 0xeb39e7a5, 0x00007702, 0x0003C20D, 0x0014180b, 0x0003C20D,
				0x00007702, 0x0003C20D, 0x00301834, 0x0003C20D, 0x00007702, 0x0003C20D, 0x00301834, 0x0003C20D,
				0x00007702, 0x0003C20D, 0x00301834, 0x0003C20D, 0xe0700105, 0x007fffad, 0x2ddf77ed, 0x001Dd414,

				0x0, 0x40801080, 0x2, 0x35A,    0x1,   0x0AB,	0x2AE, 0x1,

			}, { // 720x576
				0x23F, 0x270, 0x23F, 0x248, 0x24B, 0x00, 0x23F, 0x00,
				0x2CF, 0x377, 0x2A7, 0x2DD, 0x2FD, 0x00, 0x2CF, 0x00,
				0xFFFFFF, 0x00, 0x10100111, 0x801080,
					//+880->
				0x21101100, 0x00, 0x00, 0x10001000, 0x10000000, 0x10000000, 0x10000000, 0x10000000,
				0x8b363bd7, 0xbcb86d9d, 0xcb0f6aec, 0xeb39e7a5, 0x00007702, 0x0003C20D, 0x0014180b, 0x0003C20D,
				0x00007702, 0x0003C20D, 0x00301834, 0x0003C20D, 0x00007702, 0x0003C20D, 0x00301834, 0x0003C20D,
				0x00007702, 0x0003C20D, 0x00301834, 0x0003C20D, 0xe0700105, 0x007fffad, 0x2ddf77ed, 0x001Dd414,

				0x0, 0x40801080, 0x2, 0x35A,    0x1,   0x0AB,	0x2AE, 0x1,

			}, { // 800x600
				0x257, 0x2B1, 0x257, 0x273, 0x276, 0x00, 0x257, 0x00,
				0x31F, 0x3E7, 0x2F7, 0x331, 0x351, 0x00, 0x31F, 0x00,
				0xFFFFFF, 0x00, 0x10100111, 0x801080, // skip 12 DWORDS
				0x21101100, 0x00, 0x00, 0x10001000, 0x10000000, 0x10000000, 0x10000000, 0x10000000,
					// 600 stuff
				0x19EBA4DE, 0xCA40F23, 0x71BAD3DB, 0xA89B8064,
				0x00007702, 0x0003C20D, 0x220C03, 0x0003C20D,
				0x00007702, 0x0003C20D, 0x371629, 0x0003C20D,
				0x00007702, 0x0003C20D, 0x371629, 0x0003C20D,
				0x00007702, 0x0003C20D, 0x371629, 0x0003C20D,
				0x7CF80417, 0x1EE49, 0x1E57769C, 0x00ffffff,

				0x0, 0x40801080, 0x2, 0x30a, 0x01, 0xab,	0x2ae, 0x01
			}

		},
		{ // NTSC Video Encoding
			{ // 640x480 NTSC
				0x1df, 0x257, 0x1df, 0x20e,
				0x211, 0x0, 0x1df, 0x0,
				0x27f, 0x30f, 0x257, 0x28a,
				0x2aa, 0x0, 0x27f, 0x0,
				0x1dd414, 0, 0x10100111, 0x00801080,

				0x21101100, 0,0, 0x10001000,
				0x10000000, 0x10000000, 0x10000000, 0x10000000,
				0xffffffff, 0xF3FC68C7, 0x692DA0D6, 0xCEA52C1F, 0x00007702, 0x0003C20D, 0x00323232, 0x0003C20D,
				0x00007702, 0x0003C20D, 0x00333333, 0x0003C20D, 0x00007702, 0x0003C20D, 0x00333333, 0x0003C20D,
				0x00007702, 0x0003C20D, 0x00333333, 0x0003C20D, 0xFFFF0134, 0x007CEBFD, 0x3FFFFFFF, 0x001DBCAD,

				0x0, 0x40801080, 0x2,	0x30A,	  0x1,   0x0AB, 0x2AE, 0x1,

			}, { // 640x576 not possible in NTSC

				0,0,0,0,
				0,0,0,0,
				0,0,0,0,
				0,0,0,0,
				0,0,0,0,
				0,0,0,0,
				0,0,0,0,
				0,0,0,0,
				0,0,0,0,
				0,0,0,0,
				0,0,0,0,
				0,0,0,0,
				0,0,0,0,
				0,0,0,0,
				0,0,0,0
			}, { // 720x576 not done in NTSC

				0,0,0,0,
				0,0,0,0,
				0,0,0,0,
				0,0,0,0,
				0,0,0,0,
				0,0,0,0,
				0,0,0,0,
				0,0,0,0,
				0,0,0,0,
				0,0,0,0,
				0,0,0,0,
				0,0,0,0,
				0,0,0,0,
				0,0,0,0,
				0,0,0,0
			}, { // 800 x 600  not done yet in NTSC

				0,0,0,0,
				0,0,0,0,
				0,0,0,0,
				0,0,0,0,
				0,0,0,0,
				0,0,0,0,
				0,0,0,0,
				0,0,0,0,
				0,0,0,0,
				0,0,0,0,
				0,0,0,0,
				0,0,0,0,
				0,0,0,0,
				0,0,0,0,
				0,0,0,0
			}
		}
	},
	{ // BYTE m_baVideoModeCrtc[TV_ENCODING_COUNT][VIDEO_MODE_COUNT][0x3F];

		{ // PAL CRTC

			{ // 640x480
				0x6F, 0x4F, 0x4F, 0x93, 0x55, 0xB9, 0x0B, 0x3E, 0x00, 0x40, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00,
				0xE8, /**/  0xDF, 0x40, 0x00, 0xDF, 0x0C, 0xE3, 0xFF, 0x3B, 0x3A, 0x85, 0x00, 0x00, 0x00, /**/
				0x80, 0xFF, 0xFF, 0xA1, 0x00, 0x10, 0x14, 0xA3, 0x83, 0x00, 0x00, 0xFF, 0xFD, 0xE0, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x11, 0x02, 0x02, 0x02, 0x30, 0x00, 0xFF, 0xFF, 0xFF, 0x7F, 0x00, 0x23, 0x30,
				0x00
			},
			{ // 640x576
				0x56, 0x4F, 0x4F, 0x9c, 0x51, 0x35, 0x6f, 0xf0, 0x00, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x45, /**/  0x3f, 0x40, 0x00, 0x3F, 0x70, 0xE3, 0xFF, 0x30, 0x3A, 0x85, 0x00, 0x00, 0x00, /**/
				0x80, 0xFF, 0xFF, 0xA1, 0x00, 0x10, 0x20, 0xA3, 0x83, 0x00, 0x00, 0xFF, 0xFf, 0xE0, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x11, 0x02, 0x02, 0x22, 0x30, 0x00, 0xFF, 0xFF, 0xdF, 0x7F, 0x00, 0x20, 0x30,
				0x00
			},
			{ // 720x576
				0x64, 0x59, 0x59, 0x88, 0x5E, 0x89, 0x6F, 0xF0, 0x00, 0x60, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x42, /*0x24,*/ 0x3F, 0x68, 0x00, 0x3F, 0x70, 0xE3, 0xFF, 0x30, 0x2C, 0x85, 0x00, 0x00, 0x00, /*0x03,*/
				0x41, 0xFF, 0xEF, 0xA1, 0x00, 0x10, 0x20, 0xA3, 0x83, 0x00, 0x00, 0xFF, 0xFF, 0xE0, 0x00, 0x03,
				0xFF, 0x00, 0x00, 0x11, 0x02, 0x02, 0x23, 0x30, 0x00, 0xFF, 0xFF, 0xDF, 0x7F, 0x00, 0x20, 0x30,
				0x00
			},
			{ // 800x600 MEDIUM
				0x72 ,0x63 ,0x63 ,0x96 ,0x69 ,0x8C ,0xAD ,0xF0, 0x00 ,0x60 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00,
				0x70 ,/*0x22,*/ 0x57 ,0x90 ,0x00 ,0x57 ,0xAE ,0xE3, 0xFF ,0x30 ,0x2C ,0x85 ,0x00 ,0x00 ,0x00 , /*0x03, */
				0x41 ,0xFF ,0xEF ,0xA1 ,0x00 ,0x10 ,0x20 ,0xA3, 0x83 ,0x00 ,0x00 ,0xFF ,0xFF ,0xE0 ,0x00 ,0x03,
				0xFF ,0x00 ,0x00 ,0x11 ,0x02 ,0x02 ,0x22 ,0x30,	0x00 ,0xFF ,0xFF ,0xDF ,0x7F ,0x00 ,0x20 ,0x30,
				0x00
			}

		},

		{ // NTSC CRTC

			{// 640x480
				0x56, 0x4F, 0x4F, 0x9c, 0x51, 0x35, 0x0b, 0x3e, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0xeb, /**/  0xdf, 0x40, 0x00, 0xdF, 0x0c, 0xE3, 0xFF, 0x30, 0x3A, 0x85, 0x00, 0x00, 0x00, /**/
				0x29, 0xFe, 0xFd, 0xA1, 0x80, 0x10, 0x15, 0xA3, 0x83, 0x00, 0x00, 0xFe, 0xF3, 0xE0, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x11, 0x02, 0x02, 0x00, 0x30, 0x00, 0xFF, 0x58, 0xFe, 0xeb, 0x00, 0x00, 0x31,
				0x00
			},
			{// 640x576 Doesn't exist in NTSC
				0x56, 0x4F, 0x4F, 0x9c, 0x51, 0x35, 0x0b, 0x3e, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0xeb, /**/  0xdf, 0x40, 0x00, 0xdF, 0x0c, 0xE3, 0xFF, 0x30, 0x3A, 0x85, 0x00, 0x00, 0x00, /**/
				0x29, 0xFe, 0xFd, 0xA1, 0x80, 0x10, 0x15, 0xA3, 0x83, 0x00, 0x00, 0xFe, 0xF3, 0xE0, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x11, 0x02, 0x02, 0x00, 0x30, 0x00, 0xFF, 0x58, 0xFe, 0xeb, 0x00, 0x00, 0x31,
				0x00
			},
			{// 720x576 Doesn't exist in NTSC
				0x56, 0x4F, 0x4F, 0x9c, 0x51, 0x35, 0x0b, 0x3e, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0xeb, /**/  0xdf, 0x40, 0x00, 0xdF, 0x0c, 0xE3, 0xFF, 0x30, 0x3A, 0x85, 0x00, 0x00, 0x00, /**/
				0x29, 0xFe, 0xFd, 0xA1, 0x80, 0x10, 0x15, 0xA3, 0x83, 0x00, 0x00, 0xFe, 0xF3, 0xE0, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x11, 0x02, 0x02, 0x00, 0x30, 0x00, 0xFF, 0x58, 0xFe, 0xeb, 0x00, 0x00, 0x31,
				0x00
			},
			{// 800x600 NOT DONE YET
				0x56, 0x4F, 0x4F, 0x9c, 0x51, 0x35, 0x0b, 0x3e, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0xeb, /**/  0xdf, 0x40, 0x00, 0xdF, 0x0c, 0xE3, 0xFF, 0x30, 0x3A, 0x85, 0x00, 0x00, 0x00, /**/
				0x29, 0xFe, 0xFd, 0xA1, 0x80, 0x10, 0x15, 0xA3, 0x83, 0x00, 0x00, 0xFe, 0xF3, 0xE0, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x11, 0x02, 0x02, 0x00, 0x30, 0x00, 0xFF, 0x58, 0xFe, 0xeb, 0x00, 0x00, 0x31,
				0x00
			}

		}

	},
	{ // BYTE m_baVideoModeConexant[TV_ENCODING_COUNT][VIDEO_MODE_COUNT][0x80];

		{ // PAL Conexant

			{  // 640 x 480
				0x00,
				0x00 , 0x28 , 0x80 , 0xE4 , 0x00 , 0x00 , 0x80 , 0x80,
				0x80 , 0x13 , 0xDA , 0x4B , 0x28 , 0xA3 , 0x9F , 0x25,
				0xA3 , 0x9F , 0x25 , 0x00 , 0x00 , 0x00 , 0x00 , 0x44,
				0xC7 , 0x00 , 0x00 , 0x41 , 0x35 , 0x03 , 0x46 , 0x00,
				0x02, 0x00, 0x01, 0x60, 0x80, 0x8a, 0xa6, 0x68,

				0xc1, 0x2e, 0xf2, 0x27, 0x00, 0xb0, 0x0a, 0x0b,
				0x71, 0x5a, 0xe0, 0x36, 0x00, 0x50, 0x72, 0x1c,
				0x0d, 0x24, 0xf0, 0x58, 0x81, 0x49,   0x8c,   0x0c,  // <--- A8==81, AA==49, AC == 8C normally
				0x8c, 0x79, 0x26, 0x52, 0x00, 0x24, 0x00, 0x00,
				0x00 , 0x00 , 0x01 , 0x9C , 0x9B , 0xC0 , 0xC0 , 0x19,
				0x00 , 0x00 , 0x00 , 0x00 , 0x40 , 0x05 , 0x57 , 0x20,
				0x40 , 0x6E , 0x7E , 0xF4 , 0x51 , 0x0F , 0xF1 , 0x05,
				0xD3 , 0x78 , 0xA2 , 0x25 , 0x54 , 0xA5 , 0x00 , 0x00

			}, { // 640x576

				 0x00,
				0x00 , 0x28 , 0x80 , 0xd2 , 0x00 , 0x00 , 0x80 , 0x80,
				0x80 , 0x13 , 0xDA , 0x4B , 0x28 , 0xA3 , 0x9F , 0x25,
				0xA3 , 0x9F , 0x25 , 0x00 , 0x00 , 0x00 , 0x00 , 0x44,
				0xC7 , 0x00 , 0x00 , 0xad , 0x3f , 0x03 , 0x46 , 0x00,
				0x0f, 0x00, 0x01, 0x28, 0x88, 0x74, 0x8a, 0x42,
				0x0a, 0x16, 0x22, 0xa6, 0x00, 0x14, 0x7e, 0x03,
				0x71, 0x2a, 0x40, 0x0a, 0x00, 0x50, 0xc7, 0xf1,
				0x0a, 0x24, 0xf0, 0x5e, 0x81, 0x49,   0x8c,   0x18,  // <--- A8==81, AA==49, AC == 8C normally
				0x75, 0x17, 0x2e, 0x00, 0x00, 0x24, 0x00, 0x00,
				0x00 , 0x00 , 0x01 , 0x9C , 0x9B , 0xC0 , 0xC0 , 0x19,
				0x00 , 0x00 , 0x00 , 0x00 , 0x40 , 0x05 , 0x57 , 0x20,
				0x40 , 0x6E , 0x7E , 0xF4 , 0x51 , 0x0F , 0xF1 , 0x05,
				0xD3 , 0x78 , 0xA2 , 0x25 , 0x54 , 0xA5 , 0x00 , 0x00

			}, { // 720x576

				 0x00,
				0x00, 0x28, 0x80, 0xD2, 0x00, 0x00, 0x80, 0x80,
				0x80, 0x13, 0xDA, 0x4B, 0x28, 0xA3, 0x9F, 0x25,
				0xA3, 0x9F, 0x25, 0x00, 0x00, 0x00, 0x00, 0x44,
				0xC7, 0x00, 0x00, 0xAD, 0x3F, 0x03, 0x46, 0x04,
				0x02, 0x00, 0x01, 0xF0, 0xD0, 0x82, 0x9C, 0x5A,
				0x31, 0x16, 0x22, 0xA6, 0x00, 0x78, 0x93, 0x03,
				0x71, 0x2A, 0x40, 0x3A, 0x00, 0x50, 0x55, 0x55,
				0x0C, 0x24, 0xF0, 0x59, 0x82, 0x49, 0x8C, 0x8E,
				0xB0, 0xE6, 0x28, 0x00, 0x00, 0x24, 0x00, 0x00,
				0x00, 0x00, 0x01, 0x9C, 0x9B, 0xC0, 0xC0, 0x19,
				0x00, 0x00, 0x00, 0x00, 0x40, 0x05, 0x57, 0x20,
				0x40, 0x6E, 0x7E, 0xF4, 0x51, 0x0F, 0xF1, 0x05,
				0xD3, 0x78, 0xA2, 0x25, 0x54, 0xA5, 0x00, 0x00
			}, { // 800x600 PAL overscan-corrected MEDIUM type 9
				0x00,
				0x00, 0x28, 0x80, 0xD2, 0x00, 0x00, 0x80, 0x80,
				0x80, 0x13, 0xDA, 0x4B, 0x28, 0xA3, 0x9F, 0x25,
				0xA3, 0x9F, 0x25, 0x00, 0x00, 0x00, 0x00, 0x44,
				0xC7, 0x00, 0x00, 0xAD, 0x3F, 0x03, 0x46, 0x00,
				0x02, 0x00, 0x01, 0xA0, 0x20, 0xA2, 0xC2, 0x8E,
				0xD7, 0x1E, 0x12, 0xB8, 0x00, 0xE8, 0xAF, 0x03,
				0xB2, 0x40, 0x58, 0x3A, 0x54, 0x53, 0x55, 0x55,
				0x0F, 0x24, 0xF0, 0x57, 0x80, 0x48, 0x8C, 0x94,
				0x13, 0xE6, 0x20, 0x00, 0x00, 0x24, 0x00, 0x00,
				0x00, 0x00, 0x01, 0x9C, 0x1B, 0xC0, 0xC0, 0x19,  // b7 of C8 set low for luma LPF on Composite to stop colour fringing of HF luma
				0x00, 0x00, 0x00, 0x00, 0x40, 0x05, 0x57, 0x20,
				0x40, 0x6E, 0x7E, 0xF4, 0x51, 0x0F, 0xF1, 0x05,
				0xD3, 0x78, 0xA2, 0x25, 0x54, 0xA5, 0x00, 0x00
			}
		},

		{ // NTSC Conexant

			{ // 640x480 NTSC overscan Small
				0x00,
				0x00 , 0x28 , 0x80 , 0xA4 , 0x00 , 0x00 , 0x80 , 0x80,
				0x80 , 0x13 , 0xDA , 0x4B , 0x28 , 0xA3 , 0x9F , 0x25,
				0xA3 , 0x9F , 0x25 , 0x00 , 0x00 , 0xFF , 0x01 , 0x44,
				0xC0 , 0x00 , 0x18 , 0x15 , 0x4B , 0x00 , 0x46 , 0x00,
				0x02, 0x00, 0x01, 0x00, 0x80, 0x84, 0x96, 0x60,
				0x7D, 0x22, 0xd4, 0x27, 0x00, 0x10, 0x7e, 0x03,
				0x58, 0x4b, 0xE0, 0xa6, 0x92, 0x54, 0x0e, 0x88,
				0x0c, 0x0A, 0xE5, 0x76, 0x79, 0x44, 0x85, 0x00,
				0x00, 0x80, 0x20, 0x5A, 0x00, 0x24, 0x00, 0x00,
				0x00 , 0x00 , 0x01 , 0x9C , 0x80 , 0xC0 , 0xC0 , 0x19,
				0x00 , 0x00 , 0x00 , 0x00 , 0x40 , 0x0F , 0xFC , 0x20,
				0xD0 , 0x6F , 0x0F , 0x00 , 0x00 , 0x0C , 0xF3 , 0x09,
				0xBD , 0x66 , 0xB5 , 0x90 , 0xB2 , 0x7D , 0x00 , 0x00
			}, { // 640x576 does not exist in NTSC
				0x00,
				0x00 , 0x28 , 0x80 , 0xA4 , 0x00 , 0x00 , 0x80 , 0x80,
				0x80 , 0x13 , 0xDA , 0x4B , 0x28 , 0xA3 , 0x9F , 0x25,
				0xA3 , 0x9F , 0x25 , 0x00 , 0x00 , 0xFF , 0x01 , 0x44,
				0xC0 , 0x00 , 0x18 , 0x15 , 0x4B , 0x00 , 0x46 , 0x00,
				0x02, 0x00, 0x01, 0x00, 0x80, 0x84, 0x96, 0x60,
				0x7D, 0x22, 0xd4, 0x27, 0x00, 0x10, 0x7e, 0x03,
				0x58, 0x4b, 0xE0, 0xa6, 0x92, 0x54, 0x0e, 0x88,
				0x0c, 0x0A, 0xE5, 0x76, 0x79, 0x44, 0x85, 0x00,
				0x00, 0x80, 0x20, 0x5A, 0x00, 0x24, 0x00, 0x00,
				0x00 , 0x00 , 0x01 , 0x9C , 0x80 , 0xC0 , 0xC0 , 0x19,
				0x00 , 0x00 , 0x00 , 0x00 , 0x40 , 0x0F , 0xFC , 0x20,
				0xD0 , 0x6F , 0x0F , 0x00 , 0x00 , 0x0C , 0xF3 , 0x09,
				0xBD , 0x66 , 0xB5 , 0x90 , 0xB2 , 0x7D , 0x00 , 0x00
			}, { // 720x576 does not exist in NTSC
				0x00,
				0x00 , 0x28 , 0x80 , 0xA4 , 0x00 , 0x00 , 0x80 , 0x80,
				0x80 , 0x13 , 0xDA , 0x4B , 0x28 , 0xA3 , 0x9F , 0x25,
				0xA3 , 0x9F , 0x25 , 0x00 , 0x00 , 0xFF , 0x01 , 0x44,
				0xC0 , 0x00 , 0x18 , 0x15 , 0x4B , 0x00 , 0x46 , 0x00,
				0x02, 0x00, 0x01, 0x00, 0x80, 0x84, 0x96, 0x60,
				0x7D, 0x22, 0xd4, 0x27, 0x00, 0x10, 0x7e, 0x03,
				0x58, 0x4b, 0xE0, 0xa6, 0x92, 0x54, 0x0e, 0x88,
				0x0c, 0x0A, 0xE5, 0x76, 0x79, 0x44, 0x85, 0x00,
				0x00, 0x80, 0x20, 0x5A, 0x00, 0x24, 0x00, 0x00,
				0x00 , 0x00 , 0x01 , 0x9C , 0x80 , 0xC0 , 0xC0 , 0x19,
				0x00 , 0x00 , 0x00 , 0x00 , 0x40 , 0x0F , 0xFC , 0x20,
				0xD0 , 0x6F , 0x0F , 0x00 , 0x00 , 0x0C , 0xF3 , 0x09,
				0xBD , 0x66 , 0xB5 , 0x90 , 0xB2 , 0x7D , 0x00 , 0x00
			}, { // 800x600 not done yet in NTSC
				0x00,
				0x00, 0x28, 0x80, 0xD2, 0x00, 0x00, 0x80, 0x80,
				0x80, 0x13, 0xDA, 0x4B, 0x28, 0xA3, 0x9F, 0x25,
				0xA3, 0x9F, 0x25, 0x00, 0x00, 0x00, 0x00, 0x44,
				0xC7, 0x00, 0x00, 0xAD, 0x3F, 0x03, 0x46, 0x00,
				0x02, 0x00, 0x01, 0xA0, 0x20, 0xA2, 0xC2, 0x8E,
				0xD7, 0x1E, 0x12, 0xB8, 0x00, 0xE8, 0xAF, 0x03,
				0xB2, 0x40, 0x58, 0x3A, 0x54, 0x53, 0x55, 0x55,
				0x0F, 0x24, 0xF0, 0x57, 0x80, 0x48, 0x8C, 0x94,
				0x13, 0xE6, 0x20, 0x00, 0x00, 0x24, 0x00, 0x00,
				0x00, 0x00, 0x01, 0x9C, 0x1B, 0xC0, 0xC0, 0x19,  // b7 of C8 set low for luma LPF on Composite to stop colour fringing of HF luma
				0x00, 0x00, 0x00, 0x00, 0x40, 0x05, 0x57, 0x20,
				0x40, 0x6E, 0x7E, 0xF4, 0x51, 0x0F, 0xF1, 0x05,
				0xD3, 0x78, 0xA2, 0x25, 0x54, 0xA5, 0x00, 0x00
			}
		}
	}
};





// memory mapped IO


#define voutb(nAds, b) {	*((BYTE* volatile)((nAds))) = (b); }
#define voutw(nAds, w) {	*(( WORD* volatile)((nAds))) = (w); }
#define voutl(nAds, dw) {	*((DWORD* volatile)((nAds))) = (dw); }

#define vinb(nAds) (*(( BYTE* volatile)((nAds))))
#define vinl(nAds) (*(( DWORD* volatile)((nAds))))

void vgaout(BYTE * volatile pb, unsigned char reg, unsigned char data) {
	*pb++=reg;
	*pb=data;
}




// call with first arg video mode from enum in boot.h (eg, VIDEO_MODE_800x600)
// second arg forces the encoder to issule black raster initially when true
// false allows the video through the encoder normally.  This is used with false normally,
// but true is used on powerup so an interrupt routine can fade in the video
// this routine takes care of mapping to the correct video standard; if the requested mode
// is not available in the current video standard

void BootVgaInitializationKernel(CURRENT_VIDEO_MODE_DETAILS * pcurrentvidemodedetails)
{
	int arg_8=1, arg_C=0x43;
	DWORD dwStash=0;
	DWORD dwTempIntState;
	BYTE b;

	voutl(pcurrentvidemodedetails->m_pbBaseAddressVideo+0x600800, 0x03c00000);   // set video start address to 4M back from end of RAM

	pcurrentvidemodedetails->m_bAvPack=I2CTransmitByteGetReturn(0x10, 0x04);  // the PIC knows the AV pack type
	b=I2CTransmitByteGetReturn(0x54, 0x5A); // the eeprom defines the TV standard for the box

//	b=0x40;  // uncomment to force NTSC

	if(b != 0x40) {
		pcurrentvidemodedetails->m_bTvStandard = TV_ENCODING_PAL;
	} else {
		pcurrentvidemodedetails->m_bTvStandard = TV_ENCODING_NTSC;
	}

		// ensure that requested video mode is available for this TV standard, else step down to nearest that is

	{
		bool fConfirmed=false;
			// default mode if requested mode is crazy or doesn't care
		if((pcurrentvidemodedetails->m_nVideoModeIndex<0) || (pcurrentvidemodedetails->m_nVideoModeIndex>=VIDEO_MODE_COUNT)) {
			pcurrentvidemodedetails->m_nVideoModeIndex=VIDEO_MODE_800x600;
		}

		while((!fConfirmed) && (pcurrentvidemodedetails->m_nVideoModeIndex)) { // mode 0, 640x480 must be available in every TV_ENCODING
			if(videomodetables.m_dwaVideoModeNv[pcurrentvidemodedetails->m_bTvStandard][pcurrentvidemodedetails->m_nVideoModeIndex][0]) { // nonzero indicates supported
				fConfirmed=true;
			} else { // not supported in this TV encoding, step down
				pcurrentvidemodedetails->m_nVideoModeIndex--;
			}
		}
	}

	IoOutputByte(0x80d3, 5);  // definitively kill video out using an ACPI control pin

	vgaout(pcurrentvidemodedetails->m_pbBaseAddressVideo+NVCRTC, 0x1f, 0x57);

	voutb(pcurrentvidemodedetails->m_pbBaseAddressVideo+0x0c03c4, 0x06);
	voutb(pcurrentvidemodedetails->m_pbBaseAddressVideo+0x0c03c5, 0x57);

	vgaout(pcurrentvidemodedetails->m_pbBaseAddressVideo+NVCRTC, 0x21, 0xff);
	voutl(pcurrentvidemodedetails->m_pbBaseAddressVideo+0x680880, 0x21121111);
	vgaout(pcurrentvidemodedetails->m_pbBaseAddressVideo+0x0C03D4, 0x11, 0);

	voutl(pcurrentvidemodedetails->m_pbBaseAddressVideo+0x68050c,	vinl(pcurrentvidemodedetails->m_pbBaseAddressVideo+0x68050c)|0x10020000);

	voutb(pcurrentvidemodedetails->m_pbBaseAddressVideo+0x0C03C3, 1);
	voutb(pcurrentvidemodedetails->m_pbBaseAddressVideo+0x0C03C2, 0xe3);

	voutl(pcurrentvidemodedetails->m_pbBaseAddressVideo+0x680600, 0 /*0x0100030*/);  // this is actually set to a parameter to the kernel routine

	if(arg_8!=0) {
		voutl(pcurrentvidemodedetails->m_pbBaseAddressVideo+0x680630, 0);
		voutl(pcurrentvidemodedetails->m_pbBaseAddressVideo+0x6808c4, 0);
		voutl(pcurrentvidemodedetails->m_pbBaseAddressVideo+0x68084c, 0);
	}

	vgaout(pcurrentvidemodedetails->m_pbBaseAddressVideo+0x6013D4, 0x19, 0xe0);
	vgaout(pcurrentvidemodedetails->m_pbBaseAddressVideo+0x6013D4, 0x28, 0x80);
	vgaout(pcurrentvidemodedetails->m_pbBaseAddressVideo+0x6013D4, 0x28, 0x00);

	voutb(pcurrentvidemodedetails->m_pbBaseAddressVideo+0x6013c0, 0x20);

	voutl(pcurrentvidemodedetails->m_pbBaseAddressVideo+0x6806a0, (dwStash & 1)^1);
	vgaout(pcurrentvidemodedetails->m_pbBaseAddressVideo+NVCRTC, 0x28, 0x80 | arg_C);


		// frankenvideo-derived Video Mode setting

	{
		DWORD * volatile pdw=(DWORD * volatile)pcurrentvidemodedetails->m_pbBaseAddressVideo;
		int n=0, n1=0;
		BYTE b;

			// nVidia main registers

		while(n < (sizeof(videomodetables.m_dwaAddressesNv)/sizeof(DWORD))) {
			pdw[(videomodetables.m_dwaAddressesNv[n]>>2)]=
				videomodetables.m_dwaVideoModeNv[pcurrentvidemodedetails->m_bTvStandard][pcurrentvidemodedetails->m_nVideoModeIndex][n];
			n++;
		}

			// nVidia CRTC registers

		for(n=0;n1<(sizeof(videomodetables.m_baVideoModeCrtc[0][0]));n++) {
			if((n!=0x11) && (n!=0x1f)) {
				vgaout(pcurrentvidemodedetails->m_pbBaseAddressVideo+NVCRTC, n,
					videomodetables.m_baVideoModeCrtc[pcurrentvidemodedetails->m_bTvStandard][pcurrentvidemodedetails->m_nVideoModeIndex][n1++]
				);
			}
		}

			// Conexant init (starts at register 0x2e)
		n1=0;
		for(n=0x2e;n<0x100;n+=2) {

			switch(n) {
				case 0x6c:
				case 0xb8:
					break;

				case 0xa8:
					b=pcurrentvidemodedetails->m_bFinalConexantA8=
						videomodetables.m_baVideoModeConexant[pcurrentvidemodedetails->m_bTvStandard][pcurrentvidemodedetails->m_nVideoModeIndex][n1];
					if(pcurrentvidemodedetails->m_fForceEncoderLumaAndChromaToZeroInitially) b=0;
					I2CTransmitWord(0x45, (n<<8)|b);
					break;

				case 0xaa:
					b=pcurrentvidemodedetails->m_bFinalConexantAA=
						videomodetables.m_baVideoModeConexant[pcurrentvidemodedetails->m_bTvStandard][pcurrentvidemodedetails->m_nVideoModeIndex][n1];
					if(pcurrentvidemodedetails->m_fForceEncoderLumaAndChromaToZeroInitially) b=0;
					I2CTransmitWord(0x45, (n<<8)|b);
					break;

				case 0xac:
					b=pcurrentvidemodedetails->m_bFinalConexantAC=
						videomodetables.m_baVideoModeConexant[pcurrentvidemodedetails->m_bTvStandard][pcurrentvidemodedetails->m_nVideoModeIndex][n1];
					if(pcurrentvidemodedetails->m_fForceEncoderLumaAndChromaToZeroInitially) b=0;
					I2CTransmitWord(0x45, (n<<8)|b);
					break;

				default:
					I2CTransmitWord(0x45, (n<<8)|videomodetables.m_baVideoModeConexant[pcurrentvidemodedetails->m_bTvStandard][pcurrentvidemodedetails->m_nVideoModeIndex][n1]);
					break;
			}
			n1++;
		}
	}

	voutl(pcurrentvidemodedetails->m_pbBaseAddressVideo+0x001804, vinl(pcurrentvidemodedetails->m_pbBaseAddressVideo+0x001804)|4);  // from kernel sub_0_80045C2A
	voutl(pcurrentvidemodedetails->m_pbBaseAddressVideo+0x600140, 0);
	voutl(pcurrentvidemodedetails->m_pbBaseAddressVideo+0x609140, 0);
	voutl(pcurrentvidemodedetails->m_pbBaseAddressVideo+0x680600, 0);  // without this, blue horrors


	{
		int n=0;
		while(n<sizeof(baSequencerInit)) {
			vgaout(pcurrentvidemodedetails->m_pbBaseAddressVideo+0x0C03C4, n, baSequencerInit[n]);
			n++;
		}
	}

	{
		int n=0;
		while(n<sizeof(baGraInit)) {
			vgaout(pcurrentvidemodedetails->m_pbBaseAddressVideo+0x0C03CE, n, baGraInit[n]);
			n++;
		}
	}

	{
		int n=0;
		while(n<sizeof(baAttrInit)) {
//				voutw(0x6013c0, n|(baAttrInit[n]<<8));
			voutb(pcurrentvidemodedetails->m_pbBaseAddressVideo+0x6013c0, n);
			voutb(pcurrentvidemodedetails->m_pbBaseAddressVideo+0x6013c0, baAttrInit[n]);
			n++;
		}
	}

	pcurrentvidemodedetails->m_dwMarginXInPixelsRecommended=0;
	pcurrentvidemodedetails->m_dwMarginYInLinesRecommended=0;

		// AV-pack specific init

	switch(pcurrentvidemodedetails->m_bAvPack) {  // radically changes colours if enabled on composite
		case 0:
			{
					// this might set up RGB out?
				voutl(pcurrentvidemodedetails->m_pbBaseAddressVideo+0x680880, 0x21121111);
				voutl(pcurrentvidemodedetails->m_pbBaseAddressVideo+0x609140, 0);
				voutl(pcurrentvidemodedetails->m_pbBaseAddressVideo+0x682600, 0x100030);
				voutl(pcurrentvidemodedetails->m_pbBaseAddressVideo+0x682630, 0);
				voutl(pcurrentvidemodedetails->m_pbBaseAddressVideo+0x682634, 0);
				voutl(pcurrentvidemodedetails->m_pbBaseAddressVideo+0x682638, 0);
				voutl(pcurrentvidemodedetails->m_pbBaseAddressVideo+0x68263c, 0);
			}
			break;
		default:
			{
				voutl(pcurrentvidemodedetails->m_pbBaseAddressVideo+0x680880, 0x21101100);
				voutl(pcurrentvidemodedetails->m_pbBaseAddressVideo+0x682600,0 /* 0x100030 */);
			}
			break;
	};
	
		// resolution-specific init

	switch(pcurrentvidemodedetails->m_nVideoModeIndex) {

		case VIDEO_MODE_640x480:

			pcurrentvidemodedetails->m_dwWidthInPixels=640;
			pcurrentvidemodedetails->m_dwHeightInLines=480;

			switch(pcurrentvidemodedetails->m_bTvStandard) {

				case TV_ENCODING_PAL:
					break;

				case TV_ENCODING_NTSC:
					voutl(pcurrentvidemodedetails->m_pbBaseAddressVideo+0x682630, 2);
					voutl(pcurrentvidemodedetails->m_pbBaseAddressVideo+0x682634, 0);
					voutl(pcurrentvidemodedetails->m_pbBaseAddressVideo+0x682638, 0);
					voutl(pcurrentvidemodedetails->m_pbBaseAddressVideo+0x68263c, 0);

					voutl(pcurrentvidemodedetails->m_pbBaseAddressVideo+0x680324, 0x8b323bd7);
					voutl(pcurrentvidemodedetails->m_pbBaseAddressVideo+0x680328, 0x66913ddf);
					voutl(pcurrentvidemodedetails->m_pbBaseAddressVideo+0x68032c, 0x82afff2e);
					voutl(pcurrentvidemodedetails->m_pbBaseAddressVideo+0x680330, 0xff4f218f);
					voutl(pcurrentvidemodedetails->m_pbBaseAddressVideo+0x680504, 0x7702);
					voutl(pcurrentvidemodedetails->m_pbBaseAddressVideo+0x680508, 0x3c20d);
					voutl(pcurrentvidemodedetails->m_pbBaseAddressVideo+0x680518, 0x10101);
					voutl(pcurrentvidemodedetails->m_pbBaseAddressVideo+0x680520, 0x3c20d);
					voutl(pcurrentvidemodedetails->m_pbBaseAddressVideo+0x680544, 0x7702);
					voutl(pcurrentvidemodedetails->m_pbBaseAddressVideo+0x680548, 0x3c20d);
					voutl(pcurrentvidemodedetails->m_pbBaseAddressVideo+0x680558, 0x20202);
					voutl(pcurrentvidemodedetails->m_pbBaseAddressVideo+0x680560, 0x3c20d);
					break;

			}
			break;

		case VIDEO_MODE_640x576:
			{

				pcurrentvidemodedetails->m_dwWidthInPixels=640;
				pcurrentvidemodedetails->m_dwHeightInLines=576;
				pcurrentvidemodedetails->m_dwMarginXInPixelsRecommended=40; // pixels
				pcurrentvidemodedetails->m_dwMarginYInLinesRecommended=40; // lines
			}
			break;

		case VIDEO_MODE_720x576:
			{

				pcurrentvidemodedetails->m_dwWidthInPixels=720;
				pcurrentvidemodedetails->m_dwHeightInLines=576;
				pcurrentvidemodedetails->m_dwMarginXInPixelsRecommended=40; // pixels
				pcurrentvidemodedetails->m_dwMarginYInLinesRecommended=40; // lines
			}
			break;


		case VIDEO_MODE_800x600: // 800x600
			{
				pcurrentvidemodedetails->m_dwWidthInPixels=800;
				pcurrentvidemodedetails->m_dwHeightInLines=600;
				pcurrentvidemodedetails->m_dwMarginYInLinesRecommended=20; // lines
			}
			break;
	}

			// enable VSYNC interrupt action

#ifndef XBE
	voutl(pcurrentvidemodedetails->m_pbBaseAddressVideo+0x600140, 0x1);  // enable VSYNC interrupts
	voutl(pcurrentvidemodedetails->m_pbBaseAddressVideo+0x600100, 0x1);  // clear VSYNC int
	voutl(pcurrentvidemodedetails->m_pbBaseAddressVideo+0x608000, 0x3c00000);  //
	voutl(pcurrentvidemodedetails->m_pbBaseAddressVideo+0x600140, 1);  // enable VSYNC int
	voutl(pcurrentvidemodedetails->m_pbBaseAddressVideo+0x000140, 0x1);  // enable VSYNC interrupts
	voutl(pcurrentvidemodedetails->m_pbBaseAddressVideo+0x000100, 0x1);  // clear VSYNC int
	voutl(pcurrentvidemodedetails->m_pbBaseAddressVideo+0x008000, 0x3c00000);  //
	voutl(pcurrentvidemodedetails->m_pbBaseAddressVideo+0x000140, 1);  // enable VSYNC int
#endif

	{
		DWORD dwVideoFootprint=pcurrentvidemodedetails->m_dwWidthInPixels *pcurrentvidemodedetails->m_dwHeightInLines *4;
		DWORD dwSpare=0x400000 - dwVideoFootprint;  // what's left from the 4MByte allocation after the pixel stg is accounted for
		voutl(pcurrentvidemodedetails->m_pbBaseAddressVideo+0x600800, 0x03c00000);   // set video start address to 4M back from end of RAM
			// Oliver Schwartz's 2D accelleration mods
		voutl(pcurrentvidemodedetails->m_pbBaseAddressVideo+0x400820, dwSpare);
		voutl(pcurrentvidemodedetails->m_pbBaseAddressVideo+0x400824, dwSpare);
		voutl(pcurrentvidemodedetails->m_pbBaseAddressVideo+0x400828, dwSpare);
		voutl(pcurrentvidemodedetails->m_pbBaseAddressVideo+0x40082c, dwSpare);
		voutl(pcurrentvidemodedetails->m_pbBaseAddressVideo+0x400684, 0x03ffffff);
		voutl(pcurrentvidemodedetails->m_pbBaseAddressVideo+0x400688, 0x03ffffff);
		voutl(pcurrentvidemodedetails->m_pbBaseAddressVideo+0x40068c, 0x03ffffff);
		voutl(pcurrentvidemodedetails->m_pbBaseAddressVideo+0x400690, 0x03ffffff);
	}
		// timing reset for Conexant

	I2CTransmitWord(0x45, 0x6cc6); // nb this was inside the #ifndef XBE above, I think it should be outside, will not affect XBE

		// all these are necessary to bring up video through the conexant

	IoOutputByte(0x80d6, 5);  // ACPI IO thing seen in kernel, set to 4 or 5
	IoOutputByte(0x80d8, 4);  // ACPI IO thing seen in kernel, set to 4
	IoOutputByte(0x80d3, 4);  // ACPI IO video enable REQUIRED <-- particularly crucial to get composite out


	vgaout(pcurrentvidemodedetails->m_pbBaseAddressVideo+0x0C03C4, 1, 1); // screen on REQUIRED
	voutb(pcurrentvidemodedetails->m_pbBaseAddressVideo+0x6013c0, 0x01);

	voutb(pcurrentvidemodedetails->m_pbBaseAddressVideo+0x0C03C2, 0xe3);
	vgaout(pcurrentvidemodedetails->m_pbBaseAddressVideo+NVCRTC, 0x11, 0x20);


}



