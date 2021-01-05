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

#ifndef SRC_BIN_DAEMON_LCDPLUGINS_LCD_SCREEN_PLUGIN_HPP_
#define SRC_BIN_DAEMON_LCDPLUGINS_LCD_SCREEN_PLUGIN_HPP_

#include <cstdint>

#include <tuple>
#include <vector>
#include <string>

#include <boost/filesystem.hpp>

#include "include/LCDPluginProperties.hpp"

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
		LCDPBMFrame(const uint16_t i)
			:	_frameCounter(i)
		{
			/* initialize LCD PBM container */
			_PBMData.resize( (PBM_WIDTH / 8) * PBM_HEIGHT, 0 );
		}
		LCDPBMFrame(void) = delete;
		~LCDPBMFrame() = default;

		PBMDataArray _PBMData;

		const bool switchToNextFrame(const uint16_t currentFrameCounter);

	protected:

	private:
		const uint16_t _frameCounter;

};

class LCDPlugin
	:	virtual private PBMFile
{
	public:
		virtual ~LCDPlugin(void);

		virtual void init(FontsManager* const pFonts) = 0;
		const bool isInitialized(void) const;

		const LCDPluginProperties getPluginProperties(void) const;
		const std::string & getPluginName(void) const;
		const uint64_t getPluginID(void) const;
		const uint16_t getPluginTiming(void) const;
		const uint16_t getPluginMaxFrames(void) const;
		void resetPBMFrameIndex(void);
		void prepareNextPBMFrame(void);
		void resetEverLocked(void);

		virtual const PBMDataArray & getNextPBMFrame(
			FontsManager* const pFonts,
			const std::string & LCDKey,
			const bool lockedPlugin
		);

	protected:
		LCDPlugin(void);

		LCDPluginProperties _plugin;
		LCDPluginTempo _pluginTempo;

		void addPBMFrame(
			const fs::path & PBMDirectory,
			const std::string & file,
			const uint16_t num = 1
		);

		void addPBMClearedFrame(
			const uint16_t num = 1
		);

		const uint16_t getNextPBMFrameID(void) const;
		PBMDataArray & getCurrentPBMFrame(void);

		void writeStringOnFrame(
			FontsManager* const pFonts,
			const FontID fontID,
			const std::string & string,
			uint16_t PBMXPos,
			const uint16_t PBMYPos
		);

		void writeStringOnLastFrame(
			FontsManager* const pFonts,
			const FontID fontID,
			const std::string & string,
			uint16_t PBMXPos,
			const uint16_t PBMYPos
		);

		void drawProgressBarOnFrame(
			const uint16_t percent,
			const uint16_t PBMXPos,
			const uint16_t PBMYPos
		);

		void drawPadlockOnFrame(
			const bool lockedPlugin,
			const uint16_t PBMXPos = 1,
			const uint16_t PBMYPos = 1
		);

	private:
		bool _initialized;
		bool _everLocked;
		uint16_t _frameCounter;		/* frame counter */
		uint16_t _frameIndex;			/* frame index in the container */
		std::vector<LCDPBMFrame> _PBMFrames;
		std::vector<LCDPBMFrame>::iterator _itCurrentFrame;

		void checkPBMFrameIndex(void);
		static std::tuple<uint16_t, uint16_t> getTempo(const LCDPluginTempo tempo);
};

} // namespace GLogiK

#endif
