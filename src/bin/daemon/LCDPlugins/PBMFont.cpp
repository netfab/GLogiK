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

PBMFont::PBMFont(
	const std::string & pbmName,
	const unsigned short width,
	const unsigned short height)
	:	font_name_(pbmName),
		char_width_(width),
		char_height_(height)
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

} // namespace GLogiK

