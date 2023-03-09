/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2023  Fabrice Delliaux <netbox253@gmail.com>
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

#ifndef SRC_BIN_DAEMON_LCDPLUGINS_PBM_FILE_HPP_
#define SRC_BIN_DAEMON_LCDPLUGINS_PBM_FILE_HPP_

#include <cstdint>

#include <string>
#include <fstream>

#include "PBM.hpp"

namespace GLogiK
{

class PBMFile
{
	public:

	protected:
		PBMFile(void);
		~PBMFile(void);

		static void readPBM(
			const std::string & PBMPath,
			PixelsData & PBMData,
			const uint16_t PBMWidth,
			const uint16_t PBMHeight);

	private:
		static void parsePBMHeader(
			std::ifstream & pbm,
			std::string & magic,
			uint16_t & width,
			uint16_t & height);

		static void extractPBMData(
			std::ifstream & pbm,
			PixelsData & PBMData);

		static void closePBM(std::ifstream & pbm);

};

} // namespace GLogiK

#endif
