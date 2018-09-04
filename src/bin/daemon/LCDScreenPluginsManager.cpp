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

#include <new>

#include "lib/utils/utils.hpp"

#include "LCDScreenPluginsManager.hpp"

#include "LCDPlugins/splashscreen.hpp"
#include "LCDPlugins/systemMonitor.hpp"

namespace GLogiK
{

using namespace NSGKUtils;

LCDScreenPluginsManager::LCDScreenPluginsManager()
	:	_frameCounter(0),
		_pFonts(&_fontsManager)
{
	try {
		_plugins.push_back( new Splashscreen() );
		_plugins.push_back( new SystemMonitor() );
	}
	catch (const std::bad_alloc& e) { /* handle new() failure */
		this->stopLCDPlugins();
		throw GLogiKBadAlloc("LCD screen plugin bad allocation");
	}

	/* initialize each plugin */
	for(const auto & plugin : _plugins) {
		try {
			plugin->init(_pFonts);
		}
		catch (const GLogiKExcept & e) {
			LOG(ERROR) << e.what();
		}
	}

	/* delete all uninitialized plugins
	 * (the ones that failed to read PBM for whatever
	 * reason during initialization)
	 */
	for(auto & plugin : _plugins) {
		if( ! plugin->isInitialized() ) {
			delete plugin; plugin = nullptr;
		}
	}

	auto is_null = [] (auto & item) -> const bool { return (item == nullptr); };
	_plugins.erase(
		std::remove_if(_plugins.begin(), _plugins.end(), is_null),
		_plugins.end()
	);

	_itCurrentPlugin = _plugins.begin();
	if( _plugins.empty() ) {
		GKSysLog(LOG_WARNING, WARNING, "no LCD screen plugin initialized");
	}
	_LCDBuffer.fill(0x0);
}

LCDScreenPluginsManager::~LCDScreenPluginsManager() {
	this->stopLCDPlugins();
}

const unsigned short LCDScreenPluginsManager::getPluginTiming(void)
{
	if(_itCurrentPlugin != _plugins.end() )
		return (*_itCurrentPlugin)->getPluginTiming();

	return 1000;
}

LCDDataArray & LCDScreenPluginsManager::getNextLCDScreenBuffer(const std::string & LCDKey)
{
	try {
		/* make sure there at least one plugin */
		if(_itCurrentPlugin != _plugins.end() ) {
			_frameCounter++;

			if( _frameCounter >= (*_itCurrentPlugin)->getPluginMaxFrames() ) {
				_itCurrentPlugin++;
				if(_itCurrentPlugin == _plugins.end() )
					_itCurrentPlugin = _plugins.begin();

				/* reset frames to beginning */
				(*_itCurrentPlugin)->resetPBMFrameIndex();
				_frameCounter = 0;
			}
		}

		if(_itCurrentPlugin != _plugins.end() ) {
			(*_itCurrentPlugin)->prepareNextPBMFrame();
			this->dumpPBMDataIntoLCDBuffer(
				_LCDBuffer,
				(*_itCurrentPlugin)->getNextPBMFrame(_pFonts, LCDKey)
			);
		}
		else /* else blank screen */
			_LCDBuffer.fill(0x0);
	}
	catch (const GLogiKExcept & e) {
		LOG(ERROR) << e.what();
		_LCDBuffer.fill(0x0);
	}

	std::fill_n(_LCDBuffer.begin(), LCD_BUFFER_OFFSET, 0);
	/* the keyboard needs this magic byte */
	_LCDBuffer[0] = 0x03;

	return _LCDBuffer;
}

void LCDScreenPluginsManager::stopLCDPlugins(void) {
#if DEBUGGING_ON
	LOG(DEBUG2) << "stopping LCD screen plugins";
#endif
	for(const auto & plugin : _plugins) {
		delete plugin;
	}
	_plugins.clear();
}

/*
 * -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
 * PBM data binary format (without header), for a set of bytes (A, B, C, ...)
 * -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
 *	A7 A6 A5 A4 A3 A2 A1 A0 B7 B6 B5 B4 B3 B2 B1 B0  .  .
 *	U7 U6 U5 U4 U3 U2 U1 U0  .  .  .  .  .  .  .  .  .  . --->
 *	 .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  . ---> on PBM_WIDTH_IN_BYTES bytes
 *	 .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  . --->
 *	 .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .
 *	                      |  |  |
 *	                       \   /
 *	                        \ /
 *	                 on PBM_HEIGHT bytes
 *
 * -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
 * LCD data binary format (without header). Description coming from libg15.
 * For a set of bytes (A, B, C, etc.) the bits representing pixels will
 * appear on the LCD like this.
 * -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- -- --
 *	A0 B0 C0
 *	A1 B1 C1
 *	A2 B2 C2
 *	A3 B3 C3 ... and across for G15_LCD_WIDTH bytes
 *	A4 B4 C4
 *	A5 B5 C5
 *	A6 B6 C6
 *	A7 B7 C7
 *
 *	A0
 *	A1	<- second 8-pixel-high row starts straight after the last byte on
 *	A2		the previous row
 *	A3
 *	A4
 *	A5
 *	A6
 *	A7
 *
 *	A0
 *	 .
 *	 .
 *
 *	A0
 *	 .
 *	 .
 *
 *	A0
 *	 .
 *	 .
 *
 *	A0
 *	A1	<- only the first three bits are shown on the bottom row (the last three
 *	A2		pixels of the 43-pixel high display.)
 *
 */
void LCDScreenPluginsManager::dumpPBMDataIntoLCDBuffer(LCDDataArray & LCDBuffer, const PBMDataArray & PBMData)
{
	for(unsigned int row = 0; row < PBM_HEIGHT_IN_BYTES; ++row) {
		unsigned int LCDCol = 0;
		unsigned int indexOffset = (PBM_WIDTH * row);
		for(unsigned int PBMByte = 0; PBMByte < PBM_WIDTH_IN_BYTES; ++PBMByte) {

			for(int bit = 7; bit > -1; --bit) {

#if 0 && DEBUGGING_ON
				LOG(DEBUG2)	<< "row: " << row
							<< " PBMByte: " << PBMByte
							<< " LCDCol: " << LCDCol
							<< " indexOffset: " << indexOffset
							<< " lcd_index: " << LCDCol + indexOffset
							<< " bit: " << bit << "\n";
#endif

				LCDBuffer[LCD_BUFFER_OFFSET + LCDCol + indexOffset] =
					(((PBMData[PBMByte + (PBM_WIDTH_IN_BYTES * 0) + indexOffset] >> bit) & 1) << 0 ) |
					(((PBMData[PBMByte + (PBM_WIDTH_IN_BYTES * 1) + indexOffset] >> bit) & 1) << 1 ) |
					(((PBMData[PBMByte + (PBM_WIDTH_IN_BYTES * 2) + indexOffset] >> bit) & 1) << 2 ) |
					(((PBMData[PBMByte + (PBM_WIDTH_IN_BYTES * 3) + indexOffset] >> bit) & 1) << 3 ) |
					(((PBMData[PBMByte + (PBM_WIDTH_IN_BYTES * 4) + indexOffset] >> bit) & 1) << 4 ) |
					(((PBMData[PBMByte + (PBM_WIDTH_IN_BYTES * 5) + indexOffset] >> bit) & 1) << 5 ) |
					(((PBMData[PBMByte + (PBM_WIDTH_IN_BYTES * 6) + indexOffset] >> bit) & 1) << 6 ) |
					(((PBMData[PBMByte + (PBM_WIDTH_IN_BYTES * 7) + indexOffset] >> bit) & 1) << 7 ) ;

				LCDCol++;
			}
		}
	}
}


} // namespace GLogiK

