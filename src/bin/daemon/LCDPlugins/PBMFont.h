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

#ifndef __GLOGIKD_PBM_FONT_H__
#define __GLOGIKD_PBM_FONT_H__

#include <utility>
#include <string>
#include <map>
#include <initializer_list>

#include "PBM.h"
#include "PBMFile.h"

namespace GLogiK
{


typedef std::initializer_list<std::pair<const std::string, std::pair<unsigned short, unsigned short>>> characters_map_t;

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
			const characters_map_t charsMap = PBMFont::defaultCharsMap
		);

	private:
		PBMDataArray pbm_data_;
		const std::string font_name_;
		const unsigned int char_width_;
		const unsigned int char_height_;
		unsigned short cur_x_;
		unsigned short cur_y_;

		std::map<const std::string, std::pair<unsigned short, unsigned short>> chars_map_;

		static const characters_map_t defaultCharsMap;

		const unsigned char getCharacterLine(const unsigned short line) const;
};

} // namespace GLogiK

#endif

