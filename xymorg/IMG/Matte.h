#pragma once
//*******************************************************************************************************************
//*																													*
//*   File:       Matte.h																							*
//*   Suite:      xymorg Image Processing - primitives																*
//*   Version:    1.0.0	  Build:  01																				*
//*   Author:     Ian Tree/HMNL																						*
//*																													*
//*   Copyright 2017 - 2020 Ian J. Tree																				*
//*******************************************************************************************************************
//*	Matte.h																											*
//*																													*
//*	This header file contains the Class definition and implementation for the Matte primitive class.				*
//* The Matte class defines a spatial opacity filter for Blit operations.											*
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
#include	"RasterBuffer.h"																		//  Raster Buffer

//*******************************************************************************************************************
//*																											        *
//*   Matte Class																									*
//*                                                                                                                 *
//*   The Matte class defines a spatial opacity filter for Blit operations. The Matte is a Raster Buffer image		*
//*   of BYTES containing a percentage opacity.																		*
//*                                                                                                                 *
//*******************************************************************************************************************

namespace xymorg {

	class Matte : public xymorg::RasterBuffer<xymorg::BYTE> {
	public:

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Public Constants                                                                                              *
		//*                                                                                                                 *
		//*******************************************************************************************************************


		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Constructors                                                                                                  *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  default constructor
		//
		//  Constructs a new Matte object. The object has no height and width.
		//
		//  PARAMETERS
		//
		//
		//  RETURNS
		//
		//  NOTES
		//

		Matte() : RasterBuffer<BYTE>() {

			//  Return to caller
			return;
		}

		//  Normal Constructor
		//
		//  Constructs a fully-formed Matte, the opacity is set to 100% (transparent)
		//
		//  PARAMETERS
		//
		//		size_t			-		Image Height in pixels
		//		size_t			-		Image width in pixels
		//		BYTE			-		Default Opacity
		//
		//  RETURNS
		//
		//  NOTES
		//
		//		1.		If the specified height or width is zero then a valid but empty Matte is constructed.
		//

		Matte(const size_t H, const size_t W, BYTE DO) : RasterBuffer<BYTE>(H, W, &DO) {

			//  Return to caller
			return;
		}

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   destructor	                                                                                                *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		~Matte() {

			//  Return to caller
			return;
		}

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Public Functions                                                                                              *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  setOpacity
		//
		//  Sets the opacity at a given pixel position
		//
		//  PARAMETERS
		//
		//		size_t			-		Row index position of the desired pixel
		//		size_t			-		Column index position of the desired pixel
		//		double			-		Opacity (expressed as a fractional value 0.0 - 1.0)
		//
		//  RETURNS
		//
		//  NOTES
		//

		void		setOpacity(size_t R, size_t C, double Opacity) {
			BYTE			IOpacity = 0;

			//  Safety 
			if (Opacity < 0.0) Opacity = 0.0;
			if (Opacity > 1.0) Opacity = 1.0;

			IOpacity = BYTE(Opacity * 100.0);

			setPixel(R, C, IOpacity);

			//  Return to caller
			return;
		}

		//  getOpacity
		//
		//  Returns the opacity at a given pixel position
		//
		//  PARAMETERS
		//
		//		size_t			-		Row index position of the desired pixel
		//		size_t			-		Column index position of the desired pixel
		//
		//  RETURNS
		//
		//		double			-		Opacity (expressed as a fractional value 0.0 - 1.0)
		//
		//  NOTES
		//

		double		getOpacity(size_t R, size_t C) {
			return double(*getPixel(R, C)) / 100.0;
		}

		//  resetOpacity
		//
		//  Sets the opacity of all pixels to the given opacity
		//
		//  PARAMETERS
		//
		//		double			-		Opacity (expressed as a fractional value 0.0 - 1.0)
		//
		//  RETURNS
		//
		//  NOTES
		//

		void		resetOpacity(double Opacity) {
			BYTE			IOpacity = 0;

			//  Safety 
			if (Opacity < 0.0) Opacity = 0.0;
			if (Opacity > 1.0) Opacity = 1.0;

			IOpacity = BYTE(Opacity * 100.0);

			for (iterator MR = top(); MR != bottom(); MR++) {
				for (iterator MC = left(MR); MC != right(MR); MC++) *MC = IOpacity;
			}

			//  Return to caller
			return;
		}

		//  blit
		//
		//  This function will perform a BLock Image Transfer from the passed source image into the current image,
		//  while applying the opacity filter from the supplied Matte.
		//
		//  PARAMETERS
		//
		//		RasterBuffer&		-		Const reference to the source image
		//      RasterBuffer&		-		Reference to the target image
		//		size_t				-		Row target for the origin of the block in the target image
		//		size_t				-		Column target for the origin of the block in the target image
		//
		//  RETURNS
		//
		//  NOTES
		// 
		//   1.		The Matte MUST have the same dimensions as the source image.
		//

		void	blit(const RasterBuffer<RGB>& SrcImg, RasterBuffer<RGB>& TgtImg, size_t OR, size_t OC) {
			BoundingBox		Region = {};													//  Source region bounding box
			BoundingBox		TargetRegion = {};												//  Target region bounding box
			double			Opacity = 0.0;													//  Opacity of pixel

			//  Setup the bounding box to describe the complete image
			Region.Top = 0;
			Region.Left = 0;
			Region.Bottom = SrcImg.getHeight() - 1;
			Region.Right = SrcImg.getWidth() - 1;

			//  Check that the Matte has the same dimensions as the source image, if not a standard blit operation is performed.
			if (getHeight() != SrcImg.getHeight() || getWidth() != SrcImg.getWidth()) return TgtImg.blit(SrcImg, Region, OR, OC);

			//  Perform the copy through the opacity filter
			//  Safety
			if (SrcImg.getHeight() == 0 || SrcImg.getWidth() == 0 || SrcImg.getArray() == nullptr) return;
			if (Height == 0 || Width == 0 || Buffer == nullptr) return;
			if (Region.Top >= SrcImg.getHeight()) return;
			if (Region.Right >= SrcImg.getWidth()) return;
			if (OR >= Height) return;
			if (OC >= Width) return;
			if (Region.Bottom >= SrcImg.getHeight()) Region.Bottom = SrcImg.getHeight() - 1;
			if (Region.Right >= SrcImg.getWidth()) Region.Right = SrcImg.getWidth() - 1;

			//  Build the target region bounding box
			TargetRegion.Top = OR;
			TargetRegion.Left = OC;

			TargetRegion.Bottom = TargetRegion.Top + (Region.Bottom - Region.Top);
			TargetRegion.Right = TargetRegion.Left + (Region.Right - Region.Left);

			//  Reduce the size of the regions to fit within the target bounds
			while (TargetRegion.Bottom >= Height) {
				TargetRegion.Bottom--;
				Region.Bottom--;
			}

			while (TargetRegion.Right >= Width) {
				TargetRegion.Right--;
				Region.Right--;
			}

			//
			//  Perform a Row/Column iteration copy from the source region to the target region, observing the matte opacity.
			//  Note the opacity is treated as a binary filter no pass (0.0 - 0.5) or pass (0.5+).
			//

			RasterBuffer<RGB>::iterator	ITR = TgtImg.top(TargetRegion);

			for (RasterBuffer<RGB>::const_iterator ISR = SrcImg.top(Region); ISR != SrcImg.bottom(Region); ISR++) {
				RasterBuffer<RGB>::iterator	ITC = TgtImg.left(ITR);
				for (RasterBuffer<RGB>::const_iterator ISC = SrcImg.left(ISR); ISC != SrcImg.right(ISR); ISC++) {
					Opacity = getOpacity(ISR.getIndex(), ISC.getIndex());
					if (Opacity >= 0.5)	*ITC = *ISC;
					ITC++;
				}
				ITR++;
			}

			//  Return to caller
			return;
		}

	private:

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Private Members	                                                                                            *
		//*                                                                                                                 *
		//*******************************************************************************************************************


		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Private Functions                                                                                             *
		//*                                                                                                                 *
		//*******************************************************************************************************************

	};

}
