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
		tempo_(LCDPluginTempo::TEMPO_DEFAULT),
		initialized_(false),
		frame_count_(0),
		frame_ID_(0)
{
}

LCDPlugin::~LCDPlugin()
{
#if DEBUGGING_ON
	LOG(DEBUG2) << "deleting " << this->name_ << " LCD plugin";
#endif
}

/*
 * --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---
 * --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---
 *
 * === public === public === public === public === public ===
 *
 * --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---
 * --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---
 */

const bool LCDPlugin::isInitialized(void) const
{
	return this->initialized_;
}

void LCDPlugin::resetPBMFrameIndex(void)
{
	this->current_frame_ = this->frames_.end();
	this->checkPBMFrameIndex(); /* may throw */
}

const unsigned short LCDPlugin::getPluginTiming(void) const
{
	unsigned short timing = 0;
	unsigned short max_frames;
	std::tie(timing, max_frames) = this->getTempo(this->tempo_);
	return timing;
}

const unsigned short LCDPlugin::getPluginMaxFrames(void) const
{
	unsigned short timing = 0;
	unsigned short max_frames;
	std::tie(timing, max_frames) = this->getTempo(this->tempo_);
	return max_frames;
}

void LCDPlugin::prepareNextPBMFrame(void)
{
	/* update internal frame counter and iterator to allow the plugin
	 * to have multiples PBM loaded and simulate animation */

	if(this->frame_count_ >= (*this->current_frame_).frame_count) {
		this->current_frame_++;
		this->checkPBMFrameIndex(); /* may throw */
		this->frame_ID_ = (this->current_frame_ - this->frames_.begin());
		this->frame_count_ = 0;
	}

	this->frame_count_++; /* for next call */
}

void LCDPlugin::init(FontsManager* const pFonts)
{
	this->current_frame_ = this->frames_.begin();
	this->checkPBMFrameIndex(); /* may throw */

	this->initialized_ = true;
}

const PBMDataArray & LCDPlugin::getNextPBMFrame(FontsManager* const pFonts)
{
	return this->getCurrentPBMFrame();
}

/*
 * --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---
 * --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---
 *
 * === protected === protected === protected === protected ===
 *
 * --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---
 * --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---
 */

void LCDPlugin::addPBMFrame(
	const fs::path & pbm_dir,
	const std::string & file,
	const unsigned short num)
{
	fs::path file_path(pbm_dir);
	file_path /= file;

	this->frames_.emplace_back(num);
	try {
		this->readPBM(file_path.string(), this->frames_.back().pbm_data);
	}
	catch (const GLogiKExcept & e) {
		LOG(ERROR) << "exception while reading PBM file: " << file_path.string();
		throw;
	}
}

void LCDPlugin::addPBMClearedFrame(
	const unsigned short num)
{
	this->frames_.emplace_back(num);
	this->frames_.back().pbm_data.fill(0x0);
}

const unsigned short LCDPlugin::getNextPBMFrameID(void) const
{
	return this->frame_ID_;
}

PBMDataArray & LCDPlugin::getCurrentPBMFrame(void)
{
#if DEBUGGING_ON
	LOG(DEBUG3) << this->name_ << " PBM # " << this->frame_ID_;
#endif
	return (*this->current_frame_).pbm_data;
}

void LCDPlugin::writeStringOnFrame(
	FontsManager* const pFonts,
	const FontID fontID,
	const std::string & string,
	const unsigned int PBMXPos,
	const unsigned int PBMYPos)
{
	const unsigned short xByte = PBMXPos / 8;
	const unsigned short xModulo = PBMXPos % 8;

#if DEBUGGING_ON
	LOG(DEBUG2) << "xPos: " << PBMXPos
				<< " - xByte: " << xByte
				<< " - xByte modulo: " << xModulo;
#endif

	try {
		for(const char & c : string) {
			const std::string cur_char(1, c);
			pFonts->setFontPosition(fontID, cur_char);
		}
	}
	catch (const GLogiKExcept & e) {
		std::string warn("write string on frame failure : ");
		warn += e.what();
		GKSysLog(LOG_WARNING, WARNING, warn);
	}
}

/*
 * --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---
 * --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---
 *
 * === private === private === private === private === private ===
 *
 * --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---
 * --- --- --- --- --- --- --- --- --- --- --- --- --- --- --- ---
 */

void LCDPlugin::checkPBMFrameIndex(void)
{
	if( this->current_frame_ == this->frames_.end() ) {
		this->current_frame_ = this->frames_.begin();

		if( this->current_frame_ == this->frames_.end() )
			throw GLogiKExcept("plugin frame iterator exception");

		this->frame_count_ = 0;
		this->frame_ID_ = 0;
	}
}

std::tuple<unsigned short, unsigned short> LCDPlugin::getTempo(const LCDPluginTempo tempo)
{
	if(tempo == LCDPluginTempo::TEMPO_750_8)
		return std::make_tuple(750, 8);
	/* TEMPO_DEFAULT */
	return std::make_tuple(1000, 6);
}

} // namespace GLogiK

