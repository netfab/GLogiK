/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2025  Fabrice Delliaux <netbox253@gmail.com>
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

#ifndef SRC_BIN_DAEMON_LCDPLUGINS_PBM_FONT_HPP_
#define SRC_BIN_DAEMON_LCDPLUGINS_PBM_FONT_HPP_

#include <cstdint>

#include <utility>
#include <string>
#include <map>
#include <initializer_list>

#include "PBM.hpp"
#include "PBMFile.hpp"

namespace GLogiK
{

typedef std::initializer_list<std::pair<const std::string, std::pair<uint16_t, uint16_t>>> charactersMap_type;

class PBMFont
	:	virtual private PBMFile
{
	public:
		virtual ~PBMFont(void);

		static const std::string deg;

		const uint16_t getCenteredXPos(const std::string & string);
		const uint16_t getCenteredYPos(void);

		void printCharacterOnFrame(
			PixelsData & frame,
			const std::string & character,
			uint16_t & PBMXPos,
			const uint16_t PBMYPos
		);

	protected:
		PBMFont(
			const std::string & PBMName,
			const uint16_t PBMWidth,
			const uint16_t PBMHeight,
			const uint16_t charWidth,
			const uint16_t charHeight,
			const uint16_t fontLeftShift = 0,
			const uint16_t extraLeftShift = 0,
			const charactersMap_type charsMap = PBMFont::defaultCharsMap
		);

	private:
		PixelsData _PBMData;
		const std::string _fontName;
		const uint16_t _PBMWidth;
		const uint16_t _PBMHeight;
		const uint16_t _charWidth;
		const uint16_t _charHeight;
		const uint16_t _charBytes;
		const uint16_t _shiftCharBase;
		const uint16_t _fontLeftShift;
		const uint16_t _extraLeftShift;
		uint16_t _charX;
		uint16_t _charY;

		std::map<std::string, std::pair<uint16_t, uint16_t>> _charsMap;

		static const charactersMap_type defaultCharsMap;
		static const std::string hackstring;

		const unsigned char getCharacterLine(
			const uint16_t line,
			const uint16_t charByte
		) const;
};

} // namespace GLogiK

#endif

