//
//	Debug output scrolling code submitted by Robin Mulloy
//
//
#ifndef XBOXRT_DEBUG
#define XBOXRT_DEBUG

#define WHITE   0x00FFFFFF
#define BLACK   0x00000000
#define RED     0x00FF0000
#define GREEN   0x0000FF00
#define BLUE    0x000000FF

#define XELL_BG 0x005139CF
#define XELL_FG 0x009E8FEE

#define WHITE_16BPP   0xFFFF
#define BLACK_16BPP  0x0000

#define XELL_BG_16BPP 0x051F
#define XELL_FG_16BPP 0x9E8F

/**
 * Prints a message to whatever debug facilities might
 * be available.
 */
void debugPrint(const char *format, ...);
void debugPrintNum(int i);
void debugPrintBinary( int num );
void debugPrintHex(const char *buffer, int length);
void debugClearScreen( void );
void advanceScreen( void );

#endif
