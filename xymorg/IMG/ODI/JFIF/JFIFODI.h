#pragma once
//*******************************************************************************************************************
//*																													*
//*   File:       JFIFODI.h																							*
//*   Suite:      xymorg Image Processing - ODI																		*
//*   Version:    1.0.0	  Build:  01																				*
//*   Author:     Ian Tree/HMNL																						*
//*																													*
//*   Copyright 2007 - 2014 Ian J. Tree																				*
//*******************************************************************************************************************
//*	JFIFODI.h																										*
//*																													*
//*	This header file contains the definitions and structures that describe the contents of a JFIF file.				*
//* The first section of the file describes On-Disk Image (ODI) structures.											*
//*																													*
//*	NOTES:																											*
//*																													*
//*																													*
//*******************************************************************************************************************
//*																													*
//*   History:																										*
//*																													*
//*	1.0.0 - 07/03/2014   -  Initial version																			*
//*																													*
//*******************************************************************************************************************

//  Include basic xymorg headers
#include	"../../../types.h"																		//  xymorg type definitions

//  Include xymorg image processing primitives
#include	"../../types.h"																			//  Image processing primitive types

namespace xymorg {
	//
	//  1 byte alignment
	//
#pragma pack (push, 1)

//
//  JFIF STRUCTURES
//

//*******************************************************************************************************************
//*																													*
//*   On-Disk Image (ODI) structures																				*
//*																													*
//*******************************************************************************************************************

//  Format Field identifiers/labels
constexpr auto	JFIF_BLKID_SIG			= 0xFF;								//  Block
constexpr auto	JFIF_BLKID_SOI			= 0xD8;								//  Start-Of-Image Block
constexpr auto	JFIF_BLKID_EOI			= 0xD9;								//  End-Of-Image Block
constexpr auto	JFIF_BLKID_APP0			= 0xE0;								//  Application usage Block
constexpr auto	EXIF_BLKID_APP1			= 0xE1;								//  Application usage Block  (EXIF 1)
constexpr auto	EXIF_BLKID_APP2			= 0xE2;								//  Application usage Block  (EXIF 2)
constexpr auto	JFIF_BLKID_DQT			= 0xDB;								//  Define quantization table block
constexpr auto	JFIF_BLKID_SOF0			= 0xC0;								//  Start of Frame - non-differential Huffman (baseline DCT)
constexpr auto	JFIF_BLKID_SOF1			= 0xC1;								//  Start of Frame - non-differential Huffman (extended sequential DCT)
constexpr auto	JFIF_BLKID_SOF2			= 0xC2;								//  Start of Frame - non-differential Huffman (progressive DCT)
constexpr auto	JFIF_BLKID_SOF3			= 0xC3;								//  Start of Frame - non-differential Huffman (Lossless)
constexpr auto	JFIF_BLKID_SOF5			= 0xC5;								//  Start of Frame - differential Huffman (sequential DCT)
constexpr auto	JFIF_BLKID_SOF6			= 0xC6;								//  Start of Frame - differential Huffman (progressive DCT)
constexpr auto	JFIF_BLKID_SOF7			= 0xC7;								//  Start of Frame - differential Huffman (Lossless)
constexpr auto	JFIF_BLKID_SOF9			= 0xC9;								//  Start of Frame - non-differential arithmetic (sequential DCT)
constexpr auto	JFIF_BLKID_SOF10		= 0xCA;								//  Start of Frame - non-differential arithmetic (progressive DCT)
constexpr auto	JFIF_BLKID_SOF11		= 0xCB;								//  Start of Frame - non-differential arithmetic (Lossless)
constexpr auto	JFIF_BLKID_SOF13		= 0xCD;								//  Start of Frame - differential arithmetic (sequential DCT)
constexpr auto	JFIF_BLKID_SOF14		= 0xCE;								//  Start of Frame - differential arithmetic (progressive DCT)
constexpr auto	JFIF_BLKID_SOF15		= 0xCF;								//  Start of Frame - differential arithmetic (Lossless)
constexpr auto	JFIF_BLKID_DHT			= 0xC4;								//  Define Huffman table
constexpr auto	JFIF_BLKID_DAC			= 0xCC;								//  Define arithmetic coding
constexpr auto	JFIF_BLKID_DNL			= 0xDC;								//  Define number of lines
constexpr auto	JFIF_BLKID_SOS			= 0xDA;								//  Start of scan segment
constexpr auto	JFIF_BLKID_STUFF		= 0x00;								//  Stuffing block
constexpr auto	JFIF_BLKID_RST0			= 0xD0;								//  Restart (0) block
constexpr auto	JFIF_BLKID_RST1			= 0xD1;								//  Restart (1) block
constexpr auto	JFIF_BLKID_RST2			= 0xD2;								//  Restart (2) block
constexpr auto	JFIF_BLKID_RST3			= 0xD3;								//  Restart (3) block
constexpr auto	JFIF_BLKID_RST4			= 0xD4;								//  Restart (4) block
constexpr auto	JFIF_BLKID_RST5			= 0xD5;								//  Restart (5) block
constexpr auto	JFIF_BLKID_RST6			= 0xD6;								//  Restart (6) block
constexpr auto	JFIF_BLKID_RST7			= 0xD7;								//  Restart (7) block
constexpr auto	JFIF_BLKID_APP2			= 0xE2;								//  Application Reserved Block 2
constexpr auto	JFIF_BLKID_APP3			= 0xE3;								//  Application Reserved Block 3
constexpr auto	JFIF_BLKID_APP4			= 0xE4;								//  Application Reserved Block 4
constexpr auto	JFIF_BLKID_APP5			= 0xE5;								//  Application Reserved Block 5
constexpr auto	JFIF_BLKID_APP6			= 0xE6;								//  Application Reserved Block 6
constexpr auto	JFIF_BLKID_APP7			= 0xE7;								//  Application Reserved Block 7
constexpr auto	JFIF_BLKID_APP8			= 0xE8;								//  Application Reserved Block 8
constexpr auto	JFIF_BLKID_APP9			= 0xE9;								//  Application Reserved Block 9
constexpr auto	JFIF_BLKID_APP10		= 0xEA;								//  Application Reserved Block 10
constexpr auto	JFIF_BLKID_APP11		= 0xEB;								//  Application Reserved Block 11
constexpr auto	JFIF_BLKID_APP12		= 0xEC;								//  Application Reserved Block 12
constexpr auto	JFIF_BLKID_APP13		= 0xED;								//  Application Reserved Block 13
constexpr auto	JFIF_BLKID_APP14		= 0xEE;								//  Application Reserved Block 14
constexpr auto	JFIF_BLKID_APP15		= 0xEF;								//  Application Reserved Block 15

//  width & height
//  Both width and height and lengths are encoded in big-endian form no matter what the host architecture is.
//  Use the GetSizeBE(field) and SetSizeBE(field, value) macros to get and set these fields
#define		GetSizeBE(x) ((x[0] * 256) + x[1])				//  Get width or height
#define		SetSizeBE(x,y) {					\
							x[0] = BYTE(y/256);	\
							x[1] = BYTE(y % 256);	\
							}

//  JFIF FILE HEADER
typedef struct JFIF_FILE_HEADER
{
	BYTE		Signature;													//  Block marker JFIF_BLKID_SIG
	BYTE		ID;															//  Start of image marker JFIF_BLKID_SOI
} JFIF_FILE_HEADER;

//  JFIF FILE TRAILER
typedef struct JFIF_FILE_TRAILER
{
	BYTE		Signature;													//  Block marker JFIF_BLKID_SIG
	BYTE		ID;															//  End of image marker JFIF_BLKID_EOI
} JFIF_FILE_TRAILER;

//  Generic JFIF data block
typedef struct JFIF_DATA_BLOCK
{
	BYTE		Signature;													//  Block marker JFIF_BLKID_SIG
	BYTE		ID;															//  APP0 marker JFIF_BLKID_APP0
	BYTE		Length[2];													//  Length
} JFIF_DATA_BLOCK;

//  JFIF Application Usage Block (APP0)
typedef struct JFIF_APP0_BLOCK
{
	BYTE		Signature;													//  Block marker JFIF_BLKID_SIG
	BYTE		ID;															//  APP0 marker JFIF_BLKID_APP0
	BYTE		Length[2];													//  Length
	BYTE		Identifier[5];												//  Identifier 'J','F','I',"F',0x00
	BYTE		MajVer;														//  Major version
	BYTE		MinVer;														//  Minor version
	BYTE		DUnits;														//  Density Units
	BYTE		HPD[2];														//  Horizontal pixel density
	BYTE		VPD[2];														//  Vertical pixel density
	BYTE		ThumbW;														//  Thumbnail width
	BYTE		ThumbH;														//  Thumbnail height
} JFIF_APP0_BLOCK;

//  Density units definitions
#define			JFIF_DUNITS_NONE			0								//  No units specified
#define			JFIF_DUNITS_DPI				1								//  Dots Per Inch (DPI)
#define			JFIF_DUNITS_DPC				2								//  Dots Per Centimeter (DPC)

//  JFIF Quantisation Table 
typedef struct JFIF_QTAB
{
	BYTE		Signature;													//  Block marker JFIF_BLKID_SIG
	BYTE		ID;															//  Block identifier JFIF_BLKID_DQT
	BYTE		Length[2];													//  Length
	BYTE		PandD;														//  Precision and Destinations	
} JFIF_QTAB;

#define			GetPrecision(x)			((x & 0xF0) >> 4)					//  Extract the precision
#define			GetDestination(x)		(x & 0x0F)							//  Extract the table destination

//  The Quantisation table is followed immediately by one OR more Quantisation Value Arrays.
//  The array is either 8 bit or 16 bit entries depending on the Precision identified in the table.
//  0 ==> 8 bit and 1 ==> 16 bit.
//

//  8 bit Quantisation Value Array
typedef struct JFIF_QV8_ARRAY {
	BYTE		PandD;														//  Precision and Destinations	
	BYTE		QV[64];														//  Quantisation values
} JFIF_QV8_ARRAY;

//  16 bit Quantisation Value Array
typedef struct QV16 {
	BYTE		PV[2];
} QV16;
typedef struct JFIF_QV16_ARRAY {
	BYTE		PandD;														//  Precision and Destinations	
	QV16		QV[64];														//  Quantisation Values
} JFIF_QV16_ARRAY;

//  JFIF Frame Cmponent
typedef struct JFIF_FRAME_COMPONENT
{
	BYTE		CompID;														//  Component identifier
	BYTE		HandV;														//  Horizontal & Vertical sampling factor
	BYTE		QTable;														//  Quantization table selector
} JFIF_FRAME_COMPONENT;

#define			GetVSampFactor(x)			(x & 0x0F)						//  Extract vertical sample factor
#define			GetHSampfactor(x)			((x & 0xF0) >> 4)				//  Extract horizontal sample factor

//  JFIF Frame Header
typedef struct JFIF_FRAME_HEADER
{
	BYTE		Signature;													//  Block marker JFIF_BLKID_SIG
	BYTE		ID;															//  Block identifier JFIF_BLKID_SOFn
	BYTE		Length[2];													//  Length
	BYTE		Precision;													//  Sample Precision
	BYTE		HLines[2];													//  Number of horizontal lines (Height)
	BYTE		VLines[2];													//  Number of vertical lines (Width)
	BYTE		Components;													//  Number of components
	JFIF_FRAME_COMPONENT		Comp[256];									//  Array of components
} JFIF_FRAME_HEADER;

//  JFIF Huffman Table Definition
typedef struct JFIF_HTAB
{
	BYTE		Signature;													//  Block marker JFIF_BLKID_SIG
	BYTE		ID;															//  APP0 marker JFIF_BLKID_SOS
	BYTE		Length[2];													//  Length
	BYTE		CandD;														//  Class and Destination
	BYTE		HTL[16];													//  Table lengths
	BYTE		HTEntry[16 * 256];											//  Table entries
} JFIF_HTAB;

#define			GetClass(x)					((x & 0xF0) >> 4)				//  Get the Huffman Table class

//  JFIF Scan Component
typedef struct JFIF_SCAN_COMPONENT
{
	BYTE		ScanSelector;												//  Scan component selector
	BYTE		DCandAC;													//  Entropy table selectors
} JFIF_SCAN_COMPONENT;

#define			GetACSelector(x)			(x & 0x03)						//  Extract the AC entropy table selector
#define			GetDCSelector(x)			((x & 0x30) >> 4)				//  Extract the DC entropy table selector

//  JFIF Scan Header (Part 1)
typedef struct JFIF_SCAN_HEADER1
{
	BYTE		Signature;													//  Block marker JFIF_BLKID_SIG
	BYTE		ID;															//  APP0 marker JFIF_BLKID_SOS
	BYTE		Length[2];													//  Length
	BYTE		Components;													//  Number of components
	JFIF_SCAN_COMPONENT Comp[256];											//  Scan components
} JFIF_SCAN_HEADER1;

//  JFIF Scan Header (Part 2)
typedef struct JFIF_SCAN_HEADER2
{
	BYTE		SSpecSel;													//  Start Spectral (or predictor) selector
	BYTE		ESpecSel;													//  End Spectral (or predictor) selector
	BYTE		AHiandLo;													//  Approximation bit selectors (Hi and Lo)
} JFIF_SCAN_HEADER2;

#define			GetABSelLo(x)				(x & 0x0F)						//  Extract the low approximation bit selector
#define			GetABSelHi(x)				((x & 0xF0) >> 4)				//  Extract the high approximation bit selector

//
//  JFIF block types
//
constexpr auto	JFIF_BLOCK_FH = 'B';										//  File Header (JFIF_FILE_HEADER)
constexpr auto	JFIF_BLOCK_APP0 = 'A';										//  Basic Application Usage block (0) (JFIF_APP0_BLOCK)
constexpr auto	JFIF_BLOCK_EXIF = 'E';										//  EXIF Application Usage block (1)
constexpr auto	JFIF_BLOCK_DQT = 'Q';										//  Define Quantisation Table Block (JFIF_QTAB)
constexpr auto	JFIF_BLOCK_DHT = 'H';										//  Define Huffman Table block (JFIF_HTAB)
constexpr auto	JFIF_BLOCK_SOF0 = 'F';										//  Start Of Frame (0) block (JFIF_FRAME_HEADER)
constexpr auto	JFIF_BLOCK_SOFX = 'G';										//  Start Of Frame (x) block
constexpr auto	JFIF_BLOCK_SOS = 'S';										//  Start Of Scan block (JFIF_SCAN_HEADER1)
constexpr auto	JFIF_BLOCK_EEB = 'I';										//  Image Data (Entropy Encoded Block)
constexpr auto	JFIF_BLOCK_RST = 'R';										//  Restart scan block(s)
constexpr auto	JFIF_BLOCK_RES = 'X';										//  Application reserved block(s)
constexpr auto	JFIF_BLOCK_FT = 'T';										//  File Trailer (JFIF_FILE_TRAILER)

#pragma pack (pop)

}
