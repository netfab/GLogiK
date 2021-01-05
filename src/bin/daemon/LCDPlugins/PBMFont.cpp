/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2021  Fabrice Delliaux <netbox253@gmail.com>
 *
 *	This program is free software: you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation, either version 3 of the License, or
 *	(at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	You should have received a copy of the GNU General Public License
 *	along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <sstream>
#include <stdexcept>

#include <boost/filesystem.hpp>

#include "lib/utils/utils.hpp"

#include "PBMFont.hpp"

namespace GLogiK
{

namespace fs = boost::filesystem;
using namespace NSGKUtils;


const charactersMap_type PBMFont::defaultCharsMap =
	{
		/* -- */
		{"a", {0,0} },  {"b", {1,0} },  {"c", {2,0} },  {"d", {3,0} },  {"e", {4,0} },
		{"f", {5,0} },  {"g", {6,0} },  {"h", {7,0} },  {"i", {8,0} },  {"j", {9,0} },
		{"k", {10,0} }, {"l", {11,0} }, {"m", {12,0} }, {"n", {13,0} }, {"o", {14,0} },
		{"p", {15,0} }, {"q", {16,0} }, {"r", {17,0} }, {"s", {18,0} }, {"t", {19,0} },
		{"u", {20,0} }, {"v", {21,0} }, {"w", {22,0} }, {"x", {23,0} }, {"y", {24,0} },
		{"z", {25,0} },
		/* -- */
		{"A", {0,1} },  {"B", {1,1} },  {"C", {2,1} },  {"D", {3,1} },  {"E", {4,1} },
		{"F", {5,1} },  {"G", {6,1} },  {"H", {7,1} },  {"I", {8,1} },  {"J", {9,1} },
		{"K", {10,1} }, {"L", {11,1} }, {"M", {12,1} }, {"N", {13,1} }, {"O", {14,1} },
		{"P", {15,1} }, {"Q", {16,1} }, {"R", {17,1} }, {"S", {18,1} }, {"T", {19,1} },
		{"U", {20,1} }, {"V", {21,1} }, {"W", {22,1} }, {"X", {23,1} }, {"Y", {24,1} },
		{"Z", {25,1} },
		/* -- */
		{"1", {0,2} },  {"2", {1,2} },  {"3", {2,2} },  {"4", {3,2} },  {"5", {4,2} },
		{"6", {5,2} },  {"7", {6,2} },  {"8", {7,2} },  {"9", {8,2} },  {"0", {9,2} },
		{" ", {10,2} }, {".", {11,2} }, {":", {12,2} }, {"?", {13,2} }, {"!", {14,2} },
		{";", {15,2} }, {"/", {16,2} }, {"-", {17,2} }, {"%", {18,2} },
	};

PBMFont::PBMFont(
	const std::string & PBMName,
	const uint16_t PBMWidth,
	const uint16_t PBMHeight,
	const uint16_t charWidth,
	const uint16_t charHeight,
	const charactersMap_type charsMap)
	:	_fontName(PBMName),
		_charWidth(charWidth),
		_charHeight(charHeight),
		_charX(0),
		_charY(0),
		_charsMap(charsMap)
{
	fs::path fullpath(PBM_DATA_DIR);
	fullpath /= "fonts";
	fullpath /= PBMName;
	fullpath += ".pbm";

	try {
		/* initialize PBM container */
		_PBMData.resize( (PBMWidth / 8) * PBMHeight, 0 );
		this->readPBM(fullpath.string(), _PBMData, PBMWidth, PBMHeight);
	}
	catch (const GLogiKExcept & e) {
		LOG(ERROR) << "exception while reading PBM file: " << fullpath.string();
		throw;
	}
	catch (const std::exception & e) {
		LOG(ERROR) << "vector resize exception ? " << fullpath.string();
		throw GLogiKExcept( e.what() );
	}
}

PBMFont::~PBMFont()
{
#if DEBUGGING_ON
	LOG(DEBUG2) << "deleting font " << _fontName;
#endif
}

void PBMFont::printCharacterOnFrame(
	PBMDataArray & frame,
	const std::string & c,
	uint16_t & PBMXPos,
	const uint16_t PBMYPos)
{
	const uint16_t xByte = PBMXPos / 8;
	const uint16_t xModulo = PBMXPos % 8;

#if DEBUG_PBMFONT
	LOG(DEBUG2) << "xPos: " << PBMXPos
				<< " - xByte: " << xByte
				<< " - xByte modulo: " << xModulo;
#endif

	try {
		_charX = _charsMap.at(c).first;
		_charY = _charsMap[c].second;
	}
	catch (const std::out_of_range& oor) {
		std::ostringstream warn(_fontName, std::ios_base::app);
		warn << " font : unknown character : " << c;
		throw GLogiKExcept( warn.str() );
	}

	uint16_t index = 0;

	if(PBMXPos >= (PBM_WIDTH - _charWidth)) {
		std::ostringstream warn(_fontName, std::ios_base::app);
		warn << " font : pre-breaking write string loop : x : " << std::to_string(PBMXPos);
		throw GLogiKExcept( warn.str() );
	}
	if(PBMYPos >= (PBM_HEIGHT - _charHeight)) {
		std::ostringstream warn(_fontName, std::ios_base::app);
		warn << " font : pre-breaking write string loop : y : " << std::to_string(PBMYPos);
		throw GLogiKExcept( warn.str() );
	}

	try {
		for(uint16_t i = 0; i < _charHeight; i++) {
			unsigned char c = this->getCharacterLine(i);
			index = (PBM_WIDTH_IN_BYTES * (PBMYPos+i)) + xByte;

			// just for debugging
			//#include <bitset>
			//std::bitset<8> bits(c);
			//LOG(DEBUG) << bits.to_string();

			// FIXME _charWidth should be <= 8 here
			const uint16_t xModuloComp8 = (8 - xModulo);
			int16_t rightShift = (_charWidth - xModuloComp8);

			frame.at(index) &= (0b11111111 << xModuloComp8);
			if(rightShift <= 0) {
				frame.at(index) |= (c << (-rightShift));
			}
			else {
				frame.at(index+1) = (c << (8 - rightShift));
				frame[index] |= (c >> rightShift);
			}
		} // for each line in character
	}
	catch (const std::out_of_range& oor) {
		std::ostringstream warn(_fontName, std::ios_base::app);
		warn << " font : wrong frame index : " << std::to_string(index);
		throw GLogiKExcept( warn.str() );
	}

	PBMXPos += _charWidth;
}

const unsigned char PBMFont::getCharacterLine(const uint16_t line) const
{
	unsigned char c = 0;
	const uint16_t i =
		/* PBM_Y_line which contains the character (in bytes) */
		(_charY * _charHeight * PBM_WIDTH_IN_BYTES) +
		/* character's PBM_X position on the PBM_Y_line (in bytes) */
		( (_charX * _charWidth) / 8 ) +
		/* line in the selected character (in bytes) */
		line * PBM_WIDTH_IN_BYTES;

#if 0 && DEBUGGING_ON
	LOG(DEBUG2) << "charX: " << _charX
				<< " charY: " << _charY;
				<< " index: " << i+1;
#endif

	try {
		if(_charWidth == 6) {
			switch( (_charX % 4) ) {
				case 0 :
					c = (_PBMData.at(i) >> 2);
					break;
				case 1 :
					c = (_PBMData.at(i+1) >> 4);
					c |= ((_PBMData[i] & 0b00000011) << 4);
					break;
				case 2 :
					c = (_PBMData.at(i+1) >> 6);
					c |= ((_PBMData[i] & 0b00001111) << 2);
					break;
				case 3 :
					c = (_PBMData.at(i) & 0b00111111);
					break;
			}
		}
		else if(_charWidth == 5) {
			switch( (_charX % 8) ) {
				case 0:
					c = (_PBMData.at(i) >> 3);
					break;
				case 1:
					c = (_PBMData.at(i+1) >> 6);
					c |= ((_PBMData[i] & 0b00000111) << 2);
					break;
				case 2:
					c = ((_PBMData.at(i) & 0b00111110) >> 1);
					break;
				case 3:
					c = (_PBMData.at(i+1) >> 4);
					c |= ((_PBMData[i] & 0b00000001) << 4);
					break;
				case 4:
					c = (_PBMData.at(i+1) >> 7);
					c |= ((_PBMData[i] & 0b00001111) << 1);
					break;
				case 5:
					c = ((_PBMData.at(i) & 0b01111100) >> 2);
					break;
				case 6:
					c = (_PBMData.at(i+1) >> 5);
					c |= ((_PBMData[i] & 0b00000011) << 3);
					break;
				case 7:
					c = (_PBMData.at(i) & 0b00011111);
					break;
			}
		}
	}
	catch (const std::out_of_range& oor) {
		std::ostringstream error(_fontName, std::ios_base::app);
		error << " - wrong index : ";
		error << oor.what();
		error << " - char_width: " << std::to_string(_charWidth);
		error << " - charX: " << std::to_string(_charX);
		GKSysLog(LOG_ERR, ERROR, error.str());
	}
	return c;
}

} // namespace GLogiK

