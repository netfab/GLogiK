/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2026  Fabrice Delliaux <netbox253@gmail.com>
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
#include <string_view>

#include "lib/utils/utils.hpp"

#include "bin/daemon/LCD/fontsManager.hpp"
#include "bin/daemon/LCD/PBM/PBMFile.hpp"
#include "bin/daemon/LCD/PBM/PBM.hpp"
#include "LCDPlugin.hpp"

namespace Managers::LCDPlugins::plugin
{

using namespace NSGKUtils;

/* -- -- -- */

pluginFrame::pluginFrame(const std::uint16_t num)
	:	_numFrames(num)
{
	/* initialize PBM frame container */
	_PBMData.resize(PBM::data_size_in_bytes, 0);
}

const bool pluginFrame::switchToNextFrame(const std::uint16_t currentFrameCounter)
{
	return (currentFrameCounter >= _numFrames);
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

const bool LCDPlugin::isInitialized() const
{
	return _initialized;
}

void LCDPlugin::resetFrameIndex()
{
	_itCurrentFrame = _frames.end();
	this->checkFrameIndex(); /* may throw */
}

const std::uint64_t LCDPlugin::getPluginID() const
{
	return _plugin.getID();
}

const std::string & LCDPlugin::getPluginName() const
{
	return _plugin.getName();
}

const std::uint16_t LCDPlugin::getPluginTiming() const
{
	std::uint16_t timing = 0;
	std::uint16_t maxFrames = 0;
	std::tie(timing, maxFrames) = this->getTempo(_pluginTempo);
	return timing;
}

const std::uint16_t LCDPlugin::getPluginMaxFrames() const
{
	std::uint16_t timing = 0;
	std::uint16_t maxFrames = 0;
	std::tie(timing, maxFrames) = this->getTempo(_pluginTempo);
	return maxFrames;
}

void LCDPlugin::prepareNextFrame()
{
	GK_LOG_FUNC

	/* update internal frame counter and iterator to allow the plugin
	 * to have multiples frames loaded and simulate animation */
	if( (*_itCurrentFrame).switchToNextFrame(_frameCounter) )
	{
		_itCurrentFrame++;
		this->checkFrameIndex(); /* may throw */
		_frameIndex = (_itCurrentFrame - _frames.begin());
		_frameCounter = 0;
	}

	_frameCounter++; /* for next call */
#if DEBUGGING_ON && DEBUG_LCD_PLUGINS
	GKLog3(trace, this->getPluginName(), " - frameCount #", _frameCounter)
#endif
}

void LCDPlugin::init(FontsManager* const pFonts, const std::string & product)
{
	GK_LOG_FUNC

	this->resetFrameIndex();
	GKLog2(trace, "initialized LCD plugin: ", this->getPluginName())
	_initialized = true;
}

const PixelsData & LCDPlugin::getNextFrame(
	FontsManager* const pFonts,
	const std::string & LCDKey,
	const bool lockedPlugin)
{
	this->drawPadlockOnFrame(lockedPlugin);
	return this->getCurrentFrame();
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

auto LCDPlugin::getPluginProperties() const -> const LCDPP
{
	return _plugin;
}

void LCDPlugin::addFrameFromPBM(
	const fs::path & PBMDirectory,
	const std::string & file,
	const std::uint16_t num)
{
	GK_LOG_FUNC

	fs::path filePath(PBMDirectory);
	filePath /= file;

	this->addEmptyFrame(num);

	PBM::PBMFile::readPBM(
		filePath.string(),
		_frames.back()._PBMData,
		PBM::default_width,
		PBM::default_height
	);
}

void LCDPlugin::addEmptyFrame(const std::uint16_t num)
{
	GK_LOG_FUNC

	try
	{
		_frames.emplace_back(num);
	}
	catch (const std::exception & e)
	{
		// pluginFrame constructor vector resize exception ?
		throw GLogiKExcept("failed to initialize empty frame");
	}
}

const std::uint16_t LCDPlugin::getNextFrameID() const
{
	return _frameIndex;
}

PixelsData & LCDPlugin::getCurrentFrame()
{
	GK_LOG_FUNC

#if DEBUGGING_ON && DEBUG_LCD_PLUGINS
	GKLog3(trace, this->getPluginName(), " - Frameindex: ", _frameIndex)
#endif
	return (*_itCurrentFrame)._PBMData;
}

void LCDPlugin::writeStringOnFrame(
	FontsManager* const pFonts,
	const FontID fontID,
	std::string_view sv,
	const int16_t PBMXPos,
	const int16_t PBMYPos)
{
	GK_LOG_FUNC

	try
	{
#if DEBUGGING_ON && DEBUG_LCD_PLUGINS
		if(GKLogging::GKDebug)
		{
			LOG(trace)	<< this->getPluginName()
						<< " - frameIndex: " << _frameIndex
						<< " - writing string: " << sv;
		}
#endif
		std::uint16_t XPos, YPos = 0;

		XPos = (PBMXPos < 0) ? /* centered */
				pFonts->getCenteredXPos(fontID, sv) :
				static_cast<std::uint16_t>(PBMXPos);

		YPos = (PBMYPos < 0) ? /* centered */
				pFonts->getCenteredYPos(fontID) :
				static_cast<std::uint16_t>(PBMYPos);

		std::size_t pos = 0;
		while( pos < sv.size() )
		{
			using PBMFont = Fonts::PBMFont;

			// length of the next character. Usually , length is 1 for ASCII characters
			// but for characters encoded in UTF-8, like "°", length is 2
			std::size_t length = PBMFont::characterLength(sv, pos);
			std::string_view nc = std::string_view(sv.data() + pos, length);

			pFonts->printCharacterOnFrame(fontID, (*_itCurrentFrame)._PBMData, nc, XPos, YPos);
			pos += length;
		}
	}
	catch (const GLogiKExcept & e)
	{
		GKSysLogWarning(e.what());
	}
}

void LCDPlugin::writeStringOnLastFrame(
	FontsManager* const pFonts,
	const FontID fontID,
	std::string_view sv,
	const int16_t PBMXPos,
	const int16_t PBMYPos)
{
	GK_LOG_FUNC

	try
	{
		if( _frames.empty() )
			throw GLogiKExcept("accessing last element on empty container");
		_itCurrentFrame = --(_frames.end());
		_frameIndex = (_itCurrentFrame - _frames.begin());

		this->writeStringOnFrame(pFonts, fontID, sv, PBMXPos, PBMYPos);
	}
	catch (const GLogiKExcept & e)
	{
		GKSysLogWarning(e.what());
	}
}

void LCDPlugin::drawProgressBarOnFrame(
	const std::uint16_t percent,
	const std::uint16_t PBMXPos,
	const std::uint16_t PBMYPos)
{
	GK_LOG_FUNC

	const std::uint16_t PROGRESS_BAR_WIDTH = 102;
	const std::uint16_t PROGRESS_BAR_HEIGHT = 7;

	if(PBMXPos + PROGRESS_BAR_WIDTH > (PBM::LCD_width - 1))
	{
		std::ostringstream buffer(std::ios_base::app);
		buffer << "wrong progress bar X position : " << PBMXPos;
		GKSysLogWarning(buffer.str());
		return;
	}

	try
	{
		PixelsData & frame = (*_itCurrentFrame)._PBMData;

		// lambda
		auto draw_horizontal_line = [&frame] (const std::uint16_t index) -> void
		{
			frame[index+12] |= 0b11111100;
			for(std::uint16_t i = 0; i < 12; ++i)
				frame[index+i] = 0b11111111;
		};

		// lambda
		auto draw_progress_bar_line = [&frame, &percent]
			(const std::uint16_t index, const std::uint16_t line) -> void
		{
			const unsigned char B10 = 0b10101010;
			const unsigned char B01 = 0b01010101;
			const unsigned char B11 = 0b11111111;

			const std::uint16_t percentByte = percent / 8;
			const std::uint16_t percentModulo = percent % 8;
			/* -1 to consider the bar left border */
			const std::uint16_t leftShift = ((8-1) - percentModulo);

			const unsigned char & Byte1 = (line % 2 == 1) ? B10 : B01;
			const unsigned char & Byte2 = (line % 2 == 1) ? B01 : B10;
			const unsigned char & cByte = (leftShift % 2 == 1) ? Byte2 : Byte1;

			// lambda
			auto get_shifted_byte = [&leftShift]
				(const unsigned char & b) -> unsigned char
			{
				return static_cast<unsigned char>( b << leftShift );
			};

#if DEBUGGING_ON && DEBUG_LCD_PLUGINS
			if(GKLogging::GKDebug)
			{
				LOG(trace)	<< "index: " << index
							<< " line: " << line
							<< " pByte: " << percentByte
							<< " pModulo: " << percentModulo
							<< " leftShift: " << leftShift;
			}
#endif

			for(std::uint16_t i = 0; i < 12; ++i)
			{
				if((i == 0) and (percent < 8))
					frame[index+i] = get_shifted_byte(cByte);
				else if(i < percentByte)
					frame[index+i] = (i < 8) ? Byte1 : B11;
				else if(i == percentByte)
					frame[index+i] = (i < 8) ? get_shifted_byte(cByte) : get_shifted_byte(B11);
				else
					frame[index+i] = 0;
			}

			frame[index+12] = (percentByte == 12) ? get_shifted_byte(B11) : 0;

			frame[index]    |= 0b10000000;	/* left border */
			frame[index+12] |= 0b00000100;	/* right border */
		};

		const std::uint16_t xByte = PBMXPos / 8;
		const std::uint16_t index = (PBM::default_width_in_bytes * PBMYPos) + xByte;

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
		draw_horizontal_line(index);
		for(std::uint16_t i = 1; i < (PROGRESS_BAR_HEIGHT-1); ++i)
			draw_progress_bar_line(index + (PBM::default_width_in_bytes * i), i);
		draw_horizontal_line(index + (PBM::default_width_in_bytes * (PROGRESS_BAR_HEIGHT-1)));
	}
	catch (const std::out_of_range& oor)
	{
		GKSysLogWarning("wrong frame index");
	}
}

void LCDPlugin::drawPadlockOnFrame(
	const bool lockedPlugin,
	const std::uint16_t PBMXPos,
	const std::uint16_t PBMYPos)
{
	const std::uint16_t xByte = PBMXPos / 8;
	const std::uint16_t index = (PBM::default_width_in_bytes * PBMYPos) + xByte;

	PixelsData & frame = (*_itCurrentFrame)._PBMData;

	if( lockedPlugin )
	{
		_everLocked = true;
		frame[index+(PBM::default_width_in_bytes * 0)] = 0b00000000;
		frame[index+(PBM::default_width_in_bytes * 1)] = 0b00110000;
		frame[index+(PBM::default_width_in_bytes * 2)] = 0b01001000;
		frame[index+(PBM::default_width_in_bytes * 3)] = 0b01111000;
		frame[index+(PBM::default_width_in_bytes * 4)] = 0b01111000;
		frame[index+(PBM::default_width_in_bytes * 5)] = 0b01111000;
	}
	else
	{
		if( _everLocked )
		{
			frame[index+(PBM::default_width_in_bytes * 0)] = 0b00000110;
			frame[index+(PBM::default_width_in_bytes * 1)] = 0b00001001;
			frame[index+(PBM::default_width_in_bytes * 2)] = 0b00001000;
			frame[index+(PBM::default_width_in_bytes * 3)] = 0b01111000;
			frame[index+(PBM::default_width_in_bytes * 4)] = 0b01111000;
			frame[index+(PBM::default_width_in_bytes * 5)] = 0b01111000;
		}
		else
		{
			frame[index+(PBM::default_width_in_bytes * 0)] = 0;
			frame[index+(PBM::default_width_in_bytes * 1)] = 0;
			frame[index+(PBM::default_width_in_bytes * 2)] = 0;
			frame[index+(PBM::default_width_in_bytes * 3)] = 0;
			frame[index+(PBM::default_width_in_bytes * 4)] = 0;
			frame[index+(PBM::default_width_in_bytes * 5)] = 0;
		}
	}
}

void LCDPlugin::drawVerticalLineOnFrame(
	const std::uint16_t PBMXPos,
	const std::uint16_t PBMYPos,
	const std::uint16_t size)
{
	try
	{
		PixelsData & frame = (*_itCurrentFrame)._PBMData;

		const std::uint16_t xByte = PBMXPos / 8;
		const std::uint16_t index = (PBM::default_width_in_bytes * PBMYPos) + xByte;

		for(std::uint16_t i = 1; i < size; ++i)
			frame[index + (PBM::default_width_in_bytes * i)] |= 0b00100000;
	}
	catch (const std::out_of_range& oor)
	{
		GKSysLogWarning("wrong frame index");
	}
}

void LCDPlugin::resetPluginEverLocked()
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

void LCDPlugin::checkFrameIndex()
{
	if( _itCurrentFrame == _frames.end() )
	{
		_itCurrentFrame = _frames.begin();

		if( _itCurrentFrame == _frames.end() )
			throw GLogiKExcept("plugin frame iterator exception");

		_frameCounter = 0;
		_frameIndex = 0;
	}
}

std::tuple<std::uint16_t, std::uint16_t> LCDPlugin::getTempo(const LCDPluginTempo tempo)
{
	if(tempo == LCDPluginTempo::TEMPO_500_20)
		return std::make_tuple(500, 20);
	if(tempo == LCDPluginTempo::TEMPO_400_15)
		return std::make_tuple(400, 15);
	/* TEMPO_DEFAULT */
	return std::make_tuple(1000, 10);
}

} // namespace Managers::LCDPlugins::plugin
