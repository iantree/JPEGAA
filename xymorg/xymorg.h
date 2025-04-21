#pragma once
//*******************************************************************************************************************
//*																													*
//*   File:       xymorg.h																							*
//*   Suite:      xymorg Integration																				*
//*   Version:    3.5.3	(Build: 44) - Dev Build:  XDB-063															*
//*   Author:     Ian Tree/HMNL																						*
//*																													*
//*   Copyright 2017 - 2025 Ian J. Tree																				*
//*******************************************************************************************************************
//*																													*
//*	This header file vectors the appropriate xymorg header includes according to the sub-system requirements of		*
//* the application.																								*
//*																													*
//*	USAGE:																											*
//*																													*
//*		Define the subsystem requirements (XY_NEEDS_XXX) before including this header.								*
//*		Where XXX identifies the particular sub-system or facility that is to be included.							*
//*																													*
//*		IDENTIFIER				SUBSYSTEM/FACILITY																	*
//*		----------				------------------																	*
//*																													*
//*			MP					Multi-Programming (threaded) sub-system												*
//*			DS					Directory Scanning facility															*
//*			IMG					Image Processing components															*
//*			GIF					GIF On-Disk Image																	*
//*			JPEG				JPEG/JFIF On-Disk Image																*
//*			BMP					BMP/DIB On-Disk Image																*
//*			GRAPHS				Graphing components																	*
//*			CRYPTO				Cryptographic components															*
//*			WEBUI				Web UI components including Wezzer													*
//*			NETIO				Network I/O																			*
//*			TLS					Secure Network I/O (e.g. HTTPS)														*
//*			TEXT				Text Rendering Engine																*
//*																													*
//*	NOTES:																											*
//*																													*
//*	1.		This is a minimal subset clone of xymorg supporting ONLY the MP,DS,IMG,GIF,JPEG,BMP extensions.			*																										*
//*																													*
//*******************************************************************************************************************
//*																													*
//*   History:																										*
//*																													*
//*	3.5.0 -	02/03/2020	-	Version rebased to origin 3.5															*
//* 3.5.1 -	12/05/2020	-	Minimal Subset generated																*	
//* 3.5.2 -	20/08/2023	-	Minimal Subset regenerated																*	
//* 3.5.3 -	18/12/2024	-	Minimal Subset regenerated																*	
//*																													*
//*******************************************************************************************************************/

//
//  Include core xymorg headers
//

#include	"LPBHdrs.h"																		//  Language and Platform base headers
#include	"types.h"																		//  xymorg type definitions
#include	"consts.h"																		//  xymorg constant definitions
#include	"AppConfig.h"																	//  xymorg Application Configuration Singleton

//
//  If image processing support is required then include the necessary headers
//
#ifdef XY_NEEDS_IMG
#include	"IMG/image.h"																	//  Image Processing Support
#endif
