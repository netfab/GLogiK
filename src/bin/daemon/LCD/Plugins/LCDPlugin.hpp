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

#pragma once

#include <cstddef>
#include <cstdint>

#include <tuple>
#include <vector>
#include <string>
#include <string_view>

#include <boost/filesystem.hpp>

#include "include/LCDPP.hpp"

#include "bin/daemon/LCD/PixelsData.hpp"
#include "bin/daemon/LCD/PBM/PBM.hpp"

namespace fs = boost::filesystem;

namespace Managers::LCDPlugins::Fonts
{
enum class FontID : std::uint8_t;
class FontsManager;
}

namespace Managers::LCDPlugins::plugin
{

enum class LCDPluginTempo : std::uint8_t
{
	TEMPO_DEFAULT	= 1 << 0,
	TEMPO_500_20,
	TEMPO_400_15,
};

class pluginFrame
{
	private:
		using PixelsData = LCDPlugins::PixelsData;

	public:
		explicit pluginFrame(std::uint16_t num);
		pluginFrame() = delete;
		~pluginFrame() = default;

		PixelsData _PBMData;

		bool switchToNextFrame(std::uint16_t currentFrameCounter);

	protected:

	private:
		const std::uint16_t _numFrames;

};

class LCDPlugin
{
	private:
		using FontID = Fonts::FontID;
		using FontsManager = Fonts::FontsManager;
		using LCDPP = ::GLogiK::LCDPP;
		using PixelsData = LCDPlugins::PixelsData;

	public:
		virtual ~LCDPlugin();

		virtual void init(
			FontsManager* const pFonts,
			const std::string & product
		) = 0;

		bool isInitialized() const;

		LCDPP getPluginProperties() const;
		const std::string & getPluginName() const;
		std::uint64_t getPluginID() const;
		std::uint16_t getPluginTiming() const;
		std::uint16_t getPluginMaxFrames() const;
		void resetPluginEverLocked();

		void resetFrameIndex();
		void prepareNextFrame();

		virtual const PixelsData & getNextFrame(
			FontsManager* const pFonts,
			const std::string & LCDKey,
			bool lockedPlugin
		);

	protected:
		LCDPlugin();

		LCDPP _plugin;
		LCDPluginTempo _pluginTempo;

		void addFrameFromPBM(
			const fs::path & PBMDirectory,
			const std::string & file,
			std::uint16_t num = 1
		);

		void addEmptyFrame(std::uint16_t num = 1);

		PixelsData & getCurrentFrame();

		void writeStringOnFrame(
			FontsManager* const pFonts, // pointer to fontsManager
			FontID fontID,        		// font we want to use
			std::string_view sv,        // string to write
			std::int16_t PBMXPos,       // xPos, -1 means centered horizontally
			std::int16_t PBMYPos        // yPos, -1 means centered vertically
		);

		void writeStringOnLastFrame(
			FontsManager* const pFonts,
			FontID fontID,
			std::string_view sv,
			std::int16_t PBMXPos,
			std::int16_t PBMYPos
		);

		void drawProgressBarOnFrame(
			std::uint16_t percent,
			std::uint16_t PBMXPos,
			std::uint16_t PBMYPos
		);

		void drawPadlockOnFrame(
			bool lockedPlugin,
			std::uint16_t PBMXPos = 1,
			std::uint16_t PBMYPos = 1
		);

		void drawVerticalLineOnFrame(
			std::uint16_t PBMXPos,
			std::uint16_t PBMYPos,
			std::uint16_t size
		);

	private:
		bool _initialized;
		bool _everLocked;
		std::uint16_t _frameCounter; /* frame counter */
		std::size_t _frameIndex;   /* frame index in the container */
		std::vector<pluginFrame> _frames;
		std::vector<pluginFrame>::iterator _itCurrentFrame;

		void checkFrameIndex();
		void updateFrameIndex()
		{
			_frameIndex = static_cast<std::size_t>(_itCurrentFrame - _frames.begin());
		}
		static std::tuple<std::uint16_t, std::uint16_t> getTempo(const LCDPluginTempo tempo);
};

} // namespace Managers::LCDPlugins::plugin
