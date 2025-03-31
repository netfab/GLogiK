/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2025  Fabrice Delliaux <netbox253@gmail.com>
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

PBMFrame::PBMFrame(const uint16_t num)
	:	_numFrames(num)
{
	/* initialize PBM frame container */
	_PBMData.resize( DEFAULT_PBM_DATA_IN_BYTES, 0 );
}

const bool PBMFrame::switchToNextFrame(const uint16_t currentFrameCounter)
{
	return (currentFrameCounter >= _numFrames);
}

/* -- -- -- */

LCDPlugin::LCDPlugin()
	:	_pluginTempo(LCDPluginTempo::TEMPO_DEFAULT),
		_initialized(false),
		_everLocked(false),
		_PBMFrameCounter(0),
		_PBMFrameIndex(0)
{
}

LCDPlugin::~LCDPlugin()
{
	GK_LOG_FUNC

	GKLog2(trace, "deleting LCD plugin : ", this->getPluginName())
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
	_itCurrentPBMFrame = _PBMFrames.end();
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
	GK_LOG_FUNC

	/* update internal frame counter and iterator to allow the plugin
	 * to have multiples PBM loaded and simulate animation */
	if( (*_itCurrentPBMFrame).switchToNextFrame(_PBMFrameCounter) ) {
		_itCurrentPBMFrame++;
		this->checkPBMFrameIndex(); /* may throw */
		_PBMFrameIndex = (_itCurrentPBMFrame - _PBMFrames.begin());
		_PBMFrameCounter = 0;
	}

	_PBMFrameCounter++; /* for next call */
#if DEBUGGING_ON && DEBUG_LCD_PLUGINS
	GKLog3(trace, this->getPluginName(), " - frameCount #", _PBMFrameCounter)
#endif
}

void LCDPlugin::init(FontsManager* const pFonts, const std::string & product)
{
	GK_LOG_FUNC

	this->resetPBMFrameIndex();
	GKLog2(trace, "initialized LCD plugin: ", this->getPluginName())
	_initialized = true;
}

const PixelsData & LCDPlugin::getNextPBMFrame(
	FontsManager* const pFonts,
	const std::string & LCDKey,
	const bool lockedPlugin)
{
	this->drawPadlockOnPBMFrame(lockedPlugin);
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

const LCDPP LCDPlugin::getPluginProperties(void) const
{
	return _plugin;
}

void LCDPlugin::addPBMFrame(
	const fs::path & PBMDirectory,
	const std::string & file,
	const uint16_t num)
{
	GK_LOG_FUNC

	fs::path filePath(PBMDirectory);
	filePath /= file;

	this->addPBMEmptyFrame(num);

	this->readPBM(
		filePath.string(),
		_PBMFrames.back()._PBMData,
		DEFAULT_PBM_WIDTH,
		DEFAULT_PBM_HEIGHT
	);
}

void LCDPlugin::addPBMEmptyFrame(const uint16_t num)
{
	GK_LOG_FUNC

	try {
		_PBMFrames.emplace_back(num);
	}
	catch (const std::exception & e) {
		// PBMFrame constructor vector resize exception ?
		throw GLogiKExcept("failed to initialize PBM empty frame");
	}
}

const uint16_t LCDPlugin::getNextPBMFrameID(void) const
{
	return _PBMFrameIndex;
}

PixelsData & LCDPlugin::getCurrentPBMFrame(void)
{
	GK_LOG_FUNC

#if DEBUGGING_ON && DEBUG_LCD_PLUGINS
	GKLog3(trace, this->getPluginName(), " - PBMFrameindex: ", _PBMFrameIndex)
#endif
	return (*_itCurrentPBMFrame)._PBMData;
}

void LCDPlugin::writeStringOnPBMFrame(
	FontsManager* const pFonts,
	const FontID fontID,
	const std::string & string,
	const int16_t PBMXPos,
	const int16_t PBMYPos)
{
	GK_LOG_FUNC

	try {
#if DEBUGGING_ON && DEBUG_LCD_PLUGINS
		if(GKLogging::GKDebug) {
			LOG(trace)	<< this->getPluginName()
						<< " - PBMFrameindex: " << _PBMFrameIndex
						<< " - writing string : " << string;
		}
#endif
		uint16_t XPos, YPos = 0;

		XPos = (PBMXPos < 0) ? /* centered */
				pFonts->getCenteredXPos(fontID, string) :
				static_cast<uint16_t>(PBMXPos);

		YPos = (PBMYPos < 0) ? /* centered */
				pFonts->getCenteredYPos(fontID) :
				static_cast<uint16_t>(PBMYPos);

		for(const char & c : string) {
			const std::string character(1, c);
			pFonts->printCharacterOnFrame( fontID, (*_itCurrentPBMFrame)._PBMData, character, XPos, YPos );
		} /* for each character in the string */
	}
	catch (const GLogiKExcept & e) {
		GKSysLogWarning(e.what());
	}
}

void LCDPlugin::writeStringOnLastPBMFrame(
	FontsManager* const pFonts,
	const FontID fontID,
	const std::string & string,
	const int16_t PBMXPos,
	const int16_t PBMYPos)
{
	GK_LOG_FUNC

	try {
		if( _PBMFrames.empty() )
			throw GLogiKExcept("accessing last element on empty container");
		_itCurrentPBMFrame = --(_PBMFrames.end());
		_PBMFrameIndex = (_itCurrentPBMFrame - _PBMFrames.begin());

		this->writeStringOnPBMFrame(pFonts, fontID, string, PBMXPos, PBMYPos);
	}
	catch (const GLogiKExcept & e) {
		GKSysLogWarning(e.what());
	}
}

void LCDPlugin::drawProgressBarOnPBMFrame(
	const uint16_t percent,
	const uint16_t PBMXPos,
	const uint16_t PBMYPos)
{
	GK_LOG_FUNC

	const uint16_t PROGRESS_BAR_WIDTH = 102;
	const uint16_t PROGRESS_BAR_HEIGHT = 7;

	if(PBMXPos + PROGRESS_BAR_WIDTH > (LCD_SCREEN_WIDTH - 1)) {
		std::ostringstream buffer(std::ios_base::app);
		buffer << "wrong progress bar X position : " << PBMXPos;
		GKSysLogWarning(buffer.str());
		return;
	}

	try {
		PixelsData & frame = (*_itCurrentPBMFrame)._PBMData;

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
			if(GKLogging::GKDebug) {
				LOG(trace)	<< "index: " << index
							<< " line: " << line
							<< " pByte: " << percentByte
							<< " pModulo: " << percentModulo
							<< " leftShift: " << leftShift;
			}
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
		const uint16_t index = (DEFAULT_PBM_WIDTH_IN_BYTES * PBMYPos) + xByte;

		/* checking for out of range before progress bar drawing */
		[[maybe_unused]] const auto & pos = frame.at(index+12);

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
			drawProgressBarLine(index + (DEFAULT_PBM_WIDTH_IN_BYTES * i), i);
		}
		drawHorizontalLine(index + (DEFAULT_PBM_WIDTH_IN_BYTES * (PROGRESS_BAR_HEIGHT-1)));
	}
	catch (const std::out_of_range& oor) {
		GKSysLogWarning("wrong frame index");
	}
}

void LCDPlugin::drawPadlockOnPBMFrame(
	const bool lockedPlugin,
	const uint16_t PBMXPos,
	const uint16_t PBMYPos)
{
	const uint16_t xByte = PBMXPos / 8;
	const uint16_t index = (DEFAULT_PBM_WIDTH_IN_BYTES * PBMYPos) + xByte;

	PixelsData & frame = (*_itCurrentPBMFrame)._PBMData;

	if( lockedPlugin ) {
		_everLocked = true;
		frame[index+(DEFAULT_PBM_WIDTH_IN_BYTES * 0)] = 0b00000000;
		frame[index+(DEFAULT_PBM_WIDTH_IN_BYTES * 1)] = 0b00110000;
		frame[index+(DEFAULT_PBM_WIDTH_IN_BYTES * 2)] = 0b01001000;
		frame[index+(DEFAULT_PBM_WIDTH_IN_BYTES * 3)] = 0b01111000;
		frame[index+(DEFAULT_PBM_WIDTH_IN_BYTES * 4)] = 0b01111000;
		frame[index+(DEFAULT_PBM_WIDTH_IN_BYTES * 5)] = 0b01111000;
	}
	else {
		if( _everLocked ) {
			frame[index+(DEFAULT_PBM_WIDTH_IN_BYTES * 0)] = 0b00000110;
			frame[index+(DEFAULT_PBM_WIDTH_IN_BYTES * 1)] = 0b00001001;
			frame[index+(DEFAULT_PBM_WIDTH_IN_BYTES * 2)] = 0b00001000;
			frame[index+(DEFAULT_PBM_WIDTH_IN_BYTES * 3)] = 0b01111000;
			frame[index+(DEFAULT_PBM_WIDTH_IN_BYTES * 4)] = 0b01111000;
			frame[index+(DEFAULT_PBM_WIDTH_IN_BYTES * 5)] = 0b01111000;
		}
		else {
			frame[index+(DEFAULT_PBM_WIDTH_IN_BYTES * 0)] = 0;
			frame[index+(DEFAULT_PBM_WIDTH_IN_BYTES * 1)] = 0;
			frame[index+(DEFAULT_PBM_WIDTH_IN_BYTES * 2)] = 0;
			frame[index+(DEFAULT_PBM_WIDTH_IN_BYTES * 3)] = 0;
			frame[index+(DEFAULT_PBM_WIDTH_IN_BYTES * 4)] = 0;
			frame[index+(DEFAULT_PBM_WIDTH_IN_BYTES * 5)] = 0;
		}
	}
}

void LCDPlugin::drawVerticalLineOnPBMFrame(
	const uint16_t PBMXPos,
	const uint16_t PBMYPos,
	const uint16_t size)
{
	try {
		PixelsData & frame = (*_itCurrentPBMFrame)._PBMData;

		const uint16_t xByte = PBMXPos / 8;
		const uint16_t index = (DEFAULT_PBM_WIDTH_IN_BYTES * PBMYPos) + xByte;

		for(uint16_t i = 1; i < size; ++i) {
			frame[index + (DEFAULT_PBM_WIDTH_IN_BYTES * i)] |= 0b00100000;
		}
	}
	catch (const std::out_of_range& oor) {
		GKSysLogWarning("wrong frame index");
	}
}

void LCDPlugin::resetPluginEverLocked(void)
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
	if( _itCurrentPBMFrame == _PBMFrames.end() ) {
		_itCurrentPBMFrame = _PBMFrames.begin();

		if( _itCurrentPBMFrame == _PBMFrames.end() )
			throw GLogiKExcept("plugin frame iterator exception");

		_PBMFrameCounter = 0;
		_PBMFrameIndex = 0;
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

