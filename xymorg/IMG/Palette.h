#pragma once
//*******************************************************************************************************************
//*																													*
//*   File:       TextBox.h																							*
//*   Suite:      xymorg Image Processing - primitives																*
//*   Version:    1.0.0	  Build:  01																				*
//*   Author:     Ian Tree/HMNL																						*
//*																													*
//*   Copyright 2017 - 2021 Ian J. Tree																				*
//*******************************************************************************************************************
//*	Palette.h																										*
//*																													*
//*	This header file contains the Class definition of the Palette class is a container for classes that hold		*
//  static constant colour definitions.																				*
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

#include	"../types.h"																			//  xymorg base types
#include	"types.h"																				//  Image processing primitive types

//*******************************************************************************************************************
//*																											        *
//*   Palette Class																									*
//*                                                                                                                 *
//*   The Palette class is a container for different Palettes.														*
//*                                                                                                                 *
//*******************************************************************************************************************

namespace xymorg {

	class Palette {
	public:

		//  Prevent instantiation
		Palette() = delete;

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Public Nested Classes - Palettes                                                                              *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Web - HTML/CSS colour palette		                                                                            *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		class Web {
		public:

			//  Prevent instantiation
			Web() = delete;

			//*******************************************************************************************************************
			//*                                                                                                                 *
			//*   Public Constants - colours                                                                                    *
			//*                                                                                                                 *
			//*******************************************************************************************************************

			//  Basic colours
			static const RGB		Black;
			static const RGB		White;
			static const RGB		Red;
			static const RGB		Lime;
			static const RGB		Blue;
			static const RGB		Yellow;
			static const RGB		Cyan;
			static const RGB		Aqua;
			static const RGB		Magenta;
			static const RGB		Fuchsia;
			static const RGB		Silver;
			static const RGB		Gray;
			static const RGB		Maroon;
			static const RGB		Olive;
			static const RGB		Green;
			static const RGB		Purple;
			static const RGB		Teal;
			static const RGB		Navy;

			//  Extended colours
			static const RGB		maroon;
			static const RGB		dark_red;
			static const RGB		brown;
			static const RGB		firebrick;
			static const RGB		crimson;
			static const RGB		red;
			static const RGB		tomato;
			static const RGB		coral;
			static const RGB		indian_red;
			static const RGB		light_coral;
			static const RGB		dark_salmon;
			static const RGB		salmon;
			static const RGB		light_salmon;
			static const RGB		orange_red;
			static const RGB		dark_orange;
			static const RGB		orange;
			static const RGB		gold;
			static const RGB		dark_golden_rod;
			static const RGB		golden_rod;
			static const RGB		pale_golden_rod;
			static const RGB		dark_khaki;
			static const RGB		khaki;
			static const RGB		olive;
			static const RGB		yellow;
			static const RGB		yellow_green;
			static const RGB		dark_olive_green;
			static const RGB		olive_drab;
			static const RGB		lawn_green;
			static const RGB		chartreuse;
			static const RGB		green_yellow;
			static const RGB		dark_green;
			static const RGB		green;
			static const RGB		forest_green;
			static const RGB		lime;
			static const RGB		lime_green;
			static const RGB		light_green;
			static const RGB		pale_green;
			static const RGB		dark_sea_green;
			static const RGB		medium_spring_green;
			static const RGB		spring_green;
			static const RGB		sea_green;
			static const RGB		medium_aqua_marine;
			static const RGB		medium_sea_green;
			static const RGB		light_sea_green;
			static const RGB		dark_slate_gray;
			static const RGB		teal;
			static const RGB		dark_cyan;
			static const RGB		aqua;
			static const RGB		cyan;
			static const RGB		light_cyan;
			static const RGB		dark_turquoise;
			static const RGB		turquoise;
			static const RGB		medium_turquoise;
			static const RGB		pale_turquoise;
			static const RGB		aqua_marine;
			static const RGB		powder_blue;
			static const RGB		cadet_blue;
			static const RGB		steel_blue;
			static const RGB		corn_flower_blue;
			static const RGB		deep_sky_blue;
			static const RGB		dodger_blue;
			static const RGB		light_blue;
			static const RGB		sky_blue;
			static const RGB		light_sky_blue;
			static const RGB		midnight_blue;
			static const RGB		navy;
			static const RGB		dark_blue;
			static const RGB		medium_blue;
			static const RGB		blue;
			static const RGB		royal_blue;
			static const RGB		blue_violet;
			static const RGB		indigo;
			static const RGB		dark_slate_blue;
			static const RGB		slate_blue;
			static const RGB		medium_slate_blue;
			static const RGB		medium_purple;
			static const RGB		dark_magenta;
			static const RGB		dark_violet;
			static const RGB		dark_orchid;
			static const RGB		medium_orchid;
			static const RGB		purple;
			static const RGB		thistle;
			static const RGB		plum;
			static const RGB		violet;
			static const RGB		magenta;
			static const RGB		fuchsia;
			static const RGB		orchid;
			static const RGB		medium_violet_red;
			static const RGB		pale_violet_red;
			static const RGB		deep_pink;
			static const RGB		hot_pink;
			static const RGB		light_pink;
			static const RGB		pink;
			static const RGB		antique_white;
			static const RGB		beige;
			static const RGB		bisque;
			static const RGB		blanched_almond;
			static const RGB		wheat;
			static const RGB		corn_silk;
			static const RGB		lemon_chiffon;
			static const RGB		light_golden_rod_yellow;
			static const RGB		light_yellow;
			static const RGB		saddle_brown;
			static const RGB		sienna;
			static const RGB		chocolate;
			static const RGB		peru;
			static const RGB		sandy_brown;
			static const RGB		burly_wood;
			static const RGB		tan;
			static const RGB		rosy_brown;
			static const RGB		moccasin;
			static const RGB		navajo_white;
			static const RGB		peach_puff;
			static const RGB		misty_rose;
			static const RGB		lavender_blush;
			static const RGB		linen;
			static const RGB		old_lace;
			static const RGB		papaya_whip;
			static const RGB		sea_shell;
			static const RGB		mint_cream;
			static const RGB		slate_gray;
			static const RGB		light_slate_gray;
			static const RGB		light_steel_blue;
			static const RGB		lavender;
			static const RGB		floral_white;
			static const RGB		alice_blue;
			static const RGB		ghost_white;
			static const RGB		honeydew;
			static const RGB		ivory;
			static const RGB		azure;
			static const RGB		snow;
			static const RGB		black;
			static const RGB		dim_gray;
			static const RGB		dim_grey;
			static const RGB		gray;
			static const RGB		grey;
			static const RGB		dark_gray;
			static const RGB		dark_grey;
			static const RGB		silver;
			static const RGB		light_gray;
			static const RGB		light_grey;
			static const RGB		gainsboro;
			static const RGB		white_smoke;
			static const RGB		white;

		};

		//*******************************************************************************************************************
		//*                                                                                                                 *
		//*   Stark - High contrast colour palette for black background                                                     *
		//*                                                                                                                 *
		//*******************************************************************************************************************

		class Stark {
		public:

			//  Prevent instantiation
			Stark() = delete;

			//*******************************************************************************************************************
			//*                                                                                                                 *
			//*   Public Constants - colours                                                                                    *
			//*                                                                                                                 *
			//*******************************************************************************************************************

			static const RGB		Black;
			static const RGB		Blue;
			static const RGB		Orange;
			static const RGB		White;
			static const RGB		Yellow;
			static const RGB		Mauve;
			static const RGB		Green;
			static const RGB		Pastel_Green;
			static const RGB		Red;

		};

	};


}

//  Web Palette - Basic Colours
const xymorg::RGB xymorg::Palette::Web::Black = { 0,0,0 };
const xymorg::RGB xymorg::Palette::Web::White = { 255,255,255 };
const xymorg::RGB xymorg::Palette::Web::Red = { 255,0,0 };
const xymorg::RGB xymorg::Palette::Web::Lime = { 0,255,0 };
const xymorg::RGB xymorg::Palette::Web::Blue = { 0,0,255 };
const xymorg::RGB xymorg::Palette::Web::Yellow = { 255,255,0 };
const xymorg::RGB xymorg::Palette::Web::Cyan = { 0,255,255 };
const xymorg::RGB xymorg::Palette::Web::Aqua = { 0,255,255 };
const xymorg::RGB xymorg::Palette::Web::Magenta = { 255,0,255 };
const xymorg::RGB xymorg::Palette::Web::Fuchsia = { 255,0,255 };
const xymorg::RGB xymorg::Palette::Web::Silver = { 192,192,192 };
const xymorg::RGB xymorg::Palette::Web::Gray = { 128,128,128 };
const xymorg::RGB xymorg::Palette::Web::Maroon = { 128,0,0 };
const xymorg::RGB xymorg::Palette::Web::Olive = { 128,128,0 };
const xymorg::RGB xymorg::Palette::Web::Green = { 0,128,0 };
const xymorg::RGB xymorg::Palette::Web::Purple = { 128,0,128 };
const xymorg::RGB xymorg::Palette::Web::Teal = { 0,128,128 };
const xymorg::RGB xymorg::Palette::Web::Navy = { 0,0,128 };

//  Web Palette - Extended Colours
const xymorg::RGB xymorg::Palette::Web::maroon = { 128,0,0 };
const xymorg::RGB xymorg::Palette::Web::dark_red = { 139,0,0 };
const xymorg::RGB xymorg::Palette::Web::brown = { 165,42,42 };
const xymorg::RGB xymorg::Palette::Web::firebrick = { 178,34,34 };
const xymorg::RGB xymorg::Palette::Web::crimson = { 220,20,60 };
const xymorg::RGB xymorg::Palette::Web::red = { 255,0,0 };
const xymorg::RGB xymorg::Palette::Web::tomato = { 255,99,71 };
const xymorg::RGB xymorg::Palette::Web::coral = { 255,127,80 };
const xymorg::RGB xymorg::Palette::Web::indian_red = { 205,92,92 };
const xymorg::RGB xymorg::Palette::Web::light_coral = { 240,128,128 };
const xymorg::RGB xymorg::Palette::Web::dark_salmon = { 233,150,122 };
const xymorg::RGB xymorg::Palette::Web::salmon = { 250,128,114 };
const xymorg::RGB xymorg::Palette::Web::light_salmon = { 255,160,122 };
const xymorg::RGB xymorg::Palette::Web::orange_red = { 255,69,0 };
const xymorg::RGB xymorg::Palette::Web::dark_orange = { 255,140,0 };
const xymorg::RGB xymorg::Palette::Web::orange = { 255,165,0 };
const xymorg::RGB xymorg::Palette::Web::gold = { 255,215,0 };
const xymorg::RGB xymorg::Palette::Web::dark_golden_rod = { 184,134,11 };
const xymorg::RGB xymorg::Palette::Web::golden_rod = { 218,165,32 };
const xymorg::RGB xymorg::Palette::Web::pale_golden_rod = { 238,232,170 };
const xymorg::RGB xymorg::Palette::Web::dark_khaki = { 189,183,107 };
const xymorg::RGB xymorg::Palette::Web::khaki = { 240,230,140 };
const xymorg::RGB xymorg::Palette::Web::olive = { 128,128,0 };
const xymorg::RGB xymorg::Palette::Web::yellow = { 255,255,0 };
const xymorg::RGB xymorg::Palette::Web::yellow_green = { 154,205,50 };
const xymorg::RGB xymorg::Palette::Web::dark_olive_green = { 85,107,47 };
const xymorg::RGB xymorg::Palette::Web::olive_drab = { 107,142,35 };
const xymorg::RGB xymorg::Palette::Web::lawn_green = { 124,252,0 };
const xymorg::RGB xymorg::Palette::Web::chartreuse = { 127,255,0 };
const xymorg::RGB xymorg::Palette::Web::green_yellow = { 173,255,47 };
const xymorg::RGB xymorg::Palette::Web::dark_green = { 0,100,0 };
const xymorg::RGB xymorg::Palette::Web::green = { 0,128,0 };
const xymorg::RGB xymorg::Palette::Web::forest_green = { 34,139,34 };
const xymorg::RGB xymorg::Palette::Web::lime = { 0,255,0 };
const xymorg::RGB xymorg::Palette::Web::lime_green = { 50,205,50 };
const xymorg::RGB xymorg::Palette::Web::light_green = { 144,238,144 };
const xymorg::RGB xymorg::Palette::Web::pale_green = { 152,251,152 };
const xymorg::RGB xymorg::Palette::Web::dark_sea_green = { 143,188,143 };
const xymorg::RGB xymorg::Palette::Web::medium_spring_green = { 0,250,154 };
const xymorg::RGB xymorg::Palette::Web::spring_green = { 0,255,127 };
const xymorg::RGB xymorg::Palette::Web::sea_green = { 46,139,87 };
const xymorg::RGB xymorg::Palette::Web::medium_aqua_marine = { 102,205,170 };
const xymorg::RGB xymorg::Palette::Web::medium_sea_green = { 60,179,113 };
const xymorg::RGB xymorg::Palette::Web::light_sea_green = { 32,178,170 };
const xymorg::RGB xymorg::Palette::Web::dark_slate_gray = { 47,79,79 };
const xymorg::RGB xymorg::Palette::Web::teal = { 0,128,128 };
const xymorg::RGB xymorg::Palette::Web::dark_cyan = { 0,139,139 };
const xymorg::RGB xymorg::Palette::Web::aqua = { 0,255,255 };
const xymorg::RGB xymorg::Palette::Web::cyan = { 0,255,255 };
const xymorg::RGB xymorg::Palette::Web::light_cyan = { 224,255,255 };
const xymorg::RGB xymorg::Palette::Web::dark_turquoise = { 0,206,209 };
const xymorg::RGB xymorg::Palette::Web::turquoise = { 64,224,208 };
const xymorg::RGB xymorg::Palette::Web::medium_turquoise = { 72,209,204 };
const xymorg::RGB xymorg::Palette::Web::pale_turquoise = { 175,238,238 };
const xymorg::RGB xymorg::Palette::Web::aqua_marine = { 127,255,212 };
const xymorg::RGB xymorg::Palette::Web::powder_blue = { 176,224,230 };
const xymorg::RGB xymorg::Palette::Web::cadet_blue = { 95,158,160 };
const xymorg::RGB xymorg::Palette::Web::steel_blue = { 70,130,180 };
const xymorg::RGB xymorg::Palette::Web::corn_flower_blue = { 100,149,237 };
const xymorg::RGB xymorg::Palette::Web::deep_sky_blue = { 0,191,255 };
const xymorg::RGB xymorg::Palette::Web::dodger_blue = { 30,144,255 };
const xymorg::RGB xymorg::Palette::Web::light_blue = { 173,216,230 };
const xymorg::RGB xymorg::Palette::Web::sky_blue = { 135,206,235 };
const xymorg::RGB xymorg::Palette::Web::light_sky_blue = { 135,206,250 };
const xymorg::RGB xymorg::Palette::Web::midnight_blue = { 25,25,112 };
const xymorg::RGB xymorg::Palette::Web::navy = { 0,0,128 };
const xymorg::RGB xymorg::Palette::Web::dark_blue = { 0,0,139 };
const xymorg::RGB xymorg::Palette::Web::medium_blue = { 0,0,205 };
const xymorg::RGB xymorg::Palette::Web::blue = { 0,0,255 };
const xymorg::RGB xymorg::Palette::Web::royal_blue = { 65,105,225 };
const xymorg::RGB xymorg::Palette::Web::blue_violet = { 138,43,226 };
const xymorg::RGB xymorg::Palette::Web::indigo = { 75,0,130 };
const xymorg::RGB xymorg::Palette::Web::dark_slate_blue = { 72,61,139 };
const xymorg::RGB xymorg::Palette::Web::slate_blue = { 106,90,205 };
const xymorg::RGB xymorg::Palette::Web::medium_slate_blue = { 123,104,238 };
const xymorg::RGB xymorg::Palette::Web::medium_purple = { 147,112,219 };
const xymorg::RGB xymorg::Palette::Web::dark_magenta = { 139,0,139 };
const xymorg::RGB xymorg::Palette::Web::dark_violet = { 148,0,211 };
const xymorg::RGB xymorg::Palette::Web::dark_orchid = { 153,50,204 };
const xymorg::RGB xymorg::Palette::Web::medium_orchid = { 186,85,211 };
const xymorg::RGB xymorg::Palette::Web::purple = { 128,0,128 };
const xymorg::RGB xymorg::Palette::Web::thistle = { 216,191,216 };
const xymorg::RGB xymorg::Palette::Web::plum = { 221,160,221 };
const xymorg::RGB xymorg::Palette::Web::violet = { 238,130,238 };
const xymorg::RGB xymorg::Palette::Web::magenta = { 255,0,255 };
const xymorg::RGB xymorg::Palette::Web::fuchsia = { 255,0,255 };
const xymorg::RGB xymorg::Palette::Web::orchid = { 218,112,214 };
const xymorg::RGB xymorg::Palette::Web::medium_violet_red = { 199,21,133 };
const xymorg::RGB xymorg::Palette::Web::pale_violet_red = { 219,112,147 };
const xymorg::RGB xymorg::Palette::Web::deep_pink = { 255,20,147 };
const xymorg::RGB xymorg::Palette::Web::hot_pink = { 255,105,180 };
const xymorg::RGB xymorg::Palette::Web::light_pink = { 255,182,193 };
const xymorg::RGB xymorg::Palette::Web::pink = { 255,192,203 };
const xymorg::RGB xymorg::Palette::Web::antique_white = { 250,235,215 };
const xymorg::RGB xymorg::Palette::Web::beige = { 245,245,220 };
const xymorg::RGB xymorg::Palette::Web::bisque = { 255,228,196 };
const xymorg::RGB xymorg::Palette::Web::blanched_almond = { 255,235,205 };
const xymorg::RGB xymorg::Palette::Web::wheat = { 245,222,179 };
const xymorg::RGB xymorg::Palette::Web::corn_silk = { 255,248,220 };
const xymorg::RGB xymorg::Palette::Web::lemon_chiffon = { 255,250,205 };
const xymorg::RGB xymorg::Palette::Web::light_golden_rod_yellow = { 250,250,210 };
const xymorg::RGB xymorg::Palette::Web::light_yellow = { 255,255,224 };
const xymorg::RGB xymorg::Palette::Web::saddle_brown = { 139,69,19 };
const xymorg::RGB xymorg::Palette::Web::sienna = { 160,82,45 };
const xymorg::RGB xymorg::Palette::Web::chocolate = { 210,105,30 };
const xymorg::RGB xymorg::Palette::Web::peru = { 205,133,63 };
const xymorg::RGB xymorg::Palette::Web::sandy_brown = { 244,164,96 };
const xymorg::RGB xymorg::Palette::Web::burly_wood = { 222,184,135 };
const xymorg::RGB xymorg::Palette::Web::tan = { 210,180,140 };
const xymorg::RGB xymorg::Palette::Web::rosy_brown = { 188,143,143 };
const xymorg::RGB xymorg::Palette::Web::moccasin = { 255,228,181 };
const xymorg::RGB xymorg::Palette::Web::navajo_white = { 255,222,173 };
const xymorg::RGB xymorg::Palette::Web::peach_puff = { 255,218,185 };
const xymorg::RGB xymorg::Palette::Web::misty_rose = { 255,228,225 };
const xymorg::RGB xymorg::Palette::Web::lavender_blush = { 255,240,245 };
const xymorg::RGB xymorg::Palette::Web::linen = { 250,240,230 };
const xymorg::RGB xymorg::Palette::Web::old_lace = { 253,245,230 };
const xymorg::RGB xymorg::Palette::Web::papaya_whip = { 255,239,213 };
const xymorg::RGB xymorg::Palette::Web::sea_shell = { 255,245,238 };
const xymorg::RGB xymorg::Palette::Web::mint_cream = { 245,255,250 };
const xymorg::RGB xymorg::Palette::Web::slate_gray = { 112,128,144 };
const xymorg::RGB xymorg::Palette::Web::light_slate_gray = { 119,136,153 };
const xymorg::RGB xymorg::Palette::Web::light_steel_blue = { 176,196,222 };
const xymorg::RGB xymorg::Palette::Web::lavender = { 230,230,250 };
const xymorg::RGB xymorg::Palette::Web::floral_white = { 255,250,240 };
const xymorg::RGB xymorg::Palette::Web::alice_blue = { 240,248,255 };
const xymorg::RGB xymorg::Palette::Web::ghost_white = { 248,248,255 };
const xymorg::RGB xymorg::Palette::Web::honeydew = { 240,255,240 };
const xymorg::RGB xymorg::Palette::Web::ivory = { 255,255,240 };
const xymorg::RGB xymorg::Palette::Web::azure = { 240,255,255 };
const xymorg::RGB xymorg::Palette::Web::snow = { 255,250,250 };
const xymorg::RGB xymorg::Palette::Web::black = { 0,0,0 };
const xymorg::RGB xymorg::Palette::Web::dim_gray = { 105,105,105 };
const xymorg::RGB xymorg::Palette::Web::dim_grey = { 105,105,105 };
const xymorg::RGB xymorg::Palette::Web::gray = { 128,128,128 };
const xymorg::RGB xymorg::Palette::Web::grey = { 128,128,128 };
const xymorg::RGB xymorg::Palette::Web::dark_gray = { 169,169,169 };
const xymorg::RGB xymorg::Palette::Web::dark_grey = { 169,169,169 };
const xymorg::RGB xymorg::Palette::Web::silver = { 192,192,192 };
const xymorg::RGB xymorg::Palette::Web::light_gray = { 211,211,211 };
const xymorg::RGB xymorg::Palette::Web::light_grey = { 211,211,211 };
const xymorg::RGB xymorg::Palette::Web::gainsboro = { 220,220,220 };
const xymorg::RGB xymorg::Palette::Web::white_smoke = { 245,245,245 };
const xymorg::RGB xymorg::Palette::Web::white = { 255,255,255 };

//  Stark - High conrast colours for black background
const xymorg::RGB xymorg::Palette::Stark::Black = { 0,0,0 };
const xymorg::RGB xymorg::Palette::Stark::Blue = { 102,255,255 };
const xymorg::RGB xymorg::Palette::Stark::Orange = { 255, 128, 0 };
const xymorg::RGB xymorg::Palette::Stark::White = { 255,255,255 };
const xymorg::RGB xymorg::Palette::Stark::Yellow = { 255,255, 0 };
const xymorg::RGB xymorg::Palette::Stark::Mauve = { 204,0,204 };
const xymorg::RGB xymorg::Palette::Stark::Green = { 51,255,51 };
const xymorg::RGB xymorg::Palette::Stark::Pastel_Green = { 229,255,204 };
const xymorg::RGB xymorg::Palette::Stark::Red = { 204,0,0 };
