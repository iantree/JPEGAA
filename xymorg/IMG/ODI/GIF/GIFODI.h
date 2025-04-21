#pragma	once
//
//*******************************************************************************************************************
//*																													*
//*   File:		  GIFODI.h																							*
//*   Suite:      xymorg Image Processing - ODI																		*
//*   Version:    1.0.1	  Build:  02																				*
//*   Author:     Ian Tree/HMNL																						*
//*																													*
//*   Copyright 2017 - 2024 Ian J. Tree																				*
//*******************************************************************************************************************
//*	GIFODI.h																										*
//*																													*
//*	This header file contains the all of the structures, constants and manipulators for handling the				*
//* On-Disk Image for a GIF.         																				*
//*																													*
//*	NOTES:																											*
//*																													*
//*																													*
//*******************************************************************************************************************
//*																													*
//*   History:																										*
//*																													*
//*	1.0.0 - 07/03/2014   -  Initial version																			*
//*	1.0.1 - 08/12/2024   -  Winter Cleanup																			*
//*																													*
//*******************************************************************************************************************

//  Include basic xymorg headers
#include	"../../../types.h"																		//  xymorg type definitions

//  Include xymorg image processing primitives
#include	"../../types.h"																			//  Image processing primitive types

namespace xymorg {

	//
	//  GIF STRUCTURES
	//

	//*******************************************************************************************************************
	//*																													*
	//*   On-Disk Image (ODI) structures																				*
	//*																													*
	//*******************************************************************************************************************

	//  GIF BLOCK LABEL RANGES
#define			GIF_XBLK_SIG		0x21									//  Extension Block Introducer
#define			GIF_IMG_SEP			0x2C									//  Image separator
#define			GIF_GRB_LO			0x00									//  Graphic Rendering Blocks
#define			GIF_GRB_HI			0x7F									//
#define			GIF_CB_LO			0x80									//  Control Blocks
#define			GIF_CB_HI			0xF9									//
#define			GIF_SPB_LO			0xFA									//  Special Purpose Blocks
#define			GIF_SPB_HI			0xFF									//

	//  GIF FILE HEADER
	typedef struct GIF_FILE_HEADER {
		char		signature[3];											//  File signature 'G','I','F'
		char		version[3];												//  GIF specification version '8','9','a'
	} GIF_FILE_HEADER;

	//  GIF FILE HEADER Signature Bytes
#define			GFH_SIG1	'G'												//  Signature byte #1
#define			GFH_SIG2	'I'												//  Signature byte #2
#define			GFH_SIG3	'F'												//  Signature byte #3

	//  GIF FILE HEADER Default Version (89a)
#define			GFH_DEF_V1	'8'												//  Default Version byte #1
#define			GFH_DEF_V2	'9'												//  Default Version byte #2
#define			GFH_DL_V2	'7'												//  Downlevel Version byte #2
#define			GFH_DEF_V3	'a'												//  Default Version byte #3

	//  LOGICAL SCREEN DESCRIPTOR
	typedef struct GIF_LOGICAL_SCREEN_DESCRIPTOR {
		BYTE				width[2];										//  Image width in pixels
		BYTE				height[2];										//  Image height in pixels
		BYTE				bitsettings;									//  Settings for this image file
		BYTE				background;										//  Background colour
		BYTE				pixelAspectRatio;								//  Pixel aspect ratio
	} GIF_LOGICAL_SCREEN_DESCRIPTOR;

	//  width & height
	//  Both width and height are encoded in little-endian form no matter what the host architecture is.
	//  Use the GetSize(field) and SetSize(field, value) macros to get and set these fields
#define		GetSize(x) ((x[1] * 256) + x[0])				//  Get width or height
#define		SetSize(x,y) {					\
							x[1] = BYTE(y/256);	\
							x[0] = BYTE(y % 256);	\
						 }

	//  Use the SetFlagsOn and SetFlagsOff macros to manipulate the bit setting fields
#define		SetFlagsOn(x,y) x = x | (y)
#define		SetFlagsOff(x,y) x = x & (~(y)) 

	//  bitsettings - field definitions
	//  Bit:    76543210
	//  Field:  12223444
	//
	//  Field:  1  -  Global Colour Table Flag
	//                If set then a Global Colour Table is present and immediately follows the logical screen descriptor
	//
	//  Field:  2  -  Colour Resolution Bits
	//                Number of primary colour bits minus one. Use the ColourBits macro to extract the value
	//
	//  Field:  3  -  Sort Flag
	//                If set then the Global Colour Table is sorted in decreasing order of frequency of use.
	//                If not set then the Global Colour Table is not sorted.
	//
	//  Field:  4  -  Size of the Global Colour Table (# Entries)
	//                If the value is n then the size is 2 ** (n + 1), use the ColourTableSize macro to extract the value
	//				  The size of the colour table in bytes can be extracted with the ColourTableSizeBytes macro.
	//
#define		GIF_HDRBITS_GCT			0x80											//  Global Colour Table is present
#define		GIF_HDRBITS_RESMASK		0x70											//  Colour Resolution bit mask
#define		GIF_HDRBITS_SORT		0x08											//  GCT is sorted
#define		GIF_HDRBITS_GCTSIZEMASK 0x07											//  GCT size bit mask
#define		ColourBits(x) (((x & GIF_HDRBITS_RESMASK) >> 4) + 1)					//  Primary colour bits (bitsettings field, number of bits)
#define		SetColourBits(x,y) x = x | (((y - 1) << 4) & GIF_HDRBITS_RESMASK)		//  Set the number of colour biits
#define		ColourTableSize(x) (size_t(2) << (x & GIF_HDRBITS_GCTSIZEMASK))							//  GCT/LCT Table size (# entries)
#define		ColourTableSizeBytes(x) ((size_t(2) << (x & GIF_HDRBITS_GCTSIZEMASK)) * sizeof(RGB))	//  GCT/LCT Table size (bytes)

	//  GLOBAL/LOCAL COLOUR TABLE
	typedef struct GIF_ODI_COLOUR_TABLE {
		RGB				Entry[256];													//  Colour Table Entries
	} GIF_ODI_COLOUR_TABLE;

	//  IMAGE DESCRIPTOR
	typedef struct GIF_IMAGE_DESCRIPTOR {
		BYTE			signature;													//  Block signature 0x2C
		BYTE			left[2];													//  Left position of this image
		BYTE			top[2];														//  Top position of this image
		BYTE			width[2];													//  Width of this image
		BYTE			height[2];													//  Height of this image
		BYTE			bitsettings;												//  Settings for this image
	} GIF_IMAGE_DESCRIPTOR;

	//  IMAGE DESCRIPTOR Signature Byte
#define		ID_SIG			0x2C													//  Signature byte

	//  left, top, width and height
	//  These fields are encoded in little-endian form no matter what the host architecture is.
	//  Use the GetSize(field) and SetSize(field, value) macros to get and set these fields

	//  bitsettings - field definitions
	//  Bit:    76543210
	//  Field:  12344555
	//
	//  Field:  1  -  Local Colour Table Flag
	//                If set then a Local Colour Table is present and immediately follows the image descriptor
	//
	//  Field:  2  -  Interlace Flag
	//				  If set then this image is encoded in a 4 pass interlace pattern
	//
	//  Field:  3  -  Sort Flag
	//                If set then the Local Colour Table is sorted in decreasing order of frequency of use.
	//                If not set then the Local Colour Table is not sorted.
	//
	//  Field:  4  -  Reserved (2 bits)
	//
	//  Field:  5  -  Size of the Local Colour Table (# Entries)
	//                If the value is n then the size is 2 ** (n + 1), use the ColourTableSize macro to extract the value
	//				  The size of the colour table in bytes can be extracted with the ColourTableSizeBytes macro.
	//
#define		GIF_IDBITS_LCT			0x80											//  Local Colour Table is present
#define		GIF_IDBITS_ILC			0x40											//  Uses 4 pass interlace pattern
#define		GIF_IDBITS_SORT			0x20											//  Local Colour table is sorted
#define		GIF_IDBITS_LCTSIZEMASK  0x07											//  LCT size bit mask

#define		LColourTableSize(x) (size_t(2) << (x & GIF_IDBITS_LCTSIZEMASK))			//  GCT/LCT Table size (# entries)
#define		LColourTableSizeBytes(x) ((size_t(2) << (x & GIF_IDBITS_LCTSIZEMASK)) * 3)		//  GCT/LCT Table size (bytes)

	//  GRAPHICS CONTROL EXTENSION
	typedef struct GIF_GRAPHICS_CONTROL_EXTENSION {
		BYTE			introducer;													//  Signature introducer - 0x21
		BYTE			label;														//  Signature label - 0xF9
		BYTE			size;														//  Extension data size
		BYTE			bitsettings;												//  Settings for this extension block
		BYTE			delay[2];													//  Display delay (animation)
		BYTE			transparent;												//  Transparent colour index
		BYTE			term;														//  Block terminator - 0x00
	} GIF_GRAPHICS_CONTROL_EXTENSION;

	//  GRAPHICS CONTROL EXTENSION Signature bytes
#define		GCE_SIG_INT			0x21												//  Signature introducer
#define		GCE_SIG_LAB			0xF9												//  Signature label

	//  delay
	//  The delay field is encoded in little-endian form no matter what the host architecture is.
	//  Use the GetSize(field) and SetSize(field, value) macros to get and set this field

	//  bitsettings - field definitions
	//  Bit:    76543210
	//  Field:  11122234
	//
	//  Field:  1  -  Reserved (3 bits)
	//
	//  Field:  2  -  Disposal method
	//                The following values are defined for the disposal method
	//
	//				  0  -  No disposal specified, take no action.
	//				  1  -  Do not dispose, graphic is left in place.
	//				  2  -  Restore to background colour.
	//				  3  -  Restore to previous image, revert.
	//
	//				  Use the DisposalMethod(field) macro to access the value
	//				  Use the SetDisposalMethod(field,method) macro to set the value
	//
	//  Field:  3  -  User Input Flag
	//                If set then wait for user input before disposing the image.
	//
	//  Field:  4  -  Transparency Flag
	//				  If set then a transparency color index is provided.
	//
#define		GIF_GCEBITS_DMMASK		0x1C											//  Mask for the Disposal Method
#define		GIF_GCEBITS_UIP			0x02											//  User input flag
#define		GIF_GCEBITS_TRANSP		0x01											//  Transparency flag
#define		DisposalMethod(x) ((x & GIF_GCEBITS_DMMASK) >> 2)						//  Disposal Method
#define		SetDisposalMethod(x,y) x = (x & (~GIF_GCEBITS_DMMASK)) | (y << 2)		//  Set the disposal method

	//  IMAGE DISPOSAL METHODS
#define		FRAME_DUSPOSE_NONE		0												//  Take no action
#define		FRAME_DISPOSE_LEAVE		1												//  Leave image in place
#define		FRAME_DISPOSE_RTB		2												//  Revert to background
#define		FRAME_DISPOSE_RESTORE	3												//  Restore the previous image

	//  IMAGE_BLOB
	typedef struct GIF_IMAGE_SEGMENT {
		BYTE			size;														//  Number of data bytes in the blob
		BYTE			data[255];													//  Imade data bytes
	} GIF_IMAGE_SEGMENT;
#define		EO_DATA_SIZE		0x00												//  Size for end of data blob

	//  GIF_ENTROPY_ENCODED_BUFFER
	typedef struct GIF_ENTROPY_ENCODED_BUFFER {
		BYTE			nativecodesize;												//  Native symbol code size
		GIF_IMAGE_SEGMENT	segment;												//  Compressed image data segments
	} GIF_ENTROPY_ENCODED_BUFFER;

	//  NETSCAPE APPLICATION BLOCK
	typedef struct GIF_NETSCAPE_APPLICATION_BLOCK
	{
		BYTE			introducer;													//  Signature introducer - 0x21
		BYTE			label;														//  Signature label - 0xFF
		BYTE			size;														//  Remaining data size 0x0B
		char			appname[8];													//  Application Name "NETSCAPE"
		char			auth[3];													//  Authentication code "2.0"
		BYTE			appdatasize;												//  Size of following application data 0x03
		BYTE			subblockindex;												//  Sub-Block index 0x01
		BYTE			repeats[2];													//  Number of repititions
		BYTE			endblock;													//  End of block 0x00
	} GIF_NETSCAPE_APPLICATION_BLOCK;

	//  GIF_FILE_TRAILER
	typedef struct GIF_FILE_TRAILER {
		BYTE			signature;													//  Signature 0x3B
	} GIF_FILE_TRAILER;
#define			GIF_SIG_END		0x3B												//  Signature for the file trailer 

	//
	//  GIF block types
	//
#define			GIF_BLOCK_FH	'F'													//  File Header (GIF_FILE_HEADEER)
#define			GIF_BLOCK_LSD	'L'													//  Logical Screen Descriptor (GIF_LOGICAL_SCREEN_DESCRIPTOR)
#define			GIF_BLOCK_GCT	'G'													//  Global Colour Table (GIF_ODI_COLOUR_TABLE)
#define			GIF_BLOCK_FT	'T'													//  File Trailer (GIF_FILE_TRAILER)
#define			GIF_BLOCK_GCE	'H'													//  Graphics Control Extension (GIF_GRAPHICS_CONTROL_EXTENSION)
#define			GIF_BLOCK_ID	'I'													//  Image Descriptor (GIF_IMAGE_DESCRIPTOR)
#define			GIF_BLOCK_LCT	'C'													//  Local Colour Table (GIF_ODI_COLOUR_TABLE)
#define			GIF_BLOCK_EEB	'E'													//  Entropy Encoded Buffer (GIF_ENTROPY_ENCODED_BUFFER)
#define			GIF_BLOCK_AXB	'X'													//  Application Extension Block (e.g. GIF_NETSCAPE_APPLICATION_BLOCK)

}
