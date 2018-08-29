/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2018  Fabrice Delliaux <netbox253@gmail.com>
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

#include <utility>
#include <string>
#include <map>
#include <initializer_list>

#include "PBM.hpp"
#include "PBMFile.hpp"

namespace GLogiK
{


typedef std::initializer_list<std::pair<const std::string, std::pair<unsigned short, unsigned short>>> charactersMap_type;

class PBMFont
	:	virtual private PBMFile
{
	public:
		virtual ~PBMFont(void);

		void printCharacterOnFrame(
			PBMDataArray & frame,
			const std::string & c,
			unsigned int & PBMXPos,
			const unsigned int PBMYPos
		);

	protected:
		PBMFont(
			const std::string & pbmName,
			const unsigned short width = 6,
			const unsigned short height = 10,
			const charactersMap_type charsMap = PBMFont::defaultCharsMap
		);

	private:
		PBMDataArray _PBMData;
		const std::string _fontName;
		const unsigned int _charWidth;
		const unsigned int _charHeight;
		unsigned short _charX;
		unsigned short _charY;

		std::map<const std::string, std::pair<unsigned short, unsigned short>> _charsMap;

		static const charactersMap_type defaultCharsMap;

		const unsigned char getCharacterLine(const unsigned short line) const;
};

} // namespace GLogiK

#endif

