#pragma once
//*******************************************************************************************************************
//*																													*
//*   File:       FontStyle.h																						*
//*   Suite:      xymorg Image Processing - primitives																*
//*   Version:    1.0.0	  Build:  01																				*
//*   Author:     Ian Tree/HMNL																						*
//*																													*
//*   Copyright 2017 - 2022 Ian J. Tree																				*
//*******************************************************************************************************************
//*	FontStyle.h																										*
//*																													*
//*	This header file contains the Class definition of the FontStyle class providing constant definitions and		*
//  prinitive functions for handling font selection and conditioning.												*
//*																													*
//*	NOTES:																											*
//*																													*
//*																													*
//*******************************************************************************************************************
//*																													*
//*   History:																										*
//*																													*
//*	1.0.0 - 07/12/2022   -  Initial version																			*
//*																													*
//*******************************************************************************************************************

//  Language and Platform headers
#include	"../LPBHdrs.h"
#include	"types.h"																//  Image processing primitive types

#if (defined(_WIN32) || defined(_WIN64))
//  Use Windows defined font weights
#else
//
//  Defines for widows equivalent font weights
//
#define		FW_DONTCARE		0
#define		FW_THIN			100
#define		FW_EXTRALIGHT	200
#define		FW_ULTRALIGHT	200
#define		FW_LIGHT		300
#define		FW_NORMAL		400
#define		FW_REGULAR		400
#define		FW_MEDIUM		500
#define		FW_SEMIBOLD		600
#define		FW_DEMIBOLD		600
#define		FW_BOLD			700
#define		FW_EXTRABOLD	800
#define		FW_ULTRBOLD		800
#define		FW_HEAVY		900
#define		FW_BLACK		900
#endif

//*******************************************************************************************************************
//*																											        *
//*   FontStyle Class																								*
//*                                                                                                                 *
//*******************************************************************************************************************

namespace xymorg {

	class FontStyle {
	public:

		//  Prevent instantiation
		FontStyle() = delete;

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Public Constants																								*
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Style Bit Array																								*
		//*                                                                                                                 *
		//*******************************************************************************************************************

		static const SWITCHES	SERIF			= 0x00000001;							//  Serifed typeface
		static const SWITCHES	SANS			= 0x00000002;							//  SanSerifed typeface
		static const SWITCHES	MONOSPACE		= 0x00000004;							//  Monospaced typeface
		static const SWITCHES	CONDENSED		= 0x00000008;							//  Condensed typeface
		static const SWITCHES	ITALIC			= 0x00000010;							//  Italic typeface
		static const SWITCHES	UNDERLINED		= 0x00000020;							//  Underlined typeface
		static const SWITCHES	STRIKETHROUGH	= 0x00000040;							//  Strikethrough typeface
		static const SWITCHES	STYLES			= 0x000000FF;							//  Font Face Style bits

		//  Font Weights

		static const SWITCHES	LIGHT			= 0x00000100;							//  Lightweight font
		static const SWITCHES	NORMAL			= 0x00000200;							//  Normal weight font
		static const SWITCHES	BOLD			= 0x00000400;							//  Bold weight font
		static const SWITCHES	XBOLD			= 0x00000800;							//  Extra Bold weight font
		static const SWITCHES	WEIGHTS			= 0x00000F00;							//  Font weight bits

		//  Font Condition Controls

		static const SWITCHES	NO_ANTIALIAS	= 0x00001000;							//  Do NOT antialias glyphs

		//  Style matching

		static const SWITCHES	MATCHING		= 0x0000FFFC;							//  Matching styles

		//  Text Box Styles
		static const SWITCHES	BOX_V270		= 0x00010000;							//  Vertical box for LH side
		static const SWITCHES	BOX_VLEFT		= BOX_V270;								//  Vertical box for LH side
		static const SWITCHES	BOX_V90			= 0x00010000;							//  Vertical box for RH side
		static const SWITCHES	BOX_VRIGHT		= BOX_V90;								//  Vertical box for RH side

		//  Font Capabilities

		static const SWITCHES	SCALABLE		= 0x00100000;							//  Scalable Font
		static const SWITCHES	KERNING			= 0x00200000;							//  Kerning is available for this font

		//  Search/Condition Controls
		static const SWITCHES	ANY_FACE		= 0x01000000;							//  Search for any face with matching styles
		static const SWITCHES	TYPEFACE		= 0x02000000;							//  Search for typeface name
		static const SWITCHES	FONTNAME		= 0x04000000;							//  Search for font name
		static const SWITCHES	FONTFILE		= 0x08000000;							//  Search for specific font file name

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Additional Constants																							*
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  WINDOWS Interoperability additional spacing control
		static const bool		WINTEROP_EXTRA_SPACE = false;

		//  NON-AntiAliased GreyScale Cutoff
		static const BYTE		NOAA_GS_CUTOFF = 127;

		//  Image Screen Resolution - 96 dpi
		static const size_t		ISRes = 96;

		//  Anti-Alias Colour Quantum   20/255
		static const double		AA_COL_QUANTUM;

		//  Maxima and Minima
		static const int		MaxFaceName = 128;										//  Maximum length of a face name (qualified)

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Public Functions			  	                                                                                *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//  pointsToPixels
		//
		//  This function will convert points to pixels
		//
		//  PARAMETERS
		//
		//		size_t			-		Points
		//
		//  RETURNS
		//
		//		size_t			-		Pixels
		//
		//  NOTES
		//

		static size_t	pointsToPixels(size_t Points) {
			size_t		Pixels = 0;
			double		Pts = double(Points);

			//  Default scaling
			Pts = Pts * 1.3333;

			//  Scale for image resolution
			Pts = Pts * 96.0;
			Pts = Pts / double(ISRes);

			Pts = round(Pts);

			// Convert to Pixels
			Pixels = size_t(Pts);

			//  Return the pixel size
			return Pixels;
		}

		//  stylesMatch
		//
		//  This function will determine if two styles match
		//
		//  PARAMETERS
		//
		//		SWITCHES			-		The requested Font Styles (See FontStyle::*)
		//		SWITCHES			-		The comparator Font Styles (See FontStyle::*)
		//
		//  RETURNS
		//
		//		bool				-		True if the two styles match, otherwise false
		//
		//  NOTES
		//

		static bool		stylesMatch(SWITCHES RS, SWITCHES CS) {
			if ((RS & MATCHING) == (CS & MATCHING)) return true;
			return false;
		}

		//  getWeight
		//
		//  This function will return a Font Weight (see FW_*) for the passed style
		//
		//  PARAMETERS
		//
		//		SWITCHES			-		The requested Font Styles (See FontStyle::*)
		//
		//  RETURNS
		//
		//		int					-		Font Weight
		//
		//  NOTES
		// 
		//		1.	If multiple font weights are encoded in the style array then the finest is returned
		//

		static int	getWeight(SWITCHES AFStyle) {
			if (AFStyle & LIGHT) return FW_LIGHT;
			if (AFStyle & NORMAL) return FW_NORMAL;
			if (AFStyle & BOLD) return FW_BOLD;
			if (AFStyle & XBOLD) return FW_EXTRABOLD;
			return FW_NORMAL;
		}

		//  setWeight
		//
		//  This function will encode the passed Font Weight into the passed set of Style bits and return the completed style 
		//
		//  PARAMETERS
		//
		//		size_t				-		Font Weight
		//		SWITCHES			-		The requested Font Styles (See FontStyle::*)
		//
		//  RETURNS
		//
		//		SWITCHES			-		The requested Font Styles (See FontStyle::*)
		//
		//  NOTES
		//

		static SWITCHES		setWeight(size_t Weight, SWITCHES Style) {
			SWITCHES		NewStyle = Style;

			NewStyle = NewStyle & ~WEIGHTS;

			if (Weight <= FW_LIGHT) NewStyle = NewStyle | LIGHT;
			else if (Weight <= FW_NORMAL) NewStyle = NewStyle | NORMAL;
			else if (Weight <= FW_BOLD) NewStyle = NewStyle | BOLD;
			else NewStyle = NewStyle | XBOLD;
			return NewStyle;
		}

		//  characteriseFont
		//
		//  This function will resolve a font name into a typeface name and a set of attributes.
		//
		//  PARAMETERS
		//
		//		char*				-		Const pointer to the Font/Face Name.
		//		char*				-		Pointer to the buffer for the face name
		//		size_t				-		Size of the face name buffer
		//		SWITCHES&			-		Reference to the Requested Font Styles (See FontStyle::*)
		//
		//  RETURNS
		//
		//		SWITCHES			-		The font style attributes (See FontStyle::*)
		//
		//  NOTES
		//

		static	SWITCHES	characteriseFont(const char* FName, char* Face, size_t FaceBfrLen, SWITCHES& ReqStyle) {
			SWITCHES		FStyle = 0;															//  Accumulated Font/Face Attributes
			const char*		pEOFace = nullptr;													//  End of the face name
			const char*		pAttr = nullptr;													//  Attribute pointer
			char*			pFace = Face;														//  Pointer into the face
			size_t			FLen = 0;															//  Length of returned face
			char			Font[MaxFaceName + 1] = {};											//  Face name

			//  Safety
			if (FName == nullptr) return 0;

			//  Copy the input font/face name applying aliases
			copyFaceName(FName, Font);
			pEOFace = Font + strlen(Font);

			//  Search for known attribute strings in the font name
			pAttr = strstr(Font, "Mono");
			if (pAttr != nullptr) {
				if (pAttr < pEOFace && pAttr > Font) pEOFace = pAttr;
				FStyle = FStyle | MONOSPACE;
			}

			pAttr = strstr(Font, "Sans");
			if (pAttr != nullptr) {
				if (pAttr < pEOFace && pAttr > Font) pEOFace = pAttr;
				FStyle = FStyle | SANS;
			}
			else {
				pAttr = strstr(Font, "Serif");
				if (pAttr != nullptr) {
					if (pAttr < pEOFace && pAttr > Font) pEOFace = pAttr;
					FStyle = FStyle | SERIF;
				}
			}

			pAttr = strstr(Font, "Black");
			if (pAttr != nullptr) {
				if (pAttr < pEOFace && pAttr > Font) pEOFace = pAttr;
				FStyle = FStyle | XBOLD;
			}

			pAttr = strstr(Font, "UltraBold");
			if (pAttr != nullptr) {
				if (pAttr < pEOFace && pAttr > Font) pEOFace = pAttr;
				FStyle = FStyle | XBOLD;
			}

			pAttr = strstr(Font, "ExtraBold");
			if (pAttr != nullptr) {
				if (pAttr < pEOFace && pAttr > Font) pEOFace = pAttr;
				FStyle = FStyle | XBOLD;
			}

			if (!(FStyle & XBOLD)) {
				pAttr = strstr(Font, "Bold");
				if (pAttr != nullptr) {
					if (pAttr < pEOFace && pAttr > Font) pEOFace = pAttr;
					FStyle = FStyle | BOLD;
				}
			}

			pAttr = strstr(Font, "ExtraLight");
			if (pAttr != nullptr) {
				if (pAttr < pEOFace && pAttr > Font) pEOFace = pAttr;
				FStyle = FStyle | LIGHT;
			}
			else {
				pAttr = strstr(Font, "Light");
				if (pAttr != nullptr) {
					if (pAttr < pEOFace && pAttr > Font) pEOFace = pAttr;
					FStyle = FStyle | LIGHT;
				}
			}

			pAttr = strstr(Font, "Condensed");
			if (pAttr != nullptr) {
				if (pAttr < pEOFace && pAttr > Font) pEOFace = pAttr;
				FStyle = FStyle | CONDENSED;
			}

			pAttr = strstr(Font, "Italic");
			if (pAttr != nullptr) {
				if (pAttr < pEOFace && pAttr > Font) pEOFace = pAttr;
				FStyle = FStyle | ITALIC;
			}

			pAttr = strstr(Font, "Oblique");
			if (pAttr != nullptr) {
				if (pAttr < pEOFace && pAttr > Font) pEOFace = pAttr;
				FStyle = FStyle | ITALIC;
			}

			//
			//  Return the face name if required
			//
			if (pFace != nullptr) {
				pAttr = Font;
				FLen = 0;
				while (FLen < (FaceBfrLen - 1) && pAttr < pEOFace && *pAttr != '.') {
					if (*pAttr > ' ') {
						*pFace = *pAttr;
						pFace++;
						FLen++;
					}
					pAttr++;
				}
				*pFace = '\0';
			}

			//  Well-Known Font Styles
			FStyle = FStyle | getWellKnownStyles(Face);

			//  If no style cues were found then apply any requested styles
			if ((FStyle & STYLES) == 0) FStyle = FStyle | (ReqStyle & STYLES);

			//  If no font weight cues were found then apply any requested weights or default to NORMAL
			if ((FStyle & WEIGHTS) == 0) FStyle = FStyle | (ReqStyle & WEIGHTS);
			if ((FStyle & WEIGHTS) == 0) FStyle = FStyle | NORMAL;

			//  Return the discovered style
			return FStyle;
		}
		
	private:
		
		//  getWellKnownStyles
		//
		//  Returns a set of well known style cues for recognised fonts
		//
		//  PARAMETERS
		// 
		//		char*			-		Const pointer to the Font Face name
		//
		//  RETURNS
		// 
		//		SWITCHES		-		Font Style cues (See STYLE_*)
		// 
		//  NOTES
		//

		static SWITCHES getWellKnownStyles(const char* Face) {

			//  Times faces are Serif
			if (_stricmp(Face, "Times") == 0) return SERIF;

			//  Arial faces are SansSerif
			if (_stricmp(Face, "Arial") == 0) return SANS;

			//  No hint
			return 0;
		}

		//  copyFaceName
		//
		//  Copies the face name from input to output applying aliases and normalisations
		//
		//  PARAMETERS
		// 
		//		char*			-		Const pointer to the Input Font Face name
		//		char*			-		Pointer to the Output Font Face name
		//
		//  RETURNS
		// 
		//  NOTES
		//

		static void copyFaceName(const char* InFace, char* OutFace) {
			const char* pIn = InFace;
			char* pOut = OutFace;

			//  Safety
			if (OutFace != nullptr) *OutFace = '\0';
			if (InFace == nullptr || OutFace == nullptr) return;

			//  Copy in to out removing blanks and terminating at '.' or End-Of-String
			while (*pIn != '\0' && *pIn != '.') {
				if (*pIn > ' ') {
					*pOut = *pIn;
					pOut++;
				}
				pIn++;
			}
			*pOut = '\0';

			//  Microsoft Core Fonts
			//  1.	Andale
			if (_stricmp(OutFace, "Andalemo") == 0) {
				strcpy_s(OutFace, MaxFaceName, "AndaleMonoSans");
				return;
			}

			//  2.  Arial
			if (_stricmp(OutFace, "Arial") == 0) {
				strcpy_s(OutFace, MaxFaceName, "ArialSans");
				return;
			}
			if (_stricmp(OutFace, "Arialbd") == 0) {
				strcpy_s(OutFace, MaxFaceName, "ArialSansBold");
				return;
			}
			if (_stricmp(OutFace, "Arialbi") == 0) {
				strcpy_s(OutFace, MaxFaceName, "ArialSansBoldItalic");
				return;
			}
			if (_stricmp(OutFace, "Ariali") == 0) {
				strcpy_s(OutFace, MaxFaceName, "ArialSansItalic");
				return;
			}
			if (_stricmp(OutFace, "Ariblk") == 0) {
				strcpy_s(OutFace, MaxFaceName, "ArialExtraBold");
				return;
			}

			//  3. Microsoft Comic 
			if (_stricmp(OutFace, "Comic") == 0) {
				strcpy_s(OutFace, MaxFaceName, "ComicSans");
				return;
			}
			if (_stricmp(OutFace, "Comicbd") == 0) {
				strcpy_s(OutFace, MaxFaceName, "ComicSansBold");
				return;
			}

			//  4.  Courier
			if (_stricmp(OutFace, "Cour") == 0) {
				strcpy_s(OutFace, MaxFaceName, "CourierMonoSans");
				return;
			}
			if (_stricmp(OutFace, "Courbd") == 0) {
				strcpy_s(OutFace, MaxFaceName, "CourierMonoSansBold");
				return;
			}
			if (_stricmp(OutFace, "Courbi") == 0) {
				strcpy_s(OutFace, MaxFaceName, "CourierMonoSansBoldItalic");
				return;
			}
			if (_stricmp(OutFace, "Couri") == 0) {
				strcpy_s(OutFace, MaxFaceName, "CourierMonoSansItalic");
				return;
			}

			//  5.  Georgia
			if (_stricmp(OutFace, "Georgia") == 0) {
				strcpy_s(OutFace, MaxFaceName, "GeorgiaSerif");
				return;
			}
			if (_stricmp(OutFace, "Georgiab") == 0) {
				strcpy_s(OutFace, MaxFaceName, "GeorgiaSerifBold");
				return;
			}
			if (_stricmp(OutFace, "Georgiai") == 0) {
				strcpy_s(OutFace, MaxFaceName, "GeorgiaSerifItalic");
				return;
			}
			if (_stricmp(OutFace, "Georgiaz") == 0) {
				strcpy_s(OutFace, MaxFaceName, "GeorgiaSerifBoldItalic");
				return;
			}

			//  6.  Times New Roman
			if (_stricmp(OutFace, "Times") == 0) {
				strcpy_s(OutFace, MaxFaceName, "TimesNewRomanSerif");
				return;
			}
			if (_stricmp(OutFace, "Timesbd") == 0) {
				strcpy_s(OutFace, MaxFaceName, "TimesNewRomanSerifBold");
				return;
			}
			if (_stricmp(OutFace, "Timesbi") == 0) {
				strcpy_s(OutFace, MaxFaceName, "TimesNewRomanSerifBoldItalic");
				return;
			}
			if (_stricmp(OutFace, "Timesi") == 0) {
				strcpy_s(OutFace, MaxFaceName, "TimesNewRomanSerifItalic");
				return;
			}

			//  7.  Trebuchet
			if (_stricmp(OutFace, "Trebuc") == 0) {
				strcpy_s(OutFace, MaxFaceName, "TrebuchetSans");
				return;
			}
			if (_stricmp(OutFace, "Trebucbd") == 0) {
				strcpy_s(OutFace, MaxFaceName, "TrebuchetSansBold");
				return;
			}
			if (_stricmp(OutFace, "Trebucbi") == 0) {
				strcpy_s(OutFace, MaxFaceName, "TrebuchetSansBoldItalic");
				return;
			}
			if (_stricmp(OutFace, "Trebucit") == 0) {
				strcpy_s(OutFace, MaxFaceName, "TrebuchetSansItalic");
				return;
			}

			//  8.  Verdana
			if (_stricmp(OutFace, "Verdana") == 0) {
				strcpy_s(OutFace, MaxFaceName, "VerdanaSans");
				return;
			}
			if (_stricmp(OutFace, "Verdanab") == 0) {
				strcpy_s(OutFace, MaxFaceName, "VerdanaSansBold");
				return;
			}
			if (_stricmp(OutFace, "Verdanai") == 0) {
				strcpy_s(OutFace, MaxFaceName, "VerdanaSansItalic");
				return;
			}
			if (_stricmp(OutFace, "Verdanaz") == 0) {
				strcpy_s(OutFace, MaxFaceName, "VerdanaSansBoldItalic");
				return;
			}

			//  Return no change
			return;
		}

	};

	//
	//  Static initialisers
	//

	const double	xymorg::FontStyle::AA_COL_QUANTUM = 20.0;
}

