#pragma	once
//
//  Suite:  xymorg - Image Processing Support
//
//  Common type definitions
//

//  Base includes
#include	"../types.h"																//  xymorg types
#include	"../consts.h"																//  xymorg constants

//
//  All components are defined within the xymorg namespace
//
namespace xymorg {

	//
	//  Pseudo intrinsic types
	//

	typedef			size_t					FONTID;										//  Font ID (Handle)
	typedef			std::mt19937			PRNG;										//  Type of Pseudo Random Number Generator to use

	//
	//  Convenience types
	//


	//
	//  Macros
	//


	//
	//  GLOBAL Definitions for complex types
	//

	//
	//  Colour Spaces
	//

	//
	//  COLOURINDEX	-	256 colour index
	//

	typedef	uint8_t	COLOURINDEX;

	//
	//  YCbCr	-	8 bit per channel, 3 channels, (24 bits wide pixel)
	//

	typedef struct YCbCr {
		uint8_t			Y;			//  Luminance channel
		uint8_t			Cb;			//  Chrominance (Blue) channel
		uint8_t			Cr;			//  Chrominance (Red) channel

		//  Comparators
		bool operator == (const YCbCr& rhs) {
			if (Y != rhs.Y) return false;
			if (Cb != rhs.Cb) return false;
			if (Cr != rhs.Cr) return false;
			return true;
		}

		bool operator != (const YCbCr rhs) {
			if (Y != rhs.Y) return true;
			if (Cb != rhs.Cb) return true;
			if (Cr != rhs.Cr) return true;
			return false;
		}

		//  Documentors
		void document(std::ostream& OS) {
			OS << "[Y: " << int(Y) << ",Cb: " << int(Cb) << ",Cr: " << int(Cr) << "]";
			return;
		}

		//  Const Comparators
		bool operator == (const YCbCr& rhs) const {
			if (Y != rhs.Y) return false;
			if (Cb != rhs.Cb) return false;
			if (Cr != rhs.Cr) return false;
			return true;
		}

		bool operator != (const YCbCr rhs) const {
			if (Y != rhs.Y) return true;
			if (Cb != rhs.Cb) return true;
			if (Cr != rhs.Cr) return true;
			return false;
		}

		//  Const Documentors
		void document(std::ostream& OS) const {
			OS << "[Y: " << int(Y) << ",Cb: " << int(Cb) << ",Cr: " << int(Cr) << "]";
			return;
		}
	} YCbCr;

	//  Probability biassed three colour selection
	typedef struct TriColour {
		RGB				C1;				//  Colour 1
		RGB				C2;				//  Colour 2
		RGB				C3;				//  Colour 3
		size_t			PPC1;			//  Percentage Probability of Colour 1
		size_t			PPC2;			//  Percentage Probability of Colour 2
	} TriColour;

	//
	//  Space and dimension structures
	//

	//
	//  BoundingBox  -		Defines the spatial bounds of a box within a two dimesional image space
	//

	typedef struct BoundingBox {
		size_t			Top;			//  Top row of the box
		size_t			Left;			//  Leftmost column of the box
		size_t			Bottom;			//	Bottom row of the box
		size_t			Right;			//  Rightmost column of the box
	} BoundingBox;

	//
	//  SizeVector  -		Defines the changes to the size of an image
	//

	typedef struct SizeVector {
		int				Top;			//  Delta above the top row of the image
		int				Left;			//  Delta before the leftmost column of the image
		int				Bottom;			//	Delta below the top row of the image
		int				Right;			//  Delta after the rightmost column of the image
	} SizeVector;

	//
	//  LineSegment -		Defines a straight line from an origin point to a target point and a line width.
	//						The width is always accomodated on the Right & Below the origin-target line.
	//

	typedef struct LineSegment {
		size_t			OriginR;		//  Row index of the origin point
		size_t			OriginC;		//  Column index of the origin point
		size_t			TargetR;		//  Row index of the target point
		size_t			TargetC;		//  Column index of the target point
		size_t			Width;			//  Width of the line segment in pixels
	} LineSegment;

	//
	//  Circle -			Defines a circle with a given origin and radius.
	//						The width is always accomodated inside the circular boundary
	//

	typedef struct Circle {
		size_t			OriginR;		//  Row index of the origin point
		size_t			OriginC;		//  Column index of the origin point
		size_t			Radius;			//  Radius of the circle
		size_t			Width;			//  Width of the circle in pixels
	} Circle;

	//
	//  On-Disk Image Map
	//

	typedef struct ODIBlock {
		uint8_t			BlockType;		//  Type of block
		BYTE*			Block;			//  Pointer to the block in memory
		size_t			BlockSize;		//  Size of the block
	} ODIBlock;

	typedef struct ODIMap {
		size_t			NumBlocks;		//  Count of blocks
		size_t			NBA;			//  Number of blocks allocated in the block descriptor array
		BYTE*			Image;			//  Pointer to the in-memory image	
		size_t			ImageSize;		//  Size of the in-memory image
		ODIBlock*		Blocks;			//  Array of block descriptors
	} ODIMap;

}

