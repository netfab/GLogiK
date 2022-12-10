/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2022  Fabrice Delliaux <netbox253@gmail.com>
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

#include <cstdint>

#include <string>

#include <boost/filesystem.hpp>

#include <config.h>

#include "include/enums.hpp"

#include "lib/utils/utils.hpp"

#include "coretemp.hpp"

namespace fs = boost::filesystem;

namespace GLogiK
{

using namespace NSGKUtils;

std::vector<std::string> Coretemp::coretempIDs;

const std::string Coretemp::seekDirectoryPath(
	const std::string & basedir,
	const unsigned int start)
{
	unsigned int x = start;
	std::string out;

	try {
		while(true) {
			fs::path searched(basedir);
			searched += std::to_string(x);
			if(fs::exists(searched))
				if(fs::is_directory(searched)) {
					out = searched.string();
					break;
				}

			++x;
			
			if(x == 12) // FIXME
				break;
			}
	}
	catch (const fs::filesystem_error & e) {
		GKSysLogError("boost::filesystem error", e.what());
		throw GLogiKExcept("seek directory path error");
	}
	catch (const std::exception & e) {
		GKSysLogError("boost::filesystem (allocation) error", e.what());
		throw GLogiKExcept("seek directory path error");
	}

	return out;
}

const std::vector<std::string> & Coretemp::getCoretempID(void)
{
	GK_LOG_FUNC
	
	const std::string basedir("/sys/devices/platform/coretemp.");

	unsigned int x = 0;
	while(true) {
		try {
			const std::string rootDir = Coretemp::seekDirectoryPath(basedir, x);
			if(rootDir.empty())
				break;
			Coretemp::coretempIDs.push_back(rootDir);
		}
		catch (const GLogiKExcept & e) {
			GKSysLogWarning("can't get coretemp root directory: ", e.what());
		}

		++x;
	}

	GKLog2(trace, "coretemp ID size : ", Coretemp::coretempIDs.size())

	return Coretemp::coretempIDs;
}

Coretemp::Coretemp(const std::string & coretempID)
{
	_plugin.setID( toEnumType(LCDScreenPlugin::GK_LCD_CORETEMP) );
	_plugin.setName("coretemp");
	_plugin.setDesc("Coretemp plugin, used to get packages/cores temperatures from Intel CPUs");
	_pluginTempo = LCDPluginTempo::TEMPO_500_20;
	_coretempID = coretempID;
}

Coretemp::~Coretemp()
{
}

void Coretemp::init(FontsManager* const pFonts, const std::string & product)
{
	GK_LOG_FUNC

	GKLog2(trace, "coretemp root directory: ", _coretempID)

	fs::path PBMDirectory(PBM_DATA_DIR);
	PBMDirectory /= _plugin.getName();

	this->addPBMFrame(PBMDirectory, "skeleton.pbm"); /* frame #0 */

	std::string basedir(_coretempID); basedir += "/hwmon/hwmon";
	_hwmonID = this->seekDirectoryPath(basedir); /* may throw */

	GKLog2(trace, "hwmon root directory: ", _hwmonID)

	this->writeStringOnLastPBMFrame(pFonts, FontID::MONOSPACE85, "intel cpu temperatures", 16, 1);

	LCDPlugin::init(pFonts, product);
}

const PixelsData & Coretemp::getNextPBMFrame(
	FontsManager* const pFonts,
	const std::string & LCDKey,
	const bool lockedPlugin)
{
	GK_LOG_FUNC

	this->drawPadlockOnPBMFrame(lockedPlugin);

	struct dev {
		std::string fullstr;
		std::string input;
		std::string label;
		std::string id;
		bool isPkg;
	};

	std::vector<dev> hwmon;
	
	try {
		unsigned short x = 1;

		std::ifstream infile;
		infile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

		while(true) {
			std::string inputFile = _hwmonID;
			inputFile += "/temp";
			inputFile += std::to_string(x);
			inputFile += "_input";
		
			std::string labelFile = _hwmonID;
			labelFile += "/temp";
			labelFile += std::to_string(x);
			labelFile += "_label";
		
			dev device;

			infile.open(inputFile);
			std::getline(infile, device.input);
			infile.close();

			infile.open(labelFile);
			std::getline(infile, device.label);
			infile.close();

			device.isPkg = (device.label.substr(0, 11) == "Package id ") ? true : false;
			device.id = (device.isPkg) ? device.label.substr(11) : device.label.substr(5);

			auto & s = device.fullstr;
			s  = device.label; s += " ";
			s += device.input; s += " ";
			s += device.id;    s += " ";
			s += (device.isPkg) ? "pkg: true" : "pkg: false";

			hwmon.push_back(device);

			x++;
		}
	}
	catch (const std::ifstream::failure & e) {
#if DEBUGGING_ON && DEBUG_LCD_PLUGINS
		GKLog2(trace, "hwmon size: ", hwmon.size())
#endif
		if( hwmon.empty() ) {
			GKSysLogError("error opening/reading/closing file : ", e.what());
			throw GLogiKExcept("ifstream error");
		}
	}

	const uint16_t TEMP_POS_X = 36;
	const uint16_t TEMP_POS_Y = 22;

	uint16_t pos_x = TEMP_POS_X;
	uint16_t pos_y = TEMP_POS_Y;

	unsigned short x = 0;

	auto updatedPosX = [&pos_x] () -> const auto & {
		const uint16_t MONOSPACE85_CHARACTER_WIDTH = 5;
		const uint16_t num_chars = 6; // 0:100°

		pos_x += ((MONOSPACE85_CHARACTER_WIDTH * num_chars) + 2);
		return pos_x;
	};

	for(const auto & device : hwmon) {
		if((x > 0) and (x % 2) == 0) {
			pos_y = TEMP_POS_Y;
			this->drawVerticalLineOnPBMFrame(updatedPosX(), (TEMP_POS_Y - 2), 23);
		}

#if DEBUGGING_ON && DEBUG_LCD_PLUGINS
		GKLog2(trace, "device: ", device.fullstr)
#endif

		std::string temp(device.id);
		temp += ":";
		/* Temperature is measured in degrees Celsius and measurement resolution is 1 degree C. */
		temp += device.input.substr(0, (device.input.size() - 3));
		temp += PBMFont::deg;

		if(device.isPkg)
			this->writeStringOnPBMFrame(pFonts, FontID::MONOSPACE85, temp, 1, 22);
		else {
			this->writeStringOnPBMFrame(pFonts, FontID::MONOSPACE85, temp, pos_x, pos_y);
			pos_y += 9;
			x++;
		}
	}

	this->drawVerticalLineOnPBMFrame(updatedPosX(), (TEMP_POS_Y - 2), 23);

	return LCDPlugin::getCurrentPBMFrame();
}

} // namespace GLogiK

