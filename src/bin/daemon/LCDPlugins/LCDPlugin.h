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

#ifndef __GLOGIKD_LCD_SCREEN_PLUGIN_H__
#define __GLOGIKD_LCD_SCREEN_PLUGIN_H__

#include <cstdint>

#include <tuple>
#include <vector>
#include <string>

#include <boost/filesystem.hpp>

#include "fontsManager.h"
#include "PBM.h"
#include "PBMFile.h"

namespace fs = boost::filesystem;

namespace GLogiK
{

enum class LCDPluginTempo : uint8_t
{
	TEMPO_DEFAULT	= 1 << 0,
	TEMPO_750_8,
};

class LCDPBMFrame
{
	public:
		LCDPBMFrame(const unsigned short i)
			:	frame_count(i) {}
		~LCDPBMFrame() = default;

		const unsigned short frame_count;
		PBMDataArray pbm_data;

	protected:

	private:

};

class LCDPlugin
	:	virtual private PBMFile
{
	public:
		virtual ~LCDPlugin(void);

		virtual void init(FontsManager* const pFonts) = 0;
		const bool isInitialized(void) const;

		const unsigned short getPluginTiming(void) const;
		const unsigned short getPluginMaxFrames(void) const;
		void resetPBMFrameIndex(void);
		void prepareNextPBMFrame(void);

		virtual const PBMDataArray & getNextPBMFrame(FontsManager* const pFonts);

	protected:
		LCDPlugin(void);

		std::string name_;
		LCDPluginTempo tempo_;

		void addPBMFrame(
			const fs::path & pbm_dir,
			const std::string & file,
			const unsigned short num = 1
		);

		void addPBMClearedFrame(
			const unsigned short num = 1
		);

		const unsigned short getNextPBMFrameID(void) const;
		PBMDataArray & getCurrentPBMFrame(void);

		void writeStringOnFrame(
			FontsManager* const pFonts,
			const std::string & fontName,
			const std::string & string,
			const unsigned int PBMXPos,
			const unsigned int PBMYPos
		);

	private:
		bool initialized_;
		unsigned short frame_count_;
		unsigned short frame_ID_;
		std::vector<LCDPBMFrame> frames_;
		std::vector<LCDPBMFrame>::iterator current_frame_;

		void checkPBMFrameIndex(void);
		static std::tuple<unsigned short, unsigned short> getTempo(const LCDPluginTempo tempo);
};

} // namespace GLogiK

#endif
