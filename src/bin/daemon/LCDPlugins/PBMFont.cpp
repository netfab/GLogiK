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

#include <boost/filesystem.hpp>

#include "lib/utils/utils.h"

#include "PBMFont.h"

namespace GLogiK
{

namespace fs = boost::filesystem;
using namespace NSGKUtils;


const characters_map_t PBMFont::defaultCharsMap =
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
	};

PBMFont::PBMFont(
	const std::string & pbmName,
	const unsigned short width,
	const unsigned short height,
	const characters_map_t charsMap)
	:	font_name_(pbmName),
		char_width_(width),
		char_height_(height),
		cur_x_(0),
		cur_y_(0),
		chars_map_(charsMap)
{
	fs::path fullpath(PBM_DATA_DIR);
	fullpath /= pbmName;
	fullpath += ".pbm";

	try {
		this->readPBM(fullpath.string(), this->pbm_data_);
	}
	catch (const GLogiKExcept & e) {
		LOG(ERROR) << "exception while reading PBM file: " << fullpath.string();
		throw;
	}
}

PBMFont::~PBMFont()
{
#if DEBUGGING_ON
	LOG(DEBUG2) << "deleting font " << this->font_name_;
#endif
}

void PBMFont::setCurrentPosition(const std::string & c)
{
	try {
		this->cur_x_ = this->chars_map_.at(c).first;
		this->cur_y_ = this->chars_map_[c].second;
	}
	catch (const std::out_of_range& oor) {
		GKSysLog(LOG_WARNING, WARNING, "unknown character");
	}
}

} // namespace GLogiK

