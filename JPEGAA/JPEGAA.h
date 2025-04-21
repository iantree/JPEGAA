#pragma once
//*******************************************************************************************************************
//*																													*
//*   File:       JPEGAA.h																							*
//*   Suite:      Experimental Algorithms																			*
//*   Version:    1.0.0	(Build: 01)																					*
//*   Author:     Ian Tree/HMNL																						*
//*																													*
//*   Copyright 2017 - 2025 Ian J. Tree																				*
//*******************************************************************************************************************
//*	JPEGAA																											*
//*																													*
//*	This application demonstrates the entropy from repeated JPEG encoding.											*
//*																													*
//*	USAGE:																											*
//*																													*
//*		JPEGAA <Project>																							*
//*																													*
//*     where:-																										*
//*																													*
//*		<Project>			-	Is the path to the directory project files to use.									*
//*																													*
//*	NOTES:																											*
//*																													*
//*	1.																												*
//*																													*
//*******************************************************************************************************************
//*																													*
//*   History:																										*
//*																													*
//*	1.0.0 -		17/04/2025	-	Initial Release																		*
//*																													*
//*******************************************************************************************************************/

//  Define xymorg sub-system requirements
#define		XY_NEEDS_IMG
#define		XY_NEEDS_GIF
#define		XY_NEEDS_BMP
#define		XY_NEEDS_JPEG

//  Include xymorg headers
#include	"../xymorg/xymorg.h"															//  xymorg system headers

//  Application Headers
#include	"JAACfg.h"

//  Identification Constants
constexpr auto		APP_NAME = "JPEGAA";
constexpr auto		APP_TITLE = "JPEG Ad Absurdum";
#ifdef _DEBUG
constexpr auto		APP_VERSION = "1.0.0 build: 01 Debug";
#else
constexpr auto		APP_VERSION = "1.0.0 build: 01";
#endif

//  Forward Declarations/ Function Prototypes
void		runExperiment(JAACfg& Config);																			//  Run the experiment
xymorg::Train<xymorg::RGB>* performEncodingCycle(int Cycle, xymorg::Train<xymorg::RGB>* pImgIn, JAACfg& Config);	//  Perform an encoding cycle
