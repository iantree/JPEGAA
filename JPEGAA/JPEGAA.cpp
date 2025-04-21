//*******************************************************************************************************************
//*																													*
//*   File:       JPEGAA.cpp																						*
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

#include	"JPEGAA.h"

//
//  Main entry point for the JPEGAA application
//

int main(int argc, char* argv[])
{
	JAACfg			Config(APP_NAME, argc, argv);								//  Application configuration

	if (!Config.isLogOpen()) {
		std::cerr << "ERROR: The application logger was unable to start, " << APP_NAME << " will not execute." << std::endl;
		return EXIT_FAILURE;
	}

	//  Show that program is starting 
	Config.Log << APP_TITLE << " (" << APP_NAME << ") Version: " << APP_VERSION << " is starting." << std::endl;

	//  Verify the capture of configuration variables
	if (!Config.isValid()) {
		Config.Log << "ERROR: The application configuration is not valid, no further processing is possible." << std::endl;
		return EXIT_FAILURE;
	}

	//  Run the experiment
	runExperiment(Config);

	//  Dismiss the xymorg sub-systems
	Config.dismiss();

	//
	//  PLATFORM SPECIFIC for Windows DEBUG ONLY
	//
	//  Check for memory leaks
	//
#if ((defined(_WIN32) || defined(_WIN64)) && defined(_DEBUG))
	CheckForMemoryLeaks();
#endif

	return EXIT_SUCCESS;
}

//  runExperiment
//
//  This function is the top level function for performing the multi-cycle JPEG encoding experiment.
//
//  PARAMETERS:
// 
//			JAACfg&			-		Reference to the application configuration
//
//  RETURNS:
//
//  NOTES:
//

void	runExperiment(JAACfg& Config) {
	xymorg::Train<xymorg::RGB>*		pCurrImg = nullptr;								//  Pointer to the train of the current image
	int								CycleNo = 0;									//  Encoding cycle number

	//  Report the run configuration
	Config.Log << "INFO: The experiment will use: '" << Config.getBaseImage() << "' as a base gif image." << std::endl;
	Config.Log << "INFO: Intermediate images will be stored in the: '" << Config.getImgDir() << "' directory." << std::endl;
	Config.Log << "INFO: The experiment will perform: " << Config.getCycles() << " cycles of JPEG encoding." << std::endl;

	//  Attempt to load the image into memory
	pCurrImg = xymorg::GIF::loadImage(Config.getBaseImage(), Config.RMap);
	if (pCurrImg == nullptr) {
		Config.Log << "ERROR: Unable to load the requested base gif image, make sure that the name is correct and it is a valid gif image." << std::endl;
		Config.Log << "ERROR: The experiment could not be performed." << std::endl;
		return;
	}

	//  Determine if we have an animated GIF (not acceptable)
	if (pCurrImg->getNumFrames() > 1) {
		//  Warn
		Config.Log << "WARNING: The base gif image contains multiple frames, it will be flattened for the experiment." << std::endl;
		pCurrImg->flatten();
	}

	//  Document the image
	Config.Log << xymorg::undecorate;
	Config.Log << "JPEG Degeneration - Base Image" << std::endl;
	Config.Log << "------------------------------" << std::endl;
	Config.Log << std::endl;
	pCurrImg->document(Config.Log, true);
	Config.Log << xymorg::decorate;

	//
	//  Perform the requested number of encoding cycles
	//

	while (CycleNo < Config.getCycles() && pCurrImg != nullptr) {
		CycleNo++;
		pCurrImg = performEncodingCycle(CycleNo, pCurrImg, Config);
	}

	//  Show cycles completed
	if (pCurrImg == nullptr) Config.Log << "ERROR: The experiment did NOT complete, see previous message(s)." << std::endl;
	else Config.Log << "INFO: Experiment completed after: " << CycleNo << " encoding cycles." << std::endl;

	//  Destroy the current image train
	delete pCurrImg;

	//  Return to caller
	return;
}

//  performEncodingCycle
//
//  This function will perform a single encoding cycle
//
//  PARAMETERS:
// 
//			int				-		Cycle number
//			Train*			-		Pointer to the current (input) image train
//			JAACfg&			-		Reference to the application configuration
//
//  RETURNS:
// 
//			Train*			-		Pointer to the new image train, nullptr if the encoding cycle failed
//
//  NOTES:
// 
//	1.	The input image train is consumed by this function
//

xymorg::Train<xymorg::RGB>* performEncodingCycle(int Cycle, xymorg::Train<xymorg::RGB>* pImgIn, JAACfg& Config) {
	xymorg::Train<xymorg::RGB>*		pImgOut = nullptr;								//  Pointer to the train of the output image
	char							IFName[MAX_PATH] = {};							//  Image file name

	//  Safety
	if (pImgIn == nullptr) return nullptr;

	Config.Log << "INFO: Starting cycle: " << Cycle << " of JPEG encoding." << std::endl;

	//
	//		Step #1  -  Save the input image as a JPEG (default encoding)
	//

	//  Format the image file name
	sprintf_s(IFName, MAX_PATH, "%s/JPC%.2i.jpeg", Config.getImgDir(), Cycle);

	//  Save the image as a JPEG (default settings for encoding)
	if (!xymorg::JPEG::storeImage(IFName, Config.RMap, pImgIn)) {
		Config.Log << "ERROR: Failed to encode and store JPEG image: '" << IFName << "'." << std::endl;
		delete pImgIn;
		return nullptr;
	}
	Config.Log << "INFO: Image has been stored as a JPEG encoded image in: '" << IFName << "' on cycle: " << Cycle << "." << std::endl;

	//  Delete the image
	delete pImgIn;

	//
	//		Step #2  -  Reload the saved image
	//

	//  Load the image
	pImgOut = xymorg::JPEG::loadImage(IFName, Config.RMap);
	if (pImgOut == nullptr) {
		Config.Log << "ERROR: Failed to reload te JPEG image: '" << IFName << "'." << std::endl;
		return nullptr;
	}

	Config.Log << "INFO: The JPEG image: '" << IFName << "' has been successfully reloaded for cycle: " << Cycle << "." << std::endl;

	//
	//		Step #3  -  Document the reloaded image
	// 
	
	Config.Log << xymorg::undecorate;
	Config.Log << "JPEG Degeneration - Cycle: " << Cycle << std::endl;
	if (Cycle > 9) Config.Log << "------------------------------" << std::endl;
	else Config.Log << "-----------------------------" << std::endl;
	Config.Log << std::endl;
	pImgOut->document(Config.Log, true);
	Config.Log << xymorg::decorate;

	//
	//		Step 4  -  Save a bitmap image of the current image cycle
	//

	//  Format the image file name
	sprintf_s(IFName, MAX_PATH, "%s/JPC%.2i.bmp", Config.getImgDir(), Cycle);

	//  Save the image
	if (!xymorg::BMP::storeImage(IFName, Config.RMap, pImgOut)) {
		Config.Log << "ERROR: Failed to store a bitmap of the loaded image to: '" << IFName << "'." << std::endl;
		delete pImgOut;
		return nullptr;
	}

	//  Return the output image
	return pImgOut;
}
