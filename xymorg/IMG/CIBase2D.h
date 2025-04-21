#pragma once
//
//*******************************************************************************************************************
//*																													*
//*   File:       CIBase2D.h																						*
//*   Suite:      xymorg Image Processing - primitives																*
//*   Version:    1.0.0	  Build:  01																				*
//*   Author:     Ian Tree/HMNL																						*
//*																													*
//*   Copyright 2017 - 2020 Ian J. Tree																				*
//*******************************************************************************************************************
//*	CIBase2D.h																										*
//*																													*
//*	This header file contains the Class definition and implementation for the CIBase2D abstract base class.			*
//* The class provides a base for a compound iterator that impliments common image processing iterations over a     *
//* two dimensional image array.																					*
//*	The base class maintains current position in the 2D array and computation of the next and previous				*
//*	positions in the array. Extending classes handle the referencing of individual content of a position.			*
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
#include	"types.h"																				//  Image processing types

namespace xymorg {


	//*******************************************************************************************************************
	//*                                                                                                                 *
	//*   Local Constant Definitions                                                                                    *
	//*                                                                                                                 *
	//*******************************************************************************************************************

	//  Compund Iterator Characteristics
	//  Iterator Styles
#define		CIT_STYLE_NONE			0x00000000										//  No style (not a well formed iterator)
#define		CIT_STYLE_LINEAR		0x00000100										//  Linear style
#define		CIT_STYLE_SCANROW		0x00000200										//  Row scanning iterator
#define		CIT_STYLE_SCANCOL		0x00000400										//  Column scanning iterator
#define		CIT_STYLE_ORBITAL		0x00000800										//  Orbital scanning iterator
#define		CIT_STYLE_PRIORITY		0x00001000										//  Linear style priority iterator
#define		CIT_STYLE_MOOREHOOD		0x00002000										//  Moore style neighbourhood iterator
#define		CIT_STYLE_VNHOOD		0x00004000										//  Von Neumann style neighbourhood iterator
#define		CIT_STYLE_MOOREHOOD2	0x00008000										//  Moore style neighbourhood iterator (Depth = 2)
#define		CIT_STYLE_MCU			0x00010000										//  sequential iterator in MCU sample/desample order (2x2) DU
#define		CIT_STYLE_MCU12			0x00020000										//  sequential iterator in MCU sample/desample order (1x2) DU
#define		CIT_STYLE_MCU21			0x00040000										//  sequential iterator in MCU sample/desample order (2x1) DU
#define		CIT_STYLE_MCU11			0x00080000										//  sequential iterator in MCU sample/desample order (1x1) du

	//  Modifiers
#define		CIT_DIRECTION_REVERSE	0x00000001										//  Direction is reversed
#define		CIT_SUBITERATOR			0x00000002										//  Hierarchic sub-iterator

	//  Masks
#define		CIT_STYLE				0xFFFFFF00										//  Iterator style bits

	//*******************************************************************************************************************
	//*                                                                                                                 *
	//*   Structures                                                                                                    *
	//*                                                                                                                 *
	//*******************************************************************************************************************

	//*******************************************************************************************************************
	//*                                                                                                                 *
	//*   OrbitDescriptor Structure                                                                                     *
	//*                                                                                                                 *
	//*   Describes a single orbit in an orbital iterator.                                                              *
	//*                                                                                                                 *
	//*******************************************************************************************************************

	typedef struct OrbitDescriptor {
		BoundingBox			Extents;																//  Extent description of the orbit
		size_t				Pixels;																	//  Number of pixels in the orbit
		size_t				Orbits;																	//  Number of orbits
	} OrbitDescriptor;

	//*******************************************************************************************************************
	//*                                                                                                                 *
	//*   Neighbourhood Structure																						*
	//*                                                                                                                 *
	//*   Holds the properties describing the neighbourhood of a particular cell.                                       *
	//*                                                                                                                 *
	//*******************************************************************************************************************

	typedef struct Neighbourhood {
		size_t				TargetR;																//  Row of the target cell
		size_t				TargetC;																//  Column of the target cell
		size_t				Last;																	//  Index of the last cell in the neighbourhood
		size_t				SwitchAfter;															//  Index of the cell to switch from inner to outer orbit
		size_t				SwitchTo;																//  Index of the cell in the outer orbit to switch to
	} Neighbourhood;

	//*******************************************************************************************************************
	//*                                                                                                                 *
	//*   MCUSample Structure																							*
	//*                                                                                                                 *
	//*   Holds the properties describing the position of an MCU Sample Pixel.                                          *
	//*   MCU is the Minimum Coding Unit defined for use in JPEG image processing.										*
	//*                                                                                                                 *
	//*******************************************************************************************************************

	typedef struct MCUSample {
		size_t				MCURow;																	//  MCU row position
		size_t				MCUColumn;																//  MCU column position
		size_t				SRow;																	//  Sample (within MCU) row position
		size_t				SColumn;																//  Sample (within MCU) column position
	} MCUSample;

	//*******************************************************************************************************************
	//*                                                                                                                 *
	//*   CIBase2D Class																								*
	//*                                                                                                                 *
	//*   Classes that extend this class implement two dimensional compund iterators.									*
	//*                                                                                                                 *
	//*******************************************************************************************************************

	class CIBase2D {
	public:

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Public Constants                                                                                              *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  Qualified Iterator Styles
		static const int STYLE_LINEAR_FWD = CIT_STYLE_LINEAR;											//  Linear forward direction
		static const int STYLE_LINEAR_BKWD = CIT_STYLE_LINEAR | CIT_DIRECTION_REVERSE;					//  Linear backward direction
		static const int STYLE_SCANROW_TB = CIT_STYLE_SCANROW;											//  Row scanner (Top to Bottom)
		static const int STYLE_SCANROW_BT = CIT_STYLE_SCANROW | CIT_DIRECTION_REVERSE;					//  Row scanner (Bottom to Top)
		static const int STYLE_SCANCOL_LR = CIT_STYLE_SCANCOL;											//  Column scanner (Left to Right)
		static const int STYLE_SCANCOL_RL = CIT_STYLE_SCANCOL | CIT_DIRECTION_REVERSE;					//  Column scanner (Right to Left)
		static const int STYLE_SUBROW_TB = CIT_STYLE_SCANROW | CIT_SUBITERATOR;							//  Sub-iterator Row scanner (Top to Bottom)
		static const int STYLE_SUBROW_BT = CIT_STYLE_SCANROW | CIT_SUBITERATOR | CIT_DIRECTION_REVERSE; //  Sub-iterator Row scanner (Bottom to Top)
		static const int STYLE_SUBCOL_LR = CIT_STYLE_SCANCOL | CIT_SUBITERATOR;							//  Sub-iterator Column scanner (Left to Right)
		static const int STYLE_SUBCOL_RL = CIT_STYLE_SCANCOL | CIT_SUBITERATOR | CIT_DIRECTION_REVERSE;	//  Sub-iterator Column scanner (Right to Left)
		static const int STYLE_ORBIT_IN = CIT_STYLE_ORBITAL;											//  Orbital iterator outer to inner
		static const int STYLE_ORBIT_OUT = CIT_STYLE_ORBITAL | CIT_DIRECTION_REVERSE;					//  Orbital iterator inner to outer
		static const int STYLE_SUBORBIT_CLOCK = CIT_STYLE_ORBITAL | CIT_SUBITERATOR;					//  Orbital sub-iterator clockwise
		static const int STYLE_SUBORBIT_ANTICLOCK = CIT_STYLE_ORBITAL | CIT_SUBITERATOR | CIT_DIRECTION_REVERSE;	//  Orbital sub-iterator anticlockwise
		static const int STYLE_NEIGHBOURHOOD_MOORE = CIT_STYLE_MOOREHOOD | CIT_SUBITERATOR;				//  Neighbourhood (Moore)
		static const int STYLE_NEIGHBOURHOOD_VN = CIT_STYLE_VNHOOD | CIT_SUBITERATOR;					//  Neighbourhood (von Neumann)
		static const int STYLE_NEIGHBOURHOOD_MEND = CIT_STYLE_MOOREHOOD | CIT_DIRECTION_REVERSE | CIT_SUBITERATOR; //  End of moore neighbourhood style
		static const int STYLE_NEIGHBOURHOOD_VEND = CIT_STYLE_VNHOOD | CIT_DIRECTION_REVERSE | CIT_SUBITERATOR; //  End of Von Neumann neighbourhood style
		static const int STYLE_NEIGHBOURHOOD_MOORE2 = CIT_STYLE_MOOREHOOD2 | CIT_SUBITERATOR;			//  Neighbourhood (Moore Depth = 2)
		static const int STYLE_NEIGHBOURHOOD_MEND2 = CIT_STYLE_MOOREHOOD2 | CIT_DIRECTION_REVERSE | CIT_SUBITERATOR; //  End of moore (Depth = 2) neighbourhood style
		static const int STYLE_MCU_FWD = CIT_STYLE_MCU;													//  MCU sampling forward iterator (2x2 DU)
		static const int STYLE_MCU_BKWD = CIT_STYLE_MCU | CIT_DIRECTION_REVERSE;						//  MCU sampling Reverse iterator (2x2 DU)
		static const int STYLE_MCU12_FWD = CIT_STYLE_MCU12;												//  MCU sampling forward iterator (1x2 DU)
		static const int STYLE_MCU12_BKWD = CIT_STYLE_MCU12 | CIT_DIRECTION_REVERSE;					//  MCU sampling Reverse iterator (1x2 DU)
		static const int STYLE_MCU21_FWD = CIT_STYLE_MCU21;												//  MCU sampling forward iterator (2x1 DU)
		static const int STYLE_MCU21_BKWD = CIT_STYLE_MCU21 | CIT_DIRECTION_REVERSE;					//  MCU sampling Reverse iterator (2x1 DU)
		static const int STYLE_MCU11_FWD = CIT_STYLE_MCU11;												//  MCU sampling forward iterator (1x1 DU)
		static const int STYLE_MCU11_BKWD = CIT_STYLE_MCU11 | CIT_DIRECTION_REVERSE;					//  MCU sampling Reverse iterator (1x1 DU)

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Constructors                                                                                                  *
		//*                                                                                                                 *
		//*   The common constructor is protected to force instantiation as a base class.									*
		//*																													*
		//*******************************************************************************************************************

	protected:

		//  Normal Constructor
		//
		//	Constructs and initialises the base according to the selected style
		//
		//  PARAMETERS
		//
		//		CIBase2D&					-		Const reference to a parent iterator, or self if there is no parent
		//		size_t						-		Width of the base container array
		//		BoundingBox&				-		Const reference to the region to iterate
		//		SWITCHES					-		The iterator style bits
		//
		//  RETURNS
		//
		//  NOTES
		//

		CIBase2D(const CIBase2D& PIter, size_t W, const BoundingBox& Region, SWITCHES Style)
			: Bounds(Region), Parent(PIter), itStyle(Style), ContainerWidth(W)  {

			setInitialPosition();

			//  Return to caller
			return;
		}

		//  Copy Constructor
		//
		//	Constructs the base from the source iterator
		//
		//  PARAMETERS
		//
		//		CIBase2D&					-		Const reference to a source (sub) iterator
		//
		//  RETURNS
		//
		//  NOTES
		//

		CIBase2D(const CIBase2D& Src) 
			: Parent(Src.Parent) {

			//  Copy the current stateful members from the source
			itStyle = Src.itStyle;
			Index = Src.Index;
			Orbit = Src.Orbit;
			Bounds = Src.Bounds;
			ContainerWidth = Src.ContainerWidth;
			Hood = Src.Hood;
			MCUPos = Src.MCUPos;

			//  Return to caller
			return;
		}

		//  Move Constructor
		//
		//	Constructs the base from the source iterator
		//
		//  PARAMETERS
		//
		//		CIBase2D&&					-		Reference to a source (sub) iterator
		//
		//  RETURNS
		//
		//  NOTES
		//

		CIBase2D(CIBase2D&& Src) noexcept
			: Parent(Src.Parent) {

			//  Copy the current stateful members from the source
			itStyle = Src.itStyle;
			Index = Src.Index;
			Orbit = Src.Orbit;
			Bounds = Src.Bounds;
			ContainerWidth = Src.ContainerWidth;
			Hood = Src.Hood;
			MCUPos = Src.MCUPos;

			//  Return to caller
			return;
		}

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Destructor                                                                                                    *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		~CIBase2D() { return; }

	public:

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Public Members                                                                                                *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Public Functions                                                                                              *
		//*                                                                                                                 *
		//*   The following functions should be exposed as public in any CompoundIterator.									*
		//*																													*
		//*******************************************************************************************************************

		//  reset or flyback
		//
		//	Resets the initial position in the iterator, used in places where construction of the iterator is not wanted. 
		//
		//  PARAMETERS
		//
		//  RETURNS
		//
		//  NOTES
		//

		void reset() {
			setInitialPosition();
		}

		void flyback() { return reset(); }

		//  isLast
		//
		//	Determines if the iterator is positioned on the last element that it will deliver
		//
		//  PARAMETERS
		//
		//  RETURNS
		//
		//		bool					-	true if the current position is the last, otherwise false
		//
		//  NOTES
		//

		bool isLast() {
			//  The determination depends on the style of the iterator
			switch (itStyle) {
			case STYLE_LINEAR_FWD:
				if (Index == ((Bounds.Bottom * ContainerWidth) + Bounds.Right)) return true;
				break;

			case STYLE_LINEAR_BKWD:
				if (Index == ((Bounds.Top * ContainerWidth) + Bounds.Left + 1)) return true;
				break;

			case STYLE_SCANROW_TB:
			case STYLE_SUBROW_TB:
				if (Index == Bounds.Bottom) return true;
				break;

			case STYLE_SCANROW_BT:
			case STYLE_SUBROW_BT:
				if (Index == (Bounds.Top + 1)) return true;
				break;

			case STYLE_SCANCOL_LR:
			case STYLE_SUBCOL_LR:
				if (Index == Bounds.Right) return true;
				break;

			case STYLE_SCANCOL_RL:
			case STYLE_SUBCOL_RL:
				if (Index == (Bounds.Left + 1)) return true;
				break;

			case STYLE_ORBIT_IN:
				if (Index == Orbit.Orbits - 1) return true;
				break;

			case STYLE_ORBIT_OUT:
				if (Index == 1) return true;
				break;

			case STYLE_SUBORBIT_CLOCK:
				if (Index == Parent.Orbit.Pixels - 1) return true;
				break;

			case STYLE_SUBORBIT_ANTICLOCK:
				if (Index == 1) return true;
				break;

			case STYLE_NEIGHBOURHOOD_MOORE:
			case STYLE_NEIGHBOURHOOD_MOORE2:
				if (Index == Hood.Last) return true;
				break;

			case STYLE_NEIGHBOURHOOD_VN:
				if (Index == Hood.Last) return true;
				break;

			case STYLE_NEIGHBOURHOOD_MEND:
			case STYLE_NEIGHBOURHOOD_MEND2:
				if (Index == Hood.Last) return true;
				break;

			case STYLE_NEIGHBOURHOOD_VEND:
				if (Index == (Hood.Last + 2)) return true;
				break;

			case STYLE_MCU_FWD:
				if (MCUPos.SColumn == 15) {
					if (MCUPos.SRow == 15) {
						if (MCUPos.MCUColumn == (((Bounds.Right - Bounds.Left) + 1) / 16) - 1) {
							if (MCUPos.MCURow == (((Bounds.Bottom - Bounds.Top) + 1) / 16) - 1) return true;
						}
					}
				}
				break;

			case STYLE_MCU11_FWD:
				if (MCUPos.SColumn == 7) {
					if (MCUPos.SRow == 7) {
						if (MCUPos.MCUColumn == (((Bounds.Right - Bounds.Left) + 1) / 8) - 1) {
							if (MCUPos.MCURow == (((Bounds.Bottom - Bounds.Top) + 1) / 8) - 1) return true;
						}
					}
				}
				break;

			case STYLE_MCU21_FWD:
				if (MCUPos.SColumn == 15) {
					if (MCUPos.SRow == 7) {
						if (MCUPos.MCUColumn == (((Bounds.Right - Bounds.Left) + 1) / 16) - 1) {
							if (MCUPos.MCURow == (((Bounds.Bottom - Bounds.Top) + 1) / 8) - 1) return true;
						}
					}
				}
				break;

			case STYLE_MCU12_FWD:
				if (MCUPos.SColumn == 7) {
					if (MCUPos.SRow == 15) {
						if (MCUPos.MCUColumn == (((Bounds.Right - Bounds.Left) + 1) / 8) - 1) {
							if (MCUPos.MCURow == (((Bounds.Bottom - Bounds.Top) + 1) / 16) - 1) return true;
						}
					}
				}
				break;

			case STYLE_MCU_BKWD:
			case STYLE_MCU11_BKWD:
			case STYLE_MCU12_BKWD:
			case STYLE_MCU21_BKWD:
				if (MCUPos.SColumn == 0) {
					if (MCUPos.SRow == 0) {
						if (MCUPos.MCUColumn == 0) {
							if (MCUPos.MCURow == 0) return true;
						}
					}
				}
			}
			return false;
		}

		//  getIndex
		//
		//	Returns the current index position of the iterator
		//
		//  PARAMETERS
		//
		//  RETURNS
		//
		//		size_t					-	Current index position of the iterator
		//
		//  NOTES
		//
		//		Spatial map of neighbourhood index positions (external view) :-
		//
		//			08 09 10 11 12
		//			23 00 01 02 13
		//			22 07 -- 03 14
		//			21 06 05 04 15
		//			20 19 18 17 16
		//

		size_t	getIndex() {
			//  Neighbourhood Depth = 2 iterators must return the index position as a single continuum
			if (itStyle & CIT_STYLE_MOOREHOOD2) {
				if (itStyle & CIT_DIRECTION_REVERSE) {
					//  Reverse iterator
					if ((Index - 1) < 16) return (Index - 1) % 8;
					return ((Index - 1) % 16) + 8;
				}

				//  Forward iterator
				if (Index < 16) return Index % 8;
				return (Index % 16) + 8;
			}

			//  Neighbourhood iterators must return the index position modulus 8
			if (itStyle & (CIT_STYLE_MOOREHOOD | CIT_STYLE_VNHOOD)) {
				if (itStyle & CIT_DIRECTION_REVERSE) {
					if (itStyle & CIT_STYLE_VNHOOD) return (Index - 2) % 8;
					return (Index - 1) % 8;
				}
				return Index % 8;
			}

			//  Default
			if (itStyle & CIT_DIRECTION_REVERSE) return Index - 1;
			return Index;
		}

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Operator Overload Functions                                                                                   *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  Equality == operator
		//
		//	Returns true if the comperand iterator is at the same position as this iterator
		//
		//  PARAMETERS
		//
		//		CIBase2D&				-	Reference to the iterator to be compared
		//
		//  RETURNS
		//
		//		bool					-	true if same iterator at same position, otherwise false
		//
		//  NOTES
		//

		bool operator == (const CIBase2D& Comp) {

			//  Default comparison
			if (Index != Comp.Index) return false;
			return true;
		}

		//  Inequality != operator
		//
		//	Returns the inversion of the equality operator, i.e. returns true if the comperand has a different index position.
		//
		//  PARAMETERS
		//
		//		CIBase2D&				-	Reference to the iterator to be compared
		//
		//  RETURNS
		//
		//		bool					-	true if a different index position, otherwise false
		//
		//  NOTES
		//

		bool operator != (const CIBase2D& Comp) {

			if (Index != Comp.Index) return true;
			return false;
		}


	protected:
		//*******************************************************************************************************************
		//*																													*
		//*  Protected Members																								*
		//*																													*
		//*******************************************************************************************************************

		BoundingBox						Bounds;																	//  Bounds for an image region iterator

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Protected Functions                                                                                           *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  effectiveOffset
		//
		//	Returns the effective offset of the current entry at the current position of the iterator
		//
		//  PARAMETERS
		//
		//  RETURNS
		//
		//		size_t					-	Offset to the entry at the current index position of the iterator
		//
		//  NOTES
		//
		//		1.	It is the responsibility of any extending iterator to interpret the offset ans a position in the container
		//

		size_t	effectiveOffset() const {
			size_t			MyOff;																			//  Temp offset

			//  The effective offset is determined by the iterator style and the index position of the
			//  iterator and possibly the position of any parent iterators

			switch (itStyle) {
			case STYLE_LINEAR_FWD:
				return Index;
				break;

			case STYLE_LINEAR_BKWD:
				return Index - 1;
				break;

			case STYLE_SCANROW_TB:
				return Index * ContainerWidth;
				break;

			case STYLE_SCANROW_BT:
				return (Index - 1) * ContainerWidth;
				break;

			case STYLE_SCANCOL_LR:
				return Index;
				break;

			case STYLE_SCANCOL_RL:
				return Index - 1;
				break;

			case STYLE_SUBCOL_LR:
				if (Parent.itStyle == STYLE_SCANROW_BT) return ((Parent.Index - 1) * ContainerWidth) + Index;
				return (Parent.Index * ContainerWidth) + Index;
				break;

			case STYLE_SUBCOL_RL:
				if (Parent.itStyle == STYLE_SCANROW_BT) return ((Parent.Index - 1) * ContainerWidth) + (Index - 1);
				return (Parent.Index * ContainerWidth) + (Index - 1);
				break;

			case STYLE_SUBROW_TB:
				if (Parent.itStyle == STYLE_SCANCOL_RL) return (Index * ContainerWidth) + (Parent.Index - 1);
				return (Index * ContainerWidth) + Parent.Index;
				break;

			case STYLE_SUBROW_BT:
				if (Parent.itStyle == STYLE_SCANCOL_RL) return ((Index - 1) * ContainerWidth) + (Parent.Index - 1);
				return ((Index - 1) * ContainerWidth) + Parent.Index;
				break;

			case STYLE_ORBIT_IN:
			case STYLE_ORBIT_OUT:
				return (Orbit.Extents.Top * ContainerWidth) + Orbit.Extents.Left;
				break;

			case STYLE_SUBORBIT_CLOCK:
				//  The computation for the target pixel is based on the index and the orbit descriptor in the parent iterator
				if (Index < (Parent.Orbit.Extents.Right - Parent.Orbit.Extents.Left)) {
					//  Top row of the orbital
					return (Parent.Orbit.Extents.Top * ContainerWidth) + Parent.Orbit.Extents.Left + Index;
				}
				else {
					if (Index < ((Parent.Orbit.Extents.Right - Parent.Orbit.Extents.Left) + (Parent.Orbit.Extents.Bottom - Parent.Orbit.Extents.Top))) {
						//  Right hand edge of the orbital
						return ((Parent.Orbit.Extents.Top + (Index - (Parent.Orbit.Extents.Right - Parent.Orbit.Extents.Left))) * ContainerWidth) + Parent.Orbit.Extents.Right;
					}
					else {
						if (Index < ((2 * (Parent.Orbit.Extents.Right - Parent.Orbit.Extents.Left)) + (Parent.Orbit.Extents.Bottom - Parent.Orbit.Extents.Top))) {
							//  Bottom row of the orbital
							size_t OOffset = (Parent.Orbit.Extents.Bottom *ContainerWidth);
							OOffset += (Parent.Orbit.Extents.Right -
								(Index - ((Parent.Orbit.Extents.Right - Parent.Orbit.Extents.Left) + (Parent.Orbit.Extents.Bottom - Parent.Orbit.Extents.Top))));
							return OOffset;
						}
						else {
							//  Left hand edge of the orbital
							return ((Parent.Orbit.Extents.Top + (Parent.Orbit.Pixels - Index)) * ContainerWidth) + Parent.Orbit.Extents.Left;
						}
					}
				}
				break;

			case STYLE_SUBORBIT_ANTICLOCK:
				//  The computation for the target pixel is based on the index and the orbit descriptor in the parent iterator
				if ((Index - 1) < (Parent.Orbit.Extents.Right - Parent.Orbit.Extents.Left)) {
					//  Top row of the orbital
					return (Parent.Orbit.Extents.Top * ContainerWidth) + Parent.Orbit.Extents.Left + (Index - 1);
				}
				else {
					if ((Index - 1) < ((Parent.Orbit.Extents.Right - Parent.Orbit.Extents.Left) + (Parent.Orbit.Extents.Bottom - Parent.Orbit.Extents.Top))) {
						//  Right hand edge of the orbital
						return ((Parent.Orbit.Extents.Top + ((Index - 1) - (Parent.Orbit.Extents.Right - Parent.Orbit.Extents.Left))) * ContainerWidth) + Parent.Orbit.Extents.Right;
					}
					else {
						if ((Index - 1) < ((2 * (Parent.Orbit.Extents.Right - Parent.Orbit.Extents.Left)) + (Parent.Orbit.Extents.Bottom - Parent.Orbit.Extents.Top))) {
							//  Bottom row of the orbital
							size_t OOffset = (Parent.Orbit.Extents.Bottom * ContainerWidth);
							OOffset += (Parent.Orbit.Extents.Right -
								((Index - 1) - ((Parent.Orbit.Extents.Right - Parent.Orbit.Extents.Left) + (Parent.Orbit.Extents.Bottom - Parent.Orbit.Extents.Top))));
							return OOffset;
						}
						else {
							//  Left hand edge of the orbital
							return ((Parent.Orbit.Extents.Top + (Parent.Orbit.Pixels - (Index - 1))) * ContainerWidth) + Parent.Orbit.Extents.Left;
						}
					}
				}
				break;

			case STYLE_NEIGHBOURHOOD_MOORE:
			case STYLE_NEIGHBOURHOOD_MOORE2:
			case STYLE_NEIGHBOURHOOD_VN:
			case STYLE_NEIGHBOURHOOD_MEND:
			case STYLE_NEIGHBOURHOOD_MEND2:
			case STYLE_NEIGHBOURHOOD_VEND:
				//  The index position resolves to a R,C displacement from the target pixel
				size_t			AdjIndex;

				if (itStyle & CIT_DIRECTION_REVERSE) AdjIndex = Index - 1;
				else AdjIndex = Index;

				switch (AdjIndex) {
				case 0:
				case 8:
					//  -1,-1
					return ((Hood.TargetR - 1) * ContainerWidth) + (Hood.TargetC - 1);
					break;

				case 1:
				case 9:
					//  -1,0
					return ((Hood.TargetR - 1) * ContainerWidth) + Hood.TargetC;
					break;

				case 2:
				case 10:
					//  -1,+1
					return ((Hood.TargetR - 1) * ContainerWidth) + (Hood.TargetC + 1);
					break;

				case 3:
				case 11:
					//  0,+1
					return (Hood.TargetR * ContainerWidth) + (Hood.TargetC + 1);
					break;

				case 4:
				case 12:
					// +1,+1
					return ((Hood.TargetR + 1) * ContainerWidth) + (Hood.TargetC + 1);
					break;

				case 5:
				case 13:
					//  +1,0
					return ((Hood.TargetR + 1) * ContainerWidth) + Hood.TargetC;
					break;

				case 6:
				case 14:
					//  +1,-1
					return ((Hood.TargetR + 1) * ContainerWidth) + (Hood.TargetC - 1);
					break;

				case 7:
				case 15:
					//  0, -1
					return (Hood.TargetR * ContainerWidth) + (Hood.TargetC - 1);
					break;

				case 16:
				case 32:
					//  -2, -2
					return ((Hood.TargetR - 2) * ContainerWidth) + (Hood.TargetC - 2);
					break;

				case 17:
				case 33:
					//  -2, -1
					return ((Hood.TargetR - 2) * ContainerWidth) + (Hood.TargetC - 1);
					break;

				case 18:
				case 34:
					//  -2, 0
					return ((Hood.TargetR - 2) * ContainerWidth) + Hood.TargetC;
					break;

				case 19:
				case 35:
					//  -2, +1
					return ((Hood.TargetR - 2) * ContainerWidth) + (Hood.TargetC + 1);
					break;

				case 20:
				case 36:
					//  -2, +2
					return ((Hood.TargetR - 2) * ContainerWidth) + (Hood.TargetC + 2);
					break;

				case 21:
				case 37:
					//  -1, +2
					return ((Hood.TargetR - 1) * ContainerWidth) + (Hood.TargetC + 2);
					break;

				case 22:
				case 38:
					//  0, +2
					return (Hood.TargetR * ContainerWidth) + (Hood.TargetC + 2);
					break;

				case 23:
				case 39:
					//  +1, +2
					return ((Hood.TargetR + 1) * ContainerWidth) + (Hood.TargetC + 2);
					break;

				case 24:
				case 40:
					//  +2, +2
					return ((Hood.TargetR + 2) * ContainerWidth) + (Hood.TargetC + 2);
					break;

				case 25:
				case 41:
					//  +2, +1
					return ((Hood.TargetR + 2) * ContainerWidth) + (Hood.TargetC + 1);
					break;

				case 26:
				case 42:
					//  +2, 0
					return ((Hood.TargetR + 2) * ContainerWidth) + Hood.TargetC;
					break;

				case 27:
				case 43:
					//  +2, -1
					return ((Hood.TargetR + 2) * ContainerWidth) + (Hood.TargetC - 1);
					break;

				case 28:
				case 44:
					//  +2, -2
					return ((Hood.TargetR + 2) * ContainerWidth) + (Hood.TargetC - 2);
					break;

				case 29:
				case 45:
					//  +1, -2
					return ((Hood.TargetR + 1) * ContainerWidth) + (Hood.TargetC - 2);
					break;

				case 30:
				case 46:
					//  0, -2
					return (Hood.TargetR * ContainerWidth) + (Hood.TargetC - 2);
					break;

				case 31:
				case 47:
					//  -1, -2
					return ((Hood.TargetR - 1) * ContainerWidth) + (Hood.TargetC - 2);
					break;
				}

				//  Return the target cell
				return (Hood.TargetR * ContainerWidth) + Hood.TargetC;
				break;

				//  For MCU Sampling iterators the effective offset is computed from the sample position
			case STYLE_MCU_FWD:
			case STYLE_MCU11_FWD:
			case STYLE_MCU12_FWD:
			case STYLE_MCU21_FWD:
				MyOff = computeMCUIndex();
				return MyOff;
				break;

			case STYLE_MCU_BKWD:
			case STYLE_MCU11_BKWD:
			case STYLE_MCU12_BKWD:
			case STYLE_MCU21_BKWD:
				MyOff = computeMCUIndex();
				return MyOff - 1;
				break;
			}
			return Index;
		}

		//  incrementPosition
		//
		//	Increments the current position of the iterator
		//
		//  PARAMETERS
		//
		//  RETURNS
		//
		//  NOTES
		//

		void	incrementPosition() {
			//  Moore Depth = 2 iterators must check for switch from inner to outer orbit
			if (itStyle == STYLE_NEIGHBOURHOOD_MOORE2) {
				if (Index == Hood.SwitchAfter) {
					Index = Hood.SwitchTo;
					return;
				}
			}

			Index++;
			switch (itStyle) {
			case STYLE_ORBIT_IN:
			case STYLE_ORBIT_OUT:
				//  Orbital iterators must also update the orbit descriptor
				//  NOTE: Increment always moves to the next innermost orbit
				Orbit.Extents.Top++;
				Orbit.Extents.Bottom--;
				Orbit.Extents.Left++;
				Orbit.Extents.Right--;
				//  Compute the number of pixels in the orbit
				if (Orbit.Extents.Top == Orbit.Extents.Bottom || Orbit.Extents.Left == Orbit.Extents.Right) {
					//  Degenerate cases
					Orbit.Pixels = ((Orbit.Extents.Bottom - Orbit.Extents.Top) + 1) * ((Orbit.Extents.Right - Orbit.Extents.Left) + 1);
				}
				else Orbit.Pixels = (2 * (Orbit.Extents.Right - Orbit.Extents.Left)) + (2 * (Orbit.Extents.Bottom - Orbit.Extents.Top));
				break;

			case STYLE_LINEAR_FWD:
			case STYLE_LINEAR_BKWD:
				//  Check if the boundary has been reached (RHS of bounds) if so move to the LHS of the bounds on the next row
				if ((Index % ContainerWidth) == ((Bounds.Right + 1) % ContainerWidth)) {
					//  Do not bump the index if we are on the bottom row of the region
					if (Index <= (Bounds.Bottom * ContainerWidth)) {
						Index += ((ContainerWidth - (Bounds.Right + 1)) + Bounds.Left);
					}
				}
				break;

			case STYLE_NEIGHBOURHOOD_VN:
				Index++;
				break;

			case STYLE_MCU_FWD:
			case STYLE_MCU_BKWD:
				//  MCU Sampling iterators must also update the MCU Sample position
				if (MCUPos.SColumn == 15) {
					MCUPos.SColumn = 0;
					if (MCUPos.SRow == 15) {
						MCUPos.SRow = 0;
						if (MCUPos.MCUColumn == (((Bounds.Right - Bounds.Left) + 1) / 16) - 1) {
							MCUPos.MCUColumn = 0;
							MCUPos.MCURow++;
						}
						else MCUPos.MCUColumn++;
					}
					else MCUPos.SRow++;
				}
				else MCUPos.SColumn++;
				Index = computeMCUIndex();
				break;

			case STYLE_MCU11_FWD:
			case STYLE_MCU11_BKWD:
				//  MCU Sampling iterators must also update the MCU Sample position
				if (MCUPos.SColumn == 7) {
					MCUPos.SColumn = 0;
					if (MCUPos.SRow == 7) {
						MCUPos.SRow = 0;
						if (MCUPos.MCUColumn == (((Bounds.Right - Bounds.Left) + 1) / 8) - 1) {
							MCUPos.MCUColumn = 0;
							MCUPos.MCURow++;
						}
						else MCUPos.MCUColumn++;
					}
					else MCUPos.SRow++;
				}
				else MCUPos.SColumn++;
				Index = computeMCUIndex();
				break;

			case STYLE_MCU21_FWD:
			case STYLE_MCU21_BKWD:
				//  MCU Sampling iterators must also update the MCU Sample position
				if (MCUPos.SColumn == 15) {
					MCUPos.SColumn = 0;
					if (MCUPos.SRow == 7) {
						MCUPos.SRow = 0;
						if (MCUPos.MCUColumn == (((Bounds.Right - Bounds.Left) + 1) / 16) - 1) {
							MCUPos.MCUColumn = 0;
							MCUPos.MCURow++;
						}
						else MCUPos.MCUColumn++;
					}
					else MCUPos.SRow++;
				}
				else MCUPos.SColumn++;
				Index = computeMCUIndex();
				break;

			case STYLE_MCU12_FWD:
			case STYLE_MCU12_BKWD:
				//  MCU Sampling iterators must also update the MCU Sample position
				if (MCUPos.SColumn == 7) {
					MCUPos.SColumn = 0;
					if (MCUPos.SRow == 15) {
						MCUPos.SRow = 0;
						if (MCUPos.MCUColumn == (((Bounds.Right - Bounds.Left) + 1) / 8) - 1) {
							MCUPos.MCUColumn = 0;
							MCUPos.MCURow++;
						}
						else MCUPos.MCUColumn++;
					}
					else MCUPos.SRow++;
				}
				else MCUPos.SColumn++;
				Index = computeMCUIndex();
				break;
			}

			//  Return to caller
			return;
		}

		//  decrementPosition
		//
		//	Decrements the current position of the iterator
		//
		//  PARAMETERS
		//
		//  RETURNS
		//
		//  NOTES
		//

		void	decrementPosition() {
			//  Moore Depth = 2 iterators must check for switch from inner to outer orbit
			if (itStyle == STYLE_NEIGHBOURHOOD_MEND2) {
				if (Index == Hood.SwitchAfter) {
					Index = Hood.SwitchTo;
					return;
				}
			}

			Index--;
			switch (itStyle) {
				//  Orbital iterators must also update the orbit descriptor
				//  NOTE: Decrement always moves to the next outermost orbit
			case STYLE_ORBIT_IN:
			case STYLE_ORBIT_OUT:
				Orbit.Extents.Top--;
				Orbit.Extents.Bottom++;
				Orbit.Extents.Left--;
				Orbit.Extents.Right++;
				//  Compute the number of pixels in the orbit
				if (Orbit.Extents.Top == Orbit.Extents.Bottom || Orbit.Extents.Left == Orbit.Extents.Right) {
					//  Degenerate cases
					Orbit.Pixels = ((Orbit.Extents.Bottom - Orbit.Extents.Top) + 1) * ((Orbit.Extents.Right - Orbit.Extents.Left) + 1);
				}
				else Orbit.Pixels = (2 * (Orbit.Extents.Right - Orbit.Extents.Left)) + (2 * (Orbit.Extents.Bottom - Orbit.Extents.Top));
				break;

			case STYLE_LINEAR_FWD:
			case STYLE_LINEAR_BKWD:
				//  Check if the boundary has been reached (LHS of bounds) if so move to the RHS of the bounds on the previous row
				if ((Index % ContainerWidth) == Bounds.Left) {
					//  Do not bump the index if we are on the top row
					if (Index > ((Bounds.Top * ContainerWidth) + Bounds.Left)) {
						Index -= ((ContainerWidth - (Bounds.Right + 1)) + Bounds.Left);
					}
				}
				break;

			case STYLE_NEIGHBOURHOOD_VN:
				Index--;
				break;

			case STYLE_MCU_FWD:
			case STYLE_MCU_BKWD:
				//  MCU Sampling iterators must also update the MCU Sample position
				if (MCUPos.SColumn == 0) {
					MCUPos.SColumn = 15;
					if (MCUPos.SRow == 0) {
						MCUPos.SRow = 15;
						if (MCUPos.MCUColumn == 0) {
							if (MCUPos.MCURow > 0) {
								MCUPos.MCUColumn = (((Bounds.Right - Bounds.Left) + 1) / 16) - 1;
								MCUPos.MCURow--;
							}
						}
						else MCUPos.MCUColumn--;
					}
					else MCUPos.SRow--;
				}
				else MCUPos.SColumn--;
				break;

			case STYLE_MCU11_FWD:
			case STYLE_MCU11_BKWD:
				//  MCU Sampling iterators must also update the MCU Sample position
				if (MCUPos.SColumn == 0) {
					MCUPos.SColumn = 7;
					if (MCUPos.SRow == 0) {
						MCUPos.SRow = 7;
						if (MCUPos.MCUColumn == 0) {
							if (MCUPos.MCURow > 0) {
								MCUPos.MCUColumn = (((Bounds.Right - Bounds.Left) + 1) / 8) - 1;
								MCUPos.MCURow--;
							}
						}
						else MCUPos.MCUColumn--;
					}
					else MCUPos.SRow--;
				}
				else MCUPos.SColumn--;
				break;

			case STYLE_MCU21_FWD:
			case STYLE_MCU21_BKWD:
				//  MCU Sampling iterators must also update the MCU Sample position
				if (MCUPos.SColumn == 0) {
					MCUPos.SColumn = 15;
					if (MCUPos.SRow == 0) {
						MCUPos.SRow = 7;
						if (MCUPos.MCUColumn == 0) {
							if (MCUPos.MCURow > 0) {
								MCUPos.MCUColumn = (((Bounds.Right - Bounds.Left) + 1) / 16) - 1;
								MCUPos.MCURow--;
							}
						}
						else MCUPos.MCUColumn--;
					}
					else MCUPos.SRow--;
				}
				else MCUPos.SColumn--;
				break;

			case STYLE_MCU12_FWD:
			case STYLE_MCU12_BKWD:
				//  MCU Sampling iterators must also update the MCU Sample position
				if (MCUPos.SColumn == 0) {
					MCUPos.SColumn = 7;
					if (MCUPos.SRow == 0) {
						MCUPos.SRow = 15;
						if (MCUPos.MCUColumn == 0) {
							if (MCUPos.MCURow > 0) {
								MCUPos.MCUColumn = (((Bounds.Right - Bounds.Left) + 1) / 8) - 1;
								MCUPos.MCURow--;
							}
						}
						else MCUPos.MCUColumn--;
					}
					else MCUPos.SRow--;
				}
				else MCUPos.SColumn--;
				break;

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

		const CIBase2D&					Parent;																	//  Parent (or this) iterator
		SWITCHES						itStyle;																//  Iterator style in effect
		size_t							Index;																	//  Current index position of the iterator
		OrbitDescriptor					Orbit;																	//  Orbit descriptor for current orbit
		size_t							ContainerWidth;															//  Width of the container array
		Neighbourhood					Hood;																	//  Neighbourhood descriptor
		MCUSample						MCUPos;																	//  MCU sampling position

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Private Functions                                                                                             *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  setInitialPosition
		//
		//	Initialises the position information for the (sub) iterator according to the style in use.
		//
		//  PARAMETERS
		//
		//  RETURNS
		//
		//  NOTES
		//

		void	setInitialPosition() {
			OrbitDescriptor		InnerOrbit = {};														//  Inner orbit descriptor

			//  Initialise an iterator or a sub-iterator
			if (itStyle & CIT_SUBITERATOR) {
				//  Initialisation for a sub-iterator
				//  Set the initial position according to the iterator style
				switch (itStyle) {
				case STYLE_SUBROW_TB:
					Index = Bounds.Top;
					break;

				case STYLE_SUBROW_BT:
					Index = Bounds.Bottom + 1;
					break;

				case STYLE_SUBCOL_LR:
					Index = Bounds.Left;
					break;

				case STYLE_SUBCOL_RL:
					Index = Bounds.Right + 1;
					break;

				case STYLE_SUBORBIT_CLOCK:
					Index = 0;
					break;

				case STYLE_SUBORBIT_ANTICLOCK:
					Index = Parent.Orbit.Pixels;
					break;

				case STYLE_NEIGHBOURHOOD_MOORE:
				case STYLE_NEIGHBOURHOOD_MOORE2:
				case STYLE_NEIGHBOURHOOD_VN:
				case STYLE_NEIGHBOURHOOD_MEND:
				case STYLE_NEIGHBOURHOOD_MEND2:
				case STYLE_NEIGHBOURHOOD_VEND:
					//  Derive the current position of the parent iterator to determine the midpoint of the neighbourhood
					computeNeighbourhood();
					break;
				}

				return;
			}
			else {
				//  Initialisation for an iterator
				//  Compute the initial position of the iterator based on the style
				switch (itStyle) {
				case STYLE_LINEAR_FWD:
					Index = (Bounds.Top * ContainerWidth) + Bounds.Left;
					break;

				case STYLE_LINEAR_BKWD:
					Index = (Bounds.Bottom * ContainerWidth) + Bounds.Right + 1;
					break;

				case STYLE_SCANROW_TB:
					Index = Bounds.Top;
					break;

				case STYLE_SCANROW_BT:
					Index = Bounds.Bottom + 1;
					break;

				case STYLE_SCANCOL_LR:
					Index = Bounds.Left;
					break;

				case STYLE_SCANCOL_RL:
					Index = Bounds.Right + 1;
					break;

				case STYLE_ORBIT_IN:
					//  Inwards orbit so the initial orbit is the outside edge of the image
					Orbit.Extents = Bounds;
					Index = 0;

					//  Compute the number of pixels in the orbit
					if (Orbit.Extents.Top == Orbit.Extents.Bottom || Orbit.Extents.Left == Orbit.Extents.Right) {
						//  Degenerate cases
						Orbit.Pixels = ((Orbit.Extents.Bottom - Orbit.Extents.Top) + 1) * ((Orbit.Extents.Right - Orbit.Extents.Left) + 1);
					}
					else Orbit.Pixels = (2 * (Orbit.Extents.Right - Orbit.Extents.Left)) + (2 * (Orbit.Extents.Bottom - Orbit.Extents.Top));

					//  Compute the number of orbits available to this iterator
					InnerOrbit = Orbit;
					Orbit.Orbits = 1;
					while ((InnerOrbit.Extents.Right - InnerOrbit.Extents.Left) > 1 && (InnerOrbit.Extents.Bottom - InnerOrbit.Extents.Top) > 1) {
						Orbit.Orbits++;
						InnerOrbit.Extents.Bottom--;
						InnerOrbit.Extents.Top++;
						InnerOrbit.Extents.Right--;
						InnerOrbit.Extents.Left++;
					}
					break;

				case STYLE_ORBIT_OUT:
					//  Setup the outwards orbit and contract it to find the innermost orbit extents
					Orbit.Extents = Bounds;
					Orbit.Orbits = 1;
					while ((Orbit.Extents.Right - Orbit.Extents.Left) > 1 && (Orbit.Extents.Bottom - Orbit.Extents.Top) > 1) {
						Orbit.Orbits++;
						Orbit.Extents.Bottom--;
						Orbit.Extents.Top++;
						Orbit.Extents.Right--;
						Orbit.Extents.Left++;
					}

					//  Compute the number of pixels in the orbit
					if (Orbit.Extents.Top == Orbit.Extents.Bottom || Orbit.Extents.Left == Orbit.Extents.Right) {
						//  Degenerate cases
						Orbit.Pixels = ((Orbit.Extents.Bottom - Orbit.Extents.Top) + 1) * ((Orbit.Extents.Right - Orbit.Extents.Left) + 1);
					}
					else Orbit.Pixels = (2 * (Orbit.Extents.Right - Orbit.Extents.Left)) + (2 * (Orbit.Extents.Bottom - Orbit.Extents.Top));

					//  Set the initial value of the index
					Index = Orbit.Orbits;
					break;

				case STYLE_MCU_FWD:
				case STYLE_MCU11_FWD:
				case STYLE_MCU12_FWD:
				case STYLE_MCU21_FWD:
					//  Setup the initial sample position
					MCUPos.MCURow = 0;
					MCUPos.MCUColumn = 0;
					MCUPos.SRow = 0;
					MCUPos.SColumn = 0;
					Index = 0;
					break;

				case STYLE_MCU_BKWD:
					//  Setup for the position past the end of the MCU Sample Space
					MCUPos.MCURow = ((Bounds.Bottom - Bounds.Top) / 16)  + 1;
					MCUPos.MCUColumn = 0;
					MCUPos.SRow = 0;
					MCUPos.SColumn = 0;
					Index = computeMCUIndex();
					break;

				case STYLE_MCU11_BKWD:
					//  Setup for the position past the end of the MCU Sample Space
					MCUPos.MCURow = ((Bounds.Bottom - Bounds.Top) / 8) + 1;
					MCUPos.MCUColumn = 0;
					MCUPos.SRow = 0;
					MCUPos.SColumn = 0;
					Index = computeMCUIndex();
					break;

				case STYLE_MCU21_BKWD:
					//  Setup for the position past the end of the MCU Sample Space
					MCUPos.MCURow = ((Bounds.Bottom - Bounds.Top) / 8) + 1;
					MCUPos.MCUColumn = 0;
					MCUPos.SRow = 0;
					MCUPos.SColumn = 0;
					Index = computeMCUIndex();
					break;

				case STYLE_MCU12_BKWD:
					//  Setup for the position past the end of the MCU Sample Space
					MCUPos.MCURow = ((Bounds.Bottom - Bounds.Top) / 16) + 1;
					MCUPos.MCUColumn = 0;
					MCUPos.SRow = 0;
					MCUPos.SColumn = 0;
					Index = computeMCUIndex();
					break;
				}

				return;
			}

			//  Return to caller
			return;
		}

		//  computeMCUIndex
		//
		//  Computes the current target pixel of the iterator.
		//
		//  PARAMETERS
		//
		//  RETURNS
		//
		//		size_t		-		The index position based on the current MCU position
		//
		//  NOTES
		//

		size_t		computeMCUIndex() const {
			size_t			Position = 0;																	//  Computed position
			size_t			MCUH = 0;																		//  MCU Height
			size_t			MCUW = 0;																		//  MCU Width

			switch (itStyle) {
			case STYLE_MCU_FWD:
			case STYLE_MCU_BKWD:
				MCUH = MCUW = 16;
				break;
			case STYLE_MCU11_FWD:
			case STYLE_MCU11_BKWD:
				MCUH = MCUW = 8;
				break;
			case STYLE_MCU12_FWD:
			case STYLE_MCU12_BKWD:
				MCUH = 16;
				MCUW = 8;
				break;
			case STYLE_MCU21_FWD:
			case STYLE_MCU21_BKWD:
				MCUH = 8;
				MCUW = 16;
				break;
			}

			Position = MCUPos.MCURow * (MCUH * ((Bounds.Right - Bounds.Left) + 1));
			Position += MCUPos.SRow * ((Bounds.Right - Bounds.Left) + 1);
			Position += MCUPos.MCUColumn * MCUW;
			Position += MCUPos.SColumn;

			//  Return the computed position
			return Position;
		}

		//  computeNeighbourhood
		//
		//  Computes the current target pixel of parent iterator(s) the position is derived in Row, Column format and
		//  stored as the Top-Left position in the Orbit Extents.
		//
		//  PARAMETERS
		//
		//  RETURNS
		//
		//  NOTES
		//

		void	computeNeighbourhood() {
			size_t		Offset;

			//  Set the Top,Left of the bounds to the origin
			Hood.TargetR = 0;
			Hood.TargetC = 0;

			//  Detect the case where there is no parent
			if (&Parent == this) return;

			//  perform the computation according to the style of the parent iterator
			switch (Parent.itStyle) {
			case STYLE_LINEAR_FWD:
				Hood.TargetR = Parent.Index / ContainerWidth;
				Hood.TargetC = Parent.Index % ContainerWidth;
				break;

			case STYLE_LINEAR_BKWD:
				Hood.TargetR = (Parent.Index - 1) / ContainerWidth;
				Hood.TargetC = (Parent.Index - 1) % ContainerWidth;
				break;

			case STYLE_SUBCOL_LR:
				Hood.TargetR = Parent.Parent.Index;
				if (Parent.Parent.itStyle & CIT_DIRECTION_REVERSE) Hood.TargetR--;
				Hood.TargetC = Parent.Index;
				break;

			case STYLE_SUBCOL_RL:
				Hood.TargetR = Parent.Parent.Index;
				if (Parent.Parent.itStyle & CIT_DIRECTION_REVERSE) Hood.TargetR--;
				Orbit.Extents.Left = Parent.Index - 1;
				break;

			case STYLE_SUBROW_TB:
				Hood.TargetR = Parent.Index;
				Hood.TargetC = Parent.Parent.Index;
				if (Parent.Parent.itStyle & CIT_DIRECTION_REVERSE) Hood.TargetC--;
				break;

			case STYLE_SUBROW_BT:
				Hood.TargetR = Parent.Index - 1;
				Hood.TargetC = Parent.Parent.Index;
				if (Parent.Parent.itStyle & CIT_DIRECTION_REVERSE) Hood.TargetC--;
				break;

			case STYLE_SUBORBIT_CLOCK:
				Offset = Parent.effectiveOffset();
				Hood.TargetR = Offset / ContainerWidth;
				Hood.TargetC = Offset % ContainerWidth;
				break;

			case STYLE_SUBORBIT_ANTICLOCK:
				Offset = Parent.effectiveOffset();
				Hood.TargetR = Offset / ContainerWidth;
				Hood.TargetC = Offset % ContainerWidth;
				break;
			}

			//  Setup the start and last index values in the neighbourhood descriptor
			//  for Moore and Von Neumann (Depth = 1) neighbourhoods
			if (itStyle & (CIT_STYLE_MOOREHOOD | CIT_STYLE_VNHOOD)) {
				Hood.SwitchAfter = 0;
				Hood.SwitchTo = 0;
				//  Set the starting index and the last element index in the neighbourhood descriptor
				if (Hood.TargetR == Parent.Bounds.Top) {
					//  Top row of the iterator space
					if (Hood.TargetC == Parent.Bounds.Left) {
						Index = 3;																			//  Top left corner
						Hood.Last = 5;
					}
					else {
						if (Hood.TargetC == Parent.Bounds.Right) {
							Index = 5;																		//  Top right corner
							Hood.Last = 7;
						}
						else {
							Index = 3;																		//  Top edge
							Hood.Last = 7;
						}
					}
				}
				else {
					if (Hood.TargetR == Parent.Bounds.Bottom) {
						//  Bottom row of iterator space
						if (Hood.TargetC == Parent.Bounds.Left) {
							Index = 1;																		//  Bottom left corner
							Hood.Last = 3;
						}
						else {
							if (Hood.TargetC == Parent.Bounds.Right) {
								Index = 7;																	//  Bottom right corner
								Hood.Last = 9;
							}
							else {
								Index = 7;																	//  Bottom edge
								Hood.Last = 11;
							}
						}
					}
					else {
						//  Middle rows
						if (Hood.TargetC == Parent.Bounds.Left) {
							Index = 1;																		//  Left hand edge
							Hood.Last = 5;
						}
						else {
							if (Hood.TargetC == Parent.Bounds.Right) {
								Index = 5;																	//  Right hand edge
								Hood.Last = 9;
							}
							else {
								if (itStyle == STYLE_NEIGHBOURHOOD_MOORE) Index = 0;
								else Index = 1;																//  Central
								Hood.Last = 7;
							}
						}
					}
				}

				//  If this is the reverse direction then switch the start index and the finish
				if (itStyle & CIT_DIRECTION_REVERSE) {
					Offset = Index;
					Index = Hood.Last + 1;
					if (itStyle & CIT_STYLE_VNHOOD) Index = Index | 1;										//  Von Neumann always finish on an odd index
					Hood.Last = Offset;
				}
			}

			//  Setup the start, switchfrom, switchto and last index values in the neighbourhood descriptor
			//  for Moore Depth = 2 neighbourhood
			if (itStyle & CIT_STYLE_MOOREHOOD2) {
				//  There are 25 possible configurations for a depth = 2 Moore neighbourhood
				switch (getM2Config()) {
				case 1:
					Index = 3;
					Hood.SwitchAfter = 5;
					Hood.SwitchTo = 22;
					Hood.Last = 26;
					break;

				case 2:
					Index = 3;
					Hood.SwitchAfter = 7;
					Hood.SwitchTo = 22;
					Hood.Last = 27;
					break;

				case 3:
					Index = 3;
					Hood.SwitchAfter = 7;
					Hood.SwitchTo = 22;
					Hood.Last = 30;
					break;

				case 4:
					Index = 3;
					Hood.SwitchAfter = 7;
					Hood.SwitchTo = 25;
					Hood.Last = 30;
					break;

				case 5:
					Index = 5;
					Hood.SwitchAfter = 7;
					Hood.SwitchTo = 26;
					Hood.Last = 30;
					break;

				case 6:
					Index = 1;
					Hood.SwitchAfter = 5;
					Hood.SwitchTo = 21;
					Hood.Last = 26;
					break;

				case 7:
					Index = 0;
					Hood.SwitchAfter = 7;
					Hood.SwitchTo = 21;
					Hood.Last = 27;
					break;

				case 8:
					Index = 0;
					Hood.SwitchAfter = 7;
					Hood.SwitchTo = 21;
					Hood.Last = 31;
					break;

				case 9:
					Index = 0;
					Hood.SwitchAfter = 7;
					Hood.SwitchTo = 25;
					Hood.Last = 31;
					break;

				case 10:
					Index = 5;
					Hood.SwitchAfter = 9;
					Hood.SwitchTo = 26;
					Hood.Last = 31;
					break;

				case 11:
					Index = 1;
					Hood.SwitchAfter = 5;
					Hood.SwitchTo = 18;
					Hood.Last = 26;
					break;

				case 12:
					Index = 0;
					Hood.SwitchAfter = 7;
					Hood.SwitchTo = 17;
					Hood.Last = 27;
					break;

				case 13:
					//  Full neighbourhood
					Index = 0;
					Hood.SwitchAfter = 7;
					Hood.SwitchTo = 16;
					Hood.Last = 31;
					break;

				case 14:
					Index = 0;
					Hood.SwitchAfter = 7;
					Hood.SwitchTo = 25;
					Hood.Last = 35;
					break;

				case 15:
					Index = 5;
					Hood.SwitchAfter = 9;
					Hood.SwitchTo = 26;
					Hood.Last = 34;
					break;

				case 16:
					Index = 1;
					Hood.SwitchAfter = 5;
					Hood.SwitchTo = 18;
					Hood.Last = 23;
					break;

				case 17:
					Index = 0;
					Hood.SwitchAfter = 7;
					Hood.SwitchTo = 17;
					Hood.Last = 23;
					break;

				case 18:
					Index = 0;
					Hood.SwitchAfter = 7;
					Hood.SwitchTo = 29;
					Hood.Last = 39;
					break;

				case 19:
					Index = 0;
					Hood.SwitchAfter = 7;
					Hood.SwitchTo = 29;
					Hood.Last = 35;
					break;

				case 20:
					Index = 5;
					Hood.SwitchAfter = 9;
					Hood.SwitchTo = 29;
					Hood.Last = 34;
					break;

				case 21:
					Index = 1;
					Hood.SwitchAfter = 3;
					Hood.SwitchTo = 18;
					Hood.Last = 22;
					break;

				case 22:
					Index = 7;
					Hood.SwitchAfter = 11;
					Hood.SwitchTo = 17;
					Hood.Last = 22;
					break;

				case 23:
					Index = 7;
					Hood.SwitchAfter = 11;
					Hood.SwitchTo = 30;
					Hood.Last = 38;
					break;

				case 24:
					Index = 7;
					Hood.SwitchAfter = 11;
					Hood.SwitchTo = 30;
					Hood.Last = 35;
					break;

				case 25:
					Index = 7;
					Hood.SwitchAfter = 9;
					Hood.SwitchTo = 30;
					Hood.Last = 34;
					break;
				}

				//  If the iterator is a reverse iterator then switch and adjust the neighbourhood
				if (itStyle & CIT_DIRECTION_REVERSE) {
					Offset = Index;
					Index = Hood.Last + 1;
					Hood.Last = Offset + 1;
					Offset = Hood.SwitchAfter;
					Hood.SwitchAfter = Hood.SwitchTo + 1;
					Hood.SwitchTo = Offset + 1;
				}
			}

			//  Return to caller
			return;
		}

		//  getM2Config
		//
		//  Returns an integer that identifies the configuration of the Moore Depth = 2 neighbourhood
		//  for the current target cell.
		//
		//  PARAMETERS
		//
		//  RETURNS
		//
		//		int				-		Configuration of the current neighbourhood
		//
		//  NOTES
		//
		//		The configuration is determined by the following map :-
		//
		//		_________             ___             ________
		//		| 1  2                 3                 4  5 |
		//      | 6  7                 8                 9 10 |
		//
		//      | 11 12               13                14 15 |
		//
		//      | 16 17               18                19 20 |
		//      | 21 22               23                24 25 |
		//      ______________________________________________
		//
		//

		int		getM2Config() {
			if (Hood.TargetR == Parent.Bounds.Top) {
				//  Top row of the iterator space
				if (Hood.TargetC == Parent.Bounds.Left) return 1;
				if (Hood.TargetC == (Parent.Bounds.Left + 1)) return 2;
				if (Hood.TargetC == (Parent.Bounds.Right - 1)) return 4;
				if (Hood.TargetC == Parent.Bounds.Right) return 5;
				return 3;
			}

			if (Hood.TargetR == (Parent.Bounds.Top + 1)) {
				//  One cell below the top row
				if (Hood.TargetC == Parent.Bounds.Left) return 6;
				if (Hood.TargetC == (Parent.Bounds.Left + 1)) return 7;
				if (Hood.TargetC == (Parent.Bounds.Right - 1)) return 9;
				if (Hood.TargetC == Parent.Bounds.Right) return 10;
				return 8;
			}

			if (Hood.TargetR == (Parent.Bounds.Bottom - 1)) {
				//  One cell above the bottom row
				if (Hood.TargetC == Parent.Bounds.Left) return 16;
				if (Hood.TargetC == (Parent.Bounds.Left + 1)) return 17;
				if (Hood.TargetC == (Parent.Bounds.Right - 1)) return 19;
				if (Hood.TargetC == Parent.Bounds.Right) return 20;
				return 18;
			}

			if (Hood.TargetR == Parent.Bounds.Bottom) {
				//  Bottom row of the iterator space
				if (Hood.TargetC == Parent.Bounds.Left) return 21;
				if (Hood.TargetC == (Parent.Bounds.Left + 1)) return 22;
				if (Hood.TargetC == (Parent.Bounds.Right - 1)) return 24;
				if (Hood.TargetC == Parent.Bounds.Right) return 25;
				return 23;
			}

			//  Non-marginal rows
			if (Hood.TargetC == Parent.Bounds.Left) return 11;
			if (Hood.TargetC == (Parent.Bounds.Left + 1)) return 12;
			if (Hood.TargetC == (Parent.Bounds.Right - 1)) return 14;
			if (Hood.TargetC == Parent.Bounds.Right) return 15;
			return 13;
		}

	};


	//*******************************************************************************************************************
	//*                                                                                                                 *
	//*   Undefine Local Constants		                                                                                *
	//*                                                                                                                 *
	//*******************************************************************************************************************

#undef	CIT_STYLE_NONE
#undef	CIT_STYLE_LINEAR
#undef	CIT_STYLE_SCANROW
#undef	CIT_STYLE_SCANCOL
#undef	CIT_STYLE_ORBITAL
#undef	CIT_STYLE_PRIORITY
#undef	CIT_STYLE_MOOREHOOD
#undef	CIT_STYLE_VNHOOD
#undef	CIT_STYLE_MOOREHOOD2
#undef	CIT_STYLE_MCU
#undef	CIT_DIRECTION_REVERSE
#undef	CIT_SUBITERATOR
#undef	CIT_STYLE

}

