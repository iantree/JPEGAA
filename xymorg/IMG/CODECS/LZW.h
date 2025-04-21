#pragma once
//*******************************************************************************************************************
//*																													*
//*   File:       LZW.h																								*
//*   Suite:      xymorg Integration - LZW CODEC																	*
//*   Version:    2.1.0	  Build:  03																				*
//*   Author:     Ian Tree/HMNL																						*
//*																													*
//*   Copyright 2016 - 2020 Ian J. Tree																				*			
//*******************************************************************************************************************
//*	LZW.h																											*
//*																													*
//*	This header file contains the class definition for the LZWCODEC class, the class provides code and				*
//* decode methods for the Lempel-Ziv-Walsh compression scheme.														*
//*																													*
//*	NOTES:																											*
//*																													*
//*																													*
//*******************************************************************************************************************
//*																													*
//*   History:																										*
//*																													*
//*	1.0.0 - 26/09/2016   -  Initial version																			*
//* 2.0.0 - 04/12/2014   -  Refactoring																				*
//* 2.1.0 - 02/10/2020   -  Adapted to use the RasterBuffer                                                         *
//*																													*
//*******************************************************************************************************************

//  Include basic xymorg headers
#include	"../../LPBHdrs.h"																					//  Language and Platform base headers
#include	"../../types.h"																						//  xymorg type definitions

//  Include xymorg image processing primitives
#include	"../types.h"
#include	"../consts.h"
#include	"../../CODECS/Bitstreams.h"																			//  Bit/Byte Stream classes

//  xymorg namespace
namespace xymorg {

	//*******************************************************************************************************************
	//*                                                                                                                 *
	//*   LZW Class																										*
	//*                                                                                                                 *
	//*   This class provides the implementation of the Lempel-Ziv-Walsh compression scheme.							*
	//*                                                                                                                 *
	//*******************************************************************************************************************

	class LZW {
	private:

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Private Type Definitions                                                                                      *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		typedef uint16_t		LZWCODE;												//  Basic LZW symbol code

		//  Dictionary

		typedef struct LZWDictEntry {
			BYTE			Value;														//  Data value unencoded
			LZWCODE			Parent;														//  Parent code
		} LZWDictEntry;

		typedef struct LZWDictionary {
			LZWCODE			HiCode;		 												//  High code in use
			LZWCODE			ClearCode;													//  Clear code
			LZWCODE			EndCode;													//  End Code
			BYTE*			pStringStack;												//  Pointer to the string stack
			UINT			StackIndex;													//  Used index into the string stack
			LZWDictEntry	Entry[4096];												//  Array of dictionary entries
		} LZWDictionary;

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Private Constants			                                                                                    *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		static const int		MAX_LZW_STRINGSTACK = 4096;								//  Maximum sixe of the string stack
		static const LZWCODE	MAX_LZW_CODE = 4095;									//  Maximum possible LZW code
		
	public:

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Forward Declarations                                                                                          *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		class Emitter;
		class Collecter;

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Constructors                                                                                                  *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  Default Constructor
		//
		//  Constructs a new LZW Codec instance
		//
		//  PARAMETERS
		//
		//  RETURNS
		//
		//  NOTES
		//
		//

		LZW() {

			//  Clear the state to NULL
			pDict = nullptr;
			NativeCodeSize = 0;
			Bits = 0;
			CurrentCode = 0;
			NextCode = 0;
			StartOfString = 0;
			EndOfString = 0;
			NextEndOfString = 0;
			CleanTermination = false;
			ClearPolicy = true;
			Tokens = 0;

			//  Return to caller
			return;
		}

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Destructor                                                                                                    *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		~LZW() {

			//  Free the dictionary (if it remains allocated)
			if (pDict != nullptr) {
				free(pDict);
				pDict = nullptr;
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

		//  Behaviour Modifiers

		//  enableClearOnFull
		//
		//	Sets the Dictionary Full Reset Policy flag ON.
		//
		//  PARAMETERS
		//
		//  RETURNS
		//
		//  NOTES
		//

		void	enableClearOnFull() { ClearPolicy = true; return; }

		//  disableClearOnFull
		//
		//	Sets the Dictionary Full Reset Policy flag OFF.
		//
		//  PARAMETERS
		//
		//  RETURNS
		//
		//  NOTES
		//

		void	disableClearOnFull() { ClearPolicy = false; return; }

		//  Decoding Functions

		//  decode
		//
		//	Sets the CODEC up to decode an input ByteStream into a plain ByteStream and performs the decoding.
		//
		//  PARAMETERS
		//
		//		ByteStream&					-	Reference to an input ByteStream
		//		ByteStream&					-	Reference to an output ByteStream
		//		int							-	Native Symbol Code Size
		//
		//  RETURNS
		//
		//		bool						-	true if the decode was successful, otherwise false
		//
		//  NOTES
		//

		bool decode(ByteStream& bsIn, ByteStream& bsOut, int NCS) {

			//  Check that we have input available
			if (bsIn.getRemainder() == 0) return false;

			//  Obtain the Decoding Emitter for the input stream
			Emitter		DEmit = decode(bsIn, NCS);

			//  Loop copying the Emitted (decoded) content into the output stream
			while (DEmit.hasNext()) bsOut.next(DEmit.next());

			//  return success
			return true;
		}

		//  decode
		//
		//	Sets the CODEC up to decode an input ByteStream and returns the Emitter object that performs the decoding.
		//	The emitter is an 'on-demand' decoder providing a Just-In-Time (JIT) decoding.
		//
		//  PARAMETERS
		//
		//		ByteStream&					-	Reference to an input ByteStream
		//		int							-	Native Symbol Code Size
		//
		//  RETURNS
		//
		//		Emitter						-	Decoding Emitter
		//
		//  NOTES
		//

		Emitter	decode(ByteStream& bsIn, int NCS) {

			//  Create and initialise a new LZW Dictionary
			if (pDict != nullptr) {
				free(pDict);
				pDict = nullptr;
			}

			//  Set the native code size
			NativeCodeSize = NCS;

			//  Allocate space for the dictionary plus the string stack
			pDict = (LZWDictionary*) malloc(sizeof(LZWDictionary) + MAX_LZW_STRINGSTACK);
			if (pDict != nullptr) {
				memset(pDict, 0, sizeof(LZWDictionary) + MAX_LZW_STRINGSTACK);
				pDict->pStringStack = (BYTE*) (pDict + 1);

				//  Set the initial width of the bit stream
				Bits = NCS + 1;

				//  Set the initial codes in the dictionary
				pDict->ClearCode = 1 << NCS;
				pDict->EndCode = pDict->ClearCode + 1;
				pDict->HiCode = pDict->EndCode;
				pDict->StackIndex = 0;
				for (int iIndex = 0; iIndex < pDict->ClearCode; iIndex++) pDict->Entry[iIndex].Value = BYTE(iIndex);
			}

			//  Set the clean termination to false
			CleanTermination = false;

			//  Clear the token count
			Tokens = 0;

			//  Return the Emitter
			return Emitter(*this, bsIn);
		}

		//  Encoding Functions

		//  encode
		//
		//	Sets the CODEC up to encode an input ByteStream into an output Byte Stream
		//
		//  PARAMETERS
		//
		//		ByteStream&					-	Reference to an input ByteStream
		//		ByteStream&					-	Reference to an output ByteStream
		//		int							-	Native Symbol Code Size
		//
		//  RETURNS
		//
		//		bool					-	true if the stream was encoded, otherwise false
		//
		//  NOTES
		//

		bool	encode(ByteStream& bsIn, ByteStream& bsOut, int NCS) {

			//  Check that we have input available
			if (bsIn.getRemainder() == 0) return false;

			//  Condition the CODEC and obtain the collector
			Collecter	CIn = encode(bsOut, NCS);

			//  Set the clean termination to false
			CleanTermination = false;

			//  Push the cobtent of the input stream into the collector
			while (!bsIn.eos()) CIn.next(bsIn.next());

			//  Return success
			return true;
		}

		//  encode
		//
		//	Sets the CODEC up to encode an input ByteStream into an output Byte Stream
		//
		//  PARAMETERS
		//
		//		ByteStream&					-	Reference to an output ByteStream
		//		int							-	Native Symbol Code Size (Bits Per Pixel)
		//
		//  RETURNS
		//
		//		Collector				-	The Collector device for providing input
		//
		//  NOTES
		//

		Collecter	encode(ByteStream& bsOut, int NCS) {

			int			iIndex;																	//  Generic index

			//  Create and initialise a new LZW Dictionary
			if (pDict != NULL) {
				free(pDict);
			}

			NativeCodeSize = NCS;
			pDict = NULL;
			//  Allocate space for the dictionary plus the string stack
			pDict = (LZWDictionary*) malloc(sizeof(LZWDictionary) + MAX_LZW_STRINGSTACK);
			if (pDict != NULL) {
				memset(pDict, 0, sizeof(LZWDictionary) + MAX_LZW_STRINGSTACK);
				pDict->pStringStack = (BYTE*)(pDict + 1);

				//  Set the initial width of the bit stream
				Bits = NCS + 1;

				//  Set the initial codes to the dictionary
				pDict->ClearCode = 1 << NCS;
				pDict->EndCode = pDict->ClearCode + 1;
				pDict->HiCode = pDict->EndCode;
				pDict->StackIndex = 0;
				for (iIndex = 0; iIndex < pDict->ClearCode; iIndex++) pDict->Entry[iIndex].Value = BYTE(iIndex);

				//  Initialise the string
				StartOfString = pDict->EndCode;
				EndOfString = pDict->EndCode;
				NextEndOfString = pDict->EndCode;

				//  Clear the token count
				Tokens = 0;
			}

			//  Return the Collecter
			return Collecter(*this, bsOut);
		}

		//  wasDecodeClean
		//
		//	Returns a signal that the decoded was/was not terminated cleanly by detection of a sofr End-Of-Stream in the Token stream,
		//
		//  PARAMETERS
		//
		//  RETURNS
		//
		//		bool				-	true if the terminal condition was clean, otherwise false.
		//
		//  NOTES
		//

		bool		wasDecodeClean() { return CleanTermination; }

		//  isDictionaryFull
		//
		//	Returns a signal that the dictionary is now full
		//
		//  PARAMETERS
		//
		//  RETURNS
		//
		//		bool				-	true if the dictionary is full, otherwise false.
		//
		//  NOTES
		//

		bool		isDictionaryFull() { if (pDict->HiCode == MAX_LZW_CODE) return true; return false; }

		//  getTokenCount
		//
		//	Returns the number of tokens read from or written to the compressed stream
		//
		//  PARAMETERS
		//
		//  RETURNS
		//
		//		size_t				-	Number of tokrns
		//
		//  NOTES
		//

		size_t		getTokenCount() { return Tokens; }

private:

		//*******************************************************************************************************************
		//*																													*
		//*  Private Members																								*
		//*																													*
		//*******************************************************************************************************************

		//  Reference items for encoding/decoding
		LZWDictionary*		pDict;																	//  LZW Dictionary
		int					NativeCodeSize;															//  Native symbol code size
		int					Bits;																	//  Current bit width of the data stream
		LZWCODE				CurrentCode;															//  Current code being peocessed from the stream
		LZWCODE				NextCode;																//  Next code to be processed from the stream
		LZWCODE				StartOfString;															//  Code at the start of the current string
		LZWCODE				EndOfString;															//  Code at the end of the current string
		LZWCODE				NextEndOfString;														//  Code for the next string
		bool				CleanTermination;														//  Signal for clean image stream
		size_t				Tokens;																	//  Number of tokens Read/Written

		//
		//  The ClearPolicy flag determines the policy to be used by the encoder when the dictionary becomes full.
		//
		//	If the ClearPolicy flag is set (true) then the encoder will issue a deferred clear in the token stream
		//	to cause the decoder to reset the dictionary to the starting state.
		//	If not set (false) then the dictionary remains in use as-is and no more entries are added.
		//
		//	The default policy is on (true)
		//
		//	The decision to set the ClearPolicy on MUST be made by higher level code based on knowledge of the image
		//	structure. 
		//
		//  If a scan of the image were to reveal the most of the image had common patterns of pixels but a patch
		//  in the middle had distinctly different patterns then suppressing the disctionary clear might be a good
		//  idea.
		//
		//	Most CODEC imlementations ALWAYS issue a clear when the dictionary is full (hence the default).
		//

		bool			ClearPolicy;																//  Reset the Dictionary when it is full

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Nested Public Classes																							*
		//*                                                                                                                 *
		//*******************************************************************************************************************

public:

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Emitter Class																									*
		//*                                                                                                                 *
		//*   The Emitter functions like an iterator providing a means of emitting the decompressed data stream.			*
		//*                                                                                                                 *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		class Emitter {
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

			//  Normal Constructor
			//
			//  Constructs a new Decoding Iterator (Emitter) and conditions the CODEC to the initial state
			//
			//  PARAMETERS
			//
			//		LZWCODEC&		-		Reference to the parent LZWCODEC
			//		ByteStream&		-		Reference to the Input ByteStream
			//
			//  RETURNS
			//
			//  NOTES
			//
			//

			Emitter(LZW& Parent, ByteStream& bsIn)
				: CODEC(Parent), BStream(bsIn, false) {

				//  Prime the decoder by reading and discarding the initial clear codes from the stream
				CODEC.NextCode = CODEC.pDict->ClearCode;
				while (CODEC.NextCode == CODEC.pDict->ClearCode) {
					CODEC.NextCode = LZWCODE(BStream.next(CODEC.Bits));
					CODEC.Tokens++;
				}

				//  Set the current code from the next code
				CODEC.CurrentCode = CODEC.NextCode;

				//  Return to caller
				return;
			}

			//*******************************************************************************************************************
			//*                                                                                                                 *
			//*   Destructor                                                                                                    *
			//*                                                                                                                 *
			//*******************************************************************************************************************

			~Emitter() {
				if (CODEC.pDict != nullptr) {
					free(CODEC.pDict);
					CODEC.pDict = nullptr;
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

			//  hasNext
			//
			//  Detects if the Emitter has more data available or has reached the end
			//
			//  PARAMETERS
			//
			//  RETURNS
			//
			//		bool				-		true if a symbol is available, false if the end-of-stream has been reached
			//
			//  NOTES
			//

			bool hasNext() {
				int		iIndex;																	//  Generic Index

				//  Processing for decompression
				if (CODEC.pDict->StackIndex > 0) return true;
				if (CODEC.CurrentCode == CODEC.pDict->EndCode) {
					CODEC.CleanTermination = true;												//  Signal a clean termination condition
					return false;																//  Soft end-of-stream has been reached	
				}
				if (BStream.eos()) return false;

				//  Process the next code from the stream
				CODEC.NextCode = LZWCODE(BStream.next(CODEC.Bits));								//  Buffer the next code to use
				CODEC.Tokens++;

				//  Write the symbol string for current code to the string stack
				iIndex = CODEC.CurrentCode;
				while (iIndex >= 0) {
					CODEC.pDict->pStringStack[CODEC.pDict->StackIndex++] = CODEC.pDict->Entry[iIndex].Value;
					if (iIndex < CODEC.pDict->ClearCode) iIndex = -1;
					else iIndex = CODEC.pDict->Entry[iIndex].Parent;
				}

				//  If the next code is a clear code then reset the dictionary
				if (CODEC.NextCode == CODEC.pDict->ClearCode) {
					//  Reset the code width to the original width
					CODEC.Bits = CODEC.NativeCodeSize + 1;
					//  Reset the high used code (clear the coded entries in the dictionary)
					CODEC.pDict->HiCode = CODEC.pDict->EndCode;
					//  Reprime the read loop by reading the current code from the stream
					CODEC.CurrentCode = LZWCODE(BStream.next(CODEC.Bits));
					CODEC.Tokens++;
				}
				else {
					//  If the next code is already in the dictionary then add the first code of that string
					//  to the string just written as a new string.
					//  If not in the dictionary (yet) then add the first code from the current string to
					//  extend the string.
					//  Once the dictionary is full no more codes are added until a clear code is received.

					if (CODEC.pDict->HiCode < MAX_LZW_CODE) {
						if (CODEC.NextCode <= CODEC.pDict->HiCode) addToDictionary(CODEC.CurrentCode, getFirstCode(CODEC.NextCode));
						else addToDictionary(CODEC.CurrentCode, getFirstCode(CODEC.CurrentCode));
					}

					//  Determine if the width of the Bitstream needs to be increased 
					if (CODEC.pDict->HiCode == ((1 << CODEC.Bits) - 1) && CODEC.Bits < 12) CODEC.Bits++;

					//  Cycle the next code to become the current code
					CODEC.CurrentCode = CODEC.NextCode;
				}

				//  Default - next symbol is available in the stream
				return true;
			}

			//  next
			//
			//  Returns the next symbol from the decompressed stream
			//
			//  PARAMETERS
			//
			//  RETURNS
			//
			//		BYTE				-		Next symbol from the compressed or ecompressed stream
			//
			//  NOTES
			//
			//		The behaviour of reading after the end-of-stream is undefined (so don't do it!)
			//

			BYTE next() {
				//  Processing for decompression
				return CODEC.pDict->pStringStack[--CODEC.pDict->StackIndex];
			}

			//*******************************************************************************************************************
			//*                                                                                                                 *
			//*   Operator Overload Functions                                                                                   *
			//*                                                                                                                 *
			//*******************************************************************************************************************

		private:

			//*******************************************************************************************************************
			//*																													*
			//*  Private Members																								*
			//*																													*
			//*******************************************************************************************************************

			LZW&			CODEC;																	//  Parent CODEC
			LSBitStream		BStream;																//  Input Bit Stream

			//*******************************************************************************************************************
			//*                                                                                                                 *
			//*   Private Functions                                                                                             *
			//*                                                                                                                 *
			//*******************************************************************************************************************

			//  addToDictionary
			//
			//	This function will add a new code to the LZW dictionary.
			//
			//	PARAMETERS
			//
			//		LZWCODE						-	Code of the string to be extended
			//		LZWCODE						-	The Code value to be added to the string
			//		
			//
			//  RETURNS
			//
			//	NOTES
			//
			//

			void addToDictionary(LZWCODE String, LZWCODE NextCode) {
				//  Increment the high used codes
				CODEC.pDict->HiCode++;

				//  Fill the code entry
				CODEC.pDict->Entry[CODEC.pDict->HiCode].Parent = String;
				CODEC.pDict->Entry[CODEC.pDict->HiCode].Value = BYTE(NextCode);
				return;
			}

			//  getFirstCode
			//
			//	This function will return the first code for the string passed.
			//
			//	PARAMETERS
			//
			//		LZWCODE						-	The code for the string
			//		
			//
			//  RETURNS
			//
			//		LZWCODE	-	The first code of the passed string
			//
			//	NOTES
			//

			LZWCODE getFirstCode(LZWCODE String) {
				//  Cascade up through the parent node chain until the start of the string is located
				while (String > CODEC.pDict->EndCode) String = CODEC.pDict->Entry[String].Parent;
				return String;
			}
		};

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   collecter Class																								*
		//*                                                                                                                 *
		//*   The collecter functions like an iterator providing a means of capturing the unencoded data stream.			*
		//*                                                                                                                 *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		class Collecter {
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

			//  Normal Constructor
			//
			//  Constructs a new Encoding Iterator (collecter) and conditions the CODEC to the initial state
			//
			//  PARAMETERS
			//
			//		LZWCODEC&		-		Reference to the parent LZW CODEC
			//		ByteStream&		-		Reference to the Output Stream
			//
			//  RETURNS
			//
			//  NOTES
			//
			//

			Collecter(LZW& Parent, ByteStream& bsOut)
				: CODEC(Parent), BStream(bsOut, true) {

				//  Emit the initial CLEAR code to the compressed stream
				BStream.next(CODEC.pDict->ClearCode, CODEC.Bits);
				CODEC.Tokens++;

				//  Return to caller
				return;
			}

			//*******************************************************************************************************************
			//*                                                                                                                 *
			//*   Destructor                                                                                                    *
			//*                                                                                                                 *
			//*******************************************************************************************************************

			~Collecter() {
				if (CODEC.pDict != nullptr) {
					free(CODEC.pDict);
					CODEC.pDict = nullptr;
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

			//  next
			//
			//  Submits the next symbol from the uncompressed stream
			//
			//  PARAMETERS
			//
			//		BYTE				-		Next symbol from the umompressed stream
			//
			//  RETURNS
			//
			//  NOTES
			//
			//

			void next(BYTE NextSymbol) {

				//  Save the input symbol
				CODEC.NextCode = NextSymbol;

				//  Determine if we are starting a new string
				if (CODEC.StartOfString == CODEC.pDict->EndCode) {
					//  New String
					CODEC.StartOfString = CODEC.NextCode;
					CODEC.EndOfString = CODEC.NextCode;
					return;
				}

				//  Determine if we can extend the string and still match an existing string in the dictionary
				//  If the code returned is less than or equal to the high used code then the string continues
				//  to match an existing string in the dictionary.
				//  If the code returned exceeds the high used code then the new code must be added, extending
				//  the string, unless the dictionary is full.

				CODEC.NextEndOfString = matchString(CODEC.EndOfString, CODEC.NextCode);

				//  Test if the extended string already exists in the dictionary
				if (CODEC.NextEndOfString <= CODEC.pDict->HiCode) {
					//  Extend the string
					CODEC.EndOfString = CODEC.NextEndOfString;
					return;
				}

				//  Create a new matching string in the dictionary
				if (CODEC.pDict->HiCode < MAX_LZW_CODE) {

					//  Emit the string code to the compressed stream
					BStream.next(CODEC.EndOfString, CODEC.Bits);
					CODEC.Tokens++;

					//  Check if we need to increase the current width of the codes
					if (CODEC.pDict->HiCode == ((1 << CODEC.Bits) - 1) && CODEC.Bits < 12) CODEC.Bits++;

					//  Add the new extended code to the dictionary
					addToDictionary(CODEC.EndOfString, CODEC.NextCode);

					//  Reset the string to begin a new scan
					CODEC.StartOfString = CODEC.NextCode;
					CODEC.EndOfString = CODEC.NextCode;
					return;
				}

				//  The dictionary is now full, the action to take depends on the encoding clear policy setting
				if (CODEC.ClearPolicy) {

					//  Clear policy is to emit a deferred clear to reset the dictionary in the decoder

					//  1. Emit the last matched string
					//  2. Emit the CLEAR code
					//  3. Reset the code witdth to the initial setting
					//  4. Clear the dictionary
					//  5. Reset the compression loop

					BStream.next(CODEC.EndOfString, CODEC.Bits);
					CODEC.Tokens++;
					BStream.next(CODEC.pDict->ClearCode, CODEC.Bits);
					CODEC.Tokens++;
					CODEC.Bits = CODEC.NativeCodeSize + 1;
					CODEC.pDict->HiCode = CODEC.pDict->EndCode;
					CODEC.StartOfString = CODEC.NextCode;
					CODEC.EndOfString = CODEC.NextCode;
					return;
				}

				//  The clear policy is to continue using the full dictionary
				//  Emit the existing string and start a new string

				BStream.next(CODEC.EndOfString, CODEC.Bits);
				CODEC.Tokens++;
				CODEC.StartOfString = CODEC.NextCode;
				CODEC.EndOfString = CODEC.NextCode;

				//  Return to caller
				return;
			}

			//  signalEndOfStream
			//
			//  Signals that the end of the uncompressed data stream has been reached
			//
			//  PARAMETERS
			//
			//  RETURNS
			//
			//		size_t				-		The size of the compressed stream that was written
			//
			//  NOTES
			//
			//

			void signalEndOfStream() {

				//  Emit the final string to the compressed stream
				BStream.next(CODEC.EndOfString, CODEC.Bits);
				CODEC.Tokens++;

				//  Emit the End-Of-Stream code to the compressed stream
				BStream.next(CODEC.pDict->EndCode, CODEC.Bits);
				CODEC.Tokens++;

				//  Flush the stream - this will close the current segment and add a zero length segment to the end of the stream
				BStream.flush();

				//  Return the size of the compressed stream
				return;
			}

		private:

			//*******************************************************************************************************************
			//*																													*
			//*  Private Members																								*
			//*																													*
			//*******************************************************************************************************************

			LZW&			CODEC;																	//  Parent CODEC
			LSBitStream		BStream;																//  Output Bit Stream

			//*******************************************************************************************************************
			//*                                                                                                                 *
			//*   Private Functions                                                                                             *
			//*                                                                                                                 *
			//*******************************************************************************************************************

			//  matchString
			//
			//	This function will determine if the current string can be extended 
			//
			//	PARAMETERS
			//
			//		LZWCODE						-	The code of the current string node
			//		LZWCODE						-	The Code value to be added
			//		
			//
			//  RETURNS
			//
			//		LZWCODE						-	1.  The new code to be used for an unmatched string
			//										2.  An existing code that extends the current string
			//
			//	NOTES
			//
			//

			LZWCODE matchString(LZWCODE String, LZWCODE Code) {
				USHORT			MatchEntry;									//  Entry we are attempting to match against

				//  Boundary condition - no strings in the table yet
				if (CODEC.pDict->HiCode == CODEC.pDict->EndCode) return LZWCODE(CODEC.pDict->HiCode + 1);

				//  Search the dictionary trying to locate an entry that has the String as a parent and the code as the next character
				for (MatchEntry = CODEC.pDict->EndCode + 1; MatchEntry <= CODEC.pDict->HiCode; MatchEntry++)
				{
					if (CODEC.pDict->Entry[MatchEntry].Parent == String && CODEC.pDict->Entry[MatchEntry].Value == BYTE(Code)) break;
				}

				if (MatchEntry <= CODEC.pDict->HiCode) return MatchEntry;

				return LZWCODE(CODEC.pDict->HiCode + 1); 
			}

			//  addToDictionary
			//
			//	This function will add a new code to the LZW dictionary.
			//
			//	PARAMETERS
			//
			//		LZWCODE						-	Code of the string to be extended
			//		LZWCODE						-	The Code value to be added to the string
			//		
			//
			//  RETURNS
			//
			//	NOTES
			//
			//

			void addToDictionary(LZWCODE String, LZWCODE NextCode) {
				//  Increment the high used codes
				CODEC.pDict->HiCode++;

				//  Fill the code entry
				CODEC.pDict->Entry[CODEC.pDict->HiCode].Parent = String;
				CODEC.pDict->Entry[CODEC.pDict->HiCode].Value = BYTE(NextCode);
				return;
			}

		};

	};

}
