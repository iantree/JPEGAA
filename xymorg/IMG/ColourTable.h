#pragma once
//
//*******************************************************************************************************************
//*																													*
//*   File:       ColourTable.h																						*
//*   Suite:      xymorg Image Processing - primitives																*
//*   Version:    1.0.0	  Build:  01																				*
//*   Author:     Ian Tree/HMNL																						*
//*																													*
//*   Copyright 2017 - 2020 Ian J. Tree																				*
//*******************************************************************************************************************
//*	ColourTale.h																									*
//*																													*
//*	This header file contains the Class definition and implementation for the ColourTable template primitive.		*
//* A ColourTable holds an array of colours in the specified type and the counts of pixels of each colour.          *
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

//*******************************************************************************************************************
//*                                                                                                                 *
//*   ColourTable Template Class		                                                                            *
//*                                                                                                                 *
//*   Objects of this class provide basic storage containers for arrays of colours.									*
//*   The Template Type (C) is a colour space type such as RGB.														*
//*                                                                                                                 *
//*******************************************************************************************************************

namespace xymorg {

	template <typename C>
	class ColourTable {
	public:

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Constructors                                                                                                  *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  Default Constructor
		//
		//  Constructs a valid but empty ColourTable
		//
		//  PARAMETERS
		//
		//  RETURNS
		//
		//  NOTES
		//

		ColourTable() {

			//  Allocate the colour table array for 256 colours
			NumColours = 0;
			AllocColours = 0;
			pCT = (CTE*) malloc(256 * sizeof(CTE));
			if (pCT == nullptr) return;
			memset(pCT, 0, 256 * sizeof(CTE));
			AllocColours = 256;

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
		//  Destroys the content of the ColourTable.
		//
		//  PARAMETERS
		//
		//  RETURNS
		//
		//  NOTES
		//

		~ColourTable() {
			//  Free the internal colour table (if allocated)
			if (pCT != nullptr) free(pCT);
			
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

		size_t			getNumColours() { return NumColours; }
		void			clear() { NumColours = 0; return; }

		//
		//  Colour Table Accessors
		//

		//  getColour
		//
		//  Returns the colour at a given index in the colour table
		//
		//  PARAMETERS
		//
		//		size_t		-		Colour Table Index
		//
		//  RETURNS
		//
		//		C			-		Colour at the given index in the table
		//
		//  NOTES
		//

		C		getColour(size_t CX) {

			//  Safety
			if (CX >= AllocColours) CX = 0;
			
			return pCT[CX].Colour;
		}

		//  getCount
		//
		//  Returns the pixel count of a given index position in the colour table
		//
		//  PARAMETERS
		//
		//		size_t		-		Colour Table Index
		//
		//  RETURNS
		//
		//		UINT		-		Pixel count for the colour index
		//
		//  NOTES
		//

		size_t		getCount(size_t CX) {

			//  Safety
			if (CX >= AllocColours) CX = 0;

			return pCT[CX].Count;
		}

		//  getDisplay
		//
		//  Returns the Display Character for a given index position in the colour table
		//
		//  PARAMETERS
		//
		//		size_t		-		Colour Table Index
		//
		//  RETURNS
		//
		//		char		-		Display character for the colour index
		//
		//  NOTES
		//

		char		getDisplay(size_t CX) {

			//  Safety
			if (CX >= AllocColours) return '?';

			return pCT[CX].Display;
		}

		//  getDisplay
		//
		//  Returns the Display Character for a given colour
		//
		//  PARAMETERS
		//
		//		C&			-		Const reference to the colour to be displayed
		//
		//  RETURNS
		//
		//		char		-		Display character for the colour 
		//
		//  NOTES
		//

		char		getDisplay(const C& Colour) {
			size_t		Cix = 0;												//  Colour Index
			CTE*		pCTE = findColour(Colour, Cix);							//  Pointer to the entry for the colour
		
			//  Not found
			if (pCTE == nullptr) return '?';

			//  Return the display character for the colour
			return pCTE->Display;
		}

		//  getExtents
		//
		//  Returns the use extents for a given index position in the colour table
		//
		//  PARAMETERS
		//
		//		size_t		-		Colour Table Index
		//
		//  RETURNS
		//
		//		BoundingBox&		-		Reference to the BoundingBox extents
		//
		//  NOTES
		//

		BoundingBox&		getExtents(const size_t CX) {

			//  Safety
			if (CX >= AllocColours) return pCT[0].Extents;

			return pCT[CX].Extents;
		}

		//  getExtents
		//
		//  Returns the use extents for a given colour
		//
		//  PARAMETERS
		//
		//		C&		-		Reference to the colour
		//
		//  RETURNS
		//
		//		BoundingBox&		-		Reference to the BoundingBox extents
		//
		//  NOTES
		//

		BoundingBox& getExtents(const C& Colour) {
			size_t		Cix = 0;												//  Colour Index
			CTE*		pCTE = findColour(Colour, Cix);							//  Pointer to the entry for the colour

			// Not Found - return the first
			if (pCTE == nullptr) return pCT[0].Extents;

			return pCT[Cix].Extents;
		}

		//
		//  Colour Table Manipulators
		//

		//  add
		//
		//  Adds a pixel count to an existing or new entry in the colour table
		//
		//  PARAMETERS
		//
		//		T&			-		Const reference to the colour to be added/updated
		//		size_t		-		Pixel count to be added
		//
		//  RETURNS
		//
		//  NOTES
		//

		void	add(const C& FCol, size_t PCount) {
			CTE*		pCTE = nullptr;													//  Colour table entry (CTE)
			size_t		CX = 0;															//  Colour Index

			//  Locate the colour table entry
			pCTE = findColour(FCol, CX);

			//  If found then update the colour table entry
			if (pCTE != nullptr) {
				pCTE->Count += PCount;
				return;
			}

			//
			//  Colour does not exist in the colour table, it must be added
			//

			if (NumColours == AllocColours) {
				//  Table must be expanded
				CTE* pNewCT = (CTE*) realloc(pCT, (AllocColours + 256) * sizeof(CTE));
				if (pNewCT == nullptr) return;
				pCT = pNewCT;
				memset(&pCT[AllocColours], 0, 256 * sizeof(CTE));
				AllocColours += 256;
			}

			//  Add the new entry to the table
			pCT[NumColours].Colour = FCol;
			pCT[NumColours].Count = PCount;
			pCT[NumColours].Display = '?';												//  Undefined colour index
			pCT[NumColours].Extents.Top = 0;
			pCT[NumColours].Extents.Bottom = 0;
			pCT[NumColours].Extents.Left = 0;
			pCT[NumColours].Extents.Right = 0;
			NumColours++;

			return;
		}

		//  add
		//
		//  Adds a pixel count to an existing or new entry in the colour table also maintaining the extents
		//
		//  PARAMETERS
		//
		//		T&			-		Const reference to the colour to be added/updated
		//		size_t		-		Pixel count to be added
		//		size_t		-		Row of the pixel position
		//		size_t		-		Column of the pixel position
		//
		//  RETURNS
		//
		//  NOTES
		//

		void	add(const C& FCol, size_t PCount, size_t Row, size_t Col) {
			CTE*		pCTE = nullptr;													//  Colour table entry (CTE)
			size_t		CX = 0;															//  Colour Index

			//  Locate the colour table entry
			pCTE = findColour(FCol, CX);

			//  If found then update the colour table entry
			if (pCTE != nullptr) {
				pCTE->Count += PCount;
				if (Row < pCTE->Extents.Top) pCTE->Extents.Top = Row;
				if (Row > pCTE->Extents.Bottom) pCTE->Extents.Bottom = Row;
				if (Col < pCTE->Extents.Left) pCTE->Extents.Left = Col;
				if (Col > pCTE->Extents.Right) pCTE->Extents.Right = Col;
				return;
			}

			//
			//  Colour does not exist in the colour table, it must be added
			//

			if (NumColours == AllocColours) {
				//  Table must be expanded
				CTE* pNewCT = (CTE*) realloc(pCT, (AllocColours + 256) * sizeof(CTE));
				if (pNewCT == nullptr) return;
				pCT = pNewCT;
				memset(&pCT[AllocColours], 0, 256 * sizeof(CTE));
				AllocColours += 256;
			}

			//  Add the new entry to the table
			pCT[NumColours].Colour = FCol;
			pCT[NumColours].Count = PCount;
			pCT[NumColours].Display = '?';												//  Undefined colour index
			pCT[NumColours].Extents.Top = Row;
			pCT[NumColours].Extents.Bottom = Row;
			pCT[NumColours].Extents.Left = Col;
			pCT[NumColours].Extents.Right = Col;
			NumColours++;

			return;
		}

		//  remove
		//
		//  Removes an entry (identified by index) from the colour table
		//
		//  PARAMETERS
		//
		//		size_t		-		Index of the colour entry to be removed from the table
		//
		//  RETURNS
		//
		//  NOTES
		//

		void		remove(size_t CX) {

			if (CX >= NumColours) return;

			//  Shuffle up the table
			if (CX < (NumColours - 1)) {
				memmove(&pCT[CX], &pCT[CX + 1], (NumColours - (CX + 1)) * sizeof(CTE));
			}

			//  Clear the last entry
			memset(&pCT[NumColours - 1], 0, sizeof(CTE));

			//  Reduce the colour count
			NumColours--;
		}

		//  remove
		//
		//  Removes an entry (identified by colour) from the colour table
		//
		//  PARAMETERS
		//
		//		C&		-		Reference to a colour value to be removed
		//
		//  RETURNS
		//
		//  NOTES
		//

		void		remove(const C& Colour) {
			size_t		CX = 0;																		//  Colour index
			CTE*		pCTE = findColour(Colour, CX);												//  Pointer to the colour entry

			if (pCTE != nullptr) remove(CX);														//  Remove the identified colour index

			//  Return to caller
			return;
		}

		//  removeAll
		//
		//  Removes all colour entries in the Colour Table that are wholly contained within the passed bounding box
		//
		//  PARAMETERS
		//
		//		BoundingBox&		-		Reference to the Bounding Box defining the bounds
		//
		//  RETURNS
		//
		//  NOTES
		//
		//		The Colour Table MUST have been populated using the functions that maintain the extents
		//

		void		removeAll(const BoundingBox& Extents) {
			size_t		CX = 0;																		//  Iterator

			//  Iterate over the colour table removing qualifying colour entries
			while (CX < NumColours) {
				if (pCT[CX].Extents.Top >= Extents.Top &&
					pCT[CX].Extents.Bottom <= Extents.Bottom &&
					pCT[CX].Extents.Left >= Extents.Left &&
					pCT[CX].Extents.Right <= Extents.Right) remove(CX);
				else CX++;
			}
			
			//  Return to caller
			return;
		}

		//  combine
		//
		//  Combines to colour entries in the table into a single entry
		//
		//  PARAMETERS
		//
		//		size_t		-		Index of the colour to combine (source)
		//		size_t		-		Index of the colour to combine with (target)
		//
		//  RETURNS
		//
		//  NOTES
		//

		void		combine(size_t SC, size_t TC) {

			//  Safety
			if (SC >= NumColours) return;
			if (TC >= NumColours) return;

			//  Update the pixel count
			pCT[TC].Count += pCT[SC].Count;

			//  Update the extents
			if (pCT[SC].Extents.Top < pCT[TC].Extents.Top) pCT[TC].Extents.Top = pCT[SC].Extents.Top;
			if (pCT[SC].Extents.Left < pCT[TC].Extents.Left) pCT[TC].Extents.Left = pCT[SC].Extents.Left;
			if (pCT[SC].Extents.Bottom > pCT[TC].Extents.Bottom) pCT[TC].Extents.Bottom = pCT[SC].Extents.Bottom;
			if (pCT[SC].Extents.Right > pCT[TC].Extents.Right) pCT[TC].Extents.Right = pCT[SC].Extents.Right;

			//  Remove the source enry
			remove(SC);

			//  Return to caller
			return;
		}

		//  hasColour
		//
		//  Tests if the passed colour is in the colour table
		//
		//  PARAMETERS
		//
		//		C&		-		Reference to a colour value to be checked
		//
		//  RETURNS
		//
		//		bool	-		true if the colour is in the table, otherwise false
		//
		//  NOTES
		//

		bool		hasColour(const C& Colour) {
			size_t		CX = 0;																		//  Colour index
			CTE*		pCTE = findColour(Colour, CX);												//  Pointer to the colour entry

			if (pCTE != nullptr) return true;														//  Signal colour is present

			//  Return to caller - not found
			return false;
		}

		//  getMostUsedColour
		//
		//  Returns the most used colour currently in the table
		//
		//  PARAMETERS
		//
		//  RETURNS
		//
		//		C&		-		Reference to the most used colour
		//
		//  NOTES
		//

		C&		getMostUsedColour() {
			size_t		Colour = 0;																	//  High used colour
			size_t		HiUsed = 0;																	//  High used count

			for (size_t CX = 0; CX < NumColours; CX++) {
				if (pCT[CX].Count > HiUsed) {
					Colour = CX;
					HiUsed = pCT[CX].Count;
				}
			}

			//  Return the selected colour
			return pCT[Colour].Colour;
		}

		//  reset
		//
		//  Resets the Colout table to the empty state
		//
		//  PARAMETERS
		//
		//  RETURNS
		//
		//  NOTES
		//

		void		reset() {

			NumColours = 0;
			memset(pCT, 0, AllocColours * sizeof(C));

			//  Return to caller
			return;
		}

		//  getLowestUsedColour
		//
		//  Returns the index of the lowest used colour in the Colour Table
		//
		//  PARAMETERS
		//
		//  RETURNS
		//
		//		size_t			-		Index of the lowest used colour in the table
		//
		//  NOTES
		//

		size_t		getLowestUsedColour() {
			size_t			Lowest = 0;

			for (size_t CX = 0; CX < NumColours; CX++) {
				if (pCT[CX].Count < pCT[Lowest].Count) Lowest = CX;
			}

			//  Return the lowest used index
			return Lowest;
		}

		//  contains
		//
		//  determines if (or not) the Colour Table contains ALL of the colours in the passed table
		//
		//  PARAMETERS
		//
		//  RETURNS
		//
		//		bool			-		true if the passed table is wholly contained, otherwise false
		//
		//  NOTES
		//

		bool		contains(ColourTable<C>& Candidate) {
			bool		HasColour = false;

			//  Iterate each colour in the Candidate table
			for (size_t CCX = 0; CCX < Candidate.getNumColours(); CCX++) {
				HasColour = false;
				for (size_t CX = 0; CX < NumColours; CX++) {
					if (Candidate.getColour(CCX) == pCT[CX].Colour) {
						HasColour = true;
						break;
					}
				}

				if (!HasColour) return false;
			}

			//  Return showing success (Candidate is contained in the current table)
			return true;
		}

		//  setupForDisplay
		//
		//  Prepares the colour table for display
		//
		//  PARAMETERS
		//
		//		T*			-		Const pointer to the background colour
		//		T*			-		Const pointer to the transparent colour
		//
		//  RETURNS
		//
		//  NOTES
		//

		void		setupForDisplay(const C* BGC, const C* GSC) {
			CTE*	pCTE = nullptr;													//  Colour table entry (CTE)
			size_t	Cix = 0;														//  Colour index

			//  If present mark the Background Colour '.' and the transparent colour ' '
			if (BGC != nullptr) {
				pCTE = findColour(*BGC, Cix);
				if (pCTE != nullptr) pCTE->Display = '.';
			}
			
			if (GSC != nullptr) {
				pCTE = findColour(*GSC, Cix);
				if (pCTE != nullptr) pCTE->Display = ' ';
			}

			//  Set the rest of the display characters according to frequency
			setDisplayChars();

			//  Return to caller
			return;
		}

	protected:

		//*******************************************************************************************************************
		//*																													*
		//*  Protected Members																								*
		//*																													*
		//*******************************************************************************************************************

		size_t			NumColours;															//  Number of colours defined in the table
		size_t			AllocColours;														//  Number of colours allocated in the table

		struct CTE {
			C			Colour;																//  Colour
			size_t		Count;																//  Pixel count
			char		Display;															//  Display Character to use
			BoundingBox	Extents;															//  Extents of the colour use
		} * pCT;

	private:

		//*******************************************************************************************************************
		//*																													*
		//*  Private Members																								*
		//*																													*
		//*******************************************************************************************************************

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Private Functions                                                                                             *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  findColour
		//
		//  Locates the passed colour in the colour table (or not)
		//
		//  PARAMETERS
		//
		//		C&			-		Const reference to the colour to be searched for
		//		size_t&		-		Reference where the index of the colour will be returned
		//
		//  RETURNS
		//
		//		CTE*		-		Pointer to the table entry for the desired colour, NULL if not found in the table
		//
		//  NOTES
		//

		CTE* findColour(const C& FCol, size_t& CX) {

			//  Boundary - Colour table is empty
			if (NumColours == 0) {
				CX = 1;
				return nullptr;
			}

			//  Search the colour table
			for (CX = 0; CX < NumColours; CX++) {
				if (pCT[CX].Colour == FCol) return &pCT[CX];
			}

			//  Not Found
			return nullptr;
		}

		//  setDisplayChars
		//
		//  On a frequency basis this function assigns the available display characters to the entries in the colour table
		//
		//  PARAMETERS
		//
		//  RETURNS
		//
		//  NOTES
		//

		void setDisplayChars() {
			size_t		ColoursDone = 0;														//  Count of colours processed
			size_t		NextChar = 0;															//  Next available character
			char		DChars[] = "0123456789ABCDEF:;*!#$&+=<>~abcdefghijklmnopqrstuvwxyzGHIJKLMNOPQRSTUVWXYZ@";
			size_t		MaxChar = strlen(DChars);												//  Max available characters
			size_t		CESel = 0;																//  Selected colour table entry

			//  Process until all entries are assigned or we run out of characters
			while (ColoursDone < NumColours && NextChar < MaxChar) {

				//  Locate the first unassigned colour
				for (size_t CX = 0; CX < NumColours; CX++) {
					if (pCT[CX].Display == '?') {
						CESel = CX;
						break;
					}
				}

				//  Find the highest count entry in the colour table array that has s default display char assigned
				for (size_t CX = 0; CX < NumColours; CX++) {
					if (pCT[CX].Count >= pCT[CESel].Count && pCT[CX].Display == '?') CESel = CX;
				}

				//  Assign the next available display character to the selected entry
				pCT[CESel].Display = DChars[NextChar++];
				ColoursDone++;
			}

			//  Return to caller
			return;
		}
	};

}
