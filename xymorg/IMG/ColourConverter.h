#pragma once
//*******************************************************************************************************************
//*																													*
//*   File:       ColourConverter.h																					*
//*   Suite:      xymorg Image Processing - primitives																*
//*   Version:    1.0.0	  Build:  01																				*
//*   Author:     Ian Tree/HMNL																						*
//*																													*
//*   Copyright 2017 - 2020 Ian J. Tree																				*
//*******************************************************************************************************************
//*	ColourConverter.h																								*
//*																													*
//*	This header file contains the class definition for the ColourConverter class, the class provides colour			*
//* space mapping static functions.																					*
//*																													*
//*	NOTES:																											*
//*																													*
//*																													*
//*******************************************************************************************************************
//*																													*
//*   History:																										*
//*																													*
//*	1.0.0 - 04/08/2018   -  Initial version																			*
//*																													*
//*******************************************************************************************************************

//  Include basic xymorg headers
#include	"../LPBHdrs.h"																			//  Language and Platform base headers
#include	"../types.h"																			//  xymorg type definitions
#include	"../consts.h"																			//  xymorg constant definitions

//  Include xymorg image processing primitives
#include	"types.h"																				//  Image processing primitive types

namespace xymorg {

	//*******************************************************************************************************************
	//*																													*
	//*   ColourConverter																								*
	//*																													*
	//*	  The ColourConverter class contains static functions for mapping the pixel encoding of a single poiant to		*	
	//*   the encoding in a different colour space. Functions are also provided for mapping the contents of a			*
	//*   ChannelArray in one colour space to a ChannelArray in a different colour space.								*
	//*   The conversion functions all use integer arithmetic but the class also contains reference conversion			*
	//*   functions that perform the conversions accurately using floating point computation.							*
	//*																													*
	//*******************************************************************************************************************

	class ColourConverter {
	public:

		//  Prevent instantiation
		ColourConverter() = delete;

		//*******************************************************************************************************************
		//*																													*
		//*  Public Functions																								*
		//*																													*
		//*******************************************************************************************************************

		//  convertToRGB   (YCbCr -->  RGB)
		//
		//  Converts a pixel encoded in YCbCr to RGB
		//
		//  PARAMETERS
		//
		//		YCbCr&		-		Const reference to an YCbCr encoded pixel
		//
		//  RETURNS
		//
		//     	RGB			-		An RGB encoded pixel
		//
		//  NOTES
		//
		//		R = (Y + (1.402 * (Cr - 128)))									=    (Y + (180/128 * (Cr - 128)))
		//		G = (Y + (-0.33414 * (Cb - 128)) + (-0.71414 * (Cr - 128))		=	 (Y + (-43/128 * (Cb - 128)) + (-92/128 * (Cr - 128)))
		//		B = (Y + (1.772 * (Cb - 128)))									=	 (Y + (226/128 * (Cb - 128)))
		//

		static RGB		convertToRGB(const YCbCr& ycbcrIn) {
			RGB				rgbOut = {};														//  Output RGB value

			int				Accumulator;														//  Accumulator
			int				Term;																//  Term value

			//  Compute R  =  (Y + (180/128 * (Cr - 128)))
			Accumulator = int(ycbcrIn.Y);
			Term = int(ycbcrIn.Cr) - 128;
			Term = Term * 180;
			Term = Term / 128;
			Accumulator += Term;

			if (Accumulator < 0) Accumulator = 0;
			else {
				if (Accumulator > 255) Accumulator = 255;
			}

			rgbOut.R = BYTE(Accumulator);

			//  Compute G  =  (Y + (-43/128 * (Cb - 128)) + (-92/128 * (Cr - 128)))
			Accumulator = int(ycbcrIn.Y);
			Term = int(ycbcrIn.Cb) - 128;
			Term = Term * -43;
			Term = Term / 128;
			Accumulator += Term;

			Term = int(ycbcrIn.Cr) - 128;
			Term = Term * -92;
			Term = Term / 128;
			Accumulator += Term;

			if (Accumulator < 0) Accumulator = 0;
			else {
				if (Accumulator > 255) Accumulator = 255;
			}

			rgbOut.G = BYTE(Accumulator);

			//  Compute B = (Y + (226/128 * (Cb - 128)))
			Accumulator = int(ycbcrIn.Y);
			Term = int(ycbcrIn.Cb) - 128;
			Term = Term * 226;
			Term = Term / 128;
			Accumulator += Term;

			if (Accumulator < 0) Accumulator = 0;
			else {
				if (Accumulator > 255) Accumulator = 255;
			}

			rgbOut.B = BYTE(Accumulator);

			//  return converted value to caller
			return rgbOut;
		}

		//  convertToYCbCr   (RGB -->  YCbCr)
		//
		//  Converts a pixel encoded in RGB to YCbCr
		//
		//  PARAMETERS
		//
		//		RGB&		-		Const referencnce to an RGB encoded pixel
		//
		//  RETURNS
		//
		//		YCbCr		-		A YCbCr encoded pixel
		//
		//  NOTES
		//
		//	[Y,Cb,Cr]	=	[R,G,B] * [ 0.299   -0.16894    0.499813  ]         =  [RGB] * [  38/128   -22/128   64/128   ]
		//                            [ 0.587   -0.33167   -0.41853   ]                    [  75/128   -43/128  -54/128   ]
		//                            [ 0.114    0.50059   -0.08128   ]                    [  15/128    64/128  -10/128   ]
		//
		//		Chroma values are then shifted by +128 into the 0..255 output domain.
		//

		static YCbCr		convertToYCbCr(const RGB& rgbIn) {
			YCbCr			ycbcrOut = {};														//  Output YCbCr value
			int				Accumulator;														//  Accumulator
			int				Term;																//  Term value

			//  Compute Lumina (Y)  =  (38/128 * R) + (75/128 * G) + (15/128 * B)
			Term = int(rgbIn.R);
			Term = Term * 38;
			Term = Term / 128;
			Accumulator = Term;

			Term = int(rgbIn.G);
			Term = Term * 75;
			Term = Term / 128;
			Accumulator += Term;

			Term = int(rgbIn.B);
			Term = Term * 15;
			Term = Term / 128;
			Accumulator += Term;

			if (Accumulator < 0) Accumulator = 0;
			else {
				if (Accumulator > 255) Accumulator = 255;
			}

			ycbcrOut.Y = BYTE(Accumulator);

			//  Compute Blue Chroma (Cb) = ((-22/128 * R) + (-43/128 * G) + (64/128 * B)) + 128
			Term = int(rgbIn.R);
			Term = Term * -22;
			Term = Term / 128;
			Accumulator = Term;

			Term = int(rgbIn.G);
			Term = Term * -43;
			Term = Term / 128;
			Accumulator += Term;

			Term = int(rgbIn.B);
			Term = Term * 64;
			Term = Term / 128;
			Accumulator += Term;
			Accumulator += 128;

			if (Accumulator < 0) Accumulator = 0;
			else {
				if (Accumulator > 255) Accumulator = 255;
			}

			ycbcrOut.Cb = BYTE(Accumulator);

			//  Compute Red Chroma (Cr) = ((64/128 * R) + (-54/128 * G) + (-10/128 * B)) + 128
			Term = int(rgbIn.R);
			Term = Term * 64;
			Term = Term / 128;
			Accumulator = Term;

			Term = int(rgbIn.G);
			Term = Term * -54;
			Term = Term / 128;
			Accumulator += Term;

			Term = int(rgbIn.B);
			Term = Term * -10;
			Term = Term / 128;
			Accumulator += Term;
			Accumulator += 128;

			if (Accumulator < 0) Accumulator = 0;
			else {
				if (Accumulator > 255) Accumulator = 255;
			}

			ycbcrOut.Cr = BYTE(Accumulator);

			//  return the converted value to the caller
			return ycbcrOut;
		}

		//
		//  Reference Pixel Conversion Functions
		//

		//  referenceToYCbCr   (RGB --> YCbCr)
		//
		//  Converts a pixel encoded in RGB to YCbCr.
		//
		//  PARAMETERS
		//
		//		RGB&		-		Const Referencnce to an RGB encoded pixel
		//
		//  RETURNS
		//
		//		YCbCr		-		YCbCr encoded pixel
		//
		//  NOTES
		//
		//

		static YCbCr referenceToYCbCr(const RGB& rgbIn) {
			double			dR = 0.0, dG = 0.0, dB = 0.0;												//  RGB inputs
			double			Output = 0.0;																//  Output
			YCbCr			ycbcrOut = {};

			//  Convert incoming RGB to floating point
			dR = double(rgbIn.R);
			dG = double(rgbIn.G);
			dB = double(rgbIn.B);

			//  Compute lumina (Y)
			Output = 0.299 * dR;
			Output += 0.587 * dG;
			Output += 0.114 * dB;
			if (Output > 255.0) Output = 255.0;

			ycbcrOut.Y = BYTE(Output);

			//  Compute blue dufference (Cb)
			Output = 128.0;
			Output -= 0.168736 * dR;
			Output -= 0.331264 * dG;
			Output += 0.5 * dB;
			if (Output > 255.0) Output = 255.0;

			ycbcrOut.Cb = BYTE(Output);

			//  Compute the red difference (Cr)
			Output = 128.0;
			Output += 0.5 * dR;
			Output -= 0.418688 * dG;
			Output -= 0.081312 * dB;
			if (Output > 255.0) Output = 255.0;

			ycbcrOut.Cr = BYTE(Output);

			//  Return to caller
			return ycbcrOut;
		}

		//  referencrToRGB   (YCbCr --> RGB)
		//
		//  Converts a pixel encoded in YCbCr to RGB.
		//
		//  PARAMETERS
		//
		//		YCbCr&		-		Const reference to a YCbCr encoded pixel
		//
		//  RETURNS
		//
		//		RGB			-		RGB encoded pixel
		//
		//  NOTES
		//
		//
		static RGB referenceToRGB(const YCbCr& ycbcrIn) {
			double			dY = 0.0, dCb = 0.0, dCr = 0.0;															//  YCbCr inputs
			double			Output = 0.0;																			//  Output
			RGB				rgbOut = {};																			//  Output pixel

			//  Convert incoming YCbCr to floating point
			dY = double(ycbcrIn.Y);
			dCb = double(ycbcrIn.Cb);
			dCr = double(ycbcrIn.Cr);

			//  Compute the Red component
			Output = dY;
			Output += 1.402 * (dCr - 128.0);
			if (Output < 0.0) Output = 0.0;
			if (Output > 255.0) Output = 255.0;

			rgbOut.R = BYTE(Output);

			//  Compute the Green component
			Output = dY;
			Output -= 0.33414 * (dCb - 128.0);
			Output -= 0.71414 * (dCr - 128.0);
			if (Output < 0.0) Output = 0.0;
			if (Output > 255.0) Output = 255.0;

			rgbOut.G = BYTE(Output);

			//  Compute the Blue component
			Output = dY;
			Output += 1.772 * (dCb - 128.0);
			if (Output < 0.0) Output = 0.0;
			if (Output > 255.0) Output = 255.0;

			rgbOut.B = BYTE(Output);

			//  Return to caller
			return rgbOut;
		}

	};
}
