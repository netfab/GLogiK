/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2021  Fabrice Delliaux <netbox253@gmail.com>
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

#include <sstream>

#include "lib/utils/utils.hpp"

#include "LCDPlugin.hpp"

namespace GLogiK
{

using namespace NSGKUtils;

/* -- -- -- */

const bool LCDPBMFrame::switchToNextFrame(const uint16_t currentFrameCounter)
{
	return (currentFrameCounter >= _frameCounter);
}

/* -- -- -- */

LCDPlugin::LCDPlugin()
	:	_pluginTempo(LCDPluginTempo::TEMPO_DEFAULT),
		_initialized(false),
		_everLocked(false),
		_frameCounter(0),
		_frameIndex(0)
{
}

LCDPlugin::~LCDPlugin()
{
#if DEBUGGING_ON
	LOG(DEBUG2) << "deleting " << this->getPluginName() << " LCD plugin";
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

const uint64_t LCDPlugin::getPluginID(void) const
{
	return _plugin.getID();
}

const std::string & LCDPlugin::getPluginName(void) const
{
	return _plugin.getName();
}

const uint16_t LCDPlugin::getPluginTiming(void) const
{
	uint16_t timing = 0;
	uint16_t maxFrames = 0;
	std::tie(timing, maxFrames) = this->getTempo(_pluginTempo);
	return timing;
}

const uint16_t LCDPlugin::getPluginMaxFrames(void) const
{
	uint16_t timing = 0;
	uint16_t maxFrames = 0;
	std::tie(timing, maxFrames) = this->getTempo(_pluginTempo);
	return maxFrames;
}

void LCDPlugin::prepareNextPBMFrame(void)
{
	/* update internal frame counter and iterator to allow the plugin
	 * to have multiples PBM loaded and simulate animation */
	if( (*_itCurrentFrame).switchToNextFrame(_frameCounter) ) {
		_itCurrentFrame++;
		this->checkPBMFrameIndex(); /* may throw */
		_frameIndex = (_itCurrentFrame - _PBMFrames.begin());
		_frameCounter = 0;
	}

	_frameCounter++; /* for next call */
}

void LCDPlugin::init(FontsManager* const pFonts, const std::string & product)
{
	this->resetPBMFrameIndex();
	_initialized = true;
}

const PBMDataArray & LCDPlugin::getNextPBMFrame(
	FontsManager* const pFonts,
	const std::string & LCDKey,
	const bool lockedPlugin)
{
	this->drawPadlockOnFrame(lockedPlugin);
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

const LCDPluginProperties LCDPlugin::getPluginProperties(void) const
{
	return _plugin;
}

void LCDPlugin::addPBMFrame(
	const fs::path & PBMDirectory,
	const std::string & file,
	const uint16_t num)
{
	fs::path filePath(PBMDirectory);
	filePath /= file;

	this->addPBMClearedFrame(num);

	try {
		this->readPBM(filePath.string(), _PBMFrames.back()._PBMData, PBM_WIDTH, PBM_HEIGHT);
	}
	catch (const GLogiKExcept & e) {
		LOG(ERROR) << "exception while reading PBM file: " << filePath.string();
		throw;
	}
}

void LCDPlugin::addPBMClearedFrame(
	const uint16_t num)
{
	try {
		_PBMFrames.emplace_back(num);
	}
	catch (const std::exception & e) {
		LOG(ERROR) << "LCDPBMFrame constructor vector resize exception ?";
		throw GLogiKExcept("initializing PBM frame failure");
	}
}

const uint16_t LCDPlugin::getNextPBMFrameID(void) const
{
	return _frameIndex;
}

PBMDataArray & LCDPlugin::getCurrentPBMFrame(void)
{
#if DEBUGGING_ON && DEBUG_LCD_PLUGINS
	LOG(DEBUG3) << this->getPluginName() << " PBM # " << _frameIndex;
#endif
	return (*_itCurrentFrame)._PBMData;
}

void LCDPlugin::writeStringOnFrame(
	FontsManager* const pFonts,
	const FontID fontID,
	const std::string & string,
	const int16_t PBMXPos,
	const uint16_t PBMYPos)
{
	try {
#if DEBUGGING_ON && DEBUG_LCD_PLUGINS
		LOG(DEBUG2) << this->getPluginName() << " PBM # " << _frameIndex << " - writing string : " << string;
#endif
		uint16_t XPos = 0;
		if( PBMXPos < 0 ) { /* centered */
			XPos = pFonts->getCenteredXPos(fontID, string);
		}
		else {
			XPos = static_cast<uint16_t>(PBMXPos);
		}

		for(const char & c : string) {
			const std::string character(1, c);
			pFonts->printCharacterOnFrame( fontID, (*_itCurrentFrame)._PBMData, character, XPos, PBMYPos );
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
	const int16_t PBMXPos,
	const uint16_t PBMYPos)
{
	try {
		if( _PBMFrames.empty() )
			throw GLogiKExcept("accessing last element on empty container");
		_itCurrentFrame = --(_PBMFrames.end());
		_frameIndex = (_itCurrentFrame - _PBMFrames.begin());

		this->writeStringOnFrame(pFonts, fontID, string, PBMXPos, PBMYPos);
	}
	catch (const GLogiKExcept & e) {
		GKSysLog(LOG_WARNING, WARNING, e.what());
	}
}

void LCDPlugin::drawProgressBarOnFrame(
	const uint16_t percent,
	const uint16_t PBMXPos,
	const uint16_t PBMYPos)
{
	const uint16_t PROGRESS_BAR_WIDTH = 102;
	const uint16_t PROGRESS_BAR_HEIGHT = 7;

	if(PBMXPos + PROGRESS_BAR_WIDTH > (PBM_WIDTH - 1)) {
		std::ostringstream buffer(std::ios_base::app);
		buffer << "wrong progress bar X position : " << PBMXPos;
		GKSysLog(LOG_WARNING, WARNING, buffer.str());
		return;
	}

	try {
		PBMDataArray & frame = (*_itCurrentFrame)._PBMData;

		auto drawHorizontalLine = [&frame] (const uint16_t index) -> void
		{
			frame[index+12] |= 0b11111100;
			for(uint16_t i = 0; i < 12; ++i) {
				frame[index+i] = 0b11111111;
			}
		};

		auto drawProgressBarLine = [&frame, &percent]
			(const uint16_t index, const uint16_t line) -> void
		{
			const unsigned char B10 = 0b10101010;
			const unsigned char B01 = 0b01010101;
			const unsigned char B11 = 0b11111111;

			const uint16_t percentByte = percent / 8;
			const uint16_t percentModulo = percent % 8;
			/* -1 to consider the bar left border */
			const uint16_t leftShift = ((8-1) - percentModulo);

			const unsigned char & Byte1 = (line % 2 == 1) ? B10 : B01;
			const unsigned char & Byte2 = (line % 2 == 1) ? B01 : B10;
			const unsigned char & cByte = (leftShift % 2 == 1) ? Byte2 : Byte1;

			auto getShiftedByte = [&leftShift]
				(const unsigned char & b) -> unsigned char
			{
				return static_cast<unsigned char>( b << leftShift );
			};

#if DEBUGGING_ON && DEBUG_LCD_PLUGINS
			LOG(DEBUG3)	<< "index: " << index
						<< " line: " << line
						<< " pByte: " << percentByte
						<< " pModulo: " << percentModulo
						<< " leftShift: " << leftShift;
#endif

			for(uint16_t i = 0; i < 12; ++i) {
				if((i == 0) and (percent < 8)) {
					frame[index+i] = getShiftedByte(cByte);
				}
				else if(i < percentByte) {
					frame[index+i] = (i < 8) ? Byte1 : B11;
				}
				else if(i == percentByte) {
					frame[index+i] = (i < 8) ? getShiftedByte(cByte) : getShiftedByte(B11);
				}
				else {
					frame[index+i] = 0;
				}
			}

			frame[index+12] = (percentByte == 12) ? getShiftedByte(B11) : 0;

			frame[index]    |= 0b10000000;	/* left border */
			frame[index+12] |= 0b00000100;	/* right border */
		};

		const uint16_t xByte = PBMXPos / 8;
		const uint16_t index = (PBM_WIDTH_IN_BYTES * PBMYPos) + xByte;

		/* checking for out of range */
		frame.at(index+12);

		/*
		 *	------------- one horizontal line
		 *	| * *
		 *	|* *
		 *	| * *	five progress bar lines
		 *	|* *
		 *	| * *
		 *	------------- one horizontal line
		 */
		drawHorizontalLine(index);
		for(uint16_t i = 1; i < (PROGRESS_BAR_HEIGHT-1); ++i) {
			drawProgressBarLine(index + (PBM_WIDTH_IN_BYTES * i), i);
		}
		drawHorizontalLine(index + (PBM_WIDTH_IN_BYTES * (PROGRESS_BAR_HEIGHT-1)));
	}
	catch (const std::out_of_range& oor) {
		std::ostringstream buffer(std::ios_base::app);
		buffer << "wrong frame index";
		GKSysLog(LOG_WARNING, WARNING, buffer.str());
	}
}

void LCDPlugin::drawPadlockOnFrame(
	const bool lockedPlugin,
	const uint16_t PBMXPos,
	const uint16_t PBMYPos)
{
	const uint16_t xByte = PBMXPos / 8;
	const uint16_t index = (PBM_WIDTH_IN_BYTES * PBMYPos) + xByte;

	PBMDataArray & frame = (*_itCurrentFrame)._PBMData;

	if( lockedPlugin ) {
		_everLocked = true;
		frame[index+(PBM_WIDTH_IN_BYTES * 0)] = 0b00000000;
		frame[index+(PBM_WIDTH_IN_BYTES * 1)] = 0b00110000;
		frame[index+(PBM_WIDTH_IN_BYTES * 2)] = 0b01001000;
		frame[index+(PBM_WIDTH_IN_BYTES * 3)] = 0b01111000;
		frame[index+(PBM_WIDTH_IN_BYTES * 4)] = 0b01111000;
		frame[index+(PBM_WIDTH_IN_BYTES * 5)] = 0b01111000;
	}
	else {
		if( _everLocked ) {
			frame[index+(PBM_WIDTH_IN_BYTES * 0)] = 0b00000110;
			frame[index+(PBM_WIDTH_IN_BYTES * 1)] = 0b00001001;
			frame[index+(PBM_WIDTH_IN_BYTES * 2)] = 0b00001000;
			frame[index+(PBM_WIDTH_IN_BYTES * 3)] = 0b01111000;
			frame[index+(PBM_WIDTH_IN_BYTES * 4)] = 0b01111000;
			frame[index+(PBM_WIDTH_IN_BYTES * 5)] = 0b01111000;
		}
		else {
			frame[index+(PBM_WIDTH_IN_BYTES * 0)] = 0;
			frame[index+(PBM_WIDTH_IN_BYTES * 1)] = 0;
			frame[index+(PBM_WIDTH_IN_BYTES * 2)] = 0;
			frame[index+(PBM_WIDTH_IN_BYTES * 3)] = 0;
			frame[index+(PBM_WIDTH_IN_BYTES * 4)] = 0;
			frame[index+(PBM_WIDTH_IN_BYTES * 5)] = 0;
		}
	}
}

void LCDPlugin::resetEverLocked(void)
{
	_everLocked = false;
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

std::tuple<uint16_t, uint16_t> LCDPlugin::getTempo(const LCDPluginTempo tempo)
{
	if(tempo == LCDPluginTempo::TEMPO_500_20)
		return std::make_tuple(500, 20);
	if(tempo == LCDPluginTempo::TEMPO_400_15)
		return std::make_tuple(400, 15);
	/* TEMPO_DEFAULT */
	return std::make_tuple(1000, 10);
}

} // namespace GLogiK

