#pragma once
//*******************************************************************************************************************
//*																													*
//*   File:       Draw.h																							*
//*   Suite:      xymorg Image Processing - primitives																*
//*   Version:    1.0.1	  Build:  02																				*
//*   Author:     Ian Tree/HMNL																						*
//*																													*
//*   Copyright 2017 - 2024 Ian J. Tree																				*
//*******************************************************************************************************************
//*	Draw.h																											*
//*																													*
//*	This header file contains the Class definition and implementation for the Draw primitive class.					*
//* The Draw class exposes the static drawing primitive functions.													*
//*																													*
//*	NOTES:																											*
//*																													*
//*																													*
//*******************************************************************************************************************
//*																													*
//*   History:																										*
//*																													*
//*	1.0.0 - 04/08/2018   -  Initial version																			*
//*	1.0.1 - 08/12/2024   -  Winter Cleanup																			*
//*																													*
//*******************************************************************************************************************

//  Include basic xymorg headers
#include	"../LPBHdrs.h"																			//  Language and Platform base headers
#include	"../types.h"																			//  xymorg type definitions
#include	"../consts.h"																			//  xymorg constant definitions

//  Include xymorg image processing primitives
#include	"types.h"																				//  Image processing primitive types
#include	"RasterBuffer.h"																		//  Raster Buffer
#include	"Matte.h"																				//  Matte buffer

//*******************************************************************************************************************
//*																											        *
//*   Draw Class																									*
//*                                                                                                                 *
//*   The Draw class exposes the static drawing primitive functions.												*
//*                                                                                                                 *
//*******************************************************************************************************************

namespace xymorg {

	class Draw {
	public:

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Public Constants                                                                                              *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		static const int		ANTI_ALIAS_SATURATION = 50;											//  50% Saturation for anti-aliasing

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Constructors                                                                                                  *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  Prevent Instantiation
		Draw() = delete;

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Public Functions                                                                                              *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  drawLine
		//
		//  Draws a line segment
		//
		//  PARAMETERS
		//
		//		RasterBuffer<RGB>&	-		Reference to the Canvas to draw on
		//		LineSegment&		-		Reference to the line segment to be drawn
		//		RGB&				-		Reference to the colour to draw the line
		//
		//  RETURNS
		//
		//  NOTES
		//

		static void		drawLine(RasterBuffer<RGB>& Canvas, LineSegment& LS, const RGB& Colour) {
			return drawLine(Canvas, LS, Colour, 1.0);
		}

		//  drawLine
		//
		//  Draws a line segment
		//
		//  PARAMETERS
		//
		//		RasterBuffer<RGB>&	-		Reference to the Canvas to draw on
		//		LineSegment&		-		Reference to the line segment to be drawn
		//		RGB&				-		Reference to the colour to draw the line
		//		double				-		Mixing quotient for drawing (saturation)
		//
		//  RETURNS
		//
		//  NOTES
		//

		static void		drawLine(RasterBuffer<RGB>& Canvas, LineSegment& LS, const RGB& Colour, double mix) {
			LineSegment			Extra = LS;														//  Extra Line segments for widening the line
			bool				isSteep = false;												//  Steep line control
			BoundingBox			bbFill = {};													//  Fill extents

			//
			//  If the line is parallel to the canvas edges convert the line to a fill operation
			//
			if (LS.OriginR == LS.TargetR || LS.OriginC == LS.TargetC) {

				if (LS.OriginR == LS.TargetR) {
					//  Horizontal Line
					bbFill.Top = LS.OriginR - (LS.Width / 2);
					bbFill.Bottom = bbFill.Top + (LS.Width - 1);
					if (LS.OriginC < LS.TargetC) {
						bbFill.Left = LS.OriginC;
						bbFill.Right = LS.TargetC;
					}
					else {
						bbFill.Right = LS.OriginC;
						bbFill.Left = LS.TargetC;
					}
				}
				else {
					//  Vertical Line
					bbFill.Left = LS.OriginC - (LS.Width / 2);
					bbFill.Right = bbFill.Left + (LS.Width - 1);
					if (LS.OriginR < LS.TargetR) {
						bbFill.Top = LS.OriginR;
						bbFill.Bottom = LS.TargetR;
					}
					else {
						bbFill.Bottom = LS.OriginR;
						bbFill.Top = LS.TargetR;
					}
				}

				//  Fill the extents
				fill(Canvas, bbFill, Colour, mix);

				//  Return to caller
				return;
			}

			//  First draw the centre line segment
			Extra.Width = 1;
			drawLineSegment(Canvas, Extra, Colour, mix);

			//  If the line was a single pixel then we are done
			if (LS.Width == 1) return;

			//  Determine if the line is steep (i.e. must be widened vertically rather than horizontally)
			if (abs(double(LS.TargetR) - double(LS.OriginR)) > abs(double(LS.TargetC) - double(LS.OriginC))) isSteep = true;

			//  Widen the line until the required width is achieved
			for (size_t ExtraLines = 1; ExtraLines < LS.Width; ExtraLines++) {
				Extra = LS;
				Extra.Width = 1;
				if (ExtraLines & 1) {
					//  Extra line is above or to the left
					if (isSteep) {
						Extra.OriginC -= (ExtraLines / 2) + 1;
						Extra.TargetC -= (ExtraLines / 2) + 1;
					}
					else {
						Extra.OriginR -= (ExtraLines / 2) + 1;
						Extra.TargetR -= (ExtraLines / 2) + 1;
					}
					drawLineSegment(Canvas, Extra, Colour, mix);
				}
				else {
					//  Extra line is below or to the right
					if (isSteep) {
						Extra.OriginC += (ExtraLines / 2);
						Extra.TargetC += (ExtraLines / 2);
					}
					else {
						Extra.OriginR += (ExtraLines / 2);
						Extra.TargetR += (ExtraLines / 2);
					}
					drawLineSegment(Canvas, Extra, Colour, mix);
				}
			}

			//  Return to caller
			return;
		}

		//  drawLine
		//
		//  Draws a line segment on a Matte with the designated opacity
		//
		//  PARAMETERS
		//
		//		Matte&				-		Reference to the Matte to draw on
		//		LineSegment&		-		Reference to the line segment to be drawn
		//		double				-		Mixing quotient for drawing (saturation)
		//
		//  RETURNS
		//
		//  NOTES
		//

		static void		drawLine(Matte& CM, LineSegment& LS, double Opacity) {
			LineSegment			Extra = LS;														//  Extra Line segments for widening the line
			bool				isSteep = false;												//  Steep line control
			BoundingBox			bbFill = {};													//  Fill extents

			//
			//  If the line is parallel to the canvas edges convert the line to a fill operation
			//
			if (LS.OriginR == LS.TargetR || LS.OriginC == LS.TargetC) {

				if (LS.OriginR == LS.TargetR) {
					//  Horizontal Line
					bbFill.Top = LS.OriginR - (LS.Width / 2);
					bbFill.Bottom = bbFill.Top + (LS.Width - 1);
					if (LS.OriginC < LS.TargetC) {
						bbFill.Left = LS.OriginC;
						bbFill.Right = LS.TargetC;
					}
					else {
						bbFill.Right = LS.OriginC;
						bbFill.Left = LS.TargetC;
					}
				}
				else {
					//  Vertical Line
					bbFill.Left = LS.OriginC - (LS.Width / 2);
					bbFill.Right = bbFill.Left + (LS.Width - 1);
					if (LS.OriginR < LS.TargetR) {
						bbFill.Top = LS.OriginR;
						bbFill.Bottom = LS.TargetR;
					}
					else {
						bbFill.Bottom = LS.OriginR;
						bbFill.Top = LS.TargetR;
					}
				}

				//  Fill the extents
				fill(CM, bbFill, Opacity);

				//  Return to caller
				return;
			}

			//  First draw the centre line segment
			Extra.Width = 1;
			drawLineSegment(CM, Extra, Opacity);

			//  If the line was a single pixel then we are done
			if (LS.Width == 1) return;

			//  Determine if the line is steep (i.e. must be widened vertically rather than horizontally)
			if (abs(double(LS.TargetR) - double(LS.OriginR)) > abs(double(LS.TargetC) - double(LS.OriginC))) isSteep = true;

			//  Widen the line until the required width is achieved
			for (size_t ExtraLines = 1; ExtraLines < LS.Width; ExtraLines++) {
				Extra = LS;
				Extra.Width = 1;
				if (ExtraLines & 1) {
					//  Extra line is above or to the left
					if (isSteep) {
						Extra.OriginC -= (ExtraLines / 2) + 1;
						Extra.TargetC -= (ExtraLines / 2) + 1;
					}
					else {
						Extra.OriginR -= (ExtraLines / 2) + 1;
						Extra.TargetR -= (ExtraLines / 2) + 1;
					}
					drawLineSegment(CM, Extra, Opacity);
				}
				else {
					//  Extra line is below or to the right
					if (isSteep) {
						Extra.OriginC += (ExtraLines / 2);
						Extra.TargetC += (ExtraLines / 2);
					}
					else {
						Extra.OriginR += (ExtraLines / 2);
						Extra.TargetR += (ExtraLines / 2);
					}
					drawLineSegment(CM, Extra, Opacity);
				}
			}

			//  Return to caller
			return;
		}

		//  drawAALine
		//
		//  Draws a line segment with anti-aliasing
		//
		//  PARAMETERS
		//
		//		RasterBuffer<RGB>&	-		Reference to the Canvas to draw on
		//		LineSegment&		-		Reference to the line segment to be drawn
		//		RGB&				-		Reference to the colour to draw the line
		//
		//  RETURNS
		//
		//  NOTES
		//

		static void		drawAALine(RasterBuffer<RGB>& Canvas, LineSegment& LS, const RGB& Colour) {
			LineSegment			Extra = LS;														//  Extra Line segments for widening the line
			bool				isSteep = false;												//  Steep line control
			double				AASat = double(ANTI_ALIAS_SATURATION) / 100.0;					//  Saturation

			//  First draw the line as requested by the caller
			drawLine(Canvas, LS, Colour, 1.0);

			//  Determine if the line is steep (i.e. must be widened vertically rather than horizontally)
			if (abs(double(LS.TargetR) - double(LS.OriginR)) > abs(double(LS.TargetC) - double(LS.OriginC))) isSteep = true;

			//  Generate an additional line above/left and below/right to be drawn at 50% saturation
			if (isSteep) {
				Extra = LS;
				Extra.Width = 1;
				Extra.OriginC -= ((LS.Width / 2) + 1);
				Extra.TargetC -= ((LS.Width / 2) + 1);
				drawLineSegment(Canvas, Extra, Colour, AASat);
				Extra = LS;
				Extra.OriginC += ((LS.Width / 2) + 1);
				Extra.TargetC += ((LS.Width / 2) + 1);
				drawLineSegment(Canvas, Extra, Colour, AASat);
			}
			else {
				Extra = LS;
				Extra.Width = 1;
				Extra.OriginR -= ((LS.Width / 2) + 1);
				Extra.TargetR -= ((LS.Width / 2) + 1);
				drawLineSegment(Canvas, Extra, Colour, AASat);
				Extra = LS;
				Extra.OriginR += ((LS.Width / 2) + 1);
				Extra.TargetR += ((LS.Width / 2) + 1);
				drawLineSegment(Canvas, Extra, Colour, AASat);
			}

			//  Return to caller
			return;
		}

		//  drawAALine
		//
		//  Draws a line segment with anti-aliasing on a Matte
		//
		//  PARAMETERS
		//
		//		RasterBuffer<RGB>&	-		Reference to the Canvas to draw on
		//		LineSegment&		-		Reference to the line segment to be drawn
		//		double				-		Opacity
		//
		//  RETURNS
		//
		//  NOTES
		//

		static void		drawAALine(Matte& CM, LineSegment& LS, double Opacity) {
			LineSegment		ALS = LS;												//  Adjusted Line Segment

			//  Adjust for the anti-aliasing lines
			ALS.Width += 2;

			//  Draw a normal line segment with the designated opacity
			drawLine(CM, LS, Opacity);

			//  Return to caller
			return;
		}

		//  drawFadingLine
		//
		//  Draws a line segment that fades from full saturation to a zero saturation
		//
		//  PARAMETERS
		//
		//		RasterBuffer<RGB>&	-		Reference to the Canvas to draw on
		//		LineSegment&		-		Reference to the line segment to be drawn
		//		RGB&				-		Reference to the colour to draw the line
		//
		//  RETURNS
		//
		//  NOTES
		//

		static void		drawFadingLine(RasterBuffer<RGB>& Canvas, LineSegment& LS, const RGB& Colour) {
			return drawFadingLine(Canvas, LS, Colour, 1.0, 0.0);
		}

		//  drawFadingLine
		//
		//  Draws a line segment that fades from an initial to a final saturation
		//
		//  PARAMETERS
		//
		//		RasterBuffer<RGB>&	-		Reference to the Canvas to draw on
		//		LineSegment&		-		Reference to the line segment to be drawn
		//		RGB&				-		Reference to the colour to draw the line
		//		double				-		Mixing ratio at start of fade
		//		double				-		Mixing ratio at end of fade
		//
		//  RETURNS
		//
		//  NOTES
		//

		static void		drawFadingLine(RasterBuffer<RGB>& Canvas, LineSegment& LS, const RGB& Colour, double imix, double fmix) {
			LineSegment			Extra = LS;														//  Extra Line segments for widening the line
			bool				isSteep = false;												//  Steep line control

			//  First draw the centre line segment
			Extra.Width = 1;
			drawFadingLineSegment(Canvas, Extra, Colour, imix, fmix);

			//  If the line was a single pixel then we are done
			if (LS.Width == 1) return;

			//  Determine if the line is steep (i.e. must be widened vertically rather than horizontally)
			if (abs(double(LS.TargetR) - double(LS.OriginR)) > abs(double(LS.TargetC) - double(LS.OriginC))) isSteep = true;

			//  Widen the line until the required width is achieved
			for (size_t ExtraLines = 1; ExtraLines < LS.Width; ExtraLines++) {
				Extra = LS;
				Extra.Width = 1;
				if (ExtraLines & 1) {
					//  Extra line is above or to the left
					if (isSteep) {
						Extra.OriginC -= ((ExtraLines / 2) + 1);
						Extra.TargetC -= ((ExtraLines / 2) + 1);
					}
					else {
						Extra.OriginR -= (ExtraLines / 2) + 1;
						Extra.TargetR -= (ExtraLines / 2) + 1;
					}
					drawFadingLineSegment(Canvas, Extra, Colour, imix, fmix);
				}
				else {
					//  Extra line is below or to the right
					if (isSteep) {
						Extra.OriginC += (ExtraLines / 2);
						Extra.TargetC += (ExtraLines / 2);
					}
					else {
						Extra.OriginR += (ExtraLines / 2);
						Extra.TargetR += (ExtraLines / 2);
					}
					drawFadingLineSegment(Canvas, Extra, Colour, imix, fmix);
				}
			}

			//  Return to caller
			return;
		}

		//  drawCircle
		//
		//  Draws a Circle
		//
		//  PARAMETERS
		//
		//		RasterBuffer<RGB>&	-		Reference to the Canvas to draw on
		//		Circle&				-		Reference to the Circle descriptor to be drawn
		//		RGB&				-		Reference to the colour to draw the line
		//
		//  RETURNS
		//
		//  NOTES
		//

		static void		drawCircle(RasterBuffer<RGB>& Canvas, Circle& CD, const RGB& Colour) {
			return drawCircle(Canvas, CD, Colour, 1.0);
		}

		//  drawCircle
		//
		//  Draws a Circle using the Bresenham integer method.
		//
		//  PARAMETERS
		//
		//		RasterBuffer<RGB>&	-		Reference to the Canvas to draw on
		//		Circle&				-		Reference to the Circle descriptor to be drawn
		//		RGB&				-		Reference to the colour to draw the line
		//		double				-		Saturation to draw the circle
		//
		//  RETURNS
		//
		//  NOTES
		//

		static void		drawCircle(RasterBuffer<RGB>& Canvas, Circle& CD, const RGB& Colour, double mix) {
			int			refX = 0, refY = int(CD.Radius);										//  Plot point in the North -> North-East (primary segment)
			int			Decider = int(3) - int(2 * CD.Radius);									//  Decider value
			Circle		Extra = CD;																//  Additional circles to accomodate width
			size_t		Width = 1;																//  Width counter

			//  Draw the initial points
			propogateCircle(Canvas, CD, Colour, mix, refX, refY);

			//  Calculate the next points along the primary segment
			while (refY >= refX) {
				refX++;
				if (Decider > 0) {
					refY--;
					Decider = Decider + 4 * (refX - refY) + 10;
				}
				else Decider = Decider + 4 * refX + 6;

				//  Draw the next points in all segments
				propogateCircle(Canvas, CD, Colour, mix, refX, refY);
			}

			//  Draw concentric circles at decreasing radius until the width is achieved
			Extra.Width = 1;
			while (Width < CD.Width) {
				Extra.Radius--;
				drawCircle(Canvas, Extra, Colour, mix);
				Width++;
			}

			//  Return to caller
			return;
		}

		//  drawShadingCircle
		//
		//  Draws a single pixel Circle using the Bresenham integer method, the colour is determined by a probability TriColour
		//
		//  PARAMETERS
		//
		//		RasterBuffer<RGB>&	-		Reference to the Canvas to draw on
		//		Circle&				-		Reference to the Circle descriptor to be drawn
		//		TriColour&			-		Reference to the TriColour colour selector
		//		PRNG&				-		Reference to the Random Number source
		//
		//  RETURNS
		//
		//  NOTES
		//

		static void		drawShadingCircle(RasterBuffer<RGB>& Canvas, Circle& CD, TriColour& CS, PRNG& RGen) {
			int			refX = 0, refY = int(CD.Radius);										//  Plot point in the North -> North-East (primary segment)
			int			Decider = int(3) - int(2 * CD.Radius);									//  Decider value

			//  Draw the initial points
			propogateShadingCircle(Canvas, CD, CS, RGen, refX, refY);

			//  Calculate the next points along the primary segment
			while (refY >= refX) {
				refX++;
				if (Decider > 0) {
					refY--;
					Decider = Decider + 4 * (refX - refY) + 10;
				}
				else Decider = Decider + 4 * refX + 6;

				//  Draw the next points in all segments
				propogateShadingCircle(Canvas, CD, CS, RGen, refX, refY);
			}

			//  Return to caller
			return;
		}

		//  intersectCircle
		//
		//  This function will compute the Row/Column intercept of a circle with a radius at an angle to the vertical
		//
		//	PARAMETERS:
		//
		//		Circle&			-		Reference to the Circle definition
		//		size_t&			-		Degrees from the vertical 
		//		size_t&			-		Reference to Intercept (Row)
		//		size_t&			-		Reference to Intercept (Column)
		//
		//	RETURNS:
		//
		//	NOTES:
		//

		static void		intersectCircle(Circle& CD, size_t Degrees, size_t& Row, size_t& Column) {
			size_t		EffDeg = Degrees % 360;														//  Max 360
			size_t		Sector = EffDeg / 45;														//  Mapping sector
			size_t		DFV = EffDeg % 45;															//  Degrees from vertical
			double		DR = 0.0, DC = 0.0;															//  Delta row and column

			if (Sector & 1) DFV = 45 - DFV;
			
			//  Calculate the delta in the first segment
			DR = floor(double(CD.Radius) * cos(RADS(DFV)) + 0.5);
			DC = floor(double(CD.Radius) * sin(RADS(DFV)) + 0.5);
	
			//  Convert to points in the input sector
			switch (Sector) {
			case 0:
				Row = CD.OriginR - size_t(DR);
				Column = CD.OriginC + size_t(DC);
				break;

			case 1:
				Row = CD.OriginR - size_t(DC);
				Column = CD.OriginC + size_t(DR);
				break;

			case 2:
				Row = CD.OriginR + size_t(DC);
				Column = CD.OriginC + size_t(DR);
				break;

			case 3:
				Row = CD.OriginR + size_t(DR);
				Column = CD.OriginC + size_t(DC);
				break;

			case 4:
				Row = CD.OriginR + size_t(DR);
				Column = CD.OriginC - size_t(DC);
				break;

			case 5:
				Row = CD.OriginR + size_t(DC);
				Column = CD.OriginC - size_t(DR);
				break;

			case 6:
				Row = CD.OriginR - size_t(DC);
				Column = CD.OriginC - size_t(DR);
				break;

			case 7:
				Row = CD.OriginR - size_t(DR);
				Column = CD.OriginC - size_t(DC);
				break;
			}
			
			//  Return to caller
			return;
		}

		//  drawAACircle
		//
		//  Draws a Circle with anti-aliasing
		//
		//  PARAMETERS
		//
		//		RasterBuffer<RGB>&	-		Reference to the Canvas to draw on
		//		Circle&				-		Reference to the Circle descriptor to be drawn
		//		RGB&				-		Reference to the colour to draw the line
		//
		//  RETURNS
		//
		//  NOTES
		//

		static void		drawAACircle(RasterBuffer<RGB>& Canvas, Circle& CD, const RGB& Colour) {
			Circle				Extra = CD;														//  Additional Circles
			double				AASat = double(ANTI_ALIAS_SATURATION) / 100.0;					//  Saturation

			//  First draw the requested circle
			drawCircle(Canvas, CD, Colour, 1.0);

			//
			//  Now draw two additional circles one inside and one outside at the anti-alias saturation
			//

			Extra.Radius = Extra.Radius + 1;
			Extra.Width = 1;
			drawCircle(Canvas, Extra, Colour, AASat);

			Extra.Radius = CD.Radius - CD.Width;
			Extra.Width = 1;
			drawCircle(Canvas, Extra, Colour, AASat);

			//  Return to caller
			return;
		}

		//  fill
		//
		//  Fills an area with the passed colour
		//
		//  PARAMETERS
		//
		//		RasterBuffer<RGB>&	-		Reference to the Canvas to draw on
		//		BoundingBox&		-		Reference to the area to be filled
		//		RGB&				-		Reference to the colour to draw the line
		//
		//  RETURNS
		//
		//  NOTES
		//

		static void		fill(RasterBuffer<RGB>& Canvas, BoundingBox& Area, const RGB& Colour) {

			//  Saturation is 100%
			return fill(Canvas, Area, Colour, 1.0);
		}

		//  fill
		//
		//  Fills an area with the passed colour at the given saturation
		//
		//  PARAMETERS
		//
		//		RasterBuffer<RGB>&	-		Reference to the Canvas to draw on
		//		BoundingBox&		-		Reference to the area to be filled
		//		RGB&				-		Reference to the colour to draw the line
		//		double				-		Mixing quotient for drawing (saturation)
		//
		//  RETURNS
		//
		//  NOTES
		//

		static void		fill(RasterBuffer<RGB>& Canvas, BoundingBox& Area, const RGB& Colour, double mix) {
			BoundingBox		NArea = Area;															//  Normalised area

			//  Normalise the bounds
			if (NArea.Top > NArea.Bottom) {
				NArea.Top = Area.Bottom;
				NArea.Bottom = Area.Top;
				NArea.Left = Area.Right;
				NArea.Right = Area.Left;
			}
			else {
				if (NArea.Top == NArea.Bottom) {
					if (NArea.Left > NArea.Right) {
						NArea.Left = Area.Right;
						NArea.Right = Area.Left;
					}
				}
			}
			
			//  Iterate over the fill area
			for (RasterBuffer<RGB>::iterator RIt = Canvas.top(NArea); RIt != Canvas.bottom(NArea); RIt++) {
				for (RasterBuffer<RGB>::iterator CIt = Canvas.left(RIt); CIt != Canvas.right(RIt); CIt++) {
					if (mix == 1.0) *CIt = Colour;
					else Canvas.setPixel(RIt.getIndex(), CIt.getIndex(), Colour, mix);
				}
			}

			//  Return to caller
			return;
		}

		//  fill
		//
		//  Fills an area of a Matte with the designated opacity
		//
		//  PARAMETERS
		//
		//		Matte&				-		Reference to the Matte to draw on
		//		BoundingBox&		-		Reference to the area to be filled
		//		double				-		Opacity
		//
		//  RETURNS
		//
		//  NOTES
		//

		static void		fill(Matte& CM, BoundingBox& Area, double Opacity) {
			BoundingBox		NArea = Area;															//  Normalised area

			//  Normalise the bounds
			if (NArea.Top > NArea.Bottom) {
				NArea.Top = Area.Bottom;
				NArea.Bottom = Area.Top;
				NArea.Left = Area.Right;
				NArea.Right = Area.Left;
			}
			else {
				if (NArea.Top == NArea.Bottom) {
					if (NArea.Left > NArea.Right) {
						NArea.Left = Area.Right;
						NArea.Right = Area.Left;
					}
				}
			}

			//  Iterate over the fill area
			for (Matte::iterator RIt = CM.top(NArea); RIt != CM.bottom(NArea); RIt++) {
				for (Matte::iterator CIt = CM.left(RIt); CIt != CM.right(RIt); CIt++) {
					CM.setOpacity(RIt.getIndex(), CIt.getIndex(), Opacity);
				}
			}

			//  Return to caller
			return;
		}

		//  drawBox
		//
		//  Draws an empty  or filled box at the given position, with the given pen colour and width
		//
		//  PARAMETERS
		//
		//		RasterBuffer<RGB>&	-		Reference to the Canvas to draw on
		//		BoundingBox&		-		Reference to the box co-ordintes to be drawn
		//		size_t				-		Width of the outline
		//		RGB&				-		Reference to the colour to draw the outline
		//		RGB*				-		Pointer to the fill colour
		//
		//  RETURNS
		//
		//  NOTES
		//

		static void		drawBox(RasterBuffer<RGB>& Canvas, BoundingBox& Outline, size_t Width, const RGB& Pen, const RGB* Fill) {
			return drawBox(Canvas, Outline, Width, Pen, Fill, 1.0);
		}

		//  drawBox
		//
		//  Draws an empty or filled box at the given position, with the given pen colour and width and saturation.
		//
		//  PARAMETERS
		//
		//		RasterBuffer<RGB>&	-		Reference to the Canvas to draw on
		//		BoundingBox&		-		Reference to the box co-ordinates to be drawn
		//		size_t				-		Width of the outline
		//		RGB&				-		Reference to the colour to draw the outline
		//		RGB*				-		Pointer to the fill colour
		//		double				-		Saturation
		//
		//  RETURNS
		//
		//  NOTES
		//

		static void		drawBox(RasterBuffer<RGB>& Canvas, BoundingBox& Outline, size_t Width, const RGB& Pen, const RGB* Fill, double mix) {
			BoundingBox		Box = Outline;													//  Box Outline to draw
			LineSegment		Edge = {};														//  Line segment to draw

			//  Safety
			if (Width == 0) return;

			//  Recursively draw smaller boxes until the width is 1
			while (Width > 1) {
				Box.Bottom = Box.Bottom - 1;
				Box.Left = Box.Left + 1;
				Box.Right = Box.Right - 1;
				Box.Top = Box.Top + 1;
				drawBox(Canvas, Box, 1, Pen, nullptr, mix);
				Width--;
			}

			//  Draw the outline (single pixel)
			Edge.OriginR = Outline.Top;
			Edge.TargetR = Outline.Top;
			Edge.OriginC = Outline.Left;
			Edge.TargetC = Outline.Right;
			Edge.Width = 1;
			drawLine(Canvas, Edge, Pen, mix);

			Edge.OriginR = Outline.Top;
			Edge.TargetR = Outline.Bottom;
			Edge.OriginC = Outline.Right;
			Edge.TargetC = Outline.Right;
			Edge.Width = 1;
			drawLine(Canvas, Edge, Pen, mix);

			Edge.OriginR = Outline.Bottom;
			Edge.TargetR = Outline.Bottom;
			Edge.OriginC = Outline.Left;
			Edge.TargetC = Outline.Right;
			Edge.Width = 1;
			drawLine(Canvas, Edge, Pen, mix);

			Edge.OriginR = Outline.Top;
			Edge.TargetR = Outline.Bottom;
			Edge.OriginC = Outline.Left;
			Edge.TargetC = Outline.Left;
			Edge.Width = 1;
			drawLine(Canvas, Edge, Pen, mix);

			//  If there is a fill colour specified then fill the innermost box
			if (Fill != nullptr) {
				Box.Bottom = Box.Bottom - 1;
				Box.Left = Box.Left + 1;
				Box.Right = Box.Right - 1;
				Box.Top = Box.Top + 1;
				fill(Canvas, Box, *Fill, mix);
			}
	
			//  Return to caller
			return;
		}

		//  drawDiamond
		//
		//  Draws an empty or filled diamond at the given position, with the given pen colour and width and saturation.
		//
		//  PARAMETERS
		//
		//		RasterBuffer<RGB>&	-		Reference to the Canvas to draw on
		//		BoundingBox&		-		Reference to the diamond containing box co-ordinates to be drawn
		//		size_t				-		Width of the outline
		//		RGB&				-		Reference to the colour to draw the outline
		//		RGB*				-		Pointer to the fill colour
		//
		//  RETURNS
		//
		//  NOTES
		//

		static void		drawDiamond(RasterBuffer<RGB>& Canvas, BoundingBox& Outline, size_t Width, const RGB& Pen, const RGB* Fill) {

			//  Return Diamond at 100% saturation
			return drawDiamond(Canvas, Outline, Width, Pen, Fill, 1.0);
		}

		//  drawDiamond
		//
		//  Draws an empty or filled diamond at the given position, with the given pen colour and width and saturation.
		//
		//  PARAMETERS
		//
		//		RasterBuffer<RGB>&	-		Reference to the Canvas to draw on
		//		BoundingBox&		-		Reference to the diamond containing box co-ordinates to be drawn
		//		size_t				-		Width of the outline
		//		RGB&				-		Reference to the colour to draw the outline
		//		RGB*				-		Pointer to the fill colour
		//		double				-		Saturation
		//
		//  RETURNS
		//
		//  NOTES
		//

		static void		drawDiamond(RasterBuffer<RGB>& Canvas, BoundingBox& Outline, size_t Width, const RGB& Pen, const RGB* Fill, double mix) {
			BoundingBox		Box = {};														//  Box Outline constraining the diamond 
			LineSegment		Edge = {};														//  Line segment to draw

			//  Diamond is ALWAYS drawn in a square box with an odd number of pixels dimensions
			if ((Outline.Bottom - Outline.Top) & 1) Outline.Bottom--;
			if ((Outline.Right - Outline.Left) & 1) Outline.Right--;
			if ((Outline.Bottom - Outline.Top) != (Outline.Right - Outline.Left)) {
				if ((Outline.Bottom - Outline.Top) > (Outline.Right - Outline.Left)) {
					//  Adjust the Bottom
					Outline.Bottom = Outline.Top + (Outline.Right - Outline.Left);
				}
				else {
					//  Adjust the Right
					Outline.Right = Outline.Left + (Outline.Bottom - Outline.Top);
				}
			}

			Box = Outline;

			//  Recursively draw smaller diamonds until the width is 1
			while (Width > 1) {
				Box.Bottom = Box.Bottom - 1;
				Box.Left = Box.Left + 1;
				Box.Right = Box.Right - 1;
				Box.Top = Box.Top + 1;
				drawDiamond(Canvas, Box, 1, Pen, nullptr, mix);
			}

			//  Draw the four edges of the Diamond
			Edge.OriginR = Outline.Top;
			Edge.TargetR = Outline.Top  + ((Outline.Bottom - Outline.Top) / 2);
			Edge.OriginC = Outline.Left + ((Outline.Right - Outline.Left) / 2);
			Edge.TargetC = Outline.Right;
			Edge.Width = 1;
			drawLine(Canvas, Edge, Pen, mix);

			Edge.OriginR = Outline.Top + ((Outline.Bottom - Outline.Top) / 2);
			Edge.TargetR = Outline.Bottom;
			Edge.OriginC = Outline.Right;
			Edge.TargetC = Outline.Left + ((Outline.Right - Outline.Left) / 2);
			Edge.Width = 1;
			drawLine(Canvas, Edge, Pen, mix);

			Edge.OriginR = Outline.Bottom;
			Edge.TargetR = Outline.Top + ((Outline.Bottom - Outline.Top) / 2);
			Edge.OriginC = Outline.Left + ((Outline.Right - Outline.Left) / 2);
			Edge.TargetC = Outline.Left;
			Edge.Width = 1;
			drawLine(Canvas, Edge, Pen, mix);

			Edge.OriginR = Outline.Top + ((Outline.Bottom - Outline.Top) / 2);
			Edge.TargetR = Outline.Top;
			Edge.OriginC = Outline.Left;
			Edge.TargetC = Outline.Left + ((Outline.Right - Outline.Left) / 2);
			Edge.Width = 1;
			drawLine(Canvas, Edge, Pen, mix);

			//  If there is a fill colour specified then fill void at the center of the diamond
			if (Fill != nullptr) {
				//  Flood fill the center point of the bounding box
				Canvas.scanFill(Outline.Top + ((Outline.Bottom - Outline.Top) / 2), Outline.Left + ((Outline.Right - Outline.Left) / 2), *Fill);
			}

			//  Return to caller
			return;
		}

		//  drawCross
		//
		//  Draws an cross (+) the given position, with the given pen colour and width and 100% saturation.
		//
		//  PARAMETERS
		//
		//		RasterBuffer<RGB>&	-		Reference to the Canvas to draw on
		//		BoundingBox&		-		Reference to the diamond containing box co-ordinates to be drawn
		//		size_t				-		Width of the cross
		//		RGB&				-		Reference to the colour to draw the outline
		//
		//  RETURNS
		//
		//  NOTES
		//

		static void		drawCross(RasterBuffer<RGB>& Canvas, BoundingBox& Outline, size_t Width, const RGB& Pen) {
			return drawCross(Canvas, Outline, Width, Pen, 1.0);
		}

		//  drawCross
		//
		//  Draws an cross (+) the given position, with the given pen colour and width and saturation.
		//
		//  PARAMETERS
		//
		//		RasterBuffer<RGB>&	-		Reference to the Canvas to draw on
		//		BoundingBox&		-		Reference to the diamond containing box co-ordinates to be drawn
		//		size_t				-		Width of the cross
		//		RGB&				-		Reference to the colour to draw the outline
		//		double				-		Saturation
		//
		//  RETURNS
		//
		//  NOTES
		//

		static void		drawCross(RasterBuffer<RGB>& Canvas, BoundingBox& Outline, size_t Width, const RGB& Pen, double mix) {
			LineSegment		Line = {};														//  Line segment to draw
			size_t			Drawn = 0;														//  Width drawn

			//  Cross is ALWAYS drawn in a square box with an odd number of pixels dimensions
			if ((Outline.Bottom - Outline.Top) & 1) Outline.Bottom--;
			if ((Outline.Right - Outline.Left) & 1) Outline.Right--;
			if ((Outline.Bottom - Outline.Top) != (Outline.Right - Outline.Left)) {
				if ((Outline.Bottom - Outline.Top) > (Outline.Right - Outline.Left)) {
					//  Adjust the Bottom
					Outline.Bottom = Outline.Top + (Outline.Right - Outline.Left);
				}
				else {
					//  Adjust the Right
					Outline.Right = Outline.Left + (Outline.Bottom - Outline.Top);
				}
			}

			//  Width is ALWAYS an odd number of pixels
			Width = Width | 1;

			//
			//  Draw the center lines (irrespective of the width)
			//

			Line.OriginR = Outline.Top;
			Line.TargetR = Outline.Bottom;
			Line.OriginC = (Outline.Right - Outline.Left) / 2;
			Line.TargetC = Line.OriginC;
			Line.Width = 1;
			drawLine(Canvas, Line, Pen, mix);

			Line.OriginR = (Outline.Bottom - Outline.Top) / 2;
			Line.TargetR = Line.OriginR;
			Line.OriginC = Outline.Left;
			Line.TargetC = Outline.Right;
			Line.Width = 1;
			drawLine(Canvas, Line, Pen, mix);

			Drawn = 1;

			//  Draw progressive pairs of lines until we match the desired width
			while ((2 * Drawn) < Width) {

				Line.OriginR = Outline.Top;
				Line.TargetR = Outline.Bottom;
				Line.OriginC = ((Outline.Right - Outline.Left) / 2) - Drawn;
				Line.TargetC = Line.OriginC;
				Line.Width = 1;
				drawLine(Canvas, Line, Pen, mix);

				Line.OriginR = Outline.Top;
				Line.TargetR = Outline.Bottom;
				Line.OriginC = ((Outline.Right - Outline.Left) / 2) + Drawn;
				Line.TargetC = Line.OriginC;
				Line.Width = 1;
				drawLine(Canvas, Line, Pen, mix);

				Line.OriginR = ((Outline.Bottom - Outline.Top) / 2) - Drawn;
				Line.TargetR = Line.OriginR;
				Line.OriginC = Outline.Left;
				Line.TargetC = Outline.Right;
				Line.Width = 1;
				drawLine(Canvas, Line, Pen, mix);

				Line.OriginR = ((Outline.Bottom - Outline.Top) / 2) + Drawn;
				Line.TargetR = Line.OriginR;
				Line.OriginC = Outline.Left;
				Line.TargetC = Outline.Right;
				Line.Width = 1;
				drawLine(Canvas, Line, Pen, mix);

				//  Update the drawn count
				Drawn++;
			}

			//  Return to caller
			return;
		}

		//  drawX
		//
		//  Draws a diagonal cross (x) at the given position, with the given pen colour and width and 100% saturation.
		//
		//  PARAMETERS
		//
		//		RasterBuffer<RGB>&	-		Reference to the Canvas to draw on
		//		BoundingBox&		-		Reference to the diamond containing box co-ordinates to be drawn
		//		size_t				-		Width of the cross
		//		RGB&				-		Reference to the colour to draw the outline
		//
		//  RETURNS
		//
		//  NOTES
		//

		static void		drawX(RasterBuffer<RGB>& Canvas, BoundingBox& Outline, size_t Width, const RGB& Pen) {
			return drawX(Canvas, Outline, Width, Pen, 1.0);
		}

		//  drawX
		//
		//  Draws a diagonal cross (x) at the given position, with the given pen colour and width and saturation.
		//
		//  PARAMETERS
		//
		//		RasterBuffer<RGB>&	-		Reference to the Canvas to draw on
		//		BoundingBox&		-		Reference to the diamond containing box co-ordinates to be drawn
		//		size_t				-		Width of the cross
		//		RGB&				-		Reference to the colour to draw the outline
		//
		//  RETURNS
		//
		//  NOTES
		//

		static void		drawX(RasterBuffer<RGB>& Canvas, BoundingBox& Outline, size_t Width, const RGB& Pen, double mix) {
			LineSegment		Line = {};														//  Line segment to draw
			size_t			Drawn = 0;														//  Width drawn

			//  X is ALWAYS drawn in a square box with an odd number of pixels dimensions
			if ((Outline.Bottom - Outline.Top) & 1) Outline.Bottom--;
			if ((Outline.Right - Outline.Left) & 1) Outline.Right--;
			if ((Outline.Bottom - Outline.Top) != (Outline.Right - Outline.Left)) {
				if ((Outline.Bottom - Outline.Top) > (Outline.Right - Outline.Left)) {
					//  Adjust the Bottom
					Outline.Bottom = Outline.Top + (Outline.Right - Outline.Left);
				}
				else {
					//  Adjust the Right
					Outline.Right = Outline.Left + (Outline.Bottom - Outline.Top);
				}
			}

			//  Width is ALWAYS an odd number of pixels
			Width = Width | 1;

			//
			//  Draw the center lines (irrespective of the width)
			//

			Line.OriginR = Outline.Top;
			Line.TargetR = Outline.Bottom;
			Line.OriginC = Outline.Left;
			Line.TargetC = Outline.Right;
			Line.Width = 1;
			drawLine(Canvas, Line, Pen, mix);

			Line.OriginR = Outline.Top;
			Line.TargetR = Outline.Bottom;
			Line.OriginC = Outline.Right;
			Line.TargetC = Outline.Left;
			Line.Width = 1;
			drawLine(Canvas, Line, Pen, mix);

			Drawn = 1;

			//  Draw progressive pairs of lines until we match the desired width
			while ((2 * Drawn) < Width) {

				Line.OriginR = Outline.Top + Drawn;
				Line.TargetR = Outline.Bottom;
				Line.OriginC = Outline.Left;
				Line.TargetC = Outline.Right - Drawn;
				Line.Width = 1;
				drawLine(Canvas, Line, Pen, mix);

				Line.OriginR = Outline.Top;
				Line.TargetR = Outline.Bottom - Drawn;
				Line.OriginC = Outline.Left + Drawn;
				Line.TargetC = Outline.Right;
				Line.Width = 1;
				drawLine(Canvas, Line, Pen, mix);

				Line.OriginR = Outline.Top + Drawn;
				Line.TargetR = Outline.Bottom;
				Line.OriginC = Outline.Right;
				Line.TargetC = Outline.Left + Drawn;
				Line.Width = 1;
				drawLine(Canvas, Line, Pen, mix);

				Line.OriginR = Outline.Top;
				Line.TargetR = Outline.Bottom - Drawn;
				Line.OriginC = Outline.Right - Drawn;
				Line.TargetC = Outline.Left;
				Line.Width = 1;
				drawLine(Canvas, Line, Pen, mix);

				Drawn++;
			}

			//  Return to caller
			return;
		}

	private:

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Private Functions                                                                                             *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  drawLineSegment
		//
		//  Draws a single line segment with the specified saturation
		//
		//  PARAMETERS
		//
		//		RasterBuffer<RGB>&	-		Reference to the Canvas to draw on
		//		LineSegment&		-		Reference to the line segment to be drawn
		//		RGB&				-		Reference to the colour to draw the line
		//		double				-		Mixing quotient for drawing (saturation)
		//
		//  RETURNS
		//
		//  NOTES
		//

		static void		drawLineSegment(RasterBuffer<RGB>& Canvas, LineSegment& LS, const RGB& Colour, double mix) {
			double		Slope = 0.0;																		//  Line slope
			size_t		Target = 0;																			//  Target non-stepped Pixel (X or Y)

			//  Safety - adjust bounds outside the canvas
			if (LS.OriginC > (Canvas.getWidth() - 1)) LS.OriginR = Canvas.getWidth() - 1;
			if (LS.OriginR > (Canvas.getHeight() - 1)) LS.OriginR = Canvas.getHeight() - 1;
			if (LS.TargetC > (Canvas.getWidth() - 1)) LS.TargetR = Canvas.getWidth() - 1;
			if (LS.TargetR > (Canvas.getHeight() - 1)) LS.TargetR = Canvas.getHeight() - 1;

			//
			//  Case:	Generic line
			//

			//  Determine the larger delta (Row or Column)
			if (abs(int(LS.TargetR) - int(LS.OriginR)) >= abs(int(LS.TargetC) - int(LS.OriginC))) {
				//  Step the Y axis
				Slope = (double(LS.TargetC) - double(LS.OriginC)) / (double(LS.TargetR) - double(LS.OriginR));

				//  Step over the span of the Y axis
				if (LS.TargetR > LS.OriginR) {
					for (int Step = int(LS.OriginR); Step <= int(LS.TargetR); Step++) {
						Target = size_t(floor((double(LS.OriginC) + (double(Step) - double(LS.OriginR)) * Slope) + 0.5));
						Canvas.setPixel(Step, Target, Colour, mix);
					}
				}
				else {
					for (int Step = int(LS.OriginR); Step >= int(LS.TargetR); Step--) {
						Target = size_t(floor((double(LS.OriginC) + (double(Step) - double(LS.OriginR)) * Slope) + 0.5));
						Canvas.setPixel(Step, Target, Colour, mix);
					}
				}
			}
			else {
				//  Step the X axis
				Slope = (double(LS.TargetR) - double(LS.OriginR)) / (double(LS.TargetC) - double(LS.OriginC));

				//  Step over the span of the X axis
				if (LS.TargetC > LS.OriginC) {
					for (int Step = int(LS.OriginC); Step <= int(LS.TargetC); Step++) {
						Target = size_t(floor((double(LS.OriginR) + (double(Step) - double(LS.OriginC)) * Slope) + 0.5));
						Canvas.setPixel(Target, Step, Colour, mix);
					}
				}
				else {
					for (int Step = int(LS.OriginC); Step >= int(LS.TargetC); Step--) {
						Target = size_t(floor((double(LS.OriginR) + (double(Step) - double(LS.OriginC)) * Slope) + 0.5));
						Canvas.setPixel(Target, Step, Colour, mix);
					}
				}
			}

			//  Return to caller
			return;
		}

		//  drawLineSegment
		//
		//  Draws a single line segment on a Matte with the given Opacity
		//
		//  PARAMETERS
		//
		//		Matte&				-		Reference to the Matte to draw on
		//		LineSegment&		-		Reference to the line segment to be drawn
		//		double				-		Opacity to draw
		//
		//  RETURNS
		//
		//  NOTES
		//

		static void		drawLineSegment(Matte& CM, LineSegment& LS, double Opacity) {
			double		Slope = 0.0;																		//  Line slope
			size_t		Target = 0;																			//  Target non-stepped Pixel (X or Y)

			//  Safety - adjust bounds outside the canvas
			if (LS.OriginC > (CM.getWidth() - 1)) LS.OriginR = CM.getWidth() - 1;
			if (LS.OriginR > (CM.getHeight() - 1)) LS.OriginR = CM.getHeight() - 1;
			if (LS.TargetC > (CM.getWidth() - 1)) LS.TargetR = CM.getWidth() - 1;
			if (LS.TargetR > (CM.getHeight() - 1)) LS.TargetR = CM.getHeight() - 1;

			//
			//  Case:	Generic line
			//

			//  Determine the larger delta (Row or Column)
			if (abs(int(LS.TargetR) - int(LS.OriginR)) >= abs(int(LS.TargetC) - int(LS.OriginC))) {
				//  Step the Y axis
				Slope = (double(LS.TargetC) - double(LS.OriginC)) / (double(LS.TargetR) - double(LS.OriginR));

				//  Step over the span of the Y axis
				if (LS.TargetR > LS.OriginR) {
					for (int Step = int(LS.OriginR); Step <= int(LS.TargetR); Step++) {
						Target = size_t(floor((double(LS.OriginC) + (double(Step) - double(LS.OriginR)) * Slope) + 0.5));
						CM.setOpacity(Step, Target, Opacity);
					}
				}
				else {
					for (int Step = int(LS.OriginR); Step >= int(LS.TargetR); Step--) {
						Target = size_t(floor((double(LS.OriginC) + (double(Step) - double(LS.OriginR)) * Slope) + 0.5));
						CM.setOpacity(Step, Target, Opacity);
					}
				}
			}
			else {
				//  Step the X axis
				Slope = (double(LS.TargetR) - double(LS.OriginR)) / (double(LS.TargetC) - double(LS.OriginC));

				//  Step over the span of the X axis
				if (LS.TargetC > LS.OriginC) {
					for (int Step = int(LS.OriginC); Step <= int(LS.TargetC); Step++) {
						Target = size_t(floor((double(LS.OriginR) + (double(Step) - double(LS.OriginC)) * Slope) + 0.5));
						CM.setOpacity(Target, Step, Opacity);
					}
				}
				else {
					for (int Step = int(LS.OriginC); Step >= int(LS.TargetC); Step--) {
						Target = size_t(floor((double(LS.OriginR) + (double(Step) - double(LS.OriginC)) * Slope) + 0.5));
						CM.setOpacity(Target, Step, Opacity);
					}
				}
			}

			//  Return to caller
			return;
		}

		//  drawFadingLineSegment
		//
		//  Draws a single line segment fading from the initial to the final saturation
		//
		//  PARAMETERS
		//
		//		RasterBuffer<RGB>&	-		Reference to the Canvas to draw on
		//		LineSegment&		-		Reference to the line segment to be drawn
		//		RGB&				-		Reference to the colour to draw the line
		//		double				-		Mixing ratio at start of fade
		//		double				-		Mixing ratio at end of fade
		//
		//  RETURNS
		//
		//  NOTES
		//

		static void		drawFadingLineSegment(RasterBuffer<RGB>& Canvas, LineSegment& LS, const RGB& Colour, double imix, double fmix) {
			double		Slope = 0.0;																		//  Line slope
			size_t		Target = 0;																			//  Target non-stepped Pixel (X or Y)
			double		MixInc = 0.0;																		//  Mixture increment (per pixel)
			double		CurrMix = imix;																		//  Current mixing

			//  Safety - adjust bounds outside the canvas
			if (LS.OriginC > (Canvas.getWidth() - 1)) LS.OriginR = Canvas.getWidth() - 1;
			if (LS.OriginR > (Canvas.getHeight() - 1)) LS.OriginR = Canvas.getHeight() - 1;
			if (LS.TargetC > (Canvas.getWidth() - 1)) LS.TargetR = Canvas.getWidth() - 1;
			if (LS.TargetR > (Canvas.getHeight() - 1)) LS.TargetR = Canvas.getHeight() - 1;

			//
			//  Case:	Generic line
			//

			//  Determine the larger delta (Row or Column)
			if (int((LS.TargetR) - int(LS.OriginR)) > (int(LS.TargetC) - int(LS.OriginC))) {
				MixInc = (fmix - imix) / abs(double(LS.TargetR) - double(LS.OriginR));
				//  Step the Y axis
				Slope = (double(LS.TargetC) - double(LS.OriginC)) / (double(LS.TargetR) - double(LS.OriginR));

				//  Step over the span of the Y axis
				if (LS.TargetR > LS.OriginR) {
					for (size_t Step = LS.OriginR; Step <= LS.TargetR; Step++) {
						Target = size_t(floor((double(LS.OriginC) + (double(Step - LS.OriginR) * Slope)) + 0.5));
						Canvas.setPixel(Step, Target, Colour, CurrMix);
						CurrMix += MixInc;
					}
				}
				else {
					for (size_t Step = LS.OriginR; Step >= LS.TargetR; Step--) {
						Target = size_t(floor((double(LS.OriginC) + (double(Step - LS.OriginR) * Slope)) + 0.5));
						Canvas.setPixel(Step, Target, Colour, CurrMix);
						CurrMix += MixInc;
					}
				}
			}
			else {
				//  Step the X axis
				MixInc = (fmix - imix) / abs(double(LS.TargetC) - double(LS.OriginC));
				Slope = (double(LS.TargetR) - double(LS.OriginR)) / (double(LS.TargetC) - double(LS.OriginC));

				//  Step over the span of the X axis
				if (LS.TargetC > LS.OriginC) {
					for (size_t Step = LS.OriginC; Step <= LS.TargetC; Step++) {
						Target = size_t(floor((double(LS.OriginR) + (double(Step - LS.OriginC) * Slope)) + 0.5));
						Canvas.setPixel(Target, Step, Colour, CurrMix);
						CurrMix += MixInc;
					}
				}
				else {
					for (size_t Step = LS.OriginC; Step >= LS.TargetC; Step--) {
						Target = size_t(floor((double(LS.OriginR) + (double(Step - LS.OriginC) * Slope)) + 0.5));
						Canvas.setPixel(Target, Step, Colour, CurrMix);
						CurrMix += MixInc;
					}
				}
			}

			//  Return to caller
			return;
		}

		//  propogateCircle
		//
		//  Plots 8 symmetric points on a circle
		//
		//  PARAMETERS
		//
		//		RasterBuffer<RGB>&	-		Reference to the Canvas to draw on
		//		Circle&				-		Reference to the Circle descriptor to be drawn
		//		RGB&				-		Reference to the colour to draw the line
		//		double				-		Saturation to draw the circle
		//		size_t				-		X relative co-ordinate of primary segment plot
		//		size_t				-		Y relative co-ordinate of primary segment plot
		//
		//  RETURNS
		//
		//  NOTES
		//

		static void		propogateCircle(RasterBuffer<RGB>& Canvas, Circle& CD, const RGB& Colour, double mix, size_t X, size_t Y) {

			//  Plot each symmetric point
			Canvas.setPixel(CD.OriginR - Y, CD.OriginC + X, Colour, mix);
			Canvas.setPixel(CD.OriginR - Y, CD.OriginC - X, Colour, mix);
			Canvas.setPixel(CD.OriginR + Y, CD.OriginC + X, Colour, mix);
			Canvas.setPixel(CD.OriginR + Y, CD.OriginC - X, Colour, mix);
			Canvas.setPixel(CD.OriginR - X, CD.OriginC + Y, Colour, mix);
			Canvas.setPixel(CD.OriginR - X, CD.OriginC - Y, Colour, mix);
			Canvas.setPixel(CD.OriginR + X, CD.OriginC + Y, Colour, mix);
			Canvas.setPixel(CD.OriginR + X, CD.OriginC - Y, Colour, mix);

			//  Return to caller
			return;
		}

		//  propogateShadingCircle
		//
		//  Plots 8 symmetric points on a circle, the colour is selected from the TriColour by biassed random selection
		//
		//  PARAMETERS
		//
		//		RasterBuffer<RGB>&	-		Reference to the Canvas to draw on
		//		Circle&				-		Reference to the Circle descriptor to be drawn
		//		TriColour&			-		Reference to the colour selection
		//		PRNG&				-		Reference to the Random Number source
		//		size_t				-		X relative co-ordinate of primary segment plot
		//		size_t				-		Y relative co-ordinate of primary segment plot
		//
		//  RETURNS
		//
		//  NOTES
		//

		static void		propogateShadingCircle(RasterBuffer<RGB>& Canvas, Circle& CD, TriColour& CS, PRNG& RGen, size_t X, size_t Y) {
			RGB			Colour = {};																//  Selected colour

			//  Plot each symmetric point
			Colour = selectShadingColour(CS, RGen);
			Canvas.setPixel(CD.OriginR - Y, CD.OriginC + X, Colour, 1.0);
			Colour = selectShadingColour(CS, RGen);
			Canvas.setPixel(CD.OriginR - Y, CD.OriginC - X, Colour, 1.0);
			Colour = selectShadingColour(CS, RGen);
			Canvas.setPixel(CD.OriginR + Y, CD.OriginC + X, Colour, 1.0);
			Colour = selectShadingColour(CS, RGen);
			Canvas.setPixel(CD.OriginR + Y, CD.OriginC - X, Colour, 1.0);
			Colour = selectShadingColour(CS, RGen);
			Canvas.setPixel(CD.OriginR - X, CD.OriginC + Y, Colour, 1.0);
			Colour = selectShadingColour(CS, RGen);
			Canvas.setPixel(CD.OriginR - X, CD.OriginC - Y, Colour, 1.0);
			Colour = selectShadingColour(CS, RGen);
			Canvas.setPixel(CD.OriginR + X, CD.OriginC + Y, Colour, 1.0);
			Colour = selectShadingColour(CS, RGen);
			Canvas.setPixel(CD.OriginR + X, CD.OriginC - Y, Colour, 1.0);

			//  Return to caller
			return;
		}

		//  selectShadingColour
		//
		//  The colour is selected from the TriColour by biassed random selection
		//
		//  PARAMETERS
		//
		//		TriColour&			-		Reference to the colour selection
		//		PRNG&				-		Reference to the Random Number source
		//
		//  RETURNS
		//
		//		RGB					-		The colour selected from the TriColour
		//
		//  NOTES
		//

		static RGB		selectShadingColour(TriColour& CS, PRNG& RGen) {
			RGB			Colour = {};																//  Selected colour
			bool		Selected = false;															//  Colour selected control
			std::uniform_int_distribution<size_t>		PCDist{ 1,100 };								//  Distribution for probability of colour selection

			//  Solid primary colour
			if (CS.PPC1 == 100) {
				Colour = CS.C1;
				Selected = true;
			}

			//  Primary Colour Probability selection
			if (!Selected && CS.PPC1 > 0) {
				if (PCDist(RGen) <= CS.PPC1) {
					Colour = CS.C1;
					Selected = true;
				}
			}

			//  Secondary colour selection
			if (!Selected) {
				if (CS.PPC2 == 100) {
					Colour = CS.C2;
					Selected = true;
				}
			}

			if (!Selected && CS.PPC2 > 0) {
				if (PCDist(RGen) <= CS.PPC2) {
					Colour = CS.C2;
					Selected = true;
				}
			}

			//  Tertiary colour selection
			if (!Selected) Colour = CS.C3;

			//  Return the selected colour
			return Colour;
		}

	};

}
