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

#ifndef __GLOGIKD_PBM_FILE_H__
#define __GLOGIKD_PBM_FILE_H__

#include <string>
#include <fstream>

#include "PBM.h"

namespace GLogiK
{

class PBMFile
{
	public:

	protected:
		PBMFile(void);
		~PBMFile(void);

		static void readPBM(
			const std::string & path,
			PBMDataArray & pbm_data,
			const unsigned int expected_width = PBM_WIDTH,
			const unsigned int expected_height = PBM_HEIGHT);

	private:
		static void parsePBMHeader(
			std::ifstream & pbm,
			std::string & magic,
			unsigned int & width,
			unsigned int & height);

		static void extractPBMData(
			std::ifstream & pbm,
			PBMDataArray & pbm_data);

		static void closePBM(std::ifstream & pbm);

};

} // namespace GLogiK

#endif
