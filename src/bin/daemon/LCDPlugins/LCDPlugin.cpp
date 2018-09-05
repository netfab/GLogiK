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

#include "lib/utils/utils.hpp"

#include "LCDPlugin.hpp"

namespace GLogiK
{

using namespace NSGKUtils;

LCDPlugin::LCDPlugin()
	:	_pluginName("unknown"),
		_pluginTempo(LCDPluginTempo::TEMPO_DEFAULT),
		_initialized(false),
		_frameCounter(0),
		_frameIndex(0)
{
}

LCDPlugin::~LCDPlugin()
{
#if DEBUGGING_ON
	LOG(DEBUG2) << "deleting " << _pluginName << " LCD plugin";
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
	return _initialized;
}

void LCDPlugin::resetPBMFrameIndex(void)
{
	_itCurrentFrame = _PBMFrames.end();
	this->checkPBMFrameIndex(); /* may throw */
}

const unsigned short LCDPlugin::getPluginTiming(void) const
{
	unsigned short timing = 0;
	unsigned short maxFrames = 0;
	std::tie(timing, maxFrames) = this->getTempo(_pluginTempo);
	return timing;
}

const unsigned short LCDPlugin::getPluginMaxFrames(void) const
{
	unsigned short timing = 0;
	unsigned short maxFrames = 0;
	std::tie(timing, maxFrames) = this->getTempo(_pluginTempo);
	return maxFrames;
}

void LCDPlugin::prepareNextPBMFrame(void)
{
	/* update internal frame counter and iterator to allow the plugin
	 * to have multiples PBM loaded and simulate animation */

	if(_frameCounter >= (*_itCurrentFrame).frame_count) {
		_itCurrentFrame++;
		this->checkPBMFrameIndex(); /* may throw */
		_frameIndex = (_itCurrentFrame - _PBMFrames.begin());
		_frameCounter = 0;
	}

	_frameCounter++; /* for next call */
}

void LCDPlugin::init(FontsManager* const pFonts)
{
	this->resetPBMFrameIndex();
	_initialized = true;
}

const PBMDataArray & LCDPlugin::getNextPBMFrame(
	FontsManager* const pFonts,
	const std::string & LCDKey)
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
	const fs::path & PBMDirectory,
	const std::string & file,
	const unsigned short num)
{
	fs::path filePath(PBMDirectory);
	filePath /= file;

	_PBMFrames.emplace_back(num);
	try {
		this->readPBM(filePath.string(), _PBMFrames.back().pbm_data);
	}
	catch (const GLogiKExcept & e) {
		LOG(ERROR) << "exception while reading PBM file: " << filePath.string();
		throw;
	}
}

void LCDPlugin::addPBMClearedFrame(
	const unsigned short num)
{
	_PBMFrames.emplace_back(num);
	_PBMFrames.back().pbm_data.fill(0x0);
}

const unsigned short LCDPlugin::getNextPBMFrameID(void) const
{
	return _frameIndex;
}

PBMDataArray & LCDPlugin::getCurrentPBMFrame(void)
{
#if DEBUGGING_ON && DEBUG_LCD_PLUGINS
	LOG(DEBUG3) << _pluginName << " PBM # " << _frameIndex;
#endif
	return (*_itCurrentFrame).pbm_data;
}

void LCDPlugin::writeStringOnFrame(
	FontsManager* const pFonts,
	const FontID fontID,
	const std::string & string,
	unsigned int PBMXPos,
	const unsigned int PBMYPos)
{
	try {
#if DEBUGGING_ON && DEBUG_LCD_PLUGINS
		LOG(DEBUG2) << _pluginName << " PBM # " << _frameIndex << " - writing string : " << string;
#endif
		for(const char & c : string) {
			const std::string character(1, c);
			pFonts->printCharacterOnFrame( fontID, (*_itCurrentFrame).pbm_data, character, PBMXPos, PBMYPos );
		} /* for each character in the string */
	}
	catch (const GLogiKExcept & e) {
		GKSysLog(LOG_WARNING, WARNING, e.what());
	}
}

void LCDPlugin::writeStringOnLastFrame(
	FontsManager* const pFonts,
	const FontID fontID,
	const std::string & string,
	unsigned int PBMXPos,
	const unsigned int PBMYPos)
{
	try {
		if( _PBMFrames.empty() )
			throw GLogiKExcept("trying to access last element on empty container");
		_itCurrentFrame = --(_PBMFrames.end());
		_frameIndex = (_itCurrentFrame - _PBMFrames.begin());

		this->writeStringOnFrame(pFonts, fontID, string, PBMXPos, PBMYPos);
	}
	catch (const GLogiKExcept & e) {
		GKSysLog(LOG_WARNING, WARNING, e.what());
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
	if( _itCurrentFrame == _PBMFrames.end() ) {
		_itCurrentFrame = _PBMFrames.begin();

		if( _itCurrentFrame == _PBMFrames.end() )
			throw GLogiKExcept("plugin frame iterator exception");

		_frameCounter = 0;
		_frameIndex = 0;
	}
}

std::tuple<unsigned short, unsigned short> LCDPlugin::getTempo(const LCDPluginTempo tempo)
{
	if(tempo == LCDPluginTempo::TEMPO_500_20)
		return std::make_tuple(500, 20);
	if(tempo == LCDPluginTempo::TEMPO_400_15)
		return std::make_tuple(400, 15);
	/* TEMPO_DEFAULT */
	return std::make_tuple(1000, 10);
}

} // namespace GLogiK

