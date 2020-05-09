/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2020  Fabrice Delliaux <netbox253@gmail.com>
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
#include "lib/shared/glogik.hpp"

#include "LCDScreenPluginsManager.hpp"

#include "include/enums.hpp"

#include "LCDPlugins/endscreen.hpp"
#include "LCDPlugins/splashscreen.hpp"
#include "LCDPlugins/systemMonitor.hpp"

namespace GLogiK
{

using namespace NSGKUtils;

const LCDPluginsPropertiesArray_type LCDScreenPluginsManager::_LCDPluginsPropertiesEmptyArray = {};

LCDScreenPluginsManager::LCDScreenPluginsManager()
	:	_frameCounter(0),
		_noPlugins(false),
		_currentPluginLocked(false),
		_pFonts(&_fontsManager)
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
			plugin->init(_pFonts);
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
	_LCDBuffer.fill(0x0);
}

LCDScreenPluginsManager::~LCDScreenPluginsManager() {
	this->stopLCDPlugins();
}

const LCDPluginsPropertiesArray_type & LCDScreenPluginsManager::getLCDPluginsProperties(void) const
{
	return _pluginsPropertiesArray;
}

const unsigned short LCDScreenPluginsManager::getPluginTiming(void)
{
	if(_itCurrentPlugin != _plugins.end() )
		return (*_itCurrentPlugin)->getPluginTiming();

	return 1000;
}

void LCDScreenPluginsManager::forceNextPlugin(void)
{
	if( ! _noPlugins ) {
		if(_itCurrentPlugin != _plugins.end() ) {
#if DEBUGGING_ON
			LOG(DEBUG2) << "force jump to next LCD plugin";
			if(_currentPluginLocked) {
				LOG(DEBUG3) << "LCD plugin unlocked : " << (*_itCurrentPlugin)->getPluginName();
			}
#endif
			/* force plugin unlocking. Locking mechanism can make
			 * LCD thread hang when stopping daemon */
			_currentPluginLocked = false;
			_frameCounter = (*_itCurrentPlugin)->getPluginMaxFrames();
		}
	}
}

LCDDataArray & LCDScreenPluginsManager::getNextLCDScreenBuffer(
	const std::string & LCDKey,
	const uint64_t defaultLCDPluginsMask1
) {
	if( ! _noPlugins ) {
		try {
			/* make sure there at least one plugin */
			if(_itCurrentPlugin != _plugins.end() ) {
				_frameCounter++;

				/* pressed L1, locked plugin ? */
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
					uint64_t LCDPluginsMask1 = defaultLCDPluginsMask1;

					if( LCDPluginsMask1 == 0 ) {
						/* default enabled plugins */
						LCDPluginsMask1 |= toEnumType(LCDScreenPlugin::GK_LCD_SPLASHSCREEN);
						LCDPluginsMask1 |= toEnumType(LCDScreenPlugin::GK_LCD_SYSTEM_MONITOR);
					}

					bool found = false;
					while( ! found ) {
						/* locked plugin ? */
						if( ! _currentPluginLocked ) {
							_itCurrentPlugin++; /* jumping to next plugin */
							if(_itCurrentPlugin == _plugins.end() )
								_itCurrentPlugin = _plugins.begin();

							/* reset everLocked boolean */
							(*_itCurrentPlugin)->resetEverLocked();
						}

						/* check that current plugin is enabled */
						if( LCDPluginsMask1 & (*_itCurrentPlugin)->getPluginID() )
							found = true;
					}

					/* reset frames to beginning */
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
			else /* else blank screen */
				_LCDBuffer.fill(0x0);
		}
		catch (const GLogiKExcept & e) {
			LOG(ERROR) << e.what();
			_LCDBuffer.fill(0x0);
		}

		std::fill_n(_LCDBuffer.begin(), LCD_BUFFER_OFFSET, 0);
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

