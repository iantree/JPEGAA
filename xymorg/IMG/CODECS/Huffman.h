#pragma once
//*******************************************************************************************************************
//*																													*
//*   File:       Huffman.h																							*
//*   Suite:      xymorg Integration - HUFFMAN CODEC																*
//*   Version:    1.0.0	  Build:  01																				*
//*   Author:     Ian Tree/HMNL																						*
//*																													*
//*   Copyright 2016 - 2020 Ian J. Tree																				*
//*******************************************************************************************************************
//*	Huffman.h																										*
//*																													*
//*	This header file contains the class definition for the Huffman class, the class provides code and				*
//* decode methods for the Huffman compression scheme.																*
//*																													*
//*	NOTES:																											*
//*																													*
//*	A specialised constructor is required for each instance that provides the HuffmanTree to use.					*
//*																													*
//*																													*
//*******************************************************************************************************************
//*																													*
//*   History:																										*
//*																													*
//*	1.0.0 - 26/09/2016   -  Initial version																			*
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
	//*   Huffman Class																									*
	//*                                                                                                                 *
	//*   This class provides the implementation of the Huffman compression scheme.										*
	//*                                                                                                                 *
	//*******************************************************************************************************************

	class Huffman {
	private:
		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Private Nested Structures                                                                                     *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		typedef struct EncodedUnit {
			uint16_t				Length;													//  Number of bits
			uint16_t				Bits;													//  Bit String
		} EncodedUnit;

		//  Special coding unit for Coefficient encoding/decoding
		typedef struct JCEU {
			BYTE			Zeroes;
			BYTE			Category;
			BYTE			Sign;
			BYTE			Bits;
			uint32_t		Magnitude;
		} JCEU;

	public:

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Forward Declarations                                                                                          *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		class Emitter;
		class JPEGEmitter;
		class Collecter;
		class JPEGCollecter;
		class HuffmanTree;

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Constructors                                                                                                  *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  Default Constructor
		//
		//  Constructs a new Huffman Codec instance
		//
		//  PARAMETERS
		//
		//  RETURNS
		//
		//  NOTES
		//
		//

		Huffman() {

			//  Clear the current tree
			CurrentTree = nullptr;

			//  Return to caller
			return;
		}

	public:

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Destructor                                                                                                    *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		~Huffman() {

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

		//  Configuration Functions

		//  setTree
		//
		//  Sets the HuffmanTree object that is to be used for encoding/decoding
		//
		//  PARAMETERS
		//
		//		HuffmanTree*		-		Pointer to the HuffmanTree to use for the following operations
		//
		//  RETURNS
		//
		//  NOTES
		//
		//

		void setTree(HuffmanTree* NewTree) { CurrentTree = NewTree; return; }

		//  Decoding Functions

		//  decode
		//
		//	Sets the CODEC up to decode an input ByteStream and returns the Emitter object that performs the decoding.
		//	The emitter is an 'on-demand' decoder providing a Just-In-Time (JIT) decoding.
		//
		//  PARAMETERS
		//
		//		ByteStream&					-	Reference to an input ByteStream
		//
		//  RETURNS
		//
		//		Emitter						-	Decoding Emitter
		//
		//  NOTES
		//

		Emitter	decode(ByteStream& bsIn) {

			//  Return the Emitter
			return Emitter(*this, bsIn);
		}

		//  decodeJPEG
		//
		//	Sets the CODEC up to decode an input ByteStream and returns the Emitter object that performs the decoding.
		//	The emitter is an 'on-demand' decoder providing a Just-In-Time (JIT) decoding.
		//
		//  PARAMETERS
		//
		//		ByteStream&					-	Reference to an input ByteStream
		//
		//  RETURNS
		//
		//		JPEGEmitter						-	Decoding Emitter
		//
		//  NOTES
		//

		JPEGEmitter	decodeJPEG(ByteStream& bsIn) {

			//  Return the Emitter
			return JPEGEmitter(*this, bsIn);
		}

		//  Encoding Functions

		//  encode
		//
		//	Sets the CODEC up to encode an input ByteStream into an output Byte Stream
		//
		//  PARAMETERS
		//
		//		ByteStream&					-	Reference to an output ByteStream
		//
		//  RETURNS
		//
		//		Collector				-	The Collector device for providing input
		//
		//  NOTES
		//

		Collecter	encode(ByteStream& bsOut) {

			//  Return the Collecter
			return Collecter(*this, bsOut);
		}

		//  encodeJPEG
		//
		//	Sets the CODEC up to encode an input ByteStream into an output Byte Stream
		//
		//  PARAMETERS
		//
		//		ByteStream&					-	Reference to an output ByteStream
		//
		//  RETURNS
		//
		//		JPEGCollector				-	The Collector device for providing input
		//
		//  NOTES
		//

		JPEGCollecter	encodeJPEG(ByteStream& bsOut) {

			//  Return the Collecter
			return JPEGCollecter(*this, bsOut);
		}

private:

		//*******************************************************************************************************************
		//*																													*
		//*  Private Members																								*
		//*																													*
		//*******************************************************************************************************************

		//  HuffManTree to use for Encoding/Decoding
		HuffmanTree*						CurrentTree;

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Nested Public Classes																							*
		//*                                                                                                                 *
		//*******************************************************************************************************************

public:

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   HuffmanTree Class																								*
		//*                                                                                                                 *
		//*   The HuffmanTree class allows encoding and decoding of variable length bit codes against the tree structure	*
		//*   for the particular Huffman compression implementation.														*
		//*   The class should be extended with additional constructors that build the tree from information in the			*
		//*	  implementation context.																						*
		//*                                                                                                                 *
		//*                                                                                                                 *
		//*******************************************************************************************************************

	class HuffmanTree {
	public:
		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Public Constants                                                                                              *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Forward Declarations                                                                                          *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		class HuffmanNode;

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Constructors                                                                                                  *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  Default Constructor
		HuffmanTree() {
			RootNode = nullptr;
			CurrentNode = nullptr;
			ELUTBuilt = false;
			memset(ELUT, 0, 256 * sizeof(HuffmanNode*));
			return;
		}

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Destructor                                                                                                    *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		~HuffmanTree() {
			if (RootNode != nullptr) delete RootNode;
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

		//  Accessors
		void			setRootNode(HuffmanNode* NewRoot) { RootNode = NewRoot, CurrentNode = NewRoot; return; }
		HuffmanNode*	getRootNode() { return RootNode; }
		void			setCurrentNode(HuffmanNode* NewNode) { CurrentNode = NewNode; return; }

		//  Decoding Functions

		//  decode
		//
		//  Progressive decoding accepts the next bit from the bit stream and updates the current position in the tree
		//  returns true once the position reaches a leaf node.
		//
		//	PARAMETERS:
		//
		//		bool			-		Next bit value from the bit stream
		//
		//	RETURNS:
		//
		//		bool			-		true if positioned at a leaf node, otherwise false
		//
		//  NOTES:
		//

		bool	decode(bool NextBit) {
			if (CurrentNode == nullptr) CurrentNode = RootNode;
			if (NextBit) {
				if (CurrentNode->getOne() == nullptr) return true;
				CurrentNode = CurrentNode->getOne();
			}
			else {
				if (CurrentNode->getZero() == nullptr) return true;
				CurrentNode = CurrentNode->getZero();
			}

			return CurrentNode->isLeaf();
		}

		//  getDecode
		//
		//  Returns the decoded byte at the leaf node of the current position in the tree.
		//
		//	PARAMETERS:
		//
		//	RETURNS:
		//
		//		BYTE			-		Decoded symbol at the current leaf node
		//
		//  NOTES:
		//
		//  MUST only be called when the decode() function has signalled true (leaf node reached)
		//

		BYTE	getDecode() {
			BYTE	Symbol = 0;
			if (CurrentNode != NULL) Symbol = CurrentNode->getSymbol();
			CurrentNode = RootNode;
			return Symbol;
		}

		//  Encoding Functions

		//  encode
		//
		//  Returns the passed value encoded against the current tree structure.
		//
		//	PARAMETERS:
		//
		//		BYTE			-		Decoded symbol at the current leaf node
		//
		//	RETURNS:
		//
		//		EncodedUnit			-		Structure containing the variable length bit string of the encoded symbol
		//
		//  NOTES:
		//

		EncodedUnit	encode(BYTE Symbol) {
			EncodedUnit		Encode = { 0, 0 };

			//  If the encode lookup table has not yet been built then build it
			if (!ELUTBuilt) buildELUT();

			//  Get the address of the leaf node that holds the symbol to be encoded
			if (ELUT[Symbol] == NULL) return Encode;
			CurrentNode = ELUT[Symbol];

			//  Build up the Encod progressively by identifying which path was used to get to the ccurrent node
			while (!CurrentNode->isRoot()) {
				Encode.Length++;
				Encode.Bits = (Encode.Bits >> 1) & 0x7FFF;
				if (CurrentNode->getParent()->getOne() == CurrentNode) Encode.Bits = Encode.Bits | 0x8000;
				CurrentNode = CurrentNode->getParent();
			}
			//  Shift LSB to LSB position
			Encode.Bits = Encode.Bits >> (16 - Encode.Length);

			//  Reset the current node - in case of a mode change
			CurrentNode = RootNode;

			return Encode;
		}

		//  Documentation Functions

		//  documentTree
		//
		//  Documents the content of the tree in indented explosion format
		//
		//	PARAMETERS:
		//
		//		ostream&			-		Reference to the ostream to produce the output on
		//
		//	RETURNS:
		//
		//  NOTES:
		//

		void	documentTree(std::ostream& os) {
			//  Show titles
			os << "HUFFMAN TREE (0x" << this << ")" << std::endl;
			os << "------------" << std::endl;
			os << std::endl;

			//  Perform an indented explosion on each node in the tree
			if (RootNode == NULL) os << "ERROR: No tree has been loaded." << std::endl;
			else documentNode(os, *RootNode, 0);

			//  Return to caller
			return;
		}

	private:

		//*******************************************************************************************************************
		//*																													*
		//*  Private Members																								*
		//*																													*
		//*******************************************************************************************************************

		//  Nodes in the tree
		HuffmanNode*		RootNode;																//  Root of the tree
		HuffmanNode*		CurrentNode;															//  Node position for decoding

		//  Encoding lookup table - array of pinters to the leaf node holding the symbol
		bool				ELUTBuilt;																//  Table has been built
		HuffmanNode*		ELUT[256];																//  Array of node pointers

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Private Functions                                                                                             *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  documentNode
		//
		//  Recursive function to document a single node in the tree and recurse to the next node
		//
		//	PARAMETERS:
		//
		//		ostream&			-		Reference to the ostream to produce the output on
		//		HuffmanNode&		-		Reference to the node to be documented
		//		int					-		Current level of the exlosion
		//
		//	RETURNS:
		//
		//		bool				-		if true signals to the parent to terminate the explosion, false continue
		//
		//  NOTES:
		//

		bool documentNode(std::ostream& OS, HuffmanNode& Node, int Level) {
			int			iIndex;

			//  Recursion protection - if the levels greater than 256 this is a malformed (probably looping) tree
			//  Signal the parent to terminate the explosion.
			if (Level > 256) {
				OS << "ERROR: The Huffman Tree is malformed, probably contains loops - terminating the explosion." << std::endl;
				return true;
			}

			//  Document the NODE
			//  First output the indentation markers
			OS << " ";
			for (iIndex = 0; iIndex < Level; iIndex++) OS << ".";
			OS << " ";

			//  Show the node type and identity
			if (Node.isRoot()) OS << "Root Node (0x" << &Node << ") - ";
			else {
				if (Node.isLeaf()) OS << "Leaf Node (0x" << &Node << ") - ";
				else OS << "Branch Node (0x" << &Node << ") - ";
			}

			//  Show the Node contents
			if (Node.isLeaf()) {
				OS << "Symbol: " << int(Node.getSymbol());

				//  Show the encoding string and length
				EncodedUnit  Encode = { 0,0 };

				//  Build the encoding details
				CurrentNode = &Node;

				//  Build up the Encode progressively by identifying which path was used to get to the current node
				while (!CurrentNode->isRoot()) {
					Encode.Length++;
					Encode.Bits = (Encode.Bits >> 1) & 0x7FFF;
					if (CurrentNode->getParent()->getOne() == CurrentNode) Encode.Bits = Encode.Bits | 0x8000;
					CurrentNode = CurrentNode->getParent();
				}
				//  Shift LSB to LSB position
				Encode.Bits = Encode.Bits >> (16 - Encode.Length);

				//  Reset the current node - in case of a mode change
				CurrentNode = RootNode;

				OS << ", code: (" << Encode.Bits << ") '";
				//  Shift back MSB to absolute MSB
				Encode.Bits = Encode.Bits << (16 - Encode.Length);
				for (iIndex = 0; iIndex < Encode.Length; iIndex++) {
					if (Encode.Bits & 0x8000) OS << "1";
					else OS << "0";
					Encode.Bits = Encode.Bits << 1;
				}
				OS << "', Length: " << Encode.Length << " bits";
			}
			else OS << "Zero: 0x" << Node.getZero() << ", " << "One: 0x" << Node.getOne();
			OS << "." << std::endl;

			//
			//  Indented explosion
			//
			//  Recurse into the Zero node then into the One node.
			//  Percolate a stop signal.
			//
			if (Node.getZero() != nullptr) {
				if (documentNode(OS, *Node.getZero(), Level + 1)) return true;
			}

			if (Node.getOne() != nullptr) {
				if (documentNode(OS, *Node.getOne(), Level + 1)) return true;
			}

			//  Return to caller (next level up can continue with the explosion)
			return false;
		}

		//  buildELUT
		//
		//  Populates the array of Leaf Node pointers for each symbol in the table
		//
		//	PARAMETERS:
		//
		//	RETURNS:
		//
		//  NOTES:
		//

		void buildELUT() {
			int		iIndex;

			//  Clear the table
			for (iIndex = 0; iIndex < 256; iIndex++) ELUT[iIndex] = nullptr;

			//  Perform an indented explosion to poulate the lookup table
			setLookupEntry(*RootNode, 0);
			ELUTBuilt = true;

			//  Return to caller
			return;
		}

		//  setLookupEntry
		//
		//  Recursive function to populate the encoding loookup table
		//
		//	PARAMETERS:
		//
		//		HuffmanNode&		-		Reference to the node to be set in the table (if it is a leaf)
		//		int					-		Current level of the exlosion
		//
		//	RETURNS:
		//
		//		bool				-		if true signals to the parent to terminate the explosion, false continue
		//
		//  NOTES:
		//

		bool setLookupEntry(HuffmanNode& Node, int Level) {
			// If this is a leaf node then populate the entry in the lookup table
			if (Node.isLeaf()) {
				ELUT[Node.getSymbol()] = &Node;
				return false;
			}

			//  Initiate stop signal if malformed (looping) structure
			if (Level > 256) return true;

			//  Recurse into the zero branch then the one branch
			if (Node.getZero() != NULL) {
				if (setLookupEntry(*Node.getZero(), Level + 1)) return true;
			}

			if (Node.getOne() != NULL) {
				if (setLookupEntry(*Node.getOne(), Level + 1)) return true;
			}

			//  Return and continue
			return false;
		}

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Nested Classes	                                                                                            *
		//*                                                                                                                 *
		//*******************************************************************************************************************
	public:

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   HuffmanNode Class																								*
		//*                                                                                                                 *
		//*   The HuffmanNode class represents a single node in the HuffmanTree. It may be a branch or leaf node.			*
		//*                                                                                                                 *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		class HuffmanNode {
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
			//  Constructs a leaf node with the Symbol passed.
			//
			//	PARAMETERS:
			//
			//		HuffmanNode*	-		Pointer to the parent node for this node (NULL for the root node)
			//		BYTE			-		Decoded symbol for the leaf node
			//
			//	RETURNS:
			//
			//  NOTES:
			//

			HuffmanNode(bool MakeLeaf, HuffmanNode* NewParent, BYTE NewSymbol) {
				//  Setup the node as a leaf for the designated symbol
				Leaf = MakeLeaf;
				Parent = NewParent;
				Zero = nullptr;
				One = nullptr;
				Symbol = NewSymbol;
				return;
			}

			//*******************************************************************************************************************
			//*                                                                                                                 *
			//*   Destructor                                                                                                    *
			//*                                                                                                                 *
			//*******************************************************************************************************************

			~HuffmanNode() {
				//  Destroy the child nodes
				if (Zero != nullptr) delete Zero;
				if (One != nullptr) delete One;
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

			//  Accessors
			HuffmanNode*	getParent() { return Parent; }
			HuffmanNode*	getZero() { return Zero; }
			HuffmanNode*	getOne() { return One; }
			BYTE			getSymbol() const { return Symbol; }
			void			setParent(HuffmanNode* NewParent) { Parent = NewParent; return; }
			void			setZero(HuffmanNode* NewZero) { Zero = NewZero; return; }
			void			setOne(HuffmanNode* NewOne) { One = NewOne; return; }
			void			setSymbol(BYTE NewSymbol) { Symbol = NewSymbol; return; }
			bool			isRoot() { if (Parent == NULL) return true; return false; }
			bool			isLeaf() const { return Leaf; }

		private:

			//*******************************************************************************************************************
			//*																													*
			//*  Private Members																								*
			//*																													*
			//*******************************************************************************************************************

			//  Node Type
			bool					Leaf;																// true ==> Leaf, false ==> Branch

			//  Next nodes Above/below in the tree
			HuffmanNode*			Parent;																//  Parent node
			HuffmanNode*			Zero;																//  Node for a zero bit
			HuffmanNode*			One;																//  Node for a one bit

			//  Symbol for a leaf node
			BYTE					Symbol;																//  Symbol value

			//*******************************************************************************************************************
			//*                                                                                                                 *
			//*   Private Functions                                                                                             *
			//*                                                                                                                 *
			//*******************************************************************************************************************

		};
	};

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   collecter Class																								*
		//*                                                                                                                 *
		//*   The collecter functions like an iterator providing a means of capturing the unencoded data stream.			*
		//*   The passed symbols are huffman encoded and then added to the encoded bit stream.								*
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
			//		Huffman&		-		Reference to the parent Huffman CODEC
			//		ByteStream&		-		Reference to the Output Stream
			//
			//  RETURNS
			//
			//  NOTES
			//
			//

			Collecter(Huffman& Parent, ByteStream& bsOut)
				: CODEC(Parent), BStream(bsOut, true) {


				//  Return to caller
				return;
			}

			//*******************************************************************************************************************
			//*                                                                                                                 *
			//*   Destructor                                                                                                    *
			//*                                                                                                                 *
			//*******************************************************************************************************************

			~Collecter() {

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

			//  setTree
			//
			//  Sets the huffman tree to use for the compression
			//
			//  PARAMETERS
			//
			//		NuffmanTree*				-		Pointer to the Huffman Tree to use
			//
			//  RETURNS
			//
			//  NOTES
			//
			//

			void setTree(HuffmanTree* pNewTree) { CODEC.setTree(pNewTree); return; }

			//  next
			//
			//  Submits the next symbol from the uncompressed stream
			//
			//  PARAMETERS
			//
			//		BYTE					-		Next symbol to be encoded and emited
			//
			//  RETURNS
			//
			//  NOTES
			//
			//

			void next(BYTE NextSymbol) {
				EncodedUnit			EU = {};														//  Encoding result

				//  Encode the symbol
				EU = CODEC.CurrentTree->encode(NextSymbol);

				//  Output the encoded bits
				BStream.next(EU.Bits, EU.Length);

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

				//  Flush the Bit Stream
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

			Huffman&			CODEC;																	//  Parent CODEC
			MSBitStream			BStream;																//  Output Bit Stream

			//*******************************************************************************************************************
			//*                                                                                                                 *
			//*   Private Functions                                                                                             *
			//*                                                                                                                 *
			//*******************************************************************************************************************

		};

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Emitter Class																									*
		//*                                                                                                                 *
		//*   The Emitter functions like an iterator providing a means of emitting the decompressed data stream.			*
		//*   Symbols are decoded from the input encoded stream, decoded and returned to the caller.						*
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
			//		Huffman&		-		Reference to the parent Huffman CODEC obect
			//		ByteStream&		-		Reference to the Input ByteStream
			//
			//  RETURNS
			//
			//  NOTES
			//
			//

			Emitter(Huffman& Parent, ByteStream& bsIn)
				: CODEC(Parent), BStream(bsIn, false) {

				//  Return to caller
				return;
			}

			//*******************************************************************************************************************
			//*                                                                                                                 *
			//*   Destructor                                                                                                    *
			//*                                                                                                                 *
			//*******************************************************************************************************************

			~Emitter() {

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

			//  eos
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

			bool eos() {
				return BStream.eos();
			}

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
				if (BStream.eos()) return false;
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
			//		BYTE			-		Next decoded symbol from the stream
			//
			//  NOTES
			//

			BYTE next() {

				while (!BStream.eos()) {
					bool NextBit = false;
					if (BStream.next(1) == 1) NextBit = true;
					if (CODEC.CurrentTree->decode(NextBit)) return CODEC.CurrentTree->getDecode();				
				}

				return 0;
			}

			//  setTree
			//
			//  Sets the HuffmanTree object that is to be used for encoding/decoding
			//
			//  PARAMETERS
			//
			//		HuffmanTree*		-		Pointer to the HuffmanTree to use for the following operations
			//
			//  RETURNS
			//
			//  NOTES
			//
			//

			void setTree(HuffmanTree* NewTree) {
				//  Set the tree in the CODEC
				CODEC.setTree(NewTree);

				//  Return to caller
				return;
			}

		private:

			//*******************************************************************************************************************
			//*																													*
			//*  Private Members																								*
			//*																													*
			//*******************************************************************************************************************

			Huffman&			CODEC;																	//  Parent CODEC
			MSBitStream			BStream;																//  Input Bit Stream

		};

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   JPEGCollecter Class																							*
		//*                                                                                                                 *
		//*   The Collecter functions like an iterator providing a means of capturing the unencoded data stream.			*
		//*   A JPEG encoded stream is only partially encoded (only the category field), this special collector				*
		//*   will encode the category and emit it to the stream and pass the sign and magnitude fields unencoded			*
		//*   into the output stream.																						*
		//*                                                                                                                 *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		class JPEGCollecter {
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
			//		Huffman&		-		Reference to the parent Huffman CODEC
			//		ByteStream&		-		Reference to the Output Stream
			//
			//  RETURNS
			//
			//  NOTES
			//
			//

			JPEGCollecter(Huffman& Parent, ByteStream& bsOut)
				: CODEC(Parent), BStream(bsOut, true), ACCount(0) {

				memset(&Current, 0, sizeof(JCEU));

				//  Return to caller
				return;
			}

			//*******************************************************************************************************************
			//*                                                                                                                 *
			//*   Destructor                                                                                                    *
			//*                                                                                                                 *
			//*******************************************************************************************************************

			~JPEGCollecter() {

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

			//  setTree
			//
			//  Sets the huffman tree to use for the compression
			//
			//  PARAMETERS
			//
			//		NuffmanTree*				-		Pointer to the Huffman Tree to use
			//
			//  RETURNS
			//
			//  NOTES
			//
			//

			void setTree(HuffmanTree* pNewTree) { CODEC.setTree(pNewTree); return; }

			//  next
			//
			//  Submits the next symbol from the uncompressed stream
			//
			//  PARAMETERS
			//
			//		uint16_t				-		Next coefficient value to be encoded
			//		bool					-		Signal that the value is a DC value
			//
			//  RETURNS
			//
			//  NOTES
			//
			//

			void next(int16_t Coeff, bool IsDC) {
				if (IsDC) {

					//  Encode the current Coefficient as a DC
					encodeCoefficient(Coeff, true);

					//  Write the DC Coefficient
					writeCurrentUnit(true);

					//  Reset the ACCount
					ACCount = 0;

					//  Return to caller
					return;
				}

				//
				//  Coefficient is an AC value - accumulate leading zero values or encode and write a non-zero value
				//

				ACCount++;

				if (Coeff == 0) {
					Current.Zeroes++;

					//  If the AC Count has reached 63 then force a write of the current unit
					if (ACCount == 63) writeCurrentUnit(false);

					//  Return to caller
					return;
				}

				//  Encode the current AC Coefficient and any leading zeroes
				encodeCoefficient(Coeff, false);
				writeCurrentUnit(false);

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

				//  Flush the stream
				BStream.flush();

				//  Return to caller
				return;
			}

		private:

			//*******************************************************************************************************************
			//*																													*
			//*  Private Members																								*
			//*																													*
			//*******************************************************************************************************************

			Huffman&			CODEC;																	//  Parent CODEC
			MSBitStream			BStream;																//  Output Bit Stream

			//  State controls
	//  State controls
			JCEU				Current;																//  Current JPEG Encoding Unit
			size_t				ACCount;																//  Count of ACs 

			//*******************************************************************************************************************
			//*                                                                                                                 *
			//*   Private Functions                                                                                             *
			//*                                                                                                                 *
			//*******************************************************************************************************************

			//  writeCurrentUnit
			//
			//  Writes the current encoding unit to the encoded bit stream, if there is content
			//
			//  PARAMETERS
			//
			//		bool			-		True if the current unit is a DC value
			//
			//  RETURNS
			//
			//  NOTES
			//
			//

			void		writeCurrentUnit(bool IsDC) {
				JCEU		Pad16 = { 15, 0, 0, 0, 0 };
				EncodedUnit	EncodedCat = {};											//  Huffman encoded category

				//  Detect the condition where we are signalling End-Of-Unit (all remaining values are zero)
				if (!IsDC) {

					//  Detect nothing to write
					if (Current.Zeroes == 0 && Current.Category == 0 && Current.Magnitude == 0) {
						memset(&Current, 0, sizeof(JCEU));
						//  Return to caller
						return;
					}

					if (Current.Zeroes > 0 && (Current.Category == 0 && Current.Magnitude == 0)) {
						Current.Zeroes = 0;
					}
					else {
						//  Emit units of 16 zeroes at a time
						while (Current.Zeroes > 15) {
							EncodedCat = CODEC.CurrentTree->encode((Current.Zeroes << 4) + Current.Category);
							BStream.next(EncodedCat.Bits, EncodedCat.Length);
							if (Current.Category > 0) BStream.next(Current.Sign, 1);
							if (Current.Category > 1) BStream.next(Current.Magnitude, Current.Bits);
							Current.Zeroes = Current.Zeroes - 16;
						}
					}
				}

				//  Write to the Connector
				EncodedCat = CODEC.CurrentTree->encode((Current.Zeroes << 4) +  Current.Category);
				BStream.next(EncodedCat.Bits, EncodedCat.Length);
				if (Current.Category > 0) BStream.next(Current.Sign, 1);
				if (Current.Category > 1) BStream.next(Current.Magnitude, Current.Bits);

				//  Clear the current encoded unit
				memset(&Current, 0, sizeof(JCEU));

				//  Return to caller
				return;
			}

			//  encodeCoefficient
			//
			//  Encodes the coefficient and any preceding zeroes into the current unit
			//
			//  PARAMETERS
			//
			//		int16_t				-		Coefficient value
			//		bool				-		True if the coefficient is a DC value
			//
			//  RETURNS
			//
			//  NOTES
			//
			//

			void		encodeCoefficient(int16_t Coeff, bool IsDc) {
				(void)IsDc;
				uint32_t		Input = 0;													//  Input value
				uint32_t		Shifter = 0;												//  Shifter
				int				BOS = 0;													//  Bits of significance
				uint32_t		Mask = 0;													//  Mask for magnitude

				//  Record the sign
				if (Coeff < 0) Current.Sign = 0;
				else Current.Sign = 1;

				//  Capture the input value (always as a positive integer)
				if (Current.Sign == 1) Input = Coeff;
				else Input = (Coeff * -1);

				//  Determine the category
				Shifter = Input;
				for (int SX = 1; SX < 16; SX++) {
					if (Shifter & 1) BOS = SX;
					Shifter = Shifter >> 1;
				}

				//  Construct the mask for the magnitude
				for (int SX = 0; SX < (BOS - 1); SX++) {
					Mask = Mask << 1;
					Mask = (Mask | 1);
				}

				//  If number is negative invert the input
				if (Current.Sign == 0) Input = ~Input;

				//  Output the magnitude
				if (BOS == 0) Current.Bits = 0;
				else Current.Bits = BYTE(BOS - 1);
				Current.Category = BYTE(BOS);
				Current.Magnitude = Input & Mask;

				//  Return to caller
				return;
			}

		};

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   JPEGEmitter Class																								*
		//*                                                                                                                 *
		//*   The Emitter functions like an iterator providing a means of emitting the decompressed data stream.			*
		//*	  A JPEG stream is only partially huffman encode (only the category field). This special collecter will			*
		//*   decode the category from the encoded stream and retrieve the sign and magnitide fields natively from			*
		//*	  the input stream.																								*
		//*                                                                                                                 *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		class JPEGEmitter {
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
			//		Huffman&		-		Reference to the parent Huffman CODEC obect
			//		ByteStream&		-		Reference to the Input ByteStream
			//
			//  RETURNS
			//
			//  NOTES
			//
			//

			JPEGEmitter(Huffman& Parent, ByteStream& bsIn)
				: CODEC(Parent), BStream(bsIn, false) {

				//  Clear members to default state
				memset(&Current, 0, sizeof(JCEU));

				//  Return to caller
				return;
			}

			//*******************************************************************************************************************
			//*                                                                                                                 *
			//*   Destructor                                                                                                    *
			//*                                                                                                                 *
			//*******************************************************************************************************************

			~JPEGEmitter() {

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

			//  eos
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

			bool eos() {
				if (Current.Zeroes >= 0 && Current.Zeroes != 255) return false;
				return BStream.eos();
			}

			//  hasNext
			//
			//  Detects if the Emitter has more data available or has reached the end
			//
			//  PARAMETERS
			//
			//		bool				-		true if a the next expected coefficient is a DC coefficient, otherwise false
			//
			//  RETURNS
			//
			//		bool				-		true if a symbol is available, false if the end-of-stream has been reached
			//
			//  NOTES
			//

			bool hasNext(bool ExpectDC) {
				if (ExpectDC) {
					//  Expecting a DC value - this resets the intermediate values 
					memset(&Current, 0, sizeof(JCEU));
				}
				else {
					//  Expecting an AC value - if the current JCEU is not exhausted continue using that
					if (Current.Zeroes >= 0 && Current.Zeroes != 255) return true;
					memset(&Current, 0, sizeof(JCEU));
				}

				if (!BStream.eos()) {
					readCurrentUnit(ExpectDC);
					return true;
				}

				return false;
			}

			//  next
			//
			//  Returns the next symbol from the decompressed stream
			//
			//  PARAMETERS
			//
			//		bool				-		true if a the next expected coefficient is a DC coefficient, otherwise false
			//
			//  RETURNS
			//
			//		uint16_t			-		Next DC or AC Coefficient value
			//
			//  NOTES
			//

			int16_t next(bool ExpectDC) {
				int16_t		RetVal = 0;																	//  Return value

				if (ExpectDC) {
					//  Expecting a DC value - this is computed from the last category, sign and magnitude values just read
					RetVal = decodeCoefficient();
					Current.Zeroes = 255;
					return RetVal;
				}
				else {
					//  Expecting an AC value - this can be deferred or computed from the last category, sign and magnitude values just read
					if (Current.Zeroes > 0) {
						Current.Zeroes--;
						return 0;
					}
					else {
						//  Return the coefficient value
						RetVal = decodeCoefficient();
						Current.Zeroes = 255;
						return RetVal;
					}
				}
			}

			//  setTree
			//
			//  Sets the HuffmanTree object that is to be used for encoding/decoding
			//
			//  PARAMETERS
			//
			//		HuffmanTree*		-		Pointer to the HuffmanTree to use for the following operations
			//
			//  RETURNS
			//
			//  NOTES
			//
			//

			void setTree(HuffmanTree* NewTree) {
				//  Set the tree in the CODEC
				CODEC.setTree(NewTree);

				//  Return to caller
				return;
			}

		private:

			//*******************************************************************************************************************
			//*																													*
			//*  Private Members																								*
			//*																													*
			//*******************************************************************************************************************

			Huffman&				CODEC;																	//  Parent CODEC
			MSBitStream				BStream;																//  Input Bit Stream

			//  Current JCEU
			JCEU					Current;

			//*******************************************************************************************************************
			//*                                                                                                                 *
			//*   Private Functions                                                                                             *
			//*                                                                                                                 *
			//*******************************************************************************************************************

			//  readCurrentUnit
			//
			//  Reads the current encoding unit from the encoded bit stream
			//
			//  PARAMETERS
			//
			//		bool			-		True if the current unit is a DC value
			//
			//  RETURNS
			//
			//  NOTES
			//
			//

			void		readCurrentUnit(bool ExpectDC) {

				if (!BStream.eos()) {		
					//  Clear current position in the Huffman tree
					CODEC.CurrentTree->setCurrentNode(nullptr);
					memset(&Current, 0, sizeof(JCEU));

					while (!BStream.eos()) {
						bool NextBit = false;
						if (BStream.next(1) == 1) NextBit = true;
						if (CODEC.CurrentTree->decode(NextBit)) {
							//  Store the category
							Current.Category = CODEC.CurrentTree->getDecode();
					
							//  Read the sign and magnitude
							if (!ExpectDC) {
								Current.Zeroes = Current.Category / 16;
								Current.Category = Current.Category % 16;
							}
							if (Current.Category > 0) Current.Bits = Current.Category - 1;
							if (Current.Category > 0) Current.Sign = BYTE(BStream.next(1));
							if (Current.Category > 1) Current.Magnitude = BStream.next(Current.Bits);
							break;
						}
					}
				}

				//  If an End-Of-Unit was read set the zero count to 63
				if (Current.Zeroes == 0 && Current.Category == 0 && Current.Magnitude == 0) Current.Zeroes = 63;

				//  Return to caller
				return;
			}

			//  decodeCoefficient
			//
			//  Decodes the value in the current JPEG Encoding Unit to a Coefficient value
			//
			//  PARAMETERS
			//
			//  RETURNS
			//
			//		int16_t			-		Decoded coefficient value
			//
			//  NOTES
			//
			//

			int16_t		decodeCoefficient() const {
				int16_t			RetVal = 0;															//  Return value

				//  If the category is zero - return zero
				if (Current.Category == 0) return 0;

				//  Compute the coefficient value
				if (Current.Sign == 1) RetVal = uint16_t((1 << (Current.Category - 1)) + Current.Magnitude);
				else RetVal = uint16_t((Current.Magnitude ^ ((1 << Current.Category) - 1)) * -1);

				//  Return the computed value
				return RetVal;
			}

		};

	};

}
