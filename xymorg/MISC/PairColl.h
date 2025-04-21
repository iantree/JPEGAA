#pragma once
//*******************************************************************************************************************
//*																													*
//*   File:       PairColl.h																						*
//*   Suite:      xymorg Integration																				*
//*   Version:    1.0.0	(Build: 01)																					*
//*   Author:     Ian Tree/HMNL																						*
//*																													*
//*   Copyright 2017 - 2018 Ian J. Tree.																			*
//*******************************************************************************************************************
//*																													*
//*	This header file contains the definition for the PairColl class. The class provides storage and manipulation	*
//* for Name/Value data pairs (JSON or environment like).															*
//*																													*
//*	USAGE:																											*
//*																													*
//*																													*
//*	NOTES:																											*
//*																													*
//*	1.																												*
//*																													*
//*******************************************************************************************************************
//*																													*
//*   History:																										*
//*																													*
//*	1.0.0 -		06/11/2018	-	Initial Release																		*
//*																													*
//*******************************************************************************************************************/

//
//  Include core xymorg headers
//

#include	"../LPBHdrs.h"																		//  Language and Platform base headers
#include	"../types.h"																		//  xymorg type definitions
#include	"../consts.h"																		//  xymorg constant definitions
#include	"../StringPool.h"																	//  String Pool
#include	"../StringThing.h"																	//  String Manipulator

//
//  All components are defined within the xymorg namespace
//
namespace xymorg {

	//
	//  PairColl Class Definition
	//

	class PairColl {

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Private Types																									*
		//*                                                                                                                 *
		//*******************************************************************************************************************

	private:

		typedef struct Pair {
			STRREF			Name;															//  Variable Name
			STRREF			Value;															//  Variable Value
		} Pair;

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Private Constants		                                                                                        *
		//*                                                                                                                 *
		//*******************************************************************************************************************

	private:

		static const int		PAIR_INIT_CAP = 100;											//  Initial capacity of the Gateway variables array

	public:

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Constructors			                                                                                        *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  Default Constructor 
		//
		//  Constructs a new PairCollection
		//
		//  PARAMETERS:
		//
		//  RETURNS:
		//
		//  NOTES:
		//

		PairColl() : SPool() {

			//  Variable Pair Array
			PVCap = PAIR_INIT_CAP;
			PVCount = 0;
			pPairs = (Pair *) malloc(PVCap * sizeof(Pair));
			if (pPairs == nullptr) {
				return;
			}
			memset(pPairs, 0, PVCap * sizeof(Pair));

			//  Return to caller
			return;
		}

		//  Copy Constructor 
		//
		//  Constructs a new PairCollection as a deep copy of a source PairCollection
		//
		//  PARAMETERS:
		// 
		//		PairColl&		-		Const reference to the source collection
		//
		//  RETURNS:
		//
		//  NOTES:
		//

		PairColl(const PairColl& Src) : SPool(Src.SPool) {

			PVCap = Src.PVCap;
			PVCount = Src.PVCount;
			pPairs = (Pair*) malloc(PVCap * sizeof(Pair));
			if (pPairs == nullptr) {
				SPool.dismiss();
				PVCap = 0;
				PVCount = 0;
				return;
			}

			// Copy the array of Pairs
			memcpy(pPairs, Src.pPairs, PVCap * sizeof(Pair));

			//  Return to caller
			return;
		}

		//  Move Constructor 
		//
		//  Constructs a new PairCollection as a shallow copy of a source PairCollection, taking ownership of the content
		//
		//  PARAMETERS:
		// 
		//		PairColl&&		-		Reference to the source collection
		//
		//  RETURNS:
		//
		//  NOTES:
		//

		PairColl(PairColl&& Src) noexcept {

			PVCap = Src.PVCap;
			PVCount = Src.PVCount;
			pPairs = Src.pPairs;
			Src.PVCap = 0;
			Src.PVCount = 0;
			Src.pPairs = nullptr;

			SPool = std::move(Src.SPool);

			//  Return to caller
			return;
		}

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Destructor			                                                                                        *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  Destructor
		//
		//  Destroys the PairColl object releasing any underlying storage.
		//
		//  PARAMETERS:
		//
		//  RETURNS:
		//
		//  NOTES:
		//  

		~PairColl() {

			//  Dismiss this object
			dismiss();

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

		//  dismiss
		//
		//  This function will dismiss the pair collection, releasing acquired storage 
		//
		//  PARAMETERS:
		//
		//  RETURNS:
		//
		//  NOTES:
		//  

		void	dismiss() {

			//  Free the variable arrays (if allocated)
			if (pPairs != nullptr) free(pPairs);
			pPairs = nullptr;
			PVCap = 0;
			PVCount = 0;

			//  Dismiss the string pool contents
			SPool.dismiss();

			//  Return to caller
			return;
		}

		//  addVariable
		//
		//  This function will add or replace a vairable name and value pair in the pool
		//
		//  PARAMETERS:
		//
		//		char*			-		Const pointer to the variable name to be added
		//		char*			-		Const pointer to the variable value to be added
		//
		//  RETURNS:
		// 
		//		bool			-		true if the variable was added, otherwise false
		//
		//  NOTES:
		//  

		bool	addVariable(const char* pVName, const char* pValue) {
			size_t		VX = 0;																//  Variable index

			//  Safety
			if (pVName == nullptr) return false;
			if (pVName[0] == '\0') return false;

			//  Determine if this is a replacement or addition
			for (VX = 0; VX < PVCount; VX++) {
				if (_stricmp(SPool.getString(pPairs[VX].Name), pVName) == 0) break;
			}

			//  Add a new variable pair entry
			if (VX == PVCount) {
				if (!expandArray(PVCap, PVCount, pPairs)) return false;
				pPairs[PVCount].Name = SPool.addString(pVName);
				pPairs[PVCount].Value = SPool.addString(pValue);
				PVCount++;
			}
			else {
				//  Replace the value of the named variable in the pool - if it has changed
				if (strcmp(SPool.getString(pPairs[VX].Value), pValue) != 0) {
					if (pPairs[VX].Name != NULLSTRREF && pPairs[VX].Value != pPairs[VX].Name) SPool.deleteString(pPairs[VX].Value);
					pPairs[VX].Value = SPool.addString(pValue);
				}
			}

			//  Return to caller
			return true;
		}

		//  addVariable
		//
		//  This function will add or replace a non-null-terminated vairable name and value pair in the pool
		//
		//  PARAMETERS:
		//
		//		char*			-		Const pointer to the variable name to be added
		//		size_t			-		Length of the variable name
		//		char*			-		Const pointer to the variable value to be added
		//		size_t			-		Length of the variable value
		//
		//  RETURNS:
		// 
		//		bool			-		true if the variable was added, otherwise false
		//
		//  NOTES:
		//  

		bool	addVariable(const char* pVName, size_t VNL, const char* pValue, size_t VVL) {
			size_t		VX = 0;																//  Variable index

			//  Safety
			if (pVName == nullptr) return false;
			if (*pVName == '\0') return false;
			if (VNL == 0) return false;
			if (pValue == nullptr) VVL = 0;

			//  Determine if this is a replacement or addition
			for (VX = 0; VX < PVCount; VX++) {
				if (_strnicmp(SPool.getString(pPairs[VX].Name), pVName, VNL) == 0) {
					if (VNL == SPool.getLength(pPairs[VX].Name)) break;
				}
			}

			//  Add a new variable pair entry
			if (VX == PVCount) {
				if (!expandArray(PVCap, PVCount, pPairs)) return false;
				pPairs[PVCount].Name = SPool.addString(pVName, VNL);
				pPairs[PVCount].Value = SPool.addString(pValue, VVL);
				PVCount++;
			}
			else {
				//  Replace the value of the named variable in the pool
				if (pPairs[VX].Name != NULLSTRREF && pPairs[VX].Value != pPairs[VX].Name) SPool.deleteString(pPairs[VX].Value);
				pPairs[VX].Value = SPool.addString(pValue, VVL);
			}

			//  Return success
			return true;
		}

		//  deleteVariable
		//
		//  This function will delete vairable name and value pair from the pool
		//
		//  PARAMETERS:
		//
		//		char*			-		Const pointer to the variable name to be deleted
		//
		//  RETURNS:
		// 
		//		bool			-		true if the item was deleted, otherwise false
		//
		//  NOTES:
		//

		bool	deleteVariable(const char* pVName) {
			size_t		VX = 0;																//  Variable index

			//  Safety
			if (pVName == nullptr) return false;
			if (pVName[0] == '\0') return false;

			//  Determine if the nameed variable exists in the pairs array
			for (VX = 0; VX < PVCount; VX++) {
				if (_stricmp(SPool.getString(pPairs[VX].Name), pVName) == 0) break;
			}

			//  If the variable name was NOT FOUND - just return
			if (VX == PVCount) return false;

			//  Delete the name string
			SPool.deleteString(pPairs[VX].Name);
			
			//  Delete the value string
			if (pPairs[VX].Name != NULLSTRREF && pPairs[VX].Value != pPairs[VX].Name) SPool.deleteString(pPairs[VX].Value);

			//  Shuffle up the remaining entries in the pairs array
			for (VX++; VX < PVCount; VX++) {
				pPairs[VX - 1].Name = pPairs[VX].Name;
				pPairs[VX - 1].Value = pPairs[VX].Value;
			}

			//  Reduce the count
			PVCount--;

			//  Return to caller
			return true;
		}

		//  getLength
		//
		//  This function returns the length of the specified variable value.
		//
		//  PARAMETERS:
		//
		//		char*			-		Pointer to the variable name to be queried
		//
		//  RETURNS:
		//
		//		size_t			-		Length of the value of the variable
		//
		//  NOTES:
		//  

		size_t		getLength(const char* pVName) {

			//  Safety
			if (pVName == nullptr) return 0;
			if (pVName[0] == '\0') return 0;

			return getValueLength(pVName, PVCount, pPairs);
		}

		//  getLength
		//
		//  This function returns the length of the specified variable value (by index)
		//
		//  PARAMETERS:
		//
		//		size_t			-		Index of the variable
		//
		//  RETURNS:
		//
		//		size_t			-		Length of the value of the variable
		//
		//  NOTES:
		//  

		size_t		getLength(size_t VPX) {
			if (VPX >= PVCount) return 0;
			return SPool.getLength(pPairs[VPX].Value);
		}

		//  getNameLength
		//
		//  This function returns the length of the specified variable name (by index)
		//
		//  PARAMETERS:
		//
		//		size_t			-		Index of the variable
		//
		//  RETURNS:
		//
		//		size_t			-		Length of the name of the variable
		//
		//  NOTES:
		//  

		size_t		getNameLength(size_t VPX) {
			if (VPX >= PVCount) return 0;
			return SPool.getLength(pPairs[VPX].Name);
		}

		//  getName
		//
		//  This function returns a const pointer to the name of the given variable (by index)
		//
		//  PARAMETERS:
		//
		//		size_t			-		Index of the variable
		//
		//  RETURNS:
		//
		//		char*			-		Const pointer to the name of the variable
		//
		//  NOTES:
		//  

		const char*		getName(size_t VPX) {
			if (VPX >= PVCount) return nullptr;
			return SPool.getString(pPairs[VPX].Name);
		}

		//  getValue
		//
		//  This function returns the value of the specified variable
		//
		//  PARAMETERS:
		//
		//		char*			-		Pointer to the variable name to be queried
		//
		//  RETURNS:
		//
		//		const char*		-		Const pointer to the value of the specified variable
		//
		//  NOTES:
		//  

		const char*		getValue(const char* pVName) {

			//  Safety
			if (pVName == nullptr) return nullptr;
			if (pVName[0] == '\0') return nullptr;
			if (getLength(pVName) == 0) return SPool.getString(NULLSTRREF);

			return getValueString(pVName, PVCount, pPairs);
		}

		//  getValue
		//
		//  This function returns the value of the specified variable (by index).
		//
		//  PARAMETERS:
		//
		//		size_t			-		index of the variable to be queried
		//
		//  RETURNS:
		//
		//		const char*		-		Const pointer to the value of the specified variable
		//
		//  NOTES:
		//  

		const char*		getValue(size_t VPX) {

			//  Safety
			if (VPX >= PVCount) return nullptr;

			return SPool.getString(pPairs[VPX].Value);
		}

		//  copyValue
		//
		//  This function returns the value of the specified variable in a newly allocated buffer
		//
		//  PARAMETERS:
		//
		//		char*			-		Pointer to the variable name to be queried
		//
		//  RETURNS:
		//
		//		char*			-		Pointer to the buffer holding value of the specified variable
		//
		//  NOTES:
		//  

		char* copyValue(const char* pVName) {
			size_t			ValLen = 0;
			char*			pValue = nullptr;

			//  Safety
			if (pVName == nullptr) return nullptr;
			if (pVName[0] == '\0') return nullptr;
			ValLen = getLength(pVName);

			//  Allocate a buffer to hold the value
			ValLen++;
			pValue = (char*) malloc(ValLen);
			if (pValue == nullptr) return nullptr;
			memset(pValue, 0, ValLen);
			if (ValLen == 1) return pValue;

			//  Copy the value into the buffer
			memcpy(pValue, getValue(pVName), ValLen);

			//  Return the buffer
			return pValue;
		}

		//  copyValue
		//
		//  This function returns the value of the specified variable in a newly allocated buffer
		//
		//  PARAMETERS:
		//
		//		size_t			-		Index into the pair table of the variable to get
		//
		//  RETURNS:
		//
		//		char*			-		Pointer to the buffer holding value of the specified variable
		//
		//  NOTES:
		//  

		char* copyValue(size_t VPX) {
			size_t			ValLen = 0;
			char*			pValue = nullptr;

			//  Safety
			if (VPX >= PVCount) return nullptr;
			ValLen = getLength(VPX);

			//  Allocate a buffer to hold the value
			ValLen++;
			pValue = (char*) malloc(ValLen);
			if (pValue == nullptr) return nullptr;
			memset(pValue, 0, ValLen);
			if (ValLen == 1) return pValue;

			//  Copy the value into the buffer
			memcpy(pValue, getValue(VPX), ValLen);

			//  Return the buffer
			return pValue;
		}

		//  getCount
		//
		//  This function returns the count of pairs that are stored in the pool
		//
		//  PARAMETERS:
		//
		//  RETURNS:
		//
		//		size_t		-		Count of pairs in the pool
		//
		//  NOTES:
		//  

		size_t		getCount() const { return PVCount; }

		//  Copy Assignment operator =
		//
		//  This function copies one PairColl to another replacing the content.
		//
		//  PARAMETERS:
		// 
		//		PairColl&		-		Const reference to the source PairColl
		//
		//  RETURNS:
		//
		//		PairColl&		-		Reference to the target PairColl
		//
		//  NOTES:
		//  

		PairColl&	operator = (const PairColl& rhs) {

			if (pPairs != nullptr) free(pPairs);
			PVCap = rhs.PVCap;
			PVCount = rhs.PVCount;
			pPairs = (Pair*) malloc(PVCap * sizeof(Pair));
			if (pPairs == nullptr) {
				SPool.dismiss();
				PVCap = 0;
				PVCount = 0;
				return *this;
			}

			// Copy the array of Pairs
			memcpy(pPairs, rhs.pPairs, PVCap * sizeof(Pair));

			//  Copy the string pool
			SPool = rhs.SPool;

			//  Return to caller
			return *this;
		}

		//  Move Assignment operator =
		//
		//  This function copies one PairColl to another replacing the content ownership.
		//
		//  PARAMETERS:
		// 
		//		PairColl&		-		Const reference to the source PairColl
		//
		//  RETURNS:
		//
		//		PairColl&		-		Reference to the target PairColl
		//
		//  NOTES:
		//  

		PairColl&	operator = (PairColl&& rhs) noexcept {

			if (pPairs != nullptr) free(pPairs);
			PVCap = rhs.PVCap;
			PVCount = rhs.PVCount;
			pPairs = rhs.pPairs;

			rhs.PVCap = 0;
			rhs.PVCount = 0;
			rhs.pPairs = nullptr;

			//  Move the string pool
			SPool = std::move(rhs.SPool);

			//  Return to caller
			return *this;
		}

	private:

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Private Members																								*
		//*                                                                                                                 *
		//*******************************************************************************************************************

		StringPool			SPool;															//  String Pool (variable storage)
		
		//  Pair arrays for the variables
		size_t				PVCap;															//  Capacity for gateway variables
		size_t				PVCount;														//  Count of gateway variables
		Pair*				pPairs;															//  Pointer to the Gateway pairs array

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Private Functions																								*
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  expandArray
		//
		//  This function will expand the array that is passed to provide capacity for an additional entry
		//
		//  PARAMETERS:
		//
		//		size_t&			-		Reference to the current capacity
		//		size_t&			-		Reference to the current count
		//		Pait*&			-		Reference to the array pointer
		//
		//  RETURNS:
		//
		//		bool		-		true if the array now has capacity, otherwise false
		//
		//  NOTES:
		//  

		bool	expandArray(size_t& Capacity, size_t& Count, Pair*& pArray) {

			//  Test if capacity already exists
			if (Count < Capacity) return true;

			//  Expand the array
			Capacity += 50;
			Pair* NewArray = (Pair *) realloc(pArray, Capacity * sizeof(Pair));
			if (NewArray == nullptr) {
				free(pArray);
				return false;
			}
			else pArray = NewArray;
			return true;
		}

		//  getValueLength
		//
		//  This function will return the length of the value of the named variable in the passed pool
		//
		//  PARAMETERS:
		//
		//		char*			-		Const pointer to the variable to be queried
		//		size_t&			-		Reference to the current count
		//		Pait*&			-		Reference to the array pointer
		//
		//  RETURNS:
		//
		//		size_t		-		Length of the value (if available)
		//
		//  NOTES:
		//  

		size_t		getValueLength(const char* pVName, size_t& Count, Pair*& pArray) {

			//  Safety 
			if (pVName == nullptr) return 0;
			if (pVName[0] == '\0') return 0;
			if (Count == 0) return 0;
			
			//  Search the pool for an entry that matches the passed name (case insensitive)
			for (size_t PAX = 0; PAX < Count; PAX++) {
				if (_stricmp(pVName, SPool.getString(pArray[PAX].Name)) == 0) {
					return SPool.getLength(pArray[PAX].Value);
				}
			}

			//  Not found - return 0
			return 0;
		}

		//  getValueString
		//
		//  This function will return a pointer to the requested variable value
		//
		//  PARAMETERS:
		//
		//		char*			-		Const pointer to the variable to be queried
		//		size_t&			-		Reference to the current count
		//		Pait*&			-		Reference to the array pointer
		//
		//  RETURNS:
		//
		//		const char*		-		Const pointer to the variable value
		//
		//  NOTES:
		//  

		const char*		getValueString(const char* pVName, size_t& Count, Pair*& pArray) {

			//  Safety 
			if (pVName == nullptr) return nullptr;
			if (pVName[0] == '\0') return nullptr;
			if (Count == 0) return nullptr;

			//  Search the pool for an entry that matches the passed name (case insensitive)
			for (size_t PAX = 0; PAX < Count; PAX++) {
				if (_stricmp(pVName, SPool.getString(pArray[PAX].Name)) == 0) {
					return SPool.getString(pArray[PAX].Value);
				}
			}

			//  Not found - return an empty string
			return SPool.getString(NULLSTRREF);
		}

	};

}
