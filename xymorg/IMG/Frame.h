#pragma once
//
//*******************************************************************************************************************
//*																													*
//*   File:       Frame.h																							*
//*   Suite:      xymorg Image Processing - primitives																*
//*   Version:    1.0.0	  Build:  01																				*
//*   Author:     Ian Tree/HMNL																						*
//*																													*
//*   Copyright 2017 - 2020 Ian J. Tree																				*
//*******************************************************************************************************************
//*	Frame.h																											*
//*																													*
//*	This header file contains the Class definition and implementation for the Frame template primitive.				*
//* A Frame holds information that defines the use of a RasterBuffer in a real image.								*
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
#include	"ColourTable.h"																			//  Colour table

//*******************************************************************************************************************
//*																											        *
//*   Frame Template Class																							*
//*                                                                                                                 *
//*   Objects of this class provide information about the use of a RasterBuffer in a real image.					*
//*   The Template Type (C) is a colour space type such as RGB.														*
//*                                                                                                                 *
//*******************************************************************************************************************

namespace xymorg {

	template <typename C>
	class Frame {
	public:

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Public Constants                                                                                              *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  Frame Disposal Methods

		static const SWITCHES	DISPOSE_NONE = 0;											//  No disposal, take no action
		static const SWITCHES	DISPOSE_NOT = 1;											//  DO NOT DISPOSE, graphic is left in place
		static const SWITCHES	DISPOSE_RESTORE = 2;										//  Restore background colour
		static const SWITCHES	DISPOSE_REVERT = 4;											//  Restore previous image, revert

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Constructors                                                                                                  *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  Default Constructor
		//
		//  Constructs a valid but empty Frame
		//
		//  PARAMETERS
		//
		//  RETURNS
		//
		//  NOTES
		//

		Frame() {

			pNextFrame = nullptr;
			pPrevFrame = nullptr;

			RRow = 0;
			RCol = 0;
			TCSet = false;
			memset(&Transparent, 0, sizeof(C));
			Disposal = 0;
			Delay = 0;

			pBuffer = nullptr;

			//  Return to caller
			return;
		}

		//  Normal Constructor
		//
		//  Constructs a frame with content & context
		//
		//  PARAMETERS
		//
		//		RasterBuffer<C>*		-		Pointer to the RasterBuffer
		//		size_t					-		Row placement on canvas
		//		size_t					-		Column placement on canvas
		//		Frame<C>*				-		Pointer to previous Frame
		//
		//  RETURNS
		//
		//  NOTES
		//

		Frame(RasterBuffer<C>* pRB, size_t RR, size_t RC, Frame<C>* pPF) {

			pBuffer = pRB;

			RRow = RR;
			RCol = RC;

			TCSet = false;
			memset(&Transparent, 0, sizeof(C));
			Disposal = 0;
			Delay = 0;

			if (pPF == nullptr) {
				pNextFrame = nullptr;
				pPrevFrame = nullptr;
			}
			else {
				pPrevFrame = pPF;
				pNextFrame = pPF->getNext();
				pPF->setNext(this);
			}

			//  Return to caller
			return;
		}

		//  Copy Constructor
		//
		//  Constructs a frame from an existing frame, this is a deep copy that creates a new RasterBuffer for the new frame.
		//  The frame is constructed as a stand alone frame (not part of a train).
		//
		//  PARAMETERS
		//
		//		Frame<C>*			-		Const reference to the source frame
		//
		//  RETURNS
		//
		//  NOTES
		//

		Frame(const Frame<C>& Src) {
				
			//  Copy context from source
			RRow = Src.RRow;
			RCol = Src.RCol;
			TCSet = Src.TCSet;
			memcpy(&Transparent, &Src.Transparent, sizeof(C));
			Disposal = Src.Disposal;
			Delay = Src.Delay;

			//  Make stand alone frame
			pNextFrame = nullptr;
			pPrevFrame = nullptr;

			//  Copy the content
			pBuffer = new RasterBuffer<C>(Src.buffer());

			//  Return to caller
			return;
		}

		//  Move Constructor
		//
		//  Constructs a frame from an existing frame, transfers the content to the new frame
		//  The frame is constructed as a stand alone frame (not part of a train).
		//
		//  PARAMETERS
		//
		//		Frame<C>*			-		Reference to the source frame
		//
		//  RETURNS
		//
		//  NOTES
		//

		Frame(Frame<C>&& Src) {

			//  Copy context from source
			RRow = Src.RRow;
			RCol = Src.RCol;
			TCSet = Src.TCSet;
			memcpy(&Transparent, &Src.Transparent, sizeof(C));
			Disposal = Src.Disposal;
			Delay = Src.Delay;

			//  Make stand alone frame
			pNextFrame = nullptr;
			pPrevFrame = nullptr;

			//  Transfer ownership of the Raster Buffer
			pBuffer = Src.pBuffer;
			Src.pBuffer = nullptr;

			//  Return to caller
			return;
		}

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Destructor                                                                                                    *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  Destructor
		//
		//  Destroys the content of the Frame, including the related RasterBuffer
		//
		//  PARAMETERS
		//
		//  RETURNS
		//
		//  NOTES
		//

		~Frame() {

			if (pBuffer != NULL) delete pBuffer;

			//  Return to caller
			return;
		}

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Public Members                                                                                                *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Public Functions                                                                                              *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//
		//  Property Accessors
		//

		Frame<C>*	getNext() { return pNextFrame; }
		Frame<C>*	getPrev() { return pPrevFrame; }
		bool		hasTransparent() { return TCSet; }
		C&			getTransparent() { return Transparent; }
		SWITCHES	getDisposal() { return Disposal; }
		size_t		getDelay() { return Delay; }
		size_t		getRRow() { return RRow; }
		size_t		getRCol() { return RCol; }
		RasterBuffer<C>* getBuffer() { return pBuffer; }
		RasterBuffer<C>& buffer() { return *pBuffer; }
		const RasterBuffer<C>& buffer() const { return *pBuffer; }

		void		setNext(Frame<C>* NewNext) { pNextFrame = NewNext; return; }
		void		setPrev(Frame<C>* NewPrev) { pPrevFrame = NewPrev; return; }
		void		setTransparent(C& NewTrans) { Transparent = NewTrans; TCSet = true; return; }
		void		clearTransparent() { TCSet = false; return; }
		void		setDisposal(SWITCHES NewDisp) { Disposal = NewDisp; return; }
		void		setDelay(size_t NewDelay) { Delay = NewDelay; return; }
		void		setRRow(size_t NewR) { RRow = NewR; return; }
		void		setRCol(size_t NewC) { RCol = NewC; return; }
		void		setBuffer(RasterBuffer<C>* pNewBuf) { pBuffer = pNewBuf; return; }

		size_t		getHeight() { return pBuffer->getHeight(); }
		size_t		getWidth() { return pBuffer->getWidth(); }

		//  clearImage
		//
		//  Clears the image from the Frame
		//
		//  PARAMETERS
		//
		//  RETURNS
		//
		//  NOTES
		//

		void		clearImage() {
			clearTransparent();
			RRow = 0;
			RCol = 0;
			if (pBuffer != nullptr) {
				delete pBuffer;
				pBuffer = nullptr;
			}
			return;
		}

		//  document
		//
		//  Documents the content of the Train
		//
		//  PARAMETERS
		//
		//		std::ostream&		-		Reference to the stream to document the train on
		//		size_t				-		Frame Number
		//		bool				-		If true document the frame content
		//
		//  RETURNS
		//
		//  NOTES
		//

		void		document(std::ostream& OS, size_t FNum, bool ShowContent) {
			ColourTable<C>				CT;															//  Colour Table

			OS << "   " << std::endl;
			OS << "    FRAME" << std::endl;
			OS << "    -----" << std::endl;
			OS << "   " << std::endl;
			OS << "    Frame Number: " << FNum << "." << std::endl;
			if (pBuffer == NULL) OS << "    Size:  Unknown (No Frame Buffer)." << std::endl;
			else if (getHeight() == 0 || getWidth() == 0) OS << "    Size: Unknown (Empty Frame Buffer)." << std::endl;
			else OS << "    Size:  Height: " << getHeight() << ", Width: " << getWidth() << "." << std::endl;
			OS << "    Placement:  Row: " << RRow << ", Column: " << RCol << "." << std::endl;
			OS << "    Transparent Colour: ";
			if (hasTransparent()) Transparent.document(OS);
			else OS << "None";
			OS << "." << std::endl;
			OS << "    Render:  Disposal Method: ";
			switch (Disposal) {
			case DISPOSE_NONE:
				OS << "Not Specified, take no action";
				break;
			case DISPOSE_NOT:
				OS << "Do Not Dispose, leave the image in place";
				break;
			case DISPOSE_RESTORE:
				OS << "Restore to background colour";
				break;
			case DISPOSE_REVERT:
				OS << "Revert to previous image";
				break;
			default:
				OS << "Unknown";
				break;
			}
			if (Delay > 0) OS << ", After: " << Delay << " 1/100 second";
			OS << "." << std::endl;

			//  Display the content if requested
			if (ShowContent) {
				if (pBuffer == NULL) OS << "    Content: Not Available (No Frame Buffer)." << std::endl;
				else if (getHeight() == 0 || getWidth() == 0) OS << "    Content Not Availabe (Empty Frame Buffer)." << std::endl;
				else {
					if (TCSet) pBuffer->document(OS, nullptr, &Transparent);
					else pBuffer->document(OS, nullptr, nullptr);
				}
			}

			//  Return to caller
			return;
		}

	private:

		//*******************************************************************************************************************
		//*																													*
		//*  Private Members																								*
		//*																													*
		//*******************************************************************************************************************

		//  Train Context
		Frame<C>*	pNextFrame;													//  Pointer to the next frame
		Frame<C>*	pPrevFrame;													//  Pointer to the previous frame

		//  Image Context
		size_t		RRow;														//  Row on the canvas
		size_t		RCol;														//  Column on the canvas
		bool		TCSet;														//  Transparent Colour has been set
		C			Transparent;												//  Transparent (Green Screen)
		SWITCHES	Disposal;													//  Frame disposal method (GIF)
		size_t		Delay;														//  Pre-Disposal delay (Milliseconds)

		//  Image Content
		RasterBuffer<C>*	pBuffer;											//  Underlying Raster Buffer

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Private Functions                                                                                             *
		//*                                                                                                                 *
		//*******************************************************************************************************************

	};

}
