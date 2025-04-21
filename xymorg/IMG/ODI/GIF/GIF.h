#pragma	once
//
//*******************************************************************************************************************
//*																													*
//*   File:		  GIF.h																								*
//*   Suite:      xymorg Image Processing - ODI																		*
//*   Version:    1.0.1	  Build:  02																				*
//*   Author:     Ian Tree/HMNL																						*
//*																													*
//*   Copyright 2017 - 2024 Ian J. Tree																				*
//*******************************************************************************************************************
//*	GIF.h																											*
//*																													*
//*	This header file contains the static functions for handling a GIF On-Disk Image.								*
//*																													*
//*	NOTES:																											*
//*																													*
//*																													*
//*******************************************************************************************************************
//*																													*
//*   History:																										*
//*																													*
//*	1.0.0 - 07/03/2014   -  Initial version																			*
//*	1.0.1 - 08/12/2024   -  Winter Cleanup																			*
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
#include	"GIFODI.h"																				//  GIF ODI Definitions
#include	"../../Train.h"																			//  Image Train
#include	"../../ColourTable.h"																	//  Colour Table (array)

//  Include GIF specific components
#include	"../../CODECS/LZW.h"																	//  Lempel-Ziv-Walsh CODEC

namespace xymorg {

	//
	//  GIF class - contains ONLY static functions
	//

	class GIF {
	public:

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Public Constants                                                                                              *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  Image types
		static const int GIF_DEGENERATE_IMAGE = 1;												//  Degenerate Image
		static const int GIF_PLAIN_IMAGE = 2;													//  Plain single frame image
		static const int GIF_TILED_IMAGE = 3;													//  Tiled image
		static const int GIF_ANIMATION = 4;														//  Animated image

		//  Options for storing images
		static const SWITCHES	GIF_STORE_OPT_NOCLEAR = 1;										//  Set the clear policy off in the encoder
		static const SWITCHES	GIF_ANIMATE_OPT_ONCE = 2;										//  Play the animation once only
		static const SWITCHES	GIF_ANIMATE_OPT_PERPETUAL = 4;									//  Play the animation for ever
		static const SWITCHES	GIF_DISABLE_ANIOPT = 8;											//  Disable animation optimisation
		static const SWITCHES	GIF_FRAGMENT_ALTERNATE = 16;									//  Use alternate frame fragmenter
		static const SWITCHES	GIF_FRAGMENT_TIGHT = 32;										//  Use TIGHT (1) expansion for fragmentation
		static const SWITCHES	GIF_FRAGMENT_MEDIUM = 64;										//  Use MEDIUM (3) expansion for fragmentation
		static const SWITCHES	GIF_FRAGMENT_LOOSE = 128;										//  Use LOOSE (5) expansion for fragmentation
		static const SWITCHES	GIF_OPT_RESERVED = 0xFF000000;									//  Reserved for internal controls

		//  Internal controls
		static const SWITCHES	GIF_INT_ANIMATION = 0x80000000;									//  Serialise as animation

		//  Prevent Instantiation
		GIF() = delete;

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

			//  Unzip the image to a train
			pTrain = unbuttonImage(pImage, ImgSize);

			//  Free the image
			free(pImage);

			//  Return the train
			return pTrain;
		}

		//  storeImage
		//
		//  This static function will store the passed RGB Train into the designated GIF Image file
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

			return storeImageRGB(ImgName, VRMap, pTrain, 0);
		}

		//  storeImage
		//
		//  This static function will store the passed RGB Train into the designated GIF Image file
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

			return storeImageRGB(ImgName, VRMap, pTrain, Opts);
		}

		//  storeImageRGB
		//
		//  This static function will store the passed RGB Train into the designated GIF Image file
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

		static bool		storeImageRGB(const char* ImgName, VRMapper& VRMap, Train<RGB>* pTrain, SWITCHES Opts) {
			BYTE*		pImage = nullptr;												//  Pointer to the in-memory image
			size_t		ImgSize = 0;													//  Size of the in-memory image

			//  Safety
			if (ImgName == nullptr) return false;
			if (ImgName[0] == '\0') return false;
			if (pTrain == nullptr) return false;
			if (pTrain->getCanvasHeight() == 0 || pTrain->getCanvasWidth() == 0) return false;

			//  Set the default options when needed
			//  If the animation setting ONCE is not asserted then enforce PERPETUAL
			if (!(Opts & GIF_ANIMATE_OPT_ONCE)) Opts = Opts | GIF_ANIMATE_OPT_PERPETUAL;

			//  Build the in-memory image of the GIF On-Disk image
			pImage = buttonImage(ImgSize, pTrain, Opts);
			if (pImage == nullptr || ImgSize == 0) {
				std::cerr << "ERROR: Unable to build an GIF on-disk image for: " << ImgName << "." << std::endl;
				if (pImage != nullptr) free(pImage);
				return false;
			}

			//  Store the in-memory image  (consu,es the image)
			if (!VRMap.storeResource(ImgName, pImage, ImgSize)) {
				std::cerr << "ERROR: Failed to store GIF image: '" << ImgName << "', (" << ImgSize << " bytes)." << std::endl;
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
		//  additionaly the image will be documented.
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
		//  This static function will load the designated image into memory and provide an annotated dump of the contents
		//  plus optionally the image.
		//
		//  PARAMETERS
		//
		//		bool			-		If true document the image, otherwise document only the On-Disk image
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
			ODIMap			Map = {};															//  Map of the ODI image
			Train<RGB>*		pITrain = nullptr;													//  Pointer to the image train
			char			szFileName[MAX_PATH + 1] = {};										//  File name

			//  Show starting analysis
			OS << "GIF: ON-DISK-IMAGE ANALYSIS" << std::endl;
			OS << "===========================" << std::endl;
			OS << std::endl;
			VRMap.mapFile(ImgName, szFileName, MAX_PATH);
			OS << "Image Name: " << ImgName << " (" << szFileName << ")." << std::endl;

			//  Safety
			if (ImgName == nullptr) {
				OS << "   ERROR: Image File Name is missing." << std::endl;
				return;
			}

			if (ImgName[0] == '\0') {
				OS << "   ERROR: Image File Name is missing." << std::endl;
				return;
			}

			//  Load the on-disk image into memory
			pImage = VRMap.loadResource(ImgName, ImgSize);

			if (pImage == nullptr) {
				OS << "ERROR: The Image could NOT be loaded, invalid name or file not found." << std::endl;
				return;
			}

			//  Show the image size
			OS << "Image Size: " << ImgSize << " bytes." << std::endl;

			//  Map the Image
			Map.Image = pImage;
			Map.ImageSize = ImgSize;
			Map.NumBlocks = 0;
			Map.NBA = 0;
			Map.Blocks = nullptr;

			if (!mapImage(Map)) {
				OS << "   ERROR: The On-Disk-Image is not a valid GIF Image." << std::endl;
				free(pImage);
				return;
			}

			OS << "Image map contains: " << Map.NumBlocks << " image blocks." << std::endl;

			//  Show the File Header Block
			showFileHeader(Map, OS);

			//  Show the Logical Screen Descriptor (LSD)
			showLSD(Map, OS);

			//  Show the Global Colour Table (GCT)
			showGCT(Map, OS);

			//  Show the Image Frames (this also shows an app extension blocks)
			showFrames(Map, OS);

			//  Show the File Trailer
			showFileTrailer(Map, OS);

			//  Free the image and map
			free(pImage);
			free(Map.Blocks);

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
		//*   Private Nested Structures                                                                                     *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  Colour partition table entry
		typedef struct PTE {
		public:
			size_t		H;																			//  Height of partition
			size_t		W;																			//  Width of partition
			size_t		Pixels;																		//  Number of pixels in the partition
			size_t		Colours;																	//  Number of colours wholly contained
			double		Score;																		//  Score for the partition

			PTE() {
				H = 0;
				W = 0;
				Pixels = 0;
				Colours = 0;
				Score = 0.0;
				return;
			}
		} PTE;

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Private Functions                                                                                             *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  buttonImage
		//
		//  This static function button up the passed train into an on-disk GIF image format
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

			//  Construct the Canonical Train from the input train. The Canonical Train is directly writeble as a GIF image.
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
			Train<RGB>*		pTrain = nullptr;													//  Constructed Train	
			ODIMap			Map = {};															//  Map of the ODI image
			size_t			BlockNo = 0;														//  Block number
			size_t			GCTEnts = 0;														//  Number of entries in the GCT
			GIF_ODI_COLOUR_TABLE* pGCT = nullptr;												//  Pointer to the Global Colour Table (GCT)

			//  Build the map of the image
			Map.Image = pImage;
			Map.ImageSize = Size;
			Map.NumBlocks = 0;
			Map.NBA = 0;
			Map.Blocks = nullptr;

			if (!mapImage(Map)) return nullptr;
			if (Map.Blocks == nullptr) return nullptr;

			size_t		BlkTot = 0;
			for (BlockNo = 0; BlockNo < Map.NumBlocks; BlockNo++) {
				BlkTot += Map.Blocks[BlockNo].BlockSize;
			}

			//  Create the train
			pTrain = new Train<xymorg::RGB>();

			//  Set the Canvas Size
			BlockNo = 1;
			if (Map.Blocks[BlockNo].Block == nullptr) return nullptr;
			GIF_LOGICAL_SCREEN_DESCRIPTOR* pLSD = (GIF_LOGICAL_SCREEN_DESCRIPTOR*) Map.Blocks[BlockNo].Block;
			pTrain->setCanvasHeight(GetSize(pLSD->height));
			pTrain->setCanvasWidth(GetSize(pLSD->width));

			//  If the image has a Global Colour Table then set the background colour
			BlockNo = 2;
			if (Map.Blocks[BlockNo].BlockType == GIF_BLOCK_GCT) {
				pGCT = (GIF_ODI_COLOUR_TABLE*) Map.Blocks[BlockNo].Block;
				pTrain->setBackground(pGCT->Entry[pLSD->background]);
				GCTEnts = ColourTableSize(pLSD->bitsettings);
				BlockNo++;
			}

			//  Capture each of the image frames in turn and append to the train
			while (BlockNo < Map.NumBlocks) {

				//  Scan for the next Start-Of-Frame (Either a GCE or ID block)
				while (BlockNo < Map.NumBlocks) {
					if (Map.Blocks[BlockNo].BlockType == GIF_BLOCK_GCE) break;
					if (Map.Blocks[BlockNo].BlockType == GIF_BLOCK_ID) break;
					BlockNo++;
				}

				//  Unbutton the Frame
				if (BlockNo < Map.NumBlocks) {
					Frame<RGB>* pFrame = unbuttonFrame(Map, BlockNo, pGCT, GCTEnts);
					if (pFrame == nullptr) {
						delete pTrain;
						free(Map.Blocks);
						Map.NBA = Map.NumBlocks = 0;
						return nullptr;
					}

					pTrain->append(pFrame);

					//  Skip over the Frame
					if (Map.Blocks[BlockNo].BlockType == GIF_BLOCK_GCE) BlockNo += 2;
					else BlockNo++;
				}
			}

			//  Free the Map Blocks
			if (Map.Blocks != nullptr) free(Map.Blocks);

			//  Return the train
			return pTrain;
		}

		//  unbuttonFrame
		//
		//  This static function will unzip the content of a single frame from the map and return an RGB frame
		//
		//  PARAMETERS
		//
		//		Map&					-		Reference to the map of the in-memory image
		//		size_t					-		Block index of the first block of the frame
		//		GIF_ODI_COLOUR_TABLE*	-		Pointer to the Global Colour Table (GCT) if one exists
		//		size_t					-		Number of entries in the Global Colour Table (GCT)
		//
		//  RETURNS
		//
		//		Frame*			-		Pointer to the image Frame, NULL if it could not be unzipped
		//
		//  NOTES
		//

		static Frame<RGB>* unbuttonFrame(ODIMap& Map, size_t BlockNo, GIF_ODI_COLOUR_TABLE* pGCT, size_t GCTEnts) {
			Frame<RGB>*				pFrame = new Frame<RGB>();											//  Frame to be returned
			bool					GrenScreen = false;													//  Has a Transparent Colour
			size_t					GSCX = 0;															//  Green Screen (Transparent) colour index
			size_t					FHeight = 0;														//  Frame Height
			size_t					FWidth = 0;															//  Frame Width
			GIF_ODI_COLOUR_TABLE*	pCT = pGCT;															//  Pointer to the colour table to use
			size_t					CTEnts = GCTEnts;													//  Number of entries in the colour table

			//  If the initial block is a GCE then extract the frame display properties
			if (Map.Blocks[BlockNo].BlockType == GIF_BLOCK_GCE) {
				GIF_GRAPHICS_CONTROL_EXTENSION*		pGCE = (GIF_GRAPHICS_CONTROL_EXTENSION*) Map.Blocks[BlockNo].Block;
				pFrame->setDisposal(DisposalMethod(pGCE->bitsettings));
				pFrame->setDelay(GetSize(pGCE->delay));

				if (pGCE->bitsettings & GIF_GCEBITS_TRANSP) GrenScreen = true;
				GSCX = pGCE->transparent;
				BlockNo++;
			}

			//  We should now be positioned at the Image Descriptor - if not then bail out
			if (Map.Blocks[BlockNo].BlockType != GIF_BLOCK_ID) {
				delete pFrame;
				std::cerr << "ERROR: An unexpected block was encountered where an Image Descriptor (ID) was expected." << std::endl;
				return NULL;
			}

			//  Extract the information from the Image Descriptor
			GIF_IMAGE_DESCRIPTOR*	pID = (GIF_IMAGE_DESCRIPTOR*) Map.Blocks[BlockNo].Block;
			pFrame->setRRow(GetSize(pID->top));
			pFrame->setRCol(GetSize(pID->left));
			FHeight = GetSize(pID->height);
			FWidth = GetSize(pID->width);

			//  If the frame has a local colour table then use that
			if (pID->bitsettings & GIF_IDBITS_LCT) {
				CTEnts = ColourTableSize(pID->bitsettings);
				BlockNo++;
				if (Map.Blocks[BlockNo].BlockType != GIF_BLOCK_LCT) {
					delete pFrame;
					std::cerr << "ERROR: An unexpected block was encountered where an Local Colour Table (LCT) was expected." << std::endl;
					return nullptr;
				}
				pCT = (GIF_ODI_COLOUR_TABLE*) Map.Blocks[BlockNo].Block;
			}

			//  Having selected the colour table - capture the transparent colour in use
			if (GrenScreen) {
				if (GSCX >= CTEnts) GSCX = CTEnts - 1;
				pFrame->setTransparent(pCT->Entry[GSCX]);
			}

			//  Position to the Entropy Encoded Buffer (EEB)
			BlockNo++;
			if (Map.Blocks[BlockNo].BlockType != GIF_BLOCK_EEB) {
				delete pFrame;
				std::cerr << "ERROR: An unexpected block was encountered where an Entropy Encoded Buffer (EEB) was expected." << std::endl;
				return nullptr;
			}

			//  Build the Raster Buffer to use from the decoded content of the Entropy Encoded Buffer
			GIF_ENTROPY_ENCODED_BUFFER*		pEEB = (GIF_ENTROPY_ENCODED_BUFFER*) Map.Blocks[BlockNo].Block;
			RasterBuffer<RGB>*				pRB = nullptr;

			//  Decode an interlaced or non-interlaced image
			if (pID->bitsettings & GIF_IDBITS_ILC) pRB = decodeInterlacedImage(FHeight, FWidth, pEEB, Map.Blocks[BlockNo].BlockSize, pCT, CTEnts);
			else pRB = decodeImage(FHeight, FWidth, pEEB, Map.Blocks[BlockNo].BlockSize, pCT, CTEnts);

			if (pRB == nullptr) {
				delete pFrame;
				std::cerr << "ERROR: The Frame Image could NOT be decoded." << std::endl;
				return nullptr;
			}

			pFrame->setBuffer(pRB);

			//  Return the constructed frame
			return pFrame;
		}

		//  decodeImage
		//
		//  This static function will decode the contets of the passed Entropy Encoded Buffer (EEB) into a Raster Buffer
		//
		//  PARAMETERS
		//
		//		size_t							-		Image Rows
		//		size_t							-		Image Columns
		//		GIF_ENTROPY_ENCODED_BUFFER*		-		Pointer to the buffer to be decoded
		//		size_t							-		Size of the Entropy Encoded Buffer
		//		GIF_ODI_COLOUR_TABLE*			-		Pointer to the Colour Table to use
		//		size_t							-		Number of entries in the Colour Table
		//
		//  RETURNS
		//
		//		RasterBuffer*			-		Pointer to the Raster Buffer, NULL if one could not be built
		//
		//  NOTES
		//

		static RasterBuffer<RGB>* decodeImage(size_t IH, size_t IW, GIF_ENTROPY_ENCODED_BUFFER* pEEB, size_t EEBSize, GIF_ODI_COLOUR_TABLE* pCT, size_t CTEnts) {
			RasterBuffer<RGB>*		pRB = new RasterBuffer<RGB>(IH, IW, NULL);										//  Raster Buffer
			LZW						Decoder;																		//  LZW CODEC
			size_t					ImgSize = 0;																	//  Decoded Image Size

			//  Safety
			if (IH == 0 || IW == 0) {
				delete pRB;
				return nullptr;
			}

			if (EEBSize == 0) {
				delete pRB;
				return nullptr;
			}

			//  Form the input and output ByteStreams to use
			SegmentedStream		bsIn((BYTE *) &pEEB->segment, EEBSize - 1);

			//  The output bytestream is initialised with the number of pixels in the image, this prevents overrun
			ByteStream		bsOut(IH * IW);

			//  Decode the image
			if (!Decoder.decode(bsIn, bsOut, pEEB->nativecodesize)) {
				delete pRB;
				std::cerr << "ERROR: LZW CODEC failed to decode a GIF image contents." << std::endl;
				return nullptr;
			}

			//  Check the decoded image size
			ImgSize = bsOut.getBytesWritten();
			if (!Decoder.wasDecodeClean()) std::cerr << "WARNING: The GIF image decoder did NOT terminate as expected after reading: " << bsOut.getBytesWritten() << " pixels (" << Decoder.getTokenCount() << " tokens), image may be corrupt." << std::endl;
			if (ImgSize != (IH * IW)) {
				std::cerr << "WARNING: Decoded image size: " << ImgSize << " does not match expected size: " << IH * IW << " pixels." << std::endl;
				if (ImgSize > IH * IW) {
					std::cerr << "WARNING: " << ImgSize - (IH * IW) << " pixel overrun has been discarded." << std::endl;
					ImgSize = IH * IW;
				}
				else {
					std::cerr << "WARNING: " << (IH * IW) - ImgSize << " pixel underrun is missing from the image, after reading: " << bsIn.getBytesRead() << " bytes of segmented data." << std::endl;
				}
			}

			//  Obtain a linear iterator for the Raster Buffer
			RasterBuffer<RGB>::iterator WIt = pRB->begin();
			BYTE*  pBuffer = bsOut.getBufferAddress();

			//  Fill the image into the Raster Buffer
			for (size_t Pixel = 0; Pixel < ImgSize; Pixel++) {
				//  If the colour index is greater than the number of entries in the colour table use the first entry
				if (pBuffer[Pixel] > CTEnts) *WIt = pCT->Entry[0];
				else *WIt = pCT->Entry[pBuffer[Pixel]];
				WIt++;
			}

			//  Return the constructed Raster Buffer
			return pRB;
		}

		//  decodeInterlacedImage
		//
		//  This static function will decode the contets of the passed Entropy Encoded Buffer (EEB) into a Raster Buffer
		//  using the GIF interlaced image protocol.
		//
		//  PARAMETERS
		//
		//		size_t							-		Image Rows
		//		size_t							-		Image Columns
		//		GIF_ENTROPY_ENCODED_BUFFER*		-		Pointer to the buffer to be decoded
		//		size_t							-		Size of the Entropy Encoded Buffer
		//		GIF_ODI_COLOUR_TABLE*			-		Pointer to the Colour Table to use
		//		size_t							-		Number of entries in the Colour Table
		//
		//  RETURNS
		//
		//		RasterBuffer*			-		Pointer to the Raster Buffer, NULL if one could not be built
		//
		//  NOTES
		//

		static RasterBuffer<RGB>* decodeInterlacedImage(size_t IH, size_t IW, GIF_ENTROPY_ENCODED_BUFFER* pEEB, size_t EEBSize, GIF_ODI_COLOUR_TABLE* pCT, size_t CTEnts) {
			RasterBuffer<RGB>* pRB = new RasterBuffer<RGB>(IH, IW, nullptr);										//  Raster Buffer
			LZW						Decoder;																		//  LZW CODEC
			size_t					ImgSize = 0;																	//  Decoded Image Size

			//  Safety
			if (IH == 0 || IW == 0) {
				delete pRB;
				return nullptr;
			}

			if (EEBSize == 0) {
				delete pRB;
				return nullptr;
			}

			//  Form the input and output ByteStreams to use
			SegmentedStream		bsIn((BYTE*)&pEEB->segment, EEBSize - 1);

			//  The output bytestream is initialised with the number of pixels in the image, this prevents overrun
			ByteStream		bsOut(IH * IW);

			//  Decode the image
			if (!Decoder.decode(bsIn, bsOut, pEEB->nativecodesize)) {
				delete pRB;
				std::cerr << "ERROR: LZW CODEC failed to decode a GIF image contents." << std::endl;
				return nullptr;
			}

			//  Check the decoded image size
			ImgSize = bsOut.getBytesWritten();
			if (!Decoder.wasDecodeClean()) std::cerr << "WARNING: The GIF image decoder did NOT terminate as expected after reading: " << bsOut.getBytesWritten() << " pixels (" << Decoder.getTokenCount() << " tokens), image may be corrupt." << std::endl;
			if (ImgSize != (IH * IW)) {
				std::cerr << "WARNING: Decoded image size: " << ImgSize << " does not match expected size: " << IH * IW << " pixels." << std::endl;
				if (ImgSize > IH * IW) {
					std::cerr << "WARNING: " << ImgSize - (IH * IW) << " pixel overrun has been discarded." << std::endl;
					ImgSize = IH * IW;
				}
				else {
					std::cerr << "WARNING: " << (IH * IW) - ImgSize << " pixel underrun is missing from the image, after reading: " << bsIn.getBytesRead() << " bytes of segmented data." << std::endl;
				}
			}

			//  Obtain a linear iterator for the Raster Buffer
			BYTE*	pBuffer = bsOut.getBufferAddress();
			size_t	Pixel = 0;

			//  Perform a Row/Column iteration of the target raster buffer filling each 8th row
			for (RasterBuffer<RGB>::iterator RIt = pRB->top(); RIt != pRB->bottom(); RIt++) {
				if ((RIt.getIndex() % 8) == 0) {
					for (RasterBuffer<RGB>::iterator CIt = pRB->left(RIt); CIt != pRB->right(RIt); CIt++) {
						if (pBuffer[Pixel] > CTEnts) *CIt = pCT->Entry[0];
						else *CIt = pCT->Entry[pBuffer[Pixel]];
						Pixel++;
					}
				}
			}

			//  Perform a Row/Column iteration of the target raster buffer filling each 8th row starting from row 4
			for (RasterBuffer<RGB>::iterator RIt = pRB->top(); RIt != pRB->bottom(); RIt++) {
				if (((RIt.getIndex() - 4) % 8) == 0) {
					for (RasterBuffer<RGB>::iterator CIt = pRB->left(RIt); CIt != pRB->right(RIt); CIt++) {
						if (pBuffer[Pixel] > CTEnts) *CIt = pCT->Entry[0];
						else *CIt = pCT->Entry[pBuffer[Pixel]];
						Pixel++;
					}
				}
			}

			//  Perform a Row/Column iteration of the target raster buffer filling each 4th row starting from row 2
			for (RasterBuffer<RGB>::iterator RIt = pRB->top(); RIt != pRB->bottom(); RIt++) {
				if (((RIt.getIndex() - 2) % 4) == 0) {
					for (RasterBuffer<RGB>::iterator CIt = pRB->left(RIt); CIt != pRB->right(RIt); CIt++) {
						if (pBuffer[Pixel] > CTEnts) *CIt = pCT->Entry[0];
						else *CIt = pCT->Entry[pBuffer[Pixel]];
						Pixel++;
					}
				}
			}

			//  Perform a Row/Column iteration of the target raster buffer filling each 2th row starting from row 1
			for (RasterBuffer<RGB>::iterator RIt = pRB->top(); RIt != pRB->bottom(); RIt++) {
				if (((RIt.getIndex() - 1) % 2) == 0) {
					for (RasterBuffer<RGB>::iterator CIt = pRB->left(RIt); CIt != pRB->right(RIt); CIt++) {
						if (pBuffer[Pixel] > CTEnts) *CIt = pCT->Entry[0];
						else *CIt = pCT->Entry[pBuffer[Pixel]];
						Pixel++;
					}
				}
			}

			//  Return the constructed Raster Buffer
			return pRB;
		}

		//  mapImage
		//
		//  This static function will build the map of the on-disk image
		//
		//  PARAMETERS
		//
		//		ODIMap&			-		Reference to the ODIMap structure that will be filled
		//
		//  RETURNS
		//
		//		bool			-		true if the map could be built, otherwise false
		//
		//  NOTES
		//

		static bool mapImage(ODIMap& Map) {
			size_t			Offset = 0;														//  Offset in the in-memory image
			size_t			CTSize = 0;														//  Size of a Colour Table (bytes)
			bool			EndOfImage = false;												//  End of Image control
			BYTE*			pSOB = NULL;													//  Pointer to start of a block

			//  Initialise the block array
			Map.NBA = 256;
			Map.Blocks = (ODIBlock*) malloc(Map.NBA * sizeof(ODIBlock));
			if (Map.Blocks == NULL) {
				std::cerr << "ERROR: Unable to allocate memory for a GIF On-Disk-Image block map." << std::endl;
				return false;
			}
			memset(Map.Blocks, 0, 256 * sizeof(ODIBlock));

			//  Check that the image is large enough to contain the first block
			if (Map.ImageSize < sizeof(GIF_FILE_HEADER)) {
				std::cerr << "ERROR: The GIF Image is too small to hold a valid GIF File Header." << std::endl;
				free(Map.Blocks);
				Map.NBA = Map.NumBlocks = 0;
				return false;
			}

			//  Assign block 1 as the GIF File Header block
			Map.Blocks[Map.NumBlocks].BlockType = GIF_BLOCK_FH;
			Map.Blocks[Map.NumBlocks].Block = Map.Image;
			Map.Blocks[Map.NumBlocks].BlockSize = sizeof(GIF_FILE_HEADER);
			Map.NumBlocks = 1;

			//  Verify the header
			GIF_FILE_HEADER* pGFH = (GIF_FILE_HEADER*) Map.Blocks[0].Block;
			if (pGFH->signature[0] != GFH_SIG1 || pGFH->signature[1] != GFH_SIG2 || pGFH->signature[2] != GFH_SIG3) {
				std::cerr << "ERROR: GIF File Header does NOT contain a valid block signature." << std::endl;
				free(Map.Blocks);
				Map.NBA = Map.NumBlocks = 0;
				return false;
			}

			if (pGFH->version[0] != GFH_DEF_V1 || (pGFH->version[1] != GFH_DEF_V2 && pGFH->version[1] != GFH_DL_V2) || (pGFH->version[2] != ' ' && pGFH->version[2] != GFH_DEF_V3)) {
				std::cerr << "ERROR: GIF File Header contains an invalid image version number." << std::endl;
				free(Map.Blocks);
				Map.NBA = Map.NumBlocks = 0;
				return false;
			}

			//  Update the Offset
			Offset += Map.Blocks[Map.NumBlocks - 1].BlockSize;

			//  Check that the remaining image is large enough to hold an LSD structure
			if ((Offset + sizeof(GIF_LOGICAL_SCREEN_DESCRIPTOR)) > Map.ImageSize) {
				std::cerr << "ERROR: The GIF Image is too small to hold a valid Logical Screen Descriptor." << std::endl;
				free(Map.Blocks);
				Map.NBA = Map.NumBlocks = 0;
				return false;
			}

			//  Assign block 2 as the Logical Screen Descriptor (LSD)
			Map.Blocks[Map.NumBlocks].BlockType = GIF_BLOCK_LSD;
			Map.Blocks[Map.NumBlocks].BlockSize = sizeof(GIF_LOGICAL_SCREEN_DESCRIPTOR);
			Map.Blocks[Map.NumBlocks].Block = Map.Image + Offset;
			GIF_LOGICAL_SCREEN_DESCRIPTOR* pLSD = (GIF_LOGICAL_SCREEN_DESCRIPTOR*)Map.Blocks[Map.NumBlocks].Block;
			Map.NumBlocks++;

			//  Update the Offset
			Offset += Map.Blocks[Map.NumBlocks - 1].BlockSize;

			//  If the image has a Global Colour Table (GCT) then that appears next in the image
			if (pLSD->bitsettings & GIF_HDRBITS_GCT) {

				//  GCT is expected - determine the size of the table
				CTSize = ColourTableSizeBytes(pLSD->bitsettings);

				//  Check that the remaining image is large enough to hold the GCT of the specified size
				if ((Offset + CTSize) > Map.ImageSize) {
					std::cerr << "ERROR: The GIF Image is too small to hold a " << CTSize << " byte Global Colour Table." << std::endl;
					free(Map.Blocks);
					Map.NBA = Map.NumBlocks = 0;
					return false;
				}

				//  Assign block 3 as the Global Colour Table (GCT)
				Map.Blocks[Map.NumBlocks].BlockType = GIF_BLOCK_GCT;
				Map.Blocks[Map.NumBlocks].BlockSize = CTSize;
				Map.Blocks[Map.NumBlocks].Block = Map.Image + Offset;

				Map.NumBlocks++;

				//  Update the Offset
				Offset += Map.Blocks[Map.NumBlocks - 1].BlockSize;
			}

			//
			//  The remainder of the image will be a number of frames possibly interspersed with application extension blocks
			//  until the GIF file trailer is encountered.
			//	Process each block as encountered.
			//

			while (!EndOfImage) {

				//  Check that we have enough space in the map for at least 5 blocks - if not then expand the map
				if (Map.NumBlocks >= (Map.NBA - 5)) {
					Map.NBA += 256;
					ODIBlock*	pTemp = (ODIBlock*) realloc(Map.Blocks, Map.NBA * sizeof(ODIBlock));
					if (pTemp == nullptr) {
						std::cerr << "ERROR: Failed to reallocate the GIF Map to hold: " << Map.NBA << " blocks." << std::endl;
						free(Map.Blocks);
						Map.NBA = Map.NumBlocks = 0;
						return false;
					}
					else Map.Blocks = pTemp;
				}

				//  Make sure that we have image left to consume
				if (Offset >= Map.ImageSize) {
					std::cerr << "WARNING: The end of the GIF image was encountered before the File Trailer, file may be truncated." << std::endl;
					EndOfImage = true;
				}
				else {
					//  Test for a file trailer
					pSOB = Map.Image + Offset;
					if (*pSOB == GIF_SIG_END) {

						//  Capture the file trailer block and signal end of the image
						Map.Blocks[Map.NumBlocks].BlockType = GIF_BLOCK_FT;
						Map.Blocks[Map.NumBlocks].BlockSize = sizeof(GIF_FILE_TRAILER);
						Map.Blocks[Map.NumBlocks].Block = Map.Image + Offset;
						Map.NumBlocks++;
						EndOfImage = true;
					}
					else {

						//  Test for the start of an image frame.
						//  An image grame consists of an optional Graphics Control Extension (GCE), an Image Descriptor (ID) optionally a Local Colour Table (LCT) followed by the Image Data

						if (*pSOB == ID_SIG || (*pSOB == GCE_SIG_INT && *(pSOB + 1) == GCE_SIG_LAB)) {

							//  See if we have a Graphics Control Extension preceding the Image Descriptor
							if (*pSOB != ID_SIG) {
								
								//  Make sure that there is enough image for a GCE block
								if (Offset + sizeof(GIF_GRAPHICS_CONTROL_EXTENSION) > Map.ImageSize) {
									std::cerr << "ERROR: The GIF Image is too small to hold a valid Graphics Control Extension (GCE)." << std::endl;
									free(Map.Blocks);
									Map.NBA = Map.NumBlocks = 0;
									return false;
								}

								//  Capture the GCE block
								Map.Blocks[Map.NumBlocks].BlockType = GIF_BLOCK_GCE;
								Map.Blocks[Map.NumBlocks].BlockSize = sizeof(GIF_GRAPHICS_CONTROL_EXTENSION);
								Map.Blocks[Map.NumBlocks].Block = Map.Image + Offset;
								Map.NumBlocks++;

								//  Update the Offset
								Offset += Map.Blocks[Map.NumBlocks - 1].BlockSize;

							}

							//  Update the start of block pointer
							pSOB = Map.Image + Offset;

							//  Block MUST be an Image Descriptor
							if (*pSOB != ID_SIG) {
								std::cerr << "ERROR: The GIF Image contains a Frame that has NO Image Descriptor (ID)." << std::endl;
								free(Map.Blocks);
								Map.NBA = Map.NumBlocks = 0;
								return false;
							}

							//  Make sure that there is enough image for a GCE block
							if (Offset + sizeof(GIF_IMAGE_DESCRIPTOR) > Map.ImageSize) {
								std::cerr << "ERROR: The GIF Image is too small to hold a valid Image Descriptor (ID)." << std::endl;
								free(Map.Blocks);
								Map.NBA = Map.NumBlocks = 0;
								return false;
							}

							//  Capture the ID block
							Map.Blocks[Map.NumBlocks].BlockType = GIF_BLOCK_ID;
							Map.Blocks[Map.NumBlocks].BlockSize = sizeof(GIF_IMAGE_DESCRIPTOR);
							Map.Blocks[Map.NumBlocks].Block = Map.Image + Offset;
							GIF_IMAGE_DESCRIPTOR* pID = (GIF_IMAGE_DESCRIPTOR*) Map.Blocks[Map.NumBlocks].Block;
							Map.NumBlocks++;

							//  Update the Offset
							Offset += Map.Blocks[Map.NumBlocks - 1].BlockSize;

							//  See if the Frame has a Local Colour Table, if so capture it
							if (pID->bitsettings & GIF_IDBITS_LCT) {

								//  LCT is expected - determine the size of the table
								CTSize = LColourTableSizeBytes(pID->bitsettings);

								//  Check that the remaining image is large enough to hold the GCT of the specified size
								if ((Offset + CTSize) > Map.ImageSize) {
									std::cerr << "ERROR: The GIF Image is too small to hold a " << CTSize << " byte Local Colour Table." << std::endl;
									free(Map.Blocks);
									Map.NBA = Map.NumBlocks = 0;
									return false;
								}

								//  Assign next block as the Local Colour Table (LCT)
								Map.Blocks[Map.NumBlocks].BlockType = GIF_BLOCK_LCT;
								Map.Blocks[Map.NumBlocks].BlockSize = CTSize;
								Map.Blocks[Map.NumBlocks].Block = Map.Image + Offset;

								Map.NumBlocks++;

								//  Update the Offset
								Offset += Map.Blocks[Map.NumBlocks - 1].BlockSize;
							}

							//  The LCT (If present) is immediately followed by the Entropy Encoded Buffer
							//  First determine the size of the image.

							size_t		EEBSize = 2;

							GIF_ENTROPY_ENCODED_BUFFER* pEEB = (GIF_ENTROPY_ENCODED_BUFFER*) (Map.Image + Offset);
							GIF_IMAGE_SEGMENT* pSeg = &pEEB->segment;

							//  Loop accumulating the size of blobs (until there are no more)
							while (pSeg->size != EO_DATA_SIZE) {
								EEBSize += size_t(pSeg->size) + 1;
								pSeg = (GIF_IMAGE_SEGMENT*)((BYTE*)pSeg + pSeg->size + 1);

								//  Check that the EEB remains within the bounds of the image
								if ((Offset + EEBSize) > Map.ImageSize) {
									std::cerr << "ERROR: The GIF Image is too small to hold a " << EEBSize << " byte Entropy Encoded Buffer." << std::endl;
									free(Map.Blocks);
									Map.NBA = Map.NumBlocks = 0;
									return false;
								}
							}

							//  Assign next block as the Entropy Encoded Buffer (EEB)
							Map.Blocks[Map.NumBlocks].BlockType = GIF_BLOCK_EEB;
							Map.Blocks[Map.NumBlocks].BlockSize = EEBSize;
							Map.Blocks[Map.NumBlocks].Block = Map.Image + Offset;

							Map.NumBlocks++;

							//  Update the Offset
							Offset += Map.Blocks[Map.NumBlocks - 1].BlockSize;
						}
						else {
							//  Block (if valid) must be an application extension

							//  Update the start of block pointer
							pSOB = Map.Image + Offset;

							if (*pSOB != GIF_XBLK_SIG) {
								//  Unknown block type encountered - image is corrupt
								std::cerr << "ERROR: Invalid block encountered, Block: " << (Map.NumBlocks + 1) << ", at Offset: +" << Offset << "." << std::endl;
								std::cerr << "ERROR: Last Block mapped was Type: '" << Map.Blocks[Map.NumBlocks - 1].BlockType
									<< "', size: " << Map.Blocks[Map.NumBlocks - 1].BlockSize << "." << std::endl;

								//  Show the contents of the map
								size_t  CumOff = 0;
								for (size_t BX = 0; BX < Map.NumBlocks; BX++) {
									std::cerr << "BLOCK: " << (BX + 1) << ", at: +" << CumOff << ", Type: '" << Map.Blocks[BX].BlockType << "', Size: " << Map.Blocks[BX].BlockSize << "." << std::endl;
								}

								MemoryDumper::dumpMemory(pSOB, 256, "UNKNOWN BLOCK", std::cerr);

								free(Map.Blocks);
								Map.NBA = Map.NumBlocks = 0;
								return false;
							}

							//  Determine the size of the complete extension block
							size_t		AXBSize = 3;
							BYTE*		pDataBlock = pSOB + 2;

							while (*pDataBlock != 0) {
								AXBSize += size_t(*pDataBlock) + 1;
								pDataBlock += *pDataBlock + 1;

								//  Check that the AXB remains within the bounds of the image
								if ((Offset + AXBSize) > Map.ImageSize) {
									std::cerr << "ERROR: The GIF Image is too small to hold a " << AXBSize << " byte Application Extension Block." << std::endl;
									free(Map.Blocks);
									Map.NBA = Map.NumBlocks = 0;
									return false;
								}
							}

							//  Assign next block as the Application Extension Block
							Map.Blocks[Map.NumBlocks].BlockType = GIF_BLOCK_AXB;
							Map.Blocks[Map.NumBlocks].BlockSize = AXBSize;
							Map.Blocks[Map.NumBlocks].Block = Map.Image + Offset;

							Map.NumBlocks++;

							//  Update the Offset
							Offset += Map.Blocks[Map.NumBlocks - 1].BlockSize;
						}
					}
				}
			}

			//  Warn if the last block captured was NOT a GIF File Trailer
			if (Map.Blocks[Map.NumBlocks - 1].BlockType != GIF_BLOCK_FT) {
				std::cerr << "WARNING: The last block encountered in the GIF image was NOT a File Trailer, image may be truncated." << std::endl;
			}

			//  Check for a consistent Map:
			//
			//		A valid map may contains 0 frames ONLY IF it has a GCT
			//		Every GCE MUST have an ID and every ID must have an EEB
			//		An LCT Must follow an ID and precede an EEB
			//
			//  Evaluator States
			//
			//		1		-		GCE Detected (New Frame)
			//		2		-		ID Detected (New frame)
			//		3		-		LCT Detected 
			//		4		-		EEB Detected (End of Frame)
			//

			size_t		BlockNo = 2;
			int			EState = 4;															//  Expecting New start of frame
			int			NumFrames = 0;			

			//  Skip the GCT if present
			if (Map.Blocks[BlockNo].BlockType == GIF_BLOCK_GCT) BlockNo++;

			//  Skip any AXBs before the frame data blocks
			while (Map.Blocks[BlockNo].BlockType == GIF_BLOCK_AXB) BlockNo++;

			for (; BlockNo < Map.NumBlocks; BlockNo++) {
				switch (EState) {
				case 1:
					//  A GCE has been detected ONLY an ID is valid to follow
					if (Map.Blocks[BlockNo].BlockType != GIF_BLOCK_ID) {
						std::cerr << "ERROR: The GIF Image contains a Frame without an Image Descriptor (ID)." << std::endl;
						free(Map.Blocks);
						Map.NBA = Map.NumBlocks = 0;
						return false;
					}
					else EState = 2;
					break;

				case 2:
					//  An ID has been detected it MUST be followed by either an LCT or an EEB
					if (Map.Blocks[BlockNo].BlockType == GIF_BLOCK_EEB) EState = 4;
					else if (Map.Blocks[BlockNo].BlockType == GIF_BLOCK_LCT) EState = 3;
					else {
						std::cerr << "ERROR: The GIF Image contains a Frame without an Entropy Encoded Buffer (EEB)." << std::endl;
						free(Map.Blocks);
						Map.NBA = Map.NumBlocks = 0;
						return false;
					}
					break;

				case 3:
					//  An LCT has been detected it must be followed by an EEB
					if (Map.Blocks[BlockNo].BlockType == GIF_BLOCK_EEB) EState = 4;
					else {
						std::cerr << "ERROR: The GIF Image contains a Frame without an Entropy Encoded Buffer (EEB)." << std::endl;
						free(Map.Blocks);
						Map.NBA = Map.NumBlocks = 0;
						return false;
					}
					break;

				case 4:
					//  Outside a frame we can expect an AXB or a start of frame (GCE or ID)
					if (Map.Blocks[BlockNo].BlockType == GIF_BLOCK_GCE) EState = 1;
					else if (Map.Blocks[BlockNo].BlockType == GIF_BLOCK_ID) EState = 2;
					else {
						if (Map.Blocks[BlockNo].BlockType != GIF_BLOCK_AXB && Map.Blocks[BlockNo].BlockType != GIF_BLOCK_FT) {
							std::cerr << "ERROR: The GIF Image contains a Frame without a valid start GCE or ID." << std::endl;
							free(Map.Blocks);
							Map.NBA = Map.NumBlocks = 0;
							return false;
						}
					}
					if (EState == 1 || EState == 2) NumFrames++;
					break;
				}
			}

			//  Check the exiting state
			if (EState != 4) {
				std::cerr << "ERROR: The GIF Image contains an incomplete Frame." << std::endl;
				free(Map.Blocks);
				Map.NBA = Map.NumBlocks = 0;
				return false;
			}

			//  Check that an image with no frames has a GCT
			if (NumFrames == 0 && Map.Blocks[2].BlockType != GIF_BLOCK_GCT) {
				std::cerr << "ERROR: The GIF Image is empty (no frames) and it has NO Global Colour Table (GCT)." << std::endl;
				free(Map.Blocks);
				Map.NBA = Map.NumBlocks = 0;
				return false;
			}

			//  Return showing mapping success
			return true;
		}

		//  buildCanonicalTrain
		//
		//  This static function will build a Canonical Train (Wtiteable) from the passed Train
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
			Train<RGB>*			pCTrain = nullptr;												//  Pointer to the constructed canonical train
			Frame<RGB>*			pFF = pTrain->getFirstFrame();									//  Pointer to the first/only frame in the image
			int					ImageType = 0;													//  GIF Image Type

			//  Determine the type of GIF Image being constructed
			ImageType = categoriseTrain(pTrain);

			//  Construct the Canonical train according to the input train type
			switch (ImageType) {

			case GIF_DEGENERATE_IMAGE:
				//  DEPRECATED: The Degenerate image form is NOT supported by most CODECS
				std::cerr << "ERROR Attempted to store a degenrate GIF image, this is not supported." << std::endl;
				return nullptr;
				break;

			case GIF_PLAIN_IMAGE:
				//  Check for a valid frame in the image
				if (pFF->getBuffer() == nullptr) return nullptr;
				if (pFF->getHeight() == 0 || pFF->getWidth() == 0) return nullptr;

				//  Build the canonical train for the image
				pCTrain = buildTrainFromPlain(pTrain, Opts);
				break;

			case GIF_TILED_IMAGE:

				//  Check for a valid frame in the image
				if (pFF->getBuffer() == nullptr) return nullptr;
				if (pFF->getHeight() == 0 || pFF->getWidth() == 0) return nullptr;

				//  Build the canonical train for the image
				pCTrain = buildTrainFromPlain(pTrain, Opts);
				break;

			case GIF_ANIMATION:

				//  Check for a valid frame in the image
				if (pFF->getBuffer() == nullptr) return nullptr;
				if (pFF->getHeight() == 0 || pFF->getWidth() == 0) return nullptr;

				//  Build the canonical train for the image
				pCTrain = buildAnimatedTrain(pTrain, Opts);
				break;

			default:
				std::cerr << "ERROR: Train type: " << ImageType << " is NOT supported (yet)." << std::endl;
				break;
			}

			//  Return the constructed canonical train
			return pCTrain;
		}

		//  buildAnimatedTrain
		//
		//  This static function will build a Canonical Train (Writeable) from the passed Animated Image Train
		//
		//  PARAMETERS
		//
		//		Train*			-		Pointer to the input animated image train
		//		SWITCHES		-		Options to use for forming the Canonical Train
		//
		//  RETURNS
		//
		//		Train*			-		Pointer to the constructed Canonical Train
		//
		//  NOTES
		//

		static Train<RGB>* buildAnimatedTrain(Train<RGB>* pTrain, SWITCHES Opts) {
			Train<RGB>*			pCTrain = nullptr;												//  Pointer to the constructed canonical train
			RasterBuffer<RGB>*	pRefRB = nullptr;												//  Reference comparison image buffer for animation optimisation
			size_t				FrameNo = 0;													//  Frame number of optimisation
			Frame<RGB>*			pFrame = nullptr;												//  Animation frame

			//  Construct the base canonical train as a copy of the input train
			pCTrain = new Train<RGB>(*pTrain);

			//  If animation optimisation is disabled then move on to colour optimisation
			if (!(Opts & GIF_DISABLE_ANIOPT)) {

				//  Construct the reference image from the first set of tiles that form the first logical frame
				pFrame = pCTrain->getFirstFrame();
				pRefRB = new RasterBuffer<RGB>(pFrame->buffer());
				FrameNo = 2;

				//  Detect tiling of a logical frame
				while (pFrame->getDelay() == 0) {
					pFrame = pFrame->getNext();
					if (pFrame == nullptr) {
						delete pRefRB;
						//  Make sure that each frame is limited to 256 colours (including background and transparent)
						optimiseAnimationColourUsage(pCTrain, Opts);
						//  Return the canonical train
						return pCTrain;
					}

					FrameNo++;

					//  Update the reference buffer with any additional tiles
					pRefRB->blit(pFrame->buffer(), pFrame->getRRow(), pFrame->getRCol());
				}

				//  Optimise the following frames 
				optimiseAnimationFrames(pCTrain, FrameNo, *pRefRB, Opts);

				//  Delate the reference raster buffer
				delete pRefRB;
			}

			//  Make sure that each frame is limited to 256 colours (including background and transparent)
			optimiseAnimationColourUsage(pCTrain, Opts);

			//  Return the canonical train
			return pCTrain;
		}

		//  optimiseAnimationFrames
		//
		//  This static function will optimise the logical frame to frame transitions.
		//
		//  PARAMETERS
		//
		//		Train*					-	Pointer to the constructed Canonical Train
		//		size_t					-	Frame number of first tile to be optimised
		//		RasterBuffer<RGB>&		-	Reference to the reference image buffer i.e. canvas image after displaying the previous frame
		//		SWITCHES				-   GIF_ switch array
		//
		//  RETURNS
		//
		//  NOTES
		//

		static void		optimiseAnimationFrames(Train<RGB>* pTrain, size_t OptFNo, RasterBuffer<RGB>& RefImg, SWITCHES Opts) {
			Frame<RGB>*			pFrame = pTrain->getFirstFrame();												//  Frame being optimised
			size_t				FNo = 1;																		//  Frame number
			RasterBuffer<RGB>*	pRegionBfr = nullptr;															//  Region of Reference Image
			BoundingBox			bbReg = {};																		//  Bounding of image region
			BoundingBox			bbDiff = {};																	//  Bounding of difference region
			size_t				FPix = 0;																		//  Number of pixels in the frame
			size_t				DRPix = 0;																		//  Number of pixels in the defference region
			size_t				NMPct = 0;																		//  Non-matching region
			size_t				Expansion = 0;																	//  Expansion factor for alternate fragmenter

			//  If using the alternate fragmenter then set the allowed region expansion
			if (Opts & GIF_FRAGMENT_ALTERNATE) {
				if (Opts & GIF_FRAGMENT_TIGHT) Expansion = 1;
				else if (Opts & GIF_FRAGMENT_MEDIUM) Expansion = 3;
				else if (Opts & GIF_FRAGMENT_LOOSE) Expansion = 5;
			}

			//  Position to the frame to be optimised
			while (FNo < OptFNo) {
				pFrame = pFrame->getNext();
				FNo++;
			}

			//  If the frame is a region of the canvas image then create a temporary region of the Reference image
			if (pFrame->getRRow() != 0 || pFrame->getRCol() != 0 || pFrame->getHeight() != RefImg.getHeight() || pFrame->getWidth() != RefImg.getWidth()) {
				bbReg.Top = pFrame->getRRow();
				bbReg.Left = pFrame->getRCol();
				bbReg.Bottom = (bbReg.Top + pFrame->getHeight()) - 1;
				bbReg.Right = (bbReg.Left + pFrame->getWidth()) - 1;

				pRegionBfr = new RasterBuffer<RGB>(RefImg, bbReg);
			}
			else {
				bbReg.Top = 0;
				bbReg.Bottom = RefImg.getHeight() - 1;
				bbReg.Left = 0;
				bbReg.Right = RefImg.getWidth() - 1;
				pRegionBfr = &RefImg;
			}

			//  Perform a simple match to find the difference region
			if (pRegionBfr->matches(pFrame->buffer(), bbDiff)) {

				//  Delete the temporary buffer
				if (pRegionBfr != &RefImg) delete pRegionBfr;

				//  Get the next frame (if any)
				pFrame = pFrame->getNext();

				//  Boundary condition the frame (or tile) completely matches the Reference image and can therefore be removed
				pTrain->deleteFrame(FNo);

				//  If there are no more frames then return to caller
				if (pFrame == nullptr) return;

				//  Recurse - to process the frame now at the requested position
				return optimiseAnimationFrames(pTrain, OptFNo, RefImg, Opts);
			}

			//  Incomplete match - determine the percentage size of the non-matching region
			FPix = ((bbReg.Bottom - bbReg.Top) + 1) * ((bbReg.Right - bbReg.Left) + 1);
			DRPix = ((bbDiff.Bottom - bbDiff.Top) + 1) * ((bbDiff.Right - bbDiff.Left) + 1);
			NMPct = (DRPix * 100) / FPix;

			//  If the difference is less than the 20% threshold then save the Difference Region as a single tile
			if (NMPct < 20) {

				//  Delete the temporary buffer
				if (pRegionBfr != &RefImg) delete pRegionBfr;

				//  Create a new buffer from the difference region
				pRegionBfr = new RasterBuffer<RGB>(pFrame->buffer(), bbDiff);

				//  Delete the existing buffer in the frame
				delete pFrame->getBuffer();

				//  Place the new buffer in the frame
				pFrame->setBuffer(pRegionBfr);

				//  Adjust the origin
				pFrame->setRRow(pFrame->getRRow() + bbDiff.Top);
				pFrame->setRCol(pFrame->getRCol() + bbDiff.Left);

				//  Update the reference image
				RefImg.blit(pFrame->buffer(), pFrame->getRRow(), pFrame->getRCol());

				//  Get the next frame
				pFrame = pFrame->getNext();

				//  If there are no more frames then return to caller
				if (pFrame == nullptr) return;

				//  Recurse - to process the next frame
				return optimiseAnimationFrames(pTrain, OptFNo + 1, RefImg, Opts);
			}


			//  Build a mini-train of frames to replace the current frame
			Frame<RGB>* pRT = nullptr;
			if (Opts & GIF_FRAGMENT_ALTERNATE) pRT = fragmentFrame2(OptFNo, pFrame, pRegionBfr, bbDiff, Expansion);
			else pRT = fragmentFrame(OptFNo, pFrame, pRegionBfr, bbDiff);

			//  Delete the temporary buffer
			if (pRegionBfr != &RefImg) delete pRegionBfr;

			//  If no fragmentation was possible 
			if (pRT == nullptr) {

				//  Use this frame as-is
				//  Update the reference image
				RefImg.blit(pFrame->buffer(), pFrame->getRRow(), pFrame->getRCol());

				//  Get the next frame
				pFrame = pFrame->getNext();

				//  If there are no more frames then return to caller
				if (pFrame == nullptr) return;

				//  Recurse - to process the next frame
				return optimiseAnimationFrames(pTrain, OptFNo + 1, RefImg, Opts);
			}

			//
			//  Replace the current frame with the collection of frames (tiles) from the fragmentation process.
			//  The delay must be set in the last frame
			//

			Frame<RGB>*		pLastFrag = pRT;
			size_t			Frags = 1;
			while (pLastFrag->getNext() != nullptr) {
				Frags++;
				pLastFrag = pLastFrag->getNext();
			}

			pFrame->getPrev()->setNext(pRT);
			pRT->setPrev(pFrame->getPrev());
			if (pFrame->getNext() != nullptr) {
				pFrame->getNext()->setPrev(pLastFrag);
				pLastFrag->setNext(pFrame->getNext());
			}
			else pTrain->setCaboose(pLastFrag);
			pLastFrag->setDelay(pFrame->getDelay());
			pTrain->setNumFrames((pTrain->getNumFrames() - 1) + Frags);

			delete pFrame;

			//  Update the reference image
			pFrame = pRT;
			while (pFrame != nullptr) {
				RefImg.blit(pFrame->buffer(), pFrame->getRRow(), pFrame->getRCol());
				if (pFrame == pLastFrag) pFrame = nullptr;
				else pFrame = pFrame->getNext();
			}

			//  Position to the next frame
			pFrame = pLastFrag->getNext();

			//  If there are no more frames then return to caller
			if (pFrame == nullptr) return;

			//  Recurse - to process the next frame
			return optimiseAnimationFrames(pTrain, OptFNo + Frags, RefImg, Opts);
		}

		//  fragmentFrame
		//
		//  This static function will generate a mini-train of frame fragments holding the difference regions from the passed frame.
		//  It will scan the passed difference region to find Fragments (non-matching regions) and anti-fragments (matching regions).
		//  After scanning is complete it will combine the Fragments and anti-fragments to generate a mini-train of Frames that
		//  will express the non-matching regions while leaving the matching regions untouched.
		//
		//  PARAMETERS
		//
		//		size_t					-	Source Frame Number
		//		Frame*					-	Pointer to the frame to be fragmented
		//		RasterBuffer<RGB>*		-	Pointer to the raster buffer holding the reference comparison
		//		BoundingBox&			-	Reference to the bounding box for the difference region
		//
		//  RETURNS	
		// 
		//		Frame*					-	Pointer to the first frame of the fragments mini-train, nullptr if a mini-train could not be constructed
		//
		//  NOTES
		//

		static Frame<RGB>* fragmentFrame(size_t SFNo, Frame<RGB>* pFrame, RasterBuffer<RGB>* pRefImg, BoundingBox& bbDiff) {
			Frame<RGB>*			pFrags = nullptr;														//  mini-train of fragments
			Frame<RGB>*			pLastFrag = nullptr;													//  Last fragment

			//  Fragments array
			BoundingBox			Fragment[10] = {};														//  Fragment bounds
			size_t				FPIx[10] = {};															//  Pixels (non-matching) in each fragment
			size_t				FragsInPlay = 0;														//  Fragments active

			//  Anti-Fragments array
			BoundingBox			AntiFragment[10] = {};													//  Anti-Fragment bounds

			//  Final Fragment array
			BoundingBox			FFrag[40] = {};															//  Final fragment bounds
			size_t				FFrags = 0;																//  Final fragment count

			size_t				MPR = 0, MPC = 0;														//  Pixel Row & Column
			int					DR = 0, DC = 0;															//  Distance from existing fragment
			size_t				BDFX = 0;																//  Best distance fragment
			int					BDR = 0, BDC = 0;														//  Best distance
			bool				PixelDone = false;														//  Pixel has been consumed
			bool				VO = false, HO = false, VA = false, HA = false;							//  Overlap and adjacency controls
			bool				CollapseFragments = false;												//  Fragment collapse control
			size_t				CFX = 0;																//  Index of fragment to be collapsed

			SFNo = SFNo;

			//  Reject frames that are less than 2,000 pixels in size (too small to fragment)
			if ((pFrame->getHeight() * pFrame->getWidth()) <= 2000) return nullptr;

			//  Make a pass over the difference region assigning pixels to fragments
			for (RasterBuffer<RGB>::iterator RIt = pFrame->buffer().top(bbDiff); RIt != pFrame->buffer().bottom(bbDiff); RIt++) {
				for (RasterBuffer<RGB>::iterator CIt = pFrame->buffer().left(RIt); CIt != pFrame->buffer().right(RIt); CIt++) {
					//  Check against the reference image for a mismatched pixel
					MPR = RIt.getIndex();
					MPC = CIt.getIndex();
					if (*CIt != (*pRefImg)(MPR, MPC)) {
						//  Search the existing fragments to see if the new pixel belongs to an existing fragment
						PixelDone = false;
						for (size_t FX = 0; FX < FragsInPlay; FX++) {
							if (MPR >= Fragment[FX].Top && MPR <= Fragment[FX].Bottom && MPC >= Fragment[FX].Left && MPC <= Fragment[FX].Right) {
								PixelDone = true;
								FPIx[FX]++;
								break;
							}
						}

						//  If the pixel was not consumed by an existing fragment then it will either expand an existing fragment
						//  or seed a new fragment

						if (!PixelDone) {

							//  Boundary condition - there are no existing fragments
							if (FragsInPlay == 0) {
								Fragment[0].Top = MPR;
								Fragment[0].Bottom = MPR;
								Fragment[0].Left = MPC;
								Fragment[0].Right = MPC;
								FragsInPlay++;
								FPIx[0]++;
							}
							else {
								//  Test for expansion of an existing fragment
								BDFX = 10;
								for (size_t FX = 0; FX < FragsInPlay; FX++) {

									//  Compute the row distance
									if (MPR >= Fragment[FX].Top && MPR <= Fragment[FX].Bottom) DR = 0;
									else if (MPR < Fragment[FX].Top) DR = int(MPR) - int(Fragment[FX].Top);
									else DR = int(MPR) - int(Fragment[FX].Bottom);

									//  Compute the column distance
									if (MPC >= Fragment[FX].Left && MPC <= Fragment[FX].Right) DC = 0;
									else if (MPC < Fragment[FX].Left) DC = int(MPC) - int(Fragment[FX].Left);
									else DC = int(MPC) - int(Fragment[FX].Right);

									//  Determine if this fragment is the best match
									if (BDFX == 10 || (((DR * DR) + (DC * DC))) < ((BDR * BDR) + (BDC * BDC))) {
										BDFX = FX;
										BDR = DR;
										BDC = DC;
									}
								}

								//  Determine if the closest fragment is close enough to cause an expansion
								if ((abs(BDR) <= 5 && abs(BDC) <= 5) || FragsInPlay == 10) {
									if (BDR < 0) Fragment[BDFX].Top += BDR;
									else Fragment[BDFX].Bottom += BDR;
									if (BDC < 0) Fragment[BDFX].Left += BDC;
									else Fragment[BDFX].Right += BDC;
									FPIx[BDFX]++;
									PixelDone = true;
								}

								//  If the pixel was not consumed then use it to initialise a new fragment
								if (!PixelDone) {
									Fragment[FragsInPlay].Top = MPR;
									Fragment[FragsInPlay].Bottom = MPR;
									Fragment[FragsInPlay].Left = MPC;
									Fragment[FragsInPlay].Right = MPC;
									FPIx[FragsInPlay]++;
									FragsInPlay++;								
								}
							}
						}
					}
				}

				//  After every complete row perform an optimisation of the fragments and anti-fragments identified to collapse
				//  adjacent and occluding fragments.
				//  Only a single fragment collapse is processed on each row.

				for (size_t FX = 0; FX < FragsInPlay; FX++) {
					CollapseFragments = false;

					//  Check if any of the other fragments should collapse into this fragment
					for (size_t AFX = 0; AFX < FragsInPlay; AFX++) {
						if (AFX != FX) {
							VO = HO = VA = HA = false;
							CollapseFragments = false;

							if ((Fragment[AFX].Top >= Fragment[FX].Top && Fragment[AFX].Top <= Fragment[FX].Bottom)
								|| (Fragment[AFX].Bottom <= Fragment[FX].Bottom && Fragment[AFX].Bottom >= Fragment[FX].Top)) VO = true;
							if ((Fragment[AFX].Left >= Fragment[FX].Left && Fragment[AFX].Left <= Fragment[FX].Right)
								|| (Fragment[AFX].Right <= Fragment[FX].Right && Fragment[AFX].Right >= Fragment[FX].Left)) HO = true;

							if ((Fragment[AFX].Top == (Fragment[FX].Bottom + 1)) || (Fragment[AFX].Bottom == (Fragment[FX].Top - 1))) VA = true;
							if ((Fragment[AFX].Left == (Fragment[FX].Right + 1)) || (Fragment[AFX].Right == (Fragment[FX].Left - 1))) HA = true;

							//  If the fragments are horizontally and vertically overlapping or adjacent then signal to collapse the fragments
							if ((VO || VA) && (HO || HA)) CollapseFragments = true;

							//  Determine if the fragments have been selected for collapse
							if (CollapseFragments) {
								CFX = AFX;
								break;
							}
						}
					}

					//  Collapse the selected fragment - if one is signalled for collapse
					if (CollapseFragments) {

						//  Adjust the fragment bounds
						if (Fragment[CFX].Top < Fragment[FX].Top) Fragment[FX].Top = Fragment[CFX].Top;
						if (Fragment[CFX].Bottom > Fragment[FX].Bottom) Fragment[FX].Bottom = Fragment[CFX].Bottom;
						if (Fragment[CFX].Left < Fragment[FX].Left) Fragment[FX].Left = Fragment[CFX].Left;
						if (Fragment[CFX].Right > Fragment[FX].Right) Fragment[FX].Right = Fragment[CFX].Right;

						FPIx[FX] += FPIx[CFX];

						//  Shuffle up the fragments following the collapsed fragment
						for (size_t AFX = CFX + 1; AFX < FragsInPlay; AFX++) {
							Fragment[AFX - 1].Top = Fragment[AFX].Top;
							Fragment[AFX - 1].Bottom = Fragment[AFX].Bottom;
							Fragment[AFX - 1].Left = Fragment[AFX].Left;
							Fragment[AFX - 1].Right = Fragment[AFX].Right;

							FPIx[AFX - 1] = FPIx[AFX];
						}

						FragsInPlay--;
						break;
					}
				}
			}

			//  Scan each fragment identified (if large enough) to identify if it has a "polomint" construction

			for (size_t FX = 0; FX < FragsInPlay; FX++) {

				//  Determine if the current fragment is large enough to consider
				size_t		FSize = ((Fragment[FX].Bottom - Fragment[FX].Top) + 1) * ((Fragment[FX].Right - Fragment[FX].Left) + 1);
				if (FSize > 2000) {
					//  Fragment has at least 2,000 pixels
					if (FPIx[FX] <= (FSize / 2)) {
						//  Fragment has at most 50% occupancy (non-matched pixels)
						//  This is a "polomint" candidate
						AntiFragment[FX] = polomint(pFrame, Fragment[FX], *pRefImg);

						//  Determine if the anti-fragment should be dropped as it is too small
						if (AntiFragment[FX].Bottom > 0) {
							size_t	AFSize = ((AntiFragment[FX].Bottom - AntiFragment[FX].Top) + 1) * ((AntiFragment[FX].Right - AntiFragment[FX].Left) + 1);
							if (AFSize < 200) {
								AntiFragment[FX].Top = 0;
								AntiFragment[FX].Left = 0;
								AntiFragment[FX].Bottom = 0;
								AntiFragment[FX].Left = 0;
							}
						}
					}
				}
			}

			//  Copy the fragments to the Final Fragments array splitting any with an anti-fragment around the anti-fragment bounds
			for (size_t FX = 0; FX < FragsInPlay; FX++) {
				if (AntiFragment[FX].Bottom > 0) {

					//  Split the fragment around it's anti-fragment
					int  Split = 0;
					if (AntiFragment[FX].Top == Fragment[FX].Top) Split += 1;
					if (AntiFragment[FX].Bottom == Fragment[FX].Bottom) Split += 2;
					if (AntiFragment[FX].Left == Fragment[FX].Left) Split += 4;
					if (AntiFragment[FX].Right == Fragment[FX].Right) Split += 8;

					//  Switch according to the relation of the anti-fragemnt to the fragment
					//  classical polomint has wide top and bottom fragments, alt-classical has deep left and right fragments
					switch (Split) {
					case 0:
						//  Pure polomint split - the anti-fragment does NOT intersect with any of the fragments edges.
						//  This will cause a split into 4 fragments
						FFrag[FFrags].Top = Fragment[FX].Top;
						FFrag[FFrags].Bottom = AntiFragment[FX].Top - 1;
						FFrag[FFrags].Left = Fragment[FX].Left;
						FFrag[FFrags].Right = Fragment[FX].Right;
						FFrags++;

						FFrag[FFrags].Top = AntiFragment[FX].Top;
						FFrag[FFrags].Bottom = AntiFragment[FX].Bottom;
						FFrag[FFrags].Left = Fragment[FX].Left;
						FFrag[FFrags].Right = AntiFragment[FX].Left - 1;
						FFrags++;

						FFrag[FFrags].Top = AntiFragment[FX].Top;
						FFrag[FFrags].Bottom = AntiFragment[FX].Bottom;
						FFrag[FFrags].Left = AntiFragment[FX].Right + 1;
						FFrag[FFrags].Right = Fragment[FX].Right;
						FFrags++;

						FFrag[FFrags].Top = AntiFragment[FX].Bottom + 1;
						FFrag[FFrags].Bottom = Fragment[FX].Bottom;
						FFrag[FFrags].Left = Fragment[FX].Left;
						FFrag[FFrags].Right = Fragment[FX].Right;
						FFrags++;
						break;

					case 1:
						//  The antifragment intersects with the top edge of the fragemnt
						//  This is an alt-classical polomint without the top fragment
						FFrag[FFrags].Top = Fragment[FX].Top;
						FFrag[FFrags].Bottom = Fragment[FX].Bottom;
						FFrag[FFrags].Left = Fragment[FX].Left;
						FFrag[FFrags].Right = AntiFragment[FX].Left - 1;
						FFrags++;

						FFrag[FFrags].Top = Fragment[FX].Top;
						FFrag[FFrags].Bottom = Fragment[FX].Bottom;
						FFrag[FFrags].Left = AntiFragment[FX].Right + 1;
						FFrag[FFrags].Right = Fragment[FX].Right;
						FFrags++;

						FFrag[FFrags].Top = AntiFragment[FX].Bottom + 1;
						FFrag[FFrags].Bottom = Fragment[FX].Bottom;
						FFrag[FFrags].Left = AntiFragment[FX].Left - 1;
						FFrag[FFrags].Right = AntiFragment[FX].Right + 1;
						FFrags++;
						break;

					case 2:
						//  The anti-fragment intersects with the bottom edge of the fragment
						//  This is an alt-classical polomint without the bottom fragment
						FFrag[FFrags].Top = Fragment[FX].Top;
						FFrag[FFrags].Bottom = AntiFragment[FX].Top - 1;
						FFrag[FFrags].Left = AntiFragment[FX].Left - 1;
						FFrag[FFrags].Right = AntiFragment[FX].Right + 1;
						FFrags++;

						FFrag[FFrags].Top = Fragment[FX].Top;
						FFrag[FFrags].Bottom = Fragment[FX].Bottom;
						FFrag[FFrags].Left = Fragment[FX].Left;
						FFrag[FFrags].Right = AntiFragment[FX].Left - 1;
						FFrags++;

						FFrag[FFrags].Top = Fragment[FX].Top;
						FFrag[FFrags].Bottom = Fragment[FX].Bottom;
						FFrag[FFrags].Left = AntiFragment[FX].Right + 1;
						FFrag[FFrags].Right = Fragment[FX].Right;
						FFrags++;
						break;

					case 3:
						//  The anti-fragment intersects with the top and bottom edges of the fragment
						//  This is an alt-classical polomint without the top or bottom fragments
						FFrag[FFrags].Top = Fragment[FX].Top;
						FFrag[FFrags].Bottom = Fragment[FX].Bottom;
						FFrag[FFrags].Left = Fragment[FX].Left;
						FFrag[FFrags].Right = AntiFragment[FX].Left - 1;
						FFrags++;

						FFrag[FFrags].Top = Fragment[FX].Top;
						FFrag[FFrags].Bottom = Fragment[FX].Bottom;
						FFrag[FFrags].Left = AntiFragment[FX].Right + 1;
						FFrag[FFrags].Right = Fragment[FX].Right;
						FFrags++;
						break;

					case 4:
						//  The anti-fragment intersects with the left edge of the fragment
						//  This is a classical polomint without the left fragment
						FFrag[FFrags].Top = Fragment[FX].Top;
						FFrag[FFrags].Bottom = AntiFragment[FX].Top - 1;
						FFrag[FFrags].Left = Fragment[FX].Left;
						FFrag[FFrags].Right = Fragment[FX].Right;
						FFrags++;

						FFrag[FFrags].Top = AntiFragment[FX].Top;
						FFrag[FFrags].Bottom = AntiFragment[FX].Bottom;
						FFrag[FFrags].Left = AntiFragment[FX].Right + 1;
						FFrag[FFrags].Right = Fragment[FX].Right;
						FFrags++;

						FFrag[FFrags].Top = AntiFragment[FX].Bottom + 1;
						FFrag[FFrags].Bottom = Fragment[FX].Bottom;
						FFrag[FFrags].Left = Fragment[FX].Left;
						FFrag[FFrags].Right = Fragment[FX].Right;
						FFrags++;
						break;

					case 5:
						//  The anti-fragment intersects with the top and left of the fragment
						//  This is an alt-classical polomint without the top and left
						FFrag[FFrags].Top = Fragment[FX].Top;
						FFrag[FFrags].Bottom = Fragment[FX].Bottom;
						FFrag[FFrags].Left = AntiFragment[FX].Right + 1;
						FFrag[FFrags].Right = Fragment[FX].Right;
						FFrags++;

						FFrag[FFrags].Top = AntiFragment[FX].Bottom + 1;
						FFrag[FFrags].Bottom = Fragment[FX].Bottom;
						FFrag[FFrags].Left = AntiFragment[FX].Left - 1;
						FFrag[FFrags].Right = AntiFragment[FX].Right + 1;
						FFrags++;
						break;

					case 6:
						//  The anti-fragment intersects with the bottom and left of the fragment
						//  This is an alt-classical polomint without the bottom and left
						FFrag[FFrags].Top = Fragment[FX].Top;
						FFrag[FFrags].Bottom = AntiFragment[FX].Top - 1;
						FFrag[FFrags].Left = AntiFragment[FX].Left - 1;
						FFrag[FFrags].Right = AntiFragment[FX].Right + 1;
						FFrags++;

						FFrag[FFrags].Top = Fragment[FX].Top;
						FFrag[FFrags].Bottom = Fragment[FX].Bottom;
						FFrag[FFrags].Left = AntiFragment[FX].Right + 1;
						FFrag[FFrags].Right = Fragment[FX].Right;
						FFrags++;
						break;

					case 8:
						//  The anti-fragment intersects with the right edge of the fragment
						//  This is a classical polomint without the right fragment
						FFrag[FFrags].Top = Fragment[FX].Top;
						FFrag[FFrags].Bottom = AntiFragment[FX].Top - 1;
						FFrag[FFrags].Left = Fragment[FX].Left;
						FFrag[FFrags].Right = Fragment[FX].Right;
						FFrags++;

						FFrag[FFrags].Top = AntiFragment[FX].Top;
						FFrag[FFrags].Bottom = AntiFragment[FX].Bottom;
						FFrag[FFrags].Left = Fragment[FX].Left;
						FFrag[FFrags].Right = AntiFragment[FX].Left - 1;
						FFrags++;

						FFrag[FFrags].Top = AntiFragment[FX].Bottom + 1;
						FFrag[FFrags].Bottom = Fragment[FX].Bottom;
						FFrag[FFrags].Left = Fragment[FX].Left;
						FFrag[FFrags].Right = Fragment[FX].Right;
						FFrags++;
						break;

					case 9:
						//  The anti-fragment intersects with the top and right edge of the fragment
						//  This is an alt-classical polomint without the top and right fragments
						FFrag[FFrags].Top = Fragment[FX].Top;
						FFrag[FFrags].Bottom = Fragment[FX].Bottom;
						FFrag[FFrags].Left = Fragment[FX].Left;
						FFrag[FFrags].Right = AntiFragment[FX].Left - 1;
						FFrags++;

						FFrag[FFrags].Top = AntiFragment[FX].Bottom + 1;
						FFrag[FFrags].Bottom = Fragment[FX].Bottom;
						FFrag[FFrags].Left = AntiFragment[FX].Left - 1;
						FFrag[FFrags].Right = AntiFragment[FX].Right + 1;
						FFrags++;
						break;

					case 10:
						//  The anti-fragment intersects with the bottom and right edge of the fragment
						//  This is an alt-classical polomint without the bottom and right fragments
						FFrag[FFrags].Top = Fragment[FX].Top;
						FFrag[FFrags].Bottom = AntiFragment[FX].Top - 1;
						FFrag[FFrags].Left = AntiFragment[FX].Left - 1;
						FFrag[FFrags].Right = AntiFragment[FX].Right + 1;
						FFrags++;

						FFrag[FFrags].Top = Fragment[FX].Top;
						FFrag[FFrags].Bottom = Fragment[FX].Bottom;
						FFrag[FFrags].Left = Fragment[FX].Left;
						FFrag[FFrags].Right = AntiFragment[FX].Left - 1;
						FFrags++;
						break;

					default:
						//  SNO
						std::cerr << "ERROR: Fragment/anti-fragment configuration: " << Split << " detected, this should not occur." << std::endl;
						std::cerr << "ERROR: Fragment: T: " << Fragment[FX].Top << ", L: " << Fragment[FX].Left << ", B: " << Fragment[FX].Bottom << ", R: " << Fragment[FX].Right << "." << std::endl;
						std::cerr << "ERROR: Anti-Fragment: T: " << AntiFragment[FX].Top << ", L: " << AntiFragment[FX].Left << ", B: " << AntiFragment[FX].Bottom << ", R: " << AntiFragment[FX].Right << "." << std::endl;
						//  Copy the fragment over
						FFrag[FFrags++] = Fragment[FX];
						break;
					}
				}
				else FFrag[FFrags++] = Fragment[FX];
			}

			//  Build a mini-train of tiles from each of the final fragments discovered
			for (size_t FX = 0; FX < FFrags; FX++) {

				//  Convert the fragment bounding box from absolute to relative (to the source frame) co-ordinates
				BoundingBox bbRel = {};
				bbRel.Top = FFrag[FX].Top - pFrame->getRRow();
				bbRel.Left = FFrag[FX].Left - pFrame->getRCol();
				bbRel.Bottom = FFrag[FX].Bottom - pFrame->getRRow();
				bbRel.Right = FFrag[FX].Right - pFrame->getRCol();

				//  Construct a new Raster Buffer from the fragment
				RasterBuffer<RGB>* pNewBfr = new RasterBuffer<RGB>(pFrame->buffer(), bbRel);
				//  Construct a new frame from the Raster Buffer
				Frame<RGB>* pNewFrame = new Frame<RGB>(pNewBfr, FFrag[FX].Top, FFrag[FX].Left, pLastFrag);

				//  Update the pointers
				if (pFrags == nullptr) pFrags = pLastFrag = pNewFrame;
				else pLastFrag = pNewFrame;
			}

			//  Return the mini-train (if any)
			return pFrags;
		}

		//  polomint
		//
		//  This static function will return a bounding box describing a matching region at the centre of a fragment that
		//  matches the given region of a reference buffer.
		//
		//  PARAMETERS
		//
		//		Frame*			-		Pointer to the input frame containing the fragment
		//		BoundingBox&	-		Reference to the fragment definition
		//		RasterBuffer&	-		Reference to the reference (comparison) Raster Buffer
		//
		//  RETURNS
		//
		//		BoundingBox		-		Region definition of the matching "polomint" region of the fragment
		//
		//  NOTES
		//

		static BoundingBox		polomint(Frame<RGB>* pFrame, BoundingBox& Frag, RasterBuffer<RGB>& Ref) {
			BoundingBox			PMReg = {};													//  Polomint region
			bool				XT = true, XB = true, XL = true, XR = true;					//  Expansion permits

			//  Set the mid-point to start
			PMReg.Top = Frag.Top + (((Frag.Bottom - Frag.Top) + 1) / 2);
			PMReg.Bottom = PMReg.Top;
			PMReg.Left = Frag.Left + (((Frag.Right - Frag.Left) + 1) / 2);
			PMReg.Right = PMReg.Left;

			if (pFrame->buffer()(PMReg.Top, PMReg.Left) != Ref(PMReg.Top, PMReg.Left)) {
				//  If the midpoint does not match then abandon the search
				PMReg.Bottom = PMReg.Top = 0;
				PMReg.Right = PMReg.Left = 0;
				return PMReg;
			}

			//  Progressively expand the seed polomint region until it cannot be expanded further
			while (XT || XB || XL || XR) {

				//  Expand the polomint region towards the top of the fragment
				if (XT) {
					//  Check for limit of the expansion
					if (PMReg.Top == Frag.Top) XT = false;
					else {
						//  To expand upwards all pixels in the row above the current region MUST match
						for (size_t Col = PMReg.Left; Col <= PMReg.Right; Col++) {
							if (pFrame->buffer()(PMReg.Top - 1, Col) != Ref(PMReg.Top - 1, Col)) XT = false;
						}

						//  If expansion is still permitted update the bounds
						if (XT) PMReg.Top--;
					}
				}

				//  Expand the polomint region towards the bottom of the fragment
				if (XB) {
					//  Check for limit of the expansion
					if (PMReg.Bottom == Frag.Bottom) XB = false;
					else {
						//  To expand downwards all pixels in the row bulow the current region MUST match
						for (size_t Col = PMReg.Left; Col <= PMReg.Right; Col++) {
							if (pFrame->buffer()(PMReg.Bottom + 1, Col) != Ref(PMReg.Bottom + 1, Col)) XB = false;
						}

						//  If expansion is still permitted update the bounds
						if (XB) PMReg.Bottom++;
					}
				}

				//  Expand the polomint region towards the left of the fragment
				if (XL) {
					//  Check for limit of the expansion
					if (PMReg.Left == Frag.Left) XL = false;
					else {
						//  To expand leftwards all pixels in the column to the left the current region MUST match
						for (size_t Row = PMReg.Top; Row <= PMReg.Bottom; Row++) {
							if (pFrame->buffer()(Row, PMReg.Left - 1) != Ref(Row, PMReg.Left - 1)) XL = false;
						}

						//  If expansion is still permitted update the bounds
						if (XL) PMReg.Left--;
					}
				}

				//  Expand the polomint region towards the right of the fragment
				if (XR) {
					//  Check for limit of the expansion
					if (PMReg.Right == Frag.Right) XR = false;
					else {
						//  To expand rightwards all pixels in the column to the right the current region MUST match
						for (size_t Row = PMReg.Top; Row <= PMReg.Bottom; Row++) {
							if (pFrame->buffer()(Row, PMReg.Right + 1) != Ref(Row, PMReg.Right + 1)) XR = false;
						}

						//  If expansion is still permitted update the bounds
						if (XR) PMReg.Right++;
					}
				}
			}

			//  Return the discovered polomint region
			return PMReg;
		}

		//  fragmentFrame2
		//
		//  This function will fragment the passed frame using the Difference Map method
		//
		//	PARAMETERS:
		// 
		//		size_t				-		Frame number
		//		Frame*				-		Pointer to the test image frame to be fragmented
		//		RasterBuffer*		-		Pointer to the reference image raster buffer
		//		BoundingBox&		-		Reference to the bounding box for the difference region
		//		size_t				-		Expansion permitted during fragmentation
		//
		//	RETURNS:
		// 
		//		Frame*					-	Pointer to the first frame of the fragments mini-train, nullptr if a mini-train could not be constructed
		//
		//	NOTES:
		//

		static Frame<RGB>* fragmentFrame2(size_t SFNo, Frame<RGB>* pFrame, RasterBuffer<RGB>* pRefImg, BoundingBox& bbDiff, size_t Expansion) {
			Frame<RGB>*					pFrags = nullptr;														//  mini-train of fragments
			Frame<RGB>*					pLastFrag = nullptr;													//  Last fragment
			RasterBuffer<BYTE>*			pDiffMap = nullptr;														//  Difference Map
			size_t						DiffCount = 0;															//  Difference Count
			BYTE						RegID = 0x02;															//  region ID
			BYTE						Matched = 0x00;															//  Pixel Match
			BYTE						Mismatched = 0x01;														//  Pixel Mismatch
			size_t						RegDetect = 0;															//  Regions detected
			ColourTable<BYTE>			MapCT;																	//  Difference map colour table

			SFNo = SFNo;
			bbDiff = bbDiff;

			//  Convert the expansion into a limit
			Expansion++;

			//  Reject frames that are less than 2,000 pixels in size (too small to fragment)
			if ((pFrame->getHeight() * pFrame->getWidth()) <= 2000) return nullptr;

			//  Generate the Difference Map
			pDiffMap = pRefImg->mapDifference(pFrame->buffer(), DiffCount);
			if (pDiffMap == nullptr) return nullptr;

			//
			//  Perform a scan of the Difference Map creating regions
			//
			for (RasterBuffer<BYTE>::iterator MRit = pDiffMap->top(); MRit != pDiffMap->bottom(); MRit++) {
				for (RasterBuffer<BYTE>::iterator MCit = pDiffMap->left(MRit); MCit != pDiffMap->right(); MCit++) {
					if (*MCit == Mismatched) {
						//  Flood fill the map replacing MisMatched with the latest region ID
						pDiffMap->flood(MRit.getIndex(), MCit.getIndex(), RegID);
						RegID++;
						RegDetect++;
					}

					//  Add the entry to the colour table
					if (*MCit != Matched) MapCT.add(*MCit, 1, MRit.getIndex(), MCit.getIndex());
				}
			}

			//
			//  Collapse the region map
			//

			collapseMap(MapCT, Expansion);

			//  Build a mini-train of tiles from each of the final fragments discovered
			for (size_t FX = 0; FX < MapCT.getNumColours(); FX++) {

				//  Convert the fragment bounding box from absolute to relative (to the source frame) co-ordinates
				BoundingBox bbRel = {};
				bbRel.Top = MapCT.getExtents(FX).Top - pFrame->getRRow();
				bbRel.Left = MapCT.getExtents(FX).Left - pFrame->getRCol();
				bbRel.Bottom = MapCT.getExtents(FX).Bottom - pFrame->getRRow();
				bbRel.Right = MapCT.getExtents(FX).Right - pFrame->getRCol();

				//  Construct a new Raster Buffer from the fragment
				RasterBuffer<RGB>* pNewBfr = new RasterBuffer<RGB>(pFrame->buffer(), bbRel);
				//  Construct a new frame from the Raster Buffer
				Frame<RGB>* pNewFrame = new Frame<RGB>(pNewBfr, MapCT.getExtents(FX).Top, MapCT.getExtents(FX).Left, pLastFrag);

				//  Update the pointers
				if (pFrags == nullptr) pFrags = pLastFrag = pNewFrame;
				else pLastFrag = pNewFrame;
			}

			//  Return the mini train of fragments
			return pFrags;
		}

		//  collapseMap
		// 
		//  This function will collapse difference regions in the passed colour map
		//
		//	PARAMETERS:
		// 
		//		ColourTable&		-		Refeence to the colour table containing the regions to collapse
		//		size_t				-		Expansion limit
		//
		//	RETURNS:
		//
		//	NOTES:
		//

		static void		collapseMap(ColourTable<BYTE>& RegMap, size_t XLimit) {
			volatile size_t			NumColours = RegMap.getNumColours();								//  Iteration control
			BoundingBox				bbReg1 = {};														//  Region #1
			BoundingBox				bbReg2 = {};														//  Region #2
			size_t					Passes = 0;															//  Number of passes
			bool					Combined = true;													//  Loop control

			//
			//  Collapse any overlapping regions
			//

			for (size_t Expansion = 0; Expansion < XLimit; Expansion++) {
				while (Combined) {

					Combined = false;
					Passes++;

					for (size_t RCX = 0; RCX < NumColours; RCX++) {
						bbReg1 = RegMap.getExtents(RCX);
						//  Expand the region by the current factor
						if (bbReg1.Top >= Expansion) bbReg1.Top -= Expansion;
						if (bbReg1.Left >= Expansion) bbReg1.Left -= Expansion;
						bbReg1.Bottom += Expansion;
						bbReg1.Right += Expansion;
						for (size_t CCX = 0; CCX < NumColours; CCX++) {
							if (CCX != RCX && RegMap.getCount(RCX) > 0 && RegMap.getCount(CCX) > 0) {
								bbReg2 = RegMap.getExtents(CCX);
								//  Detect if the two regions intersect
								if (((bbReg2.Left >= bbReg1.Left) && (bbReg2.Left <= bbReg1.Right)) || ((bbReg2.Right >= bbReg1.Left) && (bbReg2.Right <= bbReg1.Right))) {
									if (((bbReg2.Top >= bbReg1.Top) && (bbReg2.Top <= bbReg1.Bottom)) || ((bbReg2.Bottom >= bbReg1.Top) && (bbReg2.Bottom <= bbReg1.Bottom))) {

										//  Overlap detected -- collapse the smaller region into the larger one
										if (RegMap.getCount(RCX) > RegMap.getCount(CCX)) RegMap.combine(CCX, RCX);
										else RegMap.combine(RCX, CCX);
										Combined = true;
										NumColours = RegMap.getNumColours();
										bbReg1 = RegMap.getExtents(RCX);
										//  Expand the region by the current factor
										if (bbReg1.Top >= Expansion) bbReg1.Top -= Expansion;
										if (bbReg1.Left >= Expansion) bbReg1.Left -= Expansion;
										bbReg1.Bottom += Expansion;
										bbReg1.Right += Expansion;
									}
								}
							}
						}
					}
				}
				Combined = true;
			}

			//  Collapse completed
			return;
		}

		//  buildTrainFromPlain
		//
		//  This static function will build a Canonical Train (Writeable) from the passed Plain Image Train
		//
		//  PARAMETERS
		//
		//		Train*			-		Pointer to the input plain image train
		//		SWITCHES		-		Options to use for forming the Canonical Train
		//
		//  RETURNS
		//
		//		Train*			-		Pointer to the constructed Canonical Train
		//
		//  NOTES
		//

		static Train<RGB>* buildTrainFromPlain(Train<RGB>* pTrain, SWITCHES Opts) {
			Train<RGB>* pCTrain = nullptr;												//  Pointer to the constructed canonical train

			//  Construct the base canonical train as a copy of the input train
			pCTrain = new Train<RGB>(*pTrain);

			//  Verify the copy
			if (pCTrain->getCanvasHeight() != pTrain->getCanvasHeight()) {
				std::cerr << "ERROR: GIF::buildTrainFromPlain() - Copy of train canvas height: " << pCTrain->getCanvasHeight() << 
					" does not equal original height: " << pTrain->getCanvasHeight() << "." << std::endl;
			}
			if (pCTrain->getCanvasWidth() != pTrain->getCanvasWidth()) {
				std::cerr << "ERROR: GIF::buildTrainFromPlain() - Copy of train canvas width: " << pCTrain->getCanvasWidth() << 
					" does not equal original width: " << pTrain->getCanvasWidth() << "." << std::endl;
			}
			if (pCTrain->getNumFrames() != pTrain->getNumFrames()) {
				std::cerr << "ERROR: GIF::buildTrainFromPlain() - Copy of train frames: " << pCTrain->getNumFrames() <<
					" does not equal original frames: " << pTrain->getNumFrames() << "." << std::endl;
			}

			if (pCTrain->getNumFrames() > 0) {
				Frame<RGB>*		pFrame = pCTrain->getFirstFrame();
				int				FNo = 1;
				while (pFrame != nullptr) {
					if (pFrame->getHeight() == 0) std::cerr << "ERROR: GIF::buildTrainFromPlain() - Frame: " << FNo << " of canonical train has zero height." << std::endl;
					if (pFrame->getWidth() == 0) std::cerr << "ERROR: GIF::buildTrainFromPlain() - Frame: " << FNo << " of canonical train has zero width." << std::endl;
					if (pFrame->getBuffer() == nullptr)std::cerr << "ERROR: GIF::buildTrainFromPlain() - Frame: " << FNo << " of canonical train has NULL raster buffer." << std::endl;
					pFrame = pFrame->getNext();
				}
			}
			else std::cerr << "ERROR: GIF::buildTrainFromPlain() - Canonical copy of input train contains no frames." << std::endl;

			//  The first step is to flatten the image
			pCTrain->flatten();

			//  Optimise Colour Usage
			//  A Canonical Train ONLY contains frames that use a maximum of 256 colours
			optimiseColourUsage(pCTrain, Opts);

			//  Return the constructed canonical train
			return pCTrain;
		}

		//  optimiseAnimationColourUsage
		//
		//  This static function will optimise the use of colours in the image. 
		//	No frame splitting is performed as this affects the structure of the animation, instead colours are replaced.
		//
		//  PARAMETERS
		//
		//		Train*			-		Pointer to the constructed Canonical Train
		//		SWITCHES		-		Options to use for forming the Canonical Train
		//
		//  RETURNS
		//
		//  NOTES
		//

		static void		optimiseAnimationColourUsage(Train<RGB>* pTrain, SWITCHES Opts) {
			Frame<RGB>*				pFrame = pTrain->getFirstFrame();							//  Only/First frame in the image
			ColourTable<RGB>		CT;															//  Colour Table

			Opts = Opts;
			//  Make sure that the background colour for the first frame is in the colour table
			CT.add(pTrain->getBackGround(), 0);

			//  Process each frame in turn
			while (pFrame != nullptr) {

				//  Add the image colours
				for (RasterBuffer<RGB>::iterator RIt = pFrame->buffer().top(); RIt != pFrame->buffer().bottom(); RIt++) {
					for (RasterBuffer<RGB>::iterator CIt = pFrame->buffer().left(RIt); CIt != pFrame->buffer().right(RIt); CIt++) {
						CT.add(*CIt, 1, RIt.getIndex(), CIt.getIndex());
					}
				}

				//  If the colour count is over 256 then we have to reduce the colour count in the frame
				while (CT.getNumColours() > 256) {

					//  Eliminate the use of the lowest used colour in the frame
					eliminateLowestUsedColour(pFrame, CT);
				}

				//  Clear the Colour Table and move on to the next frame
				CT.clear();
				pFrame = pFrame->getNext();
			}

			//  Return to caller
			return;
		}

		//  optimiseColourUsage
		//
		//  This static function will optimise the use of colours in the image. In particular it may split the frame into 
		//	multiple frames to ensure that a max of 256 colours are in use in any frame.
		//
		//  PARAMETERS
		//
		//		Train*			-		Pointer to the constructed Canonical Train
		//		SWITCHES		-		Options to use for forming the Canonical Train
		//
		//  RETURNS
		//
		//  NOTES
		//

		static void		optimiseColourUsage(Train<RGB>* pTrain, SWITCHES Opts) {
			Frame<RGB>*				pFrame = pTrain->getFirstFrame();							//  Only/First frame in the image
			bool					FirstFrame = true;											//  First Frame in the Image Train
			size_t					PartColour = 0;												//  Partition colour index
			ColourTable<RGB>		CT;															//  Colour Table

			Opts = Opts;
			//  Process each frame in turn
			while (pFrame != nullptr) {

				//  Clear down the colour table
				CT.clear();

				//  If this is the first frame then add the background colour if it is in use
				if (FirstFrame) {
					size_t		NumBkg = (pTrain->getCanvasHeight() * pTrain->getCanvasWidth()) - (pFrame->getHeight() * pFrame->getWidth());
					if (NumBkg > 0) {
						CT.add(pTrain->getBackGround(), NumBkg);
					}
					FirstFrame = false;
				}

				//  Add the image colours
				for (RasterBuffer<RGB>::iterator RIt = pFrame->buffer().top(); RIt != pFrame->buffer().bottom(); RIt++) {
					for (RasterBuffer<RGB>::iterator CIt = pFrame->buffer().left(RIt); CIt != pFrame->buffer().right(RIt); CIt++) {
						CT.add(*CIt, 1, RIt.getIndex(), CIt.getIndex());
					}
				}

				//  If we have 256 or more colours then resolve
				if (CT.getNumColours() > 256) {

					//
					//	Progressively remove frames of colour from the current image until the number of colours is 256 or less
					//

					while (CT.getNumColours() > 256) {

						//  Select the optimal partition to form a new Frame
						PartColour = selectOptimalPartition(CT);

						//  Check that we obtained a partition
						if (PartColour == CT.getNumColours()) cleaveFrame(pFrame, CT);
						else partitionFrame(pFrame, CT, PartColour);

						//  Clear and recompute the colour extents and counts
						CT.clear();
						//  Add the image colours
						for (RasterBuffer<RGB>::iterator RIt = pFrame->buffer().top(); RIt != pFrame->buffer().bottom(); RIt++) {
							for (RasterBuffer<RGB>::iterator CIt = pFrame->buffer().left(RIt); CIt != pFrame->buffer().right(RIt); CIt++) {
								CT.add(*CIt, 1, RIt.getIndex(), CIt.getIndex());
							}
						}
					}
				}

				//  Move on to the next frame
				pFrame = pFrame->getNext();
			}

			//  Return to caller
			return;
		}

		//  cleaveFrame
		//
		//  This static function will split the incoming frame into 2 (orthogonal to the longest axis). 
		//
		//		1.		Copy the selected extents to a new Raster Buffer.
		//		2.		Remove all partitioned colours from the Colour Table
		//		3.		If appropriate trim the source frame to eliminate the partitioned extents
		//		4.		Replace any pixels in the source that have been partitioned with a colour from the source
		//		5.		Replace any pixels in the partitioned frame that are in the source frame with a transparent colour
		//		6.		Insert a new frame after the source frame containing the new Raster Buffer
		//
		//  PARAMETERS
		//
		//		Frame*					-		Pointer to the frame to be partitioned
		//		ColourTable&			-		Reference to the colour table to be partitioned
		//
		//  RETURNS
		//
		//  NOTES
		//

		static void		cleaveFrame(Frame<RGB>* pFrame, ColourTable<RGB>& CT) {
			BoundingBox			PExtents = {};													//  Partition Extents

			//  Set the extents of the frame to be cleaved
			if (pFrame->getHeight() > pFrame->getWidth()) {
				//  Cleave Rows
				PExtents.Top = pFrame->getHeight() / 2;
				PExtents.Bottom = pFrame->getHeight() - 1;
				PExtents.Left = 0;
				PExtents.Right = pFrame->getWidth() - 1;
			}
			else {
				//  Cleave Columns
				PExtents.Top = 0;
				PExtents.Bottom = pFrame->getHeight() - 1;
				PExtents.Left = pFrame->getWidth() / 2;
				PExtents.Right = pFrame->getWidth() - 1;
			}

			//  Partition the frame
			partitionFrame(pFrame, CT, PExtents);

			//  Return to caller
			return;
		}

		//  eliminateLowerstUsedColour
		//
		//  This static function will eliminate the use of the lowest used colour in the passed frame.
		//
		//  PARAMETERS
		//
		//		Frame*			-		Pointer to the Frame
		//		CT&				-		Reference to the colour table
		//
		//  RETURNS
		//
		//  NOTES
		//

		static void		eliminateLowestUsedColour(Frame<RGB>* pFrame, ColourTable<RGB>& CT) {
			size_t		LUCX = CT.getLowestUsedColour();											//  Index of the lowest used colour
			RGB			TargetColour = CT.getColour(LUCX);											//  Target Colour to be eliminated
			BoundingBox	bbScan = CT.getExtents(LUCX);;												//  Extents of the scan area
			RGB			Substitute = {};															//  Substitute colour

			//  Expand the scan area one column to the left
			if (bbScan.Left > 0) bbScan.Left--;
			else if (bbScan.Top > 0) bbScan.Top--;
			else {
				bbScan.Right++;
				//  Scan to find the first non matching colour
				for (RasterBuffer<RGB>::iterator It = pFrame->buffer().begin(bbScan); It != pFrame->buffer().end(bbScan); It++) {
					if (*It != TargetColour) {
						Substitute = *It;
						break;
					}
				}
			}

			//  Perform the subsittution scan replacing the target colour with the immediately preceding colour
			for (RasterBuffer<RGB>::iterator It = pFrame->buffer().begin(bbScan); It != pFrame->buffer().end(bbScan); It++) {
				if (*It != TargetColour) {
					*It = Substitute;
				}
				else Substitute = *It;
			}

			//  Remove the colour from the colour table
			CT.remove(LUCX);

			//  return to caller
			return;
		}

		//  partitionFrame
		//
		//  This static function will partition the passed frame based on the extents of the selected colour index.
		//  The function will perform the following steps.
		//
		//		1.		Copy the selected extents to a new Raster Buffer.
		//		2.		Remove all partitioned colours from the Colour Table
		//		3.		If appropriate trim the source frame to eliminate the partitioned extents
		//		4.		Replace any pixels in the source that have been partitioned with a colour from the source
		//		5.		Replace any pixels in the partitioned frame that are in the source frame with a transparent colour
		//		6.		Insert a new frame after the source frame containing the new Raster Buffer
		//
		//  PARAMETERS
		//
		//		Frame*					-		Pointer to the frame to be partitioned
		//		ColourTable&			-		Reference to the colour table to be partitioned
		//		size_t					-		Index of the colour to use to partition the frame
		//
		//  RETURNS
		//
		//  NOTES
		//

		static void		partitionFrame(Frame<RGB>* pFrame, ColourTable<RGB>& CT, size_t PCX) {
			BoundingBox			PExtents = {};													//  Partition Extents

			//  Obtain the extents of the partition
			PExtents = CT.getExtents(PCX);

			//  Partition the frame
			partitionFrame(pFrame, CT, PExtents);

			//  Return to caller
			return;
		}

		//  partitionFrame
		//
		//  This static function will partition the passed frame by removing the indicated extents.
		//  The function will perform the following steps.
		//
		//		1.		Copy the selected extents to a new Raster Buffer.
		//		2.		Remove all partitioned colours from the Colour Table
		//		3.		If appropriate trim the source frame to eliminate the partitioned extents
		//		4.		Replace any pixels in the source that have been partitioned with a colour from the source
		//		5.		Replace any pixels in the partitioned frame that are in the source frame with a transparent colour
		//		6.		Insert a new frame after the source frame containing the new Raster Buffer
		//
		//  PARAMETERS
		//
		//		Frame*					-		Pointer to the frame to be partitioned
		//		ColourTable&			-		Reference to the colour table to be partitioned
		//		BoundingBox&			-		Reference to the extents to be excised
		//
		//  RETURNS
		//
		//  NOTES
		//

		static void		partitionFrame(Frame<RGB>* pFrame, ColourTable<RGB>& CT, BoundingBox& PExtents) {
			SizeVector			svTrim = {};													//  Size Vector for trim operations
			RasterBuffer<RGB>*	pRBPart = nullptr;												//  Partition Raster Buffer
			RGB					MostUsed = {};													//  Most used colour
			Frame<RGB>*			pNewFrame = nullptr;											//  Partitioned Frame
			bool				isOverlay = false;												//  New partition overlays the oriiginal frame

			//  Construct a new Raster Buffer to hold the Partition
			pRBPart = new RasterBuffer<RGB>((PExtents.Bottom - PExtents.Top) + 1, (PExtents.Right - PExtents.Left) + 1, nullptr);

			//  Blit the partition region into the new Raster Buffer
			pRBPart->blit(pFrame->buffer(), PExtents, 0, 0);

			//  Remove all excised colours from the source frame colour table
			CT.removeAll(PExtents);

			//
			//  Determine if the source Frame can be trimmed.
			//  The partition MUST remove complete rows or columns from the edge of the source image for this to be possible.
			//
			if (PExtents.Top == 0 && PExtents.Left == 0 && PExtents.Right == (pFrame->getWidth() - 1)) {
				//  Complete rows may be trimmed from the top of the image

				svTrim.Top = 0 - int(pRBPart->getHeight());
				svTrim.Bottom = 0;
				svTrim.Left = 0;
				svTrim.Right = 0;

				//  Trim the source image
				pFrame->buffer().resize(svTrim, nullptr);

				//  Adjust the location of the frame on the canvas
				pFrame->setRRow(pFrame->getRRow() + pRBPart->getHeight());
			}
			else if (PExtents.Bottom == (pFrame->getHeight() - 1) && PExtents.Left == 0 && PExtents.Right == (pFrame->getWidth() - 1)) {
				//  Complete rows may be trimmed from the bottom of the image

				svTrim.Top = 0;
				svTrim.Bottom = 0 - int(pRBPart->getHeight());
				svTrim.Left = 0;
				svTrim.Right = 0;

				//  Trim the source image
				pFrame->buffer().resize(svTrim, nullptr);
			}
			else if (PExtents.Left == 0 && PExtents.Top == 0 && PExtents.Bottom == (pFrame->getHeight() - 1)) {
				//  Complete columns may be trimmed from the left of the image

				svTrim.Top = 0;
				svTrim.Bottom = 0;
				svTrim.Left = 0 - int(pRBPart->getWidth());
				svTrim.Right = 0;

				//  Trim the source image
				pFrame->buffer().resize(svTrim, nullptr);

				//  Adjust the location of the frame on the canvas
				pFrame->setRCol(pFrame->getRCol() + pRBPart->getWidth());
			}
			else if (PExtents.Right == (pFrame->getWidth() - 1) && PExtents.Top == 0 && PExtents.Bottom == (pFrame->getHeight() - 1)) {
				//  Complete columns may be trimmed from the right of the image

				svTrim.Top = 0;
				svTrim.Bottom = 0;
				svTrim.Left = 0;
				svTrim.Right = 0 - int(pRBPart->getWidth());

				//  Trim the source image
				pFrame->buffer().resize(svTrim, nullptr);

			}
			else {

				//  The partition could not be removed from the source frame, eliminate pixels for the partitioned colours
				//  Obtain the substitute colour
				isOverlay = true;
				MostUsed = CT.getMostUsedColour();

				//  Iterate over the partitioned area substituting the most used colour for any partitioned colours
				for (RasterBuffer<RGB>::iterator It = pFrame->buffer().begin(PExtents); It != pFrame->buffer().end(PExtents); It++) {
					if (!CT.hasColour(*It)) *It = MostUsed;
				}

				//  Make a pass over the partitioned RasterBuffer assigning a transparent colour to any pixels that are in the source image
				for (RasterBuffer<RGB>::iterator It = pRBPart->begin(); It != pRBPart->end(); It++) {
					if (CT.hasColour(*It)) *It = MostUsed;
				}
			}


			//  Construct the new frame
			pNewFrame = new Frame<RGB>(pRBPart, pFrame->getRRow() + PExtents.Top, pFrame->getRCol() + PExtents.Left, nullptr);
			if (isOverlay) pNewFrame->setTransparent(MostUsed);
			pNewFrame->setDisposal(Frame<RGB>::DISPOSE_NOT);
			pNewFrame->setDelay(0);

			//  Insert the frame into the chain
			pNewFrame->setNext(pFrame->getNext());
			pNewFrame->setPrev(pFrame);
			pFrame->setNext(pNewFrame);
			if (pNewFrame->getNext() != nullptr) pNewFrame->getNext()->setPrev(pNewFrame);

			//  Return to caller
			return;
		}

		//  selectOptimalPartition
		//
		//  This static function will select the best partition from the passed colour table to partition the image into 2 frames.
		//
		//  PARAMETERS
		//
		//		ColourTable&			-		Reference to the colour table to be partitioned
		//
		//  RETURNS
		//
		//		size_t					-		Index of the in the colour table that defines the bounds of the partition
		//
		//  NOTES
		//

		static size_t		selectOptimalPartition(ColourTable<RGB>& CT) {
			size_t			OptimalColour = 0;															//  Optimal Colour
			double			OptimalScore = 9999999.0;													//  Best Score
			size_t			NumColours = CT.getNumColours();											//  Number of colours to consider
			size_t			CX = 0, CCX = 0;															//  Colour indexes
			size_t			Pixels = 0;																	//  Pixel count

			//  Safety
			if (NumColours == 0) return 0;

			//  Allocate the array of partition entries
			PTE* pPartTable = new PTE[NumColours];
			if (pPartTable == nullptr) return NumColours;

			//  Populate the Partition Table
			for (CX = 0; CX < NumColours; CX++) {
				//  Fill in the height * width of the partition
				CCX = CX;
				pPartTable[CX].H = (CT.getExtents(CCX).Bottom - CT.getExtents(CCX).Top) + 1;
				pPartTable[CX].W = (CT.getExtents(CCX).Right - CT.getExtents(CCX).Left) + 1;
				Pixels = pPartTable[CCX].H * pPartTable[CCX].W;
				pPartTable[CX].Pixels = Pixels;

				//  Count the number of wholly contained colour partitions within the current partition (including self)
				for (CCX = 0; CCX < NumColours; CCX++) {
					if (CT.getExtents(CCX).Top >= CT.getExtents(CX).Top &&
						CT.getExtents(CCX).Bottom <= CT.getExtents(CX).Bottom &&
						CT.getExtents(CCX).Left >= CT.getExtents(CX).Left &&
						CT.getExtents(CCX).Right <= CT.getExtents(CX).Right) pPartTable[CX].Colours++;
				}

				//  Set the score for this entry
				pPartTable[CX].Score = double(pPartTable[CX].Pixels) / (double(pPartTable[CX].Colours) * double(pPartTable[CX].Colours));
			}
			
			//  Select the best entry - this is the lowest score that removes sufficient colours to reduce the clour table to 256
			//  in a single partition.
			for (CX = 0; CX < NumColours; CX++) {
				if (pPartTable[CX].Score < OptimalScore) {
					if (((CT.getNumColours() - pPartTable[CX].Colours) <= 256) && pPartTable[CX].Colours < 256) {
						OptimalColour = CX;
						OptimalScore = pPartTable[CX].Score;
					}
				}
			}

			//  If no suitable selection was found signa to the higher level
			if (OptimalScore > 100.0) {
				delete[] pPartTable;
				return NumColours;
			}

			//  Free the Partition Table
			delete[] pPartTable;

			//  Return the index of the optimal colour
			return OptimalColour;
		}

		//  serialiseTrain
		//
		//  This static function will build an in-memory GIF image from the passed CANONICAL Train.
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
		//		The input Train MUST be a GIF CANONICAL Train
		//

		static BYTE* serialiseTrain(Train<RGB>* pTrain, size_t& ImgSize, SWITCHES Opts) {
			BYTE*			pImage = nullptr;													//  Pointer to in-memory image
			size_t			ImgAlc = 0;															//  Allocated size of the in-memory image
			size_t			ImgUsed = 0;														//  Bytes used in the in-memory image
			int				ImageType = 0;														//  GIF Image Type
			size_t			ImgEst = 0;															//  (Over) Estimated initial size of the image
			Frame<RGB>*		pFrame = pTrain->getFirstFrame();									//  Pointer to the current frame
			ColourTable<RGB>			CT;														//  Colour table for the image
			GIF_ODI_COLOUR_TABLE		GCT = {};												//  Global Colour Table


			//  Determine the type of GIF Image being constructed
			ImageType = categoriseTrain(pTrain);

			//  If the image is an animation then flag that
			if (ImageType == GIF_ANIMATION) Opts = Opts | GIF_INT_ANIMATION;					//  Flag an animation being serialised

			//  Allocate space for the complete image (it will auto expand if too small)
			ImgEst = 4096 + (256 * sizeof(RGB)) + (pTrain->getCanvasHeight() * pTrain->getCanvasWidth());
			ImgAlc = 0;
			ImgUsed = 0;
			pImage = (BYTE*) malloc(ImgEst);
			if (pImage == nullptr) return nullptr;
			ImgAlc = ImgEst;
			memset(pImage, 0, ImgEst);

			//  Images ALWAYS use a Global Colour Table (GCT) for the first frame in the train. Build the GCT
			//  Add the background colour as the first entry in the table
			CT.add(pTrain->getBackGround(), 0);

			//  Add all pixels from the initial frame
			for (RasterBuffer<RGB>::iterator It = pFrame->buffer().begin(); It != pFrame->buffer().end(); It++) CT.add(*It, 1);

			//  Enrich the Global Colour Table (If possible) with the colours from all subsequent frames
			//  This is done to try and get away with ONLY using a Global Colour Table
			pFrame = pFrame->getNext();
			while (CT.getNumColours() < 256 && pFrame != nullptr) {
				for (RasterBuffer<RGB>::iterator It = pFrame->buffer().begin(); It != pFrame->buffer().end(); It++) {
					if (CT.getNumColours() == 256) break;
					CT.add(*It, 0);
				}
				pFrame = pFrame->getNext();
			}

			//  Populate the on-disk colour table from the captured colour table
			for (size_t CX = 0; CX < CT.getNumColours(); CX++) 	GCT.Entry[CX] = CT.getColour(CX);

			//  Append a GIF File Header to the stream
			appendFileHeader(pImage, ImgUsed);

			//  Append a Logical Screen Descriptor (LSD) to the image
			appendLSD(pImage, ImgUsed, pTrain->getCanvasHeight(), pTrain->getCanvasWidth(), CT.getNumColours(), 0);

			//  Append the Global Colour Table (GCT) to the in-memory image
			appendColourTable(pImage, ImgUsed, &GCT, CT.getNumColours());

			//  If the image is an Animation then append a Netscape Application Extension Block to set the play iteration counter
			if (Opts & GIF_INT_ANIMATION) appendNNAXB(pImage, ImgUsed, Opts);

			//  Append the images (GCE, ID, optional LCT, EEB) for each frame
			pFrame = pTrain->getFirstFrame();
			while (pFrame != nullptr) {

				//  Check that we have enough free space in the image memory block for the frame
				ImgEst = 256 + (256 & sizeof(RGB)) + (pFrame->getHeight() * pFrame->getWidth());
				if ((ImgAlc - ImgUsed) < ImgEst) {
					//  Expand the image memory block
					BYTE* pNewIBfr = (BYTE*) realloc(pImage, ImgAlc + ImgEst);
					if (pNewIBfr != nullptr) {
						pImage = pNewIBfr;
						ImgAlc += ImgEst;
						memset(pImage + ImgUsed, 0, ImgAlc - ImgUsed);
					}
				}

				//  Append the image data
				appendImage(pImage, ImgUsed, pFrame, CT, Opts);

				//  Move on to the next frame
				pFrame = pFrame->getNext();
			}

			//  Append the GIF File Trailer
			appendFileTrailer(pImage, ImgUsed);

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
		//  This static function will append a GIF File header to the in-memory image.
		//
		//  PARAMETERS
		//
		//		Byte*			-		Pointer to the in-memory image
		//		size_t&			-		Reference to the size used of the in-memory image
		//
		//  RETURNS
		//
		//  NOTES
		//
		//		GIF Image are ALWAYS generated to the latest specification version 'GIF89a'
		//

		static void		appendFileHeader(BYTE* pImage, size_t& ImgUsed) {
			GIF_FILE_HEADER*		pFH = (GIF_FILE_HEADER*) (pImage + ImgUsed);								//  Current point is a File Header

			//  Complete the File Header data
			//  File signature bytes
			pFH->signature[0] = GFH_SIG1;
			pFH->signature[1] = GFH_SIG2;
			pFH->signature[2] = GFH_SIG3;

			//  Image version bytes
			pFH->version[0] = GFH_DEF_V1;
			pFH->version[1] = GFH_DEF_V2;
			pFH->version[2] = GFH_DEF_V3;

			//  Update the size of the image
			ImgUsed += sizeof(GIF_FILE_HEADER);

			//  Return to caller
			return;
		}

		//  appendLSD
		//
		//  This static function will append a Logical Screen Descriptor (LSD) to the in-memory image.
		//
		//  PARAMETERS
		//
		//		Byte*			-		Pointer to the in-memory image
		//		size_t&			-		Reference to the size used of the in-memory image
		//		size_t			-		Canvas Height
		//		size_t			-		Canvas Width
		//		size_t			-		Number of entries in the Global Colour Table (GCT), 0 - implies NO GCT
		//		size_t			-		Coulour index of the background colour
		//
		//  RETURNS
		//
		//  NOTES
		//
		//

		static void		appendLSD(BYTE* pImage, size_t& ImgUsed, size_t CanvasHeight, size_t CanvasWidth, size_t GCTEnts, size_t BCGX) {
			GIF_LOGICAL_SCREEN_DESCRIPTOR* pLSD = (GIF_LOGICAL_SCREEN_DESCRIPTOR*) (pImage + ImgUsed);

			//  Clear the LSD
			memset(pLSD, 0, sizeof(GIF_LOGICAL_SCREEN_DESCRIPTOR));

			//  Fill in the LSD
			//  Canvas Dimensions
			SetSize(pLSD->height, CanvasHeight);
			SetSize(pLSD->width, CanvasWidth);

			//  Set 8 bit colour channel (i.e. 24 bit RGB)
			SetColourBits(pLSD->bitsettings, 8);

			//  If there is a GCT then set the GCT flag and the number of entries in the Colour Table
			if (GCTEnts > 0) {
				pLSD->bitsettings = pLSD->bitsettings | GIF_HDRBITS_GCT;

				size_t	CTCap = 2;
				size_t	CTCapVal = 0;

				while (CTCap < GCTEnts) {
					CTCap = CTCap << 1;
					CTCapVal++;
				}

				pLSD->bitsettings = pLSD->bitsettings | BYTE(CTCapVal & GIF_HDRBITS_GCTSIZEMASK);
			}

			//  Set the background colour index
			pLSD->background = BYTE(BCGX);

			//  Update the image size used
			ImgUsed += sizeof(GIF_LOGICAL_SCREEN_DESCRIPTOR);

			//  Return to caller
			return;
		}

		//  appendColourTable
		//
		//  This static function will append a Colour Table (Global or Local) to the image
		//
		//  PARAMETERS
		//
		//		Byte*							-		Pointer to the in-memory image
		//		size_t&							-		Reference to the size used of the in-memory image
		//		GIF_ODI_COLOUR_TABLE*			-		Pointer to the colour table to be inserted
		//		size_t							-		Number of entries in the Colour Table
		//
		//  RETURNS
		//
		//  NOTES
		//
		//		It is the callers responsibility to ensure that the image has sufficient capacity to hold the colour table
		//

		static void		appendColourTable(BYTE* pImage, size_t& ImgUsed, GIF_ODI_COLOUR_TABLE* pCT, size_t CTEnts) {
			size_t			CTCap = 2;														//  Table Capacity

			//  Compute the capacity
			while (CTCap < CTEnts) CTCap = CTCap << 1;

			//  Append the Colour Table to the image
			memcpy(pImage + ImgUsed, pCT, CTCap * sizeof(RGB));

			//  Update the image size
			ImgUsed += (CTCap * sizeof(RGB));

			//  Return to caller
			return;
		}

		//  appendImage
		//
		//  This static function will append an image (GCE, ID & EEB) 
		//
		//  PARAMETERS
		//
		//		Byte*					-		Pointer to the in-memory image
		//		size_t&					-		Reference to the size used of the in-memory image
		//		Frame*					-		Pointer to the Frame to be appended
		//		CoulourTable&			-		Reference to the Global Colour Table
		//		SWITCHES				-		Options to use for storing the image
		//
		//  RETURNS
		//
		//  NOTES
		//

		static void		appendImage(BYTE* pImage, size_t& ImgUsed, Frame<RGB>* pFrame, ColourTable<RGB>& GCT, SWITCHES Opts) {
			ColourTable<RGB>			LCT;													//  Colour table for the image
			GIF_ODI_COLOUR_TABLE		LocCT = {};												//  Local Colour Table
			size_t						CTEnts = 0;												//  Number of entries in the colour table
			bool						UseGCT = false;											//  Global/Local colour table use
			size_t						TCX = 0;												//  Transparent Colour Index

			//  Build the colour table for the current image
			for (RasterBuffer<RGB>::iterator It = pFrame->buffer().begin(); It != pFrame->buffer().end(); It++) LCT.add(*It, 1);

			//  Determine if the current frame can use the GCT
			if (GCT.contains(LCT)) {
				UseGCT = true;
				//  Build the on-disk image for the Global Colout Table
				for (size_t CX = 0; CX < GCT.getNumColours(); CX++) {
					LocCT.Entry[CX] = GCT.getColour(CX);
				}
				CTEnts = GCT.getNumColours();
			}
			else {
				//  Build the On-Disk Image for the Local Colour Table
				for (size_t CX = 0; CX < LCT.getNumColours(); CX++) {
					LocCT.Entry[CX] = LCT.getColour(CX);
				}
				CTEnts = LCT.getNumColours();
			}

			//  If the frame has a transparent (Green Screen) colour then determine the index in the colour table
			if (pFrame->hasTransparent()) {
				for (TCX = 0; TCX < CTEnts; TCX++) {
					if (LocCT.Entry[TCX] == pFrame->getTransparent()) break;
				}
			}

			//  Append the Graphics Control Extension (GCE)
			appendGCE(pImage, ImgUsed, pFrame, TCX);

			//  Append the Image Descriptor (ID) and optionally the Local Colour Table
			if (UseGCT) appendID(pImage, ImgUsed, pFrame, 0);
			else {
				appendID(pImage, ImgUsed, pFrame, CTEnts);
				appendColourTable(pImage, ImgUsed, &LocCT, CTEnts);
			}

			//  Append the Entropy Encoded Buffer (EEB)
			appendEEB(pImage, ImgUsed, pFrame, &LocCT, CTEnts, Opts);

			//  Return to caller
			return;
		}

		//  appendGCE
		//
		//  This static function will append a Graphicas Control Extension (CGE) to the image
		//
		//  PARAMETERS
		//
		//		Byte*					-		Pointer to the in-memory image
		//		size_t&					-		Reference to the size used of the in-memory image
		//		Frame*					-		Pointer to the Frame to be appended
		//		size_t					-		Transparent Colour Index
		//
		//  RETURNS
		//
		//  NOTES
		//

		static void		appendGCE(BYTE* pImage, size_t& ImgUsed, Frame<RGB>* pFrame, size_t TCX) {
			GIF_GRAPHICS_CONTROL_EXTENSION*		pGCE = (GIF_GRAPHICS_CONTROL_EXTENSION*) (pImage + ImgUsed);		//  Pointer to the GCE

			//  Clear the GCE
			memset(pGCE, 0, sizeof(GIF_GRAPHICS_CONTROL_EXTENSION));

			//  Fill the GCE Fields
			pGCE->introducer = GCE_SIG_INT;
			pGCE->label = GCE_SIG_LAB;
			pGCE->size = 4;
			SetDisposalMethod(pGCE->bitsettings, BYTE(pFrame->getDisposal()));
			if (pFrame->hasTransparent()) {
				pGCE->bitsettings = BYTE(pGCE->bitsettings | GIF_GCEBITS_TRANSP);
				pGCE->transparent = BYTE(TCX);
			}
			SetSize(pGCE->delay, pFrame->getDelay());

			//  Update the image size
			ImgUsed += sizeof(GIF_GRAPHICS_CONTROL_EXTENSION);

			//  Return to caller
			return;
		}

		//  appendID
		//
		//  This static function will append an Image Descriptor (ID) to the image
		//
		//  PARAMETERS
		//
		//		Byte*					-		Pointer to the in-memory image
		//		size_t&					-		Reference to the size used of the in-memory image
		//		Frame*					-		Pointer to the Frame to be appended
		//		size_t					-		Number of entries in the Local Colour Table (LCT), 0 ==> Use GCT
		//
		//  RETURNS
		//
		//  NOTES
		//
		//		We NEVER mark the colour table as sorted.
		//		We do NOT support interlaced images.
		//

		static void		appendID(BYTE* pImage, size_t& ImgUsed, Frame<RGB>* pFrame, size_t LCTE) {
			GIF_IMAGE_DESCRIPTOR*	pID = (GIF_IMAGE_DESCRIPTOR*)(pImage + ImgUsed);				//  Pointer to the ID

			//  Clear the ID
			memset(pID, 0, sizeof(GIF_IMAGE_DESCRIPTOR));

			//  Fill in the Image descriptor
			pID->signature = ID_SIG;
			SetSize(pID->height, pFrame->getHeight());
			SetSize(pID->width, pFrame->getWidth());
			SetSize(pID->top, pFrame->getRRow());
			SetSize(pID->left, pFrame->getRCol());

			//  If a local colour table is used by this frame then indicate and set the size
			if (LCTE > 0) {
				pID->bitsettings = pID->bitsettings | GIF_IDBITS_LCT;

				size_t	CTCap = 2;
				size_t	CTCapVal = 0;

				while (CTCap < LCTE) {
					CTCap = CTCap << 1;
					CTCapVal++;
				}

				pID->bitsettings = pID->bitsettings | (CTCapVal & GIF_IDBITS_LCTSIZEMASK);
			}

			//  Update the image size
			ImgUsed += sizeof(GIF_IMAGE_DESCRIPTOR);

			//  Return to caller
			return;
		}

		//  appendEEB
		//
		//  This static function will build and append an Entropy Encoded Buffer (EEB) to the image
		//
		//  PARAMETERS
		//
		//		Byte*					-		Pointer to the in-memory image
		//		size_t&					-		Reference to the size used of the in-memory image
		//		Frame*					-		Pointer to the Frame to be appended
		//		GIF_ODI_COLOUR_TABLE*	-		Pointer to the colour table for the frame (LCT or GCT)
		//		size_t					-		Number of entries in the Colour Table (LCT)
		//		SWITCHES				-		Options to use for storing the image
		//
		//  RETURNS
		//
		//  NOTES
		//

		static void		appendEEB(BYTE* pImage, size_t& ImgUsed, Frame<RGB>* pFrame, GIF_ODI_COLOUR_TABLE* pCT, size_t CTE, SWITCHES Opts) {
			LZW			Encoder;																					//  LZW encoder
			GIF_ENTROPY_ENCODED_BUFFER*		pEEB = (GIF_ENTROPY_ENCODED_BUFFER*) (pImage + ImgUsed);				//  Pointer to the EEB
			size_t		EEBSize = 1;																				//  Size of the EEB
			size_t		CTCap = 2;																					//  Colour Table Capacity
			size_t		CTCapVal = 1;																				//  Bits needed to index the colour table
			size_t		CX = 0;																						//  Colour Index
			SegmentedStream		bsOut(pFrame->getHeight() * pFrame->getWidth(), 4096);								//  Extensible segmented Byte Stream

			//  Calculate the native symbol size - based on the nunber of bits needed to index the colour table

			while (CTCap < CTE) {
				CTCap = CTCap << 1;
				CTCapVal++;
			}

			///// FIX - Min NCS == 2 //////////
			if (CTCapVal == 1) CTCapVal = 2;
			///////////////////////////////////

			//  Set the native symbol size
			pEEB->nativecodesize = BYTE(CTCapVal);

			//  If the Assert Clear option is passed then set the encoder clear policy on
			if (Opts & GIF_STORE_OPT_NOCLEAR) Encoder.disableClearOnFull();

			//  Obtain the Collector to pass the image into the encoder
			LZW::Collecter	ImgIn = Encoder.encode(bsOut, int(CTCapVal));
			size_t			PixEmitted = 0;

			//  Pass the pixels in the image one at a time to the collector as the index into the colour table
			for (RasterBuffer<RGB>::iterator It = pFrame->buffer().begin(); It != pFrame->buffer().end(); It++) {
				for (CX = 0; CX < CTE; CX++) {
					if (*It == pCT->Entry[CX]) break;
				}
				ImgIn.next(BYTE(CX));
				PixEmitted++;
				if (bsOut.eos()) {
					std::cerr << "ERROR: End-Of-Stream signalled on the output byte stream after emitting: " << PixEmitted << " pixels (" << Encoder.getTokenCount() << " tokens)." << std::endl;
					break;
				}
			}

			//  Signal end of the image stream to the collector
			ImgIn.signalEndOfStream();

			//  Copy the segmented stream generated to the EEB
			memcpy(&pEEB->segment, bsOut.getBufferAddress(), bsOut.getBytesWritten());
			EEBSize += bsOut.getBytesWritten();

			//  Update the Image Size
			ImgUsed += EEBSize;

			//  Return to caller
			return;
		}

		//  appendNNAXB
		//
		//  This static function will append a GIF Netscape Navigator Application Extension Block
		//
		//  PARAMETERS
		//
		//		BYTE*			-		Pointer to the in-memory image
		//		size_t&			-		Reference to the size used of the in-memory image
		//		SWITCHES		-		Options
		//
		//  RETURNS
		//
		//  NOTES
		//

		static void		appendNNAXB(BYTE* pImage, size_t& ImgUsed, SWITCHES Opts) {
			GIF_NETSCAPE_APPLICATION_BLOCK*		pNNAXB = (GIF_NETSCAPE_APPLICATION_BLOCK*)(pImage + ImgUsed);			//  App Extansion Block (Netscape)

			//  Fill in the block
			pNNAXB->introducer = GIF_XBLK_SIG;
			pNNAXB->label = 0xFF;
			pNNAXB->size = BYTE(11);
			memcpy(&pNNAXB->appname, "NETSCAPE", 8);
			memcpy(&pNNAXB->auth, "2.0", 3);
			pNNAXB->appdatasize = BYTE(3);
			pNNAXB->subblockindex = BYTE(1);

			//  Fill in the repeat number acconrding to the selected option
			if (Opts & GIF_ANIMATE_OPT_ONCE) {
				pNNAXB->repeats[0] = 0;
				pNNAXB->repeats[1] = 0;
			}
			else {
				pNNAXB->repeats[0] = 0xFF;
				pNNAXB->repeats[1] = 0xFF;
			}

			pNNAXB->endblock = 0x00;

			//  Update the Image Size
			ImgUsed += sizeof(GIF_NETSCAPE_APPLICATION_BLOCK);

			//  Return to caller
			return;
		}

		//  appendFileTrailer
		//
		//  This static function will append a GIF File trailer to the in-memory image.
		//
		//  PARAMETERS
		//
		//		Byte*			-		Pointer to the in-memory image
		//		size_t&			-		Reference to the size used of the in-memory image
		//
		//  RETURNS
		//
		//  NOTES
		//
		//		GIF Image are ALWAYS generated to the latest specification version 'GIF89a'
		//

		static void		appendFileTrailer(BYTE* pImage, size_t& ImgUsed) {
			GIF_FILE_TRAILER*		pFT = (GIF_FILE_TRAILER*) (pImage + ImgUsed);							//  GIF File Trailer

			//  Fill in the trailer
			pFT->signature = GIF_SIG_END;

			//  Update the image size
			ImgUsed += sizeof(GIF_FILE_TRAILER);

			//  Return to caller
			return;
		}

		//  categoriseTrain
		//
		//  This static function will categorised the Train as a GIF_* Image Type
		//
		//  PARAMETERS
		//
		//		Train*			-		Pointer to the input Train
		//
		//  RETURNS
		//
		//		int				-		GIF Image Type
		//
		//  NOTES
		//

		static int		categoriseTrain(Train<RGB>* pTrain) {
			size_t		NumFrames = 0;															//  Number of frames in the train
			size_t		CumDelay = 0;															//  Cumulative display delay
			Frame<RGB>* pFrame = pTrain->getFirstFrame();										//  Frame

			while (pFrame != nullptr) {
				NumFrames++;
				CumDelay += pFrame->getDelay();
				pFrame = pFrame->getNext();
			}

			//  Determine the type
			if (NumFrames == 0) return GIF_DEGENERATE_IMAGE;

			if (NumFrames == 1) return GIF_PLAIN_IMAGE;

			if (CumDelay == 0) return GIF_TILED_IMAGE;

			return GIF_ANIMATION;
		}

		//  Analysis Documentation Functions

		//  showFileHeader
		//
		//  This static function will document the GIF File Header
		//
		//  PARAMETERS
		//
		//		ODIMap&			-		Reference to the GIF On-Disk-Image map
		//		std::ostream&	-		Reference to the output stream
		//
		//  RETURNS
		//
		//  NOTES
		//

		static void		showFileHeader(ODIMap& Map, std::ostream& OS) {
			size_t			BlockNo = 0;													//  Block number for File Header
			size_t			Offset = 0;														//  Offset in the file
			
			GIF_FILE_HEADER*	pFH = (GIF_FILE_HEADER*) (Map.Image + Offset);				//  Pointer to the File Header

			OS << std::endl;
			OS << "GIF FILE HEADER - Block: " << BlockNo << ", Offset: +" << Offset << ", Size: " << Map.Blocks[BlockNo].BlockSize << "." << std::endl;

			//  Dump the memory image
			OS << std::endl;
			MemoryDumper::dumpMemory(Map.Image + Offset, sizeof(GIF_FILE_HEADER), "File Header", OS);
			OS << std::endl;

			//  Show the content
			OS << "File Signature: '" << pFH->signature[0] << pFH->signature[1] << pFH->signature[2] << "'." << std::endl;
			OS << "GIF Verion:      " << pFH->version[0] << pFH->version[1] << pFH->version[2] << "." << std::endl;

			//  Return to caller
			return;
		}

		//  showLSD
		//
		//  This static function will document the GIF Logical Screen Descriptor (LSD)
		//
		//  PARAMETERS
		//
		//		ODIMap&			-		Reference to the GIF On-Disk-Image map
		//		std::ostream&	-		Reference to the output stream
		//
		//  RETURNS
		//
		//  NOTES
		//

		static void		showLSD(ODIMap& Map, std::ostream& OS) {
			size_t			BlockNo = 1;													//  Block number for the LSD
			size_t			Offset = 0;														//  Offset in the file
			char			PrintLine[MAX_PATH] = {};										//  Printline

			for (size_t B = 0; B < BlockNo; B++) Offset += Map.Blocks[B].BlockSize;			//  Accumulate the offset to the LSD

			GIF_LOGICAL_SCREEN_DESCRIPTOR*	pLSD = (GIF_LOGICAL_SCREEN_DESCRIPTOR*) (Map.Image + Offset);

			OS << std::endl;
			OS << "GIF LOGICAL SCREEN DESCRIPTOR - Block: " << BlockNo << ", Offset: +" << Offset << ", Size: " << Map.Blocks[BlockNo].BlockSize << "." << std::endl;

			//  Dump the memory image
			OS << std::endl;
			MemoryDumper::dumpMemory(Map.Image + Offset, sizeof(GIF_LOGICAL_SCREEN_DESCRIPTOR), "LSD", OS);
			OS << std::endl;

			//  Show the content
			sprintf_s(PrintLine, MAX_PATH, "Canvas Height: \t\t\t%i pixels.", GetSize(pLSD->height));
			OS << PrintLine << std::endl;
			sprintf_s(PrintLine, MAX_PATH, "Canvas Width: \t\t\t%i pixels.", GetSize(pLSD->width));
			OS << PrintLine << std::endl;
			sprintf_s(PrintLine, MAX_PATH, "Bit Fields: \t\t\t%i (0x%.2x).", int(pLSD->bitsettings), int(pLSD->bitsettings));
			OS << PrintLine << std::endl;
			if (pLSD->bitsettings & GIF_HDRBITS_GCT) {
				OS << "   0x80 - Image has a Global Colour Table (GCT) with " << ColourTableSize(pLSD->bitsettings) << " entries." << std::endl;
				if (pLSD->bitsettings & GIF_HDRBITS_SORT) OS << "   0x08 - The Global Colour Table is sorted." << std::endl;
			}
			sprintf_s(PrintLine, MAX_PATH, "Colour Resolution: \t\t%i bits.", ColourBits(pLSD->bitsettings));
			OS << PrintLine << std::endl;
			if (pLSD->bitsettings & GIF_HDRBITS_GCT) {
				sprintf_s(PrintLine, MAX_PATH, "Background Colour Index: \t%i.", int(pLSD->background));
				OS << PrintLine << std::endl;
			}

			//  Return to caller
			return;
		}

		//  showGCT
		//
		//  This static function will document the GIF Global Colour Table (GCT) if there is one in the image
		//
		//  PARAMETERS
		//
		//		ODIMap&			-		Reference to the GIF On-Disk-Image map
		//		std::ostream&	-		Reference to the output stream
		//
		//  RETURNS
		//
		//  NOTES
		//

		static void		showGCT(ODIMap& Map, std::ostream& OS) {
			size_t			BlockNo = 1;													//  Block number for the LSD
			size_t			Offset = 0;														//  Offset in the file
			size_t			CTEnts = 0;														//  Number of entries in the colour table

			for (size_t B = 0; B < BlockNo; B++) Offset += Map.Blocks[B].BlockSize;			//  Accumulate the offset to the LSD

			GIF_LOGICAL_SCREEN_DESCRIPTOR* pLSD = (GIF_LOGICAL_SCREEN_DESCRIPTOR*) (Map.Image + Offset);

			Offset += Map.Blocks[BlockNo].BlockSize;
			BlockNo++;

			//  See if a GCT is present at Block 2
			if (Map.Blocks[BlockNo].BlockType != GIF_BLOCK_GCT) return;

			//  Get the count of entries
			CTEnts = ColourTableSize(pLSD->bitsettings);
			if (CTEnts == 0) return;

			GIF_ODI_COLOUR_TABLE*		pGCT = (GIF_ODI_COLOUR_TABLE*)(Map.Image + Offset);

			OS << std::endl;
			OS << "GIF GLOBAL COLOUR TABLE - Block: " << BlockNo << ", Offset: +" << Offset << ", Size: " << Map.Blocks[BlockNo].BlockSize << "." << std::endl;

			//  Dump the memory image
			OS << std::endl;
			MemoryDumper::dumpMemory(Map.Image + Offset, CTEnts * sizeof(RGB), "GCT", OS);
			OS << std::endl;

			OS << std::endl;
			OS << "Colour Table Entries: " << CTEnts << "." << std::endl;
			OS << std::endl;

			//  Show each of the entries in the table
			for (size_t CX = 0; CX < CTEnts; CX++) {
				OS << " Entry: " << CX << ", Colour: ";
				pGCT->Entry[CX].document(OS);
				OS << "." << std::endl;
			}

			//  Return to caller
			return;
		}

		//  showFrames
		//
		//  This static function will document the Image Frames in the On-Disk image
		//
		//  PARAMETERS
		//
		//		ODIMap&			-		Reference to the GIF On-Disk-Image map
		//		std::ostream&	-		Reference to the output stream
		//
		//  RETURNS
		//
		//  NOTES
		//

		static void		showFrames(ODIMap& Map, std::ostream& OS) {
			size_t			BlockNo = 1;													//  Block number for the LSD
			size_t			Offset = 0;														//  Offset in the file
			size_t			GCTEnts = 0;													//  Number of entries in the global colour table
			size_t			FrameNo = 0;													//  Frame Number
			char			PrintLine[MAX_PATH] = {};										//  Printline
			char			Disposal[50] = {};												//  Disposal method

			for (size_t B = 0; B < BlockNo; B++) Offset += Map.Blocks[B].BlockSize;			//  Accumulate the offset to the LSD

			GIF_LOGICAL_SCREEN_DESCRIPTOR*	pLSD = (GIF_LOGICAL_SCREEN_DESCRIPTOR*) (Map.Image + Offset);

			Offset += Map.Blocks[BlockNo].BlockSize;
			BlockNo++;

			//  See if a GCT is present at Block 2
			if (Map.Blocks[BlockNo].BlockType == GIF_BLOCK_GCT) {
				GCTEnts = ColourTableSize(pLSD->bitsettings);
				BlockNo++;
				Offset += GCTEnts * sizeof(RGB);
			}

			//
			//  Loop processing each set of frame blocks until the fie trailer is encountered
			//

			while (BlockNo < Map.NumBlocks && Map.Blocks[BlockNo].BlockType != GIF_BLOCK_FT) {

				//  If we have a start of frame (GCE or ID) then process the new frame
				if (Map.Blocks[BlockNo].BlockType == GIF_BLOCK_GCE || Map.Blocks[BlockNo].BlockType == GIF_BLOCK_ID) {
					OS << std::endl;
					FrameNo++;
					OS << "GIF IMAGE FRAME: " << FrameNo << "." << std::endl;

					//  Process the Graphics Control Extension if present
					if (Map.Blocks[BlockNo].BlockType == GIF_BLOCK_GCE) {

						GIF_GRAPHICS_CONTROL_EXTENSION* pGCE = (GIF_GRAPHICS_CONTROL_EXTENSION*) (Map.Image + Offset);

						OS << std::endl;
						OS << "GIF GRAPHICS CONTROL EXTENSION - Block: " << BlockNo << ", Offset: +" << Offset << ", Size: " << Map.Blocks[BlockNo].BlockSize << "." << std::endl;

						//  Dump the memory image
						OS << std::endl;
						MemoryDumper::dumpMemory(Map.Image + Offset, sizeof(GIF_GRAPHICS_CONTROL_EXTENSION), "GCE", OS);
						OS << std::endl;

						//  Show the GCE Contents
						sprintf_s(PrintLine, MAX_PATH, "Bit Fields: \t\t%i (0x%.2x).", int(pGCE->bitsettings), int(pGCE->bitsettings));
						OS << PrintLine << std::endl;
						switch (DisposalMethod(pGCE->bitsettings)) {
						case 0:
							strcpy_s(Disposal, 50, "No disposal specified, take no action.");
							break;
						case 1:
							strcpy_s(Disposal, 50, "Do not dispose, graphic is left in place.");
							break;
						case 2:
							strcpy_s(Disposal, 50, "Restore to background colour.");
							break;
						case 3:
							strcpy_s(Disposal, 50, "Restore to previous image, revert.");
							break;
						default:
							strcpy_s(Disposal, 50, "Unknown disposal method.");
							break;
						}
						sprintf_s(PrintLine, MAX_PATH, "   Disposal Method: \t\t%i - %s", DisposalMethod(pGCE->bitsettings), Disposal);
						OS << PrintLine << std::endl;
						if (pGCE->bitsettings & GIF_GCEBITS_TRANSP) {
							sprintf_s(PrintLine, MAX_PATH, "   Transaprect Colour Index: \t\t%i.", DisposalMethod(pGCE->bitsettings));
							OS << PrintLine << std::endl;
						}
						sprintf_s(PrintLine, MAX_PATH, "Disposal Delay: \t%i 1/100s of a second.", GetSize(pGCE->delay));
						OS << PrintLine << std::endl;

						//  Skip over the block
						Offset += Map.Blocks[BlockNo].BlockSize;
						BlockNo++;
					}

					//  Now positioned at the Image Descriptor (ID)

					GIF_IMAGE_DESCRIPTOR*		pID = (GIF_IMAGE_DESCRIPTOR*)(Map.Image + Offset);

					OS << std::endl;
					OS << "GIF IMAGE DESCRIPTOR - Block: " << BlockNo << ", Offset: +" << Offset << ", Size: " << Map.Blocks[BlockNo].BlockSize << "." << std::endl;

					//  Dump the memory image
					OS << std::endl;
					MemoryDumper::dumpMemory(Map.Image + Offset, sizeof(GIF_IMAGE_DESCRIPTOR), "ID", OS);
					OS << std::endl;

					//  Show the ID Contents
					sprintf_s(PrintLine, MAX_PATH, "Bit Fields: \t%i (0x%.2x).", int(pID->bitsettings), int(pID->bitsettings));
					OS << PrintLine << std::endl;
					if (pID->bitsettings & GIF_IDBITS_LCT) {
						OS << "   0x80 - Image has a Local Colour Table (LCT) with " << LColourTableSize(pID->bitsettings) << " entries." << std::endl;
						if (pID->bitsettings & GIF_IDBITS_SORT) OS << "   0x20 - The Local Colour Table is sorted." << std::endl;
					}
					if (pID->bitsettings & GIF_IDBITS_ILC) OS << "This image will be displayed using a 4 pass interlaced display method." << std::endl;
					sprintf_s(PrintLine, MAX_PATH, "Placement: \t[%i, %i] (Top, Left).", GetSize(pID->top), GetSize(pID->left));
					OS << PrintLine << std::endl;
					sprintf_s(PrintLine, MAX_PATH, "Image Height: \t%i pixels.", GetSize(pID->height));
					OS << PrintLine << std::endl;
					sprintf_s(PrintLine, MAX_PATH, "Image Width: \t%i pixels.", GetSize(pID->width));
					OS << PrintLine << std::endl;

					//  Skip over the block
					Offset += Map.Blocks[BlockNo].BlockSize;
					BlockNo++;

					//  If signalled the we are now positioned at the Local Colour Table (LCT)
					if (pID->bitsettings & GIF_IDBITS_LCT) {
						size_t		CTEnts = 0;

						//  Get the count of entries
						CTEnts = LColourTableSize(pID->bitsettings);
						if (CTEnts > 0) {

							GIF_ODI_COLOUR_TABLE* pLCT = (GIF_ODI_COLOUR_TABLE*) (Map.Image + Offset);

							OS << std::endl;
							OS << "GIF LOCAL COLOUR TABLE - Block: " << BlockNo << ", Offset: +" << Offset << ", Size: " << Map.Blocks[BlockNo].BlockSize << "." << std::endl;

							//  Dump the memory image
							OS << std::endl;
							MemoryDumper::dumpMemory(Map.Image + Offset, CTEnts * sizeof(RGB), "LCT", OS);
							OS << std::endl;

							OS << std::endl;
							OS << "Colour Table Entries: " << CTEnts << "." << std::endl;
							OS << std::endl;

							//  Show each of the entries in the table
							for (size_t CX = 0; CX < CTEnts; CX++) {
								OS << " Entry: " << CX << ", Colour: ";
								pLCT->Entry[CX].document(OS);
								OS << "." << std::endl;
							}
						}

						//  Skip over the block
						Offset += Map.Blocks[BlockNo].BlockSize;
						BlockNo++;
					}

					//  Now positioned at the EEB

					GIF_ENTROPY_ENCODED_BUFFER*		pEEB = (GIF_ENTROPY_ENCODED_BUFFER*)(Map.Image + Offset);

					OS << std::endl;
					OS << "GIF ENTROPY ENCODED IMAGE BUFFER - Block: " << BlockNo << ", Offset: +" << Offset << ", Size: " << Map.Blocks[BlockNo].BlockSize << "." << std::endl;

					//  Dump the memory image
					OS << std::endl;
					size_t DumpLen = 288;
					if (Map.Blocks[BlockNo].BlockSize < DumpLen) DumpLen = Map.Blocks[BlockNo].BlockSize;

					MemoryDumper::dumpMemory(Map.Image + Offset, DumpLen, "EEB", OS);
					OS << std::endl;

					// Show the EEB contents
					sprintf_s(PrintLine, MAX_PATH, "Native Code Size: \t%i.", int(pEEB->nativecodesize));
					OS << PrintLine << std::endl;

					//  Accumulate the Prefix & Data size
					BYTE*	pSOS = (BYTE*) &pEEB->segment;
					GIF_IMAGE_SEGMENT*	pSeg = (GIF_IMAGE_SEGMENT*) pSOS;
					size_t	Segments = 0;
					size_t	Data = 0;

					while (pSeg->size != 0 && ((Segments + Data) < Map.Blocks[BlockNo].BlockSize)) {
						Segments++;
						Data += pSeg->size;
						pSeg = (GIF_IMAGE_SEGMENT*) (pSOS + Segments + Data);
					}

					if (pSeg->size == 0) Segments++;

					sprintf_s(PrintLine, MAX_PATH, "Image Buffer: \t\t%llu Segments, %llu bytes of image data.", (unsigned long long)(Segments), (unsigned long long)(Data));
					OS << PrintLine << std::endl;

					//  Skip over the block
					Offset += Map.Blocks[BlockNo].BlockSize;
					BlockNo++;

				}
				else if (Map.Blocks[BlockNo].BlockType == GIF_BLOCK_AXB) {

					//  Show Application Extension
					OS << std::endl;
					OS << "GIF APPLICATION EXTENSION BLOCK - Block: " << BlockNo << ", Offset: +" << Offset << ", Size: " << Map.Blocks[BlockNo].BlockSize << "." << std::endl;

					//  Dump the memory image
					OS << std::endl;
					MemoryDumper::dumpMemory(Map.Image + Offset, Map.Blocks[BlockNo].BlockSize, "AXB", OS);
					OS << std::endl;

					//  Skip over the block
					Offset += Map.Blocks[BlockNo].BlockSize;
					BlockNo++;
				}
				else if (Map.Blocks[BlockNo].BlockType != GIF_BLOCK_FT) {

					OS << std::endl;
					OS << "ERROR: UNKNOWN BLOCK ENCOUNTERED: - Block: " << BlockNo << ", Offset: +" << Offset << ", Size: " << Map.Blocks[BlockNo].BlockSize << "." << std::endl;

					//  Dump the memory image
					OS << std::endl;
					MemoryDumper::dumpMemory(Map.Image + Offset, 32, "UNKNOWN", OS);
					OS << std::endl;
					
					Offset += Map.Blocks[BlockNo].BlockSize;
					BlockNo++;
				}
			}

			//  Return to caller
			return;
		}

		//  showTrailer
		//
		//  This static function will document the GIF File Trailer
		//
		//  PARAMETERS
		//
		//		ODIMap&			-		Reference to the GIF On-Disk-Image map
		//		std::ostream&	-		Reference to the output stream
		//
		//  RETURNS
		//
		//  NOTES
		//

		static void		showFileTrailer(ODIMap& Map, std::ostream& OS) {
			size_t			BlockNo = 0;													//  Block number for File Header
			size_t			Offset = 0;														//  Offset in the file

			while (Map.Blocks[BlockNo].BlockType != GIF_BLOCK_FT) {
				Offset += Map.Blocks[BlockNo].BlockSize;
				BlockNo++;
			}

			OS << std::endl;
			OS << "GIF FILE TRAILER - Block: " << BlockNo << ", Offset: +" << Offset << ", Size: " << Map.Blocks[BlockNo].BlockSize << "." << std::endl;

			//  Dump the memory image
			OS << std::endl;
			MemoryDumper::dumpMemory(Map.Image + Offset, sizeof(GIF_FILE_TRAILER), "File Trailer", OS);
			OS << std::endl;

			//  Return to caller
			return;
		}

	};
}
