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

#include <algorithm>
#include <new>

#include "lib/utils/utils.hpp"
#include "lib/shared/glogik.hpp"

#include "LCDScreenPluginsManager.hpp"

#include "LCDPlugins/endscreen.hpp"
#include "LCDPlugins/splashscreen.hpp"
#include "LCDPlugins/systemMonitor.hpp"

#include "include/enums.hpp"

namespace GLogiK
{

using namespace NSGKUtils;

const LCDPluginsPropertiesArray_type LCDScreenPluginsManager::_LCDPluginsPropertiesEmptyArray = {};

LCDScreenPluginsManager::LCDScreenPluginsManager(const std::string & product)
	:	_pFonts(&_fontsManager),
		_frameCounter(0),
		_noPlugins(false),
		_currentPluginLocked(false)
{
	try {
		_plugins.push_back( new Splashscreen() );
		_plugins.push_back( new SystemMonitor() );
		_plugins.push_back( new Endscreen() );
	}
	catch (const std::bad_alloc& e) { /* handle new() failure */
		this->stopLCDPlugins();
		throw GLogiKBadAlloc("LCD screen plugin bad allocation");
	}

	/* initialize each plugin */
	for(const auto & plugin : _plugins) {
		try {
			plugin->init(_pFonts, product);
			_pluginsPropertiesArray.push_back( plugin->getPluginProperties() );
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
		_noPlugins = true;
	}

	/* initialize LCD frame container */
	_LCDBuffer.resize( DEFAULT_PBM_DATA_IN_BYTES + LCD_DATA_HEADER_OFFSET, 0 );
}

LCDScreenPluginsManager::~LCDScreenPluginsManager() {
	this->stopLCDPlugins();
}

const LCDPluginsPropertiesArray_type & LCDScreenPluginsManager::getLCDPluginsProperties(void) const
{
	return _pluginsPropertiesArray;
}

const uint16_t LCDScreenPluginsManager::getPluginTiming(void)
{
	if(_itCurrentPlugin != _plugins.end() )
		return (*_itCurrentPlugin)->getPluginTiming();

	return 1000;
}

void LCDScreenPluginsManager::unlockPlugin(void)
{
	if(_currentPluginLocked) {
#if DEBUGGING_ON
		if( ! _noPlugins ) {
			if(_itCurrentPlugin != _plugins.end() ) {
				LOG(DEBUG2) << "LCD plugin unlocked : " << (*_itCurrentPlugin)->getPluginName();
			}
		}
#endif
		_currentPluginLocked = false;
	}
}

const uint64_t LCDScreenPluginsManager::getCurrentPluginID(void)
{
	if( ! _noPlugins ) {
		if(_itCurrentPlugin != _plugins.end() ) {
			return (*_itCurrentPlugin)->getPluginID();
		}
	}

	return 0;
}

void LCDScreenPluginsManager::jumpToNextPlugin(void)
{
	if( ! _noPlugins ) {
		if(_itCurrentPlugin != _plugins.end() ) {
#if DEBUGGING_ON
			LOG(DEBUG2) << "jumping to next LCD plugin";
#endif
			/* make sure it is unlocked */
			_currentPluginLocked = false;
			_frameCounter = (*_itCurrentPlugin)->getPluginMaxFrames();
		}
	}
}

const bool LCDScreenPluginsManager::findOneLCDScreenPlugin(const uint64_t LCDPluginsMask1) const
{
	bool ret = false;

	if( ! _noPlugins ) {
		for(auto it = _plugins.cbegin(); it != _plugins.cend(); ++it) {
			/* check that current plugin is loaded */
			if( LCDPluginsMask1 & (*it)->getPluginID() ) {
				ret = true;
				break;
			}
		}
	}

	return ret;
}

PixelsData & LCDScreenPluginsManager::getNextLCDScreenBuffer(
	const std::string & LCDKey,
	const uint64_t LCDPluginsMask1
) {
	if( ! _noPlugins ) {
		try {
			/* make sure there at least one plugin */
			if(_itCurrentPlugin != _plugins.end() ) {
				_frameCounter++;

				/* pressed locking key ? */
				if(LCDKey == LCD_KEY_L2) {
					_currentPluginLocked = ! (_currentPluginLocked);
#if DEBUGGING_ON
					if( _currentPluginLocked ) {
						LOG(DEBUG3) << "LCD plugin   locked : " << (*_itCurrentPlugin)->getPluginName();
					}
					else {
						LOG(DEBUG3) << "LCD plugin unlocked : " << (*_itCurrentPlugin)->getPluginName();
					}
#endif
				}

				if( _frameCounter >= (*_itCurrentPlugin)->getPluginMaxFrames() ) {
					bool found = false;
					const std::vector<LCDPlugin*>::const_iterator itFirstPlugin = _itCurrentPlugin;
					while( ! found ) {
						/* locked plugin ? */
						if( ! _currentPluginLocked ) {
							_itCurrentPlugin++; /* jumping to next plugin */
							if(_itCurrentPlugin == _plugins.end() )
								_itCurrentPlugin = _plugins.begin();

							/* reset everLocked boolean */
							(*_itCurrentPlugin)->resetPluginEverLocked();
						}

						/* check that current plugin is enabled */
						if( LCDPluginsMask1 & (*_itCurrentPlugin)->getPluginID() )
							found = true;

						if( (! found) and (_itCurrentPlugin == itFirstPlugin) ) {
							const std::string warn("detected potential infinite loop");
							GKSysLog(LOG_WARNING, WARNING, warn);
							throw GLogiKExcept(warn);
						}
					}

					/* reset PBM frame to beginning */
					(*_itCurrentPlugin)->resetPBMFrameIndex();
					_frameCounter = 0;
				}
			}

			if(_itCurrentPlugin != _plugins.end() ) {
				(*_itCurrentPlugin)->prepareNextPBMFrame();
				this->dumpPBMDataIntoLCDBuffer(
					_LCDBuffer,
					(*_itCurrentPlugin)->getNextPBMFrame(_pFonts, LCDKey, _currentPluginLocked)
				);
			}
			else {
				/* blank screen */
				std::fill(_LCDBuffer.begin(), _LCDBuffer.end(), 0x0);
			}
		}
		catch (const GLogiKExcept & e) {
			LOG(ERROR) << e.what();
			std::fill(_LCDBuffer.begin(), _LCDBuffer.end(), 0x0);
		}

		std::fill_n(_LCDBuffer.begin(), LCD_DATA_HEADER_OFFSET, 0x0);
	}

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
 *	 .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  . ---> on DEFAULT_PBM_WIDTH_IN_BYTES bytes
 *	 .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  . --->
 *	 .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .  .
 *	                      |  |  |
 *	                       \   /
 *	                        \ /
 *	             on DEFAULT_PBM_HEIGHT bytes
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
void LCDScreenPluginsManager::dumpPBMDataIntoLCDBuffer(PixelsData & LCDBuffer, const PixelsData & PBMData)
{
	for(unsigned int row = 0; row < DEFAULT_PBM_HEIGHT_IN_BYTES; ++row) {
		unsigned int LCDCol = 0;
		unsigned int indexOffset = (DEFAULT_PBM_WIDTH * row);
		for(unsigned int PBMByte = 0; PBMByte < DEFAULT_PBM_WIDTH_IN_BYTES; ++PBMByte) {

			for(int bit = 7; bit > -1; --bit) {

#if 0 && DEBUGGING_ON
				LOG(DEBUG2)	<< "row: " << row
							<< " PBMByte: " << PBMByte
							<< " LCDCol: " << LCDCol
							<< " indexOffset: " << indexOffset
							<< " lcd_index: " << LCDCol + indexOffset
							<< " bit: " << bit << "\n";
#endif

				LCDBuffer[LCD_DATA_HEADER_OFFSET + LCDCol + indexOffset] =
					(((PBMData[PBMByte + (DEFAULT_PBM_WIDTH_IN_BYTES * 0) + indexOffset] >> bit) & 1) << 0 ) |
					(((PBMData[PBMByte + (DEFAULT_PBM_WIDTH_IN_BYTES * 1) + indexOffset] >> bit) & 1) << 1 ) |
					(((PBMData[PBMByte + (DEFAULT_PBM_WIDTH_IN_BYTES * 2) + indexOffset] >> bit) & 1) << 2 ) |
					(((PBMData[PBMByte + (DEFAULT_PBM_WIDTH_IN_BYTES * 3) + indexOffset] >> bit) & 1) << 3 ) |
					(((PBMData[PBMByte + (DEFAULT_PBM_WIDTH_IN_BYTES * 4) + indexOffset] >> bit) & 1) << 4 ) |
					(((PBMData[PBMByte + (DEFAULT_PBM_WIDTH_IN_BYTES * 5) + indexOffset] >> bit) & 1) << 5 ) |
					(((PBMData[PBMByte + (DEFAULT_PBM_WIDTH_IN_BYTES * 6) + indexOffset] >> bit) & 1) << 6 ) |
					(((PBMData[PBMByte + (DEFAULT_PBM_WIDTH_IN_BYTES * 7) + indexOffset] >> bit) & 1) << 7 ) ;

				LCDCol++;
			}
		}
	}
}


} // namespace GLogiK

