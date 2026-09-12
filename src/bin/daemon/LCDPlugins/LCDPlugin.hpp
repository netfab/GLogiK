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

#ifndef SRC_BIN_DAEMON_LCDPLUGINS_LCD_SCREEN_PLUGIN_HPP_
#define SRC_BIN_DAEMON_LCDPLUGINS_LCD_SCREEN_PLUGIN_HPP_

#include <cstdint>

#include <tuple>
#include <vector>
#include <string>

#include <boost/filesystem.hpp>

#include "include/LCDPP.hpp"

#include "fontsManager.hpp"
#include "PBM.hpp"
#include "PBMFile.hpp"

namespace fs = boost::filesystem;

namespace GLogiK::daemon
{

enum class LCDPluginTempo : std::uint8_t
{
	TEMPO_DEFAULT	= 1 << 0,
	TEMPO_500_20,
	TEMPO_400_15,
};

class PBMFrame
{
	public:
		PBMFrame(const std::uint16_t num);
		PBMFrame(void) = delete;
		~PBMFrame() = default;

		PixelsData _PBMData;

		const bool switchToNextFrame(const std::uint16_t currentFrameCounter);

	protected:

	private:
		const std::uint16_t _numFrames;

};

class LCDPlugin
	:	virtual protected PBMFile
{
	public:
		virtual ~LCDPlugin(void);

		virtual void init(
			FontsManager* const pFonts,
			const std::string & product
		) = 0;

		const bool isInitialized(void) const;

		const LCDPP getPluginProperties(void) const;
		const std::string & getPluginName(void) const;
		const std::uint64_t getPluginID(void) const;
		const std::uint16_t getPluginTiming(void) const;
		const std::uint16_t getPluginMaxFrames(void) const;
		void resetPluginEverLocked(void);

		void resetPBMFrameIndex(void);
		void prepareNextPBMFrame(void);

		virtual const PixelsData & getNextPBMFrame(
			FontsManager* const pFonts,
			const std::string & LCDKey,
			const bool lockedPlugin
		);

	protected:
		LCDPlugin(void);

		LCDPP _plugin;
		LCDPluginTempo _pluginTempo;

		void addPBMFrame(
			const fs::path & PBMDirectory,
			const std::string & file,
			const std::uint16_t num = 1
		);

		void addPBMEmptyFrame(const std::uint16_t num = 1);

		const std::uint16_t getNextPBMFrameID(void) const;
		PixelsData & getCurrentPBMFrame(void);

		void writeStringOnPBMFrame(
			FontsManager* const pFonts,
			const FontID fontID,
			const std::string & string,
			const int16_t PBMXPos,
			const int16_t PBMYPos
		);

		void writeStringOnLastPBMFrame(
			FontsManager* const pFonts,
			const FontID fontID,
			const std::string & string,
			const int16_t PBMXPos,
			const int16_t PBMYPos
		);

		void drawProgressBarOnPBMFrame(
			const std::uint16_t percent,
			const std::uint16_t PBMXPos,
			const std::uint16_t PBMYPos
		);

		void drawPadlockOnPBMFrame(
			const bool lockedPlugin,
			const std::uint16_t PBMXPos = 1,
			const std::uint16_t PBMYPos = 1
		);

		void drawVerticalLineOnPBMFrame(
			const std::uint16_t PBMXPos,
			const std::uint16_t PBMYPos,
			const std::uint16_t size
		);

	private:
		bool _initialized;
		bool _everLocked;
		std::uint16_t _PBMFrameCounter;	/* frame counter */
		std::uint16_t _PBMFrameIndex;	/* frame index in the container */
		std::vector<PBMFrame> _PBMFrames;
		std::vector<PBMFrame>::iterator _itCurrentPBMFrame;

		void checkPBMFrameIndex(void);
		static std::tuple<std::uint16_t, std::uint16_t> getTempo(const LCDPluginTempo tempo);
};

} // namespace GLogiK::daemon

#endif
