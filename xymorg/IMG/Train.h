#pragma once
//
//*******************************************************************************************************************
//*																													*
//*   File:       Train.h																							*
//*   Suite:      xymorg Image Processing - primitives																*
//*   Version:    1.0.1	  Build:  02																				*
//*   Author:     Ian Tree/HMNL																						*
//*																													*
//*   Copyright 2017 - 2025 Ian J. Tree																				*
//*******************************************************************************************************************
//*	Train.h																											*
//*																													*
//*	This header file contains the Class definition and implementation for the Train template primitive.				*
//* A Train holds information that defines the use of a series of Frames in a real image.							*
//*																													*
//*	NOTES:																											*
//*																													*
//*																													*
//*******************************************************************************************************************
//*																													*
//*   History:																										*
//*																													*
//*	1.0.0 - 04/08/2018   -  Initial version																			*
//*	1.0.1 - 05/01/2025   -  Winter Cleanup																			*
//*																													*
//*******************************************************************************************************************

//  Include basic xymorg headers
#include	"../LPBHdrs.h"																			//  Language and Platform base headers
#include	"../types.h"																			//  xymorg type definitions
#include	"../consts.h"																			//  xymorg constant definitions

//  Include xymorg image processing primitives
#include	"types.h"																				//  Image processing primitive types
#include	"Frame.h"																				//  Image Frame

//*******************************************************************************************************************
//*																											        *
//*   Train Template Class																							*
//*                                                                                                                 *
//*   Objects of this class provide information about the use of a series of Frames in a real image.				*
//*   The Template Type (C) is a colour space type such as RGB.														*
//*                                                                                                                 *
//*******************************************************************************************************************

namespace xymorg {

	template <typename C>
	class Train {
	public:

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Constructors                                                                                                  *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  Default Constructor
		//
		//  Constructs a valid but empty Train
		//
		//  PARAMETERS
		//
		//  RETURNS
		//
		//  NOTES
		//

		Train() {

			NumFrames = 0;
			pFirstFrame = nullptr;
			pCaboose = nullptr;

			CanH = 0;
			CanW = 0;
			memset(&Background, 0, sizeof(C));

			//  Return to caller
			return;
		}

		//  Normal Constructor
		//
		//  Constructs a train with content & context
		//
		//  PARAMETERS
		//
		//		size_t					-		Canvas Height
		//		size_t					-		Canvas Width
		//		C*						-		Pointer to the background colour
		//
		//  RETURNS
		//
		//  NOTES
		//

		Train(size_t CH, size_t CW, C* pBkg) {

			memset(&Background, 0, sizeof(C));
			CanH = CH;
			CanW = CW;
			if (pBkg != nullptr) memcpy(&Background, pBkg, sizeof(C));

			NumFrames = 0;
			pFirstFrame = nullptr;
			pCaboose = nullptr;

			//  Return to caller
			return;
		}

		//  Copy Constructor
		//
		//  Constructs a train from an existing train, this is a deep copy that creates a copy of each frame in the train.
		//
		//  PARAMETERS
		//
		//		Train<C>*			-		Const reference to the source train
		//
		//  RETURNS
		//
		//  NOTES
		//

		Train(const Train<C>& Src) {
			Frame<C>*		pSource = Src.pFirstFrame;												//  Start of the source train of frames

			//  Copy the context
			CanH = Src.CanH;
			CanW = Src.CanW;
			memcpy(&Background, &Src.Background, sizeof(C));
			NumFrames = 0;
			pFirstFrame = nullptr;
			pCaboose = nullptr;

			//  Construct a new train of frames from the source train
			while (pSource != nullptr) {

				//  append a new frame
				append(new Frame<C>(*pSource));

				//  Move on to the next frame
				pSource = pSource->getNext();
			}

			//  Return to caller
			return;
		}

		//  Move Constructor
		//
		//  Constructs a train from an existing train, this is a deep copy that takes ownership of the content of the source frame
		//
		//  PARAMETERS
		//
		//		Train<C>*			-		Reference to the source train
		//
		//  RETURNS
		//
		//  NOTES
		//

		Train(Train<C>&& Src) {

			//  Copy the context
			CanH = Src.CanH;
			CanW = Src.CanW;
			memcpy(&Background, &Src.Background, sizeof(C));

			//  Acquire the ownership of the content
			NumFrames = Src.NumFrames;
			pFirstFrame = Src.pFirstFrame;
			pCaboose = Src.pCaboose;

			Src.NumFrames = 0;
			Src.pFirstFrame = nullptr;
			Src.pCaboose = nullptr;

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
		//  Destroys the content of the Train, including all of the Frames. 
		//
		//  PARAMETERS
		//
		//  RETURNS
		//
		//  NOTES
		//

		~Train() {
			Frame<C>*	pDelFrame = pFirstFrame;

			//  Delete the train of frames
			while (pDelFrame != nullptr) {
				pFirstFrame = pDelFrame->getNext();
				delete pDelFrame;
				pDelFrame = pFirstFrame;
			}

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

		size_t		getCanvasWidth() { return CanW; }
		size_t		getCanvasHeight() { return CanH; }
		C&			getBackGround() { return Background; }
		size_t		getNumFrames() { return NumFrames; }
		Frame<C>*	getFirstFrame() { return pFirstFrame; }
		Frame<C>*	getLastFrame() { return pCaboose; }
		Frame<C>*	getCaboose() { return pCaboose; }

		void		setCanvasWidth(size_t NewWidth) { CanW = NewWidth; return; }
		void		setCanvasHeight(size_t NewHeight) { CanH = NewHeight; return; }
		void		setBackground(C& NewBkg) { Background = NewBkg; return; }
		void		setNumFrames(size_t NewNF) { NumFrames = NewNF; return; }
		void		setFirstFrame(Frame<C>* NewFF) { pFirstFrame = NewFF; return; }
		void		setLastFrame(Frame<C>* NewLF) { pCaboose = NewLF; return; }
		void		setCaboose(Frame<C>* NewLF) { pCaboose = NewLF; return; }

		//
		//  Train Manipulators
		//

		//  append
		//
		//  Appends a new Frame to the Train 
		//
		//  PARAMETERS
		//
		//		Frame<C>*		-		Pointer to the frame to be appended
		//
		//  RETURNS
		//
		//  NOTES
		//

		void		append(Frame<C>* NewLF) {

			if (NewLF == nullptr) return;

			//  Boundary
			if (pFirstFrame == nullptr) pFirstFrame = NewLF;

			//  Maintain the train
			NewLF->setNext(nullptr);
			NewLF->setPrev(pCaboose);
			if (pCaboose != nullptr) pCaboose->setNext(NewLF);
			pCaboose = NewLF;
			NumFrames++;

			//  Return to caller
			return;
		}

		//  append
		//
		//  Appends a new Frame to the Train from a Raster Buffer 
		//
		//  PARAMETERS
		//
		//		RasterBuffer<C>*		-		Pointer to the Raster Buffer to be incorporated
		//
		//  RETURNS
		//
		//  NOTES
		//

		void		append(RasterBuffer<C>* NewRB) {

			if (NewRB == nullptr) return;

			//  Construct the new frame
			append(0, 0, NewRB);

			//  Return to caller
			return;
		}

		//  append
		//
		//  Appends a new Frame to the Train from a Raster Buffer 
		//
		//  PARAMETERS
		//
		//		size_t					-		Row position on image canvas
		//		size_t					-		Column position on image canvas
		//		RasterBuffer<C>*		-		Pointer to the Raster Buffer to be incorporated
		//
		//  RETURNS
		//
		//  NOTES
		//

		void		append(size_t CR, size_t CC, RasterBuffer<C>* NewRB) {

			append(new Frame<C>(NewRB, CR, CC, nullptr));

			//  Return to caller
			return;
		}

		//  deleteFrame
		//
		//  Deletes the identified frame from the train.
		//
		//  PARAMETERS
		//
		//		size_t					-		Frame number to be deleted
		//
		//  RETURNS
		//
		//  NOTES
		//

		void		deleteFrame(size_t FNo) {
			Frame<C>*		pFrame = pFirstFrame;													//  Pointer to current frame
			size_t			FID = 1;																//  Frame Id (Number)

			//  Safety
			if (FNo == 0) return;

			//  Position to the target frame
			while (FID < FNo && pFrame != nullptr) {
				pFrame = pFrame->getNext();
				FID++;
			}

			//  Frame NOT found
			if (pFrame == nullptr) return;

			//  Boundary frame is first
			if (FNo == 1) {
				pFirstFrame = pFrame->getNext();
				NumFrames--;
				delete pFrame;
				return;
			}
			
			//  Boundary frame is last
			if (FNo == NumFrames) {
				pCaboose = pFrame->getPrev();
				pCaboose->setNext(nullptr);
				NumFrames--;
				delete pFrame;
				return;
			}

			//  Frame is in a chain
			pFrame->getPrev()->setNext(pFrame->getNext());
			pFrame->getNext()->setPrev(pFrame->getPrev());
			NumFrames--;
			delete pFrame;
			return;
		}

		//  flatten
		//
		//  This function will "flatten" the sequence of frames in the train into a single frame containing the canvas image that will result.
		//  Green Screen transparency IS honoured.
		//
		//  PARAMETERS
		//
		//  RETURNS
		//
		//  NOTES
		//

		void		flatten() {
			RasterBuffer<C>*		pCanRB = nullptr;														//  Raster Buffer for canvas

			//  First autocorrect the train - this will ensure that the canvas size is sufficient
			autocorrect();

			//  Create a new Raster Buffer to hold the canvas image
			if (CanH == 0 || CanW == 0) return;
			pCanRB = new RasterBuffer<C>(CanH, CanW, &Background);

			//  If there are no existing frames then simply set the canvas as the sole frame
			if (pFirstFrame == nullptr) {
				append(pCanRB);
				return;
			}

			//
			//  Build up the complete canvas image by overlaying the source one frame at a time onto the output buffer
			//  If the frame has a transparent (Green Screen) colour then that is honoured.
			//

			while (pFirstFrame != nullptr) {
				if (pFirstFrame->hasTransparent()) pCanRB->blit(pFirstFrame->buffer(), pFirstFrame->getRRow(), pFirstFrame->getRCol(), pFirstFrame->getTransparent());
				else pCanRB->blit(pFirstFrame->buffer(), pFirstFrame->getRRow(), pFirstFrame->getRCol());
				pCaboose = pFirstFrame;
				pFirstFrame = pCaboose->getNext();
				delete pCaboose;
			}

			pFirstFrame = pCaboose = nullptr;
			NumFrames = 0;

			//  Append the new canvas frame
			append(pCanRB);

			//  Return to caller
			return;
		}

		//  autocorrect
		//
		//  Corrects the canvas size and frame count (if needed)
		//
		//  PARAMETERS
		//
		//  RETURNS
		//
		//  NOTES
		//

		void	autocorrect() {
			size_t			Frames = 0;															//  Count of frames
			size_t			CH = 0, CW = 0;														//  Canvas height and width
			Frame<C>*		pFrame = nullptr;													//  Pointer to the current frame
			Frame<C>*		pLastFrame = nullptr;												//  Pointer to last frame

			pFrame = pFirstFrame;																//  Start at first frame

			while (pFrame != nullptr) {
				Frames++;
				if ((pFrame->getRRow() + pFrame->getHeight()) > CH) CH = (pFrame->getRRow() + pFrame->getHeight());
				if ((pFrame->getRCol() + pFrame->getWidth()) > CW) CW = (pFrame->getRCol() + pFrame->getWidth());

				pLastFrame = pFrame;
				pFrame = pFrame->getNext();
			}

			NumFrames = Frames;
			if (CH > CanH) CanH = CH;
			if (CW > CanW) CanW = CW;
			pCaboose = pLastFrame;

			//  Return to caller
			return;
		}

		//  document
		//
		//  Documents the content of the Train
		//
		//  PARAMETERS
		//
		//		std::ostream&		-		Reference to the stream to document the train on
		//		bool				-		If true document the frame content
		//
		//  RETURNS
		//
		//  NOTES
		//

		void	document(std::ostream& OS, bool ShowContent) {
			size_t		FNo = 1;																	//  Frame number
			Frame<C>*	pFrame = pFirstFrame;														//  Pointer to the current frame

			OS << "  TRAIN" << std::endl;
			OS << "  -----" << std::endl;
			OS << "   " << std::endl;
			OS << "  Canvas:  Height: " << CanH << ", Width: " << CanW << "." << std::endl;
			OS << "  Background Colour: ";
			Background.document(OS);
			OS << "." << std::endl;
			if (NumFrames == 0) {
				OS << "  The train is empty (contains no frames)." << std::endl;
				return;
			}
			OS << "  Number of Frames: " << NumFrames << "." << std::endl;

			//  Document each frame in turn
			while (pFrame != nullptr) {
				pFrame->document(OS, FNo, ShowContent);
				pFrame = pFrame->getNext();
				FNo++;
			}
			OS << "   " << std::endl;

			//  Return to caller
			return;
		}

	private:

		//*******************************************************************************************************************
		//*																													*
		//*  Private Members																								*
		//*																													*
		//*******************************************************************************************************************

		//  Train Content
		size_t		NumFrames;													//  Frame Number
		Frame<C>*	pFirstFrame;												//  Pointer to the next frame
		Frame<C>*	pCaboose;													//  Pointer to the previous frame

		//  Image Context
		size_t		CanH;														//  Row on the canvas
		size_t		CanW;														//  Column on the canvas
		C			Background;													//  Background colour

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Private Functions                                                                                             *
		//*                                                                                                                 *
		//*******************************************************************************************************************

	};

}
