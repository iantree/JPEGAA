#pragma	once
//
//*******************************************************************************************************************
//*																													*
//*   File:		  DIB.h																								*
//*   Suite:      xymorg Image Processing - ODI																		*
//*   Version:    1.0.0	  Build:  01																				*
//*   Author:     Ian Tree/HMNL																						*
//*																													*
//*   Copyright 2017 - 2020 Ian J. Tree																				*
//*******************************************************************************************************************
//*	DIB.h																											*
//*																													*
//*	This header file contains the static functions for handling a DIB On-Disk Image.								*
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
#include	"../../../LPBHdrs.h"																	//  Language and Platform base headers
#include	"../../../types.h"																		//  xymorg type definitions
#include	"../../../VRMapper.h"																	//  Resource Mapper

//  Additional xymorg components
#include	"../../../MemoryDumper.h"																//  Memory Dump

//  Include xymorg image processing primitives
#include	"../../types.h"																			//  Image processing primitive types
#include	"../../consts.h"																		//  Image processing constants
#include	"DIBODI.h"																				//  GIF ODI Definitions
#include	"../../Train.h"																			//  Image Train
#include	"../../ColourTable.h"																	//  Colour Table (array)

namespace xymorg {

	//
	//  DIB class - contains ONLY static functions
	//

	class DIB {
	public:

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Public Constants                                                                                              *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		static const SWITCHES		DIB_STORE_OPT_FORCE_RGB24 = 1;								//  Enforce RGB 24 bit encodeing
		static const SWITCHES		DIB_STORE_OPT_ENABLE_RLEC = 2;								//  Enable RLE compression
		static const SWITCHES		DIB_STORE_OPT_FORCE_RLEC = 4;								//  Force RLE compression

		//  Machine States for RLE decoding
		static const int			RLE_DOING_RUN = 1;											//  Default state emitting run
		static const int			RLE_DOING_ABSOLUTE = 2;										//  Doing absolute pixels
		static const int			RLE_DOING_POSITION = 3;										//  Moving relative to current point

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Public Types		                                                                                            *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//   Structure for Bit Map Colour Tables
		typedef struct RGBQuad {
			uint8_t			R;
			uint8_t			G;
			uint8_t			B;
			uint8_t			Pad;																//  Reserved
		} RGBQuad;

		//  Prevent Instantiation
		DIB() = delete;

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Public Functions                                                                                              *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  loadImage
		//
		//  This static function will load the designated image into memory and return the content as an RGB train.
		//
		//  PARAMETERS
		//
		//		char*			-		Pointer to the name of the image to be loaded
		//		VRMapper&		-		Reference to the resource mapper to use
		//
		//  RETURNS
		//
		//		Train*			-		Pointer to the image Train, NULL if it could not be loaded
		//
		//  NOTES
		//

		static Train<RGB>* loadImage(const char* ImgName, VRMapper& VRMap) {

			return loadImageRGB(ImgName, VRMap);
		}

		//  loadImageRGB
		//
		//  This static function will load the designated image into memory and return the content as an RGB train.
		//
		//  PARAMETERS
		//
		//		char*			-		Pointer to the name of the image to be loaded
		//		VRMapper&		-		Reference to the resource mapper to use
		//
		//  RETURNS
		//
		//		Train*			-		Pointer to the image Train, NULL if it could not be loaded
		//
		//  NOTES
		//

		static Train<RGB>* loadImageRGB(const char* ImgName, VRMapper& VRMap) {
			BYTE*			pImage = nullptr;													//  Pointer to the in-memory image
			size_t			ImgSize = 0;														//  Image Size
			Train<RGB>*		pTrain = nullptr;													//  Constructed Train	

			//  Safety
			if (ImgName == nullptr) return nullptr;
			if (ImgName[0] == '\0') return nullptr;

			//  Load the on-disk image into memory
			pImage = VRMap.loadResource(ImgName, ImgSize);
			if (pImage == nullptr) return nullptr;

			//  Unbutton the image to a train
			pTrain = unbuttonImage(pImage, ImgSize);

			//  Free the image
			free(pImage);

			//  Return the train
			return pTrain;
		}

		//  storeImage
		//
		//  This static function will store the passed RGB Train into the designated DIB (BMP) Image file
		//
		//  PARAMETERS
		//
		//		char*			-		Pointer to the name of the image to be stored
		//		VRMapper&		-		Reference to the resource mapper to use
		//		Train*			-		Pointer to the Train to store
		//
		//  RETURNS
		//
		//		bool			-		true if the image was successfully stored, otherwise false
		//
		//  NOTES
		//

		static bool		storeImage(const char* ImgName, VRMapper& VRMap, Train<RGB>* pTrain) {

			return storeRGBImage(ImgName, VRMap, pTrain, 0);
		}

		//  storeImage
		//
		//  This static function will store the passed RGB Train into the designated DIB (BMP) Image file
		//
		//  PARAMETERS
		//
		//		char*			-		Pointer to the name of the image to be stored
		//		VRMapper&		-		Reference to the resource mapper to use
		//		Train*			-		Pointer to the Train to store
		//		SWITCHES		-		Options to use for storing images
		//
		//  RETURNS
		//
		//		bool			-		true if the image was successfully stored, otherwise false
		//
		//  NOTES
		//

		static bool		storeImage(const char* ImgName, VRMapper& VRMap, Train<RGB>* pTrain, SWITCHES Opts) {

			return storeRGBImage(ImgName, VRMap, pTrain, Opts);
		}

		//  storeRGBImage
		//
		//  This static function will store the passed RGB Train into the designated DIB (BMP) Image file
		//
		//  PARAMETERS
		//
		//		char*			-		Pointer to the name of the image to be stored
		//		VRMapper&		-		Reference to the resource mapper to use
		//		Train*			-		Pointer to the Train to store
		//		SWITCHES		-		Options to use for storing images
		//
		//  RETURNS
		//
		//		bool			-		true if the image was successfully stored, otherwise false
		//
		//  NOTES
		//

		static bool		storeRGBImage(const char* ImgName, VRMapper& VRMap, Train<RGB>* pTrain, SWITCHES Opts) {
			BYTE*		pImage = nullptr;												//  Pointer to the in-memory image
			size_t		ImgSize = 0;													//  Size of the in-memory image

			//  Safety
			if (ImgName == nullptr) return false;
			if (ImgName[0] == '\0') return false;
			if (pTrain == nullptr) return false;
			if (pTrain->getCanvasHeight() == 0 || pTrain->getCanvasWidth() == 0) return false;

			//  Set the default options when needed

			//  Build the in-memory image of the DIB On-Disk image
			pImage = buttonImage(ImgSize, pTrain, Opts);
			if (pImage == nullptr || ImgSize == 0) {
				std::cerr << "ERROR: Unable to build an DIB/BMP on-disk image for: " << ImgName << "." << std::endl;
				if (pImage != nullptr) free(pImage);
				return false;
			}

			//  Store the in-memory image  (consumes the image memory allocation)
			if (!VRMap.storeResource(ImgName, pImage, ImgSize)) {
				std::cerr << "ERROR: Failed to store DIB/BMP image: '" << ImgName << "', (" << ImgSize << " bytes)." << std::endl;
				return false;
			}

			//  Reurn shwing success
			return true;
		}

		//  analyseImage
		//
		//  This static function will load the designated image into memory and provide an annotated dump of the contents
		//
		//  PARAMETERS
		//
		//		char*			-		Pointer to the name of the image to be analysed
		//		VRMapper&		-		Reference to the resource mapper to use
		//		std::ostream&	-		Reference to the output stream for the analysis
		//
		//  RETURNS
		//
		//  NOTES
		//

		static void		analyseImage(const char* ImgName, VRMapper& VRMap, std::ostream& OS) {
			return analyseImage(false, ImgName, VRMap, OS);
		}

		//  analyseImageEx
		//
		//  This static function will load the designated image into memory and provide an annotated dump of the contents
		//	additionally document the image contents.
		//
		//  PARAMETERS
		//
		//		char*			-		Pointer to the name of the image to be analysed
		//		VRMapper&		-		Reference to the resource mapper to use
		//		std::ostream&	-		Reference to the output stream for the analysis
		//
		//  RETURNS
		//
		//  NOTES
		//

		static void		analyseImageEx(const char* ImgName, VRMapper& VRMap, std::ostream& OS) {
			return analyseImage(true, ImgName, VRMap, OS);
		}

		//  analyseImage
		// 
		//
		//  This static function will load the designated image into memory and provide an annotated dump of the contents
		//  and optionally document the image contents.
		//
		//  PARAMETERS
		//
		//		bool			-		If true show the image contents, otherwise show only the On-Disk Image
		//		char*			-		Pointer to the name of the image to be analysed
		//		VRMapper&		-		Reference to the resource mapper to use
		//		std::ostream&	-		Reference to the output stream for the analysis
		//
		//  RETURNS
		//
		//  NOTES
		//

		static void		analyseImage(bool ShowImage, const char* ImgName, VRMapper& VRMap, std::ostream& OS) {
			BYTE*			pImage = nullptr;													//  Pointer to the in-memory image
			size_t			ImgSize = 0;														//  Image Size
			size_t			Offset = 0;															//  Offset in the file
			size_t			BlockNo = 1;														//  Block number
			Train<RGB>*		pITrain = nullptr;													//  Pointer to the train holding the image
			char			szFileName[MAX_PATH + 1] = {};										//  File name

			//  Show starting analysis
			OS << "DIB(BMP): ON-DISK-IMAGE ANALYSIS" << std::endl;
			OS << "================================" << std::endl;
			OS << std::endl;

			//  Safety
			if (ImgName == nullptr) {
				OS << "   ERROR: Image File Name is missing." << std::endl;
				return;
			}

			if (ImgName[0] == '\0') {
				OS << "   ERROR: Image File Name is missing." << std::endl;
				return;
			}

			VRMap.mapFile(ImgName, szFileName, MAX_PATH);
			OS << "Image Name: " << ImgName << " (" << szFileName << ")." << std::endl;

			//  Load the on-disk image into memory
			pImage = VRMap.loadResource(ImgName, ImgSize);

			if (pImage == nullptr) {
				OS << "   ERROR: The Image could NOT be loaded, invalid name or file not found." << std::endl;
				return;
			}

			//  Show the image size
			OS << "Image Size: " << ImgSize << " bytes." << std::endl;

			//  Show Details from the file header
			showFileHeader(pImage, Offset, BlockNo, OS);
			BlockNo++;

			//  Show details from the image header
			showImageHeader(pImage, Offset, BlockNo, OS);
			BlockNo++;

			//  Show details of the Colour Table (If Present)
			if (showColourTable(pImage, Offset, BlockNo, OS)) BlockNo++;

			//  Show the image pixel array
			showImage(pImage, Offset, BlockNo, ImgSize, OS);

			//  Free the image
			free(pImage);

			//
			//  If requested then load and document the image
			//
			if (ShowImage) {

				//  Load the image into a Train
				pITrain = loadImage(ImgName, VRMap);
				if (pITrain == nullptr) {
					OS << "ERROR: Failed to load the image into memory for documentation." << std::endl;
					return;
				}

				//  Document the image
				pITrain->document(OS, true);

				//  Destroy the train
				delete pITrain;
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

		//  buttonImage
		//
		//  This static function button up the passed train into an on-disk DIB image format
		//
		//  PARAMETERS
		//
		//		size_t&			-		Size of the in-memory image (bytes)
		//		Train*			-		Pointer to the image Train to be buttoned
		//		SWIRCHES		-		Options for storing the image
		//
		//  RETURNS
		//
		//		BYTE*			-		Pointer to the in-memory image
		//
		//  NOTES
		//

		static BYTE* buttonImage(size_t& ImgSize, Train<RGB>* pTrain, SWITCHES Opts) {
			BYTE*			pImage = nullptr;											//  In-Memory image
			size_t			NewImgSize = 0;												//  Size of the in-memory image
			Train<RGB>*		pCTrain = nullptr;											//  Canonical Train (Writeable)

			//  Auto adjust the Train Canvas Size
			pTrain->autocorrect();

			//  Construct the Canonical Train from the input train. The Canonical Train is directly writeble as a DIB image.
			pCTrain = buildCanonicalTrain(pTrain, Opts);
			if (pCTrain == nullptr) return nullptr;

			//  Serialise the Canonical Train into a memory image
			pImage = serialiseTrain(pCTrain, NewImgSize, Opts);

			//  Delete the Canonical Train
			delete pCTrain;

			//  Return the constructed image
			ImgSize = NewImgSize;
			return pImage;
		}

		//  unbuttonImage
		//
		//  This static function will map the in-memory image and unbutton the content to an RGB train.
		//
		//  PARAMETERS
		//
		//		BYTE*			-		Pointer to the in-memory image to be unbuttoned to a Train
		//		size_t			-		Size of the in-memory image (bytes)
		//
		//  RETURNS
		//
		//		Train*			-		Pointer to the image Train, NULL if it could not be unbuttoned
		//
		//  NOTES
		//

		static Train<RGB>* unbuttonImage(BYTE* pImage, size_t Size) {
			Train<RGB>* pTrain = nullptr;													//  Constructed Train	
			size_t			Offset = 0;															//  Offset into the image
			size_t			FileSize = 0;														//  File size from File Header
			size_t			PAOffset = 0;														//  Offset to the Pixel Array
			size_t			CanvasH = 0;														//  Canvas height
			size_t			CanvasW = 0;														//  Canvas width
			DIB_HEADER* pDIH = nullptr;														//  Pointer to the image header
			size_t			CTEnts = 0;															//  Number of colour table entries
			size_t			CTSize = 0;															//  Size of the colour table
			RGBQuad* pCT = nullptr;														//  Pointer to the colour table

			//  Obtain the filesize from the file header
			FileSize = getFileSize(pImage, Offset, PAOffset);
			if (FileSize == 0) {
				std::cerr << "ERROR: Bitmap image file has an invalid file header block." << std::endl;
				return NULL;
			}

			//  Obtain the image Height and Width and a pointer to the Bitmap Image Header
			pDIH = getImageHeader(pImage, Offset, CanvasH, CanvasW);
			if (pDIH == nullptr) {
				std::cerr << "ERROR: Bitmap image file contains an invalid image header." << std::endl;
				return nullptr;
			}

			//  Check the consistency of the information before we try to process the image
			if (FileSize != Size) {
				std::cerr << "ERROR: Bitmap image file has an inconsistent file size." << std::endl;
				return nullptr;
			}

			//  Determine the number of entries and size of the colour table
			CTEnts = pDIH->Colours;

			//  Make sure that this is consistent with the Bits Per Pixel setting
			switch (pDIH->BPP) {
			case 1:
				//  Monochrome Bitmap
				if (CTEnts != 2) {
					std::cerr << "ERROR: Bitmap image has inconsistent colour table information." << std::endl;
					return nullptr;
				}
				break;

			case 4:
				//  16 (Max) colour bitmap
				if (CTEnts > 16) {
					std::cerr << "ERROR: Bitmap image has inconsistent colour table information." << std::endl;
					return nullptr;
				}
				if (CTEnts == 0) CTEnts = 16;
				break;

			case 8:
				//  256 (Max) colour bitmap
				if (CTEnts > 256) {
					std::cerr << "ERROR: Bitmap image has inconsistent colour table information." << std::endl;
					return nullptr;
				}
				if (CTEnts == 0) CTEnts = 256;
				break;

			case 24:
				//  24 bit colour - has no colour table
				if (CTEnts > 0) {
					std::cerr << "ERROR: Bitmap image has inconsistent colour table information." << std::endl;
					return nullptr;
				}
				break;

			default:
				std::cerr << "ERROR: Bitmap image has an invalid Bits Per Pixel colour setting." << std::endl;
				return nullptr;
			}

			CTSize = CTEnts * sizeof(RGBQuad);

			//  Check the offset to the Pixel Array
			if ((Offset + CTSize) != PAOffset) {
				std::cerr << "ERROR: Bitmap image has an invalid/inconsistent file layout." << std::endl;
				return nullptr;
			}

			//  Check the Pixel Array size
			if (pDIH->BitSettings == DIB_ENC_RGB) {
				if ((Offset + CTSize + pDIH->BMSize) != FileSize) {
					std::cerr << "ERROR: Bitmap image has an invalid/inconsistent file layout." << std::endl;
					return nullptr;
				}
			}

			//  Image appears to be consistent - proceed with the unbuttoning
			pTrain = new Train<RGB>(CanvasH, CanvasW, nullptr);

			//  Capture the pointer to the colour table
			if (CTSize > 0) {
				pCT = (RGBQuad*)(pImage + Offset);
				Offset += CTSize;
			}

			//  The offset now points to the pixel array - unbutton it and append as a new frame
			Frame<RGB>* pFrame = unbuttonFrame(pImage + Offset, pDIH, pCT, CTEnts);
			if (pFrame == nullptr) {
				std::cerr << "ERROR: Bitmap image decoder was unable to extract the image." << std::endl;
				delete pTrain;
				return nullptr;
			}
			pTrain->append(pFrame);

			//  Return the unbuttoned frame
			return pTrain;
		}

		//  unbuttonImage
		//
		//  This static function will map the in-memory image and unbutton the content to an RGB train.
		//
		//  PARAMETERS
		//
		//		BYTE*			-		Pointer to the Pixel Array holding the image
		//		DIB_HEADER*		-		Pointer to the image header
		//		RGBQuad*		-		Pointer to the colour table
		//		size_t			-		Number of entries in the colour table
		//
		//  RETURNS
		//
		//		Frame*			-		Pointer to the image Frame, NULL if it could not be unbuttoned
		//
		//  NOTES
		//

		static Frame<RGB>* unbuttonFrame(BYTE* pPixelArray, DIB_HEADER* pDIH, RGBQuad* pCT, size_t CTEnts) {
			RasterBuffer<RGB>*			pRB = new RasterBuffer<RGB>(pDIH->Height, pDIH->Width, nullptr);
			size_t						ScanLineSize = 0;															//  Size of a scan line
			size_t						LineSize = 0;																//  Pixel counter (per scan line)
			BYTE*						pPixels = pPixelArray;														//  Current pointer into the pixel array
			BYTE						EncPixels = 0;																//  Encoded set of pixels
			size_t						BitsUsed = 0;																//  Number of bits used (so far) in encoded set of pixels

			//  Unbutton the Pixel Array according to the encoding
			switch (pDIH->BPP) {
			case 1:
				//  Monochrome 8 pixels per byte encoding
				ScanLineSize =pDIH->Width / 8;
				if ((pDIH->Width % 8) > 0) ScanLineSize++;
				if ((ScanLineSize % 4) > 0) ScanLineSize += (4 - (ScanLineSize % 4));

				//  Iterate over the image (from bottom to top) filling in the pixels
				for (RasterBuffer<RGB>::iterator RIt = pRB->bottom(); RIt != pRB->top(); RIt--) {
					LineSize = 0;
					BitsUsed = 0;
					for (RasterBuffer<RGB>::iterator CIt = pRB->left(RIt); CIt != pRB->right(RIt); CIt++) {

						EncPixels = (*pPixels >> (7 - BitsUsed)) & 0x01;

						(*CIt).R = pCT[EncPixels].B;
						(*CIt).G = pCT[EncPixels].G;
						(*CIt).B = pCT[EncPixels].R;

						BitsUsed++;
						if (BitsUsed == 8) {
							pPixels++;
							LineSize++;
							BitsUsed = 0;
						}
					}

					//  Skip over any padding
					if (BitsUsed > 0) {
						LineSize++;
						pPixels++;
					}
					while (LineSize < ScanLineSize) {
						LineSize++;
						pPixels++;
					}
				}
				break;

			case 4:
				//  16 colour - 1 pixel per nibble encoding
				ScanLineSize = pDIH->Width / 2;
				ScanLineSize += (pDIH->Width % 2);
				if ((ScanLineSize % 4) > 0) ScanLineSize += (4 - (ScanLineSize % 4));

				//  If RLE compression is in effect then decode the buffer
				if (pDIH->BitSettings == DIB_ENC_RLE8) decodeImageFromRLE4(pPixels, pDIH->BMSize, *pRB, pCT, CTEnts);
				else {
					//  Iterate over the image (from bottom to top) filling in the pixels
					for (RasterBuffer<RGB>::iterator RIt = pRB->bottom(); RIt != pRB->top(); RIt--) {
						LineSize = 0;
						BitsUsed = 0;
						for (RasterBuffer<RGB>::iterator CIt = pRB->left(RIt); CIt != pRB->right(RIt); CIt++) {
							if (BitsUsed == 0) EncPixels = (*pPixels & 0xF0) >> 4;
							else EncPixels = (*pPixels & 0x0F);

							if (EncPixels < CTEnts) {
								//  Valid colour
								(*CIt).R = pCT[EncPixels].B;
								(*CIt).G = pCT[EncPixels].G;
								(*CIt).B = pCT[EncPixels].R;
							}
							BitsUsed += 4;
							if (BitsUsed == 8) {
								pPixels++;
								LineSize++;
								BitsUsed = 0;
							}
						}

						//  Skip over any padding
						if (BitsUsed == 4) {
							LineSize++;
							pPixels++;
						}
						while (LineSize < ScanLineSize) {
							LineSize++;
							pPixels++;
						}
					}
				}
				break;

			case 8:
				//  256 colour - one pixel per byte encoding
				ScanLineSize = pDIH->Width;
				if ((ScanLineSize % 4) > 0) ScanLineSize += (4 - (ScanLineSize % 4));

				//  If RLE compression is in effect then decode the buffer
				if (pDIH->BitSettings == DIB_ENC_RLE8) decodeImageFromRLE8(pPixels, pDIH->BMSize, *pRB, pCT, CTEnts);
				else {
					//  Iterate over the image (from bottom to top) filling in the pixels
					for (RasterBuffer<RGB>::iterator RIt = pRB->bottom(); RIt != pRB->top(); RIt--) {
						LineSize = 0;
						for (RasterBuffer<RGB>::iterator CIt = pRB->left(RIt); CIt != pRB->right(RIt); CIt++) {
							if (*pPixels < CTEnts) {
								//  Valid colour
								(*CIt).R = pCT[*pPixels].B;
								(*CIt).G = pCT[*pPixels].G;
								(*CIt).B = pCT[*pPixels].R;
							}

							pPixels++;
							LineSize++;
						}

						//  Skip over any padding
						while (LineSize < ScanLineSize) {
							LineSize++;
							pPixels++;
						}
					}
				}
				break;

			case 24:
				//  24 bit colour - unencoded image
				ScanLineSize = pDIH->Width * sizeof(RGB);
				if ((ScanLineSize % 4) > 0) ScanLineSize += (4 - (ScanLineSize % 4));

				//  Iterate over the image (from bottom to top) filling in the pixels
				for (RasterBuffer<RGB>::iterator RIt = pRB->bottom(); RIt != pRB->top(); RIt--) {
					LineSize = 0;
					for (RasterBuffer<RGB>::iterator CIt = pRB->left(RIt); CIt != pRB->right(RIt); CIt++) {
						RGB* pPixel = (RGB*) pPixels;
						(*CIt).R = pPixel->B;
						(*CIt).G = pPixel->G;
						(*CIt).B = pPixel->R;

						pPixels += sizeof(RGB);
						LineSize += sizeof(RGB);
					}

					//  Skip over any padding
					while (LineSize < ScanLineSize) {
						LineSize++;
						pPixels++;
					}
				}
				break;
			}

			//  Return the constructed frame
			return new Frame<RGB>(pRB, 0, 0, nullptr);
		}

		//  decodeImageFromRLE8
		//
		//  This static function will populate the image raster buffer from an RLE8 compressed source.
		//
		//  PARAMETERS
		//
		//		BYTE*			-		Pointer to the Pixel Array (RLE8 Encoded source)
		//		size_t			-		Size of the Pixel Array
		//		RasterBuffer&	-		Reference to the Raster Buffer to be populated
		//
		//  RETURNS
		//
		//  NOTES
		//

		static void		decodeImageFromRLE8(BYTE* pPA, size_t PASize, RasterBuffer<RGB>& RB, RGBQuad* pCT, size_t CTEnts) {
			(void)PASize;
			int				DecodingState = RLE_DOING_RUN;															//  Decoding Machine State
			BYTE*			pCurrentToken = pPA;																	//  Pointer to the current token
			BYTE*			pNextToken = pPA;																		//  Pointer to the next token
			DIB_RLE_BLOCK*	pCurrentOp = (DIB_RLE_BLOCK*) pCurrentToken;											//  Pointer to the current operation

			//  Controls for processing a Run 
			int			RunCount = 0;																				//  Count of pixels remaining in the run
			size_t		Symbol = 0;																					//  Symbol being repeated

			//  Controls for processing an absolute string of symbols
			BYTE*		pString = nullptr;																			//  Pointer to the string being inserted
			int			AbsSymbolCount = 0;																			//  Count of symbols in absolute run
			int			PaddingBytes = 0;																			//  Count of padding bytes

			//  Controls for processing a relative position (move)
			int			SkipRows = 0;																				//  Number of rows to skip
			size_t		SkipToColumn = 0;																			//  Target column

			//
			//  Iterate over the RasterBuffer Bottom to Top, Left to Right filling in the pixels from the encoding
			//

			for (RasterBuffer<RGB>::iterator RIt = RB.bottom(); RIt != RB.top(); RIt--) {

				//  If a move (realtive position) is being processed then skip over any rows
				if (DecodingState == RLE_DOING_POSITION && SkipRows > 0) SkipRows--;
				else {
					//  Process the next row of the image
					for (RasterBuffer<RGB>::iterator CIt = RB.left(RIt); CIt != RB.right(RIt); CIt++) {

						//  If a move (realtive position) is being processed and we have acheved the target position then switch back to encoded mode
						if (DecodingState == RLE_DOING_POSITION && SkipToColumn == CIt.getIndex()) DecodingState = RLE_DOING_RUN;

						//
						//  Process the current pixel according to the state of the decoding engine
						//

						switch (DecodingState) {
						//
						//  Inserting a run of the current symbol
						//
						case RLE_DOING_RUN:

							//  If the current run is exhausetd then switch to the next run, which may be a different operation
							if (RunCount == 0) {
								pCurrentToken = pNextToken;
								pCurrentOp = (DIB_RLE_BLOCK*) pCurrentToken;

								//  Setup the controls for the current operation and determine the position of the next token
								if (pCurrentOp->RunLen > 0) {
									//  Operation is a repeat symbol oprtation
									RunCount = pCurrentOp->RunLen;
									Symbol = pCurrentOp->Symbol;
									pNextToken = pCurrentToken + 2;
								}
								else {
									//  Escape - a diferent operation - determined by the Symbol value
									if (pCurrentOp->Symbol == 0) {
										//  End-Of-Line signal
										pNextToken = pCurrentToken + 2;
										//  Remaining pixels in the current row are not updated
										break;
									}
									else if (pCurrentOp->Symbol == 1) {
										//  End of the bitmap - return to caller
										return;
									}
									else if (pCurrentOp->Symbol == 2) {
										//  This is a positioning operation - capture the relative postion vector
										SkipRows = *(pCurrentToken + 3);
										SkipToColumn = CIt.getIndex() + *(pCurrentToken + 2);
										if (SkipToColumn >= RB.getWidth()) SkipToColumn = 0;
										DecodingState = RLE_DOING_POSITION;
										pNextToken = pCurrentToken + 4;
									}
									else {
										//  This is an 'absolute' mode - a non-repeating string of single pixels
										AbsSymbolCount = pCurrentOp->Symbol;
										pString = pCurrentToken + 2;
										//  Compute any padding
										PaddingBytes = AbsSymbolCount & 1;
										pNextToken = pCurrentToken + 2 + AbsSymbolCount + PaddingBytes;

										//  Setup the first character from the string to be set
										RunCount = 1;
										Symbol = *pString;
										pString++;
										AbsSymbolCount--;

										//  Determine the modal state for the next operation
										if (AbsSymbolCount > 0) DecodingState = RLE_DOING_ABSOLUTE;
									}
								}
							}

							//  If there is a symbol for the pixel then set the pixel
							if (RunCount > 0) {
								//  Set the pixel
								if (Symbol < CTEnts) {
									//  Valid colour
									(*CIt).R = pCT[Symbol].B;
									(*CIt).G = pCT[Symbol].G;
									(*CIt).B = pCT[Symbol].R;
								}
								RunCount--;
							}

							break;

						//
						//  Insreting an absolte string of symbols
						//
						case RLE_DOING_ABSOLUTE:
							Symbol = *pString;
							pString++;
							AbsSymbolCount--;
							if (Symbol < CTEnts) {
								//  Valid colour
								(*CIt).R = pCT[Symbol].B;
								(*CIt).G = pCT[Symbol].G;
								(*CIt).B = pCT[Symbol].R;
							}

							//  If this is the end of the absolute string then switch back to encoded mode
							if (AbsSymbolCount == 0) DecodingState = RLE_DOING_RUN;

							break;

							//
							//  Performing a relative position (move) operation
							//
						case RLE_DOING_POSITION:
							//  Do not update the current pixel
							break;
						}
					}

					//  If we are doing run encoding and we have an EOL here then skip if
					if (DecodingState == RLE_DOING_RUN && RunCount == 0) {
						DIB_RLE_BLOCK*	pNextOp = (DIB_RLE_BLOCK*) pNextToken;
						if (pNextOp->RunLen == 0 && pNextOp->Symbol == 0) {
							pCurrentToken = pNextToken;
							pNextToken = pCurrentToken + 2;
						}
					}
				}
			}

			//  Return to caller
			return;
		}

		//  decodeImageFromRLE4
		//
		//  This static function will populate the image raster buffer from an RLE8 compressed source.
		//
		//  PARAMETERS
		//
		//		BYTE*			-		Pointer to the Pixel Array (RLE8 Encoded source)
		//		size_t			-		Size of the Pixel Array
		//		RasterBuffer&	-		Reference to the Raster Buffer to be populated
		//
		//  RETURNS
		//
		//  NOTES
		//

		static void		decodeImageFromRLE4(BYTE* pPA, size_t PASize, RasterBuffer<RGB>& RB, RGBQuad* pCT, size_t CTEnts) {
			(void)PASize;
			int				DecodingState = RLE_DOING_RUN;															//  Decoding Machine State
			BYTE*			pCurrentToken = pPA;																	//  Pointer to the current token
			BYTE*			pNextToken = pPA;																		//  Pointer to the next token
			DIB_RLE_BLOCK*	pCurrentOp = (DIB_RLE_BLOCK*)pCurrentToken;												//  Pointer to the current operation

			//  Controls for processing a Run 
			int			RunCount = 0;																				//  Count of pixels remaining in the run
			size_t		Symbol = 0;																					//  Symbol being repeated
			bool		HiOrderNibble = true;																		//  Nibble to use for index
			size_t		Index = 0;																					//  Index of Colour

			//  Controls for processing an absolute string of symbols
			BYTE*		pString = nullptr;																			//  Pointer to the string being inserted
			int			AbsSymbolCount = 0;																			//  Count of symbols in absolute run
			int			StringLen = 0;																				//  String Length
			int			PaddingBytes = 0;																			//  Count of padding bytes

			//  Controls for processing a relative position (move)
			int			SkipRows = 0;																				//  Number of rows to skip
			size_t		SkipToColumn = 0;																			//  Target column

			//
			//  Iterate over the RasterBuffer Bottom to Top, Left to Right filling in the pixels from the encoding
			//

			for (RasterBuffer<RGB>::iterator RIt = RB.bottom(); RIt != RB.top(); RIt--) {

				//  If a move (realtive position) is being processed then skip over any rows
				if (DecodingState == RLE_DOING_POSITION && SkipRows > 0) SkipRows--;
				else {
					//  Process the next row of the image
					for (RasterBuffer<RGB>::iterator CIt = RB.left(RIt); CIt != RB.right(RIt); CIt++) {

						//  If a move (realtive position) is being processed and we have acheved the target position then switch back to encoded mode
						if (DecodingState == RLE_DOING_POSITION && SkipToColumn == CIt.getIndex()) DecodingState = RLE_DOING_RUN;

						//
						//  Process the current pixel according to the state of the decoding engine
						//

						switch (DecodingState) {
							//
							//  Inserting a run of the current symbol
							//
						case RLE_DOING_RUN:

							//  If the current run is exhausetd then switch to the next run, which may be a different operation
							if (RunCount == 0) {
								pCurrentToken = pNextToken;
								pCurrentOp = (DIB_RLE_BLOCK*) pCurrentToken;

								//  Setup the controls for the current operation and determine the position of the next token
								if (pCurrentOp->RunLen > 0) {
									//  Operation is a repeat symbol oprtation
									RunCount = pCurrentOp->RunLen;
									Symbol = pCurrentOp->Symbol;
									HiOrderNibble = true;
									pNextToken = pCurrentToken + 2;
								}
								else {
									//  Escape - a diferent operation - determined by the Symbol value
									if (pCurrentOp->Symbol == 0) {
										//  End-Of-Line signal
										pNextToken = pCurrentToken + 2;
										//  Remaining pixels in the current row are not updated
										break;
									}
									else if (pCurrentOp->Symbol == 1) {
										//  End of the bitmap - return to caller
										return;
									}
									else if (pCurrentOp->Symbol == 2) {
										//  This is a positioning operation - capture the relative postion vector
										SkipRows = *(pCurrentToken + 3);
										SkipToColumn = CIt.getIndex() + *(pCurrentToken + 2);
										if (SkipToColumn >= RB.getWidth()) SkipToColumn = 0;
										DecodingState = RLE_DOING_POSITION;
										pNextToken = pCurrentToken + 4;
									}
									else {
										//  This is an 'absolute' mode - a non-repeating string of single pixels
										AbsSymbolCount = pCurrentOp->Symbol;
										pString = pCurrentToken + 2;
										StringLen = (AbsSymbolCount / 2) + (AbsSymbolCount & 1);

										//  Compute any padding
										PaddingBytes = StringLen & 1;
										pNextToken = pCurrentToken + 2 + StringLen + PaddingBytes;
										HiOrderNibble = true;

										//  Setup the first character from the string to be set
										RunCount = 1;
										Symbol = *pString;
										AbsSymbolCount--;

										//  Determine the modal state for the next operation
										if (AbsSymbolCount > 0) DecodingState = RLE_DOING_ABSOLUTE;
									}
								}
							}

							//  If there is a symbol for the pixel then set the pixel
							if (RunCount > 0) {
								if (HiOrderNibble) Index = (Symbol & 0xF0) >> 4;
								else Index = Symbol & 0x0F;
								//  Set the pixel
								if (Index < CTEnts) {
									//  Valid colour
									(*CIt).R = pCT[Index].B;
									(*CIt).G = pCT[Index].G;
									(*CIt).B = pCT[Index].R;
								}
								RunCount--;
								HiOrderNibble = HiOrderNibble ^ true;
							}

							break;

							//
							//  Insreting an absolte string of symbols
							//
						case RLE_DOING_ABSOLUTE:
							if (HiOrderNibble) Index = (Symbol & 0xF0) >> 4;
							else Index = Symbol & 0x0F;
							if (Index < CTEnts) {
								//  Valid colour
								(*CIt).R = pCT[Index].B;
								(*CIt).G = pCT[Index].G;
								(*CIt).B = pCT[Index].R;
							}

							AbsSymbolCount--;
							HiOrderNibble = HiOrderNibble ^ true;
							if (HiOrderNibble) {
								Symbol = *pString;
								pString++;
							}

							//  If this is the end of the absolute string then switch back to encoded mode
							if (AbsSymbolCount == 0) DecodingState = RLE_DOING_RUN;

							break;

							//
							//  Performing a relative position (move) operation
							//
						case RLE_DOING_POSITION:
							//  Do not update the current pixel
							break;
						}
					}

					//  If we are doing run encoding and we have an EOL here then skip if
					if (DecodingState == RLE_DOING_RUN && RunCount == 0) {
						DIB_RLE_BLOCK* pNextOp = (DIB_RLE_BLOCK*)pNextToken;
						if (pNextOp->RunLen == 0 && pNextOp->Symbol == 0) {
							pCurrentToken = pNextToken;
							pNextToken = pCurrentToken + 2;
						}
					}
				}
			}

			//  Return to caller
			return;
		}

		//  getFileSize
		//
		//  This static function will validate the bitmap file header and return the size of the file and the offset to the pixel array
		//
		//  PARAMETERS
		//
		//		BYTE*			-		Pointer to the in-memory image
		//		size_t&			-		Reference to the size used of the in-memory image
		//		size_t&			-		Reference to the variable to hold the offset to the pixel arry
		//
		//  RETURNS
		//
		//  NOTES
		//

		static size_t		getFileSize(BYTE* pImage, size_t& Offset, size_t& PAOffset) {
			DIB_FILE_HEADER* pDFH = (DIB_FILE_HEADER*)(pImage + Offset);

			PAOffset = 0;
			
			//  Validate the file signature
			if (pDFH->Sig[0] != DEFAULT_DIB_FSIG1 || pDFH->Sig[1] != DEFAULT_DIB_FSIG2) return 0;

			//  Update the offset
			Offset += sizeof(DIB_FILE_HEADER);

			//  Return the pixel array offset and the file size
			PAOffset = pDFH->PAOffset;
			return pDFH->FSize;
		}

		//  getImageHeader
		//
		//  This static function will return a pointer to the bitmap image header and the canvas dimensions
		//
		//  PARAMETERS
		//
		//		BYTE*			-		Pointer to the in-memory image
		//		size_t&			-		Reference to the size used of the in-memory image
		//		size_t&			-		Reference to the variable to hold the canvas height
		//		size_t&			-		Reference to the variable to hold the canvas width
		//
		//  RETURNS
		//
		//  NOTES
		//

		static DIB_HEADER*	getImageHeader(BYTE* pImage, size_t& Offset, size_t& CH, size_t& CW) {
			DIB_HEADER*		pDIH = (DIB_HEADER*)(pImage + Offset);										//  Pointer to the image header

			//  Update the offset
			Offset += sizeof(DIB_HEADER);

			//  Return the canvas dimensions and the pointer to the image header
			CH = pDIH->Height;
			CW = pDIH->Width;
			return pDIH;
		}

		//  buildCanonicalTrain
		//
		//  This static function will build a Canonical Train (Writeable) from the passed Train
		//
		//  PARAMETERS
		//
		//		Train*			-		Pointer to the input train
		//		SWITCHES		-		Optins to use for forming the Canonical Train
		//
		//  RETURNS
		//
		//		Train*			-		Pointer to the constructed Canonical Train
		//
		//  NOTES
		//

		static Train<RGB>* buildCanonicalTrain(Train<RGB>* pTrain, SWITCHES Opts) {
			(void)Opts;
			Train<RGB>*		pCTrain = nullptr;	

			//  For DIB (BMP) images the canonical image is a flattened copy of the input train
			pCTrain = new Train<RGB>(*pTrain);

			pCTrain->flatten();

			//  Return the constructed canonical train
			return pCTrain;
		}

		//  serialiseTrain
		//
		//  This static function will build an in-memory DIB/BMP image from the passed CANONICAL Train.
		//
		//  PARAMETERS
		//
		//		Train*			-		Pointer to the input Canonical Train
		//		size_t&			-		Reference to the size of the in-memory image
		//		SWITCHES		-		Options to use for storing the image
		//
		//  RETURNS
		//
		//		BYTE*			-		Pointer to the constructed in-memory image
		//
		//  NOTES
		//
		//		The input Train MUST be a DIB/BMP CANONICAL Train
		//

		static BYTE* serialiseTrain(Train<RGB>* pTrain, size_t& ImgSize, SWITCHES Opts) {
			BYTE*					pImage = nullptr;											//  Pointer to in-memory image
			size_t					ImgAlc = 0;													//  Allocated size of the in-memory image
			size_t					ImgUsed = 0;												//  Bytes used in the in-memory image
			size_t					ImgEst = 0;													//  (Over) Estimated initial size of the image
			ColourTable<RGB>		CT;															//  Colour Table for image
			Frame<RGB>*				pFrame = pTrain->getFirstFrame();							//  Only frame in the image

			//  Capture the colour usage
			for (RasterBuffer<RGB>::iterator It = pFrame->buffer().begin(); It != pFrame->buffer().end(); It++) CT.add(*It, 1);

			//  Allocate memory for the image
			ImgEst = 4096 + (256 * sizeof(RGBQuad)) + ((pTrain->getCanvasHeight() + 4) * (pTrain->getCanvasWidth() + 4) * sizeof(RGB));
			ImgAlc = 0;
			ImgUsed = 0;
			pImage = (BYTE*) malloc(ImgEst);
			if (pImage == nullptr) return nullptr;
			ImgAlc = ImgEst;
			memset(pImage, 0, ImgEst);

			//  Append the file haeder
			appendFileHeader(pImage, ImgUsed);

			//  Append the image header
			appendImageHeader(pFrame, CT, pImage, ImgUsed, Opts);

			//  Append the image
			appendImage(pFrame, CT, pImage, ImgUsed, Opts);

			//  Update the File Size in the File Header
			DIB_FILE_HEADER* pDFH = (DIB_FILE_HEADER*) pImage;												//  File header structure
			pDFH->FSize = UINT(ImgUsed);

			//  Release unused memory in the image block
			if ((ImgAlc - ImgUsed) > 256) {
				BYTE* pNewIBfr = (BYTE*) realloc(pImage, ImgUsed);
				if (pNewIBfr != nullptr) {
					pImage = pNewIBfr;
					ImgAlc = ImgUsed;
				}
			}

			//  Return the in-memory image to the caller
			ImgSize = ImgUsed;
			return pImage;
		}

		//  appendFileHeader
		//
		//  This static function will append a DIB/BMP File header to the in-memory image.
		//
		//  PARAMETERS
		//
		//		BYTE*			-		Pointer to the in-memory image
		//		size_t&			-		Reference to the size used of the in-memory image
		//
		//  RETURNS
		//
		//  NOTES
		//

		static void		appendFileHeader(BYTE* pImage, size_t& ImgUsed) {
			DIB_FILE_HEADER*	pDFH = (DIB_FILE_HEADER*) (pImage + ImgUsed);									//  File header structure

			//  Clear the structure
			memset(pDFH, 0, sizeof(DIB_FILE_HEADER));

			//  Fill the file header structure, the File size filed will be fixed up later
			pDFH->Sig[0] = DEFAULT_DIB_FSIG1;
			pDFH->Sig[1] = DEFAULT_DIB_FSIG2;
			pDFH->FSize = 0;																					//  To be fixed up later
			pDFH->PAOffset = sizeof(DIB_FILE_HEADER) + sizeof(DIB_HEADER);										//  To be fixed up later

			//  Update the used memory
			ImgUsed += sizeof(DIB_FILE_HEADER);

			//  Return to caller
			return;
		}

		//  appendImageHeader
		//
		//  This static function will append a DIB/BMP Image header to the in-memory image.
		//
		//  PARAMETERS
		//
		//		Frame<RGB>*			-		Pointer to the image frame
		//		ColourTable<RGB>&	-		Reference to the image colour table
		//		Byte*				-		Pointer to the in-memory image
		//		size_t&				-		Reference to the size used of the in-memory image
		//		SWITCHES			-		Options for store operations
		//
		//  RETURNS
		//
		//  NOTES
		//

		static void		appendImageHeader(Frame<RGB>* pFrame, ColourTable<RGB>& CT, BYTE* pImage, size_t& ImgUsed, SWITCHES Opts) {
			DIB_HEADER*			pDIH = (DIB_HEADER*) (pImage + ImgUsed);										//  Image Header

			//  Clear the structure
			memset(pDIH, 0, sizeof(DIB_HEADER));

			//  Fill in the image header
			pDIH->DIBHdrSize = sizeof(DIB_HEADER);
			pDIH->Height = UINT(pFrame->getHeight());
			pDIH->Width = UINT(pFrame->getWidth());
			pDIH->ResH = DIB_RES_72DPI;
			pDIH->ResV = DIB_RES_72DPI;

			//  Determine the encoding to use
			pDIH->Planes = 1;																					//  Always use a single plane

			//  Set the encoding to be used.
			//  NOTE: The actual BitMap size will be fixed up later

			//  If 24 bit RGB encoding is forced then use that setting
			if (Opts & DIB_STORE_OPT_FORCE_RGB24) {
				//  24 bit colour BitMap
				pDIH->BPP = 24;
				pDIH->BitSettings = DIB_ENC_RGB;
				pDIH->BMSize = UINT(pFrame->getHeight() * pFrame->getWidth() * 3);
			}
			else {

				if (CT.getNumColours() <= 2) {
					//  Monochrome BitMap
					pDIH->BPP = 1;
					pDIH->BitSettings = DIB_ENC_RGB;
					pDIH->BMSize = UINT((pFrame->getWidth() * pFrame->getHeight()) / 8);
				}
				else if (CT.getNumColours() <= 16) {
					//  16 colour BitMap
					pDIH->BPP = 4;
					//  NOTE: If we assert RLE4 encoding here it may revert to uncompressed RGB encoding later if the
					//  option is not forced and the compressed stream is larger than the uncompressed stream
					if (Opts & (DIB_STORE_OPT_ENABLE_RLEC | DIB_STORE_OPT_FORCE_RLEC)) pDIH->BitSettings = DIB_ENC_RLE4;
					else pDIH->BitSettings = DIB_ENC_RGB;
					pDIH->BMSize = UINT((pFrame->getWidth() * pFrame->getHeight()) / 2);
				}
				else if (CT.getNumColours() <= 256) {
					//  256 colour BitMap
					pDIH->BPP = 8;
					//  NOTE: If we assert RLE8 encoding here it may revert to uncompressed RGB encoding later if the
					//  option is not forced and the compressed stream is larger than the uncompressed stream
					if (Opts & (DIB_STORE_OPT_ENABLE_RLEC | DIB_STORE_OPT_FORCE_RLEC)) pDIH->BitSettings = DIB_ENC_RLE8;
					else pDIH->BitSettings = DIB_ENC_RGB;
					pDIH->BMSize = UINT(pFrame->getWidth() * pFrame->getHeight());
				}
				else {
					//  24 bit colour BitMap
					pDIH->BPP = 24;
					pDIH->BitSettings = DIB_ENC_RGB;
					pDIH->BMSize = UINT(pFrame->getHeight() * pFrame->getWidth() * 3);
				}
			}

			//  Update the size used
			ImgUsed += sizeof(DIB_HEADER);

			//  Return to caller
			return;
		}

		//  appendImage
		//
		//  This static function will append a DIB/BMP Image to the in-memory image.
		//
		//  PARAMETERS
		//
		//		Frame<RGB>*			-		Pointer to the image frame
		//		ColourTable<RGB>&	-		Reference to the image colour table
		//		Byte*				-		Pointer to the in-memory image
		//		size_t&				-		Reference to the size used of the in-memory image
		//		SWITCHES			-		Options for store operations
		//
		//  RETURNS
		//
		//  NOTES
		//
		//		1.	The RGBTRIPLE and RGBQUAD structures in the specification store the Blue (B) first and the Red (R) last.
		//			The R & B members are swapped on assignment to maintain the correct colours.
		//

		static void		appendImage(Frame<RGB>* pFrame, ColourTable<RGB>& CT, BYTE* pImage, size_t& ImgUsed, SWITCHES Opts) {
			DIB_FILE_HEADER*	pDFH = (DIB_FILE_HEADER*) pImage;												//  File Header
			DIB_HEADER*			pDIH = (DIB_HEADER*) ((pImage + ImgUsed) - sizeof(DIB_HEADER));					//  Image Header
			size_t				ScanLineSize = 0;																//  Scan line size
			size_t				BMISize = 0;																	//  Bit Map Image size
			size_t				LineSize = 0;																	//  Line size written
			size_t				CISize = 0;																		//  Compressed image size
			RGB*				pPixel = nullptr;																//  Pixel pointer for 24 bit encoding
			RGBQuad*			pCTE = nullptr;																	//  Pointer to a colour table entry
			RGB					Colour = {};																	//  Pixel Colour
			BYTE*				p8Pixels = nullptr;																//  8 pixel array for monochrome encoding
			BYTE				PX8 = 0;																		//  8 pixel array for monochrome encoding
			size_t				BitsCaptured = 0;																//  Bits captured
			BYTE*				p2Pixels = nullptr;																//  2 pixel array for 16 colour encodng
			BYTE				PX2 = 0;																		//  2 pixel array for 16 colour encodng
			size_t				CX = 0;																			//  Colour Index
			BYTE*				p1Pixels = nullptr;																//  Single pixel for 256 colours encoding

			//
			//  If the Bites per Pixel is set to be less than 24 then we need to first append the colour table
			//

			switch (pDIH->BPP) {

			case 1:
				//  Monochrome - 2 entries in the colour table
				ScanLineSize = pFrame->getWidth() / 8;
				if ((pFrame->getWidth() % 8) > 0) ScanLineSize++;
				if ((ScanLineSize % 4) > 0) ScanLineSize += (4 - (ScanLineSize % 4));
				pCTE = (RGBQuad*) (pImage + ImgUsed);
				for (CX = 0; CX < 2; CX++) {
					Colour = CT.getColour(CX);
					pCTE->R = Colour.B;
					pCTE->G = Colour.G;
					pCTE->B = Colour.R;
					pCTE->Pad = 0;
					pCTE++;
				}
				ImgUsed += 2 * sizeof(RGBQuad);
				pDFH->PAOffset += 2 * sizeof(RGBQuad);
				pDIH->Colours = 2;
				pDIH->IColours = 2;
				break;

			case 4:
				//  16 colour
				ScanLineSize = pFrame->getWidth() / 2;
				ScanLineSize += (pFrame->getWidth() % 2);
				if ((ScanLineSize % 4) > 0) ScanLineSize += (4 - (ScanLineSize % 4));
				pCTE = (RGBQuad*) (pImage + ImgUsed);
				for (CX = 0; CX < 16; CX++) {
					Colour = CT.getColour(CX);
					pCTE->R = Colour.B;
					pCTE->G = Colour.G;
					pCTE->B = Colour.R;
					pCTE->Pad = 0;
					pCTE++;
				}
				ImgUsed += 16 * sizeof(RGBQuad);
				pDFH->PAOffset += 16 * sizeof(RGBQuad);
				pDIH->Colours = 16;
				pDIH->IColours = UINT(CT.getNumColours());
				break;

			case 8:
				//  256 colour
				ScanLineSize = pFrame->getWidth();
				if ((ScanLineSize % 4) > 0) ScanLineSize += (4 - (ScanLineSize % 4));
				pCTE = (RGBQuad*)(pImage + ImgUsed);
				for (CX = 0; CX < 256; CX++) {
					Colour = CT.getColour(CX);
					pCTE->R = Colour.B;
					pCTE->G = Colour.G;
					pCTE->B = Colour.R;
					pCTE->Pad = 0;
					pCTE++;
				}
				ImgUsed += 256 * sizeof(RGBQuad);
				pDFH->PAOffset += 256 * sizeof(RGBQuad);
				pDIH->Colours = 256;
				pDIH->IColours = UINT(CT.getNumColours());
				break;

			case 24:
				//  24 bit colour - NO COLOUR TABLE
				ScanLineSize = pFrame->getWidth() * sizeof(RGB);
				if ((ScanLineSize % 4) > 0) ScanLineSize += (4 - (ScanLineSize % 4));
				pDIH->Colours = 0;
				pDIH->IColours = 0;
				break;
			}

			//  Append the actual image using the specified encoding
			switch (pDIH->BPP) {

			case 1:
				//  Monochrome
				p8Pixels = pImage + ImgUsed;

				for (RasterBuffer<RGB>::iterator RIt = pFrame->buffer().bottom(); RIt != pFrame->buffer().top(); RIt--) {
					//  Perform a scan line addition
					LineSize = 0;
					PX8 = 0;
					BitsCaptured = 0;
					for (RasterBuffer<RGB>::iterator CIt = pFrame->buffer().left(RIt); CIt != pFrame->buffer().right(RIt); CIt++) {

						//  Determine monochrome encoding
						if (*CIt == CT.getColour(1)) PX8 = PX8 | 1;
						BitsCaptured++;

						//  If 8 pixels have been captured then add the encoded byte to the scan line
						if (BitsCaptured == 8) {
							*p8Pixels = PX8;
							p8Pixels++;
							LineSize++;
							PX8 = 0;
							BitsCaptured = 0;
						}
						else PX8 = PX8 << 1;
					}

					//  Deal with any residue in the accumulated scan encoding
					if (BitsCaptured > 0) {
						PX8 = PX8 << (8 - BitsCaptured);
						*p8Pixels = PX8;
						p8Pixels++;
						LineSize++;
					}

					//  Pad the scan line
					while (LineSize < ScanLineSize) {
						*p8Pixels = 0;
						p8Pixels++;
						LineSize++;
					}

					BMISize += LineSize;
				}
				break;

			case 4:

				//  If RLE compression is enabled then build the compressed image buffer. If the option is not forced
				//  and the compressed size is larger than the uncompressed size then revert to the uncompressed encoding

				if (pDIH->BitSettings == DIB_ENC_RLE4) {
					BYTE*	pCB = compressImageRLE4(pFrame->buffer(), CT, CISize);

					//  Determine if compression was possible
					if (pCB == nullptr || CISize == 0) {
						pDIH->BitSettings = DIB_ENC_RGB;
					}
					else {
						//  If the compressed encoding is forced OR the compressed buffer is smaller than the uncompressed
						//  use the compressed buffer, otherwise recert to uncompressed
						if (Opts & DIB_STORE_OPT_FORCE_RLEC || CISize < (ScanLineSize * pFrame->getHeight())) {
							//  Append the compressed image to the on-dosk image
							BMISize = CISize;
							memcpy(pImage + ImgUsed, pCB, BMISize);
							free(pCB);
							break;
						}
						else {
							//  Revert to uncompressed
							pDIH->BitSettings = DIB_ENC_RGB;
							free(pCB);
						}	
					}
				}

				//  16 colour uncompressed
				p2Pixels = pImage + ImgUsed;

				for (RasterBuffer<RGB>::iterator RIt = pFrame->buffer().bottom(); RIt != pFrame->buffer().top(); RIt--) {
					//  Perform a scan line addition
					LineSize = 0;
					PX2 = 0;
					BitsCaptured = 0;
					for (RasterBuffer<RGB>::iterator CIt = pFrame->buffer().left(RIt); CIt != pFrame->buffer().right(RIt); CIt++) {

						//  Determine the colour index
						for (CX = 0; CX < CT.getNumColours(); CX++) {
							if (*CIt == CT.getColour(CX)) break;
						}
						PX2 = PX2 | BYTE(CX);
						BitsCaptured += 4;

						//  If 2 pixels have been captured then add the encoded byte to the scan line
						if (BitsCaptured == 8) {
							*p2Pixels = PX2;
							p2Pixels++;
							LineSize++;
							PX2 = 0;
							BitsCaptured = 0;
						}
						else PX2 = PX2 << 4;
					}

					//  Deal with any residue in the accumulated scan encoding
					if (BitsCaptured > 0) {
						PX2 = PX2 << 4;
						*p2Pixels = PX2;
						p2Pixels++;
						LineSize++;
					}

					//  Pad the scan line
					while (LineSize < ScanLineSize) {
						*p2Pixels = 0;
						p2Pixels++;
						LineSize++;
					}

					BMISize += LineSize;
				}
				break;

			case 8:

				//  If RLE compression is enabled then build the compressed image buffer. If the option is not forced
				//  and the compressed size is larger than the uncompressed size then revert to the uncompressed encoding

				if (pDIH->BitSettings == DIB_ENC_RLE8) {
					BYTE* pCB = compressImageRLE8(pFrame->buffer(), CT, CISize);

					//  Determine if compression was possible
					if (pCB == nullptr || CISize == 0) {
						pDIH->BitSettings = DIB_ENC_RGB;
					}
					else {
						//  If the compressed encoding is forced OR the compressed buffer is smaller than the uncompressed
						//  use the compressed buffer, otherwise recert to uncompressed
						if (Opts & DIB_STORE_OPT_FORCE_RLEC || CISize < (ScanLineSize * pFrame->getHeight())) {
							//  Append the compressed image to the on-dosk image
							BMISize = CISize;
							memcpy(pImage + ImgUsed, pCB, BMISize);
							free(pCB);
							break;
						}
						else {
							//  Revert to uncompressed
							pDIH->BitSettings = DIB_ENC_RGB;
							free(pCB);
						}
					}
				}

				//  256 colour Uncompressed
				p1Pixels = pImage + ImgUsed;

				for (RasterBuffer<RGB>::iterator RIt = pFrame->buffer().bottom(); RIt != pFrame->buffer().top(); RIt--) {
					//  Perform a scan line addition
					LineSize = 0;
					for (RasterBuffer<RGB>::iterator CIt = pFrame->buffer().left(RIt); CIt != pFrame->buffer().right(RIt); CIt++) {

						//  Determine the colour index
						for (CX = 0; CX < CT.getNumColours(); CX++) {
							if (*CIt == CT.getColour(CX)) break;
						}
						*p1Pixels = BYTE(CX);
						p1Pixels++;
						LineSize++;
					}

					//  Pad the scan line
					while (LineSize < ScanLineSize) {
						*p1Pixels = 0;
						p1Pixels++;
						LineSize++;
					}

					BMISize += LineSize;
				}
				break;

			case 24:
				//  24 bit colour
				//  NOTE: Triplets are stored in the image array in BGR order
				pPixel = (RGB*) (pImage + ImgUsed);

				for (RasterBuffer<RGB>::iterator RIt = pFrame->buffer().bottom(); RIt != pFrame->buffer().top(); RIt--) {
					//  Perform a scan line addition
					LineSize = 0;
					for (RasterBuffer<RGB>::iterator CIt = pFrame->buffer().left(RIt); CIt != pFrame->buffer().right(RIt); CIt++) {
						pPixel->R = (*CIt).B;
						pPixel->G = (*CIt).G;
						pPixel->B = (*CIt).R;
						pPixel++;
						LineSize += 3;
					}

					//  Pad the scan line
					while (LineSize < ScanLineSize) {
						pPixel->R = 0;
						pPixel = (RGB*) ((BYTE*) pPixel + 1);
						LineSize++;
					}

					BMISize += LineSize;
				}

				break;
			}

			//  Update the image size
			pDIH->BMSize = UINT(BMISize);
			ImgUsed += BMISize;

			//  Return to caller
			return;
		}

		//  compressImageRLE8
		//
		//  This static function will construct a buffer holding the RLE8 encoded compressed image
		//
		//  PARAMETERS
		//
		//		RasterBuffer<RGB>&	-		Reference to the raster buffer holding the image
		//		ColourTable<RGB>&	-		Reference to the image colour table
		//		size_t&				-		Reference to the size of the compressed image
		//
		//  RETURNS
		//
		//		BYTE*				-		Memory buffer holding the compressed image
		//
		//  NOTES
		//
		//

		static BYTE* compressImageRLE8(RasterBuffer<RGB>& RB, ColourTable<RGB>& CT, size_t& CISize) {
			ByteStream	bsCB((RB.getHeight() * RB.getWidth() * 2), 0);						//  Byte steam to hold the buffer
			size_t		CX = 0;																//  Colour Index
			BYTE		NewSymbol = 0;														//  New Symbol read from the image

			//  Controls for a Run
			BYTE		Symbol = 0;															//  Symbol for the run
			size_t		RunLen = 0;															//  Run Length

			//  Controls for an absolute pixel string
			size_t		StringLen = 0;														//  String Length
			BYTE		String[256] = {};													//  String buffer
			BYTE		EOBSymbol = 0;														//  End of Buffer Symbol
			size_t		SameEOBCount = 0;													//  Count of symbols the same at the end of buffer

			//  Clear the returned size
			CISize = 0;

			//
			//  Iterate over the raster buffer collection the compressed image from the bottom to the top
			//

			for (RasterBuffer<RGB>::iterator RIt = RB.bottom(); RIt != RB.top(); RIt--) {
				for (RasterBuffer<RGB>::iterator CIt = RB.left(RIt); CIt != RB.right(RIt); CIt++) {

					//  Capture the next symbol
					//  Determine the colour index
					for (CX = 0; CX < CT.getNumColours(); CX++) {
						if (*CIt == CT.getColour(CX)) break;
					}
					NewSymbol = BYTE(CX);

					if (RunLen > 0) {
						//  A possible run is in progress
						if (NewSymbol == Symbol) {
							//  If the run is maxed out then emit it and start a new run
							if (RunLen == 255) {
								bsCB.next(BYTE(RunLen));
								bsCB.next(Symbol);
								RunLen = 0;
							}
							RunLen++;
						}
						else {
							//  Run has ended - if the run is long enough then emit the run
							if (RunLen > 2) {
								bsCB.next(BYTE(RunLen));
								bsCB.next(Symbol);
								//  New symbol starts the next run
								Symbol = NewSymbol;
								RunLen = 1;
							}
							else {
								//  Short Run (1 or 2 symbols)
								//  Switch to a possible absoulute string
								while (RunLen > 0) {
									RunLen--;
									String[RunLen] = Symbol;
									StringLen++;
								}

								String[StringLen] = NewSymbol;
								StringLen++;
								EOBSymbol = NewSymbol;
								SameEOBCount = 1;
							}
						}
					}
					else if (StringLen > 0) {
						//  A possible absolute string is in progress
						//  Add the new symbol to the end of the string
						String[StringLen] = NewSymbol;
						StringLen++;
						if (NewSymbol == EOBSymbol) SameEOBCount++;
						else {
							EOBSymbol = NewSymbol;
							SameEOBCount = 1;
						}

						//  If we have a run of 3 same at the end of the buffer then emit the absoloute string and switch back to a run
						//  with the 3 symbols
						if (SameEOBCount == 3) {
							if (StringLen > 5) {
								//  Emit as an absolute string
								StringLen -= 3;
								bsCB.next(0);
								bsCB.next(BYTE(StringLen));
								for (size_t BX = 0; BX < StringLen; BX++) {
									bsCB.next(String[BX]);
								}
								//  Pad if necessary
								if (StringLen & 1) {
									bsCB.next(0);
								}
							}
							else {
								//  Emit the one or two symbols as a single symbol run
								StringLen -= 3;
								for (size_t BX = 0; BX < StringLen; BX++) {
									bsCB.next(1);
									bsCB.next(String[BX]);
								}	
							}

							StringLen = 0;
							Symbol = EOBSymbol;
							RunLen = 3;
						}
					}
					else {
						//  Start of a line - the first symbol becomes the first of a possible run
						Symbol = NewSymbol;
						RunLen = 1;
					}
				}

				//  Emit the last symbol from the line
				if (RunLen > 0) {
					bsCB.next(BYTE(RunLen));
					bsCB.next(Symbol);
					RunLen = 0;
				}
				else {
					if (StringLen > 0) {
						if (StringLen > 2) {
							bsCB.next(0);
							bsCB.next(BYTE(StringLen));
							for (size_t BX = 0; BX < StringLen; BX++) {
								bsCB.next(String[BX]);
							}
							//  Pad if necessary
							if (StringLen & 1) {
								bsCB.next(0);
							}
						}
						else {
							for (size_t BX = 0; BX < StringLen; BX++) {
								bsCB.next(1);
								bsCB.next(String[BX]);
							}
						}
						StringLen = 0;
					}
				}

				//  Emit the End Of Line
				bsCB.next(0);
				bsCB.next(0);
			}

			//  Emit - End-Of-Bitmap
			bsCB.next(0);
			bsCB.next(1);

			//  Return the compressed buffer to the caller
			return bsCB.acquireBuffer(CISize);
		}

		//  compressImageRLE4
		//
		//  This static function will construct a buffer holding the RLE4 encoded compressed image
		//
		//  PARAMETERS
		//
		//		RasterBuffer<RGB>&	-		Reference to the raster buffer holding the image
		//		ColourTable<RGB>&	-		Reference to the image colour table
		//		size_t&				-		Reference to the size of the compressed image
		//
		//  RETURNS
		//
		//		BYTE*				-		Memory buffer holding the compressed image
		//
		//  NOTES
		//
		//  The RLE4 compression scheme is identical to the RLE8 scheme except that runs may encode alternating 4 bit
		//  symbols and strings are packed in 4 bit nibbles.
		//
		//

		static BYTE* compressImageRLE4(RasterBuffer<RGB>& RB, ColourTable<RGB>& CT, size_t& CISize) {
			ByteStream	bsCB((RB.getHeight() * RB.getWidth() * 2), 0);						//  Byte steam to hold the buffer
			MSBitStream bsOut(bsCB, true);													//  Output Bit Stream
			size_t		CX = 0;																//  Colour Index
			BYTE		NewSymbol = 0;														//  New Symbol read from the image

			//  Controls for a Run
			BYTE		Symbol = 0;															//  Symbol for the run
			size_t		RunLen = 0;															//  Run Length

			//  Controls for an absolute pixel string
			size_t		StringLen = 0;														//  String Length
			BYTE		String[256] = {};													//  String buffer
			BYTE		EOBSymbol = 0;														//  End of Buffer Symbol
			size_t		SameEOBCount = 0;													//  Count of symbols the same at the end of buffer

			//  Controls for an alternating pixel run
			size_t		AltStringLen = 0;													//  Alternating String Length
			BYTE		Symbol1 = 0;														//  First symbol
			BYTE		Symbol2 = 0;														//  Second symbol

			//  Clear the returned size
			CISize = 0;

			//
			//  Iterate over the raster buffer collection the compressed image from the bottom to the top
			//

			for (RasterBuffer<RGB>::iterator RIt = RB.bottom(); RIt != RB.top(); RIt--) {
				for (RasterBuffer<RGB>::iterator CIt = RB.left(RIt); CIt != RB.right(RIt); CIt++) {

					//  Capture the next symbol
					//  Determine the colour index
					for (CX = 0; CX < CT.getNumColours(); CX++) {
						if (*CIt == CT.getColour(CX)) break;
					}
					NewSymbol = BYTE(CX);

					if (RunLen > 0) {
						//  A possible run is in progress
						if (NewSymbol == Symbol) {
							//  If the run is maxed out then emit it and start a new run
							if (RunLen == 255) {
								bsOut.next(BYTE(RunLen), 8);
								bsOut.next(Symbol, 4);
								bsOut.next(Symbol, 4);
								RunLen = 0;
							}
							RunLen++;
						}
						else {
							//  Run has ended - if the run is long enough then emit the run
							if (RunLen > 2) {
								bsOut.next(BYTE(RunLen), 8);
								bsOut.next(Symbol, 4);
								bsOut.next(Symbol, 4);
								//  New symbol starts the next run
								Symbol = NewSymbol;
								RunLen = 1;
							}
							else {
								//  Short Run (1 or 2 symbols)
								//  Switch to a possible absoulute string
								while (RunLen > 0) {
									RunLen--;
									String[RunLen] = Symbol;
									StringLen++;
								}

								String[StringLen] = NewSymbol;
								StringLen++;
								EOBSymbol = NewSymbol;
								SameEOBCount = 1;
							}
						}
					}
					else if (StringLen > 0) {
						//  A possible absolute string is in progress
						//  Add the new symbol to the end of the string
						String[StringLen] = NewSymbol;
						StringLen++;
						if (NewSymbol == EOBSymbol) SameEOBCount++;
						else {
							EOBSymbol = NewSymbol;
							SameEOBCount = 1;
						}

						//  If we have a run of 3 same at the end of the buffer then emit the absoloute string and switch back to a run
						//  with the 3 symbols
						if (SameEOBCount == 3) {
							if (StringLen > 5) {
								//  Emit as an absolute string
								StringLen -= 3;

								//  Test for a possible alternating pixel string that can be converted to a run
								AltStringLen = 0;
								Symbol1 = String[0];
								Symbol2 = String[1];

								//  Count the number of alternating symbols in the string
								while (AltStringLen < StringLen) {
									if (AltStringLen & 1) {
										if (String[AltStringLen] == Symbol2) AltStringLen++;
										else break;
									}
									else {
										if (String[AltStringLen] == Symbol1) AltStringLen++;
										else break;
									}
								}

								//  See if any alternating string was found
								if (AltStringLen > 2) {

									//  Emit the alternating string as a run
									bsOut.next(uint32_t(AltStringLen), 8);
									bsOut.next(Symbol1, 4);
									bsOut.next(Symbol2, 4);

									//  Remove the alternating string from the string
									memmove(&String[0], &String[AltStringLen], StringLen - AltStringLen);
									StringLen -= AltStringLen;
								}

								if (StringLen > 0) {
									if (StringLen > 2) {
										bsOut.next(0, 8);
										bsOut.next(BYTE(StringLen), 4);
										for (size_t BX = 0; BX < StringLen; BX++) {
											bsOut.next(String[BX], 4);
										}
										//  Pad if necessary
										size_t  StringLenWritten = StringLen / 2;
										if (StringLen & 1) {
											bsOut.next(0, 4);
											StringLenWritten++;
										}
										if (StringLenWritten & 1) {
											bsOut.next(0, 8);
										}
									}
									else {
										//  Emit the one or two symbols as a single symbol run
										StringLen -= 3;
										if (StringLen == 2) {
											bsOut.next(2, 8);
											bsOut.next(String[0], 4);
											bsOut.next(String[1], 4);
										}
										else {
											bsOut.next(1, 8);
											bsOut.next(String[0], 4);
											bsOut.next(0, 4);
										}
									}
								}
							}
							else {
								//  Emit the one or two symbols as a single symbol run
								StringLen -= 3;
								if (StringLen == 2) {
									bsOut.next(2, 8);
									bsOut.next(String[0], 4);
									bsOut.next(String[1], 4);
								}
								else {
									bsOut.next(1, 8);
									bsOut.next(String[0], 4);
									bsOut.next(0, 4);
								}
							}

							StringLen = 0;
							Symbol = EOBSymbol;
							RunLen = 3;
						}
					}
					else {
						//  Start of a line - the first symbol becomes the first of a possible run
						Symbol = NewSymbol;
						RunLen = 1;
					}
				}

				//  Emit the last symbol from the line
				if (RunLen > 0) {
					bsOut.next(BYTE(RunLen), 8);
					bsOut.next(Symbol, 4);
					bsOut.next(Symbol, 4);
					RunLen = 0;
				}
				else {
					if (StringLen > 0) {
						if (StringLen > 2) {
							bsOut.next(0, 8);
							bsOut.next(BYTE(StringLen), 8);
							for (size_t BX = 0; BX < StringLen; BX++) {
								bsOut.next(String[BX], 4);
							}
							//  Pad if necessary
							size_t  StringLenWritten = StringLen / 2;
							if (StringLen & 1) {
								bsOut.next(0, 4);
								StringLenWritten++;
							}
							if (StringLenWritten & 1) {
								bsOut.next(0, 8);
							}
						}
						else {
							if (StringLen == 2) {
								bsOut.next(2, 8);
								bsOut.next(String[0], 4);
								bsOut.next(String[1], 4);
							}
							else {
								bsOut.next(1, 8);
								bsOut.next(String[0], 4);
								bsOut.next(0, 4);
							}
						}
						StringLen = 0;
					}
				}

				//  Emit the End Of Line
				bsOut.next(0, 8);
				bsOut.next(0, 8);
			}

			//  Emit - End-Of-Bitmap
			bsOut.next(0, 8);
			bsOut.next(1, 8);

			//  Flush the stream
			bsOut.flush();

			//  Return the compressed buffer to the caller
			return bsCB.acquireBuffer(CISize);
		}

		//
		//  Analysis functions
		//

		//  showFileHeader
		//
		//  This static function will document the contents of the BitMap file header
		//
		//  PARAMETERS
		//
		//		BYTE*				-		Pointer to the in-memory image
		//		size_t&				-		Reference to the offset in the in-memory image
		//		size_t				-		Block Number
		//		std::ostream&		-		Reference to the output stream
		//
		//  RETURNS
		//
		//  NOTES
		//

		static void		showFileHeader(BYTE* pImage, size_t& Offset, size_t BlockNo, std::ostream& OS) {
			DIB_FILE_HEADER*		pDFH = (DIB_FILE_HEADER*) (pImage + Offset);

			OS << std::endl;
			OS << "DIB/BMP FILE HEADER - Block: " << BlockNo << ", Offset: +" << Offset << ", Size: " << sizeof(DIB_FILE_HEADER) << "." << std::endl;
			OS << std::endl;
			MemoryDumper::dumpMemory(pImage + Offset, sizeof(DIB_FILE_HEADER), "DFH", OS);

			//  Show the block contents
			OS << std::endl;
			OS << "File Signature:        '" << pDFH->Sig[0] << pDFH->Sig[1] << "'." << std::endl;
			OS << "File Size:             " << pDFH->FSize << "." << std::endl;
			OS << "App Data (1):          " << pDFH->App1 << "." << std::endl;
			OS << "App Data (2):          " << pDFH->App2 << "." << std::endl;
			OS << "Offset to Pixel Array: " << pDFH->PAOffset << "." << std::endl;

			//  Update the offset
			Offset += sizeof(DIB_FILE_HEADER);

			//  Return to caller
			return;
		}

		//  showImageHeader
		//
		//  This static function will document the contents of the BitMap image header
		//
		//  PARAMETERS
		//
		//		BYTE*				-		Pointer to the in-memory image
		//		size_t&				-		Reference to the offset in the in-memory image
		//		size_t				-		Block Number
		//		std::ostream&		-		Reference to the output stream
		//
		//  RETURNS
		//
		//  NOTES
		//

		static void		showImageHeader(BYTE* pImage, size_t& Offset, size_t BlockNo, std::ostream& OS) {
			DIB_HEADER*			pDIH = (DIB_HEADER*)(pImage + Offset);
			char				PrintLine[MAX_PATH] = {};										//  Printline

			OS << std::endl;
			OS << "DIB/BMP IMAGE HEADER - Block: " << BlockNo << ", Offset: +" << Offset << ", Size: " << sizeof(DIB_HEADER) << "." << std::endl;
			OS << std::endl;
			MemoryDumper::dumpMemory(pImage + Offset, sizeof(DIB_HEADER), "DIH", OS);

			//  Show the block contents
			OS << std::endl;
			OS << "Header Size:            " << pDIH->DIBHdrSize << "." << std::endl;
			OS << "Image Height:           " << pDIH->Height << "." << std::endl;
			OS << "Image Width:            " << pDIH->Width << "." << std::endl;
			OS << "Display Planes:         " << pDIH->Planes << "." << std::endl;
			OS << "Bits Per Pixel:         " << pDIH->BPP << "." << std::endl;
			sprintf_s(PrintLine, MAX_PATH, "Bit Fields: \t\t%i (0x%.4x).", int(pDIH->BitSettings), int(pDIH->BitSettings));
			OS << PrintLine << std::endl;
			if (pDIH->BitSettings == DIB_ENC_RGB) OS << "     " << pDIH->BitSettings << " - No Compression." << std::endl;
			else if (pDIH->BitSettings == DIB_ENC_RLE4) OS << "     " << pDIH->BitSettings << " - RLE4 Compression." << std::endl;
			else if (pDIH->BitSettings == DIB_ENC_RLE8) OS << "     " << pDIH->BitSettings << " - RLE8 Compression." << std::endl;
			OS << "Image Bit Map Size:     " << pDIH->BMSize << "." << std::endl;
			OS << "Resolution (Horiz):     " << pDIH->ResH << "." << std::endl;
			OS << "Resolution (Vert):      " << pDIH->ResV << "." << std::endl;
			OS << "Colours:                " << pDIH->Colours << "." << std::endl;
			OS << "Important Colours:      " << pDIH->IColours << "." << std::endl;

			//  Update the offset
			Offset += sizeof(DIB_HEADER);

			//  Return to caller
			return;
		}

		//  showColourTable
		//
		//  This static function will document the contents of the BitMap Colour Table (if one is present)
		//
		//  PARAMETERS
		//
		//		BYTE*				-		Pointer to the in-memory image
		//		size_t&				-		Reference to the offset in the in-memory image
		//		size_t				-		Block Number
		//		std::ostream&		-		Reference to the output stream
		//
		//  RETURNS
		//
		//		bool				-		true if the image has a colour table, otherwise false
		//
		//  NOTES
		//

		static bool		showColourTable(BYTE* pImage, size_t& Offset, size_t BlockNo, std::ostream& OS) {
			DIB_HEADER*		pDIH = (DIB_HEADER*) (pImage + Offset - sizeof(DIB_HEADER));
			size_t			CTEnts = 0;																	//  Number of entries in the colour table
			size_t			CTSize = 0;																	//  Size of the colour table
			RGBQuad*		pCTE = (RGBQuad*)(pImage + Offset);											//  Pointer to first colour table entry

			//  See if there is a colour table present - 24 bit colour DIBs have no colour table, otherwize determine the size
			switch (pDIH->BPP) {
			case 1:
				//  Monochrome (2 colour table entries)
				CTEnts = 2;
				CTSize = CTEnts * sizeof(RGBQuad);
				break;

			case 4:
				//  16 colour table entries
				CTEnts = 16;
				CTSize = CTEnts * sizeof(RGBQuad);
				break;

			case 8:
				//  256 colour table entries
				CTEnts = 256;
				CTSize = CTEnts * sizeof(RGBQuad);
				break;

			case 24:
				//  No colour tab;e
				return false;

			default:
				//  Invalid setting
				OS << "ERROR: The Bits Per Pixel setting: " << pDIH->BPP << " is invalid." << std::endl;
				return false;
			}

			OS << std::endl;
			OS << "DIB/BMP COLOUR TABLE - Block: " << BlockNo << ", Offset: +" << Offset << ", Size: " << CTSize << "." << std::endl;
			OS << std::endl;
			MemoryDumper::dumpMemory(pImage + Offset, CTSize, "CTA", OS);

			//  Dump each entry in the table
			OS << std::endl;
			OS << "Number of Colours:     " << CTEnts << "." << std::endl;
			OS << std::endl;
			
			for (size_t CX = 0; CX < CTEnts; CX++) {
				OS << "Index: " << CX << ", Colour: {R: " << int(pCTE->R) << ", G: " << int(pCTE->G) << ", B: " << int(pCTE->B) << "}." << std::endl;
				pCTE++;
			}

			//  Update the offset
			Offset += CTSize;

			//  Return to caller
			return true;
		}

		//  showImage
		//
		//  This static function will document the contents of the BitMap image pixel array
		//
		//  PARAMETERS
		//
		//		BYTE*				-		Pointer to the in-memory image
		//		size_t&				-		Reference to the offset in the in-memory image
		//		size_t				-		Block Number
		//		size_t				-		Size of the file
		//		std::ostream&		-		Reference to the output stream
		//
		//  RETURNS
		//
		//  NOTES
		//

		static void		showImage(BYTE* pImage, size_t& Offset, size_t BlockNo, size_t FSize, std::ostream& OS) {
			size_t			IPASize = FSize - Offset;													//  Size of the pixel array
			size_t			DumpLen = IPASize;															//  Length to dump

			//  Cap the dump length
			if (DumpLen > 256) DumpLen = 256;

			OS << std::endl;
			OS << "DIB/BMP IMAGE PIXEL ARRAY - Block: " << BlockNo << ", Offset: +" << Offset << ", Size: " << IPASize << "." << std::endl;
			OS << std::endl;
			MemoryDumper::dumpMemory(pImage + Offset, DumpLen, "IPA", OS);

			//  Update the offset
			Offset += IPASize;

			//  Return to caller
			return;
		}

	};

	//
	//  BMP class - This is an alias for the DIB class
	//

	class BMP : public DIB {
	public:

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Public Constants                                                                                              *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		static const SWITCHES		BMP_STORE_OPT_FORCE_RGB24 = DIB_STORE_OPT_FORCE_RGB24;		//  Enforce RGB 24 bit encodeing
		static const SWITCHES		BMP_STORE_OPT_ENABLE_RLEC = DIB_STORE_OPT_ENABLE_RLEC;		//  Enable RLE compression
		static const SWITCHES		BMP_STORE_OPT_FORCE_RLEC = DIB_STORE_OPT_FORCE_RLEC;		//  Force RLE compression

		//  Prevent instantiation
		BMP() = delete;
	};
}
