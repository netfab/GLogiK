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

#ifndef SRC_BIN_DAEMON_LCDPLUGINS_LCD_SCREEN_PLUGIN_HPP_
#define SRC_BIN_DAEMON_LCDPLUGINS_LCD_SCREEN_PLUGIN_HPP_

#include <cstdint>

#include <tuple>
#include <vector>
#include <string>

#include <boost/filesystem.hpp>

#include "fontsManager.hpp"
#include "PBM.hpp"
#include "PBMFile.hpp"

namespace fs = boost::filesystem;

namespace GLogiK
{

enum class LCDPluginTempo : uint8_t
{
	TEMPO_DEFAULT	= 1 << 0,
	TEMPO_500_20,
	TEMPO_400_15,
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

		virtual const PBMDataArray & getNextPBMFrame(
			FontsManager* const pFonts,
			const std::string & LCDKey
		);

	protected:
		LCDPlugin(void);

		std::string _pluginName;
		LCDPluginTempo _pluginTempo;

		void addPBMFrame(
			const fs::path & PBMDirectory,
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
			const FontID fontID,
			const std::string & string,
			unsigned int PBMXPos,
			const unsigned int PBMYPos
		);

		void writeStringOnLastFrame(
			FontsManager* const pFonts,
			const FontID fontID,
			const std::string & string,
			unsigned int PBMXPos,
			const unsigned int PBMYPos
		);

	private:
		bool _initialized;
		unsigned short _frameCounter;		/* frame counter */
		unsigned short _frameIndex;			/* frame index in the container */
		std::vector<LCDPBMFrame> _PBMFrames;
		std::vector<LCDPBMFrame>::iterator _itCurrentFrame;

		void checkPBMFrameIndex(void);
		static std::tuple<unsigned short, unsigned short> getTempo(const LCDPluginTempo tempo);
};

} // namespace GLogiK

#endif
