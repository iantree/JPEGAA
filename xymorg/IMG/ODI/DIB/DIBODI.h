#pragma once
//
//*******************************************************************************************************************
//*																													*
//*   File:       DIBODI.h																							*
//*   Suite:      xymorg Image Processing - ODI																		*
//*   Version:    1.0.0	  Build:  01																				*
//*   Author:     Ian Tree/HMNL																						*
//*																													*
//*   Copyright 2007 - 2020 Ian J. Tree																				*
//*******************************************************************************************************************
//*	DIBODI.h																										*
//*																													*
//*	This header file contains the definitions and structures that describe the contents of a BMP file.				*
//* (Device Independent Bitmap (DIB) or Bitmap BMP).																*
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
	//  DIB STRUCTURES
	//

	//  DIB File Header
	typedef struct DIB_FILE_HEADER
	{
		BYTE			Sig[2];																					//  Signature Bytes
		UINT			FSize;																					//  File size (bytes)
		USHORT			App1;																					//  Application specific (1)
		USHORT			App2;																					//  Application specific (2)
		UINT			PAOffset;																				//  Offset to the pixel array
	} DIB_FILE_HEADER;

#define				DEFAULT_DIB_FSIG1			'B'																//  Default file signature
#define				DEFAULT_DIB_FSIG2			'M'																//  Default file signature

//  Encoding descriptors
#define				DIB_ENC_RGB					0																//  Plain RGB
#define				DIB_ENC_RLE8				1																//  8bit Run Length Encoding
#define				DIB_ENC_RLE4				2																//  4bit Run Length Encoding
#define				DIB_ENC_BITFIELDS			3																//  Bitfields (masks) define the encoding
#define				DIB_ENC_RMASK				0x00FF0000														//  Red Mask
#define				DIB_ENC_GMASK				0x0000FF00														//  Green Channel mask
#define				DIB_ENC_BMASK				0x000000FF														//  Blue Channel mask
#define				DIB_ENC_AMASK				0xFF000000														//  Alpha Channel Mask

//  Resolutions
#define				DIB_RES_72DPI				2835															//  72 dpi resolution

	//  DIB Header
	typedef struct DIB_HEADER
	{
		UINT			DIBHdrSize;																				//  Size of the DIB header structure
		UINT			Width;																					//  Image width (pixels)
		UINT			Height;																					//  Image height (pixels)
		USHORT			Planes;																					//  Number of colour planes
		USHORT			BPP;																					//  Number of bits per pixel
		SWITCHES		BitSettings;																			//  Bit settings
		UINT			BMSize;																					//  Size of raw bit map data
		UINT			ResH;																					//  Horizontal resolution (pixels per meter)
		UINT			ResV;																					//  Vertical resolution (pixels per meter)
		UINT			Colours;																				//  Number of colours in the palette
		UINT			IColours;																				//  Number of important clours
	} DIB_HEADER;

	//  DIB Header Extension
	typedef struct DIB_HEADER_EXTENSION
	{
		UINT			RMask;																					//  Red channel mask
		UINT			GMask;																					//  Green channel mask
		UINT			BMask;																					//  Blue channel mask
		UINT			AMask;																					//  Alpha channel mask
		UINT			ColourSpace;																			//  Colour space specifier
		BYTE			EndPoints[36];																			//  CIE endpoints
		UINT			RGamma;																					//  Red Gamma
		UINT			GGamma;																					//  Green Gamma
		UINT			BGamma;																					//  Blue Gamma
	} DIB_HEADER_EXTENSION;

	//  Generic DIB Data Block
	typedef struct DIB_DATA_BLOCK {
		BYTE			Sig[2];																					//  Signature Bytes
	} DIB_DATA_BLOCK;

	//  RLE encoded block (run)
	typedef struct DIB_RLE_BLOCK {
		uint8_t			RunLen;																					//  Run Length
		uint8_t			Symbol;																					//  Symbol value
	} DIB_RLE_BLOCK;

#pragma pack (pop)

}
