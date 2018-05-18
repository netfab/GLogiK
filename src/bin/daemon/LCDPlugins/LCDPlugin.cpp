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

#include "lib/utils/utils.h"

#include "LCDPlugin.h"

namespace GLogiK
{

using namespace NSGKUtils;

LCDPlugin::LCDPlugin()
	:	name_("unknown"),
		tempo_(1),
		initialized_(false),
		frame_count_(0)
{
}

LCDPlugin::~LCDPlugin()
{
#if DEBUGGING_ON
	LOG(DEBUG2) << "deleting " << this->name_ << " LCD plugin";
#endif
}

const bool LCDPlugin::isInitialized(void) const
{
	return this->initialized_;
}

void LCDPlugin::resetPBMFrameIndex(void)
{
	this->current_frame_ = this->frames_.end();
	this->checkPBMFrameIndex(); /* may throw */
}

const unsigned short LCDPlugin::getPluginTempo(void)
{
	return this->tempo_;
}

const PBMDataArray & LCDPlugin::getNextPBMFrame(void)
{
	/* update internal index and iterator to allow the plugin
	 * to have multiples PBM loaded and simulate animation */

	if(this->frame_count_ >= (*this->current_frame_).frame_count) {
		this->current_frame_++;
		this->frame_count_ = 0;
	}

	this->checkPBMFrameIndex();

	this->frame_count_++; /* for next call */

#if DEBUGGING_ON
	LOG(DEBUG3) << this->name_ << " frame # " << (this->current_frame_ - this->frames_.begin()) ;
#endif
	return (*this->current_frame_).pbm_data;
}

void LCDPlugin::init(void)
{
	this->current_frame_ = this->frames_.begin();
	this->checkPBMFrameIndex(); /* may throw */

	this->initialized_ = true;
}

void LCDPlugin::checkPBMFrameIndex(void)
{
	if( this->current_frame_ == this->frames_.end() ) {
		this->current_frame_ = this->frames_.begin();

		if( this->current_frame_ == this->frames_.end() )
			throw GLogiKExcept("plugin frame iterator exception");

		this->frame_count_ = 0;
	}
}

void LCDPlugin::addPBMFrame(
	const std::string & path,
	const std::string & file,
	const unsigned short num)
{
	std::string filepath(path);
	filepath += "/";		// TODO boost:fs:path
	filepath += file;
	this->frames_.emplace_back(num);
	try {
		this->readPBM(filepath, this->frames_.back().pbm_data);
	}
	catch (const GLogiKExcept & e) {
		LOG(ERROR) << "exception while reading PBM file: " << filepath;
		throw;
	}
}

void LCDPlugin::addPBMClearedFrame(
	const unsigned short num)
{
	this->frames_.emplace_back(num);
	this->frames_.back().pbm_data.fill(0x0);
}

} // namespace GLogiK

