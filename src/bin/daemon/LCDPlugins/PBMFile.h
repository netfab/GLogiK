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

#include <vector>
#include <string>
#include <fstream>

#include "PBM.h"

namespace GLogiK
{

struct PBMFrame
{
	PBMFrame(const unsigned short i)
		:	max_index(i) {}

	unsigned short max_index;
	PBMDataArray pbm_data;
};

class PBMFile
{
	public:
		~PBMFile(void);

		void readPBM(
			const std::string & path,
			const unsigned short max_index = 0,
			const unsigned int expected_width = PBM_WIDTH,
			const unsigned int expected_height = PBM_HEIGHT);

		void checkFrameIndex(const bool reset=false);
		const PBMDataArray & getNextPBMData(void);

	protected:
		PBMFile(void);
		std::string name_;

	private:
		unsigned short current_index_;
		std::vector<PBMFrame> frames_;
		std::vector<PBMFrame>::iterator current_frame_;

		void parsePBMHeader(
			std::ifstream & pbm,
			std::string & magic,
			unsigned int & width,
			unsigned int & height);

		void extractPBMData(
			std::ifstream & pbm,
			PBMDataArray & pbm_data);

		void closePBM(std::ifstream & pbm);

};

} // namespace GLogiK

#endif
