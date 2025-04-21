#pragma once
//*******************************************************************************************************************
//*																													*
//*   File:       JAACfg.h																							*
//*   Suite:      Experimental Algorithms																			*
//*   Version:    1.0.0	(Build: 01)																					*
//*   Author:     Ian Tree/HMNL																						*
//*																													*
//*   Copyright 2017 - 2025 Ian J. Tree																				*
//*******************************************************************************************************************
//*	JAACfg																											*
//*																													*
//*	This header extends the xymorg AppConfig class to define the class that provides the singleton containing		*
//* all application configuration data plus the xymorg service access objects.										*
//*																													*
//*	USAGE:																											*
//*																													*
//*		The class definition must extend the xymorg::AppConfig class												*
//*																													*
//*	NOTES:																											*
//*																													*
//*	1.																												*
//*																													*
//*******************************************************************************************************************
//*	CONFIGURATION XML SPECIFICATION																					*
//*	---------------------------																						*
//*																													*
//*		<jaa cycles="n">																							*
//*			<images>d</images>																						*
//*			<baseimage>b</baseimage>																				*
//*		</jaa>																										*
//*																													*
//*			where n is the number of cycles of JPEG encoding to perform												*
//*			where d is the (virtual) directory for all images (input & generated).									*
//*			where b is the name of the base image to use															*
//*																													*
//*******************************************************************************************************************
//*	COMMAND LINE SPECIFICATION																						*
//*	--------------------------																						*
//*																													*
//*		JPEGAA <image directory> <base image name> -V -E -C:n														*
//*																													*
//*		Where -V or (-v)	==>	Verbose logging enabled																*
//*		-E or (-e)			==> Echo the log to the console															*
//*		-C:n or -c:n		==> Perform n cycles of jpeg encoding													*
//*																													*
//*******************************************************************************************************************
//*																													*
//*   History:																										*
//*																													*
//*	1.0.0 -		17/04/2025	-	Initial Release																		*
//*																													*
//*******************************************************************************************************************/

//  Include xymorg headers
#include	"../xymorg/xymorg.h"															//  xymorg system headers

//
//  JAACfg Class
//

class JAACfg : public xymorg::AppConfig {
public:

	//*******************************************************************************************************************
	//*                                                                                                                 *
	//*   Constructors			                                                                                        *
	//*                                                                                                                 *
	//*******************************************************************************************************************

	//  Constructor 
	//
	//  Constructs the application configuration object and loads the persistent settings from the config file and command line
	//
	//  PARAMETERS:
	//
	//		char *			-		Const pointer to the application name
	//		int				-		Count of application invocation parameters
	//		char*[]			-		Array of pointers to the application invocation parameters
	//
	//  RETURNS:
	//
	//  NOTES:
	//

	JAACfg(const char* szAppName, int argc, char* argv[])
		: xymorg::AppConfig(szAppName, argc, argv)
		, ConfigValid(false)
		, NumCycles(0)
		, RID(NULLSTRREF)
		, RBI(NULLSTRREF)
	{
		//  Handle any command line parameters
		if (handleCmdLine(argc, argv)) ConfigValid = true;

		//  Handle the local application configuration settings (if needed)
		if (!ConfigValid) {
			if (pCfgImg == nullptr) handleNoConfig();
			else handleConfig();
		}

		//  Release the configuration image
		releaseConfigImage();

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
	//  Destroys the JAACfg object, dismissing the underlying objects/allocations
	//
	//  PARAMETERS:
	//
	//  RETURNS:
	//
	//  NOTES:
	//  

	~JAACfg() {

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

	//  isValid
	//
	//  This function will return the state of the configuration
	//
	//	PARAMETERS:
	//
	//	RETURNS:
	//
	//		bool		-		The current validity state
	//
	//	NOTES:
	//

	bool	isValid() const { return ConfigValid; }

	//  getCycles
	//
	//  This function will return the number of encoding cycles requested
	//
	//	PARAMETERS:
	//
	//	RETURNS:
	//
	//		int		-		The requested number of cycles
	//
	//	NOTES:
	//

	int		getCycles() const { return NumCycles; }

	//  getImgDir
	//
	//  This function will return the name of the images directory
	//
	//	PARAMETERS:
	//
	//	RETURNS:
	//
	//		char*		-		Const pointer to the image directory name
	//
	//	NOTES:
	//

	const char*		getImgDir() { return SPool.getString(RID); }

	//  getBaseImage
	//
	//  This function will return the name of the base image to use
	//
	//	PARAMETERS:
	//
	//	RETURNS:
	//
	//		char*		-		Const pointer to the base image name
	//
	//	NOTES:
	//

	const char*		getBaseImage() { return SPool.getString(RBI); }

private:

	//*******************************************************************************************************************
	//*                                                                                                                 *
	//*   Private Members			                                                                                    *
	//*                                                                                                                 *
	//*******************************************************************************************************************

	bool				ConfigValid;												//  Validity state of configuration
	int					NumCycles;													//  Number of cycles to perform
	xymorg::STRREF		RID;														//  Reference to the images directory name
	xymorg::STRREF		RBI;														//  Reference to the base image name

	//*******************************************************************************************************************
	//*                                                                                                                 *
	//*   Private Functions                                                                                             *
	//*                                                                                                                 *
	//*******************************************************************************************************************

	//  handleCmdLine
	//
	//  This function will handle the parsing of parameters from the command line.
	//
	//  PARAMETERS:
	//
	//		int				-		Count of application invocation parameters
	//		char*[]			-		Array of pointers to the application invocation parameters
	//
	//  RETURNS:
	// 
	//		bool		-		if true then parsing of the configuration XML is not required
	//
	//  NOTES:
	//

	bool	handleCmdLine(int argc, char* argv[]) {
		int				FirstSwitch = 1;															//  First switch parameter 
		bool			Configured = false;															//  Fully configured indicator
		bool			SWValid = false;															//  Switch validity
		const char*		pScan = nullptr;															//  Scanning pointer
		char			TempFN[MAX_PATH] = {};														//  Temp file name

		//  No parameters are present on the command line - use the config XML file
		if (argc == 1) return false;

		//  If the first command line parameter is in use (xymorg project root directory) then bump the first switch
		if (isFirstCLPUsed()) {
			FirstSwitch = 2;
			if (argc == 2) return false;
		}

		//
		//  If the command line is being used then there should be AT least 3 parameters remaining to be processed
		//
		if ((argc - FirstSwitch) < 3) return false;

		//
		//  Extract the first two available parameters -
		// 

		if (argv[FirstSwitch][0] == '-') return false;
		RID = SPool.addString(argv[FirstSwitch]);
		FirstSwitch++;

		if (argv[FirstSwitch][0] == '-') return false;
		RBI = SPool.addString(argv[FirstSwitch]);
		FirstSwitch++;

		//  Check that the input image is a GIF
		pScan = SPool.getString(RBI);
		pScan += strlen(pScan);
		//  Scan backwards for the last '.' in the string
		while (*pScan != '.' && pScan > SPool.getString(RBI)) pScan--;
		if (*pScan != '.') {
			Log << "ERROR: The base image name <baseimage> name supplied is invalid." << std::endl;
			return false;
		}
		if (_strcmpi(pScan, ".gif") != 0) {
			Log << "ERROR: The base image name <baseimage> name supplied is unacceptable, it must be a GIF image." << std::endl;
			return false;
		}

		//  If the base image is just a file name (no directory) then prefix it with <images directory>/
		if (strchr(SPool.getString(RBI), '/') == nullptr && strchr(SPool.getString(RBI), '\\') == nullptr) {
			sprintf_s(TempFN, MAX_PATH, "%s/%s", SPool.getString(RID), SPool.getString(RBI));
			RBI = SPool.replaceString(RBI, TempFN);
		}


		//  Process each switch in turn
		for (int SWX = FirstSwitch; SWX < argc; SWX++) {
			SWValid = false;

			//  Test for logging verbosity asserted
			if (strlen(argv[SWX]) == 2) {
				if (_memicmp(argv[SWX], "-V", 2) == 0) {
					SWValid = true;
					setVerboseLogging(true);
				}
			}

			//  Test for log echoing
			if (strlen(argv[SWX]) == 2) {
				if (_memicmp(argv[SWX], "-E", 2) == 0) {
					SWValid = true;
					setEchoLogging(true);
				}
			}

			//  Test for number of trials
			if (strlen(argv[SWX]) > 3) {
				if (_memicmp(argv[SWX], "-C:", 3) == 0) {
					SWValid = true;
					NumCycles = atoi(argv[SWX] + 3);
					if (NumCycles < 1) {
						Log << "ERROR: The number of cycles specified on the command line -C:n, n MUST be greater than 0." << std::endl;
						return false;
					}
					else Configured = true;
				}
			}

			//  Invalid switch
			if (!SWValid) {
				Log << "ERROR: Command line parameter: '" << argv[SWX] << "' is invalid and has been ignored." << std::endl;
			}
		}

		ConfigValid = true;

		//  Return the configuration state
		return Configured;
	}

	//  handleNoConfig
	//
	//  This function is the handler for the "No Config Loaded" event. It will clear the validity flag.
	//
	//  PARAMETERS:
	//
	//  RETURNS:
	//
	//  NOTES:
	//

	void handleNoConfig() {

		//  Clear the validity flag
		ConfigValid = false;

		Log << "ERROR: Unable to load the application configuration." << std::endl;

		//  Return to caller
		return;
	}

	//  handleConfig
	//
	//  This function is the handler for the "Config Loaded" event. It will parse the application specific values from the 
	//  configuration file and set the validity indicator.
	//
	//  PARAMETERS:
	//
	//  RETURNS:
	//
	//  NOTES:
	//

	void	handleConfig() {
		xymorg::XMLMicroParser					CfgXML(pCfgImg);									//  XML Micro Parser for the application configuration file
		xymorg::XMLMicroParser::XMLIterator		JAANode = CfgXML.getScope("jaa");					//  Root definition node of the configuration
		const char*								pScan = nullptr;									//  Scanning pointer
		char									TempFN[MAX_PATH] = {};								//  Temp file name

		//  Safety/Validity
		ConfigValid = false;
		if (!CfgXML.isValid()) {
			Log << "ERROR: The configuration XML is not a valid XML document." << std::endl;
			return;
		}

		if (JAANode.isNull() || JAANode.isClosing()) {
			Log << "ERROR: There is no valid <jaa> node in the configuration XML document." << std::endl;
			return;
		}

		//  Extract the number of cycles to perform
		NumCycles = JAANode.getAttributeInt("cycles");

		//  Validate the number of trials
		if (NumCycles <= 0) {
			Log << "ERROR: The number of cycles (cycles=) attribute on the <jaa> node is invalid or missing." << std::endl;
			return;
		}

		//
		//  Get the images directory name (if present), if not present set it to "images"
		//

		RID = captureFilename(JAANode, "images");
		if (RID == NULLSTRREF) RID = SPool.addString("images");

		//
		//  Get the base image name
		//

		RBI = captureFilename(JAANode, "baseimage");
		if (RBI == NULLSTRREF) {
			Log << "ERROR: Unable to determine the base image name <baseimage> in the <jaa> section." << std::endl;
			return;
		}

		//  Check that the input image is a GIF
		pScan = SPool.getString(RBI);
		pScan += strlen(pScan);
		//  Scan backwards for the last '.' in the string
		while (*pScan != '.' && pScan > SPool.getString(RBI)) pScan--;
		if (*pScan != '.') {
			Log << "ERROR: The base image name <baseimage> name supplied is invalid." << std::endl;
			return;
		}
		if (_strcmpi(pScan, ".gif") != 0) {
			Log << "ERROR: The base image name <baseimage> name supplied is unacceptable, it must be a GIF image." << std::endl;
			return;
		}

		//  If the base image is just a file name (no directory) then prefix it with <images directory>/
		if (strchr(SPool.getString(RBI), '/') == nullptr && strchr(SPool.getString(RBI), '\\') == nullptr) {
			sprintf_s(TempFN, MAX_PATH, "%s/%s", SPool.getString(RID), SPool.getString(RBI));
			RBI = SPool.replaceString(RBI, TempFN);
		}

		//  Show configuration is valid
		ConfigValid = true;

		//  Return to caller
		return;
	}

	//  captureFilename
	//
	//  This function will capture the file name specified in the named section.
	//
	//  PARAMETERS:
	// 
	//		XMLIterator&		-		Reference to an XML iterator positioned to the sort section
	//		char*				-		The name of the section 
	//  RETURNS:
	// 
	//		xymorg::STRREF		-		String reference token for the captured filename
	//
	//  NOTES:
	//

	xymorg::STRREF	captureFilename(xymorg::XMLMicroParser::XMLIterator& SNode, const char* Section) {
		xymorg::XMLMicroParser::XMLIterator			FNode = SNode.getScope(Section);				//  Section Node iterator
		const char* pText = nullptr;								//  Pointer to the text
		size_t										TextLen = 0;									//  Text length

		if (FNode.isNull() || FNode.isAtEnd()) return NULLSTRREF;

		pText = FNode.getElementValue(TextLen);
		if (TextLen == 0) return NULLSTRREF;
		return SPool.addString(pText, TextLen);
	}

};
