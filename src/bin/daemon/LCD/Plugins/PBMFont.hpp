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

#pragma once

#include <cstdint>

#include <string>
#include <string_view>

#include "bin/daemon/LCD/PixelsData.hpp"
#include "PBMFile.hpp"

namespace Managers::LCDPlugins::plugin
{

class PBMFont
	:	virtual protected PBMFile
{
	private:
		using PixelsData = ::Managers::LCDPlugins::PixelsData;

	public:
		virtual ~PBMFont();

		static const std::size_t characterLength(std::string_view sv, std::size_t pos);

		const std::uint16_t getCenteredXPos(std::string_view sv);
		const std::uint16_t getCenteredYPos();

		void printCharacterOnFrame(
			PixelsData & frame,
			std::string_view character,
			std::uint16_t & PBMXPos,
			const std::uint16_t PBMYPos
		);

	protected:
		PBMFont(
			const std::string & PBMName,
			const std::uint16_t PBMWidth,
			const std::uint16_t PBMHeight,
			const std::uint16_t charWidth,
			const std::uint16_t charHeight,
			const std::uint16_t fontLeftShift = 0,
			const std::uint16_t extraLeftShift = 0
		);

	private:
		PixelsData _PBMData;
		const std::string _fontName;
		const std::uint16_t _PBMWidth;
		// LLVM/Clang warning -Wunused-private-field
		[[maybe_unused]] const std::uint16_t _PBMHeight;
		const std::uint16_t _charWidth;
		const std::uint16_t _charHeight;
		const std::uint16_t _charBytes;
		const std::uint16_t _shiftCharBase;
		const std::uint16_t _fontLeftShift;
		const std::uint16_t _extraLeftShift;
		std::uint16_t _charX;
		std::uint16_t _charY;

		struct Point {
			std::uint16_t x;
			std::uint16_t y;
		};

		struct CharMapping {
			std::string_view character;
			Point point;
		};

		static constexpr CharMapping charactersMap[] =
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
			{";", {15,2} }, {"/", {16,2} }, {"-", {17,2} }, {"%", {18,2} }, {"°", {19,2} },
		};
		static constexpr std::string_view hackstring = "mGMW%";

		static const Point* findCharacter(std::string_view character);
		const unsigned char getCharacterLine(
			const std::uint16_t line,
			const std::uint16_t charByte
		) const;
};

} // namespace Managers::LCDPlugins::plugin
