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

#include "lib/utils/utils.h"

#include "LCDScreenPluginsManager.h"

#include "LCDPlugins/foo.h"

namespace GLogiK
{

using namespace NSGKUtils;

LCDScreenPluginsManager::LCDScreenPluginsManager() {
	try {
		this->plugins_.push_back( new foo() );
	}
	catch (const std::bad_alloc& e) { /* handle new() failure */
		this->stopLCDPlugins();
		throw GLogiKBadAlloc("LCD screen plugin bad allocation");
	}

	/* initialize each plugin */
	for(const auto & plugin : this->plugins_) {
		try {
			plugin->init();
		}
		catch (const GLogiKExcept & e) {
			LOG(ERROR) << e.what();
		}
	}
}

LCDScreenPluginsManager::~LCDScreenPluginsManager() {
	this->stopLCDPlugins();
}

void LCDScreenPluginsManager::stopLCDPlugins(void) {
#if DEBUGGING_ON
	LOG(DEBUG2) << "stopping LCD screen plugins";
#endif
	for(const auto & plugin : this->plugins_) {
		delete plugin;
	}
	this->plugins_.clear();
}


void LCDScreenPluginsManager::dumpPBMDataIntoLCDBuffer(PBMDataArray & lcd_buffer, const PBMDataArray & pbm_data)
{
	for(unsigned int row = 0; row < PBM_HEIGHT_IN_BYTES; ++row) {
		unsigned int lcd_col = 0;
		unsigned int index_offset = (PBM_WIDTH * row);
		for(unsigned int pbm_byte = 0; pbm_byte < PBM_WIDTH_IN_BYTES; ++pbm_byte) {

			for(int bit = 7; bit > -1; --bit) {

// FIXME
#if 0 && DEBUGGING_ON
				LOG(DEBUG2)	<< "row: " << row
							<< " pbm_byte: " << pbm_byte
							<< " lcd_col: " << lcd_col
							<< " index_offset: " << index_offset
							<< " lcd_index: " << lcd_col + index_offset
							<< " bit: " << bit << "\n";
#endif

				lcd_buffer[lcd_col + index_offset] =
					(((pbm_data[pbm_byte + (PBM_WIDTH_IN_BYTES * 0) + index_offset] >> bit) & 1) << 0 ) |
					(((pbm_data[pbm_byte + (PBM_WIDTH_IN_BYTES * 1) + index_offset] >> bit) & 1) << 1 ) |
					(((pbm_data[pbm_byte + (PBM_WIDTH_IN_BYTES * 2) + index_offset] >> bit) & 1) << 2 ) |
					(((pbm_data[pbm_byte + (PBM_WIDTH_IN_BYTES * 3) + index_offset] >> bit) & 1) << 3 ) |
					(((pbm_data[pbm_byte + (PBM_WIDTH_IN_BYTES * 4) + index_offset] >> bit) & 1) << 4 ) |
					(((pbm_data[pbm_byte + (PBM_WIDTH_IN_BYTES * 5) + index_offset] >> bit) & 1) << 5 ) |
					(((pbm_data[pbm_byte + (PBM_WIDTH_IN_BYTES * 6) + index_offset] >> bit) & 1) << 6 ) |
					(((pbm_data[pbm_byte + (PBM_WIDTH_IN_BYTES * 7) + index_offset] >> bit) & 1) << 7 ) ;

				lcd_col++;
			}
		}
	}
}


} // namespace GLogiK

