/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2026  Fabrice Delliaux <netbox253@gmail.com>
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

//#include <bitset>
//#include <iomanip> // XXX see debug code
#include <sstream>
#include <stdexcept>
#include <string_view>

#include <boost/filesystem.hpp>

#include "lib/utils/utils.hpp"

#include "bin/daemon/LCD/PBM/PBMFile.hpp"
#include "bin/daemon/LCD/PBM/PBM.hpp"
#include "PBMFont.hpp"

namespace Managers::LCDPlugins::Fonts
{

namespace fs = boost::filesystem;
using namespace NSGKUtils;

PBMFont::PBMFont(
	const std::string & PBMName,
	const std::uint16_t PBMWidth,
	const std::uint16_t PBMHeight,
	const std::uint16_t charWidth,
	const std::uint16_t charHeight,
	const std::uint16_t fontLeftShift,
	const std::uint16_t extraLeftShift)
	:	_fontName(PBMName),
		_PBMWidth(PBMWidth),
		_PBMHeight(PBMHeight),
		_charWidth(charWidth),
		_charHeight(charHeight),
		_charBytes(((charWidth / 8) == 0) ? 1 : (charWidth / 8)),
		_shiftCharBase(((charWidth % 8) == 0) ? 8 : charWidth),
		_fontLeftShift(fontLeftShift),
		_extraLeftShift(extraLeftShift),
		_charX(0),
		_charY(0)
{
	GK_LOG_FUNC

	fs::path fullpath(PBM_DATA_DIR);
	fullpath /= "fonts";
	fullpath /= PBMName;
	fullpath += ".pbm";

	try
	{
		/* initialize PBM container */
		_PBMData.resize( (PBMWidth / 8) * PBMHeight, 0 );
		PBM::PBMFile::readPBM(fullpath.string(), _PBMData, PBMWidth, PBMHeight);
	}
	catch (const std::exception & e)
	{
		GKSysLogError("vector resize exception ? ", fullpath.string());
		throw GLogiKExcept( e.what() );
	}
}

PBMFont::~PBMFont()
{
	GK_LOG_FUNC

	GKLog2(trace, "deleting font ", _fontName)
}

const std::size_t PBMFont::characterLength(std::string_view sv, std::size_t pos)
{
	// lambda
	auto check_char = [&sv] (std::size_t p, unsigned char c) -> bool
	{
		return (static_cast<unsigned char>(sv[p]) == c);
	};

	// ° in UTF-8 = C2 B0
	// TODO : see utfcpp library
	if( pos + 1 < sv.size() && check_char(pos, 0xC2) && check_char(pos+1, 0xB0) )
		return 2;

	return 1;
}

const std::uint16_t PBMFont::getCenteredXPos(std::string_view sv)
{
	unsigned int XPos = PBM::LCD_width;
	std::size_t pos = 0;

	while( pos < sv.size() )
	{
		const std::size_t length = PBMFont::characterLength(sv, pos);

		XPos -= (_charWidth - _fontLeftShift);
		// XXX
		if(_extraLeftShift > 0)
		{
			std::string_view character = sv.substr(pos, length);

			if(hackstring.find(character) == std::string::npos)
				XPos += _extraLeftShift;
		}

		pos += length;
	}

	return static_cast<std::uint16_t>(XPos/2);
}

const std::uint16_t PBMFont::getCenteredYPos()
{
	std::uint16_t YPos = PBM::LCD_height;
	YPos -= _charHeight;
	return static_cast<std::uint16_t>(YPos/2);
}

void PBMFont::printCharacterOnFrame(
	PixelsData & frame,
	std::string_view sv,
	std::uint16_t & PBMXPos,
	const std::uint16_t PBMYPos)
{
	GK_LOG_FUNC

	if(const Point* point = PBMFont::findCharacter(sv))
	{
		_charX = point->x;
		_charY = point->y;
	}
	else
	{
		std::ostringstream warn(_fontName, std::ios_base::app);
		warn << " font : unknown character : " << sv;
		throw GLogiKExcept( warn.str() );
	}

	if(PBMXPos >= (PBM::LCD_width - _charWidth))
	{
		std::ostringstream warn(_fontName, std::ios_base::app);
		warn << " font : pre-breaking write string loop : x : " << std::to_string(PBMXPos);
		throw GLogiKExcept( warn.str() );
	}
	if(PBMYPos >= (PBM::LCD_height - _charHeight))
	{
		std::ostringstream warn(_fontName, std::ios_base::app);
		warn << " font : pre-breaking write string loop : y : " << std::to_string(PBMYPos);
		throw GLogiKExcept( warn.str() );
	}

	std::uint16_t index = 0;

	const std::uint16_t xByte = PBMXPos / 8;
	const std::uint16_t xModulo = PBMXPos % 8;

	const std::uint16_t xModuloComp8 = (8 - xModulo);

	/* if _shiftCharBase == 8, rightShift = xModulo */
	const int16_t rightShift = static_cast<int16_t>(_shiftCharBase - xModuloComp8);

#if DEBUG_PBMFONT
	if(GKLogging::GKDebug)
	{
		LOG(trace)	<< "xPos: " << PBMXPos
					<< " - xByte: " << xByte
					<< " - xByte modulo: " << xModulo;
		LOG(trace)	<< "PBMFont charBytes: " << _charBytes;
	}
#endif

	try
	{
		for(std::uint16_t i = 0; i < _charHeight; i++)
			for(std::uint16_t j = 0; j < _charBytes; j++)
			{
				const unsigned char c = this->getCharacterLine(i, j);
				unsigned int t = (PBM::default_width_in_bytes * (PBMYPos+i)) + xByte + j;
				index = static_cast<uint16_t>(t);

				frame.at(index) &= (0b11111111 << xModuloComp8);
				if(rightShift > 0)
				{
					frame.at(index+1) = (c << (8 - rightShift));
					frame[index] |= (c >> rightShift);
				}
				else
					frame.at(index) |= (c << (-rightShift));
			}
	}
	catch (const std::out_of_range& oor)
	{
		std::ostringstream warn(_fontName, std::ios_base::app);
		warn << " font : wrong frame index : " << std::to_string(index);
		throw GLogiKExcept( warn.str() );
	}

	PBMXPos += static_cast<uint16_t>(_charWidth - _fontLeftShift);

	// XXX ugly hack
	// applying another leftshift, except for those characters
	// which are very wide compared to others
	if(_extraLeftShift > 0)
		if(hackstring.find(sv) == std::string::npos)
			PBMXPos -= _extraLeftShift;
}

auto PBMFont::findCharacter(std::string_view sv) -> const Point*
{
    for (const auto & mapping : PBMFont::charactersMap)
	{
		/* XXX debug code
		if (mapping.character == "°" || sv == "°")
		{
			LOG(trace)	<< "mapping: size=" << mapping.character.size()
						<< ", character: size=" << sv.size();
			LOG(trace) << "mapping bytes: ";
			for (unsigned char c : mapping.character)
				LOG(trace) << std::hex << static_cast<int>(c) << ' ';

			LOG(trace) << "character bytes: ";
			for (unsigned char c : sv)
				LOG(trace) << std::hex << static_cast<int>(c) << ' ';
		}
		*/

		if (mapping.character == sv)
			return &mapping.point;
	}

	return nullptr;
}

const unsigned char PBMFont::getCharacterLine(
	const std::uint16_t line,
	const std::uint16_t charByte) const
{
	GK_LOG_FUNC

	unsigned char c = 0;
	const std::uint16_t i = static_cast<std::uint16_t>(
		/* PBM_Y_line which contains the character (in bytes) */
		(_charY * _charHeight * (_PBMWidth / 8)) +
		/* character's PBM_X position on the PBM_Y_line (in bytes) */
		( (_charX * _charWidth) / 8 ) +
		/* line in the selected character (in bytes) */
		line * (_PBMWidth / 8)
	);

#if 0 && DEBUGGING_ON
	LOG(trace)	<< "charX: " << _charX
				<< " charY: " << _charY;
				<< " index: " << i+1;
#endif

	try
	{
		if(_charWidth == 6)
		{
			switch( (_charX % 4) )
			{
				case 0 :
					{
						const unsigned int value = (_PBMData.at(i) >> 2);
						c = static_cast<unsigned char>(value);
						break;
					}
				case 1 :
					{
						const unsigned int value =
							( (_PBMData.at(i+1) >> 4) | ((_PBMData[i] & 0b00000011) << 4) );
						c = static_cast<unsigned char>(value);
						break;
					}
				case 2 :
					{
						const unsigned int value =
							( (_PBMData.at(i+1) >> 6) | ((_PBMData[i] & 0b00001111) << 2) );
						c = static_cast<unsigned char>(value);
						break;
					}
				case 3 :
					{
						const unsigned int value = (_PBMData.at(i) & 0b00111111);
						c = static_cast<unsigned char>(value);
						break;
					}
			}
		}
		else if(_charWidth == 5)
		{
			switch( (_charX % 8) )
			{
				case 0:
					{
						const unsigned int value = (_PBMData.at(i) >> 3);
						c = static_cast<unsigned char>(value);
						break;
					}
				case 1:
					{
						const unsigned int value =
							( (_PBMData.at(i+1) >> 6) | ((_PBMData[i] & 0b00000111) << 2) );
						c = static_cast<unsigned char>(value);
						break;
					}
				case 2:
					{
						const unsigned int value = ( (_PBMData.at(i) & 0b00111110) >> 1 );
						c = static_cast<unsigned char>(value);
						break;
					}
				case 3:
					{
						const unsigned int value =
							( (_PBMData.at(i+1) >> 4) | ((_PBMData[i] & 0b00000001) << 4) );
						c = static_cast<unsigned char>(value);
						break;
					}
				case 4:
					{
						const unsigned int value =
							( (_PBMData.at(i+1) >> 7) | ((_PBMData[i] & 0b00001111) << 1) );
						c = static_cast<unsigned char>(value);
						break;
					}
				case 5:
					{
						const unsigned int value = ( (_PBMData.at(i) & 0b01111100) >> 2 );
						c = static_cast<unsigned char>(value);
						break;
					}
				case 6:
					{
						const unsigned int value =
							( (_PBMData.at(i+1) >> 5) | ((_PBMData[i] & 0b00000011) << 3) );
						c = static_cast<unsigned char>(value);
						break;
					}
				case 7:
					{
						const unsigned int value = (_PBMData.at(i) & 0b00011111);
						c = static_cast<unsigned char>(value);
						break;
					}
			}
		}
		else if((_charWidth % 8) == 0)
		{
			/* charByte modifier when _charWidth multiple of 8 */
			const unsigned int value = (_PBMData.at(i + charByte));
			c = static_cast<unsigned char>(value);
		}
	}
	catch (const std::out_of_range& oor)
	{
		std::ostringstream error(_fontName, std::ios_base::app);
		error << " - wrong index : ";
		error << oor.what();
		error << " - char_width: " << std::to_string(_charWidth);
		error << " - charX: " << std::to_string(_charX);
		GKSysLogError(error.str());
	}

	//std::bitset<8> bits(c);
	//LOG(trace) << bits.to_string();

	return c;
}

} // namespace Managers::LCDPlugins::Fonts

