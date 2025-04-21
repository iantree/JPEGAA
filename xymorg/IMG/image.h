#pragma once
//*******************************************************************************************************************
//*																													*
//*   File:       image.h																							*
//*   Suite:      xymorg Image Processing																			*
//*   Version:    1.0.0	(Build: 01) - Dev Build:  XDB-037															*
//*   Author:     Ian Tree/HMNL																						*
//*																													*
//*   Copyright 2010 - 2020 Ian J. Tree																				*
//*******************************************************************************************************************
//*																													*
//*	This header file vectors the appropriate xymorg image processing headers according to the requirements of		*
//* the application.																								*
//*																													*
//*	USAGE:																											*
//*																													*
//*																													*
//*	NOTES:																											*
//*																													*
//*	1.																												*
//*																													*
//*******************************************************************************************************************
//*																													*
//*   History:																										*
//*																													*
//*	1.0.0 -		02/12/2017	-	Initial Release																		*
//*																													*
//*******************************************************************************************************************/

//
//  Common types and constants
//

#include	"types.h"
#include	"consts.h"

//
//  Basic Components
//

#include	"Palette.h"															//  RGB Colour Palettes
#include	"RasterBuffer.h"													//  Primitive Raster Buffer
#include	"ColourTable.h"														//  Colour Table (array)
#include	"Frame.h"															//  Frame of an image
#include	"Train.h"															//  Train of Frames making up an image
#include	"Draw.h"															//  Drawing primitives
#include	"Matte.h"															//  Matte class

//  Optional ODI Components
#ifdef XY_NEEDS_GIF
#include	"ODI/GIF/GIF.h"														//  GIF Image format
#endif

#ifdef XY_NEEDS_DIB
#include	"ODI/DIB/DIB.h"														//  DIB Image format
#endif

#ifdef XY_NEEDS_BMP
#include	"ODI/DIB/DIB.h"														//  DIB Image format
#endif

#ifdef XY_NEEDS_JFIF
#include	"ODI/JFIF/JFIF.h"													//  JFIF Image format
#endif

#ifdef XY_NEEDS_JPEG
#include	"ODI/JFIF/JFIF.h"													//  JPEG image format
#endif
