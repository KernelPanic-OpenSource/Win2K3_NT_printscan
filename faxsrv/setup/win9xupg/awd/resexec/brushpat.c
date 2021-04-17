/*==============================================================================
This file defines 8x8 pixel monochrome patterns for standard GDI brushes.  
The patterns are for 6 hatched brushes and 65 gray levels of solid brushes.

05-30-93     RajeevD     Created.
02-15-94     RajeevD     Integrated into unified resource executor.
05-12-94     RajeevD     Ported to C for Win32.
==============================================================================*/

#define DB(b7,b6,b5,b4,b3,b2,b1,b0)\
  ((b7<<7)|(b6<<6)|(b5<<5)|(b4<<4)|(b3<<3)|(b2<<2)|(b1<<1)|b0)

const unsigned char BrushPat[71][8] = 
{
	// HS_HORIZONTAL
	DB(0,0,0,0,0,0,0,0),
	DB(0,0,0,0,0,0,0,0),
	DB(0,0,0,0,0,0,0,0),
	DB(1,1,1,1,1,1,1,1),
	DB(1,1,1,1,1,1,1,1),
	DB(0,0,0,0,0,0,0,0),
	DB(0,0,0,0,0,0,0,0),
	DB(0,0,0,0,0,0,0,0),

	// HS_VERTICAL
	DB(0,0,0,1,1,0,0,0),
	DB(0,0,0,1,1,0,0,0),
	DB(0,0,0,1,1,0,0,0),
	DB(0,0,0,1,1,0,0,0),
	DB(0,0,0,1,1,0,0,0),
	DB(0,0,0,1,1,0,0,0),
	DB(0,0,0,1,1,0,0,0),
	DB(0,0,0,1,1,0,0,0),

	// HS_FDIAGONAL
	DB(0,0,0,1,1,0,0,0),
	DB(0,0,0,0,1,1,0,0),
	DB(0,0,0,0,0,1,1,0),
	DB(0,0,0,0,0,0,1,1),
	DB(1,0,0,0,0,0,0,1),
	DB(1,1,0,0,0,0,0,0),
	DB(0,1,1,0,0,0,0,0),
	DB(0,0,1,1,0,0,0,0),

	// HS_BDIAGONAL
	DB(0,0,0,1,1,0,0,0),
	DB(0,0,1,1,0,0,0,0),
	DB(0,1,1,0,0,0,0,0),
	DB(1,1,0,0,0,0,0,0),
	DB(1,0,0,0,0,0,0,1),
	DB(0,0,0,0,0,0,1,1),
	DB(0,0,0,0,0,1,1,0),
	DB(0,0,0,0,1,1,0,0),

	// HS_CROSS
	DB(0,0,0,1,1,0,0,0),
	DB(0,0,0,1,1,0,0,0),
	DB(0,0,0,1,1,0,0,0),
	DB(1,1,1,1,1,1,1,1),
	DB(1,1,1,1,1,1,1,1),
	DB(0,0,0,1,1,0,0,0),
	DB(0,0,0,1,1,0,0,0),
	DB(0,0,0,1,1,0,0,0),

	// HS_DIAGCROSS 
	DB(0,0,0,1,1,0,0,0),
	DB(0,0,1,1,1,1,0,0),
	DB(0,1,1,0,0,1,1,0),
	DB(1,1,0,0,0,0,1,1),
	DB(1,0,0,0,0,0,0,1),
	DB(1,1,0,0,0,0,1,1),
	DB(0,1,1,0,0,1,1,0),
	DB(0,0,1,1,1,1,0,0),

	// BS_SOLID[0,0h]
	DB(0,0,0,0,0,0,0,0),
	DB(0,0,0,0,0,0,0,0),
	DB(0,0,0,0,0,0,0,0),
	DB(0,0,0,0,0,0,0,0),
	DB(0,0,0,0,0,0,0,0),
	DB(0,0,0,0,0,0,0,0),
	DB(0,0,0,0,0,0,0,0),
	DB(0,0,0,0,0,0,0,0),

	// BS_SOLID[0,1,h]
	DB(0,0,0,0,0,0,0,0),
	DB(0,0,0,0,0,0,0,0),
	DB(0,0,0,0,0,0,0,0),
	DB(0,0,0,0,0,0,0,0),
	DB(0,0,0,0,0,0,0,0),
	DB(0,0,0,0,0,0,0,0),
	DB(0,0,0,0,0,1,0,0),
	DB(0,0,0,0,0,0,0,0),

	// BS_SOLID[02h]
	DB(0,0,0,0,0,0,0,0),
	DB(0,0,0,0,0,0,0,0),
	DB(0,1,0,0,0,0,0,0),
	DB(0,0,0,0,0,0,0,0),
	DB(0,0,0,0,0,0,0,0),
	DB(0,0,0,0,0,0,0,0),
	DB(0,0,0,0,0,1,0,0),
	DB(0,0,0,0,0,0,0,0),

	// BS_SOLID[03h]
	DB(0,0,0,0,0,0,0,0),
	DB(0,0,0,0,0,0,0,0),
	DB(0,1,0,0,0,0,0,0),
	DB(0,0,0,0,0,0,0,0),
	DB(0,0,0,0,0,0,0,0),
	DB(0,0,0,0,0,0,0,0),
	DB(0,0,0,0,0,1,1,0),
	DB(0,0,0,0,0,0,0,0),

	// BS_SOLID[04h]
	DB(0,0,0,0,0,0,0,0),
	DB(0,0,0,0,0,0,0,0),
	DB(0,1,1,0,0,0,0,0),
	DB(0,0,0,0,0,0,0,0),
	DB(0,0,0,0,0,0,0,0),
	DB(0,0,0,0,0,0,0,0),
	DB(0,0,0,0,0,1,1,0),
	DB(0,0,0,0,0,0,0,0),

	// BS_SOLID[05h]
	DB(0,0,0,0,0,0,0,0),
	DB(0,0,0,0,0,0,0,0),
	DB(0,1,1,0,0,0,0,0),
	DB(0,0,0,0,0,0,0,0),
	DB(0,0,0,0,0,0,0,0),
	DB(0,0,0,0,0,0,1,0),
	DB(0,0,0,0,0,1,1,0),
	DB(0,0,0,0,0,0,0,0),

	// BS_SOLID[06h]
	DB(0,0,0,0,0,0,0,0),
	DB(0,0,1,0,0,0,0,0),
	DB(0,1,1,0,0,0,0,0),
	DB(0,0,0,0,0,0,0,0),
	DB(0,0,0,0,0,0,0,0),
	DB(0,0,0,0,0,0,1,0),
	DB(0,0,0,0,0,1,1,0),
	DB(0,0,0,0,0,0,0,0),

	// BS_SOLID[07h]
	DB(0,0,0,0,0,0,0,0),
	DB(0,0,1,0,0,0,0,0),
	DB(0,1,1,0,0,0,0,0),
	DB(0,0,0,0,0,0,0,0),
	DB(0,0,0,0,0,0,0,0),
	DB(0,0,0,0,0,1,1,0),
	DB(0,0,0,0,0,1,1,0),
	DB(0,0,0,0,0,0,0,0),

	// BS_SOLID[08h]
	DB(0,0,0,0,0,0,0,0),
	DB(0,1,1,0,0,0,0,0),
	DB(0,1,1,0,0,0,0,0),
	DB(0,0,0,0,0,0,0,0),
	DB(0,0,0,0,0,0,0,0),
	DB(0,0,0,0,0,1,1,0),
	DB(0,0,0,0,0,1,1,0),
	DB(0,0,0,0,0,0,0,0),

	// BS_SOLID[09h]
	DB(0,0,0,0,0,0,0,0),
	DB(0,1,1,0,0,0,0,0),
	DB(0,1,1,0,0,0,0,0),
	DB(0,0,0,0,0,0,0,0),
	DB(0,0,0,0,0,0,0,0),
	DB(0,0,0,0,1,1,1,0),
	DB(0,0,0,0,0,1,1,0),
	DB(0,0,0,0,0,0,0,0),

	// BS_SOLID[0Ah]
	DB(0,0,0,0,0,0,0,0),
	DB(1,1,1,0,0,0,0,0),
	DB(0,1,1,0,0,0,0,0),
	DB(0,0,0,0,0,0,0,0),
	DB(0,0,0,0,0,0,0,0),
	DB(0,0,0,0,1,1,1,0),
	DB(0,0,0,0,0,1,1,0),
	DB(0,0,0,0,0,0,0,0),

	// BS_SOLID[0Bh]
	DB(0,0,0,0,0,0,0,0),
	DB(1,1,1,0,0,0,0,0),
	DB(0,1,1,0,0,0,0,0),
	DB(0,0,0,0,0,0,0,0),
	DB(0,0,0,0,0,0,0,0),
	DB(0,0,0,0,1,1,1,0),
	DB(0,0,0,0,1,1,1,0),
	DB(0,0,0,0,0,0,0,0),

	// BS_SOLID[0Ch]
	DB(0,0,0,0,0,0,0,0),
	DB(1,1,1,0,0,0,0,0),
	DB(1,1,1,0,0,0,0,0),
	DB(0,0,0,0,0,0,0,0),
	DB(0,0,0,0,0,0,0,0),
	DB(0,0,0,0,1,1,1,0),
	DB(0,0,0,0,1,1,1,0),
	DB(0,0,0,0,0,0,0,0),

	// BS_SOLID[0Dh]
	DB(0,0,0,0,0,0,0,0),
	DB(1,1,1,0,0,0,0,0),
	DB(1,1,1,0,0,0,0,0),
	DB(0,0,0,0,0,0,0,0),
	DB(0,0,0,0,0,0,0,0),
	DB(0,0,0,0,1,1,1,0),
	DB(0,0,0,0,1,1,1,0),
	DB(0,0,0,0,0,1,0,0),

	// BS_SOLID[0Eh]
	DB(0,0,0,0,0,0,0,0),
	DB(1,1,1,0,0,0,0,0),
	DB(1,1,1,0,0,0,0,0),
	DB(0,1,0,0,0,0,0,0),
	DB(0,0,0,0,0,0,0,0),
	DB(0,0,0,0,1,1,1,0),
	DB(0,0,0,0,1,1,1,0),
	DB(0,0,0,0,0,1,0,0),

	// BS_SOLID[0Fh]
	DB(0,0,0,0,0,0,0,0),
	DB(1,1,1,0,0,0,0,0),
	DB(1,1,1,0,0,0,0,0),
	DB(0,1,0,0,0,0,0,0),
	DB(0,0,0,0,0,0,0,0),
	DB(0,0,0,0,1,1,1,0),
	DB(0,0,0,0,1,1,1,0),
	DB(0,0,0,0,0,1,1,0),

	// BS_SOLID[1,0h]
	DB(0,0,0,0,0,0,0,0),
	DB(1,1,1,0,0,0,0,0),
	DB(1,1,1,0,0,0,0,0),
	DB(0,1,1,0,0,0,0,0),
	DB(0,0,0,0,0,0,0,0),
	DB(0,0,0,0,1,1,1,0),
	DB(0,0,0,0,1,1,1,0),
	DB(0,0,0,0,0,1,1,0),

	// BS_SOLID[1,1,h]
	DB(0,0,0,0,0,0,0,0),
	DB(1,1,1,0,0,0,0,0),
	DB(1,1,1,0,0,0,0,0),
	DB(0,1,1,0,0,0,0,0),
	DB(0,0,0,0,0,0,0,0),
	DB(0,0,0,0,1,1,1,0),
	DB(0,0,0,0,1,1,1,0),
	DB(0,0,0,0,1,1,1,0),

	// BS_SOLID[1,2h]
	DB(0,0,0,0,0,0,0,0),
	DB(1,1,1,0,0,0,0,0),
	DB(1,1,1,0,0,0,0,0),
	DB(1,1,1,0,0,0,0,0),
	DB(0,0,0,0,0,0,0,0),
	DB(0,0,0,0,1,1,1,0),
	DB(0,0,0,0,1,1,1,0),
	DB(0,0,0,0,1,1,1,0),

	// BS_SOLID[1,3h]
	DB(0,0,0,0,0,0,0,0),
	DB(1,1,1,0,0,0,0,0),
	DB(1,1,1,0,0,0,0,0),
	DB(1,1,1,0,0,0,0,0),
	DB(0,0,0,0,0,0,0,0),
	DB(0,0,0,0,1,1,1,1),
	DB(0,0,0,0,1,1,1,0),
	DB(0,0,0,0,1,1,1,0),

	// BS_SOLID[1,4h]
	DB(0,0,0,0,0,0,0,0),
	DB(1,1,1,1,0,0,0,0),
	DB(1,1,1,0,0,0,0,0),
	DB(1,1,1,0,0,0,0,0),
	DB(0,0,0,0,0,0,0,0),
	DB(0,0,0,0,1,1,1,1),
	DB(0,0,0,0,1,1,1,0),
	DB(0,0,0,0,1,1,1,0),

	// BS_SOLID[1,5h]
	DB(0,0,0,0,0,0,0,0),
	DB(1,1,1,1,0,0,0,0),
	DB(1,1,1,0,0,0,0,0),
	DB(1,1,1,0,0,0,0,0),
	DB(0,0,0,0,0,0,0,0),
	DB(0,0,0,0,1,1,1,1),
	DB(0,0,0,0,1,1,1,1),
	DB(0,0,0,0,1,1,1,0),

	// BS_SOLID[1,6h]
	DB(0,0,0,0,0,0,0,0),
	DB(1,1,1,1,0,0,0,0),
	DB(1,1,1,1,0,0,0,0),
	DB(1,1,1,0,0,0,0,0),
	DB(0,0,0,0,0,0,0,0),
	DB(0,0,0,0,1,1,1,1),
	DB(0,0,0,0,1,1,1,1),
	DB(0,0,0,0,1,1,1,0),

	// BS_SOLID[1,7h]
	DB(0,0,0,0,0,0,0,0),
	DB(1,1,1,1,0,0,0,0),
	DB(1,1,1,1,0,0,0,0),
	DB(1,1,1,0,0,0,0,0),
	DB(0,0,0,0,0,1,0,0),
	DB(0,0,0,0,1,1,1,1),
	DB(0,0,0,0,1,1,1,1),
	DB(0,0,0,0,1,1,1,0),

	// BS_SOLID[1,8h]
	DB(0,1,0,0,0,0,0,0),
	DB(1,1,1,1,0,0,0,0),
	DB(1,1,1,1,0,0,0,0),
	DB(1,1,1,0,0,0,0,0),
	DB(0,0,0,0,0,1,0,0),
	DB(0,0,0,0,1,1,1,1),
	DB(0,0,0,0,1,1,1,1),
	DB(0,0,0,0,1,1,1,0),

	// BS_SOLID[1,9h]
	DB(0,1,0,0,0,0,0,0),
	DB(1,1,1,1,0,0,0,0),
	DB(1,1,1,1,0,0,0,0),
	DB(1,1,1,0,0,0,0,0),
	DB(0,0,0,0,0,1,1,0),
	DB(0,0,0,0,1,1,1,1),
	DB(0,0,0,0,1,1,1,1),
	DB(0,0,0,0,1,1,1,0),

	// BS_SOLID[1,Ah]
	DB(0,1,1,0,0,0,0,0),
	DB(1,1,1,1,0,0,0,0),
	DB(1,1,1,1,0,0,0,0),
	DB(1,1,1,0,0,0,0,0),
	DB(0,0,0,0,0,1,1,0),
	DB(0,0,0,0,1,1,1,1),
	DB(0,0,0,0,1,1,1,1),
	DB(0,0,0,0,1,1,1,0),

	// BS_SOLID[1,Bh]
	DB(0,1,1,0,0,0,0,0),
	DB(1,1,1,1,0,0,0,0),
	DB(1,1,1,1,0,0,0,0),
	DB(1,1,1,0,0,0,0,0),
	DB(0,0,0,0,1,1,1,0),
	DB(0,0,0,0,1,1,1,1),
	DB(0,0,0,0,1,1,1,1),
	DB(0,0,0,0,1,1,1,0),

	// BS_SOLID[1,Ch]
	DB(1,1,1,0,0,0,0,0),
	DB(1,1,1,1,0,0,0,0),
	DB(1,1,1,1,0,0,0,0),
	DB(1,1,1,0,0,0,0,0),
	DB(0,0,0,0,1,1,1,0),
	DB(0,0,0,0,1,1,1,1),
	DB(0,0,0,0,1,1,1,1),
	DB(0,0,0,0,1,1,1,0),

	// BS_SOLID[1,Dh]
	DB(1,1,1,0,0,0,0,0),
	DB(1,1,1,1,0,0,0,0),
	DB(1,1,1,1,0,0,0,0),
	DB(1,1,1,0,0,0,0,0),
	DB(0,0,0,0,1,1,1,0),
	DB(0,0,0,0,1,1,1,1),
	DB(0,0,0,0,1,1,1,1),
	DB(0,0,0,0,1,1,1,1),

	// BS_SOLID[1,Eh]
	DB(1,1,1,0,0,0,0,0),
	DB(1,1,1,1,0,0,0,0),
	DB(1,1,1,1,0,0,0,0),
	DB(1,1,1,1,0,0,0,0),
	DB(0,0,0,0,1,1,1,0),
	DB(0,0,0,0,1,1,1,1),
	DB(0,0,0,0,1,1,1,1),
	DB(0,0,0,0,1,1,1,1),

	// BS_SOLID[1,Fh]
	DB(1,1,1,0,0,0,0,0),
	DB(1,1,1,1,0,0,0,0),
	DB(1,1,1,1,0,0,0,0),
	DB(1,1,1,1,0,0,0,0),
	DB(0,0,0,0,1,1,1,1),
	DB(0,0,0,0,1,1,1,1),
	DB(0,0,0,0,1,1,1,1),
	DB(0,0,0,0,1,1,1,1),

	// BS_SOLID[20h]
	DB(1,1,1,1,0,0,0,0),
	DB(1,1,1,1,0,0,0,0),
	DB(1,1,1,1,0,0,0,0),
	DB(1,1,1,1,0,0,0,0),
	DB(0,0,0,0,1,1,1,1),
	DB(0,0,0,0,1,1,1,1),
	DB(0,0,0,0,1,1,1,1),
	DB(0,0,0,0,1,1,1,1),

	// BS_SOLID[21,h]
	DB(1,1,1,1,0,0,0,0),
	DB(1,1,1,1,0,0,0,0),
	DB(1,1,1,1,0,0,0,0),
	DB(1,1,1,1,0,0,0,0),
	DB(0,0,0,1,1,1,1,1),
	DB(0,0,0,0,1,1,1,1),
	DB(0,0,0,0,1,1,1,1),
	DB(0,0,0,0,1,1,1,1),

	// BS_SOLID[22h]
	DB(1,1,1,1,0,0,0,1),
	DB(1,1,1,1,0,0,0,0),
	DB(1,1,1,1,0,0,0,0),
	DB(1,1,1,1,0,0,0,0),
	DB(0,0,0,1,1,1,1,1),
	DB(0,0,0,0,1,1,1,1),
	DB(0,0,0,0,1,1,1,1),
	DB(0,0,0,0,1,1,1,1),

	// BS_SOLID[23h]
	DB(1,1,1,1,0,0,0,1),
	DB(1,1,1,1,0,0,0,0),
	DB(1,1,1,1,0,0,0,0),
	DB(1,1,1,1,0,0,0,0),
	DB(0,0,0,1,1,1,1,1),
	DB(0,0,0,0,1,1,1,1),
	DB(0,0,0,0,1,1,1,1),
	DB(0,0,0,1,1,1,1,1),

	// BS_SOLID[24h]
	DB(1,1,1,1,0,0,0,1),
	DB(1,1,1,1,0,0,0,0),
	DB(1,1,1,1,0,0,0,0),
	DB(1,1,1,1,0,0,0,1),
	DB(0,0,0,1,1,1,1,1),
	DB(0,0,0,0,1,1,1,1),
	DB(0,0,0,0,1,1,1,1),
	DB(0,0,0,1,1,1,1,1),

	// BS_SOLID[25h]
	DB(1,1,1,1,0,0,0,1),
	DB(1,1,1,1,0,0,0,0),
	DB(1,1,1,1,0,0,0,0),
	DB(1,1,1,1,0,0,0,1),
	DB(1,0,0,1,1,1,1,1),
	DB(0,0,0,0,1,1,1,1),
	DB(0,0,0,0,1,1,1,1),
	DB(0,0,0,1,1,1,1,1),

	// BS_SOLID[26h]
	DB(1,1,1,1,1,0,0,1),
	DB(1,1,1,1,0,0,0,0),
	DB(1,1,1,1,0,0,0,0),
	DB(1,1,1,1,0,0,0,1),
	DB(1,0,0,1,1,1,1,1),
	DB(0,0,0,0,1,1,1,1),
	DB(0,0,0,0,1,1,1,1),
	DB(0,0,0,1,1,1,1,1),

	// BS_SOLID[27h]
	DB(1,1,1,1,1,0,0,1),
	DB(1,1,1,1,0,0,0,0),
	DB(1,1,1,1,0,0,0,0),
	DB(1,1,1,1,0,0,0,1),
	DB(1,0,1,1,1,1,1,1),
	DB(0,0,0,0,1,1,1,1),
	DB(0,0,0,0,1,1,1,1),
	DB(0,0,0,1,1,1,1,1),

	// BS_SOLID[28h]
	DB(1,1,1,1,1,0,1,1),
	DB(1,1,1,1,0,0,0,0),
	DB(1,1,1,1,0,0,0,0),
	DB(1,1,1,1,0,0,0,1),
	DB(1,0,1,1,1,1,1,1),
	DB(0,0,0,0,1,1,1,1),
	DB(0,0,0,0,1,1,1,1),
	DB(0,0,0,1,1,1,1,1),

	// BS_SOLID[29h]
	DB(1,1,1,1,1,0,1,1),
	DB(1,1,1,1,0,0,0,0),
	DB(1,1,1,1,0,0,0,0),
	DB(1,1,1,1,0,0,0,1),
	DB(1,1,1,1,1,1,1,1),
	DB(0,0,0,0,1,1,1,1),
	DB(0,0,0,0,1,1,1,1),
	DB(0,0,0,1,1,1,1,1),

	// BS_SOLID[2Ah]
	DB(1,1,1,1,1,1,1,1),
	DB(1,1,1,1,0,0,0,0),
	DB(1,1,1,1,0,0,0,0),
	DB(1,1,1,1,0,0,0,1),
	DB(1,1,1,1,1,1,1,1),
	DB(0,0,0,0,1,1,1,1),
	DB(0,0,0,0,1,1,1,1),
	DB(0,0,0,1,1,1,1,1),

	// BS_SOLID[2Bh]
	DB(1,1,1,1,1,1,1,1),
	DB(1,1,1,1,0,0,0,0),
	DB(1,1,1,1,0,0,0,0),
	DB(1,1,1,1,0,0,0,1),
	DB(1,1,1,1,1,1,1,1),
	DB(0,0,0,0,1,1,1,1),
	DB(0,0,0,1,1,1,1,1),
	DB(0,0,0,1,1,1,1,1),

	// BS_SOLID[2Ch]
	DB(1,1,1,1,1,1,1,1),
	DB(1,1,1,1,0,0,0,0),
	DB(1,1,1,1,0,0,0,1),
	DB(1,1,1,1,0,0,0,1),
	DB(1,1,1,1,1,1,1,1),
	DB(0,0,0,0,1,1,1,1),
	DB(0,0,0,1,1,1,1,1),
	DB(0,0,0,1,1,1,1,1),

	// BS_SOLID[2Dh]
	DB(1,1,1,1,1,1,1,1),
	DB(1,1,1,1,0,0,0,0),
	DB(1,1,1,1,0,0,0,1),
	DB(1,1,1,1,0,0,0,1),
	DB(1,1,1,1,1,1,1,1),
	DB(0,0,0,1,1,1,1,1),
	DB(0,0,0,1,1,1,1,1),
	DB(0,0,0,1,1,1,1,1),

	// BS_SOLID[2Eh]
	DB(1,1,1,1,1,1,1,1),
	DB(1,1,1,1,0,0,0,1),
	DB(1,1,1,1,0,0,0,1),
	DB(1,1,1,1,0,0,0,1),
	DB(1,1,1,1,1,1,1,1),
	DB(0,0,0,1,1,1,1,1),
	DB(0,0,0,1,1,1,1,1),
	DB(0,0,0,1,1,1,1,1),

	// BS_SOLID[2Fh]
	DB(1,1,1,1,1,1,1,1),
	DB(1,1,1,1,0,0,0,1),
	DB(1,1,1,1,0,0,0,1),
	DB(1,1,1,1,0,0,0,1),
	DB(1,1,1,1,1,1,1,1),
	DB(0,0,0,1,1,1,1,1),
	DB(0,0,0,1,1,1,1,1),
	DB(1,0,0,1,1,1,1,1),

	// BS_SOLID[30h]
	DB(1,1,1,1,1,1,1,1),
	DB(1,1,1,1,0,0,0,1),
	DB(1,1,1,1,0,0,0,1),
	DB(1,1,1,1,1,0,0,1),
	DB(1,1,1,1,1,1,1,1),
	DB(0,0,0,1,1,1,1,1),
	DB(0,0,0,1,1,1,1,1),
	DB(1,0,0,1,1,1,1,1),

	// BS_SOLID[31,h]
	DB(1,1,1,1,1,1,1,1),
	DB(1,1,1,1,0,0,0,1),
	DB(1,1,1,1,0,0,0,1),
	DB(1,1,1,1,1,0,0,1),
	DB(1,1,1,1,1,1,1,1),
	DB(0,0,0,1,1,1,1,1),
	DB(0,0,0,1,1,1,1,1),
	DB(1,0,1,1,1,1,1,1),

	// BS_SOLID[32h]
	DB(1,1,1,1,1,1,1,1),
	DB(1,1,1,1,0,0,0,1),
	DB(1,1,1,1,0,0,0,1),
	DB(1,1,1,1,1,0,1,1),
	DB(1,1,1,1,1,1,1,1),
	DB(0,0,0,1,1,1,1,1),
	DB(0,0,0,1,1,1,1,1),
	DB(1,0,1,1,1,1,1,1),

	// BS_SOLID[33h]
	DB(1,1,1,1,1,1,1,1),
	DB(1,1,1,1,0,0,0,1),
	DB(1,1,1,1,0,0,0,1),
	DB(1,1,1,1,1,0,1,1),
	DB(1,1,1,1,1,1,1,1),
	DB(0,0,0,1,1,1,1,1),
	DB(0,0,0,1,1,1,1,1),
	DB(1,1,1,1,1,1,1,1),

	// BS_SOLID[34h]
	DB(1,1,1,1,1,1,1,1),
	DB(1,1,1,1,0,0,0,1),
	DB(1,1,1,1,0,0,0,1),
	DB(1,1,1,1,1,1,1,1),
	DB(1,1,1,1,1,1,1,1),
	DB(0,0,0,1,1,1,1,1),
	DB(0,0,0,1,1,1,1,1),
	DB(1,1,1,1,1,1,1,1),

	// BS_SOLID[35h]
	DB(1,1,1,1,1,1,1,1),
	DB(1,1,1,1,0,0,0,1),
	DB(1,1,1,1,0,0,0,1),
	DB(1,1,1,1,1,1,1,1),
	DB(1,1,1,1,1,1,1,1),
	DB(0,0,0,1,1,1,1,1),
	DB(1,0,0,1,1,1,1,1),
	DB(1,1,1,1,1,1,1,1),

	// BS_SOLID[36h]
	DB(1,1,1,1,1,1,1,1),
	DB(1,1,1,1,0,0,0,1),
	DB(1,1,1,1,1,0,0,1),
	DB(1,1,1,1,1,1,1,1),
	DB(1,1,1,1,1,1,1,1),
	DB(0,0,0,1,1,1,1,1),
	DB(1,0,0,1,1,1,1,1),
	DB(1,1,1,1,1,1,1,1),

	// BS_SOLID[37h]
	DB(1,1,1,1,1,1,1,1),
	DB(1,1,1,1,0,0,0,1),
	DB(1,1,1,1,1,0,0,1),
	DB(1,1,1,1,1,1,1,1),
	DB(1,1,1,1,1,1,1,1),
	DB(1,0,0,1,1,1,1,1),
	DB(1,0,0,1,1,1,1,1),
	DB(1,1,1,1,1,1,1,1),

	// BS_SOLID[38h]
	DB(1,1,1,1,1,1,1,1),
	DB(1,1,1,1,1,0,0,1),
	DB(1,1,1,1,1,0,0,1),
	DB(1,1,1,1,1,1,1,1),
	DB(1,1,1,1,1,1,1,1),
	DB(1,0,0,1,1,1,1,1),
	DB(1,0,0,1,1,1,1,1),
	DB(1,1,1,1,1,1,1,1),

	// BS_SOLID[39h]
	DB(1,1,1,1,1,1,1,1),
	DB(1,1,1,1,1,0,0,1),
	DB(1,1,1,1,1,0,0,1),
	DB(1,1,1,1,1,1,1,1),
	DB(1,1,1,1,1,1,1,1),
	DB(1,1,0,1,1,1,1,1),
	DB(1,0,0,1,1,1,1,1),
	DB(1,1,1,1,1,1,1,1),

	// BS_SOLID[3Ah]
	DB(1,1,1,1,1,1,1,1),
	DB(1,1,1,1,1,1,0,1),
	DB(1,1,1,1,1,0,0,1),
	DB(1,1,1,1,1,1,1,1),
	DB(1,1,1,1,1,1,1,1),
	DB(1,1,0,1,1,1,1,1),
	DB(1,0,0,1,1,1,1,1),
	DB(1,1,1,1,1,1,1,1),

	// BS_SOLID[3Bh]
	DB(1,1,1,1,1,1,1,1),
	DB(1,1,1,1,1,1,0,1),
	DB(1,1,1,1,1,0,0,1),
	DB(1,1,1,1,1,1,1,1),
	DB(1,1,1,1,1,1,1,1),
	DB(1,1,1,1,1,1,1,1),
	DB(1,0,0,1,1,1,1,1),
	DB(1,1,1,1,1,1,1,1),

	// BS_SOLID[3Ch]
	DB(1,1,1,1,1,1,1,1),
	DB(1,1,1,1,1,1,1,1),
	DB(1,1,1,1,1,0,0,1),
	DB(1,1,1,1,1,1,1,1),
	DB(1,1,1,1,1,1,1,1),
	DB(1,1,1,1,1,1,1,1),
	DB(1,0,0,1,1,1,1,1),
	DB(1,1,1,1,1,1,1,1),

	// BS_SOLID[3Dh]
	DB(1,1,1,1,1,1,1,1),
	DB(1,1,1,1,1,1,1,1),
	DB(1,1,1,1,1,0,0,1),
	DB(1,1,1,1,1,1,1,1),
	DB(1,1,1,1,1,1,1,1),
	DB(1,1,1,1,1,1,1,1),
	DB(1,0,1,1,1,1,1,1),
	DB(1,1,1,1,1,1,1,1),

	// BS_SOLID[3Eh]
	DB(1,1,1,1,1,1,1,1),
	DB(1,1,1,1,1,1,1,1),
	DB(1,1,1,1,1,0,1,1),
	DB(1,1,1,1,1,1,1,1),
	DB(1,1,1,1,1,1,1,1),
	DB(1,1,1,1,1,1,1,1),
	DB(1,0,1,1,1,1,1,1),
	DB(1,1,1,1,1,1,1,1),

	// BS_SOLID[3Fh]
	DB(1,1,1,1,1,1,1,1),
	DB(1,1,1,1,1,1,1,1),
	DB(1,1,1,1,1,0,1,1),
	DB(1,1,1,1,1,1,1,1),
	DB(1,1,1,1,1,1,1,1),
	DB(1,1,1,1,1,1,1,1),
	DB(1,1,1,1,1,1,1,1),
	DB(1,1,1,1,1,1,1,1),

	// BS_SOLID[40h]
	DB(1,1,1,1,1,1,1,1),
	DB(1,1,1,1,1,1,1,1),
	DB(1,1,1,1,1,1,1,1),
	DB(1,1,1,1,1,1,1,1),
	DB(1,1,1,1,1,1,1,1),
	DB(1,1,1,1,1,1,1,1),
	DB(1,1,1,1,1,1,1,1),
	DB(1,1,1,1,1,1,1,1),
};

