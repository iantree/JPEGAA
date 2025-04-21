#pragma	once
//*******************************************************************************************************************
//*																													*
//*   File:		  JFIF.h																							*
//*   Suite:      xymorg Image Processing - ODI																		*
//*   Version:    1.0.0	  Build:  01																				*
//*   Author:     Ian Tree/HMNL																						*
//*																													*
//*   Copyright 2017 - 2020 Ian J. Tree																				*
//*******************************************************************************************************************
//*	JFIF.h																											*
//*																													*
//*	This header file contains the static functions for handling a JFIF/JPEG On-Disk Image.							*
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
#include	"../../../consts.h"																		//  xymorg constant definitions
#include	"../../../VRMapper.h"																	//  Resource Mapper

//  Additional xymorg components
#include	"../../../MemoryDumper.h"																//  Memory Dump

//  Include xymorg image processing primitives
#include	"../../types.h"																			//  Image processing primitive types
#include	"../../consts.h"																		//  Image processing constants
#include	"../../ColourConverter.h"																//  Colour space conversions
#include	"JFIFODI.h"																				//  JFIF ODI Definitions
#include	"../../Train.h"																			//  Image Train
#include	"../../ColourTable.h"																	//  Colour Table (array)
#include	"../../CODECS/Huffman.h"																//  Base Huffman CODEC

namespace xymorg {

	//
	// JFIF class - contains ONLY static functions
	//

	class JFIF {
	public:

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Public Constants                                                                                              *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  Options for storing images
		static const SWITCHES	JFIF_STORE_OPT_HIFI = 1;										//  High Fidelity Image 1x1 sampling

	private:

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Private Types			                                                                                        *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  Basic Data Unit (DU) Structure
		typedef struct DU {
			int16_t		DC;
			int16_t		AC[63];
		} DU;

		//  Minimum Coding Unit (MCU) structure for a single colour channel
		typedef struct CMCU {
			DU				CDU[4];
		} CMCU;

		//  Minimum Coding Unit (MCU) structure
		typedef struct MCU {
			DU				DUY[4];												//  Y-Channel
			DU				DUCb[4];											//  Cb-Channel
			DU				DUCr[4];											//  Cr-Channel
		} MCU;

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Private Nested Classes                                                                                        *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   JPEGQuantizer Class																							*
		//*                                                                                                                 *
		//*   The JPEG Quantizer class provides functions for quantizing and dequantizing values, it holds the values of	*
		//*   a specific quantization table.																				*
		//*                                                                                                                 *
		//*******************************************************************************************************************

		class JPEGQuantizer {
		public:

			//*******************************************************************************************************************
			//*                                                                                                                 *
			//*   Constructors                                                                                                  *
			//*                                                                                                                 *
			//*******************************************************************************************************************

			//  Normal Constructor
			//
			//  Constructs a new JPEGQuantizer from an 8 or 16 bit quantization table
			//
			//  PARAMETERS
			//
			//		BYTE*		-		Pointer to the Quantization table to load
			//
			//  RETURNS
			//
			//  NOTES
			//
			//

			JPEGQuantizer(const BYTE* pQT) {
				JFIF_QV8_ARRAY*		pQV8 = (JFIF_QV8_ARRAY*) pQT;												//  8 bit value array
				JFIF_QV16_ARRAY*	pQV16 = (JFIF_QV16_ARRAY*) pQT;												//  16 bit value array
				int					vIndex;																		//  Value index

				//  Populate the values from the table
				if (GetPrecision(pQV8->PandD) == 0) {
					//  8-bit precision
					for (vIndex = 0; vIndex < 64; vIndex++) QTable[vIndex] = pQV8->QV[vIndex];
				}
				else {
					//  16-bit precision
					for (vIndex = 0; vIndex < 64; vIndex++) QTable[vIndex] = GetSizeBE(pQV16->QV[vIndex].PV);
				}

				//  Return to caller
				return;
			}

			//*******************************************************************************************************************
			//*                                                                                                                 *
			//*   Destructor                                                                                                    *
			//*                                                                                                                 *
			//*******************************************************************************************************************

			~JPEGQuantizer() {
				//  return to caller
				return;
			}

			//*******************************************************************************************************************
			//*                                                                                                                 *
			//*   Public Functions                                                                                              *
			//*                                                                                                                 *
			//*******************************************************************************************************************

			//  quantize
			//
			//  Quantizes the value of the input DU in-place
			//
			//  PARAMETERS
			//
			//		DU&				-		Reference to the Input Data Unit
			//
			//  RETURNS
			//
			//  NOTES
			//
			//

			void quantize(DU& duIn) const {
				int			tIndex = 0;																//  Index into the quantization table

				duIn.DC = (duIn.DC + (QTable[0] / 2)) / QTable[0];
				for (tIndex = 0; tIndex < 63; tIndex++) duIn.AC[tIndex] = (duIn.AC[tIndex] + (QTable[tIndex] / 2)) / QTable[tIndex];

				//  Return to caller
				return;
			}

			//  quantize
			//
			//  Quantizes the value of the input DU in-flight
			//
			//  PARAMETERS
			//
			//		DU*				-		Pointer to the Input Data Unit
			//
			//  RETURNS
			//
			//		DU				-		Quanized Data Unit
			//
			//  NOTES
			//
			//

			DU quantize(DU* pDUIn) const {
				int			tIndex = 0;															//  Index into the quantization table
				DU			QDU = {};															//  Quantized Data Unit

				QDU.DC = (pDUIn->DC + (QTable[0] / 2)) / QTable[0];
				for (tIndex = 0; tIndex < 63; tIndex++) QDU.AC[tIndex] = (pDUIn->AC[tIndex] + (QTable[tIndex] / 2)) / QTable[tIndex];

				//  Return the quantized Data Unit
				return QDU;
			}

			//  dequantize
			//
			//  Quantizes the value of the input DU in-place
			//
			//  PARAMETERS
			//
			//		DU&				-		Reference to the Input Data Unit
			//
			//  RETURNS
			//
			//  NOTES
			//
			//

			void dequantize(DU& duIn) const {
				int			tIndex = 0;															//  Index into the quantization table

				duIn.DC = duIn.DC * QTable[0];
				for (tIndex = 0; tIndex < 63; tIndex++) duIn.AC[tIndex] = duIn.AC[tIndex] * QTable[tIndex];

				//  Return to caller
				return;
			}

			//  dequantize
			//
			//  Quantizes the value of the input DU in-flight
			//
			//  PARAMETERS
			//
			//		DU*				-		Pointer to the Input Data Unit
			//
			//  RETURNS
			//
			//		DU				-		Dequanized Data Unit
			//
			//  NOTES
			//
			//

			DU dequantize(DU* pDUIn) const {
				int			tIndex = 0;																//  Index into the quantization table
				DU			DQDU = {};																//  Dequantized Data Unit

				DQDU.DC = pDUIn->DC * QTable[0];
				for (tIndex = 0; tIndex < 63; tIndex++) DQDU.AC[tIndex] = pDUIn->AC[tIndex] * QTable[tIndex];

				//  Return the dequantized Data Unit
				return DQDU;
			}

			//  serialize
			//
			//  Serializes the quanisation table in JPEG format, including the leading Precision and Destination encoding BYTE
			//
			//  PARAMETERS
			//
			//		BYTE			-		Precision and Destination Byte
			//		size_t&			-		Reference to the variable to hold the size of the table
			//
			//  RETURNS
			//
			//		BYTE*			-		Pointer to a buffer holding the table definition
			//
			//  NOTES
			//
			//

			BYTE* serialize(BYTE PND, size_t& STSize) const {
				BYTE*		pDT = nullptr;														//  Pointer to the table definition
				BYTE*		pTE = nullptr;														//  Pointer to the next table entry

				//  Determine the allocation size of the table
				if (GetPrecision(PND) == 0) STSize = size_t(1) + (size_t(64) * size_t(1));
				else STSize = size_t(1) + (size_t(64) * size_t(2));

				//  Allocate space for the table
				pDT = (BYTE*) malloc(STSize);
				if (pDT == nullptr) return nullptr;

				//  Set the precision and destination byte
				*pDT = PND;

				//  Setup the table entry pointer
				pTE = pDT + 1;

				//  Iterate over the table expressing it to the serialised image
				for (int QX = 0; QX < 64; QX++) {
					if (GetPrecision(PND) == 0) {
						*pTE = BYTE(QTable[QX]);
						pTE++;
					}
					else {
						SetSizeBE(pTE, QTable[QX]);
						pTE += 2;
					}
				}

				//  Return the table pointer to the caller
				return pDT;
			}

		private:

			//*******************************************************************************************************************
			//*																													*
			//*  Private Members																								*
			//*																													*
			//*******************************************************************************************************************

			uint16_t			QTable[64];														//  Quantization table values
		};

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   DecoderPipeline Class																							*
		//*                                                                                                                 *
		//*   The DecoderPipeline class is the main engine for decoding JPEG encoded image data.							*
		//*   It links together and synchronises the operation of a chain of components to provide the complete decoding	*
		//*   mechanics.																									*
		//*                                                                                                                 *
		//*******************************************************************************************************************

		class DecoderPipeline {

			//*******************************************************************************************************************
			//*                                                                                                                 *
			//*   Forward Declarations                                                                                          *
			//*                                                                                                                 *
			//*******************************************************************************************************************

		private:

			//*******************************************************************************************************************
			//*                                                                                                                 *
			//*   Private Nested Classes																						*
			//*   Pipeline Processing Components																				*
			//*                                                                                                                 *
			//*******************************************************************************************************************

			//*******************************************************************************************************************
			//*                                                                                                                 *
			//*   DUBuilder Class																								*
			//*                                                                                                                 *
			//*   The DUBuilder class builds a fully formed Data Unit (DU) from the next 64 codes obtained from the entropy		*
			//*   encoded stream.																								*
			//*                                                                                                                 *
			//*                                                                                                                 *
			//*******************************************************************************************************************

			class DUBuilder {
			public:

				//*******************************************************************************************************************
				//*                                                                                                                 *
				//*   Constructors                                                                                                  *
				//*                                                                                                                 *
				//*******************************************************************************************************************

				//  Default Constructor

				DUBuilder() {
					//  Clear the DU
					memset(&NewDU, 0, sizeof(DU));

					//  Clear the tables
					DCTree = nullptr;
					ACTree = nullptr;
					Input = nullptr;

					//  Clear the previous DC values
					PreviousDC[0] = 0;
					PreviousDC[1] = 0;
					PreviousDC[2] = 0;

					BitsRead[0] = 0;
					BitsRead[1] = 0;
					BitsRead[2] = 0;
					DUCount = 0;

					//  Clear controls
					EndOfUnit = false;
					SkipZeros = 0;

					//  Return to caller
					return;
				}

				//*******************************************************************************************************************
				//*                                                                                                                 *
				//*   Public Functions                                                                                              *
				//*                                                                                                                 *
				//*******************************************************************************************************************

				//  hasNext
				//
				//  Determines if more data is available from the pipeline
				//
				//  PARAMETERS:
				//
				//  RETURNS:
				//
				//      bool				-		true if more data is available, otherwise false
				//
				//	NOTES:
				// 
				//

				bool hasNext() {
					return !Input->eos();
				}

				//  nextDU
				//
				//  Retrieves the decoded DU from the entropy encoded stream, applies the differential coding to the DC value
				//  and returns the completed DU.
				//
				//  PARAMETERS:
				//
				//		int				-		Channel Number being read
				//
				//  RETURNS:
				//
				//      DU				-		The completed Data Unit (DU)
				//
				//	NOTES:
				// 
				//  The caller must have set the DC & AC HuffmanTree addreses for he current channel prior to making this call.
				//

				DU nextDU(int Channel) {
					int			acIndex;															//  Index into the AC array

					//  Clear the DU
					memset(&NewDU, 0, sizeof(DU));

					//  Set the DC Tree in the Huffman CODEC
					Input->setTree(DCTree);

					//  Read the next value from the Input - if none is available this is a premature termination of the input stream
					//
					if (Input->hasNext(true)) {
						PreviousDC[Channel] += Input->next(true);
						NewDU.DC = PreviousDC[Channel];
					}
					else {
						memset(&NewDU, 0, sizeof(DU));
						std::cerr << "ERROR: The Input stream has termnated prematurely. No more input will be available." << std::endl;
						return NewDU;
					}

					//  Set the AC Tree in the Huffman CODEC
					Input->setTree(ACTree);

					//  Read the next 63 AC values into the DU
					for (acIndex = 0; acIndex < 63; acIndex++) {
						if (Input->hasNext(false)) {
							NewDU.AC[acIndex] = Input->next(false);
						}
						else {
							memset(&NewDU, 0, sizeof(DU));
							std::cerr << "ERROR: The Input stream has termnated prematurely. No more input will be available." << std::endl;
							return NewDU;
						}
					}

					//  Return the completed DU
					DUCount++;
					return NewDU;
				}

				//  Configuration Functions

				void setDCHuffmanTree(Huffman::HuffmanTree* NewDCTree) { DCTree = NewDCTree; DCTree->setCurrentNode(nullptr); return; }
				void setACHuffmanTree(Huffman::HuffmanTree* NewACTree) { ACTree = NewACTree; ACTree->setCurrentNode(nullptr); return; }
				void setInput(Huffman::JPEGEmitter* NewEmitter) { Input = NewEmitter; return; }

			private:

				//*******************************************************************************************************************
				//*																													*
				//*  Private Members																								*
				//*																													*
				//*******************************************************************************************************************

				//  Data Unit
				DU								NewDU;

				//  Persistent Data
				uint16_t						PreviousDC[3];										//  Previous DC values (per channel)

				//  Input Object
				Huffman::JPEGEmitter*			Input;												//  Source emitter

				//  DC & AC HuffmanTree objects
				Huffman::HuffmanTree*			DCTree;												//  DC Huffman Tree
				Huffman::HuffmanTree*			ACTree;												//  AC Huffman Tree

				//  Controls
				bool							EndOfUnit;											//  End of unit detected
				uint16_t						SkipZeros;											//  Number of zero values to skip

				//  Instrumentation
				size_t							BitsRead[3];										//  Bits read on the designated channel
				size_t							DUCount;											//  Count of DUs emitted

			};

			//*******************************************************************************************************************
			//*                                                                                                                 *
			//*   Dequantizer Class																								*
			//*                                                                                                                 *
			//*   The Dequantizer class reads the next DU from the DUBuilder and dequantizes the entries in the DU.				*
			//*                                                                                                                 *
			//*                                                                                                                 *
			//*******************************************************************************************************************

			class Dequantizer {
			public:

				//*******************************************************************************************************************
				//*                                                                                                                 *
				//*   Constructors                                                                                                  *
				//*                                                                                                                 *
				//*******************************************************************************************************************

				//  Default Constructor

				Dequantizer() {
					//  Clear the DU
					memset(&NewDU, 0, sizeof(DU));

					//  Clear the tables
					DCTree = nullptr;
					ACTree = nullptr;
					Q = nullptr;
					Input = nullptr;

					//  Return to caller
					return;
				}

				//*******************************************************************************************************************
				//*                                                                                                                 *
				//*   Public Functions                                                                                              *
				//*                                                                                                                 *
				//*******************************************************************************************************************

				//  hasNext
				//
				//  Determines if more data is available from the pipeline
				//
				//  PARAMETERS:
				//
				//  RETURNS:
				//
				//      bool				-		true if more data is available, otherwise false
				//
				//	NOTES:
				// 
				//

				bool hasNext() {
					return Input->hasNext();
				}

				//  nextDU
				//
				//  Retrieves the decoded DU from the entropy encoded stream, applies the differential coding to the DC value
				//  and returns the completed DU.
				//
				//  PARAMETERS:
				//
				//		int				-		Channel Number being read
				//
				//  RETURNS:
				//
				//      DU				-		The completed Data Unit (DU)
				//
				//	NOTES:
				// 
				//  The caller must have set the DC & AC HuffmanTree and JPEGQuantizer addreses for he current channel prior to making this call.
				//

				DU nextDU(int Channel) {

					//  Clear the DU
					memset(&NewDU, 0, sizeof(DU));

					//  Set the DC and AC Tree in the DU Builder
					Input->setDCHuffmanTree(DCTree);
					Input->setACHuffmanTree(ACTree);

					//  Read the Next DU from the Builder
					NewDU = Input->nextDU(Channel);

					//  Dequantize all entries in the DU
					Q->dequantize(NewDU);

					//  Return the Dequantized DU
					return NewDU;
				}

				//  Configuration Functions

				void setDCHuffmanTree(Huffman::HuffmanTree* NewDCTree) { DCTree = NewDCTree; return; }
				void setACHuffmanTree(Huffman::HuffmanTree* NewACTree) { ACTree = NewACTree; return; }
				void setQuantizer(JPEGQuantizer* NewQ) { Q = NewQ; return; }
				void setInput(DUBuilder* NewBuilder) { Input = NewBuilder; return; }

			private:

				//*******************************************************************************************************************
				//*																													*
				//*  Private Members																								*
				//*																													*
				//*******************************************************************************************************************

				//  Data Unit
				DU							NewDU;

				//  Input Object
				DUBuilder*					Input;												//  Source emitter

				//  DC & AC HuffmanTree & JPEGQuantizer objects
				Huffman::HuffmanTree*		DCTree;												//  DC Huffman Tree
				Huffman::HuffmanTree*		ACTree;												//  AC Huffman Tree
				JPEGQuantizer*				Q;													//  Quantizer

			};

			//*******************************************************************************************************************
			//*                                                                                                                 *
			//*   Unzigzagger Class																								*
			//*                                                                                                                 *
			//*   The Unzigzagger class reads the next available DU from the Dequantizer and undoes the Xig-Zag staggered		*
			//*   order of coefficients in the DU.																				*
			//*                                                                                                                 *
			//*                                                                                                                 *
			//*******************************************************************************************************************

			class Unzigzagger {
			public:

				//*******************************************************************************************************************
				//*                                                                                                                 *
				//*   Constructors                                                                                                  *
				//*                                                                                                                 *
				//*******************************************************************************************************************

				//  Default Constructor

				Unzigzagger() {
					//  Clear the DU
					memset(&NewDU, 0, sizeof(DU));

					//  Clear the tables
					DCTree = nullptr;
					ACTree = nullptr;
					Q = nullptr;
					Input = nullptr;

					//  Return to caller
					return;
				}

				//*******************************************************************************************************************
				//*                                                                                                                 *
				//*   Public Functions                                                                                              *
				//*                                                                                                                 *
				//*******************************************************************************************************************

				//  hasNext
				//
				//  Determines if more data is available from the pipeline
				//
				//  PARAMETERS:
				//
				//  RETURNS:
				//
				//      bool				-		true if more data is available, otherwise false
				//
				//	NOTES:
				// 
				//

				bool hasNext() {
					return Input->hasNext();
				}

				//  nextDU
				//
				//  Retrieves the decoded DU from the entropy encoded stream, applies the differential coding to the DC value
				//  and returns the completed DU.
				//
				//  PARAMETERS:
				//
				//		int				-		Channel Number being read
				//
				//  RETURNS:
				//
				//      DU				-		The completed Data Unit (DU)
				//
				//	NOTES:
				// 
				//  The caller must have set the DC & AC HuffmanTree and JPEGQuantizer addreses for he current channel prior to making this call.
				//

				DU nextDU(int Channel) {
					uint16_t			cIndex;
					uint16_t			UZZ[64] = { 0, 1, 5, 6, 14, 15, 27, 28,
											2, 4, 7, 13, 16, 26, 29, 42,
											3, 8, 12, 17, 25, 30, 41, 43,
											9, 11, 18, 24, 31, 40, 44, 53,
											10, 19, 23, 32, 39, 45, 52, 54,
											20, 22, 33, 38, 46, 51, 55, 60,
											21, 34, 37, 47, 50, 56, 59, 61,
											35, 36, 48, 49, 57, 58, 62, 63 };

					//  Clear the DU
					memset(&NewDU, 0, sizeof(DU));

					//  Set the DC and AC Tree in the DU Builder
					Input->setDCHuffmanTree(DCTree);
					Input->setACHuffmanTree(ACTree);
					Input->setQuantizer(Q);

					//  Read the Next DU from the Builder
					NewDU = Input->nextDU(Channel);

					//  Perform the UnZigZag operation
					for (cIndex = 1; cIndex < 64; cIndex++) UZZ[cIndex] = NewDU.AC[UZZ[cIndex] - 1];
					for (cIndex = 1; cIndex < 64; cIndex++) NewDU.AC[cIndex - 1] = UZZ[cIndex];

					//  Return the Dequantized DU
					return NewDU;
				}

				//  Configuration Functions

				void setDCHuffmanTree(Huffman::HuffmanTree* NewDCTree) { DCTree = NewDCTree; return; }
				void setACHuffmanTree(Huffman::HuffmanTree* NewACTree) { ACTree = NewACTree; return; }
				void setQuantizer(JPEGQuantizer* NewQ) { Q = NewQ; return; }
				void setInput(Dequantizer* NewDQ) { Input = NewDQ; return; }

			private:

				//*******************************************************************************************************************
				//*																													*
				//*  Private Members																								*
				//*																													*
				//*******************************************************************************************************************

				//  Data Unit
				DU							NewDU;

				//  Input Object
				Dequantizer*				Input;												//  Source emitter

				//  DC & AC HuffmanTree & JPEGQuantizer objects
				Huffman::HuffmanTree*		DCTree;												//  DC Huffman Tree
				Huffman::HuffmanTree*		ACTree;												//  AC Huffman Tree
				JPEGQuantizer*				Q;													//  Quantizer

			};

			//*******************************************************************************************************************
			//*                                                                                                                 *
			//*   DCTInverter Class																								*
			//*                                                                                                                 *
			//*   The DCTInverter class reads the next available DU from the Unzigzagger and applies the inverse DCT			*
			//*   (Discrete Cosine Transform) transform to the DU.																*
			//*                                                                                                                 *
			//*                                                                                                                 *
			//*******************************************************************************************************************

			class DCTInverter {
			public:

				//*******************************************************************************************************************
				//*                                                                                                                 *
				//*   Constructors                                                                                                  *
				//*                                                                                                                 *
				//*******************************************************************************************************************

				//  Default Constructor

				DCTInverter() {
					//  Clear the DU
					memset(&NewDU, 0, sizeof(DU));

					//  Clear the tables
					DCTree = nullptr;
					ACTree = nullptr;
					Q = nullptr;
					Input = nullptr;

					//  Return to caller
					return;
				}

				//*******************************************************************************************************************
				//*                                                                                                                 *
				//*   Public Functions                                                                                              *
				//*                                                                                                                 *
				//*******************************************************************************************************************

				//  hasNext
				//
				//  Determines if more data is available from the pipeline
				//
				//  PARAMETERS:
				//
				//  RETURNS:
				//
				//      bool				-		true if more data is available, otherwise false
				//
				//	NOTES:
				// 
				//

				bool hasNext() {
					return Input->hasNext();
				}

				//  nextDU
				//
				//  Retrieves the decoded DU from the entropy encoded stream, applies the differential coding to the DC value
				//  and returns the completed DU.
				//
				//  PARAMETERS:
				//
				//		int				-		Channel Number being read
				//
				//  RETURNS:
				//
				//      DU				-		The completed Data Unit (DU)
				//
				//	NOTES:
				// 
				//  The caller must have set the DC & AC HuffmanTree and JPEGQuantizer addreses for he current channel prior to making this call.
				//

				DU nextDU(int Channel) {
					DU			InputDU;
					uint16_t	yIndex, xIndex, uIndex, vIndex;																		//  Cell indexes
					double		Coefficient, Sample, Element, Accumulator;															//  Working calculation variables

					//  Clear the DU
					memset(&NewDU, 0, sizeof(DU));

					//  Set the DC and AC Tree in the DU Builder
					Input->setDCHuffmanTree(DCTree);
					Input->setACHuffmanTree(ACTree);
					Input->setQuantizer(Q);

					//  Read the Next DU from the Builder
					InputDU = Input->nextDU(Channel);

					//
					//  Apply the Inverse DCT transform
					//
					//  Convert each coefficient in the Data unit in turn
					for (yIndex = 0; yIndex < 8; yIndex++)
					{
						for (xIndex = 0; xIndex < 8; xIndex++)
						{
							//  Perform the inverse DCT transform
							Accumulator = 0.0;
							for (uIndex = 0; uIndex < 8; uIndex++)
							{
								for (vIndex = 0; vIndex < 8; vIndex++)
								{
									if (uIndex == 0) Element = 1.0 / sqrt(2);
									else Element = 1.0;
									if (vIndex == 0) Element = Element * (1.0 / sqrt(2));

									if (uIndex == 0 && vIndex == 0) Coefficient = double(InputDU.DC);
									else Coefficient = double(InputDU.AC[((vIndex * 8) + uIndex) - 1]);

									Element = Element * Coefficient;
									Element = Element * cos(((((2.0 * double(xIndex)) + 1.0) * double(uIndex)) * Pi) / 16.0);
									Element = Element * cos(((((2.0 * double(yIndex)) + 1.0) * double(vIndex)) * Pi) / 16.0);
									Accumulator += Element;
								}
							}

							Sample = Accumulator / 4.0;

							//  Output the result
							if (yIndex == 0 && xIndex == 0) NewDU.DC = uint16_t(Sample);
							else NewDU.AC[((yIndex * 8) + xIndex) - 1] = uint16_t(Sample);
						}
					}

					//  Return the DU
					return NewDU;
				}

				//  Configuration Functions

				void setDCHuffmanTree(Huffman::HuffmanTree* NewDCTree) { DCTree = NewDCTree; return; }
				void setACHuffmanTree(Huffman::HuffmanTree* NewACTree) { ACTree = NewACTree; return; }
				void setQuantizer(JPEGQuantizer* NewQ) { Q = NewQ; return; }
				void setInput(Unzigzagger* NewUZZ) { Input = NewUZZ; return; }

			private:

				//*******************************************************************************************************************
				//*																													*
				//*  Private Members																								*
				//*																													*
				//*******************************************************************************************************************

				//  Data Unit
				DU								NewDU;

				//  Input Object
				Unzigzagger*					Input;												//  Source emitter

				//  DC & AC HuffmanTree & JPEGQuantizer objects
				Huffman::HuffmanTree*			DCTree;												//  DC Huffman Tree
				Huffman::HuffmanTree*			ACTree;												//  AC Huffman Tree
				JPEGQuantizer*					Q;													//  Quantizer

			};

			//*******************************************************************************************************************
			//*                                                                                                                 *
			//*   Upshifter Class																								*
			//*                                                                                                                 *
			//*   The Upshifter class reads the next available DU from the DCTInverrter and applies the up shift to the DU		*
			//*   Coefficients, which are now Y, Cb or Cr channel values.														*
			//*                                                                                                                 *
			//*                                                                                                                 *
			//*******************************************************************************************************************

			class Upshifter {
			public:

				//*******************************************************************************************************************
				//*                                                                                                                 *
				//*   Constructors                                                                                                  *
				//*                                                                                                                 *
				//*******************************************************************************************************************

				//  Default Constructor

				Upshifter() {
					//  Clear the DU
					memset(&NewDU, 0, sizeof(DU));

					//  Clear the tables
					DCTree = nullptr;
					ACTree = nullptr;
					Q = nullptr;
					Input = nullptr;
					Precision = 0;

					DURead[0] = 0;
					DURead[1] = 0;
					DURead[2] = 0;

					//  Return to caller
					return;
				}

				//*******************************************************************************************************************
				//*                                                                                                                 *
				//*   Public Functions                                                                                              *
				//*                                                                                                                 *
				//*******************************************************************************************************************

				//  hasNext
				//
				//  Determines if more data is available from the pipeline
				//
				//  PARAMETERS:
				//
				//  RETURNS:
				//
				//      bool				-		true if more data is available, otherwise false
				//
				//	NOTES:
				// 
				//

				bool hasNext() {
					return Input->hasNext();
				}

				//  nextDU
				//
				//  Retrieves the decoded DU from the entropy encoded stream, applies the differential coding to the DC value
				//  and returns the completed DU.
				//
				//  PARAMETERS:
				//
				//		int				-		Channel Number being read
				//
				//  RETURNS:
				//
				//      DU				-		The completed Data Unit (DU)
				//
				//	NOTES:
				// 
				//  The caller must have set the DC & AC HuffmanTree and JPEGQuantizer addreses for he current channel prior to making this call.
				//

				DU nextDU(int Channel) {
					uint16_t			acIndex;
					uint16_t			Shift = 1 << (Precision - 1);																//  Shift level

					//  Clear the DU
					memset(&NewDU, 0, sizeof(DU));

					//  Set the DC and AC Tree in the DU Builder
					Input->setDCHuffmanTree(DCTree);
					Input->setACHuffmanTree(ACTree);
					Input->setQuantizer(Q);

					//  Read the next DU from the pipeline
					NewDU = Input->nextDU(Channel);
					DURead[Channel]++;

					//  Shift and clamp the DC
					NewDU.DC += Shift;
					if (NewDU.DC < 0) NewDU.DC = 0;
					if (NewDU.DC > 255) NewDU.DC = 255;

					//  Shift and clamp the AC coefficients
					for (acIndex = 0; acIndex < 63; acIndex++) {
						NewDU.AC[acIndex] += Shift;
						if (NewDU.AC[acIndex] < 0) NewDU.AC[acIndex] = 0;
						if (NewDU.AC[acIndex] > 255) NewDU.AC[acIndex] = 255;
					}

					//  Return the updated DU
					return NewDU;
				}

				//  Configuration Functions

				void setDCHuffmanTree(Huffman::HuffmanTree* NewDCTree) { DCTree = NewDCTree; return; }
				void setACHuffmanTree(Huffman::HuffmanTree* NewACTree) { ACTree = NewACTree; return; }
				void setQuantizer(JPEGQuantizer* NewQ) { Q = NewQ; return; }
				void setInput(DCTInverter* NewInverter) { Input = NewInverter; return; }
				void setPrecision(int NewPrecision) { Precision = NewPrecision; return; }

			private:

				//*******************************************************************************************************************
				//*																													*
				//*  Private Members																								*
				//*																													*
				//*******************************************************************************************************************

				//  Data Unit
				DU								NewDU;

				//  Input Object
				DCTInverter*					Input;												//  Source emitter

				//  DC & AC HuffmanTree & JPEGQuantizer objects
				Huffman::HuffmanTree*			DCTree;												//  DC Huffman Tree
				Huffman::HuffmanTree*			ACTree;												//  AC Huffman Tree
				JPEGQuantizer*					Q;													//  Quantizer

				//  Precision
				int								Precision;											//  Precision (current frame)

				//  Instrumentation
				size_t							DURead[3];

			};

			//*******************************************************************************************************************
			//*                                                                                                                 *
			//*   CMCUBuilder Class																								*
			//*                                                                                                                 *
			//*   The CMCUBuilder class A complete CMCU from the DUs obtained from the pipeline.								*
			//*                                                                                                                 *
			//*                                                                                                                 *
			//*******************************************************************************************************************

			class CMCUBuilder {
			public:

				//*******************************************************************************************************************
				//*                                                                                                                 *
				//*   Local Definitions		                                                                                        *
				//*                                                                                                                 *
				//*******************************************************************************************************************

				//  SDU structure overlays a DU providing a normalised spatial access to the coefficients
				typedef struct SDU {
					uint16_t			C[64];														//  Coefficients
				} SDU;

				//*******************************************************************************************************************
				//*                                                                                                                 *
				//*   Constructors                                                                                                  *
				//*                                                                                                                 *
				//*******************************************************************************************************************

				//  Default Constructor

				CMCUBuilder() {
					//  Clear the DU
					memset(&NewCMCU, 0, sizeof(CMCU));
					MCUFF = 0x22;

					//  Clear the tables
					DCTree = nullptr;
					ACTree = nullptr;
					Q = nullptr;
					Input = nullptr;

					//  Return to caller
					return;
				}

				//*******************************************************************************************************************
				//*                                                                                                                 *
				//*   Public Functions                                                                                              *
				//*                                                                                                                 *
				//*******************************************************************************************************************

				//  hasNext
				//
				//  Determines if more data is available from the pipeline
				//
				//  PARAMETERS:
				//
				//  RETURNS:
				//
				//      bool				-		true if more data is available, otherwise false
				//
				//	NOTES:
				// 
				//

				bool hasNext() {
					return Input->hasNext();
				}

				//  nextCMCU
				//
				//  Retrieves the decoded DUs from the pipeline and populates then returnd the MCU
				//
				//  PARAMETERS:
				//
				//		int				-		Channel Number being read
				//		int				-		Horizontal Sampling Factor
				//		int				-		Vertical Sampling factor
				//
				//  RETURNS:
				//
				//      CMCU				-		The completed Sincle Channel Minimum Coding Unit (CMCU)
				//
				//	NOTES:
				// 
				//  The caller must have set the DC & AC HuffmanTree and JPEGQuantizer addreses for he current channel prior to making this call.
				//

				CMCU nextCMCU(int Channel, int HSF, int VSF) {

					//  Clear the MCU
					memset(&NewCMCU, 0, sizeof(CMCU));

					//  Set the DC and AC Tree in the DU Builder
					Input->setDCHuffmanTree(DCTree);
					Input->setACHuffmanTree(ACTree);
					Input->setQuantizer(Q);

					//  Read or copy each DU in turn
					//  Start with the top left DU, this is always read from the pipeline
					NewCMCU.CDU[0] = Input->nextDU(Channel);

					//  If the horizontal sampling factor is 2 then read the next horizontal otherwise upsample the previous entry
					if (HSF == 2) NewCMCU.CDU[1] = Input->nextDU(Channel);
					else upsampleHorizontal(0, 1);

					//  If the Vertical sampling factor is 2 then read the next vertical otherwise copy it
					if (VSF == 2) {
						NewCMCU.CDU[2] = Input->nextDU(Channel);
						if (HSF == 2) NewCMCU.CDU[3] = Input->nextDU(Channel);
						else upsampleHorizontal(2, 3);
					}
					else {
						upsampleVertical(0, 2);
						upsampleVertical(1, 3);
					}

					//  Return the assembled CMCU
					return NewCMCU;
				}

				//  Configuration Functions

				void setDCHuffmanTree(Huffman::HuffmanTree* NewDCTree) { DCTree = NewDCTree; return; }
				void setACHuffmanTree(Huffman::HuffmanTree* NewACTree) { ACTree = NewACTree; return; }
				void setQuantizer(JPEGQuantizer* NewQ) { Q = NewQ; return; }
				void setInput(Upshifter* NewShifter) { Input = NewShifter; return; }
				void setMCUFF(BYTE NewFormFactor) { MCUFF = NewFormFactor; }

			private:

				//*******************************************************************************************************************
				//*																													*
				//*  Private Members																								*
				//*																													*
				//*******************************************************************************************************************

				//  Minimum Coding Unit (CMCU) - Single Channel
				CMCU							NewCMCU;
				BYTE							MCUFF;												//  MCU Form Factor

				//  Input Object
				Upshifter*						Input;												//  Source emitter

				//  DC & AC HuffmanTree & JPEGQuantizer objects
				Huffman::HuffmanTree*			DCTree;												//  DC Huffman Tree
				Huffman::HuffmanTree*			ACTree;												//  AC Huffman Tree
				JPEGQuantizer*					Q;													//  Quantizer

				//*******************************************************************************************************************
				//*                                                                                                                 *
				//*   Private Functions                                                                                             *
				//*                                                                                                                 *
				//*******************************************************************************************************************

				//  upsampleHorizontal
				//
				//  Upsamples horizontal direction - expands one DU horizontally ont two
				//
				//  PARAMETES
				//
				//		USHORT				-			Index of the source DU within the CMCU
				//		USHORT				-			Index of the target DU within the CMCU
				//
				//	RETURNS
				//
				//	NOTES
				//

				void upsampleHorizontal(USHORT sX, USHORT tX) {
					SDU* pSrc = (SDU*)&NewCMCU.CDU[sX];
					SDU* pDst = (SDU*)&NewCMCU.CDU[tX];
					short			rIndex, cIndex;														//  Row & coloumn indexes

					for (rIndex = 0; rIndex < 8; rIndex++) {
						for (cIndex = 7; cIndex >= 0; cIndex--) {
							pDst->C[(rIndex * 8) + cIndex] = pSrc->C[(rIndex * 8) + 4 + (cIndex / 2)];
						}

						for (cIndex = 7; cIndex >= 1; cIndex--) {
							pSrc->C[(rIndex * 8) + cIndex] = pSrc->C[(rIndex * 8) + (cIndex / 2)];
						}
					}

					//  Return to caller
					return;
				}

				//  upsampleVertical
				//
				//  Upsamples vertical direction - expands one DU vertically ont two
				//
				//  PARAMETES
				//
				//		USHORT				-			Index of the source DU within the CMCU
				//		USHORT				-			Index of the target DU within the CMCU
				//
				//	RETURNS
				//
				//	NOTES
				//

				void upsampleVertical(USHORT sX, USHORT tX) {
					SDU* pSrc = (SDU*)&NewCMCU.CDU[sX];
					SDU* pDst = (SDU*)&NewCMCU.CDU[tX];
					short			rIndex, cIndex;														//  Row & coloumn indexes

					for (cIndex = 0; cIndex < 8; cIndex++) {
						for (rIndex = 7; rIndex >= 0; rIndex--) {
							pDst->C[(rIndex * 8) + cIndex] = pSrc->C[(((rIndex / 2) + 4) * 8) + cIndex];
						}

						for (rIndex = 7; rIndex >= 1; rIndex--) {
							pSrc->C[(rIndex * 8) + cIndex] = pSrc->C[((rIndex / 2) * 8) + cIndex];
						}
					}

					//  Return to caller
					return;
				}

			};

			//*******************************************************************************************************************
			//*                                                                                                                 *
			//*   MCUBuilder Class																								*
			//*                                                                                                                 *
			//*   The MCUBuilder class complete MCU from the CMCUs obtained from the pipeline.									*
			//*                                                                                                                 *
			//*                                                                                                                 *
			//*******************************************************************************************************************

			class MCUBuilder {
			public:

				//*******************************************************************************************************************
				//*                                                                                                                 *
				//*   Constructors                                                                                                  *
				//*                                                                                                                 *
				//*******************************************************************************************************************

				//  Default Constructor

				MCUBuilder() {
					//  Clear the MCU
					memset(&NewMCU, 0, sizeof(MCU));
					MCUFF = 0x22;

					//  Clear the tables
					DCTree[0] = nullptr;
					DCTree[1] = nullptr;
					DCTree[1] = nullptr;
					ACTree[0] = nullptr;
					ACTree[1] = nullptr;
					ACTree[2] = nullptr;
					Q[0] = nullptr;
					Q[1] = nullptr;
					Q[2] = nullptr;
					Input = nullptr;

					YRead = 0;
					CbRead = 0;
					CrRead = 0;

					HSF[0] = 0;
					HSF[1] = 0;
					HSF[2] = 0;

					VSF[0] = 0;
					VSF[0] = 0;
					VSF[0] = 0;

					//  Return to caller
					return;
				}

				//*******************************************************************************************************************
				//*                                                                                                                 *
				//*   Public Functions                                                                                              *
				//*                                                                                                                 *
				//*******************************************************************************************************************

				//  hasNext
				//
				//  Determines if more data is available from the pipeline
				//
				//  PARAMETERS:
				//
				//  RETURNS:
				//
				//      bool				-		true if more data is available, otherwise false
				//
				//	NOTES:
				// 
				//

				bool hasNext() {
					return Input->hasNext();
				}

				//  nextMCU
				//
				//  Retrieves the decoded DUs from the pipeline and populates then returnd the MCU
				//
				//  PARAMETERS:
				//
				//  RETURNS:
				//
				//      MCU				-		The completed Minimum Coding Unit (MCU)
				//
				//	NOTES:
				// 
				//  The caller must have set the DC & AC HuffmanTree and JPEGQuantizer addreses for all channels prior to making this call.
				//

				MCU nextMCU() {
					CMCU			ChanMCU;

					//  Clear the MCU
					memset(&NewMCU, 0, sizeof(MCU));

					//  Read the Y Channel MCU

					//  Set the DC and AC Tree in the CMCU Builder
					Input->setDCHuffmanTree(DCTree[0]);
					Input->setACHuffmanTree(ACTree[0]);
					Input->setQuantizer(Q[0]);

					//  Read the CMCU
					ChanMCU = Input->nextCMCU(0, HSF[0], VSF[0]);
					YRead++;

					//  Populate the MCU
					NewMCU.DUY[0] = ChanMCU.CDU[0];
					NewMCU.DUY[1] = ChanMCU.CDU[1];
					NewMCU.DUY[2] = ChanMCU.CDU[2];
					NewMCU.DUY[3] = ChanMCU.CDU[3];

					//  Read the Cb Channel MCU

					//  Set the DC and AC Tree in the CMCU Builder
					Input->setDCHuffmanTree(DCTree[1]);
					Input->setACHuffmanTree(ACTree[1]);
					Input->setQuantizer(Q[1]);

					//  Read the CMCU
					ChanMCU = Input->nextCMCU(1, HSF[1], VSF[1]);
					CbRead++;

					//  Populate the MCU
					NewMCU.DUCb[0] = ChanMCU.CDU[0];
					NewMCU.DUCb[1] = ChanMCU.CDU[1];
					NewMCU.DUCb[2] = ChanMCU.CDU[2];
					NewMCU.DUCb[3] = ChanMCU.CDU[3];

					//  Read the Cr Channel MCU

					//  Set the DC and AC Tree in the CMCU Builder
					Input->setDCHuffmanTree(DCTree[2]);
					Input->setACHuffmanTree(ACTree[2]);
					Input->setQuantizer(Q[2]);

					//  Read the CMCU
					ChanMCU = Input->nextCMCU(2, HSF[2], VSF[2]);
					CrRead++;

					//  Populate the MCU
					NewMCU.DUCr[0] = ChanMCU.CDU[0];
					NewMCU.DUCr[1] = ChanMCU.CDU[1];
					NewMCU.DUCr[2] = ChanMCU.CDU[2];
					NewMCU.DUCr[3] = ChanMCU.CDU[3];

					//  Return the assembled MCU to the caller
					return NewMCU;
				}

				//  Configuration Functions

				void setDCHuffmanTree(Huffman::HuffmanTree* NewDCTree, int Channel) { DCTree[Channel] = NewDCTree; return; }
				void setACHuffmanTree(Huffman::HuffmanTree* NewACTree, int Channel) { ACTree[Channel] = NewACTree; return; }
				void setQuantizer(JPEGQuantizer* NewQ, int Channel) { Q[Channel] = NewQ; return; }
				void setInput(CMCUBuilder* NewBuilder) { Input = NewBuilder; return; }
				void setHSF(int NewFactor, int Channel) { HSF[Channel] = NewFactor; return; }
				void setVSF(int NewFactor, int Channel) { VSF[Channel] = NewFactor; return; }
				void setMCUFF(BYTE NewFormFactor) { 
					MCUFF = NewFormFactor; 
					if (Input != nullptr) Input->setMCUFF(NewFormFactor);
					return;
				}

			private:

				//*******************************************************************************************************************
				//*																													*
				//*  Private Members																								*
				//*																													*
				//*******************************************************************************************************************

				//  Minimum Coding Unit (MCU)
				MCU								NewMCU;
				BYTE							MCUFF;												//  MCU Form Factor

				//  Input Object
				CMCUBuilder*					Input;												//  Source emitter

				//  DC & AC HuffmanTree & JPEGQuantizer objects
				Huffman::HuffmanTree*			DCTree[3];											//  DC Huffman Tree
				Huffman::HuffmanTree*			ACTree[3];											//  AC Huffman Tree
				JPEGQuantizer*					Q[3];												//  Quantizer

				//  Sampling factors
				int								HSF[3];												//  Horizontal Sampling Factor
				int								VSF[3];												//  Vertical Sampling Factor

				//  Instrumentation
				size_t							YRead;
				size_t							CbRead;
				size_t							CrRead;

			};

		public:

			//*******************************************************************************************************************
			//*                                                                                                                 *
			//*   Public Nested Classes                                                                                         *
			//*                                                                                                                 *
			//*******************************************************************************************************************

			//*******************************************************************************************************************
			//*                                                                                                                 *
			//*   Emitter Class																									*
			//*                                                                                                                 *
			//*   The Emitter class emits the pixels obtained fropm the pipeline one pixel at a time in MCU packages.			*
			//*   Any consumer must be aware that the pixel are emitted on a per MCU basis and in Row/Column order for			*
			//*   each MCU, this includes pixels that are off the canvas but used to make up complete MCUs.						*
			//*                                                                                                                 *
			//*                                                                                                                 *
			//*******************************************************************************************************************

			class Emitter {
			public:

				//*******************************************************************************************************************
				//*                                                                                                                 *
				//*   Constructors                                                                                                  *
				//*                                                                                                                 *
				//*******************************************************************************************************************

				//  Normal Constructor
				//
				//  Constructs a new Emitter and connects it to the end (output side) of the pipeline.
				//
				//  PARAMETERS
				//
				//		MCUBuilder*			-		Pointer to the MCUBuilder at the end of the pipeline
				//		BYTE				-		The form Factor of the MCU
				//
				//	RETURNS
				//
				//	NOTES
				//

				Emitter(MCUBuilder* pEndOfPipe, BYTE FF) {
					//  Clear the MCU
					memset(&NewMCU, 0, sizeof(MCU));
					MCUFF = FF;

					//  Clear the input object and set the index count to trigger a read of the next MCU
					Input = nullptr;
					rIndex = 16;
					cIndex = 16;
					MCURead = 0;

					//  Connect to the end of the pipe
					Input = pEndOfPipe;

					//  Return to caller
					return;
				}

				//*******************************************************************************************************************
				//*                                                                                                                 *
				//*   Public Functions                                                                                              *
				//*                                                                                                                 *
				//*******************************************************************************************************************

				//  hasNext
				//
				//  Detects if the Emitter has more data available or has reached the end
				//
				//  PARAMETERS
				//
				//  RETURNS
				//
				//		bool				-		true if a pixel is available, false if not
				//
				//  NOTES
				//

				bool hasNext() {
					if (rIndex <= 15) return true;
					return Input->hasNext();
				}

				//  next
				//
				//  Returns (emits) the next pixel from the pipeline
				//
				//  PARAMETERS
				//
				//  RETURNS
				//
				//		YCbCr					-		Next YCbCr encoded pixel
				//
				//  NOTES
				//

				YCbCr next() {
					YCbCr			Pixel = {};

					//  See if MCU is exhausted
					if (rIndex == 16) {
						NewMCU = Input->nextMCU();
						MCURead++;
						rIndex = 0;
						cIndex = 0;
					}

					//  Output the next pixel
					if (rIndex < 8 && cIndex < 8) {
						//  DU = 0
						if (rIndex == 0 && cIndex == 0) {
							Pixel.Y = BYTE(NewMCU.DUY[0].DC);
							Pixel.Cb = BYTE(NewMCU.DUCb[0].DC);
							Pixel.Cr = BYTE(NewMCU.DUCr[0].DC);
						}
						else {
							Pixel.Y = BYTE(NewMCU.DUY[0].AC[((rIndex * 8) + cIndex) - 1]);
							Pixel.Cb = BYTE(NewMCU.DUCb[0].AC[((rIndex * 8) + cIndex) - 1]);
							Pixel.Cr = BYTE(NewMCU.DUCr[0].AC[((rIndex * 8) + cIndex) - 1]);
						}
					}
					else {
						if (rIndex < 8 && cIndex > 7) {
							//  DU = 1
							if (rIndex == 0 && cIndex == 8) {
								Pixel.Y = BYTE(NewMCU.DUY[1].DC);
								Pixel.Cb = BYTE(NewMCU.DUCb[1].DC);
								Pixel.Cr = BYTE(NewMCU.DUCr[1].DC);
							}
							else {
								Pixel.Y = BYTE(NewMCU.DUY[1].AC[((rIndex * 8) + (cIndex - 8)) - 1]);
								Pixel.Cb = BYTE(NewMCU.DUCb[1].AC[((rIndex * 8) + (cIndex - 8)) - 1]);
								Pixel.Cr = BYTE(NewMCU.DUCr[1].AC[((rIndex * 8) + (cIndex - 8)) - 1]);
							}
						}
						else {
							if (rIndex > 7 && cIndex < 8) {
								//  DU = 2
								if (rIndex == 8 && cIndex == 0) {
									Pixel.Y = BYTE(NewMCU.DUY[2].DC);
									Pixel.Cb = BYTE(NewMCU.DUCb[2].DC);
									Pixel.Cr = BYTE(NewMCU.DUCr[2].DC);
								}
								else {
									Pixel.Y = BYTE(NewMCU.DUY[2].AC[(((rIndex - 8) * 8) + cIndex) - 1]);
									Pixel.Cb = BYTE(NewMCU.DUCb[2].AC[(((rIndex - 8) * 8) + cIndex) - 1]);
									Pixel.Cr = BYTE(NewMCU.DUCr[2].AC[(((rIndex - 8) * 8) + cIndex) - 1]);
								}
							}
							else {
								//  DU = 3
								if (rIndex == 8 && cIndex == 8) {
									Pixel.Y = BYTE(NewMCU.DUY[3].DC);
									Pixel.Cb = BYTE(NewMCU.DUCb[3].DC);
									Pixel.Cr = BYTE(NewMCU.DUCr[3].DC);
								}
								else {
									Pixel.Y = BYTE(NewMCU.DUY[3].AC[(((rIndex - 8) * 8) + (cIndex - 8)) - 1]);
									Pixel.Cb = BYTE(NewMCU.DUCb[3].AC[(((rIndex - 8) * 8) + (cIndex - 8)) - 1]);
									Pixel.Cr = BYTE(NewMCU.DUCr[3].AC[(((rIndex - 8) * 8) + (cIndex - 8)) - 1]);
								}
							}
						}
					}

					//  Increment the position
					cIndex++;
					if (cIndex == 16) {
						rIndex++;
						cIndex = 0;
					}

					//  Adjust the position according to the Form Factor in use
					switch (MCUFF) {
					//  All DUs are returned
					case 0x22:
						break;
					
					//  Only DU: 0 and 2 are returned
					case 0x12:
						if (cIndex == 8) {
							cIndex = 0;
							rIndex++;
						}
						break;

					//  Only DU: 0 and 1 are returned
					case 0x21:
						if (rIndex == 8) rIndex = 16;
						break;

					//  Only DU: 0 is returned
					case 0x11:
						if (cIndex == 8) {
							cIndex = 0;
							rIndex++;
						}
						if (rIndex == 8) rIndex = 16;
						break;
					}

					//  Return the assembled pixel
					return Pixel;
				}

			private:

				//*******************************************************************************************************************
				//*																													*
				//*  Private Members																								*
				//*																													*
				//*******************************************************************************************************************

				//  Minimum Coding Unit (MCU)
				MCU								NewMCU;
				BYTE							MCUFF;												//  MCU Form Factor

				//  Input Object
				MCUBuilder*						Input;												//  Source emitter

				//  Processing Indexes
				int								rIndex;												//  Row index (in virtual MCU)
				int								cIndex;												//  Column index (in virtual MCU)

				//  Instrumentation
				size_t							MCURead;											//  Count of MCUs read

			};

			//*******************************************************************************************************************
			//*																													*
			//*  Constructor																									*
			//*																													*
			//*******************************************************************************************************************

			DecoderPipeline() {

				//  Setup the plumbing for the pipeline
				DUDQ.setInput(&DUB);
				DUUZ.setInput(&DUDQ);
				IDCT.setInput(&DUUZ);
				DUUS.setInput(&IDCT);
				CMCUB.setInput(&DUUS);
				MCUB.setInput(&CMCUB);

				MCUFF = 0x22;

				//  Return to caller
				return;
			}

			//*******************************************************************************************************************
			//*                                                                                                                 *
			//*   Public Functions                                                                                              *
			//*                                                                                                                 *
			//*******************************************************************************************************************

			//  Configuration Functions
			void		setDequantizer(int Channel, JPEGQuantizer* pNewDQ) { MCUB.setQuantizer(pNewDQ, Channel); return; }
			void		setDCDecoder(int Channel, Huffman::HuffmanTree* pNewDec) { MCUB.setDCHuffmanTree(pNewDec, Channel); return; }
			void		setACDecoder(int Channel, Huffman::HuffmanTree* pNewDec) { MCUB.setACHuffmanTree(pNewDec, Channel); return; }
			void		setHSPM(int Channel, int Samples) { MCUB.setHSF(Samples, Channel); return; }
			void		setVSPM(int Channel, int Samples) { MCUB.setVSF(Samples, Channel); return; }
			void		setPrecision(int Precision) { DUUS.setPrecision(Precision); return; }
			void		setMCUFF(BYTE NewFormFactor) { MCUFF = NewFormFactor; MCUB.setMCUFF(NewFormFactor); return; }

			//  decode
			//
			//  Connects the input and output ends of the decoding pipeline
			//
			//  PARAMETERS
			//
			//		Huffman::Emitter*			-			Pointer to the Huffman CODEC Emitter that provides the input to the pipeline
			//
			//  RETURNS
			//
			//		JPEGEmitter					-			The Emitter that emits the output from the pieline
			//
			//  NOTES
			//
			//	Before using he emitter callers MUST setup all of the configuration elements for the decode operation :-
			//
			//		1.			JPEGQuantizer for each channel.
			//		2.			DC Huffman Trees for each channel.
			//		3.			AC Huffman Trees for each channel.
			//		4.			Horizontal and Vertical sampling factors for each channel.
			//		5.			Precision (Channel width in bits)
			//		6.			The MCU Form Factor
			//

			Emitter		decode(Huffman::JPEGEmitter* HCEmitter) {

				//  Connect the input to the start of the pipeline
				DUB.setInput(HCEmitter);

				//  Return the output Emitter - connected to the end of the pipeline
				return Emitter(&MCUB, MCUFF);
			}

		private:

			//*******************************************************************************************************************
			//*																													*
			//*  Private Members																								*
			//*																													*
			//*******************************************************************************************************************

			//  Processing Pipeline Components

			DUBuilder						DUB;														//  DU Builder
			Dequantizer						DUDQ;														//  DU Dequantizer
			Unzigzagger						DUUZ;														//  DU Unzigzagger
			DCTInverter						IDCT;														//  Inverse DCT
			Upshifter						DUUS;														//  DU Upshifter
			CMCUBuilder						CMCUB;														//  CMCU Builder
			MCUBuilder						MCUB;														//  MCU Builder

			BYTE							MCUFF;														//  MCU Form factor

		};

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   EncoderPipeline Class																							*
		//*                                                                                                                 *
		//*   The EncoderPipeline class is the main engine for encoding JPEG image data.									*
		//*   It links together and synchronises the operation of a chain of components to provide the complete encoding	*
		//*   mechanics.																									*
		//*                                                                                                                 *
		//*******************************************************************************************************************

		class EncoderPipeline {
		public:

			//*******************************************************************************************************************
			//*                                                                                                                 *
			//*   Constructors                                                                                                  *
			//*                                                                                                                 *
			//*******************************************************************************************************************

			//  Normal Constructor
			//
			//  Constructs a new EncodePipeline instance.
			//
			//  PARAMETERS
			//
			//  RETURNS
			//
			//  NOTES
			//
			//

			EncoderPipeline() {

				//  Setup the internal plumbing for the pipeline
				MCUS.setOutput(&CMCUS);
				CMCUS.setOutput(&DSDU);
				DSDU.setOutput(&DCTDU);
				DCTDU.setOutput(&ZZDU);
				ZZDU.setOutput(&QDU);
				QDU.setOutput(&SDU);

				MCUFF = 0x22;

				//  Return to caller
				return;
			}

			//*******************************************************************************************************************
			//*                                                                                                                 *
			//*   Destructor                                                                                                    *
			//*                                                                                                                 *
			//*******************************************************************************************************************

			~EncoderPipeline() {

				//  Return to caller
				return;
			}

		private:

			//*******************************************************************************************************************
			//*                                                                                                                 *
			//*   Private Nested Classes																						*
			//*   Pipeline Processing Components																				*
			//*                                                                                                                 *
			//*******************************************************************************************************************

			//*******************************************************************************************************************
			//*                                                                                                                 *
			//*   DUSplitter Class																								*
			//*                                                                                                                 *
			//*   The DUSplitter class receives the next available DU from the pipeline and splits the DU into numbers.			*
			//*   The numbers are passed to the Huffman CODEC collecter.														*
			//*                                                                                                                 *
			//*                                                                                                                 *
			//*******************************************************************************************************************

			class DUSplitter {
			public:
				//*******************************************************************************************************************
				//*                                                                                                                 *
				//*   Constructors                                                                                                  *
				//*                                                                                                                 *
				//*******************************************************************************************************************

				DUSplitter() {
					//  Clear the tables
					DCTree = nullptr;
					ACTree = nullptr;
					PreviousDC[0] = 0;
					PreviousDC[1] = 0;
					PreviousDC[2] = 0;

					COEmitted = 0;

					Output = nullptr;

					//  Return to caller
					return;
				}

				//*******************************************************************************************************************
				//*                                                                                                                 *
				//*   Public Functions                                                                                              *
				//*                                                                                                                 *
				//*******************************************************************************************************************

				//  Configuration Functions

				void setDCHuffmanTree(Huffman::HuffmanTree* NewDCTree) { DCTree = NewDCTree; return; }
				void setACHuffmanTree(Huffman::HuffmanTree* NewACTree) { ACTree = NewACTree; return; }
				void setOutput(Huffman::JPEGCollecter* NewOutput) { Output = NewOutput; return; }

				//  next
				//
				//  Accepts the next available DU from the pipeline and splits it into discrete coefficients.
				//	Individual coefficients are then passed to the Huffman CODEC collecter.
				//
				//  PARAMETERS
				//
				//		DU&		-		Reference to the sutffed DU
				//		int		-		Channel
				//
				//  RETURNS
				//
				//  NOTES
				//

				void		next(DU& StuffedDU, int Channel) {
					int16_t		DeltaDC = 0;														//  Delta from the previous DC value

					DeltaDC = StuffedDU.DC - PreviousDC[Channel];									//  Compute the delta
					PreviousDC[Channel] = StuffedDU.DC;

					//  Set the DC Huffman Tree
					Output->setTree(DCTree);

					//  Output the DC Delta value
					Output->next(DeltaDC, true);

					//  Set the AC Huffman Tree
					Output->setTree(ACTree);

					//  Output the 63 AC values
					for (size_t ACX = 0; ACX < 63; ACX++) Output->next(StuffedDU.AC[ACX], false);

					COEmitted += 64;

					//  Return to caller
					return;
				}

				//  signalEndOfStream
				//
				//  Signals that the end of the image data stream has been reached
				//
				//  PARAMETERS
				//
				//  RETURNS
				//
				//  NOTES
				//
				//

				void	signalEndOfStream() {

					//  Pass the signal up the pipeline
					Output->signalEndOfStream();

					//  Return to caller
					return;
				}

			private:
				//*******************************************************************************************************************
				//*																													*
				//*  Private Members																								*
				//*																													*
				//*******************************************************************************************************************

				//  Output Object
				Huffman::JPEGCollecter*			Output;

				//  DC & AC HuffmanTree & JPEGQuantizer objects
				Huffman::HuffmanTree*			DCTree;												//  DC Huffman Tree
				Huffman::HuffmanTree*			ACTree;												//  AC Huffman Tree

				//  Persistent Data
				uint16_t						PreviousDC[3];										//  Previous DC values (per channel)

				//  Instrumentation
				size_t							COEmitted;											//  Coefficients emitted
			};

			//*******************************************************************************************************************
			//*                                                                                                                 *
			//*   Quantizer Class																								*
			//*                                                                                                                 *
			//*   The Quantizer class receives the next available DU from the pipeline and quantizes the coefficients.			*
			//*   It then passes the DU to the next component in the pipeline.													*
			//*                                                                                                                 *
			//*                                                                                                                 *
			//*******************************************************************************************************************

			class Quantizer {
			public:
				//*******************************************************************************************************************
				//*                                                                                                                 *
				//*   Constructors                                                                                                  *
				//*                                                                                                                 *
				//*******************************************************************************************************************

				//  Default Constructor
				Quantizer() {
					//  Clear the tables
					DCTree = nullptr;
					ACTree = nullptr;
					Q = nullptr;

					//  Clear the counters
					DUStuffed = 0;

					//  Clear the output
					Output = nullptr;

					//  Return to caller
					return;
				}

				//*******************************************************************************************************************
				//*                                                                                                                 *
				//*   Public Functions                                                                                              *
				//*                                                                                                                 *
				//*******************************************************************************************************************

				//  Configuration Functions

				void setDCHuffmanTree(Huffman::HuffmanTree* NewDCTree) { DCTree = NewDCTree; return; }
				void setACHuffmanTree(Huffman::HuffmanTree* NewACTree) { ACTree = NewACTree; return; }
				void setQuantizer(JPEGQuantizer* NewQ) { Q = NewQ; return; }
				void setOutput(DUSplitter* NewOutput) { Output = NewOutput; return; }

				//  next
				//
				//  Accepts the next available DU from the pipeline and applies Discrete Cosine Transform (DCT).
				//	Then passes the DU to the next component with the appropriate resources (Huffman Trees and Quantizers).
				//
				//  PARAMETERS
				//
				//		DU&				-		Reference to the sutffed DU
				//		int				-		Channel Number being read
				//
				//  RETURNS
				//
				//  NOTES
				//

				void		next(DU& StuffedDU, int Channel) {

					//  Dequantize the passed DU
					Q->quantize(StuffedDU);

					//  Set the selected huffman trees and quantizer in the next component
					Output->setDCHuffmanTree(DCTree);
					Output->setACHuffmanTree(ACTree);

					//  Pass along to the next component in the pipeline
					Output->next(StuffedDU, Channel);
					DUStuffed++;

					//  Return to caller
					return;
				}

				//  signalEndOfStream
				//
				//  Signals that the end of the image data stream has been reached
				//
				//  PARAMETERS
				//
				//  RETURNS
				//
				//  NOTES
				//
				//

				void	signalEndOfStream() {

					//  Pass the signal up the pipeline
					Output->signalEndOfStream();

					//  Return to caller
					return;
				}

			private:
				//*******************************************************************************************************************
				//*																													*
				//*  Private Members																								*
				//*																													*
				//*******************************************************************************************************************

				//  Output Object
				DUSplitter*						Output;

				//  DC & AC HuffmanTree & JPEGQuantizer objects
				Huffman::HuffmanTree*			DCTree;												//  DC Huffman Tree
				Huffman::HuffmanTree*			ACTree;												//  AC Huffman Tree
				JPEGQuantizer*					Q;													//  Quantizer

				//  Instrumentation
				size_t							DUStuffed;

			};

			//*******************************************************************************************************************
			//*                                                                                                                 *
			//*   ZigZagger Class																								*
			//*                                                                                                                 *
			//*   The ZigZagger class receives the next available DU from the pipeline and places it into Zig-Zag order.		*
			//*   It then passes the DU to the next component in the chain.														*
			//*                                                                                                                 *
			//*                                                                                                                 *
			//*******************************************************************************************************************

			class ZigZagger {
			public:
				//*******************************************************************************************************************
				//*                                                                                                                 *
				//*   Constructors                                                                                                  *
				//*                                                                                                                 *
				//*******************************************************************************************************************

				//  Default Constructor
				ZigZagger() {
					//  Clear the tables
					DCTree = nullptr;
					ACTree = nullptr;
					Q = nullptr;

					//  Clear the counters
					DUStuffed = 0;

					//  Clear the output
					Output = nullptr;

					//  Return to caller
					return;
				}

				//*******************************************************************************************************************
				//*                                                                                                                 *
				//*   Public Functions                                                                                              *
				//*                                                                                                                 *
				//*******************************************************************************************************************

				//  Configuration Functions

				void setDCHuffmanTree(Huffman::HuffmanTree* NewDCTree) { DCTree = NewDCTree; return; }
				void setACHuffmanTree(Huffman::HuffmanTree* NewACTree) { ACTree = NewACTree; return; }
				void setQuantizer(JPEGQuantizer* NewQ) { Q = NewQ; return; }
				void setOutput(Quantizer* NewOutput) { Output = NewOutput; return; }

				//  next
				//
				//  Accepts the next available DU from the pipeline and applies the ZigZag reordering
				//	
				//
				//  PARAMETERS
				//
				//		DU&				-		Reference to the sutffed DU
				//		int				-		Channel Number being read
				//
				//  RETURNS
				//
				//  NOTES
				//

				void		next(DU& StuffedDU, int Channel) {
					int16_t			UZZ[64] = { 0, 1, 8, 16, 9, 2, 3, 10,
												 17, 24, 32, 25, 18, 11, 4, 5,
												 12, 19, 26, 33, 40, 48, 41, 34,
												 27, 20, 13, 6, 7, 14, 21, 28,
												 35, 42, 49, 56, 57, 50, 43, 36,
												 29, 22, 15, 23, 30, 37, 44, 51,
												 58, 59, 52, 45, 38, 31, 39, 46,
												 53, 60, 61, 54, 47, 55, 62, 63 };

					for (uint16_t iIndex = 1; iIndex < 64; iIndex++) UZZ[iIndex] = StuffedDU.AC[UZZ[iIndex] - 1];

					for (uint16_t iIndex = 1; iIndex < 64; iIndex++) StuffedDU.AC[iIndex - 1] = UZZ[iIndex];

					//  Set the tables and pass the DU up the pipeline
					Output->setDCHuffmanTree(DCTree);
					Output->setACHuffmanTree(ACTree);
					Output->setQuantizer(Q);

					Output->next(StuffedDU, Channel);

					DUStuffed++;

					//  Return to caller
					return;
				}

				//  signalEndOfStream
				//
				//  Signals that the end of the image data stream has been reached
				//
				//  PARAMETERS
				//
				//  RETURNS
				//
				//  NOTES
				//
				//

				void	signalEndOfStream() {

					//  Pass the signal up the pipeline
					Output->signalEndOfStream();

					//  Return to caller
					return;
				}

			private:
				//*******************************************************************************************************************
				//*																													*
				//*  Private Members																								*
				//*																													*
				//*******************************************************************************************************************

				//  Output Object
				Quantizer*						Output;

				//  DC & AC HuffmanTree & JPEGQuantizer objects
				Huffman::HuffmanTree*			DCTree;												//  DC Huffman Tree
				Huffman::HuffmanTree*			ACTree;												//  AC Huffman Tree
				JPEGQuantizer*					Q;													//  Quantizer

				//  Instrumentation
				size_t							DUStuffed;

			};

			//*******************************************************************************************************************
			//*                                                                                                                 *
			//*   DCT Class																										*
			//*                                                                                                                 *
			//*   The DCT class receives the next available DU from the pipeline and applies Discrete Cosine Transform (DCT)	*
			//*   to the DU before passing it on up the pipeline.																*
			//*                                                                                                                 *
			//*                                                                                                                 *
			//*******************************************************************************************************************

			class DCT {
			public:

				//*******************************************************************************************************************
				//*                                                                                                                 *
				//*   Constructors                                                                                                  *
				//*                                                                                                                 *
				//*******************************************************************************************************************

				//  Default Constructor
				DCT() {

					//  Clear the tables
					DCTree = nullptr;
					ACTree = nullptr;
					Q = nullptr;

					//  Clear the counters
					DUStuffed = 0;

					//  Clear the output
					Output = nullptr;

					//  Return to caller
					return;
				}

				//*******************************************************************************************************************
				//*                                                                                                                 *
				//*   Public Functions                                                                                              *
				//*                                                                                                                 *
				//*******************************************************************************************************************

				//  Configuration Functions

				void setDCHuffmanTree(Huffman::HuffmanTree* NewDCTree) { DCTree = NewDCTree; return; }
				void setACHuffmanTree(Huffman::HuffmanTree* NewACTree) { ACTree = NewACTree; return; }
				void setQuantizer(JPEGQuantizer* NewQ) { Q = NewQ; return; }
				void setOutput(ZigZagger* NewOutput) { Output = NewOutput; return; }

				//  next
				//
				//  Accepts the next available DU from the pipeline and applies Discrete Cosine Transform (DCT).
				//	Then passes the DU to the next component with the appropriate resources (Huffman Trees and Quantizers).
				//
				//  PARAMETERS
				//
				//		DU&				-		Reference to the sutffed DU
				//		int				-		Channel Number being read
				//
				//  RETURNS
				//
				//  NOTES
				//

				void	next(DU& StuffedDU, int Channel) {
					double					Element;																		//  Discrete element
					double					Accumulator;																	//  Accumulator for element summing
					DU						Out = {};																		//  Output DU

					//  Convert each coefficient in the Data unit in turn
					for (uint16_t vIndex = 0; vIndex < 8; vIndex++)
					{
						for (uint16_t uIndex = 0; uIndex < 8; uIndex++)
						{
							//  Perform the forward DCT transform
							Accumulator = 0.0;

							for (uint16_t xIndex = 0; xIndex < 8; xIndex++)
							{
								for (uint16_t yIndex = 0; yIndex < 8; yIndex++)
								{
									if (xIndex == 0 && yIndex == 0) Element = double(StuffedDU.DC);
									else Element = double(StuffedDU.AC[((yIndex * 8) + xIndex) - 1]);
									Element = Element * cos(((((2.0 * double(xIndex)) + 1.0) * double(uIndex)) * Pi) / 16.0);
									Element = Element * cos(((((2.0 * double(yIndex)) + 1.0) * double(vIndex)) * Pi) / 16.0);
									Accumulator += Element;
								}
							}

							if (vIndex == 0) Accumulator = Accumulator * (1.0 / sqrt(2));
							if (uIndex == 0) Accumulator = Accumulator * (1.0 / sqrt(2));
							Accumulator = Accumulator / 4.0;

							//  Assign the sample to the Data Unit
							if (vIndex == 0 && uIndex == 0) Out.DC = short(floor(Accumulator + 0.5));
							else Out.AC[((vIndex * 8) + uIndex) - 1] = short(floor(Accumulator + 0.5));
						}
					}

					//  Set the selected huffman trees and quantizer in the next component
					Output->setDCHuffmanTree(DCTree);
					Output->setACHuffmanTree(ACTree);
					Output->setQuantizer(Q);

					//  Pass the transformed DU to the output component
					Output->next(Out, Channel);

					//  Return to caller
					return;
				}

				//  signalEndOfStream
				//
				//  Signals that the end of the image data stream has been reached
				//
				//  PARAMETERS
				//
				//  RETURNS
				//
				//  NOTES
				//
				//

				void	signalEndOfStream() {

					//  Pass the signal up the pipeline
					Output->signalEndOfStream();

					//  Return to caller
					return;
				}

			private:
				//*******************************************************************************************************************
				//*																													*
				//*  Private Members																								*
				//*																													*
				//*******************************************************************************************************************

				//  Output Object
				ZigZagger*						Output;

				//  DC & AC HuffmanTree & JPEGQuantizer objects
				Huffman::HuffmanTree*			DCTree;												//  DC Huffman Tree
				Huffman::HuffmanTree*			ACTree;												//  AC Huffman Tree
				JPEGQuantizer*					Q;													//  Quantizer

				//  Instrumentation
				size_t							DUStuffed;
			};

			//*******************************************************************************************************************
			//*                                                                                                                 *
			//*   Downshifter Class																								*
			//*                                                                                                                 *
			//*   The Downshifter class receives the next available DU from the pipeline and applies the down shift to the DU	*
			//*   Coefficients, ready for DCT encoding.																			*
			//*                                                                                                                 *
			//*                                                                                                                 *
			//*******************************************************************************************************************

			class Downshifter {
			public:

				//*******************************************************************************************************************
				//*                                                                                                                 *
				//*   Constructors                                                                                                  *
				//*                                                                                                                 *
				//*******************************************************************************************************************

				//  Default Constructor
				Downshifter() {
					//  Clear the tables
					DCTree = nullptr;
					ACTree = nullptr;
					Q = nullptr;

					//  Clear the counters
					DUStuffed = 0;

					//  Set the precision
					Precision = 0;

					// Clear the output
					Output = nullptr;

					//  Return to caller
					return;
				}

				//*******************************************************************************************************************
				//*                                                                                                                 *
				//*   Public Functions                                                                                              *
				//*                                                                                                                 *
				//*******************************************************************************************************************

				//  Configuration Functions

				void setDCHuffmanTree(Huffman::HuffmanTree* NewDCTree) { DCTree = NewDCTree; return; }
				void setACHuffmanTree(Huffman::HuffmanTree* NewACTree) { ACTree = NewACTree; return; }
				void setQuantizer(JPEGQuantizer* NewQ) { Q = NewQ; return; }
				void setOutput(DCT* NewOutput) { Output = NewOutput; return; }
				void setPrecision(int NewPrecision) { Precision = NewPrecision; return; }

				//  next
				//
				//  Accepts a completed DU shifts the values before stuffing the DU up the pipeline
				//	with the appropriate resources (Huffman Trees and Quantizers).
				//
				//  PARAMETERS
				//
				//		DU&				-		Reference to the stuffed DU
				//		int				-		Channel Number being read
				//
				//  RETURNS
				//
				//  NOTES
				//

				void	next(DU& StuffedDU, int Channel) {
					int16_t			Shift = 1 << (Precision - 1);											//  Shift value

					//  Set the selected huffman trees and quantizer in the next component
					Output->setDCHuffmanTree(DCTree);
					Output->setACHuffmanTree(ACTree);
					Output->setQuantizer(Q);

					//  Shift the DC coefficient
					StuffedDU.DC -= Shift;
					for (int ACX = 0; ACX < 63; ACX++) StuffedDU.AC[ACX] -= Shift;

					//  Stuff the DU onto the next component in the pipelain
					Output->next(StuffedDU, Channel);

					//  Return to caller
					return;
				}

				//  signalEndOfStream
				//
				//  Signals that the end of the image data stream has been reached
				//
				//  PARAMETERS
				//
				//  RETURNS
				//
				//  NOTES
				//
				//

				void	signalEndOfStream() {

					//  Pass the signal up the pipeline
					Output->signalEndOfStream();

					//  Return to caller
					return;
				}

			private:
				//*******************************************************************************************************************
				//*																													*
				//*  Private Members																								*
				//*																													*
				//*******************************************************************************************************************

				//  Output Object
				DCT*							Output;

				//  DC & AC HuffmanTree & JPEGQuantizer objects
				Huffman::HuffmanTree*			DCTree;												//  DC Huffman Tree
				Huffman::HuffmanTree*			ACTree;												//  AC Huffman Tree
				JPEGQuantizer*					Q;													//  Quantizer

				//  Precision
				int								Precision;											//  Precision (current frame)

				//  Instrumentation
				size_t							DUStuffed;

			};

			//*******************************************************************************************************************
			//*                                                                                                                 *
			//*   CMCUSplitter Class																							*
			//*                                                                                                                 *
			//*   The CMCUSplitter class splits the stuffed CMCU into separate DUs (per channel) and stuffs them into the		*
			//*	  next component in the pipeline. The Sampling factors (horizontal & Vertical) are accounted for by this		*
			//*   component.																									*
			//*                                                                                                                 *
			//*                                                                                                                 *
			//*******************************************************************************************************************

			class CMCUSplitter {
			public:

				//*******************************************************************************************************************
				//*                                                                                                                 *
				//*   Constructors                                                                                                  *
				//*                                                                                                                 *
				//*******************************************************************************************************************

				//  Default Constructor
				CMCUSplitter() {

					//  Clear the tables
					DCTree = nullptr;
					ACTree = nullptr;
					Q = nullptr;
					MCUFF = 0x22;

					//  Clear the counters
					DUStuffed = 0;

					// Clear the output
					Output = nullptr;

					//  Return to caller
					return;
				}

				//*******************************************************************************************************************
				//*                                                                                                                 *
				//*   Public Functions                                                                                              *
				//*                                                                                                                 *
				//*******************************************************************************************************************

				//  Configuration Functions

				void setDCHuffmanTree(Huffman::HuffmanTree* NewDCTree) { DCTree = NewDCTree; return; }
				void setACHuffmanTree(Huffman::HuffmanTree* NewACTree) { ACTree = NewACTree; return; }
				void setQuantizer(JPEGQuantizer* NewQ) { Q = NewQ; return; }
				void setOutput(Downshifter* NewOutput) { Output = NewOutput; return; }
				void setMCUFF(BYTE NewFormFactor) { MCUFF = NewFormFactor; return; }

				//  next
				//
				//  Accepts a completed CMCU and splits it into separate DUs, accounting for sampling,  and stuffs the DUs up the pipeline 
				//	with the appropriate resources (Huffman Trees and Quantizers).
				//
				//  PARAMETERS
				//
				//		CMCU&			-		Reference to the stuffed CMCU
				//		int				-		Channel Number being read
				//		int				-		Horizontal Sampling Factor
				//		int				-		Vertical Sampling factor
				//
				//  RETURNS
				//
				//  NOTES
				//

				void	next(CMCU& StuffedCMCU, int Channel, int HSF, int VSF) {

					//  Set the DC/AC trees and quantizer for the channel
					Output->setDCHuffmanTree(DCTree);
					Output->setACHuffmanTree(ACTree);
					Output->setQuantizer(Q);

					//  Regardless of the sampling factor we always pass on the top-left DU
					Output->next(StuffedCMCU.CDU[0], Channel);
					DUStuffed++;

					//  If the horizontal sampling factor is 2 then stuff the top right DU
					if (HSF == 2) {
						Output->next(StuffedCMCU.CDU[1], Channel);
						DUStuffed++;
					}

					//  If the vertical sampling factor is 2 then stuff the bottom
					if (VSF == 2) {

						Output->next(StuffedCMCU.CDU[2], Channel);
						DUStuffed++;

						//  If the horizontal sampling factor is also 2 then stuff the bottom right
						if (HSF == 2) {
							Output->next(StuffedCMCU.CDU[3], Channel);
							DUStuffed++;
						}
					}

					//  Return to caller
					return;
				}

				//  signalEndOfStream
				//
				//  Signals that the end of the image data stream has been reached
				//
				//  PARAMETERS
				//
				//  RETURNS
				//
				//  NOTES
				//
				//

				void	signalEndOfStream() {

					//  Pass the signal up the pipeline
					Output->signalEndOfStream();

					//  Return to caller
					return;
				}

			private:
				//*******************************************************************************************************************
				//*																													*
				//*  Private Members																								*
				//*																													*
				//*******************************************************************************************************************

				BYTE							MCUFF;												//  MCU Form Factor

				//  Output Object
				Downshifter*					Output;												//  Downshifter component

				//  DC & AC HuffmanTree & JPEGQuantizer objects
				Huffman::HuffmanTree*			DCTree;												//  DC Huffman Tree
				Huffman::HuffmanTree*			ACTree;												//  AC Huffman Tree
				JPEGQuantizer*					Q;													//  Quantizer

				//  Instrumentation
				size_t							DUStuffed;

			};

			//*******************************************************************************************************************
			//*                                                                                                                 *
			//*   MCUSplitter Class																								*
			//*                                                                                                                 *
			//*   The MCUSplitter class splits the stuffed MCU into separate CMCUs (per channel) and stuffs them into the		*
			//	  next component in the pipeline.																				*
			//*                                                                                                                 *
			//*                                                                                                                 *
			//*******************************************************************************************************************

			class MCUSplitter {
			public:

				//*******************************************************************************************************************
				//*                                                                                                                 *
				//*   Constructors                                                                                                  *
				//*                                                                                                                 *
				//*******************************************************************************************************************

				//  Default Constructor
				MCUSplitter() {
					//  Clear the MCU
					memset(&NewMCU, 0, sizeof(MCU));
					MCUFF = 0x22;

					//  Clear the tables
					DCTree[0] = nullptr;
					DCTree[1] = nullptr;
					DCTree[1] = nullptr;
					ACTree[0] = nullptr;
					ACTree[1] = nullptr;
					ACTree[2] = nullptr;
					Q[0] = nullptr;
					Q[1] = nullptr;
					Q[2] = nullptr;

					YStuffed = 0;
					CbStuffed = 0;
					CrStuffed = 0;

					Output = nullptr;

					HSF[0] = 0;
					HSF[1] = 0;
					HSF[2] = 0;

					VSF[0] = 0;
					VSF[1] = 0;
					VSF[2] = 0;

					//  Return to caller
					return;
				}

				//*******************************************************************************************************************
				//*                                                                                                                 *
				//*   Public Functions                                                                                              *
				//*                                                                                                                 *
				//*******************************************************************************************************************

				//  Configuration Functions

				void setDCHuffmanTree(Huffman::HuffmanTree* NewDCTree, int Channel) { DCTree[Channel] = NewDCTree; return; }
				void setACHuffmanTree(Huffman::HuffmanTree* NewACTree, int Channel) { ACTree[Channel] = NewACTree; return; }
				void setQuantizer(JPEGQuantizer* NewQ, int Channel) { Q[Channel] = NewQ; return; }
				void setHSF(int NewFactor, int Channel) { HSF[Channel] = NewFactor; return; }
				void setVSF(int NewFactor, int Channel) { VSF[Channel] = NewFactor; return; }
				void setOutput(CMCUSplitter* NewOutput) { Output = NewOutput; return; }
				void setMCUFF(BYTE NewFormFactor) { MCUFF = NewFormFactor; Output->setMCUFF(NewFormFactor); return; }

				//  next
				//
				//  Accepts a completed MCU and splits it into separate CMCUs and stuffs the CMCUs up the pipeline 
				//	with the appropriate resources (Huffman Trees and Quantizers).
				//
				//  PARAMETERS
				//
				//		MCU&		-		Reference to the stffed MCU
				// 
				//  RETURNS
				//
				//  NOTES
				//

				void	next(MCU& StuffedMCU) {
					CMCU			ChanMCU = {};

					//  Process the Y (Luminance) channel

					//  Set the Huffman Trees and Quantizer for the Y-channel
					Output->setDCHuffmanTree(DCTree[0]);
					Output->setACHuffmanTree(ACTree[0]);
					Output->setQuantizer(Q[0]);

					//  Prepare the CMCU
					ChanMCU.CDU[0] = StuffedMCU.DUY[0];
					if (MCUFF == 0x22) {
						ChanMCU.CDU[1] = StuffedMCU.DUY[1];
						ChanMCU.CDU[2] = StuffedMCU.DUY[2];
						ChanMCU.CDU[3] = StuffedMCU.DUY[3];
					}
					//  Stuff the CMCU up the pipeline
					Output->next(ChanMCU, 0, HSF[0], VSF[0]);
					YStuffed++;

					//  Process the Cb (Chrominance) channel

					//  Set the Huffman Trees and Quantizer for the Cb-channel
					Output->setDCHuffmanTree(DCTree[1]);
					Output->setACHuffmanTree(ACTree[1]);
					Output->setQuantizer(Q[1]);

					//  Prepare the CMCU
					ChanMCU.CDU[0] = StuffedMCU.DUCb[0];
					if (MCUFF == 0x22) {
						ChanMCU.CDU[1] = StuffedMCU.DUCb[1];
						ChanMCU.CDU[2] = StuffedMCU.DUCb[2];
						ChanMCU.CDU[3] = StuffedMCU.DUCb[3];
					}

					//  Stuff the CMCU up the pipeline
					Output->next(ChanMCU, 1, HSF[1], VSF[1]);
					CbStuffed++;

					//  Process the Cr (Chrominance) channel

					//  Set the Huffman Trees and Quantizer for the Cr-channel
					Output->setDCHuffmanTree(DCTree[2]);
					Output->setACHuffmanTree(ACTree[2]);
					Output->setQuantizer(Q[2]);

					//  Prepare the CMCU
					ChanMCU.CDU[0] = StuffedMCU.DUCr[0];
					if (MCUFF == 0x22) {
						ChanMCU.CDU[1] = StuffedMCU.DUCr[1];
						ChanMCU.CDU[2] = StuffedMCU.DUCr[2];
						ChanMCU.CDU[3] = StuffedMCU.DUCr[3];
					}
					//  Stuff the CMCU up the pipeline
					Output->next(ChanMCU, 2, HSF[2], VSF[2]);
					CrStuffed++;

					//  Return to caller
					return;
				}

				//  signalEndOfStream
				//
				//  Signals that the end of the image data stream has been reached
				//
				//  PARAMETERS
				//
				//  RETURNS
				//
				//  NOTES
				//
				//

				void	signalEndOfStream() {

					//  Pass the signal up the pipeline
					Output->signalEndOfStream();

					//  Return to caller
					return;
				}

			private:

				//*******************************************************************************************************************
				//*																													*
				//*  Private Members																								*
				//*																													*
				//*******************************************************************************************************************

				//  Minimum Coding Unit (MCU)
				MCU								NewMCU;
				BYTE							MCUFF;												//  MCU Form factor

				//  Output Object
				CMCUSplitter*					Output;												//  Channel MCU splitter

				//  DC & AC HuffmanTree & JPEGQuantizer objects
				Huffman::HuffmanTree*			DCTree[3];											//  DC Huffman Tree
				Huffman::HuffmanTree*			ACTree[3];											//  AC Huffman Tree
				JPEGQuantizer*					Q[3];												//  Quantizer

				//  Sampling factors
				int								HSF[3];												//  Horizontal Sampling Factor
				int								VSF[3];												//  Vertical Sampling Factor

				//  Instrumentation
				size_t							YStuffed;
				size_t							CbStuffed;
				size_t							CrStuffed;

			};

		public:

			//*******************************************************************************************************************
			//*                                                                                                                 *
			//*   Public Nested Classes                                                                                         *
			//*                                                                                                                 *
			//*******************************************************************************************************************

			//*******************************************************************************************************************
			//*                                                                                                                 *
			//*   Collecter Class																								*
			//*                                                                                                                 *
			//*   The Collecter class is fed pixels (in MCU iterator sequence) and builds successive MCUs from the pixels.		*
			//*   Once completed each MCU is pushed up the pipeline for processing.												*
			//*                                                                                                                 *
			//*                                                                                                                 *
			//*******************************************************************************************************************

			class Collecter {
			public:

				//*******************************************************************************************************************
				//*                                                                                                                 *
				//*   Constructors                                                                                                  *
				//*                                                                                                                 *
				//*******************************************************************************************************************

				//  Normal Constructor
				//
				//  Constructs a new Collecter and connects it to the start (input side) of the pipeline.
				//
				//  PARAMETERS
				//
				//		MCUSplitter*			-		Pointer to the MCUSplitter at the start of the pipeline
				//		BYTE					-		MCU Form Factor
				//
				//	RETURNS
				//
				//	NOTES
				//

				Collecter(MCUSplitter* pStartOfPipe, BYTE FF) {
					//  Clear the MCU
					memset(&NewMCU, 0, sizeof(MCU));
					MCUFF = FF;

					//  Set the index count to the start of the MCU
					rIndex = 0;
					cIndex = 0;
					MCUStuffed = 0;

					//  Connect the output to the end of the pipeline
					Output = pStartOfPipe;

					//  Return the input collecter
					return;
				}

				//*******************************************************************************************************************
				//*                                                                                                                 *
				//*   Public Functions                                                                                              *
				//*                                                                                                                 *
				//*******************************************************************************************************************

				//  Configuration functions

				void setDCHuffmanTree(Huffman::HuffmanTree* NewDCTree, int Channel) { Output->setDCHuffmanTree(NewDCTree, Channel); return; }
				void setACHuffmanTree(Huffman::HuffmanTree* NewACTree, int Channel) { Output->setACHuffmanTree(NewACTree, Channel); return; }
				void setQuantizer(JPEGQuantizer* NewQ, int Channel) { Output->setQuantizer(NewQ, Channel); return; }
				void setHSF(int NewFactor, int Channel) { Output->setHSF(NewFactor, Channel); return; }
				void setVSF(int NewFactor, int Channel) { Output->setVSF(NewFactor, Channel); return; }

				//  next
				//
				//  Accepts a new pixel (YCbCr) and assembles into the MCU under construction.
				//	Once a full MCU is constructed it is pushed into the pipeline.
				//
				//  PARAMETERS
				//
				//  RETURNS
				//
				//  NOTES
				//

				void	next(YCbCr& Pixel) {

					//  Collect the next pixel
					if (rIndex < 8 && cIndex < 8) {
						//  DU = 0
						if (rIndex == 0 && cIndex == 0) {
							NewMCU.DUY[0].DC = Pixel.Y;
							NewMCU.DUCb[0].DC = Pixel.Cb;
							NewMCU.DUCr[0].DC = Pixel.Cr;
						}
						else {
							NewMCU.DUY[0].AC[((rIndex * 8) + cIndex) - 1] = Pixel.Y;
							NewMCU.DUCb[0].AC[((rIndex * 8) + cIndex) - 1] = Pixel.Cb;
							NewMCU.DUCr[0].AC[((rIndex * 8) + cIndex) - 1] = Pixel.Cr;
						}
					}
					else {
						if (rIndex < 8 && cIndex > 7) {
							//  DU = 1
							if (rIndex == 0 && cIndex == 8) {
								NewMCU.DUY[1].DC = Pixel.Y;
								NewMCU.DUCb[1].DC = Pixel.Cb;
								NewMCU.DUCr[1].DC = Pixel.Cr;
							}
							else {
								NewMCU.DUY[1].AC[((rIndex * 8) + (cIndex - 8)) - 1] = Pixel.Y;
								NewMCU.DUCb[1].AC[((rIndex * 8) + (cIndex - 8)) - 1] = Pixel.Cb;
								NewMCU.DUCr[1].AC[((rIndex * 8) + (cIndex - 8)) - 1] = Pixel.Cr;
							}
						}
						else {
							if (rIndex > 7 && cIndex < 8) {
								//  DU = 2
								if (rIndex == 8 && cIndex == 0) {
									NewMCU.DUY[2].DC = Pixel.Y;
									NewMCU.DUCb[2].DC = Pixel.Cb;
									NewMCU.DUCr[2].DC = Pixel.Cr;
								}
								else {
									NewMCU.DUY[2].AC[(((rIndex - 8) * 8) + cIndex) - 1] = Pixel.Y;
									NewMCU.DUCb[2].AC[(((rIndex - 8) * 8) + cIndex) - 1] = Pixel.Cb;
									NewMCU.DUCr[2].AC[(((rIndex - 8) * 8) + cIndex) - 1] = Pixel.Cr;
								}
							}
							else {
								//  DU = 3
								if (rIndex == 8 && cIndex == 8) {
									NewMCU.DUY[3].DC = Pixel.Y;
									NewMCU.DUCb[3].DC = Pixel.Cb;
									NewMCU.DUCr[3].DC = Pixel.Cr;
								}
								else {
									NewMCU.DUY[3].AC[(((rIndex - 8) * 8) + (cIndex - 8)) - 1] = Pixel.Y;
									NewMCU.DUCb[3].AC[(((rIndex - 8) * 8) + (cIndex - 8)) - 1] = Pixel.Cb;
									NewMCU.DUCr[3].AC[(((rIndex - 8) * 8) + (cIndex - 8)) - 1] = Pixel.Cr;
								}
							}
						}
					}

					//  Increment the position
					cIndex++;
					if ((cIndex == 16 && MCUFF == 0x22) || (cIndex == 8 && MCUFF == 0x11)) {
						rIndex++;
						cIndex = 0;
					}

					//  If the MCU is now full then stuff it into the pipeline
					if ((rIndex == 16 && MCUFF == 0x22) || (rIndex == 8 && MCUFF == 0x11)) {

						//  Stuff the MCU into the pipeline
						Output->next(NewMCU);
						MCUStuffed++;

						//  Clear the MCU
						memset(&NewMCU, 0, sizeof(MCU));

						//  Reset to start a new MCU
						rIndex = 0;
						cIndex = 0;
					}

					//  Return to caller
					return;
				}

				//  signalEndOfStream
				//
				//  Signals that the end of the image data stream has been reached
				//
				//  PARAMETERS
				//
				//  RETURNS
				//
				//  NOTES
				//
				//

				void	signalEndOfStream() {

					//  If there is a partial MCU assembled (SNO) then stuff it into the pipeline
					if (rIndex > 0 || cIndex > 0) {
						//  Stuff the partial MCU into the pipeline
						Output->next(NewMCU);
					}

					//  Send the End-Of-Stream signal to the pipeline
					Output->signalEndOfStream();

					//  Return to caller
					return;
				}

			private:

				//*******************************************************************************************************************
				//*																													*
				//*  Private Members																								*
				//*																													*
				//*******************************************************************************************************************

				//  Minimum Coding Unit (MCU)
				MCU								NewMCU;
				BYTE							MCUFF;												//  MCU Form Factor

				//  Output Object
				MCUSplitter*					Output;												//  MCU Splitter object

				//  Processing Indexes
				int								rIndex;												//  Row index (in virtual MCU)
				int								cIndex;												//  Column index (in virtual MCU)

				//  Instrumentation
				UINT							MCUStuffed;											//  Count of MCUs stuffed

			};

			//*******************************************************************************************************************
			//*                                                                                                                 *
			//*   Public Functions                                                                                              *
			//*                                                                                                                 *
			//*******************************************************************************************************************

			//  Configuration Functions

			//  Configuration Functions
			void		setQuantizer(int Channel, JPEGQuantizer* pNewDQ) { MCUS.setQuantizer(pNewDQ, Channel); return; }
			void		setDCEncoder(int Channel, Huffman::HuffmanTree* pNewDec) { MCUS.setDCHuffmanTree(pNewDec, Channel); return; }
			void		setACEncoder(int Channel, Huffman::HuffmanTree* pNewDec) { MCUS.setACHuffmanTree(pNewDec, Channel); return; }
			void		setHSPM(int Channel, int Samples) { MCUS.setHSF(Samples, Channel); return; }
			void		setVSPM(int Channel, int Samples) { MCUS.setVSF(Samples, Channel); return; }
			void		setPrecision(int Precision) { DSDU.setPrecision(Precision); return; }
			void		setMCUFF(BYTE NewFormFactor) { MCUFF = NewFormFactor; MCUS.setMCUFF(NewFormFactor); return; }

			//  encode
			//
			//  Connects the input and output ends of the encoding pipeline
			//
			//  PARAMETERS
			//
			//		HuffmanCODEC::Collector*		-			Pointer to the Huffman CODEC Collector that provides the sink for the encoded image
			//		BYTE							-			The MCU Form Factor
			//
			//  RETURNS
			//
			//		Collector						-			The Collector that accepts the pixel stream of the unencoded image
			//
			//  NOTES
			//
			//	Before using he emitter callers MUST setup all of the configuration elements for the decode operation :-
			//
			//		1.			JPEGQuantizer for each channel.
			//		2.			DC Huffman Trees for each channel.
			//		3.			AC Huffman Trees for each channel.
			//		4.			Horizontal and Vertical sampling factors for each channel.
			//		5			Precision (Channel width in bits)
			//
			//	The Encoder ONLY supports MCU Form Factors 0f 0x11 and 0x22.
			//

			Collecter		encode(Huffman::JPEGCollecter* HCCollector, BYTE FF) {

				//  Connect the end of the pipline to the Huffman CODEC collecter
				SDU.setOutput(HCCollector);

				//  Return the collecter 
				return Collecter(&MCUS, FF);
			}

		private:

			//*******************************************************************************************************************
			//*																													*
			//*  Private Members																								*
			//*																													*
			//*******************************************************************************************************************

			//  Encoding Pipeline Components
			MCUSplitter			MCUS;																	//  MCU Splitter
			CMCUSplitter		CMCUS;																	//  Channel MCU Splitter
			Downshifter			DSDU;																	//  DU down shifter
			DCT					DCTDU;																	//  Discrete Cosine Transform
			ZigZagger			ZZDU;																	//  Zig Zagger
			Quantizer			QDU;																	//  Quantizer
			DUSplitter			SDU;																	//  DU Sppitter

			//  MCU Form Factor
			BYTE				MCUFF;		
		};

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   JPEGHuffmanTree Class																							*
		//*                                                                                                                 *
		//*   Objects of this class provide the a constructor for a Huffman::HuffmanTree object.							*
		//*   The Huffman Tree is constructed from a DHT Huffman Table definition.											*
		//*                                                                                                                 *
		//*******************************************************************************************************************

		class JPEGHuffmanTree : public Huffman::HuffmanTree {
		public:
			//*******************************************************************************************************************
			//*                                                                                                                 *
			//*   Type Definitions		                                                                                        *
			//*                                                                                                                 *
			//*******************************************************************************************************************

			typedef		Huffman::HuffmanTree::HuffmanNode		NODE;

			//*******************************************************************************************************************
			//*                                                                                                                 *
			//*   Forward Declarations                                                                                          *
			//*                                                                                                                 *
			//*******************************************************************************************************************

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

			JPEGHuffmanTree(const JFIF_HTAB& HT)
				: Huffman::HuffmanTree() {

				int			MaxLevel = 1;														//  Maximum level of the tree
				int			iIndex;																//  Generic index
				int			SymbolsAdded = 0;													//  Total count of symbols added
				int			SymbolsThisLevel;													//  Number of symbols to add at this level
				NODE*		Root = NULL;														//  Root node

				//  Find the maximum level in the tree
				for (iIndex = 15; iIndex > 0; iIndex--) {
					if (HT.HTL[iIndex] > 0) {
						MaxLevel = iIndex + 1;
						break;
					}
				}

				//  Add A root node to the tree
				Root = new NODE(false, NULL, 0);
				setRootNode(Root);

				//  Process each level in the tree in turn
				for (iIndex = 1; iIndex <= MaxLevel; iIndex++) {
					//  Set the number of symbols to be added at this level
					SymbolsThisLevel = HT.HTL[iIndex - 1];

					//  Perform an explosion to add the nodes to the tree at the required level
					explodeNode(*Root, iIndex, &SymbolsThisLevel, &SymbolsAdded, HT, 0);
				}

				//  Return to caller
				return;
			}

			//*******************************************************************************************************************
			//*                                                                                                                 *
			//*   Public Functions                                                                                              *
			//*                                                                                                                 *
			//*******************************************************************************************************************

			//  serialise
			//
			//	This function will construct a serialised for of the tree definition and return a buffer with the definition.
			//
			//	PARAMETERS
			//
			//		BYTE						-	The encoded Class and Destination of the table
			//		size_t&						-	Refernce to the variable that will hold the length of the table
			//
			//  RETURNS
			//
			//		BYTE*						-	Pointer to the buffer containing the table
			//
			//	NOTES
			//
			//

			BYTE*		serialize(BYTE CandD, size_t& TSize) {
				BYTE*		pTab = nullptr;																//  Pointer to the buffer
				BYTE*		pCounter = nullptr;															//  Pointer to the first counter
				BYTE*		pEntry = nullptr;															//  Pointer to the next entry to be filled
				BYTE		Filled[16] = {};															//  Array of filled entries of each length

				//  Allocate the buffer (max possible size)
				TSize = 0;
				pTab = (BYTE*) malloc(size_t(1) + size_t(16) + size_t(256));
				if (pTab == nullptr) return nullptr;
				memset(pTab, 0, size_t(1) + size_t(16) + size_t(256));

				//  Set the class and destination
				*pTab = CandD;

				//  Setup to start encoding the table definition
				TSize = 17;																				//  Class and Destination plus 16 counters
				pCounter = pTab + 1;
				pEntry = pTab + 17;

				//
				//  Make two passes (indented explosions) over the tree. The first pass will fill in the counters and the second
				//  will fill in the entries.
				//

				countEntries(*getRootNode(), pCounter, 0);

				fillEntries(*getRootNode(), pCounter, Filled, pEntry, 0);

				//  Update the size of the complete table
				for (int SX = 0; SX < 16; SX++) TSize += Filled[SX];

				//  Return the pointer to the buffer with the definition
				return pTab;
			}

		private:

			//*******************************************************************************************************************
			//*                                                                                                                 *
			//*   Private Functions                                                                                             *
			//*                                                                                                                 *
			//*******************************************************************************************************************

			//  countEntries
			//
			//	This function will explode the node passed - for each leaf encountered it will increment the count of
			//  codes of the length (depth in the tree).
			//
			//	PARAMETERS
			//
			//		NODE&						-	Reference to the node to be exploded
			//		BYTE*						-	Pointer to the count of codes of given length
			//		int							-	Current level
			//
			//  RETURNS
			//
			//		bool		-		true if the explosion should terminate
			//
			//	NOTES
			//
			//

			bool		countEntries(HuffmanNode& Node, BYTE* Counters, int Level) {

				//  Recursion protection - if the levels greater than 16 this is a malformed (probably looping) tree
				//  Signal the parent to terminate the explosion.
				if (Level > 16) return true;

				//  If the node is a leaf then update the Counter array
				if (Node.isLeaf()) {
					Counters[Level - 1]++;
					return false;
				}

				//  The node is a branch, recurse first into the zero path then the one path, percolating a stop signal
				if (Node.getZero() != nullptr) {
					if (countEntries(*(Node.getZero()), Counters, Level + 1)) return true;
				}

				if (Node.getOne() != nullptr) {
					if (countEntries(*(Node.getOne()), Counters, Level + 1)) return true;
				}

				//  Return to the next level up to continuse the explosion
				return false;
			}

			//  fillEntries
			//
			//	This function will explode the node passed - for each leaf encountered it will insert the symbol into
			//  appropriate slot in the table.
			//
			//	PARAMETERS
			//
			//		NODE&						-	Reference to the node to be exploded
			//		BYTE*						-	Pointer to the count of codes of given length
			//		BYTE*						-	Pointer to the count of codes filled of a given length
			//		BYTE*						-	Pointer to the array of entries
			//		int							-	Current level
			//
			//  RETURNS
			//
			//		bool		-		true if the explosion should terminate
			//
			//	NOTES
			//
			//

			bool		fillEntries(HuffmanNode& Node, BYTE* Counters, BYTE* Filled, BYTE* Entries, int Level) {
				int			Index = 0;

				//  Recursion protection - if the levels greater than 16 this is a malformed (probably looping) tree
				//  Signal the parent to terminate the explosion.
				if (Level > 16) return true;

				//  If the node is a leaf then update the Counter array
				if (Node.isLeaf()) {
					//  Compute where the entry should be placed
					//  Index = Index of first entry for the length + Count of entries of that length already filled

					for (int LX = 1; LX < Level; LX++) Index += Counters[LX - 1];
					Index += Filled[Level - 1];
					Entries[Index] = Node.getSymbol();
					Filled[Level - 1]++;
					return false;
				}

				//  The node is a branch, recurse first into the zero path then the one path, percolating a stop signal
				if (Node.getZero() != nullptr) {
					if (fillEntries(*(Node.getZero()), Counters, Filled, Entries,  Level + 1)) return true;
				}

				if (Node.getOne() != nullptr) {
					if (fillEntries(*(Node.getOne()), Counters, Filled, Entries, Level + 1)) return true;
				}

				//  Return to the next level up to continuse the explosion
				return false;
			}

			//  explodeNode
			//
			//	This function will explode the node passed - if the node is at the target level then it inserts the child nodes
			//
			//	PARAMETERS
			//
			//		NODE&						-	Reference to the node to be exploded
			//		int							-	Target level
			//		*int						-	Pointer to the count of symbols to be added at this level
			//		*int						-   Pointer to the total number of symbols added
			//		JFIF_HTAB&					-	Const reference to the Huffman table being loaded
			//		int							-	Current level
			//		
			//
			//  RETURNS
			//
			//	NOTES
			//
			//

			void explodeNode(NODE& Node, int Target, int* pSTL, int* pSyms, const JFIF_HTAB& HT, int Level) {
				NODE* pNewNode;

				//  If this is a leaf node then no more to do
				if (Node.isLeaf()) return;

				//  If we are above the target level then just recurse into the next level down
				if (Level < (Target - 1)) {
					//  Explode Zero followed by One
					explodeNode(*Node.getZero(), Target, pSTL, pSyms, HT, Level + 1);
					explodeNode(*Node.getOne(), Target, pSTL, pSyms, HT, Level + 1);
					return;
				}

				//  We are at the target level to build - first build the Zero Child Node
				if (*pSTL > 0) {
					pNewNode = new NODE(true, &Node, HT.HTEntry[(*pSyms)++]);
					(*pSTL)--;
				}
				else pNewNode = new NODE(false, &Node, 0);
				Node.setZero(pNewNode);

				if (*pSTL > 0) {
					pNewNode = new NODE(true, &Node, HT.HTEntry[(*pSyms)++]);
					(*pSTL)--;
				}
				else pNewNode = new NODE(false, &Node, 0);
				Node.setOne(pNewNode);

				//  Return to caller
				return;
			}

		};

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Private Types		                                                                                            *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  JPEG Resource Directory
		typedef struct JRD {
			BYTE					MCUFF;														//  MCU Form Factor
			BYTE					QUse[3];													//  Quantisation Table Use (by channel)
			JPEGQuantizer*			pQ[4];														//  (De)Quantizers
			BYTE					DCUse[3];													//  DC Huffman Tree Use (by Channel)
			JPEGHuffmanTree*		pHTDC[4];													//  DC Huffman Tree
			BYTE					ACUse[3];													//  AC Huffman Tree Use (by Channel)
			JPEGHuffmanTree*		pHTAC[4];													//  AC Huffman Tree
			int						HSF[3];														//  Horizontal Sampling factor
			int						VSF[3];														//  Vertical Sampling factor
		} JRD;

	public:

		//  Prevent Instantiation
		JFIF() = delete;

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
		//  This static function will store the passed RGB Train into the designated JFIF (JPEG) Image file
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
		//  This static function will store the passed RGB Train into the designated JFIF (JPEG) Image file
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
		//  This static function will store the passed RGB Train into the designated JFIF (JPEG) Image file
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
				std::cerr << "ERROR: Unable to build an JFIF/JPEG on-disk image for: " << ImgName << "." << std::endl;
				if (pImage != nullptr) free(pImage);
				return false;
			}

			//  Store the in-memory image  (consumes the image memory allocation)
			if (!VRMap.storeResource(ImgName, pImage, ImgSize)) {
				std::cerr << "ERROR: Failed to store JFIF/JPEG image: '" << ImgName << "', (" << ImgSize << " bytes)." << std::endl;
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
		//  additionally document the contents of the image.
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
		//	and optionally document the image contents.
		//
		//  PARAMETERS
		//
		//		bool			-		If true document the image contents, otherwise only document the On-Disk image
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
			size_t			Offset = 0;															//  Offset in the file
			size_t			BlockNo = 0;														//  Block number
			size_t			FrameNo = 0;														//  Frame number
			Train<RGB>*		pITrain = nullptr;													//  Pointer to the train holding the image
			char			szFileName[MAX_PATH + 1] = {};										//  File name

			//  Show starting analysis
			OS << "JFIF(JPEG): ON-DISK-IMAGE ANALYSIS" << std::endl;
			OS << "==================================" << std::endl;
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

			//  Map the image
			Map.Image = pImage;
			Map.ImageSize = ImgSize;
			Map.NumBlocks = 0;
			Map.NBA = 0;
			Map.Blocks = nullptr;

			if (!mapImage(Map)) {
				OS << "ERROR: The On-Disk-Image is not a valid JFIF Image." << std::endl;
				free(pImage);
				return;
			}

			OS << "Image map contains: " << Map.NumBlocks << " image blocks." << std::endl;

			//  Report on each block in turn
			for (BlockNo = 0; BlockNo < Map.NumBlocks; BlockNo++) {

				switch (Map.Blocks[BlockNo].BlockType) {
				case JFIF_BLOCK_FH:
					showFileHeader(BlockNo, Offset, Map, OS);
					break;

				case JFIF_BLOCK_APP0:
					showApp0(BlockNo, Offset, Map, OS);
					break;

				case JFIF_BLOCK_EXIF:
					showEXIF(BlockNo, Offset, Map, OS);
					break;

				case JFIF_BLOCK_DQT:
					showDQT(BlockNo, Offset, Map, OS);
					break;

				case JFIF_BLOCK_DHT:
					showDHT(BlockNo, Offset, Map, OS);
					break;

				case JFIF_BLOCK_SOF0:
					FrameNo++;
					showSOF0(BlockNo, FrameNo, Offset, Map, OS);
					break;

				case JFIF_BLOCK_SOFX:
					FrameNo++;
					showSOFX(BlockNo, FrameNo, Offset, Map, OS);
					break;

				case JFIF_BLOCK_SOS:
					showSOS(BlockNo, Offset, Map, OS);
					break;

				case JFIF_BLOCK_EEB:
					showEEB(BlockNo, Offset, Map, OS);
					break;

				case JFIF_BLOCK_RST:
					showRST(BlockNo, Offset, Map, OS);
					break;

				case JFIF_BLOCK_FT:
					showFT(BlockNo, Offset, Map, OS);
					break;

				default:
					OS << std::endl;
					OS << "UNKNOWN BLOCK TYPE'" << Map.Blocks[BlockNo].BlockType << "' - Block: " << BlockNo + 1 << ", size: " << Map.Blocks[BlockNo].BlockSize << "." << std::endl;
					break;
				}

				//  Update the offset
				Offset += Map.Blocks[BlockNo].BlockSize;
			}

			//  Free the image
			free(pImage);

			//  Free the image map
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
			Train<YCbCr>*	pCTrain = nullptr;											//  Canonical Train (Writeable)

			//  Auto adjust the Train Canvas Size
			pTrain->autocorrect();

			//  Construct the Canonical Train from the input train. The Canonical Train is directly writeble as a JPEG image.
			//  The Canonical train IS YCbCr colourspace encoded.
			//  The train is a flttened (single frame) train;
			//  The dimensions of the frame will be expanded to complete MCU boundaries, the original size is preseved in the Train

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
			JRD				ResDir = {};														//  Resource Directory
			size_t			BlockNo = 0;														//  Block Index

			//  Build the map of the image
			Map.Image = pImage;
			Map.ImageSize = Size;
			Map.NumBlocks = 0;
			Map.NBA = 0;
			Map.Blocks = nullptr;

			if (!mapImage(Map)) return nullptr;

			//  Create the train
			pTrain = new Train<xymorg::RGB>();

			//  Capture each of the image frames in turn and append to the train
			while (BlockNo < Map.NumBlocks) {

				//  Switch according to the block type - processing only blocks that are of interest at this level (above a frame)
				switch (Map.Blocks[BlockNo].BlockType) {
				case JFIF_BLOCK_DQT:
					//  Define a new Quantisation table
					addQuantizer(Map, BlockNo, ResDir);

					//  Move to the next block
					BlockNo++;
					break;

				case JFIF_BLOCK_DHT:
					//  Define a new Huffman Tree
					addHuffmanTree(Map, BlockNo, ResDir);

					//  Move to the next block
					BlockNo++;
					break;

				case JFIF_BLOCK_SOF0:
					//  Start of Frame (baseline DCT)
					BlockNo += addFrame(Map, BlockNo, ResDir, pTrain);
					break;

				case JFIF_BLOCK_SOFX:
					//  Alternate Frame - skipped
					BlockNo += skipFrame(Map, BlockNo);
					break;

				default:
					//  Not interested in other block types (at this level) - move on to the next block
					BlockNo++;
					break;
				}
			}

			//  Purge any accumulated resources from the directory
			for (size_t RX = 0; RX < 4; RX++) {
				if (ResDir.pQ[RX] != nullptr) delete ResDir.pQ[RX];
				if (ResDir.pHTDC[RX] != nullptr) delete ResDir.pHTDC[RX];
				if (ResDir.pHTAC[RX] != nullptr) delete ResDir.pHTAC[RX];
			}

			//  Free the map
			free(Map.Blocks);

			//  Auto adjust the train
			pTrain->autocorrect();

			//  Return the unbuttoned frame
			return pTrain;
		}

		//  addFrame
		//
		//  This static function will add a new Frame to the image Train
		//
		//  PARAMETERS
		//
		//		Map&					-		Reference to the map of the in-memory image
		//		size_t					-		Block index of the first block of the frame
		//		JRD&					-		Reference to the resource directory
		//		pTrain*					-		Pointer to the train to be extended
		//
		//  RETURNS
		//
		//		size_t					-		Number of blocks consumed by this frame
		//
		//  NOTES
		//

		static size_t		addFrame(ODIMap& Map, size_t BlockNo, JRD& ResDir, Train<RGB>* pTrain) {
			size_t					BlocksConsumed = 0;												//  Number of blocks consumed by the frame
			JFIF_FRAME_HEADER*		pSOF = (JFIF_FRAME_HEADER*) Map.Blocks[BlockNo].Block;			//  Start of Frame block
			JFIF_SCAN_HEADER1*		pSH = nullptr;													//  Start of Scan block
			JFIF_SCAN_HEADER2*		pSHX = nullptr;													//  Start of Scan Extension block
			BYTE*					pBuffer = nullptr;												//  Address of the image buffer
			size_t					BufferSize = 0;													//  Size of the image buffer
			int						MaxHS = 0;														//  Max horizontal samples
			int						MaxVS = 0;														//  Max vertical samples
			size_t					FrameH = 0;														//  Frame Height
			size_t					FrameW = 0;														//  Frame Width
			size_t					ScanH = 0;														//  Scan Height
			size_t					ScanW = 0;														//  Scan Width
			RasterBuffer<RGB>*		pRB = nullptr;													//  Pointer to the Raster Buffer
			DecoderPipeline			Pipe;															//  Decoder Pipeline
			Huffman					EDC;															//  Entropy Decoder

			//  If the sampling precision is not 8 bits or the colour components is not 3 then skip this frame
			if (pSOF->Precision != 8 || pSOF->Components != 3) {
				std::cerr << "ERROR: Attempting to decode a JPEG frame that does NOT have a 3x8 colour scheme, skipping frame." << std::endl;
				BlocksConsumed = 1;
				while ((BlockNo + BlocksConsumed) < Map.NumBlocks) {
					if (Map.Blocks[BlockNo + BlocksConsumed].BlockType == JFIF_BLOCK_SOF0 || Map.Blocks[BlockNo + BlocksConsumed].BlockType == JFIF_BLOCK_SOFX) break;
					BlocksConsumed++;
				}
				return BlocksConsumed;
			}

			//  Prepare the pipeline for the frame
			Pipe.setPrecision(pSOF->Precision);

			//  Condition the pipe for each channel
			for (size_t CX = 0; CX < 3; CX++) {
				//  Set the Horizontal and Vertical Sampling Factor
				Pipe.setHSPM(int(CX), GetHSampfactor(pSOF->Comp[CX].HandV));
				Pipe.setVSPM(int(CX), GetVSampFactor(pSOF->Comp[CX].HandV));

				//  Determine the max sampling
				if (GetHSampfactor(pSOF->Comp[CX].HandV) > MaxHS) MaxHS = GetHSampfactor(pSOF->Comp[CX].HandV);
				if (GetVSampFactor(pSOF->Comp[CX].HandV) > MaxVS) MaxVS = GetVSampFactor(pSOF->Comp[CX].HandV);

				//  Set the dequantizer to use
				Pipe.setDequantizer(int(CX), ResDir.pQ[pSOF->Comp[CX].QTable]);
			}

			//  Determine the MCU Form Factor that will be used
			ResDir.MCUFF = BYTE((MaxHS << 4) + MaxVS);
			if (MaxHS == 1) MaxHS = 8;
			else MaxHS = 16;
			if (MaxVS == 1) MaxVS = 8;
			else MaxVS = 16;

			//  Capture the frame and scan height and width
			FrameH = GetSizeBE(pSOF->HLines);
			FrameW = GetSizeBE(pSOF->VLines);
			if (FrameH & (size_t(MaxVS) - size_t(1))) ScanH = MaxVS;
			if (FrameW & (size_t(MaxHS) - size_t(1))) ScanW = MaxHS;
			ScanH += (FrameH & ~(size_t(MaxVS) - size_t(1)));
			ScanW += (FrameW & ~(size_t(MaxHS) - size_t(1)));
			BlocksConsumed++;

			//  Set the MCU Form factor in the pipeline
			Pipe.setMCUFF(ResDir.MCUFF);

			//
			//  Cycle through the following blocks setting up the decode pipeline ready to decode 
			//
			while ((BlockNo + BlocksConsumed) < Map.NumBlocks) {
				if (Map.Blocks[BlockNo + BlocksConsumed].BlockType == JFIF_BLOCK_SOF0 || Map.Blocks[BlockNo + BlocksConsumed].BlockType == JFIF_BLOCK_SOFX) break;
				if (Map.Blocks[BlockNo + BlocksConsumed].BlockType == JFIF_BLOCK_FT) break;
				
				switch (Map.Blocks[BlockNo + BlocksConsumed].BlockType) {

				case JFIF_BLOCK_DQT:
					//  Define new quantizer
					addQuantizer(Map, BlockNo + BlocksConsumed, ResDir);
					BlocksConsumed++;
					break;

				case JFIF_BLOCK_DHT:
					//  Define new Huffman Tree
					addHuffmanTree(Map, BlockNo + BlocksConsumed, ResDir);
					BlocksConsumed++;
					break;

				case JFIF_BLOCK_SOS:
					//  Start of Scan - prepare for performing the image decoding
					pSH = (JFIF_SCAN_HEADER1*) Map.Blocks[BlockNo + BlocksConsumed].Block;
					pSHX = (JFIF_SCAN_HEADER2*) (&pSH->Comp[pSH->Components]);

					for (size_t CX = 0; CX < 3; CX++) {
						Pipe.setDCDecoder(int(CX), ResDir.pHTDC[GetDCSelector(pSH->Comp[CX].DCandAC)]);
						Pipe.setACDecoder(int(CX), ResDir.pHTAC[GetACSelector(pSH->Comp[CX].DCandAC)]);
					}
					BlocksConsumed++;
					break;

				case JFIF_BLOCK_EEB:
					//  Accumulate the total size of the input buffer and capture the address of the first block
					if (pBuffer == nullptr) pBuffer = Map.Blocks[BlockNo + BlocksConsumed].Block;
					BufferSize += Map.Blocks[BlockNo + BlocksConsumed].BlockSize;
					BlocksConsumed++;
					break;

				case JFIF_BLOCK_RST:
					//  Include the restart interval in the buffer length
					BufferSize += Map.Blocks[BlockNo + BlocksConsumed].BlockSize;
					BlocksConsumed++;
					break;
				}
			}

			//  Setup a raster buffer to hold the decoded output.
			//  NOTE: The ouput is rounded up (height and width) to accomodate complete MCUs

			pRB = new RasterBuffer<RGB>(ScanH, ScanW, nullptr);
			
			//  Setup a stuffed byte stream to provide the input image
			StuffedStream		bsIn(pBuffer, BufferSize);

			//
			//  Arrange the plumbing for the input and output:
			//
			//		StuffedStream ==> Huffman ==> Huffman::Emitter ==> Pipeline ==> Pipeline::Emitter ==> RasterBuffer
			//

			Huffman::JPEGEmitter		Source = EDC.decodeJPEG(bsIn);
			DecoderPipeline::Emitter	Sink = Pipe.decode(&Source);

			//
			//  Populate the Raster Buffer. Use an iterator that operates in sequential MCU order over the buffer (matching the decode sequence).
			//  Pixels are returned one at a time in the YCbCr colour space so must be converted to RGB before adding to the Raster Buffer.
			//

			for (RasterBuffer<RGB>::iterator It = pRB->firstMCU(ResDir.MCUFF); It != pRB->lastMCU(ResDir.MCUFF); It++) {
				if (Sink.hasNext()) {
					*It = ColourConverter::convertToRGB(Sink.next());
				}
				else {
					std::cerr << "ERROR: JPEG decoding stream has terminated before filling an image buffer." << std::endl;
					break;
				}
			}

			//  If the extracted image was larger than the actual image size (MCU boundaries) then resize the image
			if (ScanH > FrameH || ScanW > FrameW) {
				SizeVector		svCrop = {};

				svCrop.Bottom = int(FrameH - ScanH);
				svCrop.Right = int(FrameW - ScanW);

				pRB->resize(svCrop, nullptr);
			}

			//  Create a frame to carry the Raster Buffer and append it to the train
			pTrain->append(new Frame<RGB>(pRB, 0, 0, nullptr));

			//  Return the number of blocks consumed
			return BlocksConsumed;
		}

		//  skipFrame
		//
		//  This static function will skip over an unsupported frame
		//
		//  PARAMETERS
		//
		//		Map&					-		Reference to the map of the in-memory image
		//		size_t					-		Block index of the first block of the frame
		//
		//  RETURNS
		//
		//		size_t					-		Number of blocks consumed by this frame
		//
		//  NOTES
		//

		static size_t		skipFrame(ODIMap& Map, size_t BlockNo) {
			size_t					BlocksConsumed = 0;												//  Number of blocks consumed by the frame

			BlocksConsumed = 1;
			while ((BlockNo + BlocksConsumed) < Map.NumBlocks) {
				if (Map.Blocks[BlockNo + BlocksConsumed].BlockType == JFIF_BLOCK_SOF0 || Map.Blocks[BlockNo + BlocksConsumed].BlockType == JFIF_BLOCK_SOFX) break;
				if (Map.Blocks[BlockNo + BlocksConsumed].BlockType == JFIF_BLOCK_FT) break;
				BlocksConsumed++;
			}
			return BlocksConsumed;
		}

		//  addQuantizer
		//
		//  This static function will add a new Quantizer to the resource directory
		//
		//  PARAMETERS
		//
		//		Map&					-		Reference to the map of the in-memory image
		//		size_t					-		Block index of the first block of the frame
		//		JRD&					-		Reference to the resource directory
		//
		//  RETURNS
		//
		//  NOTES
		//
		//		A quantizer table block MAY contain multiple tables
		//

		static void		addQuantizer(ODIMap& Map, size_t BlockNo, JRD& ResDir) {
			BYTE*				pNextTable = Map.Blocks[BlockNo].Block + 4;
			size_t				Residue = Map.Blocks[BlockNo].BlockSize - 4;					//  Remaining space for tables

			while (Residue >= sizeof(JFIF_QV8_ARRAY)) {
				size_t				Precision = GetPrecision(*pNextTable);						//  Precision of the next table
				size_t				QX = GetDestination(*pNextTable);							//  The identifying destination

				//  If the table is already in use then delete the existing incumbent
				if (QX <= 3) {
					if (ResDir.pQ[QX] != nullptr) delete ResDir.pQ[QX];
				}

				if (Precision == 0) {
					//  8 bit table
					//  Build a new Quantizer for the selected destination
					if (QX <= 3) ResDir.pQ[QX] = new JPEGQuantizer(pNextTable);
					pNextTable += sizeof(JFIF_QV8_ARRAY);
					Residue -= sizeof(JFIF_QV8_ARRAY);
				}
				else {
					//  16 bit table
					//  Build a new Quantizer for the selected destination
					if (QX <= 3) ResDir.pQ[QX] = new JPEGQuantizer(pNextTable);
					pNextTable += sizeof(JFIF_QV16_ARRAY);
					Residue -= sizeof(JFIF_QV16_ARRAY);
				}
			}

			//  return to caller
			return;
		}

		//  addHuffmanTree
		//
		//  This static function will add a new HuffmanTree (DC or AC) to the resource directory
		//
		//  PARAMETERS
		//
		//		Map&					-		Reference to the map of the in-memory image
		//		size_t					-		Block index of the first block of the frame
		//		JRD&					-		Reference to the resource directory
		//
		//  RETURNS
		//
		//  NOTES
		//

		static void		addHuffmanTree(ODIMap& Map, size_t BlockNo, JRD& ResDir) {
			JFIF_HTAB*		pHT = (JFIF_HTAB*) Map.Blocks[BlockNo].Block;							//  Pointer to the definition table block
			size_t			Class = GetClass(pHT->CandD);											//  Class (AC or DC)
			size_t			HX = GetDestination(pHT->CandD);										//  The identifying destination

			//  DC Class Table
			if (HX <= 3) {
				if (Class == 0) {

					//  If this resource is already allocated then delete the existing instantiation
					if (ResDir.pHTDC[HX] != nullptr) delete ResDir.pHTDC[HX];

					//  Build a new DC Huffman Tree for the selected destination
					ResDir.pHTDC[HX] = new JPEGHuffmanTree(*pHT);
				}
				else {
					//  If this resource is already allocated then delete the existing instantiation
					if (ResDir.pHTAC[HX] != nullptr) delete ResDir.pHTAC[HX];

					//  Build a new DC Huffman Tree for the selected destination
					ResDir.pHTAC[HX] = new JPEGHuffmanTree(*pHT);
				}
			}

			//  Return to caller
			return;
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

		static Train<YCbCr>* buildCanonicalTrain(Train<RGB>* pTrain, SWITCHES Opts) {
			Train<RGB>*				pITrain = nullptr;														//  Flattened Image
			Train<YCbCr>*			pCTrain = nullptr;														//  YCbCr version
			YCbCr					Background =  ColourConverter::convertToYCbCr(pTrain->getBackGround()); //  Background colour
			Frame<RGB>*				pIFrame = nullptr;														//  Source frame
			size_t					IH = 0;																	//  Input Height
			size_t					IW = 0;																	//  Input Width
			size_t					OH = 0;																	//  Output Height
			size_t					OW = 0;																	//  Output Width
			RasterBuffer<YCbCr>*	pNewRB = nullptr;														//  Target Raster Buffer
			YCbCr					LastPixel = {};															//  Last pixel converted
			BYTE					MCUFF = 0;																//  MCU Form Factor

			//  Determine the MCU Form Factor to use
			if (Opts & JFIF_STORE_OPT_HIFI) MCUFF = 0x11;
			else MCUFF = 0x22;

			//  For JPEG images the canonical image is a flattened copy of the input train
			pITrain = new Train<RGB>(*pTrain);
			pITrain->flatten();

			//  Form the output train in the YCbCr colour space
			pCTrain = new Train<YCbCr>(pTrain->getCanvasHeight(), pTrain->getCanvasWidth(), &Background);

			//  Prepare to convert the frame
			pIFrame = pITrain->getFirstFrame();

			IH = pIFrame->getHeight();
			IW = pIFrame->getWidth();

			if (MCUFF == 0x22) {
				OH = IH & 0xFFFFFFF0;
				OW = IW & 0xFFFFFFF0;

				if (IH & 15) OH += 16;
				if (IW & 15) OW += 16;
			}
			else {
				OH = IH & 0xFFFFFFF8;
				OW = IW & 0xFFFFFFF8;

				if (IH & 7) OH += 8;
				if (IW & 7) OW += 8;
			}

			//  Construct the target Raster Buffer
			pNewRB = new RasterBuffer<YCbCr>(OH, OW, &Background);

			//
			//  Copy the source Raster Buffer to the target, converting the colour space to YCbCr.
			//  If filling is needed then the last coloumn and last row of the source are used
			//

			//  Source Buffer Iterators
			RasterBuffer<RGB>::iterator		SRIt = pIFrame->buffer().top();
			
			RasterBuffer<YCbCr>::iterator	LastRow = pNewRB->top();

			//  Iterate over the target
			for (RasterBuffer<YCbCr>::iterator TRIt = pNewRB->top(); TRIt != pNewRB->bottom(); TRIt++) {

				if (SRIt != pIFrame->buffer().bottom()) {
					//  Copy the row from the source extending to the right with the last pixel
					RasterBuffer<RGB>::iterator		SCIt = pIFrame->buffer().left(SRIt);

					for (RasterBuffer<YCbCr>::iterator TCIt = pNewRB->left(TRIt); TCIt != pNewRB->right(TRIt); TCIt++) {

						if (SCIt != pIFrame->buffer().right(SRIt)) LastPixel = ColourConverter::convertToYCbCr(*SCIt);
						*TCIt = LastPixel;
						SCIt++;
					}

					//  If we are before the last row of the source increment the last row iterator
					if (!SRIt.isLast()) LastRow++;
					SRIt++;
				}
				else {
					//  Copy the last row in the target that was copied from the source
					RasterBuffer<YCbCr>::iterator		SCIt = pNewRB->left(LastRow);
					for (RasterBuffer<YCbCr>::iterator TCIt = pNewRB->left(TRIt); TCIt != pNewRB->right(TRIt); TCIt++) *TCIt = *SCIt;
				}
			}

			//  Append a frame to the target train containing the new raster buffer
			pCTrain->append(new Frame<YCbCr>(pNewRB, 0, 0, nullptr));

			//  Delete the intermediate train
			delete pITrain;

			//  Return the constructed canonical train
			return pCTrain;
		}

		//  serialiseTrain
		//
		//  This static function will build an in-memory JFIF/JPEG image from the passed CANONICAL Train.
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
		//		The input Train MUST be a JPEG CANONICAL Train
		//

		static BYTE* serialiseTrain(Train<YCbCr>* pTrain, size_t& ImgSize, SWITCHES Opts) {
			BYTE*					pImage = nullptr;											//  Pointer to in-memory image
			size_t					ImgAlc = 0;													//  Allocated size of the in-memory image
			size_t					ImgUsed = 0;												//  Bytes used in the in-memory image
			size_t					ImgEst = 0;													//  (Over) Estimated initial size of the image
			JRD						ResDir = {};												//  Resource Directory
			
			//  Allocate memory for the image
			ImgEst = 4096 + (256 * sizeof(YCbCr)) + ((pTrain->getCanvasHeight() + 4) * (pTrain->getCanvasWidth() + 4) * sizeof(YCbCr));
			ImgEst += ((size_t(64) * size_t(3)) + (size_t(256) * size_t(6)));
			ImgAlc = 0;
			ImgUsed = 0;
			pImage = (BYTE*) malloc(ImgEst);
			if (pImage == nullptr) return nullptr;
			ImgAlc = ImgEst;
			memset(pImage, 0, ImgEst);

			//  Select the resources to use for image encoding (Quantisation tables and Huffman Trees)
			selectResources(pTrain, ResDir, Opts);

			//  Append the file header to the image
			appendFileHeader(pImage, ImgUsed);

			//  Appent the Basic (APP0) header
			appendBasicHeader(pImage, ImgUsed);

			//  Append the Quantisation Tables used
			appendQuantisationTables(pImage, ImgUsed, ResDir);

			//  Append Start Of Frame (SOF0)
			appendStartOfFrame(pTrain, pImage, ImgUsed, ResDir, Opts);

			//  Append the Huffman Tree Table definitions that will be used
			appendHuffmanTrees(pImage, ImgUsed, ResDir, Opts);

			//  Append the Start Of Scan block
			appendStartOfScan(pImage, ImgUsed, ResDir, Opts);

			//  Append the Entropy Encoded Block (Image)
			appendImage(pTrain, pImage, ImgUsed, ResDir, Opts);

			//  Append the File Trailer block to the image
			appendFileTrailer(pImage, ImgUsed);

			//  Release unused memory in the image block
			if ((ImgAlc - ImgUsed) > 256) {
				BYTE* pNewIBfr = (BYTE*) realloc(pImage, ImgUsed);
				if (pNewIBfr != nullptr) {
					pImage = pNewIBfr;
					ImgAlc = ImgUsed;
				}
			}

			//
			//  Purge the accumulated resources from the directory
			//

			for (int CX = 0; CX < 3; CX++) {

				if (ResDir.pQ[CX] != nullptr) {
					 delete ResDir.pQ[CX];
					if (ResDir.pQ[CX + 1] == ResDir.pQ[CX]) ResDir.pQ[CX + 1] = nullptr;
					ResDir.pQ[CX] = nullptr;
				}

				if (ResDir.pHTDC[CX] != nullptr) {
					delete ResDir.pHTDC[CX];
					if (ResDir.pHTDC[CX + 1] == ResDir.pHTDC[CX]) ResDir.pHTDC[CX + 1] = nullptr;
					ResDir.pHTDC[CX] = nullptr;
				}

				if (ResDir.pHTAC[CX] != nullptr) {
					delete ResDir.pHTAC[CX];
					if (ResDir.pHTAC[CX + 1] == ResDir.pHTAC[CX]) ResDir.pHTAC[CX + 1] = nullptr;
					ResDir.pHTAC[CX] = nullptr;
				}
			}

			//  Return the in-memory image to the caller
			ImgSize = ImgUsed;
			return pImage;
		}

		//  appendImage
		//
		//  This static function will append the Entropy Encoded Block (Image)
		//
		//  PARAMETERS
		//
		//		Train*			-		Pointer to the train being stored
		//		Byte*			-		Pointer to the in-memory image
		//		size_t&			-		Reference to the size used of the in-memory image
		//		JRD&			-		Reference to the JPEG Resource Directory
		//		SWITCHES		-		Options array
		//
		//  RETURNS
		//
		//  NOTES
		//
		//

		static void		appendImage(Train<YCbCr>* pTrain, BYTE* pImage, size_t& ImgUsed, JRD& ResDir, SWITCHES Opts) {
			(void)Opts;
			StuffedStream			bsOut(2 * pTrain->getCanvasWidth() * pTrain->getCanvasHeight(), pTrain->getCanvasWidth() * pTrain->getCanvasHeight());
			EncoderPipeline			Pipe;															//  Encoder Pipeline
			Huffman					EEC;															//  Entropy Encoder
			BYTE*					pEncImg = nullptr;												//  Pointer to the encoded image buffer
			size_t					EISize = 0;														//  Encoded image size

			//  Prepare the pipeline for encoding the frame
			Pipe.setPrecision(8);																	//  Precision (channel width) always 8 bits
			Pipe.setMCUFF(ResDir.MCUFF);															//  Set the MCU Form factor

			//  Set the channel specific characteristics
			for (int CX = 0; CX < 3; CX++) {
				//  Set the Horizontal & Vertical sampling factors
				Pipe.setHSPM(CX, ResDir.HSF[CX]);
				Pipe.setVSPM(CX, ResDir.VSF[CX]);

				//  Set the Quantizer
				Pipe.setQuantizer(CX, ResDir.pQ[CX]);

				//  Set the DC & AC Huffman Trees
				Pipe.setDCEncoder(CX, ResDir.pHTDC[CX]);
				Pipe.setACEncoder(CX, ResDir.pHTAC[CX]);
			}

			//
			//  Arrange the plumbing for the input and output:
			//
			//		RasterBuffer  ==> Pipeline::Collecter ==> Pipeline ==> Huffman::Collecter ==> StuffedStream
			//

			Huffman::JPEGCollecter		Sink = EEC.encodeJPEG(bsOut);
			EncoderPipeline::Collecter	Source = Pipe.encode(&Sink, ResDir.MCUFF);

			//
			//  Iterate over the raster buffer in MCU sequence feeding the pixels to the pipeline collecter
			//

			for (RasterBuffer<YCbCr>::iterator It = pTrain->getFirstFrame()->buffer().firstMCU(ResDir.MCUFF); It != pTrain->getFirstFrame()->buffer().lastMCU(ResDir.MCUFF); It++) {
				Source.next(*It);
			}

			//  Signal end of the pixel stream to the pipeline
			Source.signalEndOfStream();

			//  Acquire the stuffed stream buffer
			pEncImg = bsOut.acquireBuffer(EISize);

			//  Copy the encoded image into the image
			memcpy(pImage + ImgUsed, pEncImg, EISize);

			//  Update the image size
			ImgUsed += EISize;

			//  Free the encoding buffer
			free(pEncImg);

			//  Return to caller
			return;
		}

		//  appendStartOfScan
		//
		//  This static function will append the Start Of Scan (SOS) block to the image
		//
		//  PARAMETERS
		//
		//		Byte*			-		Pointer to the in-memory image
		//		size_t&			-		Reference to the size used of the in-memory image
		//		JRD&			-		Reference to the JPEG Resource Directory
		//		SWITCHES		-		Options array
		//
		//  RETURNS
		//
		//  NOTES
		//
		//

		static void		appendStartOfScan(BYTE* pImage, size_t& ImgUsed, JRD& ResDir, SWITCHES Opts) {
			(void)Opts;
			JFIF_SCAN_HEADER1*		pSOS = (JFIF_SCAN_HEADER1*) (pImage + ImgUsed);					//  Pointer to the SOS structure
			JFIF_SCAN_HEADER2*		pROS = nullptr;													//  Pointer to the rest of fields

			//  Set the ID
			pSOS->Signature = JFIF_BLKID_SIG;
			pSOS->ID = JFIF_BLKID_SOS;

			//  Set the length
			SetSizeBE(pSOS->Length, uint16_t((3 * sizeof(JFIF_SCAN_COMPONENT)) + sizeof(JFIF_SCAN_HEADER2) + 3));

			//  Set the components
			pSOS->Components = 3;
			pSOS->Comp[0].ScanSelector = 1;
			pSOS->Comp[0].DCandAC = (ResDir.DCUse[0] << 4) + ResDir.ACUse[0];
			pSOS->Comp[1].ScanSelector = 2;
			pSOS->Comp[1].DCandAC = (ResDir.DCUse[1] << 4) + ResDir.ACUse[1];
			pSOS->Comp[2].ScanSelector = 3;
			pSOS->Comp[2].DCandAC = (ResDir.DCUse[2] << 4) + ResDir.ACUse[2];

			pROS = (JFIF_SCAN_HEADER2*)(pImage + ImgUsed + (3 * sizeof(JFIF_SCAN_COMPONENT)) + 5);

			//  Fill the rest of fields
			pROS->AHiandLo = 0;
			pROS->SSpecSel = 0;
			pROS->ESpecSel = 63;

			//  Update the size of image used
			ImgUsed += ((3 * sizeof(JFIF_SCAN_COMPONENT)) + 5 + sizeof(JFIF_SCAN_HEADER2));

			//  Return to caller
			return;
		}

		//  appendHuffmanTrees
		//
		//  This static function will append each of the Huffman Trees that are in use
		//
		//  PARAMETERS
		//
		//		Byte*			-		Pointer to the in-memory image
		//		size_t&			-		Reference to the size used of the in-memory image
		//		JRD&			-		Reference to the JPEG Resource Directory
		//		SWITCHES		-		Options array
		//
		//  RETURNS
		//
		//  NOTES
		//
		//

		static void		appendHuffmanTrees(BYTE* pImage, size_t& ImgUsed, JRD& ResDir, SWITCHES Opts) {
			(void)Opts;
			JFIF_HTAB*		pHT = nullptr;															//  Pointer to the table being defined
			BYTE			CandD = 0;																//  Class and Destination encoding
			BYTE*			pSHT = nullptr;															//  Pointer to the serialised form of the table
			size_t			SHTSize = 0;															//  Size of the serialised form of the table
			BYTE*			pAppend = nullptr;														//  Pointer to the append point for a table contents

			//  In theory we could have 6 different Huffman trees (a DC and AC tree for each component).
			//  However in normal use we would expect the two chrominance channels to use the same DC and
			//  the same AC trees, therefore we normally expect 4 tables.

			//  Express the Y channel (luminance) DC Huffman Tree
			pHT = (JFIF_HTAB*) (pImage + ImgUsed);
			pHT->Signature = JFIF_BLKID_SIG;
			pHT->ID = JFIF_BLKID_DHT;
			CandD = ResDir.DCUse[0];																//  Class is DC or lossless

			//  Get the serialised form of the table
			pSHT = ResDir.pHTDC[ResDir.DCUse[0]]->serialize(CandD, SHTSize);
			if (pSHT == nullptr) return;

			//  Set the length
			SetSizeBE(pHT->Length, uint16_t(SHTSize + 2));

			//  Add the serialised form of the table to the image
			pAppend = (pImage + ImgUsed + 4);
			memcpy(pAppend, pSHT, SHTSize);
			free(pSHT);

			//  Update the size of the image
			ImgUsed += (SHTSize + 4);

			//  Express the Cb channel (chrominance) DC Huffman Tree, unless it is already expressed as for the luminance channel
			if (ResDir.DCUse[1] != ResDir.DCUse[0]) {
				pHT = (JFIF_HTAB*) (pImage + ImgUsed);
				pHT->Signature = JFIF_BLKID_SIG;
				pHT->ID = JFIF_BLKID_DHT;
				CandD = ResDir.DCUse[1];																//  Class is DC or lossless

				//  Get the serialised form of the table
				pSHT = ResDir.pHTDC[ResDir.DCUse[1]]->serialize(CandD, SHTSize);
				if (pSHT == nullptr) return;

				//  Set the length
				SetSizeBE(pHT->Length, uint16_t(SHTSize + 2));

				//  Add the serialised form of the table to the image
				pAppend = (pImage + ImgUsed + 4);
				memcpy(pAppend, pSHT, SHTSize);
				free(pSHT);

				//  Update the size of the image
				ImgUsed += (SHTSize + 4);
			}

			//  Express the Cr channel (chrominance) DC Huffman Tree, unless it is already expressed as for the Y or Cb channels
			if (ResDir.DCUse[2] != ResDir.DCUse[1] && ResDir.DCUse[2] != ResDir.DCUse[0]) {
				pHT = (JFIF_HTAB*) (pImage + ImgUsed);
				pHT->Signature = JFIF_BLKID_SIG;
				pHT->ID = JFIF_BLKID_DHT;
				CandD = ResDir.DCUse[2];																//  Class is DC or lossless

				//  Get the serialised form of the table
				pSHT = ResDir.pHTDC[ResDir.DCUse[2]]->serialize(CandD, SHTSize);
				if (pSHT == nullptr) return;

				//  Set the length
				SetSizeBE(pHT->Length, uint16_t(SHTSize + 2));

				//  Add the serialised form of the table to the image
				pAppend = (pImage + ImgUsed + 4);
				memcpy(pAppend, pSHT, SHTSize);
				free(pSHT);

				//  Update the size of the image
				ImgUsed += (SHTSize + 4);
			}

			//
			//   Express the AC trees
			//

			//  Express the Y channel (luminance) AC Huffman Tree
			pHT = (JFIF_HTAB*)(pImage + ImgUsed);
			pHT->Signature = JFIF_BLKID_SIG;
			pHT->ID = JFIF_BLKID_DHT;
			CandD = ResDir.ACUse[0] | 0x10;																//  Class is AC

			//  Get the serialised form of the table
			pSHT = ResDir.pHTAC[ResDir.ACUse[0]]->serialize(CandD, SHTSize);
			if (pSHT == nullptr) return;

			//  Set the length
			SetSizeBE(pHT->Length, uint16_t(SHTSize + 2));

			//  Add the serialised form of the table to the image
			pAppend = (pImage + ImgUsed + 4);
			memcpy(pAppend, pSHT, SHTSize);
			free(pSHT);

			//  Update the size of the image
			ImgUsed += (SHTSize + 4);

			//  Express the Cb channel (chrominance) DC Huffman Tree, unless it is already expressed as for the luminance channel
			if (ResDir.ACUse[1] != ResDir.ACUse[0]) {
				pHT = (JFIF_HTAB*)(pImage + ImgUsed);
				pHT->Signature = JFIF_BLKID_SIG;
				pHT->ID = JFIF_BLKID_DHT;
				CandD = ResDir.ACUse[1] | 0x10;															//  Class is AC

				//  Get the serialised form of the table
				pSHT = ResDir.pHTAC[ResDir.ACUse[1]]->serialize(CandD, SHTSize);
				if (pSHT == nullptr) return;

				//  Set the length
				SetSizeBE(pHT->Length, uint16_t(SHTSize + 2));

				//  Add the serialised form of the table to the image
				pAppend = (pImage + ImgUsed + 4);
				memcpy(pAppend, pSHT, SHTSize);
				free(pSHT);

				//  Update the size of the image
				ImgUsed += (SHTSize + 4);
			}

			//  Express the Cr channel (chrominance) DC Huffman Tree, unless it is already expressed as for the Y or Cb channels
			if (ResDir.ACUse[2] != ResDir.ACUse[1] && ResDir.ACUse[2] != ResDir.ACUse[0]) {
				pHT = (JFIF_HTAB*)(pImage + ImgUsed);
				pHT->Signature = JFIF_BLKID_SIG;
				pHT->ID = JFIF_BLKID_DHT;
				CandD = ResDir.ACUse[2] | 0x10;															//  Class is AC

				//  Get the serialised form of the table
				pSHT = ResDir.pHTAC[ResDir.ACUse[2]]->serialize(CandD, SHTSize);
				if (pSHT == nullptr) return;

				//  Set the length
				SetSizeBE(pHT->Length, uint16_t(SHTSize + 2));

				//  Add the serialised form of the table to the image
				pAppend = (pImage + ImgUsed + 4);
				memcpy(pAppend, pSHT, SHTSize);
				free(pSHT);

				//  Update the size of the image
				ImgUsed += (SHTSize + 4);
			}

			//  Return to caller
			return;
		}

		//  appendStartOfFrame
		//
		//  This static function will append the Start-Of-Frame (SOF0)
		//
		//  PARAMETERS
		//
		//		Train*			-		Pointer to the train being stored
		//		Byte*			-		Pointer to the in-memory image
		//		size_t&			-		Reference to the size used of the in-memory image
		//		JRD&			-		Reference to the JPEG Resource Directory
		//		SWITCHES		-		Options array
		//
		//  RETURNS
		//
		//  NOTES
		//
		//		We only support Baseline DCT encoding so only use the SOF0
		//
		//

		static void		appendStartOfFrame(Train<YCbCr>* pTrain, BYTE* pImage, size_t& ImgUsed, JRD& ResDir, SWITCHES Opts) {
			(void)Opts;
			JFIF_FRAME_HEADER*		pFH = (JFIF_FRAME_HEADER*) (pImage + ImgUsed);									//  Pointer to the SOF0 block
			JFIF_FRAME_COMPONENT*	pFC = (JFIF_FRAME_COMPONENT*) (pImage + ImgUsed + 10);							//  Pointer to the first component in the frame
			//  Fill in the Identifier
			pFH->Signature = JFIF_BLKID_SIG;
			pFH->ID = JFIF_BLKID_SOF0;

			//  Set the sampling precision (channel width) - always 8 bit
			pFH->Precision = 8;

			//  Set the height and width of the image
			SetSizeBE(pFH->HLines, uint16_t(pTrain->getCanvasHeight()));
			SetSizeBE(pFH->VLines, uint16_t(pTrain->getCanvasWidth()));

			//  Set the number of components (always 3)
			pFH->Components = 3;

			//  Fill in each of the components Y, Cb and Cr in turn
			//  Start with the luminance (Y) component
			if (ResDir.MCUFF == 0x11) {
				pFC->CompID = 1;
				pFC->HandV = 0x11;																	//  Sampling factor H & V are both 1
				ResDir.HSF[0] = 1;
				ResDir.VSF[0] = 1;
				pFC->QTable = ResDir.QUse[0];														//  Y - Quantisation table
			}
			else {
				pFC->CompID = 1;
				pFC->HandV = 0x22;																	//  Sampling factor H & V are both 2
				ResDir.HSF[0] = 2;
				ResDir.VSF[0] = 2;
				pFC->QTable = ResDir.QUse[0];														//  Y - Quantisation table
			}
			//  Next set the Cb component
			pFC++;
			pFC->CompID = 2;
			pFC->HandV = 0x11;																	//  Sampling factor H & V are both 1
			ResDir.HSF[1] = 1;
			ResDir.VSF[1] = 1;
			pFC->QTable = ResDir.QUse[1];														//  Cb - Quantisation table

			//  Finally set the Cr compnent
			pFC++;
			pFC->CompID = 3;
			pFC->HandV = 0x11;																	//  Sampling factor H & V are both 1
			ResDir.HSF[2] = 1;
			ResDir.VSF[2] = 1;
			pFC->QTable = ResDir.QUse[2];														//  Cr - Quantisation table

			//  Set the length in the block
			SetSizeBE(pFH->Length, uint16_t(8 + (3 * sizeof(JFIF_FRAME_COMPONENT))));

			//  Update the image size
			ImgUsed += (GetSizeBE(pFH->Length) + 2);

			//  Return to caller
			return;
		}

		//  appendQuantisationTables
		//
		//  This static function will append the quantisation tables that have been selected to use with the image
		//
		//  PARAMETERS
		//
		//		Byte*			-		Pointer to the in-memory image
		//		size_t&			-		Reference to the size used of the in-memory image
		//		JRD&			-		Reference to the JPEG Resource Directory
		//
		//  RETURNS
		//
		//  NOTES
		//
		//

		static void		appendQuantisationTables(BYTE* pImage, size_t& ImgUsed, JRD& ResDir) {
			bool		TabDefined = false;																	//  Table already defined control
			BYTE		Dest = 0;																			//  Next free destination
			BYTE*		pSTI = nullptr;																		//  Serialised table image
			size_t		SISize = 0;																			//  Size of the serialised image

			//
			//  We use a minimum of 1 and a maximum of 3 quantisation tables.
			//  Most commonly we use two tables, one for the luminance channel and one for both chrominance channels.
			//

			for (int QX = 0; QX < 3; QX++) {

				Dest = ResDir.QUse[QX];

				if (ResDir.pQ[QX] != nullptr) {
					//  Determine if the current table is already defined
					TabDefined = false;
					if (QX > 0) {
						for (int CompX = QX - 1; CompX >= 0; CompX--) {
							if (ResDir.pQ[CompX] == ResDir.pQ[QX]) TabDefined = true;
						}
					}

					//  If the table is NOT already define then add the definition to the image
					if (!TabDefined) {
						JFIF_DATA_BLOCK*	pBlock = (JFIF_DATA_BLOCK*) (pImage + ImgUsed);
						pBlock->Signature = JFIF_BLKID_SIG;
						pBlock->ID = JFIF_BLKID_DQT;

						//  Obtain the serialised image of the table
						pSTI = ResDir.pQ[QX]->serialize(Dest, SISize);
						if (pSTI == NULL) return;
						SetSizeBE(pBlock->Length, uint16_t(SISize + 2));
						memcpy(pBlock + 1, pSTI, SISize);
						free(pSTI);
						ImgUsed += (sizeof(JFIF_DATA_BLOCK) + SISize);
					}
				}
			}

			//  Return to caller
			return;
		}

		//  appendBasicHeader
		//
		//  This static function will append a JFIF Basic (APP0) header to the in-memory image.
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
		//

		static void		appendBasicHeader(BYTE* pImage, size_t& ImgUsed) {
			JFIF_APP0_BLOCK*		pAH = (JFIF_APP0_BLOCK*) (pImage + ImgUsed);

			pAH->Signature = JFIF_BLKID_SIG;
			pAH->ID = JFIF_BLKID_APP0;
			SetSizeBE(pAH->Length, uint16_t(sizeof(JFIF_APP0_BLOCK) - 2));

			//  Set the JFIF identifier field
			pAH->Identifier[0] = 'J';
			pAH->Identifier[1] = 'F';
			pAH->Identifier[2] = 'I';
			pAH->Identifier[3] = 'F';
			pAH->Identifier[4] = '\0';

			//  Set the JFIF version (1.1)
			pAH->MajVer = 1;
			pAH->MinVer = 1;

			//  Set the density units	-	Pixels per Inch
			pAH->DUnits = JFIF_DUNITS_DPI;

			//  Set the image density 96 vertical and 96 horizontal
			SetSizeBE(pAH->VPD, 96);
			SetSizeBE(pAH->HPD, 96);

			//  Set the thumbnail size (none)
			pAH->ThumbH = 0;
			pAH->ThumbW = 0;

			//  Update the image size
			ImgUsed += sizeof(JFIF_APP0_BLOCK);

			//  Return to caller
			return;
		}

		//  appendFileHeader
		//
		//  This static function will append a JFIF File header to the in-memory image.
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
		//

		static void		appendFileHeader(BYTE* pImage, size_t& ImgUsed) {
			JFIF_FILE_HEADER*		pFH = (JFIF_FILE_HEADER*) (pImage + ImgUsed);

			pFH->Signature = JFIF_BLKID_SIG;
			pFH->ID = JFIF_BLKID_SOI;

			ImgUsed += sizeof(JFIF_FILE_HEADER);

			//  Return to caller
			return;
		}

		//  appendFileTrailer
		//
		//  This static function will append a JFIF File trailer to the in-memory image.
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
		//

		static void		appendFileTrailer(BYTE* pImage, size_t& ImgUsed) {
			JFIF_FILE_TRAILER*		pFT = (JFIF_FILE_TRAILER*)(pImage + ImgUsed);

			pFT->Signature = JFIF_BLKID_SIG;
			pFT->ID = JFIF_BLKID_EOI;

			ImgUsed += sizeof(JFIF_FILE_TRAILER);

			//  Return to caller
			return;
		}

		//  selectResources
		//
		//  This static function will populate the passed resource directory with appropriate resources.
		//	Quantizers and Huffman Treea for each channel.
		//
		//	This implementation will populate the directory with generic default Quanizers and Huffman Trees without
		//  attempting to optimise the resources used by inspection of the image.
		//
		//  PARAMETERS
		//
		//		Train*			-		Pointer to the input Canonical Train
		//		JRD&			-		Reference to the resource directory structure
		//		SWITCHES		-		Options to use for storing the image
		//
		//  RETURNS
		//
		//  NOTES
		//
		
		static void		selectResources(Train<YCbCr>* pTrain, JRD& ResDir, SWITCHES Opts) {
			(void)pTrain;
			//  Default quantisation table for luminance channel (8 bit precision)
			JFIF_QV8_ARRAY		DefY8 = { 0, {16,11,10,16,24,40,51,61,
											12,12,14,19,26,58,60,55,
											14,13,16,24,40,57,69,56,
											14,17,22,29,51,87,80,62,
											18,22,37,56,68,109,103,77,
											24,35,55,64,81,104,113,92,
											49,64,78,87,103,121,120,101,
											72,92,95,98,112,100,103,99} };

			//  Alternate quantisation table for luminance channel (8 bit precision)
			JFIF_QV8_ARRAY		AltY8 = { 0, {6,4,5,6,5,4,6,6,
											5,6,7,7,6,8,10,16,
											10,10,9,9,10,20,14,15,
											12,16,23,20,24,24,23,20,
											22,22,26,29,37,31,26,27,
											35,28,22,22,32,44,32,35,
											38,39,41,42,41,25,31,45,
											48,45,40,48,37,40,41,40} };

			//  Alternate (from Paint) quantisation table for luminance channel (8 bit precision)
			JFIF_QV8_ARRAY		PQTY8 = { 0, {2,1,1,2,1,1,2,2,2,2,2,2,2,2,3,5,
											3,3,3,3,3,6,4,4,3,5,7,6,7,7,7,6,
											7,7,8,9,11,9,8,8,10,8,7,7,10,13,10,10,
											11,12,12,12,12,7,9,14,15,13,12,14,11,12,12,12} };

			//  Default quantisation table for chrominance channels (8 bit precision)
			JFIF_QV8_ARRAY		DefC8 = { 0, {17,18,24,47,99,99,99,99,
											18,21,26,66,99,99,99,99,
											24,36,56,99,99,99,99,99,
											47,66,99,99,99,99,99,99,
											99,99,99,99,99,99,99,99,
											99,99,99,99,99,99,99,99,
											99,99,99,99,99,99,99,99,
											99,99,99,99,99,99,99,99} };


			//  Alternate quantisation table for chrominance channels (8 bit precision)
			JFIF_QV8_ARRAY		AltC8 = { 0, {7,7,7,10,8,10,19,10,
											10,19,40,26,22,26,40,40,
											40,40,40,40,40,40,40,40,
											40,40,40,40,40,40,40,40,
											40,40,40,40,40,40,40,40,
											40,40,40,40,40,40,40,40,
											40,40,40,40,40,40,40,40,
											40,40,40,40,40,40,40,40} };

			//  Alternate (from Paint) quantisation table for chrominance channels (8 bit precision)
			JFIF_QV8_ARRAY		PQTC8 = { 0, {2,2,2,3,3,3,6,3,3,6,12,8,7,8,12,12,
											12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,
											12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,
											12,12,12,12,12,12,12,12,12,12,12,12,12,12,12,12} };

			//  Default Huffman Tree definition for luminance channel DC categories
			BYTE				DefYDC8[64] = { 0xFF, 0xC4, 0x00, 31, 0,
												0, 1, 5, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 
												0,
												1, 2, 3, 4, 5,
												6,
												7,
												8,
												9,
												10,
												11};

			//  Default Huffman Tree definition for chrominance channels DC categories
			BYTE				DefCDC8[64] = { 0xFF, 0xC4, 0x00, 31, 0,
												0, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0,
												0, 1, 2,
												3,
												4,
												5,
												6,
												7,
												8,
												9,
												10,
												11};

			//  Default Huffman Tree definition for luminance channel AC categories
			BYTE				DefYAC8[256] = { 0xFF, 0xC4, 0x00, 181, 0,
												0, 2, 1, 3, 3, 2, 4, 3, 5, 5, 4, 4, 0, 0, 1, 125,
												1, 2,
												3,
												0, 4, 17,
												5, 18, 33,
												49, 65,
												6, 19, 81, 97,
												7, 34, 113,
												20, 50, 129, 145, 161,
												8, 35, 66, 177, 193,
												21, 82, 209, 240,
												36, 51, 98, 114,
												130,
												9, 10, 22, 23, 24, 25, 26, 37, 38, 39, 40, 41, 42, 52, 53, 54,
												55, 56, 57, 58, 67, 68, 69, 70, 71, 72, 73, 74, 83, 84, 85, 86,
												87, 88, 89, 90, 99, 100, 101, 102, 103, 104, 105, 106, 115, 116, 117, 118,
												119, 120, 121, 122, 131, 132, 133, 134, 135, 136, 137, 138, 146, 147, 148, 149,
												150, 151, 152, 153, 154, 162, 163, 164, 165, 166, 167, 168, 169, 170, 178, 179,
												180, 181, 182, 183, 184, 185, 186, 194, 195, 196, 197, 198, 199, 200, 201, 202,
												210, 211, 212, 213, 214, 215, 216, 217, 218, 225, 226, 227, 228, 229, 230, 231,
												232, 233, 234, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250 };

			//  Default Huffman Tree definition for chrominance channels AC categories
			BYTE				DefCAC8[256] = { 0xFF, 0xC4, 0x00, 181, 0,
												0, 2, 1, 2, 4, 4, 3, 4, 7, 5, 4, 4, 0, 1, 2, 119,
												0, 1,
												2,
												3, 17,
												4, 5, 33, 49,
												6, 18, 65, 81,
												7, 97, 113,
												19, 34, 50, 129,
												8, 20, 66, 145, 161, 177, 193,
												9, 35, 51, 82, 240,
												21, 98, 114, 209,
												10, 22, 36, 52,
												225,
												37, 241,
												23, 24, 25, 26, 38, 39, 40, 41, 42, 53, 54, 55, 56, 57, 58, 67,
												68, 69, 70, 71, 72, 73, 74, 83, 84, 85, 86, 87, 88, 89, 90, 99,
												100, 101, 102, 103, 104, 105, 106, 115, 116, 117, 118, 119, 120, 121, 122, 130,
												131, 132, 133, 134, 135, 136, 137, 138, 146, 147, 148, 169, 150, 151, 152, 153,
												154, 162, 163, 164, 165, 166, 167, 168, 169, 170, 178, 179, 180, 181, 182, 183,
												184, 185, 186, 194, 195, 196, 197, 198, 199, 200, 201, 202, 210, 211, 212, 213,
												214, 215, 216, 217, 218, 226, 227, 228, 229, 230, 231, 232, 233, 234, 242, 243,
												244, 245, 246, 247, 248, 249, 250 };

			//  Select the MCU Form Factor to use
			if (Opts & JFIF_STORE_OPT_HIFI) ResDir.MCUFF = 0x11;
			else ResDir.MCUFF = 0x22;

			//  Set the default luminance quantization table
			ResDir.pQ[0] = new JPEGQuantizer((BYTE*) &AltY8);
			ResDir.QUse[0] = 0;

			//  Set the default chrominance quantization table for both Cb and Cr channels
			ResDir.pQ[2] = ResDir.pQ[1] = new JPEGQuantizer((BYTE*) &AltC8);
			ResDir.QUse[2] = ResDir.QUse[1] = 1;

			//  Set the Default DC Huffman trees, luminance followed by chrominace
			ResDir.pHTDC[0] = new JPEGHuffmanTree((JFIF_HTAB&) DefYDC8);
			ResDir.DCUse[0] = 0;
			ResDir.pHTDC[2] = ResDir.pHTDC[1] = new JPEGHuffmanTree((JFIF_HTAB&) DefCDC8);
			ResDir.DCUse[2] = ResDir.DCUse[1] = 1;

			//  Set the Default AC Huffman trees, luminance followed by chrominace
			ResDir.pHTAC[0] = new JPEGHuffmanTree((JFIF_HTAB&) DefYAC8);
			ResDir.ACUse[0] = 0;
			ResDir.pHTAC[2] = ResDir.pHTAC[1] = new JPEGHuffmanTree((JFIF_HTAB&) DefCAC8);
			ResDir.ACUse[2] = ResDir.ACUse[1] = 1;

			//  Return to caller
			return;
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
			bool			EndOfImage = false;												//  End Of Image
			size_t			EEBSize = 0;													//  Size of entropy encoded block
			BYTE*			pSOB = nullptr;													//  Pointer to start of a block

			//  Initialise the block array
			Map.NBA = 256;
			Map.Blocks = (ODIBlock*) malloc(256 * sizeof(ODIBlock));
			if (Map.Blocks == NULL) {
				std::cerr << "ERROR: Unable to allocate memory for a JFIF On-Disk-Image block map." << std::endl;
				return false;
			}
			memset(Map.Blocks, 0, 256 * sizeof(ODIBlock));

			//  Check that the image is large enough to contain the first block
			if (Map.ImageSize < sizeof(JFIF_FILE_HEADER)) {
				std::cerr << "ERROR: The JFIF Image is too small to hold a valid JFIF File Header." << std::endl;
				free(Map.Blocks);
				Map.NBA = Map.NumBlocks = 0;
				return false;
			}

			//  Allocate the first block as a file header
			Map.Blocks[Map.NumBlocks].BlockType = JFIF_BLOCK_FH;
			Map.Blocks[Map.NumBlocks].Block = Map.Image;
			Map.Blocks[Map.NumBlocks].BlockSize = sizeof(JFIF_FILE_HEADER);
			Map.NumBlocks = 1;

			//  Verify the header
			JFIF_FILE_HEADER*	pFH = (JFIF_FILE_HEADER*) (Map.Image + Offset);
			if (pFH->Signature != JFIF_BLKID_SIG || pFH->ID != JFIF_BLKID_SOI) {
				std::cerr << "ERROR: JFIF file does NOT start with a valid File Header." << std::endl;
				free(Map.Blocks);
				Map.NBA = Map.NumBlocks = 0;
				return false;
			}

			//  Update the offset
			Offset += sizeof(JFIF_FILE_HEADER);

			//
			//  Process all remaining blocks until the End-Of-Image is encuntered or we run out of image
			//

			while ((!EndOfImage) && Offset < Map.ImageSize) {

				//  Address the next block
				JFIF_DATA_BLOCK*	pBlock = (JFIF_DATA_BLOCK*)	(Map.Image + Offset);
				if (pBlock->Signature != JFIF_BLKID_SIG) {
					std::cerr << "ERROR: Block: " << Map.NumBlocks + 1 << " at Offset: +" << Offset << " does not start with a valid block signature." << std::endl;
					free(Map.Blocks);
					Map.NBA = Map.NumBlocks = 0;
					return false;
				}

				//  Switch according to the type of the block encountered
				switch (pBlock->ID) {

				case JFIF_BLKID_APP0:
					//  Application usage (Type 0 - basic) block
					Map.Blocks[Map.NumBlocks].BlockType = JFIF_BLOCK_APP0;
					Map.Blocks[Map.NumBlocks].Block = (Map.Image + Offset);
					Map.Blocks[Map.NumBlocks].BlockSize = (GetSizeBE(pBlock->Length) + 2);
					Map.NumBlocks++;

					Offset += (GetSizeBE(pBlock->Length) + 2);
					break;

				case EXIF_BLKID_APP1:
					//  Application usage (Type 1 - EXIF) block
					Map.Blocks[Map.NumBlocks].BlockType = JFIF_BLOCK_EXIF;
					Map.Blocks[Map.NumBlocks].Block = (Map.Image + Offset);
					Map.Blocks[Map.NumBlocks].BlockSize = (GetSizeBE(pBlock->Length) + 2);
					Map.NumBlocks++;

					Offset += (GetSizeBE(pBlock->Length) + 2);
					break;

				case JFIF_BLKID_DQT:
					//  Define quantisation table block
					Map.Blocks[Map.NumBlocks].BlockType = JFIF_BLOCK_DQT;
					Map.Blocks[Map.NumBlocks].Block = (Map.Image + Offset);
					Map.Blocks[Map.NumBlocks].BlockSize = (GetSizeBE(pBlock->Length) + 2);
					Map.NumBlocks++;

					Offset += (GetSizeBE(pBlock->Length) + 2);
					break;

				case JFIF_BLKID_DHT:
					//  Define huffman table block
					Map.Blocks[Map.NumBlocks].BlockType = JFIF_BLOCK_DHT;
					Map.Blocks[Map.NumBlocks].Block = (Map.Image + Offset);
					Map.Blocks[Map.NumBlocks].BlockSize = (GetSizeBE(pBlock->Length) +2);
					Map.NumBlocks++;

					Offset += (GetSizeBE(pBlock->Length) + 2);
					break;

				case JFIF_BLKID_SOF0:
					//  Start of Frame (0) block
					Map.Blocks[Map.NumBlocks].BlockType = JFIF_BLOCK_SOF0;
					Map.Blocks[Map.NumBlocks].Block = (Map.Image + Offset);
					Map.Blocks[Map.NumBlocks].BlockSize = (GetSizeBE(pBlock->Length) + 2);
					Map.NumBlocks++;

					Offset += (GetSizeBE(pBlock->Length) + 2);
					break;

				case JFIF_BLKID_SOF1:
				case JFIF_BLKID_SOF2:
				case JFIF_BLKID_SOF3:
				case JFIF_BLKID_SOF5:
				case JFIF_BLKID_SOF6:
				case JFIF_BLKID_SOF7:
				case JFIF_BLKID_SOF9:
				case JFIF_BLKID_SOF10:
				case JFIF_BLKID_SOF11:
				case JFIF_BLKID_SOF13:
				case JFIF_BLKID_SOF14:
				case JFIF_BLKID_SOF15:

					//  Start of Frame (x) block
					Map.Blocks[Map.NumBlocks].BlockType = JFIF_BLOCK_SOFX;
					Map.Blocks[Map.NumBlocks].Block = (Map.Image + Offset);
					Map.Blocks[Map.NumBlocks].BlockSize = (GetSizeBE(pBlock->Length) + 2);
					Map.NumBlocks++;

					Offset += (GetSizeBE(pBlock->Length) + 2);
					break;

				case JFIF_BLKID_SOS:
					//  Start of Scan block
					Map.Blocks[Map.NumBlocks].BlockType = JFIF_BLOCK_SOS;
					Map.Blocks[Map.NumBlocks].Block = (Map.Image + Offset);
					Map.Blocks[Map.NumBlocks].BlockSize = (GetSizeBE(pBlock->Length) + 2);
					Map.NumBlocks++;

					Offset += (GetSizeBE(pBlock->Length) + 2);

					//
					//  Start-Of-Scan block is ALWAYS immediately followed by the Entropy Encoded Block (EEB)
					//  having no block descriptor.
					//

					EEBSize = 0;
					//  Scan the block to determine the length of the EEB
					pSOB = Map.Image + Offset;
					while (*pSOB != JFIF_BLKID_SIG || *(pSOB + 1) == JFIF_BLKID_STUFF) {
						EEBSize++;
						pSOB++;
					}

					Map.Blocks[Map.NumBlocks].BlockType = JFIF_BLOCK_EEB;
					Map.Blocks[Map.NumBlocks].Block = (Map.Image + Offset);
					Map.Blocks[Map.NumBlocks].BlockSize = EEBSize;
					Map.NumBlocks++;

					Offset += EEBSize;
					break;

				case JFIF_BLKID_EOI:
					//  End Of Image block
					Map.Blocks[Map.NumBlocks].BlockType = JFIF_BLOCK_FT;
					Map.Blocks[Map.NumBlocks].Block = (Map.Image + Offset);
					Map.Blocks[Map.NumBlocks].BlockSize = 2;
					Map.NumBlocks++;

					Offset += 2;
					EndOfImage = true;
					break;

				case JFIF_BLKID_RST0:
				case JFIF_BLKID_RST1:
				case JFIF_BLKID_RST2:
				case JFIF_BLKID_RST3:
				case JFIF_BLKID_RST4:
				case JFIF_BLKID_RST5:
				case JFIF_BLKID_RST6:
				case JFIF_BLKID_RST7:
					//  Restart Scan blocks
					Map.Blocks[Map.NumBlocks].BlockType = JFIF_BLOCK_RST;
					Map.Blocks[Map.NumBlocks].Block = (Map.Image + Offset);
					Map.Blocks[Map.NumBlocks].BlockSize = (GetSizeBE(pBlock->Length) + 2);
					Map.NumBlocks++;

					Offset += (GetSizeBE(pBlock->Length) + 2);

					//
					//  Restart block is ALWAYS immediately followed by the Entropy Encoded Block (EEB)
					//  having no block descriptor.
					//

					EEBSize = 0;
					//  Scan the block to determine the length of the EEB
					pSOB = Map.Image + Offset;
					while (*pSOB != JFIF_BLKID_SIG || *(pSOB + 1) == JFIF_BLKID_STUFF) {
						EEBSize++;
						pSOB++;
					}

					Map.Blocks[Map.NumBlocks].BlockType = JFIF_BLOCK_EEB;
					Map.Blocks[Map.NumBlocks].Block = (Map.Image + Offset);
					Map.Blocks[Map.NumBlocks].BlockSize = EEBSize;
					Map.NumBlocks++;

					Offset += EEBSize;
					break;

				case JFIF_BLKID_APP2:
				case JFIF_BLKID_APP3:
				case JFIF_BLKID_APP4:
				case JFIF_BLKID_APP5:
				case JFIF_BLKID_APP6:
				case JFIF_BLKID_APP7:
				case JFIF_BLKID_APP8:
				case JFIF_BLKID_APP9:
				case JFIF_BLKID_APP10:
				case JFIF_BLKID_APP11:
				case JFIF_BLKID_APP12:
				case JFIF_BLKID_APP13:
				case JFIF_BLKID_APP14:
				case JFIF_BLKID_APP15:
					//  Application Reserved blocks
					Map.Blocks[Map.NumBlocks].BlockType = JFIF_BLOCK_RES;
					Map.Blocks[Map.NumBlocks].Block = (Map.Image + Offset);
					Map.Blocks[Map.NumBlocks].BlockSize = (GetSizeBE(pBlock->Length) + 2);
					Map.NumBlocks++;

					Offset += (GetSizeBE(pBlock->Length) + 2);
					break;

				default:
					std::cerr << "ERROR: Unknown/Unexpected block type: " << int(pBlock->ID) << " was encountered at offset: +" << Offset << " in the file." << std::endl;
					free(Map.Blocks);
					Map.NBA = Map.NumBlocks = 0;
					return false;
				}
			}

			//  Return successful mapping
			return true;
		}

		//  Analysis Documentation Functions

		//  showFileHeader
		//
		//  This static function will document the JFIF File Header
		//
		//  PARAMETERS
		//
		//		size_t			-		Block number
		//		size_t			-		Offset in the file
		//		ODIMap&			-		Reference to the GIF On-Disk-Image map
		//		std::ostream&	-		Reference to the output stream
		//
		//  RETURNS
		//
		//  NOTES
		//

		static void		showFileHeader(size_t BlockNo, size_t Offset, ODIMap& Map, std::ostream& OS) {

			OS << std::endl;
			OS << "JFIF/JPEG FILE HEADER - Block: " << BlockNo + 1 << ", Offset: +" << Offset << ", Size: " << Map.Blocks[BlockNo].BlockSize << "." << std::endl;

			//  Dump the memory image
			OS << std::endl;
			MemoryDumper::dumpMemory(Map.Image + Offset, sizeof(JFIF_FILE_HEADER), "File Header", OS);
			OS << std::endl;

			//  Show the content

			//  Return to caller
			return;
		}

		//  showApp0
		//
		//  This static function will document the Application Basic Info block (APP0)
		//
		//  PARAMETERS
		//
		//		size_t			-		Block number
		//		size_t			-		Offset in the file
		//		ODIMap&			-		Reference to the GIF On-Disk-Image map
		//		std::ostream&	-		Reference to the output stream
		//
		//  RETURNS
		//
		//  NOTES
		//

		static void		showApp0(size_t BlockNo, size_t Offset, ODIMap& Map, std::ostream& OS) {
			BYTE					IDStr[5] = {};														//  Captured ID String
			char					szDenUnits[30];														//  Density units string

			JFIF_APP0_BLOCK*	pAPP0 = (JFIF_APP0_BLOCK*) Map.Blocks[BlockNo].Block;

			OS << std::endl;
			OS << "JFIF/JPEG APPLICATION BASIC INFORMATION (APP0) - Block: " << BlockNo + 1 << ", Offset: +" << Offset << ", Size: " << Map.Blocks[BlockNo].BlockSize << "." << std::endl;

			//  Dump the memory image
			OS << std::endl;
			MemoryDumper::dumpMemory(Map.Image + Offset, sizeof(JFIF_APP0_BLOCK), "Basic Info", OS);
			OS << std::endl;

			//  Show the details
			IDStr[0] = pAPP0->Identifier[0];
			IDStr[1] = pAPP0->Identifier[1];
			IDStr[2] = pAPP0->Identifier[2];
			IDStr[3] = pAPP0->Identifier[3];
			IDStr[4] = pAPP0->Identifier[4];
			if (IDStr[4] != '\0') IDStr[4] = '\0';
			OS << " Identifier:     '" << IDStr << "'." << std::endl;
			OS << " Version:        " << int(pAPP0->MajVer) << "." << int(pAPP0->MinVer) << "." << std::endl;
			if (pAPP0->DUnits != 0) {
				switch (pAPP0->DUnits) {
				case JFIF_DUNITS_DPI:
					strcpy_s(szDenUnits, 30, "Dots Per Inch (DPI)");
					break;

				case JFIF_DUNITS_DPC:
					strcpy_s(szDenUnits, 30, "Dots Per Centimeter (DPC)");
					break;

				default:
					strcpy_s(szDenUnits, 30, "Unknown units");
					break;
				}
				OS << " Image Density:  " << GetSizeBE(pAPP0->VPD) << " vertical by " << GetSizeBE(pAPP0->HPD) << " horizontal " << szDenUnits << "." << std::endl;
			}

			if (pAPP0->ThumbH > 0 && pAPP0->ThumbW > 0) {
				OS << "Thumbnail:   " << pAPP0->ThumbH << "x" << pAPP0->ThumbW << " pixels." << std::endl;
			}

			//  Return to caller
			return;
		}

		//  showEXIF
		//
		//  This static function will document an EXIF information block
		//
		//  PARAMETERS
		//
		//		size_t			-		Block number
		//		size_t			-		Offset in the file
		//		ODIMap&			-		Reference to the GIF On-Disk-Image map
		//		std::ostream&	-		Reference to the output stream
		//
		//  RETURNS
		//
		//  NOTES
		//

		static void		showEXIF(size_t BlockNo, size_t Offset, ODIMap& Map, std::ostream& OS) {

			OS << std::endl;
			OS << "JFIF/JPEG EXIF INFORMATION - Block: " << BlockNo + 1 << ", Offset: +" << Offset << ", Size: " << Map.Blocks[BlockNo].BlockSize << "." << std::endl;

			//  Dump the memory image
			OS << std::endl;
			MemoryDumper::dumpMemory(Map.Image + Offset, Map.Blocks[BlockNo].BlockSize, "EXIF INFO", OS);
			OS << std::endl;

			//  Show the content


			//  Return to caller
			return;
		}

		//  showDQT
		//
		//  This static function will document a Define Quantisation Table block
		//
		//  PARAMETERS
		//
		//		size_t			-		Block number
		//		size_t			-		Offset in the file
		//		ODIMap&			-		Reference to the GIF On-Disk-Image map
		//		std::ostream&	-		Reference to the output stream
		//
		//  RETURNS
		//
		//  NOTES
		//
		//		A DQT block MAY contain multiple quantisation tables
		//

		static void		showDQT(size_t BlockNo, size_t Offset, ODIMap& Map, std::ostream& OS) {
			size_t				Residue = Map.Blocks[BlockNo].BlockSize - 4;								//  Remaining space in the block for tables
			BYTE*				pNextTable = Map.Blocks[BlockNo].Block + 4;									//  Pointer to the next table in the block
			JFIF_QV8_ARRAY*		pQV8 = (JFIF_QV8_ARRAY*) (pNextTable);										//  8 bit value array
			JFIF_QV16_ARRAY*	pQV16 = (JFIF_QV16_ARRAY*) (pNextTable);									//  16 bit value array
			size_t				lIndex, vIndex;																//  Generic indexes

			OS << std::endl;
			OS << "JFIF/JPEG DEFINE QUANTISATION TABLE - Block: " << BlockNo + 1 << ", Offset: +" << Offset << ", Size: " << Map.Blocks[BlockNo].BlockSize << "." << std::endl;

			//  Dump the memory image
			OS << std::endl;
			MemoryDumper::dumpMemory(Map.Image + Offset, Map.Blocks[BlockNo].BlockSize, "DQT", OS);
			OS << std::endl;

			//  Show the detailed information for each table
			while (Residue >= sizeof(JFIF_QV8_ARRAY)) {

				OS << std::endl;
				OS << " Table Identifier:     " << GetDestination(pQV8->PandD) << "." << std::endl;
				OS << " Table Precision:      " << 8 + (8 * GetPrecision(pQV8->PandD)) << " bits." << std::endl;
				OS << " Values:-" << std::endl;
				OS << std::endl;

				//  Show the value array
				for (lIndex = 0; lIndex < 4; lIndex++) {
					if (lIndex == 0) OS << " 00 - 15 : ";
					else OS << " " << lIndex * 16 << " - " << ((lIndex + 1) * 16) - 1 << " : ";
					for (vIndex = 0; vIndex < 16; vIndex++) {
						if (GetPrecision(pQV8->PandD) == 0) OS << int(pQV8->QV[(lIndex * 16) + vIndex]);
						else OS << GetSizeBE(pQV16->QV[(lIndex * 16) + vIndex].PV);
						if (vIndex == 15) OS << ".";
						else OS << ",";
					}
					OS << std::endl;
				}

				if (GetPrecision(pQV8->PandD) == 0) {
					pNextTable += sizeof(JFIF_QV8_ARRAY);
					Residue -= sizeof(JFIF_QV8_ARRAY);
				}
				else {
					pNextTable += sizeof(JFIF_QV16_ARRAY);
					Residue -= sizeof(JFIF_QV16_ARRAY);
				}

				pQV8 = (JFIF_QV8_ARRAY*)(pNextTable);
				pQV16 = (JFIF_QV16_ARRAY*)(pNextTable);
			}

			//  Return to caller
			return;
		}

		//  showDHT
		//
		//  This static function will document a Define Huffman Tree block
		//
		//  PARAMETERS
		//
		//		size_t			-		Block number
		//		size_t			-		Offset in the file
		//		ODIMap&			-		Reference to the GIF On-Disk-Image map
		//		std::ostream&	-		Reference to the output stream
		//
		//  RETURNS
		//
		//  NOTES
		//

		static void		showDHT(size_t BlockNo, size_t Offset, ODIMap& Map, std::ostream& OS) {
			JFIF_HTAB*		pHT = (JFIF_HTAB*) Map.Blocks[BlockNo].Block;							//  Define Huffman Tree block
			int				clIndex;																//  Index of code lengths
			size_t			cOffset = 0;															//  Offset to codes

			OS << std::endl;
			OS << "JFIF/JPEG DEFINE HUFFMAN TREE - Block: " << BlockNo + 1 << ", Offset: +" << Offset << ", Size: " << Map.Blocks[BlockNo].BlockSize << "." << std::endl;

			//  Dump the memory image
			OS << std::endl;
			MemoryDumper::dumpMemory(Map.Image + Offset, Map.Blocks[BlockNo].BlockSize, "DHT", OS);
			OS << std::endl;

			//  Show the content
			OS << std::endl;
			OS << " Class:            " << GetClass(pHT->CandD) << " - ";
			if (GetClass(pHT->CandD) == 0) OS << "DC or lossless table." << std::endl;
			else OS << "AC table." << std::endl;
			OS << " Identifier:       " << GetDestination(pHT->CandD) << "." << std::endl;

			//  Show the raw table entries
			for (clIndex = 0; clIndex < 16; clIndex++) {
				if (pHT->HTL[clIndex] > 0) {
					OS << std::endl;
					OS << int(pHT->HTL[clIndex]) << " codes of length " << clIndex + 1 << " bits." << std::endl;
					MemoryDumper::dumpMemory(&pHT->HTEntry[cOffset], pHT->HTL[clIndex], "DECODED VALUES", OS);
					cOffset += pHT->HTL[clIndex];
				}
			}

			OS << std::endl;

			//  Construct a new JPEGHuffmanTree from the definition
			JPEGHuffmanTree ThisTree(*pHT);

			//  Get the tree to document itself
			ThisTree.documentTree(OS);

			//  Return to caller
			return;
		}

		//  showSOF0
		//
		//  This static function will document a Start Of Frame (type: 0, Baseline DCT) block 
		//
		//  PARAMETERS
		//
		//		size_t			-		Block number
		//		size_t			-		Frame number
		//		size_t			-		Offset in the file
		//		ODIMap&			-		Reference to the GIF On-Disk-Image map
		//		std::ostream&	-		Reference to the output stream
		//
		//  RETURNS
		//
		//  NOTES
		//

		static void		showSOF0(size_t BlockNo, size_t FrameNo, size_t Offset, ODIMap& Map, std::ostream& OS) {
			JFIF_FRAME_HEADER*		pFH = (JFIF_FRAME_HEADER*) (Map.Image + Offset);							//  Frame Headers Block

			OS << std::endl;
			OS << "JFIF/JPEG (Baseline DCT) FRAME: " << FrameNo << " HEADER - Block: " << BlockNo + 1 << ", Offset: +" << Offset << ", Size: " << Map.Blocks[BlockNo].BlockSize << "." << std::endl;

			//  Dump the memory image
			OS << std::endl;
			MemoryDumper::dumpMemory(Map.Image + Offset, Map.Blocks[BlockNo].BlockSize, "SOF0", OS);
			OS << std::endl;

			//  Show the content
			OS << std::endl;
			OS << " Frame Type:                    0 - non-differential Huffman (baseline DCT)." << std::endl;
			OS << " Sampling Precision:            " << int(pFH->Precision) << " bits." << std::endl;
			OS << " Number of lines (height):      ";
			if (GetSizeBE(pFH->HLines) == 0) OS << "0 - will be defined by a DNL block during scanning.";
			else OS << GetSizeBE(pFH->HLines) << ".";
			OS << std::endl;
			OS << " Samples per line (width):      " << GetSizeBE(pFH->VLines) << "." << std::endl;
			OS << " Number of colour components:   " << int(pFH->Components) << "." << std::endl;

			//  Show information from each component
			OS << std::endl;
			for (size_t CX = 0; CX < pFH->Components; CX++) {
				OS << " Component Identifier: " << int(pFH->Comp[CX].CompID);
				OS << ":" << std::endl;
				OS << "      Horizontal Sampling Factor:      " << GetHSampfactor(pFH->Comp[CX].HandV) << "." << std::endl;
				OS << "      Vertical Sampling Factor:        " << GetVSampFactor(pFH->Comp[CX].HandV) << "." << std::endl;
				OS << "      Quantization Table:              " << int(pFH->Comp[CX].QTable) << "." << std::endl;
				OS << std::endl;
			}

			//  Return to caller
			return;
		}

		//  showSOF1
		//
		//  This static function will document a Start Of Frame (type: 1) block 
		//
		//  PARAMETERS
		//
		//		size_t			-		Block number
		//		size_t			-		Frame number
		//		size_t			-		Offset in the file
		//		ODIMap&			-		Reference to the JFIF On-Disk-Image map
		//		std::ostream&	-		Reference to the output stream
		//
		//  RETURNS
		//
		//  NOTES
		//

		static void		showSOFX(size_t BlockNo, size_t FrameNo, size_t Offset, ODIMap& Map, std::ostream& OS) {
			JFIF_DATA_BLOCK*	pSOF = (JFIF_DATA_BLOCK*)(Map.Image + Offset);
			int			SOFType = (pSOF->ID & 0x0F);

			OS << std::endl;
			OS << "JFIF/JPEG (Type: " << SOFType << ") FRAME: " << FrameNo << " HEADER - Block: " << BlockNo + 1 << ", Offset: +" << Offset << ", Size: " << Map.Blocks[BlockNo].BlockSize << "." << std::endl;

			//  Dump the memory image
			OS << std::endl;
			MemoryDumper::dumpMemory(Map.Image + Offset, Map.Blocks[BlockNo].BlockSize, "SOFx", OS);
			OS << std::endl;

			//  Return to caller
			return;
		}

		//  showSOS
		//
		//  This static function will document a Start Of Scan block
		//
		//  PARAMETERS
		//
		//		size_t			-		Block number
		//		size_t			-		Offset in the file
		//		ODIMap&			-		Reference to the JFIF On-Disk-Image map
		//		std::ostream&	-		Reference to the output stream
		//
		//  RETURNS
		//
		//  NOTES
		//

		static void		showSOS(size_t BlockNo, size_t Offset, ODIMap& Map, std::ostream& OS) {
			JFIF_SCAN_HEADER1*		pSH = (JFIF_SCAN_HEADER1*) (Map.Image + Offset);								//  Start Of Scan Header
			JFIF_SCAN_HEADER2*		pSHX = (JFIF_SCAN_HEADER2*) (&pSH->Comp[pSH->Components]);						//  Header extension

			OS << std::endl;
			OS << "JFIF/JPEG START OF SCAN HEADER - Block: " << BlockNo + 1 << ", Offset: +" << Offset << ", Size: " << Map.Blocks[BlockNo].BlockSize << "." << std::endl;

			//  Dump the memory image
			OS << std::endl;
			MemoryDumper::dumpMemory(Map.Image + Offset, Map.Blocks[BlockNo].BlockSize, "SOS", OS);
			OS << std::endl;

			//  Show details
			for (size_t CX = 0; CX < pSH->Components; CX++) {
				OS << " Scan Colour Component: " << int(pSH->Comp[CX].ScanSelector) << " will use DC Huffman Table: " << GetDCSelector(pSH->Comp[CX].DCandAC);
				OS << " and AC Huffman Table: " << GetACSelector(pSH->Comp[CX].DCandAC) << "." << std::endl;
			}

			OS << " Start of spectral selection (first DCT coefficient):   " << int(pSHX->SSpecSel) << "." << std::endl;
			OS << " End of spectral selection (last DCT coefficient):      " << int(pSHX->ESpecSel) << "." << std::endl;
			OS << " Successive approximation bit position                  High: " << GetABSelHi(pSHX->AHiandLo) << ", Low: " << GetABSelLo(pSHX->AHiandLo) << "." << std::endl;

			//  Return to caller
			return;
		}

		//  showEEB
		//
		//  This static function will document a Entropy Encoded Block block
		//
		//  PARAMETERS
		//
		//		size_t			-		Block number
		//		size_t			-		Offset in the file
		//		ODIMap&			-		Reference to the JFIF On-Disk-Image map
		//		std::ostream&	-		Reference to the output stream
		//
		//  RETURNS
		//
		//  NOTES
		//

		static void		showEEB(size_t BlockNo, size_t Offset, ODIMap& Map, std::ostream& OS) {
			size_t		DumpLen = 0;																//  Dump length
			BYTE*		pSeg = Map.Image + Offset + 4;
			size_t		Stuffs = 0;																	//  Count of stuffing bytes

			OS << std::endl;
			OS << "JFIF/JPEG ENTROPY ENCODED - Block: " << BlockNo + 1 << ", Offset: +" << Offset << ", Size: " << Map.Blocks[BlockNo].BlockSize << "." << std::endl;

			//  Dump the memory image
			OS << std::endl;
			DumpLen = Map.Blocks[BlockNo].BlockSize;
			if (DumpLen > 512) DumpLen = 512;
			MemoryDumper::dumpMemory(Map.Image + Offset, DumpLen, "EEB", OS);
			OS << std::endl;

			//  Scan the entropy encoded segment to count the stuffing bytes
			while (*pSeg != JFIF_BLKID_SIG || *(pSeg + 1) == JFIF_BLKID_STUFF) {
				if (*pSeg == JFIF_BLKID_SIG) Stuffs++;
				pSeg++;
			}

			//  Show details
			OS << std::endl;
			OS << " Stuffing bytes:            " << Stuffs << "." << std::endl;
			OS << " Image data:                " << Map.Blocks[BlockNo].BlockSize - Stuffs << "." << std::endl;

			//  Return to caller
			return;
		}

		//  showRST
		//
		//  This static function will document a Restart block
		//
		//  PARAMETERS
		//
		//		size_t			-		Block number
		//		size_t			-		Offset in the file
		//		ODIMap&			-		Reference to the JFIF On-Disk-Image map
		//		std::ostream&	-		Reference to the output stream
		//
		//  RETURNS
		//
		//  NOTES
		//

		static void		showRST(size_t BlockNo, size_t Offset, ODIMap& Map, std::ostream& OS) {
			JFIF_DATA_BLOCK*	pRB = (JFIF_DATA_BLOCK*) (Map.Image + Offset);

			OS << std::endl;
			OS << "JFIF/JPEG RESTART - Block: " << BlockNo + 1 << ", Offset: +" << Offset << ", Size: " << Map.Blocks[BlockNo].BlockSize << "." << std::endl;

			//  Dump the memory image
			OS << std::endl;
			MemoryDumper::dumpMemory(Map.Image + Offset, Map.Blocks[BlockNo].BlockSize, "RST", OS);
			OS << std::endl;

			//  Show details
			OS << std::endl;
			OS << " Interval Identifier:       " << int(pRB->ID & 0x07) << "." << std::endl;

			//  Return to caller
			return;
		}

		//  showFT
		//
		//  This static function will document a File Trailer block
		//
		//  PARAMETERS
		//
		//		size_t			-		Block number
		//		size_t			-		Offset in the file
		//		ODIMap&			-		Reference to the JFIF On-Disk-Image map
		//		std::ostream&	-		Reference to the output stream
		//
		//  RETURNS
		//
		//  NOTES
		//

		static void		showFT(size_t BlockNo, size_t Offset, ODIMap& Map, std::ostream& OS) {

			OS << std::endl;
			OS << "JFIF/JPEG FILE TRAILER - Block: " << BlockNo + 1 << ", Offset: +" << Offset << ", Size: " << Map.Blocks[BlockNo].BlockSize << "." << std::endl;

			//  Dump the memory image
			OS << std::endl;
			MemoryDumper::dumpMemory(Map.Image + Offset, sizeof(JFIF_FILE_HEADER), "File Trailer", OS);
			OS << std::endl;

			//  Return to caller
			return;
		}
	};

	//
	//  JPEG class - This is an alias for the JFIF class
	//

	class JPEG : public JFIF {
	public:

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Public Constants                                                                                              *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  Options for storing images
		static const SWITCHES	JPEG_STORE_OPT_HIFI = 1;										//  High Fidelity Image 1x1 sampling

		//  Prevent instantiation
		JPEG() = delete;
	};
}
