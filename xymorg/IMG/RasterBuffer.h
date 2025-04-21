#pragma once
//
//*******************************************************************************************************************
//*																													*
//*   File:       RasterBuffer.h																					*
//*   Suite:      xymorg Image Processing - primitives																*
//*   Version:    1.1.1	  Build:  03																				*
//*   Author:     Ian Tree/HMNL																						*
//*																													*
//*   Copyright 2017 - 2024 Ian J. Tree																				*
//*******************************************************************************************************************
//*	RasterBuffer.h																									*
//*																													*
//*	This header file contains the Class definition and implementation for the RasterBuffer template primitive.		*
//* A RasterBuffer is a single plane buffer holding a two dimensional array of pixels with the defined              *
//* colur space.																									*
//*																													*
//*	NOTES:																											*
//*																													*
//*																													*
//*******************************************************************************************************************
//*																													*
//*   History:																										*
//*																													*
//*	1.0.0 - 04/08/2018   -  Initial version																			*
//*	1.1.0 - 23/11/2022   -  Added difference() map generation function												*
//*	1.1.1 - 08/12/2024   -  Winter Cleanup																			*
//*																													*
//*******************************************************************************************************************

//  Include basic xymorg headers
#include	"../LPBHdrs.h"																			//  Language and Platform base headers
#include	"../types.h"																			//  xymorg type definitions
#include	"../consts.h"																			//  xymorg constant definitions

//  Additional platform headers
#include	<stack>																					//  Stack container

//  Include xymorg image processing primitives
#include	"types.h"																				//  Image processing primitive types
#include	"CIBase2D.h"																			//  2D Iterator Base Class
#include	"ColourTable.h"																			//  Colour Table

//*******************************************************************************************************************
//*                                                                                                                 *
//*   RasterBuffer Template Class		                                                                            *
//*                                                                                                                 *
//*   Objects of this class provide basic storage containers for images stored as a raster array.					*
//*   The Template Type (T) is a colour space type such as RGB.														*
//*                                                                                                                 *
//*******************************************************************************************************************

namespace xymorg {

	template <typename T>
	class RasterBuffer {
	public:

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Forward Declarations                                                                                          *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		class CompoundIterator;
		class ConstCompoundIterator;

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

		//  Default Constructormatches
		//
		//  Constructs a valid but empty RasterBuffer
		//
		//  PARAMETERS
		//
		//  RETURNS
		//
		//  NOTES
		//

		RasterBuffer() {
			T			DVal = {};																		//  Initial value for dummy reference

			//  Clear the image properties
			Height = 0;
			Width = 0;

			//  Clear the buffer pointer
			Buffer = NULL;

			//  Initialise dummy reference
			DREntry = DVal;

			//  Return to caller
			return;
		}

		//  Normal Constructor
		//
		//  Constructs a fully-formed Raster Buffer, the image storage is initialised to the passed colour or cleared.
		//
		//  PARAMETERS
		//
		//		size_t			-		Image Height in pixels
		//		size_t			-		Image width in pixels
		//		<T>*			-		Optional pointer to a colur space value to use for initialisation of the storage
		//
		//  RETURNS
		//
		//  NOTES
		//
		//		1.		If the initialiser colour is not specified then the underlying storage is initialised to 0x00.
		//		2.		If the specified height or width is zero then a valid but empty RasterBuffer is construvted.
		//

		RasterBuffer(const size_t H, const size_t W, const T* pInitColour) {
			T			DVal = {};																		//  Initial value for dummy reference
			size_t		Pixels = H * W;																	//  Number of pixels in the image

			//  Initialise dummy reference
			DREntry = DVal;

			//  Check for Height or Width is zero
			if (Pixels == 0) {
				//  Construct as per the default constructor
				Height = 0;
				Width = 0;
				Buffer = nullptr;
				return;
			}

			//  Set the properties
			Height = H;
			Width = W;

			//  Allocate and initialise the underlying storage
			Buffer = (T* ) malloc(Pixels * sizeof(T));

			//  If we failed to allocate storage then treat as per the default constructor
			if (Buffer == nullptr) {
				Height = 0;
				Width = 0;
				return;
			}

			//  If we have an initial colour then initialise the buffer with that
			if (pInitColour != nullptr) {
				for (size_t iIndex = 0; iIndex < Pixels; iIndex++) Buffer[iIndex] = *pInitColour;
			}
			else {
				//  Default initialisation
				memset(Buffer, 0, Pixels * sizeof(T));
			}

			//  Return to caller
			return;
		}

		//  Copy Constructor
		//
		//  Constructs a fully-formed Raster Buffer, the image properties and content are copied from the source image.
		//
		//  PARAMETERS
		//
		//		RasterBuffer&	-		Const reference to the source RasterBuffer
		//
		//  RETURNS
		//
		//  NOTES
		//

		RasterBuffer(const RasterBuffer& Src) {
			T			DVal = {};																		//  Initial value for dummy reference

			//  Initialise dummy reference
			DREntry = DVal;

			//  Copy the properties over from the source
			Height = Src.Height;
			Width = Src.Width;

			//  If either the height or width was zero then create as with the default constructor
			if (Height == 0 || Width == 0 || Src.Buffer == nullptr) {
				Height = 0;
				Width = 0;
				Buffer = nullptr;
				return;
			}

			//  Allocate and initialise the underlying storage
			Buffer = (T*) malloc(Height * Width * sizeof(T));

			//  If we failed to allocate storage then treat as per the default constructor
			if (Buffer == nullptr) {
				Height = 0;
				Width = 0;
				return;
			}

			//  Copy the buffer contents across from the source
			memcpy(Buffer, Src.Buffer, Height * Width * sizeof(T));

			//  Return to caller
			return;
		}

		//  Copy Region Constructor
		//
		//  Constructs a fully-formed Raster Buffer, the image properties and content are copied from a designated region of the source image.
		//
		//  PARAMETERS
		//
		//		RasterBuffer&	-		Const reference to the source RasterBuffer
		//      BoundingBox&	-		Const reference to the bounding box of the region
		//
		//  RETURNS
		//
		//  NOTES
		//

		RasterBuffer(const RasterBuffer& Src, const BoundingBox& bbReg) {
			T			DVal = {};																		//  Initial value for dummy reference
			T*			pNP = nullptr;																	//  Pointer to the next pixel to be stored

			//  Initialise dummy reference
			DREntry = DVal;

			//  Set the height & width of the image
			Height = (bbReg.Bottom - bbReg.Top) + 1;
			Width = (bbReg.Right - bbReg.Left) + 1;

			//  TRACE
			//std::cout << "TRACE: Creating new RasterBuffer from region." << std::endl;
			//std::cout << "TRACE: Region: T: " << bbReg.Top << ", L: " << bbReg.Left << ", B: " << bbReg.Bottom << ", R: " << bbReg.Right << "." << std::endl;
			//std::cout << "TRACE: Source Container: H: " << Src.getHeight() << ", W: " << Src.getWidth() << "." << std::endl;

			//  If either the height or width was zero then create as with the default constructor
			if (Height == 0 || Width == 0 || Src.Buffer == nullptr) {
				Height = 0;
				Width = 0;
				Buffer = nullptr;
				return;
			}

			//  Allocate and initialise the underlying storage
			Buffer = (T*) malloc(Height * Width * sizeof(T));

			//  If we failed to allocate storage then treat as per the default constructor
			if (Buffer == nullptr) {
				Height = 0;
				Width = 0;
				return;
			}

			//  Copy the source image to the target using a sequential bounded iterator over the source
			pNP = Buffer;
			for (const_iterator CIt = Src.begin(bbReg); CIt != Src.end(bbReg); CIt++) {
				*pNP = *CIt;
				pNP++;
			}

			//  Return to caller
			return;
		}

		//  Move Constructor
		//
		//  Constructs a fully-formed Raster Buffer, the image properties and content are acquired from the source RasterBuffer.
		//
		//  PARAMETERS
		//
		//		RasterBuffer&&	-		Reference to the source RasterBuffer
		//
		//  RETURNS
		//
		//  NOTES
		//

		RasterBuffer(RasterBuffer&& Src) noexcept {
			T			DVal = {};																		//  Initial value for dummy reference

			//  Initialise dummy reference
			DREntry = DVal;

			//  Free the buffer if it is allocated
			if (Buffer != nullptr) free(Buffer);

			//  Copy the properties over from the source
			Height = Src.Height;
			Width = Src.Width;
			Buffer = Src.Buffer;

			//  If the source was in an invalid state then create this as per the default constructor and leave the source unchanged
			if (Height == 0 || Width == 0 || Buffer == nullptr) {
				Height = 0;
				Width = 0;
				Buffer = nullptr;
				return;
			}

			//  Take ownership of the underlying buffer from the source
			Src.Buffer = nullptr;

			//  Clear the source to the ground state
			Src.Height = 0;
			Src.Width = 0;

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
		//  Destroys the content of the RasterBuffer.
		//
		//  PARAMETERS
		//
		//  RETURNS
		//
		//  NOTES
		//

		~RasterBuffer() {

			//  If the buffer has been allocated (not null) then it is owned by this RasterBuffer and MUST be freed
			if (Buffer != nullptr) free(Buffer);

			//  Clear the properties so that any use-after-free instances treat the RasterBuffer as default constructed
			Height = 0;
			Width = 0;
			Buffer = nullptr;

			//  Return to caller
			return;
		}


		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Public Members                                                                                                *
		//*******************************************************************************************************************

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Public Functions                                                                                              *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//
		//  Property Accessors
		//

		size_t		getHeight() const { return Height; }
		size_t		getWidth() const { return Width; }
		T*			getArray() { return Buffer; }
		const T*	getArray() const { return Buffer; }

		//  document
		//
		//  Documents the content of the Raster Buffer
		//
		//  PARAMETERS
		//
		//		std::ostream&		-		Reference to the stream to document the Raster Buffer on
		//		T*					-		Const pointer to the background colour
		//		T*					-		Const pointer to the transparent colour
		//
		//  RETURNS
		//
		//  NOTES
		//

		void		document(std::ostream& OS, T* BGC, T* GSC) {
			int					PrintWidth = 1;																	//  Print width
			char				PStr[20] = {};																	//  Print decoration string
			unsigned long long TempLLU = 0;																		//  Temp
			ColourTable<T>		CT;																				//  Colour Table

			//  Populate the colour table
			for (iterator RIt = top(); RIt != bottom(); RIt++) {
				for (iterator CIt = left(RIt); CIt != right(RIt); CIt++) {
					CT.add(*CIt, 1, RIt.getIndex(), CIt.getIndex());
				}
			}

			//  Prepare the colour table for display
			CT.setupForDisplay(BGC, GSC);

			OS << "    " << std::endl;
			OS << "    COLOURS:" << std::endl;
			OS << "    " << std::endl;

			for (size_t CX = 0; CX < CT.getNumColours(); CX++) {
				OS << "    C: " << CX << ", RGB: {" << int(CT.getColour(CX).R) << "," << int(CT.getColour(CX).G) << "," << int(CT.getColour(CX).B)
					<< "}, Count: " << CT.getCount(CX) << ", Display: '" << CT.getDisplay(CX) << "', "
					<< "Used within: [" << CT.getExtents(CX).Top << ", " << CT.getExtents(CX).Left << ", " << CT.getExtents(CX).Bottom << ", " << CT.getExtents(CX).Right << "]." << std::endl;
			}

			//  TITLE
			OS << std::endl;
			OS << "    IMAGE MAP:" << std::endl;
			OS << std::endl;

			//  Determine the print width and height for decoration
			if (Height > 9 || Width > 9) PrintWidth = 2;
			if (Height > 99 || Width > 99) PrintWidth = 3;
			if (Height > 999 || Width > 999) PrintWidth = 4;

			//  Show the column decoration
			decorateColumns(Width, PrintWidth, OS);

			//  Iterate over the image in Row, Column sequance displaying each scan line with decoration
			for (iterator RIt = top(); RIt != bottom(); RIt++) {
				TempLLU = RIt.getIndex();
				sprintf_s(PStr, 20, "    %*llu: ", PrintWidth, TempLLU);
				OS << PStr;
				for (iterator CIt = left(RIt); CIt != right(RIt); CIt++) OS << CT.getDisplay(*CIt);
				TempLLU = RIt.getIndex();
				sprintf_s(PStr, 20, " :%*llu", PrintWidth, TempLLU);
				OS << PStr << std::endl;
			}

			//  Show the column decoration
			OS << std::endl;
			decorateColumns(Width, PrintWidth, OS);
			OS << std::endl;

			//  Return to caller
			return;
		}

		//
		//  Individual Pixel Accessors
		//

		//  getPixel
		//
		//  Returns a pointer to the pixel at the given offset (index) in the buffer array
		//
		//  PARAMETERS
		//
		//		size_t			-		Offset (Index) of the requested pixel
		//
		//  RETURNS
		//
		//		T*				-		Pointer to the colour of the specified pixel
		//
		//  NOTES
		//
		//		1.		Returns NULL if the requested pixel is unavailable
		//

		T*		getPixel(const size_t Offset) {
			//  If the RasterBuffer is invalid or default return NULL
			if (Height == 0 || Width == 0 || Buffer == NULL) return NULL;

			//  If the supplied offset overruns the buffer then return NULL
			if (Offset >= (Height * Width)) return NULL;

			//  Return the pointer to the given pixel
			return &Buffer[Offset];
		}

		const T*	getPixel(const size_t Offset) const {
			//  If the RasterBuffer is invalid or default return NULL
			if (Height == 0 || Width == 0 || Buffer == NULL) return NULL;

			//  If the supplied offset overruns the buffer then return NULL
			if (Offset >= (Height * Width)) return NULL;

			//  Return the pointer to the given pixel
			return &Buffer[Offset];
		}

		//  getPixel
		//
		//  Returns a pointer to the pixel at the given Row & Column position
		//
		//  PARAMETERS
		//
		//		size_t			-		Row index position of the desired pixel
		//		size_t			-		Column index position of the desired pixel
		//
		//  RETURNS
		//
		//		T*				-		Pointer to the colour of the specified pixel
		//
		//  NOTES
		//
		//		1.		Returns NULL if the requested pixel is unavailable
		//

		T*		getPixel(const size_t R, const size_t C) {
			//  If the RasterBuffer is invalid or default return NULL
			if (Height == 0 || Width == 0 || Buffer == NULL) return NULL;

			//  If the supplied position is ouside the array bounds then return NULL
			if (R >= Height || C >= Width) return NULL;

			//  Return the pointer to the given pixel
			return &Buffer[(R * Width) + C];
		}

		const T*	getPixel(const size_t R, const size_t C) const {
			//  If the RasterBuffer is invalid or default return NULL
			if (Height == 0 || Width == 0 || Buffer == NULL) return NULL;

			//  If the supplied position is ouside the array bounds then return NULL
			if (R >= Height || C >= Width) return NULL;

			//  Return the pointer to the given pixel
			return &Buffer[(R * Width) + C];
		}

		//  setPixel
		//
		//  Sets the value of the given pixel to the passed colour.
		//
		//  PARAMETERS
		//
		//		size_t			-		Offset (Index) of the requested pixel
		//		T&				-		Const reference to the colour to set at the gin pixel position (Index)
		//
		//  RETURNS
		//
		//  NOTES
		//

		void	setPixel(const size_t Offset, const T& Colour) {
			//  If the RasterBuffer is invalid or default return NULL
			if (Height == 0 || Width == 0 || Buffer == NULL) return;

			//  If the supplied offset overruns the buffer then return NULL
			if (Offset >= (Height * Width)) return;

			//  Set the pixel to the desired colour and return
			Buffer[Offset] = Colour;
			return;
		}

		//  setPixel
		//
		//  Sets the value of the given Row & Column position to the passed colour.
		//
		//  PARAMETERS
		//
		//		size_t			-		Row index position of the desired pixel
		//		size_t			-		Column index position of the desired pixel
		//		T&				-		Const reference to the colour to set at the given pixel position (Index)
		//
		//  RETURNS
		//
		//  NOTES
		//

		void	setPixel(const size_t R, const size_t C, const T& Colour) {
			//  If the RasterBuffer is invalid or default return NULL
			if (Height == 0 || Width == 0 || Buffer == NULL) return;

			//  If the supplied position is ouside the array bounds then return NULL
			if (R >= Height || C >= Width) return;

			//  Set the pixel to the desired colour and return
			Buffer[(R * Width) + C] = Colour;
			return;
		}

		//  setPixel
		//
		//  Sets the value of the given Row & Column position to the passed colour mixed with the existing colour at 
		//  the given position. The mixing ratio is passed as a double (0.0 - 1.0) where the new colour contributes
		//  mix to the new colour and the old colour contributes (1.0 - mix) to the colour.
		//
		//  PARAMETERS
		//
		//		size_t			-		Row index position of the desired pixel
		//		size_t			-		Column index position of the desired pixel
		//		RGB&			-		Const reference to the RGB colour to set at the given pixel position (Index)
		//		double			-		Mixing factor
		//
		//  RETURNS
		//
		//  NOTES
		//

		void	setPixel(const size_t R, const size_t C, const RGB& Colour, double mix) {
			RGB			MixedColour = {};														//  Mixed colour
			RGB			OldColour = {};															//  Existing colour at pixel position

			//  Boundary conditions
			if (mix == 0.0) return;
			if (mix == 1.0) return setPixel(R, C, Colour);

			//  Mix the colour
			OldColour = Buffer[(R * Width) + C];
			MixedColour.R = BYTE(floor((double(Colour.R) * mix) + 0.5));
			MixedColour.R += BYTE(floor((double(OldColour.R) * (1.0 - mix)) + 0.5));
			MixedColour.G = BYTE(floor((double(Colour.G) * mix) + 0.5));
			MixedColour.G += BYTE(floor((double(OldColour.G) * (1.0 - mix)) + 0.5));
			MixedColour.B = BYTE(floor((double(Colour.B) * mix) + 0.5));
			MixedColour.B += BYTE(floor((double(OldColour.B) * (1.0 - mix)) + 0.5));

			return setPixel(R, C, MixedColour);
		}

		//
		//  Array reference functions
		//

		//  Array Reference []
		//
		//  Returns a reference to the pixel at the given offset (index) in the buffer array
		//
		//  PARAMETERS
		//
		//		size_t			-		Offset (Index) of the requested pixel
		//
		//  RETURNS
		//
		//		T&				-		Reference to the colour of the specified pixel
		//
		//  NOTES
		//
		//		1.		Returns a valid reference to a dummy colour point pixel if the pixel is unavailable
		//

		T&		operator [] (const size_t Offset) {
			T*		pPixel = getPixel(Offset);

			if (pPixel == NULL) pPixel = &DREntry;
			return *pPixel;
		}

		const T&	operator [] (const size_t Offset) const {
			const T*		pPixel = getPixel(Offset);
		
			if (pPixel == NULL) pPixel = &DREntry;
			return *pPixel;
		}

		//  Array Reference (Row,Column)
		//
		//  Returns a reference to the pixel at the given Row & Column position
		//
		//  PARAMETERS
		//
		//		size_t			-		Row index position of the desired pixel
		//		size_t			-		Column index position of the desired pixel
		//
		//  RETURNS
		//
		//		T&				-		Referencer to the colour of the specified pixel
		//
		//  NOTES
		//
		//		1.		Returns a valid reference to a dummy colour point pixel if the pixel is unavailable
		//

		T&		operator () (const size_t R, const size_t C) {
			T*		pPixel = getPixel(R, C);

			if (pPixel == NULL) pPixel = &DREntry;
			return *pPixel;
		}

		const T&	operator () (const size_t R, const size_t C) const {
			const T*		pPixel = getPixel(R, C);
		
			if (pPixel == NULL) pPixel = &DREntry;
			return *pPixel;
		}

		//
		//   Assigment functions
		//

		//  Copy Assignment = operator
		//
		//  This is treated the same as a copy constructor performing a deep copy of the source RasterBuffer
		//
		//  PARAMETERS
		//
		//		RasterBuffer<T>&		-	Constant reference to the source RasterBuffer
		//
		//  RETURNS
		//
		//		RasterBuffer<T>&		-	Reference to the target RasterBuffer
		//
		//  NOTES
		//

		RasterBuffer<T>& operator = (const RasterBuffer& Src) {

			//  Clear the target to the ground state
			Height = 0;
			Width = 0;
			if (Buffer != NULL) free(Buffer);
			Buffer = NULL;

			//  If the source is not valid then return the default object
			if (Src.Height == 0 || Src.Width == 0 || Src.Buffer == NULL) return *this;

			//  Copy the properties over from the source
			Height = Src.Height;
			Width = Src.Width;

			//  Allocate and initialise the underlying storage
			Buffer = (T*) malloc(Height * Width * sizeof(T));

			//  If we failed to allocate storage then treat as per the default constructor
			if (Buffer == NULL) {
				Height = 0;
				Width = 0;
				return *this;
			}

			//  Copy the buffer contents across from the source
			memcpy(Buffer, Src.Buffer, Height * Width * sizeof(T));

			//  Return a reference to the target object
			return *this;
		}

		//  Move Assignment = operator
		//
		//  This is treated the same as a move constructor acquiring ownersip of the underlying content buffer
		//
		//  PARAMETERS
		//
		//		RasterBuffer<T>&&		-	Reference to the source RasterBuffer
		//
		//  RETURNS
		//
		//		RasterBuffer<T>&		-	Reference to the target RasterBuffer
		//
		//  NOTES
		//

		RasterBuffer<T>&	operator = (RasterBuffer&& Src) noexcept {

			//  Clear the target to the ground state
			Height = 0;
			Width = 0;
			if (Buffer != nullptr) free(Buffer);
			Buffer = nullptr;

			//  If the source is not valid then return the default object
			if (Src.Height == 0 || Src.Width == 0 || Src.Buffer == nullptr) return *this;

			//  Copy the properties over from the source
			Height = Src.Height;
			Width = Src.Width;

			//  Acquire ownership of the underlying buffer
			Buffer = Src.Buffer;
			Src.Buffer = nullptr;
			Src.Height = 0;
			Src.Width = 0;

			//  Return a reference to the target
			return *this;
		}

		//  resize
		//
		//  This function will resize the RasterBuffer with the size changes indicated in the passed size vector
		//
		//  PARAMETERS
		//
		//		SizeVector&			-		Reference to the size changes to be made
		//		<T>*				-		(optional) pointer to use to fill additional space
		//
		//  RETURNS
		//
		//		bool				-		true if the image was resized, otherwise false
		//
		//  NOTES
		//

		bool	resize(const SizeVector& Deltas, const T* pFillColour) {
			BoundingBox			bbSource = {};																	//  Source bounding box
			BoundingBox			bbTarget = {};																	//  Target bounding box
			size_t				NewHeight = Height + Deltas.Top + Deltas.Bottom;								//  New image height
			size_t				NewWidth = Width + Deltas.Left + Deltas.Right;									//  New image width

			//  Make sure that we have safe dimensions for the new image
			if (NewHeight > 0 && NewWidth > 0) {

				RasterBuffer<T>		Temp(NewHeight, NewWidth, pFillColour);										//  Temporary target

				//  Compute the source and target bounding boxes
				if (Deltas.Top < 0) bbSource.Top = size_t(0 - Deltas.Top);
				else bbSource.Top = 0;
				if (Deltas.Top > 0) bbTarget.Top = Deltas.Top;
				else bbTarget.Top = 0;

				if (Deltas.Left < 0) bbSource.Left = size_t(0 - Deltas.Left);
				else bbSource.Left = 0;
				if (Deltas.Left > 0) bbTarget.Left = Deltas.Left;
				else bbTarget.Left = 0;

				if (Deltas.Bottom < 0) bbSource.Bottom = (Height + Deltas.Bottom) - 1;
				else bbSource.Bottom = Height - 1;
				
				if (Deltas.Right < 0) bbSource.Right = (Width + Deltas.Right) - 1;
				else bbSource.Right = Width - 1;

				bbTarget.Bottom = bbTarget.Top + (bbSource.Bottom - bbSource.Top);
				bbTarget.Right = bbTarget.Left + (bbSource.Right - bbSource.Left);

				//
				//  Perform a Row/Column iteration copy from the source region to the target region
				//

				iterator	ITR = Temp.top(bbTarget);

				for (iterator ISR = top(bbSource); ISR != bottom(bbSource); ISR++) {
					iterator	ITC = Temp.left(ITR);
					for (iterator ISC = left(ISR); ISC != right(ISR); ISC++) {
						*ITC = *ISC;
						ITC++;
					}
					ITR++;
				}

				//  Move the resulting image back over the original
				*this = Temp;

				//  Return showing resize was successful
				return true;
			}

			//  Show resize failed
			return false;
		}

		//  clear
		//
		//  This function will clear the entire image to the passed colour
		//
		//  PARAMETERS
		//
		//		T&			-		Const reference to the colour to clear the image to
		//
		//  RETURNS
		//
		//  NOTES
		//

		void		clear(const T& To) {
			BoundingBox		bbAll = {};													//  Bounding box for complete image

			//  Set the bounding box to the complete image
			bbAll.Top = 0;
			bbAll.Left = 0;
			bbAll.Bottom = Height - 1;
			bbAll.Right = Width - 1;

			return clear(bbAll, To);
		}
		 
		//  clear
		//
		//  This function will clear a specified region of the image to the passed colour
		//
		//  PARAMETERS
		//
		//		BoundingBox&	-		Reference to the region to be cleared
		//		T&				-		Const reference to the colour to clear the image to
		//
		//  RETURNS
		//
		//  NOTES
		//

		void		clear(BoundingBox& Region, const T& To) {

			//  Check the Region
			if (Region.Top > Region.Bottom) return;
			if (Region.Left > Region.Right) return;
			if (Region.Bottom >= Height) return;
			if (Region.Right >= Width) return;

			//  Use a bounded linear iterator to traverse the region and set the colour
			for (iterator It = begin(Region); It != end(Region); It++) *It = To;

			//  Return to caller
			return;
		}

		//  blit
		//
		//  This function will perform a BLock Image Transfer from the passed source image into the current image.
		//
		//  PARAMETERS
		//
		//		RasterBuffer&		-		Const reference to the source image
		//		size_t				-		Row target for the origin of the block in the target image
		//		size_t				-		Column target for the origin of the block in the target image
		//
		//  RETURNS
		//
		//  NOTES
		//

		void	blit(const RasterBuffer<T>& SrcImg, size_t OR, size_t OC) {
			BoundingBox		bbRegion = {};													//   Source region bounding box

			//  Setup the bounding box to describe the complete image
			bbRegion.Top = 0;
			bbRegion.Left = 0;
			bbRegion.Bottom = SrcImg.getHeight() - 1;
			bbRegion.Right = SrcImg.getWidth() - 1;

			//  Return the blit of the region
			return blit(SrcImg, bbRegion, OR, OC);
		}

		//  blit
		//
		//  This function will perform a BLock Image Transfer from the passed source image into the current image.
		//  This version of the function honours the Transparent (Green Screen) colour passed.
		//
		//  PARAMETERS
		//
		//		RasterBuffer&		-		Const reference to the source image
		//		size_t				-		Row target for the origin of the block in the target image
		//		size_t				-		Column target for the origin of the block in the target image
		//		C&					-		Const reference to the Green Screen (Transparent) Colour
		//
		//  RETURNS
		//
		//  NOTES
		//

		void	blit(const RasterBuffer<T>& SrcImg, size_t OR, size_t OC, const T& GreenScreen) {
			BoundingBox		bbRegion = {};													//   Source region bounding box

			//  Setup the bounding box to describe the complete image
			bbRegion.Top = 0;
			bbRegion.Left = 0;
			bbRegion.Bottom = SrcImg.getHeight() - 1;
			bbRegion.Right = SrcImg.getWidth() - 1;

			//  Return the blit of the region
			return blit(SrcImg, bbRegion, OR, OC, GreenScreen);
		}

		//  blit
		//
		//  This function will perform a BLock Image Transfer from the passed source image region into the current image.
		//
		//  PARAMETERS
		//
		//		RasterBuffer&		-		Const reference to the source image
		//		BoundingBox&		-		Reference to the source bounding box describing the region to be copied
		//		size_t				-		Row target for the origin of the block in the target image
		//		size_t				-		Column target for the origin of the block in the target image
		//
		//  RETURNS
		//
		//  NOTES
		//

		void	blit(const RasterBuffer<T>& SrcImg, BoundingBox& Region, size_t OR, size_t OC) {
			BoundingBox		TargetRegion = {};												//  Target region of the current image

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
			//  Perform a Row/Column iteration copy from the source region to the target region
			//

			iterator	ITR = top(TargetRegion);

			for (const_iterator ISR = SrcImg.top(Region); ISR != SrcImg.bottom(Region); ISR++) {
				iterator	ITC = left(ITR);
				for (const_iterator ISC = SrcImg.left(ISR); ISC != SrcImg.right(ISR); ISC++) {
					*ITC = *ISC;
					ITC++;
				}
				ITR++;
			}

			//  Return to caller
			return;
		}

		//  blit
		//
		//  This function will perform a BLock Image Transfer from the passed source image region into the current image.
		//  This version of the function honours the Transparent (Green Screen) colour passed.
		//
		//  PARAMETERS
		//
		//		RasterBuffer&		-		Const reference to the source image
		//		BoundingBox&		-		Reference to the source bounding box describing the region to be copied
		//		size_t				-		Row target for the origin of the block in the target image
		//		size_t				-		Column target for the origin of the block in the target image
		//		C&					-		Const reference to the Green Screen (Transparent) Colour
		//
		//  RETURNS
		//
		//  NOTES
		//

		void	blit(const RasterBuffer<T>& SrcImg, BoundingBox& Region, size_t OR, size_t OC, const T& GreenScreen) {
			BoundingBox		TargetRegion = {};												//  Target region of the current image

			//  Safety
			if (SrcImg.getHeight() == 0 || SrcImg.getWidth() == 0 || SrcImg.getArray() == NULL) return;
			if (Height == 0 || Width == 0 || Buffer == NULL) return;
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
			//  Perform a Row/Column iteration copy from the source region to the target region
			//

			iterator	ITR = top(TargetRegion);

			for (const_iterator ISR = SrcImg.top(Region); ISR != SrcImg.bottom(Region); ISR++) {
				iterator	ITC = left(ITR);
				for (const_iterator ISC = SrcImg.left(ISR); ISC != SrcImg.right(ISR); ISC++) {
					if (*ISC != GreenScreen) *ITC = *ISC;
					ITC++;
				}
				ITR++;
			}

			//  Return to caller
			return;
		}


		//  replaceColour
		//
		//  This function will replace all occurrence of the given colour with another.
		//
		//  PARAMETERS
		//
		//		T&					-		Const reference to the colour to be replaced
		//		T&					-		Const reference to the colour to replace the source colour
		//
		//  RETURNS
		//
		//  NOTES
		//

		void		replaceColour(const T& From, const T& To) {
			BoundingBox			BBRep = {};												//  Bounding box for image

			BBRep.Top = 0;
			BBRep.Left = 0;
			BBRep.Bottom = getHeight() - 1;
			BBRep.Right = getWidth() - 1;
			replaceColour(From, To, BBRep);
			return;
		}

		//  replaceColour
		//
		//  This function will replace all occurrence of the given colour with another within the given bounds
		//
		//  PARAMETERS
		//
		//		T&					-		Const reference to the colour to be replaced
		//		T&					-		Const reference to the colour to replace the source colour
		//		BoundingBox&		-		Const reference to the bounds within which to replace the colour
		//
		//  RETURNS
		//
		//  NOTES
		//

		void		replaceColour(const T& From, const T& To, const BoundingBox& Bounds) {

			for (iterator ISR = top(Bounds); ISR != bottom(Bounds); ISR++) {
				for (iterator ISC = left(ISR); ISC != right(ISR); ISC++) {
					if (*ISC == From) *ISC = To;	
				}
			}

			//  Return to caller
			return;
		}

		//  flood
		//
		//  This function will replace all occurrence of the colour at the given co-ordinates with the supplied replacement
		//  colour in a flood fill operation.
		//
		//  PARAMETERS
		//
		//		size_t				-		Starting co-ordinate Row
		//		size_t				-		Starting co-ordinate Column
		//		T&					-		Const reference to the colour to replace the source colour
		//
		//  RETURNS
		//
		//  NOTES
		//

		void		flood(size_t R, size_t C, const T& To) {
			BoundingBox			BBRep = {};												//  Bounding box for image

			BBRep.Top = 0;
			BBRep.Left = 0;
			BBRep.Bottom = getHeight() - 1;
			BBRep.Right = getWidth() - 1;
			return flood(R, C, To, BBRep);
		}

		//  flood
		//
		//  This function will replace all occurrence of the colour at the given co-ordinates with the supplied replacement
		//  colour in a flood fill operation, the flooding is confined to the given bounds.
		//
		//  PARAMETERS
		//
		//		size_t				-		Starting co-ordinate Row
		//		size_t				-		Starting co-ordinate Column
		//		T&					-		Const reference to the colour to replace the source colour
		//		BoundingBox&		-		Reference to the Bounding box constraining the flooding
		//
		//  RETURNS
		//
		//  NOTES
		//

		void		flood(size_t R, size_t C, const T& To, BoundingBox& Within) {
			T		Source = *getPixel(R, C);											//  Original Colour
			struct PPOS {
				size_t			R;														//  Pixel Row
				size_t			C;														//  Pixel Column
			}  PixelAt = {R, C};
			PPOS				Candidate = {};											//  Candidate Pixel
			std::stack<PPOS>	Stack;													//  Stack of pixels

			//  Push the original source co-ordinates onto the stack
			Stack.push(PixelAt);

			//  Process the contents of the stack until it is empty
			while (!Stack.empty()) {
				//  Pop the next pixel to process off the stack
				PixelAt = Stack.top();
				Stack.pop();
				//  Replace the colour
				setPixel(PixelAt.R, PixelAt.C, To);
				
				//  Add adjoining pixels that match to the stack
				Candidate = PixelAt;
				if (Candidate.R > Within.Top) {
					//  Row above the target pixel
					Candidate.R--;
					if (*getPixel(Candidate.R, Candidate.C) == Source) Stack.push(Candidate);
					if (Candidate.C > Within.Left) {
						Candidate.C--;
						if (*getPixel(Candidate.R, Candidate.C) == Source) Stack.push(Candidate);
						Candidate.C++;
					}
					if (Candidate.C < Within.Right) {
						Candidate.C++;
						if (*getPixel(Candidate.R, Candidate.C) == Source) Stack.push(Candidate);
						Candidate.C--;
					}
					Candidate.R++;
				}

				//  Same row as target pixel
				if (Candidate.C > Within.Left) {
					Candidate.C--;
					if (*getPixel(Candidate.R, Candidate.C) == Source) Stack.push(Candidate);
					Candidate.C++;
				}
				if (Candidate.C < Within.Right) {
					Candidate.C++;
					if (*getPixel(Candidate.R, Candidate.C) == Source) Stack.push(Candidate);
					Candidate.C--;
				}

				if (Candidate.R < Within.Bottom) {
					//  Row below the target pixel
					Candidate.R++;
					if (*getPixel(Candidate.R, Candidate.C) == Source) Stack.push(Candidate);
					if (Candidate.C > Within.Left) {
						Candidate.C--;
						if (*getPixel(Candidate.R, Candidate.C) == Source) Stack.push(Candidate);
						Candidate.C++;
					}
					if (Candidate.C < Within.Right) {
						Candidate.C++;
						if (*getPixel(Candidate.R, Candidate.C) == Source) Stack.push(Candidate);
						Candidate.C--;
					}
					Candidate.R--;
				}
			}

			//  Return to caller
			return;
		}

		//  scanFill
		//
		//  This function will replace all occurrence of the colour at the given co-ordinates with the supplied replacement
		//  colour in a flood fill operation, scanFill differs from flood in that only the von Neumann neighbourhood is
		//  considered for candidates (above, below, left and right).
		//
		//  PARAMETERS
		//
		//		size_t				-		Starting co-ordinate Row
		//		size_t				-		Starting co-ordinate Column
		//		T&					-		Const reference to the colour to replace the source colour
		//
		//  RETURNS
		//
		//  NOTES
		//

		void		scanFill(size_t R, size_t C, const T& To) {
			BoundingBox			BBRep = {};												//  Bounding box for image

			BBRep.Top = 0;
			BBRep.Left = 0;
			BBRep.Bottom = getHeight() - 1;
			BBRep.Right = getWidth() - 1;
			return scanFill(R, C, To, BBRep);
		}

		//  scanFill
		//
		//  This function will replace all occurrence of the colour at the given co-ordinates with the supplied replacement
		//  colour in a flood fill operation, scanFill differs from flood in that only the von Neumann neighbourhood is
		//  considered for candidates (above, below, left and right).
		//
		//  PARAMETERS
		//
		//		size_t				-		Starting co-ordinate Row
		//		size_t				-		Starting co-ordinate Column
		//		T&					-		Const reference to the colour to replace the source colour
		//
		//  RETURNS
		//
		//  NOTES
		//

		void		scanFill(size_t R, size_t C, const T& To, BoundingBox& Within) {
			T		Source = *getPixel(R, C);											//  Original Colour
			struct PPOS {
				size_t			R;														//  Pixel Row
				size_t			C;														//  Pixel Column
			}  PixelAt = { R, C };
			PPOS				Candidate = {};											//  Candidate Pixel
			std::stack<PPOS>	Stack;													//  Stack of pixels

			//  Push the original source co-ordinates onto the stack
			Stack.push(PixelAt);

			//  Process the contents of the stack until it is empty
			while (!Stack.empty()) {
				//  Pop the next pixel to process off the stack
				PixelAt = Stack.top();
				Stack.pop();
				//  Replace the colour
				setPixel(PixelAt.R, PixelAt.C, To);

				//  Add adjoining pixels that match to the stack
				Candidate = PixelAt;
				if (Candidate.R > Within.Top) {
					//  Row above the target pixel
					Candidate.R--;
					if (*getPixel(Candidate.R, Candidate.C) == Source) Stack.push(Candidate);
					Candidate.R++;
				}

				//  Same row as target pixel
				if (Candidate.C > Within.Left) {
					Candidate.C--;
					if (*getPixel(Candidate.R, Candidate.C) == Source) Stack.push(Candidate);
					Candidate.C++;
				}
				if (Candidate.C < Within.Right) {
					Candidate.C++;
					if (*getPixel(Candidate.R, Candidate.C) == Source) Stack.push(Candidate);
					Candidate.C--;
				}

				if (Candidate.R < Within.Bottom) {
					//  Row below the target pixel
					Candidate.R++;
					if (*getPixel(Candidate.R, Candidate.C) == Source) Stack.push(Candidate);
					Candidate.R--;
				}
			}

			//  Return to caller
			return;
		}

		//
		//  Image Transformation Functions
		//

		//  rotate
		//
		//  This function will rotate the image by the given number of degrees (right-angles).
		//
		//  PARAMETERS
		//
		//		size_t				-		Rotation Degrees 0, 90, 180, 270
		//
		//  RETURNS
		//
		//  NOTES
		//

		void		rotate(size_t Degrees) {
			size_t				RA = (Degrees / 90) % 4;										//  Number of right angles to rotate
			RasterBuffer<T>*	Replacement = nullptr;											//  Replacement content

			//  Create the target raster buffer
			if (RA == 0) return;
			if (RA == 2) Replacement = new RasterBuffer<T>(Height, Width, nullptr);
			else Replacement = new RasterBuffer<T>(Width, Height, nullptr);

			iterator TRIt = Replacement->top();													//  Target iterators
			iterator TCIt = Replacement->left(TRIt);

			switch (RA) {

			case 1:
				//  Rotate clockwise by 90 degrees
				//  Iterate over the source copying each pixel to the target
				for (iterator SCIt = left(); SCIt != right(); SCIt++) {
					for (iterator SRIt = bottom(SCIt); SRIt != top(SCIt); SRIt--) {
						*TCIt = *SRIt;
						TCIt++;
					}
					TRIt++;
					TCIt.reset();
				}
				break;

			case 2:
				//  Rotate clockwise by 180 degrees
				//  Iterate over the source copying each pixel to the target
				for (iterator SRIt = bottom(); SRIt != top(); SRIt--) {
					for (iterator SCIt = right(SRIt); SCIt != left(SRIt); SCIt--) {
						*TCIt = *SRIt;
						TCIt++;
					}
					TRIt++;
					TCIt.reset();
				}
				break;

			case 3:
				//  Rotate clockwise by 270 degrees
				//  Iterate over the source copying each pixel to the target
				for (iterator SCIt = right(); SCIt != left(); SCIt--) {
					for (iterator SRIt = top(SCIt); SRIt != bottom(SCIt); SRIt++) {
						*TCIt = *SRIt;
						TCIt++;
					}
					TRIt++;
					TCIt.reset();
				}
				break;
			}

			//  Move the content of the replacemet buffer to this buffer
			*this = std::move(*Replacement);
			delete Replacement;

			//  Return to caller
			return;
		}

		//  flipHorizontal
		//
		//  This function will flip the image left to right
		//
		//  PARAMETERS
		//
		//  RETURNS
		//
		//  NOTES
		//

		void	flipHorizontal() {
			RasterBuffer<T>* Replacement = new RasterBuffer<T>(Height, Width, nullptr);				//  Replacement content

			iterator TRIt = Replacement->top();														//  Target iterators
			iterator TCIt = Replacement->left(TRIt);

			//  Copy the source to the target - reversing columns
			for (iterator SRIt = top(); SRIt != bottom(); SRIt++) {
				for (iterator SCIt = right(SRIt); SCIt != left(SRIt); SCIt--) {
					*TCIt = *SCIt;
					TCIt++;
				}
				TRIt++;
				TCIt.reset();
			}

			//  Move the content of the replacemet buffer to this buffer
			*this = std::move(*Replacement);
			delete Replacement;

			//  Return to caller
			return;
		}

		//  flipVertical
		//
		//  This function will flip the image top to bottom
		//
		//  PARAMETERS
		//
		//  RETURNS
		//
		//  NOTES
		//

		void	flipVertical() {
			RasterBuffer<T>* Replacement = new RasterBuffer<T>(Height, Width, nullptr);				//  Replacement content

			iterator TRIt = Replacement->top();														//  Target iterators
			iterator TCIt = Replacement->left(TRIt);

			//  Copy the source to the target - reversing rows
			for (iterator SRIt = bottom(); SRIt != top(); SRIt--) {
				for (iterator SCIt = left(SRIt); SCIt != right(SRIt); SCIt++) {
					*TCIt = *SCIt;
					TCIt++;
				}
				TRIt++;
				TCIt.reset();
			}

			//  Move the content of the replacemet buffer to this buffer
			*this = std::move(*Replacement);
			delete Replacement;

			//  Return to caller
			return;
		}

		//
		//  Image comparison primitives
		//

		//  matches
		//
		//  This function will find the difference between this Raster Buffer and the passed buffer. 
		//  On mismatch it will identify the bounds of the canvas that do not match.
		//
		//  PARAMETERS
		//
		//		RasterBuffer&				-		Reference to the Raster Buffer to be compared
		//		BoundingBox&				-		Reference to the bounds of the image that do NOT match
		//
		//  RETURNS
		//
		//		bool						-		true if the image matches completely, otherwise false
		//
		//  NOTES
		//

		bool	matches(RasterBuffer<T>& Comp, BoundingBox& Diff) {
			bool			DiffDetected = false;											//  Difference detected

			//  Clear the bounding box
			Diff.Top = 0;
			Diff.Bottom = 0;
			Diff.Left = 0;
			Diff.Right = 0;

			//  If the dimensions of the two images differ then return a single difference region with the complete image
			if (Comp.getHeight() != Height || Comp.getWidth() != Width) {
				Diff.Bottom = Height - 1;
				Diff.Right = Width - 1;
				return false;
			}

			//  Make passes from each edge of the image to determine the outline extents of the differences
			//  Top
			DiffDetected = false;
			Diff.Top = Height;
			for (iterator RIt = top(); RIt != bottom(); RIt++) {
				for (iterator CIt = left(RIt); CIt != right(RIt); CIt++) {
					if (*CIt != *(Comp.getPixel(RIt.getIndex(), CIt.getIndex()))) {
						DiffDetected = true;
						break;
					}
				}
				if (DiffDetected) {
					Diff.Top = RIt.getIndex();
					break;
				}
			}

			//  NO Difference Detected
			if (Diff.Top == Height) return true;

			//  Bottom
			DiffDetected = false;
			for (iterator RIt = bottom(); RIt != top(); RIt--) {
				for (iterator CIt = left(RIt); CIt != right(RIt); CIt++) {
					if (*CIt != *(Comp.getPixel(RIt.getIndex(), CIt.getIndex()))) {
						DiffDetected = true;
						break;
					}
				}
				if (DiffDetected) {
					Diff.Bottom = RIt.getIndex();
					break;
				}
			}

			//  Left
			DiffDetected = false;
			for (iterator CIt = left(); CIt != right(); CIt++) {
				for (iterator RIt = top(CIt); RIt != bottom(CIt); RIt++) {
					if (*RIt != *(Comp.getPixel(RIt.getIndex(), CIt.getIndex()))) {
						DiffDetected = true;
						break;
					}
				}
				if (DiffDetected) {
					Diff.Left = CIt.getIndex();
					break;
				}
			}

			//  Right
			DiffDetected = false;
			for (iterator CIt = right(); CIt != left(); CIt--) {
				for (iterator RIt = top(CIt); RIt != bottom(CIt); RIt++) {
					if (*RIt != *(Comp.getPixel(RIt.getIndex(), CIt.getIndex()))) {
						DiffDetected = true;
						break;
					}
				}
				if (DiffDetected) {
					Diff.Right = CIt.getIndex();
					break;
				}
			}

			//  Show not matching
			return false;
		}

		//  mapDifference
		//
		//  This function will return a RasterBuffer of BYTES that identify the differences between the passed test image and this.
		//
		//  PARAMETERS
		//
		//		RasterBuffer&	-		Reference to the test Raster Buffer
		//		size_t&			-		Reference to the Difference Pixel Counter
		//
		//  RETURNS
		//
		//		RasterBuffer*	-		Pointer to the difference map, nullptr if it could not be produced
		//
		//  NOTES
		//

		RasterBuffer<BYTE>* mapDifference(const RasterBuffer<RGB>& Comp, size_t& DiffCount) {
			BYTE					Matched = 0x00;													//  Pixel Matches
			BYTE					Mismatched = 0x01;												//  Pixel Mismatch
			RasterBuffer<BYTE>*		pDiffBuffer = nullptr;											//  Difference map buffer

			//  Clear difference count
			DiffCount = 0;

			//  Safety check that both images have the same dimensions
			if (Comp.getHeight() != Height || Comp.getWidth() != Width) return nullptr;

			//  Create the difference map raster buffer, initialise each pixel to Matched.
			pDiffBuffer = new RasterBuffer<BYTE>(Height, Width, &Matched);
			if (pDiffBuffer == nullptr) return nullptr;

			//  Scan the comparator image comparing pixels to the corresponding pixel in the this image if not matched set
			//  the corresponding pixel in the difference map to Mismatched and count the mismatches.

			//  Iterator pair for the this image
			RasterBuffer<RGB>::iterator RefRit = top();
			RasterBuffer<RGB>::iterator RefCit = left(RefRit);

			//  Iterator pair for the Difference Map
			RasterBuffer<BYTE>::iterator DMRit = pDiffBuffer->top();
			RasterBuffer<BYTE>::iterator DMCit = pDiffBuffer->left(DMRit);

			//  Scanning (2d) loop over the comparator image.
			//  Reference image and Difference Map iterators must be synchronised with the test image iterators
			for (RasterBuffer<RGB>::const_iterator CompRit = Comp.top(); CompRit != Comp.bottom(); CompRit++) {
				for (RasterBuffer<RGB>::const_iterator CompCit = Comp.left(CompRit); CompCit != Comp.right(CompRit); CompCit++) {
					if (*CompCit != *RefCit) {
						*DMCit = Mismatched;
						DiffCount++;
					}

					//  Synch Ref & Diff Map iterators
					RefCit++;
					DMCit++;
				}

				//  Synch Ref & Diff Map iterators
				RefCit.flyback();
				RefRit++;
				DMCit.flyback();
				DMRit++;
			}

			return pDiffBuffer;
		}

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Iterator Creation/Initialisation functions      (non-const)                                                   *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  Reference type definitions
		typedef CompoundIterator		iterator;
		typedef	ConstCompoundIterator	const_iterator;

		//  Linear iterator treats the container as a one dimensional array which may be iterated bi-directionally
		iterator begin() {
			BoundingBox		Region = { 0, 0, Height - 1, Width - 1};
			return iterator(*this, Region, iterator::STYLE_LINEAR_FWD);
		}

		iterator end() {
			BoundingBox		Region = { 0, 0, Height - 1, Width - 1};
			return iterator(*this, Region, iterator::STYLE_LINEAR_BKWD);
		}

		//  Region Linear iterator treats the container region as a one dimensional array which may be iterated bi-directionally
		iterator begin(const BoundingBox& Region) { return iterator(*this, Region, iterator::STYLE_LINEAR_FWD); }
		iterator end(const BoundingBox& Region) { return iterator(*this, Region, iterator::STYLE_LINEAR_BKWD); }

		//  Scan line iterator and sub-iterator treats the container as a two dimensional array both of which may be iterated bi-directionally
		iterator top() {
			BoundingBox		Region = { 0, 0, Height - 1, Width - 1};
			return iterator(*this, Region, iterator::STYLE_SCANROW_TB);
		}

		iterator bottom() {
			BoundingBox		Region = { 0, 0, Height - 1, Width - 1};
			return iterator(*this, Region, iterator::STYLE_SCANROW_BT);
		}

		iterator left() {
			BoundingBox		Region = { 0, 0, Height - 1, Width - 1};
			return iterator(*this, Region, iterator::STYLE_SCANCOL_LR);
		}

		iterator right() {
			BoundingBox		Region = { 0, 0, Height - 1, Width - 1};
			return iterator(*this, Region, iterator::STYLE_SCANCOL_RL);
		}

		iterator top(const iterator& PIT) { return iterator(PIT, iterator::STYLE_SUBROW_TB); }
		iterator bottom(const iterator& PIT) { return iterator(PIT, iterator::STYLE_SUBROW_BT); }
		iterator left(const iterator& PIT) { return iterator(PIT, iterator::STYLE_SUBCOL_LR); }
		iterator right(const iterator& PIT) { return iterator(PIT, iterator::STYLE_SUBCOL_RL); }

		//  Region Scan line iterator treats the container as a two dimensional array both of which may be iterated bi-directionally
		iterator top(const BoundingBox& Region) { return iterator(*this, Region, iterator::STYLE_SCANROW_TB); }
		iterator bottom(const BoundingBox& Region) { return iterator(*this, Region, iterator::STYLE_SCANROW_BT); }
		iterator left(const BoundingBox& Region) { return iterator(*this, Region, iterator::STYLE_SCANCOL_LR); }
		iterator right(const BoundingBox& Region) { return iterator(*this, Region, iterator::STYLE_SCANCOL_RL); }

		//  Orbital iterator and sub-iterator treats the container as a series of concentric orbits
		iterator outer() {
			BoundingBox		Region = { 0, 0, Height - 1, Width - 1};
			return iterator(*this, Region, iterator::STYLE_ORBIT_IN);
		}

		iterator inner() {
			BoundingBox		Region = { 0, 0, Height - 1, Width - 1};
			return iterator(*this, Region, iterator::STYLE_ORBIT_OUT);
		}

		iterator start(const iterator& PIT) { return iterator(PIT, iterator::STYLE_SUBORBIT_CLOCK); }
		iterator finish(const iterator& PIT) { return iterator(PIT, iterator::STYLE_SUBORBIT_ANTICLOCK); }

		//  Region Orbital iterator and sub-iterator treats the container as a series of concentric orbits
		iterator outer(const BoundingBox& Region) { return iterator(*this, Region, iterator::STYLE_ORBIT_IN); }
		iterator inner(const BoundingBox& Region) { return iterator(*this, Region, iterator::STYLE_ORBIT_OUT); }

		//  Neighbourhood sub-iterators provide access to Moore or Von Neumann neibourhoods of the current position of an iterator
		iterator hood(const iterator& PIT) { return iterator(PIT, iterator::STYLE_NEIGHBOURHOOD_MOORE); }
		iterator vhood(const iterator& PIT) { return iterator(PIT, iterator::STYLE_NEIGHBOURHOOD_VN); }
		iterator endhood(const iterator& PIT) { return iterator(PIT, iterator::STYLE_NEIGHBOURHOOD_MEND); }
		iterator endvhood(const iterator& PIT) { return iterator(PIT, iterator::STYLE_NEIGHBOURHOOD_VEND); }
		iterator hood2(const iterator& PIT) { return iterator(PIT, iterator::STYLE_NEIGHBOURHOOD_MOORE2); }
		iterator endhood2(const iterator& PIT) { return iterator(PIT, iterator::STYLE_NEIGHBOURHOOD_MEND2); }

		//  MCU Sampling Iterators provide access to defined blobs of the image in the container
		iterator firstMCU(BYTE FF) {
			BoundingBox		Region = { 0, 0, Height - 1, Width - 1};
			SWITCHES		Style = iterator::STYLE_MCU_FWD;
			if (FF == 0x12) Style = iterator::STYLE_MCU12_FWD;
			else if (FF == 0x21) Style = iterator::STYLE_MCU21_FWD;
			else if (FF == 0x11) Style = iterator::STYLE_MCU11_FWD;
			return iterator(*this, Region, Style);
		}

		iterator lastMCU(BYTE FF) {
			BoundingBox		Region = { 0, 0, Height - 1, Width - 1};
			SWITCHES		Style = iterator::STYLE_MCU_BKWD;
			if (FF == 0x12) Style = iterator::STYLE_MCU12_BKWD;
			else if (FF == 0x21) Style = iterator::STYLE_MCU21_BKWD;
			else if (FF == 0x11) Style = iterator::STYLE_MCU11_BKWD;
			return iterator(*this, Region, Style);
		}

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Iterator Creation/Initialisation functions      (Const)		                                                *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  Linear iterator treats the container as a one dimensional array which may be iterated bi-directionally
		const_iterator begin() const {
			BoundingBox		Region = { 0, 0, Height - 1, Width - 1};
			return const_iterator(*this, Region, const_iterator::STYLE_LINEAR_FWD);
		}

		const_iterator end() const {
			BoundingBox		Region = { 0, 0, Height - 1, Width - 1};
			return const_iterator(*this, Region, const_iterator::STYLE_LINEAR_BKWD);
		}

		//  Region Linear iterator treats the container as a one dimensional array which may be iterated bi-directionally
		const_iterator begin(const BoundingBox& Region) const { return const_iterator(*this, Region, const_iterator::STYLE_LINEAR_FWD); }
		const_iterator end(const BoundingBox& Region) const { return const_iterator(*this, Region, const_iterator::STYLE_LINEAR_BKWD); }

		//  Scan line iterator and sub-iterator treats the container as a two dimensional array both of which may be iterated bi-directionally
		const_iterator top() const {
			BoundingBox		Region = { 0, 0, Height - 1, Width - 1};
			return const_iterator(*this, Region, const_iterator::STYLE_SCANROW_TB);
		}

		const_iterator bottom() const {
			BoundingBox		Region = { 0, 0, Height - 1, Width - 1};
			return const_iterator(*this, Region, const_iterator::STYLE_SCANROW_BT);
		}

		const_iterator left() const {
			BoundingBox		Region = { 0, 0, Height - 1, Width - 1};
			return const_iterator(*this, Region, const_iterator::STYLE_SCANCOL_LR);
		}

		const_iterator right() const {
			BoundingBox		Region = { 0, 0, Height - 1, Width - 1};
			return const_iterator(*this, Region, const_iterator::STYLE_SCANCOL_RL);
		}

		const_iterator top(const const_iterator& PIT) const { return const_iterator(PIT, const_iterator::STYLE_SUBROW_TB); }
		const_iterator bottom(const const_iterator& PIT) const { return const_iterator(PIT, const_iterator::STYLE_SUBROW_BT); }
		const_iterator left(const const_iterator& PIT) const { return const_iterator(PIT, const_iterator::STYLE_SUBCOL_LR); }
		const_iterator right(const const_iterator& PIT) const { return const_iterator(PIT, const_iterator::STYLE_SUBCOL_RL); }

		//  Region Scan line iterator treats the container as a two dimensional array both of which may be iterated bi-directionally
		const_iterator top(const BoundingBox& Region) const { return const_iterator(*this, Region, const_iterator::STYLE_SCANROW_TB); }
		const_iterator bottom(const BoundingBox& Region) const { return const_iterator(*this, Region, const_iterator::STYLE_SCANROW_BT); }
		const_iterator left(const BoundingBox& Region) const { return const_iterator(*this, Region, const_iterator::STYLE_SCANCOL_LR); }
		const_iterator right(const BoundingBox& Region) const { return const_iterator(*this, Region, const_iterator::STYLE_SCANCOL_RL); }


		//  Orbital iterator and sub-iterator treats the container as a series of concentric orbits
		const_iterator outer() const {
			BoundingBox		Region = { 0, 0, Height - 1, Width - 1};
			return const_iterator(*this, Region, const_iterator::STYLE_ORBIT_IN);
		}

		const_iterator inner() const {
			BoundingBox		Region = { 0, 0, Height - 1, Width - 1};
			return const_iterator(*this, Region, const_iterator::STYLE_ORBIT_OUT);
		}

		const_iterator start(const const_iterator& PIT) const { return const_iterator(PIT, const_iterator::STYLE_SUBORBIT_CLOCK); }
		const_iterator finish(const const_iterator& PIT) const { return const_iterator(PIT, const_iterator::STYLE_SUBORBIT_ANTICLOCK); }

		//  Region Orbital iterator and sub-iterator treats the container as a series of concentric orbits
		const_iterator outer(const BoundingBox& Region) const { return const_iterator(*this, Region, const_iterator::STYLE_ORBIT_IN); }
		const_iterator inner(const BoundingBox& Region) const { return const_iterator(*this, Region, const_iterator::STYLE_ORBIT_OUT); }

		//  Neighbourhood iterators
		const_iterator hood(const const_iterator& PIT) const { return const_iterator(PIT, const_iterator::STYLE_NEIGHBOURHOOD_MOORE); }
		const_iterator vhood(const const_iterator& PIT) const { return const_iterator(PIT, const_iterator::STYLE_NEIGHBOURHOOD_VN); }
		const_iterator endhood(const const_iterator& PIT) const { return const_iterator(PIT, const_iterator::STYLE_NEIGHBOURHOOD_MEND); }
		const_iterator envdhood(const const_iterator& PIT) const { return const_iterator(PIT, const_iterator::STYLE_NEIGHBOURHOOD_VEND); }
		const_iterator hood2(const const_iterator& PIT) const { return const_iterator(PIT, const_iterator::STYLE_NEIGHBOURHOOD_MOORE2); }
		const_iterator endhood2(const const_iterator& PIT) const { return const_iterator(PIT, const_iterator::STYLE_NEIGHBOURHOOD_MEND2); }


		//  MCU Sampling Iterators
		const_iterator firstMCU() const {
			BoundingBox		Region = { 0, 0, Height - 1, Width - 1};
			return const_iterator(*this, Region, iterator::STYLE_MCU_FWD);
		}

		const_iterator LastMCU() const {
			BoundingBox		Region = { 0, 0, Height - 1, Width - 1};
			return const_iterator(*this, Region, iterator::STYLE_MCU_BKWD);
		}

	protected:

		//*******************************************************************************************************************
		//*																													*
		//*  Private Members																								*
		//*																													*
		//*******************************************************************************************************************

		//  Image Properties

		size_t			Height;																		//  Image height
		size_t			Width;																		//  Image width

		//  Image Storage Array

		T*				Buffer;																		//  Image storage

	private:

		//*******************************************************************************************************************
		//*																													*
		//*  Private Members																								*
		//*																													*
		//*******************************************************************************************************************

		//  Dummy colour point for NULL Reference Protection

		T				DREntry;																	//  Dummy entry

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Private Functions                                                                                             *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  decorateColumns
		//
		//  This function is used by the document() function to show the column number decoration
		//
		//  PARAMETERS
		//
		//		size_t				-		Width of the image
		//		size_t				-		Print Width for the row numbers
		//		std::osteam&		-		Reference to the output stream on which to write
		//
		//  RETURNS
		//
		//  NOTES
		//

		void		decorateColumns(size_t IW, size_t PrintWidth, std::ostream& OS) {
			size_t			Lines = PrintWidth;																	//  Number of lines to print
			size_t			iIndex = 0, jIndex = 0;																//  Generic indexes
			size_t			SLim = size_t(pow(10, Lines));														//  Switch Limit
			size_t			Counter = 0;																		//  Counter
			size_t			Value = 0;																			//  Value to print

			for (iIndex = 0; iIndex < Lines; iIndex++)
			{
				for (jIndex = 0; jIndex < size_t(PrintWidth + 6); jIndex++) OS << " ";
				Value = 0;
				Counter = 0;
				SLim = SLim / 10;
				for (jIndex = 0; jIndex < IW; jIndex++)
				{
					OS << Value;
					Counter++;
					if (Counter == SLim)
					{
						Value++;
						if (Value == 10) Value = 0;
						Counter = 0;
					}
				}
				OS << std::endl;
			}

			OS << std::endl;

			//  Return to caller
			return;
		}

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Nested Classes																								*
		//*                                                                                                                 *
		//*******************************************************************************************************************

		public:

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   CompoundIterator Class																						*
		//*                                                                                                                 *
		//*   The CompoundIterator  provides a collection of different iterators that iterate over the content of the       *
		//*   RasterBuffer.                                                                                                 *
		//*                                                                                                                 *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		class CompoundIterator : public CIBase2D {
		public:

			//*******************************************************************************************************************
			//*                                                                                                                 *
			//*   Iterator Traits                                                                                               *
			//*                                                                                                                 *
			//*******************************************************************************************************************

			typedef std::bidirectional_iterator_tag		iterator_category;
			typedef	T									value_type;
			typedef	std::ptrdiff_t						difference_type;
			typedef	T&									reference;
			typedef	T*									pointer;
			typedef CompoundIterator					self;

			//*******************************************************************************************************************
			//*                                                                                                                 *
			//*   Constructors                                                                                                  *
			//*                                                                                                                 *
			//*******************************************************************************************************************


			//  Normal Constructor
			//
			//	Constructs and initialises the iterator according to the selected style
			//
			//  PARAMETERS
			//
			//		RasterBuffer<T>&			-		Reference to the base container (RasterBuffer) of the iterator
			//		BoundingBox&				-		Const reference to the region to iterate
			//		SWITCHES					-		The iterator style bits
			//
			//  RETURNS
			//
			//  NOTES
			//

			CompoundIterator(RasterBuffer<T>& Container, const BoundingBox& Region, const SWITCHES Style)
			: CIBase2D(*this, Container.getWidth(), Region, Style), CA(Container) {

				//  Return to caller
				return;
			}

			//  Sub-Iterator Constructor
			//
			//	Constructs and initialises the iterator, the container and bounding box are inherited from the parent
			//
			//  PARAMETERS
			//
			//		CompoundIterator&			-		Const reference to a parent iterator
			//		SWITCHES					-		The iterator style bits
			//
			//  RETURNS
			//
			//  NOTES
			//

			CompoundIterator(const self& Parent, const SWITCHES Style)
			: CIBase2D(Parent, Parent.CA.getWidth(), Parent.Bounds, Style), CA(Parent.CA) {

				//  Return to caller
				return;
			}

			//  Iterator/sub-iterator copy constructor
			//
			//	Constructs a copy of the current iterator
			//
			//  PARAMETERS
			//
			//		CompoundIterator&			-		Const reference to the source iterator
			//
			//  RETURNS
			//
			//  NOTES
			//

			CompoundIterator(const self& Src) : CIBase2D(Src), CA(Src.CA) {
				
				//  Return to caller
				return;
			}

			//  Iterator/sub-iterator move constructor
			//
			//	Constructs a copy of the current iterator
			//
			//  PARAMETERS
			//
			//		CompoundIterator&&			-		Reference to the source iterator
			//
			//  RETURNS
			//
			//  NOTES
			//

			CompoundIterator(self&& Src) noexcept : CIBase2D(Src), CA(Src.CA) {

				//  Return to caller
				return;
			}

			//*******************************************************************************************************************
			//*                                                                                                                 *
			//*   Destructor                                                                                                    *
			//*                                                                                                                 *
			//*******************************************************************************************************************

			~CompoundIterator() {
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

			//  Dereference * operator
			//
			//	Returns a non-const reference to the encoded pixel at the current index position in the iterator.
			//
			//  PARAMETERS
			//
			//  RETURNS
			//
			//		T&					-	Reference to the encoded pixel at the current index position
			//
			//  NOTES
			//
			//		1.	NULL reference protection is provided by the container which always returns a valid reference.
			//

			reference	operator * () {
				return CA[effectiveOffset()];
			}

			//  Pointer -> operator
			//
			//	Returns a pointer to the entry in the container at the current index position
			//
			//  PARAMETERS
			//
			//  RETURNS
			//
			//		T*					-	Pointer to the encoded pixel at the current index position
			//
			//  NOTES
			//

			pointer	operator -> () {
				return CA.getPixel(effectiveOffset());
			}

			//  Prefix Increment ++X operator
			//
			//  Increments the index position of the operator and returns the updated iterator.
			//
			//  PARAMETERS
			//
			//  RETURNS
			//
			//		CompundIterator<T>&					-	Reference to the updated iterator
			//
			//  NOTES
			//

			self&	operator ++ () {
				incrementPosition();
				return *this;
			}

			//  Postfix Increment X++ operator
			//
			//  Increments the index position of the operator and returns a pre-modified clone of the iterator.
			//
			//  PARAMETERS
			//
			//  RETURNS
			//
			//		CompundIterator<T>					-	The premodified iterator
			//
			//  NOTES
			//
			//		1.	We return a copy of the pre-modified iterator for safety
			//

			self	operator ++ (int) {
				self	PreMod(*this);																	//  Pre-modified copy of the iterator

				incrementPosition();
				return PreMod;
			}

			//  Prefix decrement --X operator
			//
			//  decrements the index position of the operator and returns the updated iterator.
			//
			//  PARAMETERS
			//
			//  RETURNS
			//
			//		CompundIterator<T>&					-	Reference to the updated iterator
			//
			//  NOTES
			//

			self&	operator -- () {
				decrementPosition();
				return *this;
			}

			//  Postfix Decrement X-- operator
			//
			//  Decrements the index position of the operator and returns a pre-modified clone of the iterator.
			//
			//  PARAMETERS
			//
			//  RETURNS
			//
			//		CompundIterator<T>					-	The premodified iterator
			//
			//  NOTES
			//
			//		1.	We return a copy of the pre-modified iterator for safety
			//

			self	operator -- (int) {
				self	PreMod(*this);																	//  Pre-modified copy of the iterator

				decrementPosition();
				return PreMod;
			}

		private:

			//*******************************************************************************************************************
			//*																													*
			//*  Private Members																								*
			//*																													*
			//*******************************************************************************************************************

			RasterBuffer<T>&				CA;																//  Container array being iterated
		};

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   ConstCompoundIterator Class																					*
		//*                                                                                                                 *
		//*   The ConstCompoundIterator  provides a collection of different iterators that iterate over the content of a    *
		//*   const referenced RasterBuffer.                                                                                *
		//*                                                                                                                 *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		class ConstCompoundIterator : public CIBase2D {
		public:

			//*******************************************************************************************************************
			//*                                                                                                                 *
			//*   Iterator Traits                                                                                               *
			//*                                                                                                                 *
			//*******************************************************************************************************************

			typedef std::bidirectional_iterator_tag		iterator_category;
			typedef	T									value_type;
			typedef	std::ptrdiff_t						difference_type;
			typedef	const T&							reference;
			typedef	const T*							pointer;
			typedef ConstCompoundIterator				self;

			//*******************************************************************************************************************
			//*                                                                                                                 *
			//*   Constructors                                                                                                  *
			//*                                                                                                                 *
			//*******************************************************************************************************************


			//  Normal Constructor
			//
			//	Constructs and initialises the iterator according to the selected style
			//
			//  PARAMETERS
			//
			//		RasterBuffer<T>&			-		Const reference to the base container (RasterBuffer) of the iterator
			//		BoundingBox&				-		Const reference to the region to iterate
			//		SWITCHES					-		The iterator style bits
			//
			//  RETURNS
			//
			//  NOTES
			//

			ConstCompoundIterator(const RasterBuffer<T>& Container, const BoundingBox& Region, const SWITCHES Style)
				: CIBase2D(*this, Container.getWidth(), Region, Style), CA(Container) {

				//  Return to caller
				return;
			}

			//  Sub-Iterator Constructor
			//
			//	Constructs and initialises the iterator, the container and bounding box are inherited from the parent
			//
			//  PARAMETERS
			//
			//		CompoundIterator&			-		Const reference to a parent iterator
			//		SWITCHES					-		The iterator style bits
			//
			//  RETURNS
			//
			//  NOTES
			//

			ConstCompoundIterator(const self& Parent, const SWITCHES Style)
				: CIBase2D(Parent, Parent.CA.getWidth(), Parent.Bounds, Style), CA(Parent.CA) {

				//  Return to caller
				return;
			}

			//  Iterator/sub-iterator copy constructor
			//
			//	Constructs a copy of the current iterator
			//
			//  PARAMETERS
			//
			//		CompoundIterator&			-		Const reference to the source iterator
			//
			//  RETURNS
			//
			//  NOTES
			//

			ConstCompoundIterator(const self& Src) : CIBase2D(Src), CA(Src.CA) {

				//  Return to caller
				return;
			}

			//  Iterator/sub-iterator move constructor
			//
			//	Constructs a copy of the current iterator
			//
			//  PARAMETERS
			//
			//		CompoundIterator&&			-		Reference to the source iterator
			//
			//  RETURNS
			//
			//  NOTES
			//

			ConstCompoundIterator(self&& Src) noexcept : CIBase2D(Src), CA(Src.CA) {

				//  Return to caller
				return;
			}

			//*******************************************************************************************************************
			//*                                                                                                                 *
			//*   Destructor                                                                                                    *
			//*                                                                                                                 *
			//*******************************************************************************************************************

			~ConstCompoundIterator() {
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

			//  Dereference * operator
			//
			//	Returns a non-const reference to the encoded pixel at the current index position in the iterator.
			//
			//  PARAMETERS
			//
			//  RETURNS
			//
			//		T&					-	Reference to the encoded pixel at the current index position
			//
			//  NOTES
			//
			//		1.	NULL reference protection is provided by the container which always returns a valid reference.
			//

			reference	operator * () {
				return CA[effectiveOffset()];
			}

			//  Pointer -> operator
			//
			//	Returns a pointer to the entry in the container at the current index position
			//
			//  PARAMETERS
			//
			//  RETURNS
			//
			//		T*					-	Pointer to the encoded pixel at the current index position
			//
			//  NOTES
			//

			pointer	operator -> () {
				return CA.getPixel(effectiveOffset());
			}

			//  Prefix Increment ++X operator
			//
			//  Increments the index position of the operator and returns the updated iterator.
			//
			//  PARAMETERS
			//
			//  RETURNS
			//
			//		CompundIterator<T>&					-	Reference to the updated iterator
			//
			//  NOTES
			//

			self&	operator ++ () {
				incrementPosition();
				return *this;
			}

			//  Postfix Increment X++ operator
			//
			//  Increments the index position of the operator and returns a pre-modified clone of the iterator.
			//
			//  PARAMETERS
			//
			//  RETURNS
			//
			//		CompundIterator<T>					-	The premodified iterator
			//
			//  NOTES
			//
			//		1.	We return a copy of the pre-modified iterator for safety
			//

			self	operator ++ (int) {
				self	PreMod(*this);																	//  Pre-modified copy of the iterator

				incrementPosition();
				return PreMod;
			}

			//  Prefix decrement --X operator
			//
			//  decrements the index position of the operator and returns the updated iterator.
			//
			//  PARAMETERS
			//
			//  RETURNS
			//
			//		CompundIterator<T>&					-	Reference to the updated iterator
			//
			//  NOTES
			//

			self&	operator -- () {
				decrementPosition();
				return *this;
			}

			//  Postfix Decrement X-- operator
			//
			//  Decrements the index position of the operator and returns a pre-modified clone of the iterator.
			//
			//  PARAMETERS
			//
			//  RETURNS
			//
			//		CompundIterator<T>					-	The premodified iterator
			//
			//  NOTES
			//
			//		1.	We return a copy of the pre-modified iterator for safety
			//

			self	operator -- (int) {
				self	PreMod(*this);																	//  Pre-modified copy of the iterator

				decrementPosition();
				return PreMod;
			}

		private:

			//*******************************************************************************************************************
			//*																													*
			//*  Private Members																								*
			//*																													*
			//*******************************************************************************************************************

			const RasterBuffer<T>&		CA;																//  Container array being iterated
		};

	};
}
