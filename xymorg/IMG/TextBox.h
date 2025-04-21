#pragma once
//*******************************************************************************************************************
//*																													*
//*   File:       TextBox.h																							*
//*   Suite:      xymorg Image Processing - primitives																*
//*   Version:    1.0.0	  Build:  01																				*
//*   Author:     Ian Tree/HMNL																						*
//*																													*
//*   Copyright 2017 - 2020 Ian J. Tree																				*
//*******************************************************************************************************************
//*	TextBox.h																										*
//*																													*
//*	This header file contains the Class definition and implementation for the TextBox primitive class.				*
//* The TextBox class exposes the text drawing primitive functions.													*
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
#include	"../xymorg.h"			

//  Include xymorg image processing primitives
#include	"types.h"																				//  Image processing primitive types
#include	"RasterBuffer.h"																		//  Raster Buffer
#include	"Draw.h"																				//  Drawing primitives
#include	"FontStyle.h"																			//  Font Style constants

//  Include the text rendering engine - Specialised for OS
#if (defined(_WIN32) || defined(_WIN64))
#include	"../WINDOWS/TextRenderer.h"																//  Windows Text Rendering Engine
#else
#include	"../UNIX/TextRenderer.h"																//  UNIX/Linux Text Rendering Engine
#endif

//*******************************************************************************************************************
//*																											        *
//*   TextBox Class																									*
//*                                                                                                                 *
//*   The TextBox class exposes the static text drawing primitive functions.										*
//*                                                                                                                 *
//*******************************************************************************************************************

namespace xymorg {

	class TextBox {
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
		//  Constructs a new TextBox object
		//
		//  PARAMETERS
		//
		//		AppConfig&			-		Reference to the Application Configuration
		//
		//  RETURNS
		//
		//  NOTES
		//

		TextBox(AppConfig& Config) : Engine(Config.TRE) {

			//  Return to caller
			return;
		}

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   destructor	                                                                                                *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		~TextBox() {

			//  Return to caller
			return;
		}

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Public Functions                                                                                              *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  selectFont
		//
		//  Resolves and optionally caches a font variant for use in text boxes.
		//
		//  PARAMETERS
		//
		//		char*				-		Const pointer to the Font Face Name
		//		size_t				-		Height in Points
		//		SWITCHES			-		Font Style bit array (see FontStyle::*)
		//
		//  RETURNS
		//
		//		FONTID				-		The ID (Handle) for the specified Font
		//
		//  NOTES
		//

		FONTID		selectFont(const char* Face, size_t Height, SWITCHES AFStyle) {

			//  Pass the call through to the Text Rendering Engine
			return Engine.selectFont(Face, Height, AFStyle);
		}

		//  getBox
		//
		//  Returns the Raster Buffer containing the desired text. 
		//
		//  PARAMETERS
		//
		//		char*				-		Const pointer to the string to be expressed
		//		FONTID				-		Font Selector
		//		RGB&				-		Reference to the Background colour
		//		RGB&				-		Reference to the Text colour
		//		SWITCHES			-		Font Style bit array (see FontStyle::*)
		//
		//  RETURNS
		//
		//		RasterBuffer*		-		Pointer to the Raster Buffer
		//
		//  NOTES
		//
		//		Call selectFont() to add the requested font with the requisite styles/metrics to the Font Cache before calling getBox.
		//

		RasterBuffer<RGB>* getBox(const char* pString, FONTID FID, const RGB& BC, const RGB& TC, SWITCHES AFStyle) {
			RasterBuffer<RGB>*			pRB = nullptr;									//  Raster Buffer under construction
			
			//  Degenerate cases
			if (pString == nullptr) return nullptr;
			if (pString[0] == '\0') return nullptr;

			//  Pass the call to the Text rendering Engine
			pRB = Engine.getBox(pString, FID, BC, TC, AFStyle);

			//  Return the Raster Buffer
			return pRB;
		}

	private:

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Private Members	                                                                                            *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		TextRenderer&		Engine;														//  OS Rendering Engine

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Private Functions                                                                                             *
		//*                                                                                                                 *
		//*******************************************************************************************************************

	};

}
