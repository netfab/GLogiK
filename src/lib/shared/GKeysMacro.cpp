/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2023  Fabrice Delliaux <netbox253@gmail.com>
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
#include <iterator>

#include "GKeysMacro.hpp"

#include "lib/utils/utils.hpp"

namespace GLogiK
{

using namespace NSGKUtils;

const macro_type GKeysMacro::emptyMacro = {};

void GKeysMacro::checkMacro(macro_type & macro)
{
	std::vector<MacroEvent> pressedEvents;
	std::vector<MacroEvent> releasedEvents;

	this->fillInVectors(macro, pressedEvents, releasedEvents);
	this->fixMacroReleaseEvents(pressedEvents, releasedEvents, macro);

	//// debug code
	//KeyEvent e1(KEY_UNKNOWN, EventValue::EVENT_KEY_PRESS, 1);
	//KeyEvent e2(KEY_UNKNOWN, EventValue::EVENT_KEY_RELEASE, 1);
	//for(unsigned int i = 0; i < 12; ++i) {
	//	macro.push_back(e1);
	//	macro.push_back(e2);
	//}

	if(macro.size() >= MACRO_T_MAX_SIZE) {
		LOG(warning) << "macro size greater than MACRO_T_MAX_SIZE, fixing it";
		pressedEvents.clear();
		releasedEvents.clear();
		this->fillInVectors(macro, pressedEvents, releasedEvents);
		this->fixMacroSize(pressedEvents, releasedEvents, macro);
	}
}

void GKeysMacro::fillInVectors(
	const macro_type & macro,
	std::vector<MacroEvent> & pressedEvents,
	std::vector<MacroEvent> & releasedEvents)
{
	for(unsigned int i = 0; i != macro.size(); ++i) {
		const auto & keyEvent = macro[i];
		MacroEvent e(keyEvent, i);

		if( keyEvent.event == EventValue::EVENT_KEY_PRESS )
			pressedEvents.push_back(e);
		else if( keyEvent.event == EventValue::EVENT_KEY_RELEASE )
			releasedEvents.push_back(e);
	}
}

void GKeysMacro::fixMacroReleaseEvents(
	const std::vector<MacroEvent> & pressedEvents,
	std::vector<MacroEvent> & releasedEvents,
	macro_type & macro)
{
	GK_LOG_FUNC

	/* fix missing release events */
	for(const auto & pressed : pressedEvents) {
		bool found = false;
		for(auto it = releasedEvents.begin(); it != releasedEvents.end(); ++it) {
			if(pressed.key.code == (*it).key.code) {
				if(pressed.index < (*it).index) {
					/* KEY_RELEASE event found for current KEY_PRESS event */
					releasedEvents.erase(it);
					found = true;
					break; /* jumping to next press event */
				}
			}
		}
		if( ! found ) {
			LOG(warning) << "missing release event for index " << toUInt(pressed.index) << " - adding event";
			KeyEvent e = pressed.key;
			e.event = EventValue::EVENT_KEY_RELEASE;
			e.interval = 1;
			macro.push_back(e);
		}
	}

	/* remove redundant release events in reverse order */
	if( ! releasedEvents.empty() ) {
		GKLog2(trace, "some redundant release events were found : ", releasedEvents.size())

		std::reverse(releasedEvents.begin(), releasedEvents.end());

		for(const auto & redundant : releasedEvents) {
			GKLog2(trace, "erasing redundant release event at index : ", toUInt(redundant.index))

			auto it = std::next(macro.begin(), redundant.index);
			macro.erase(it);
		}
	}
}

void GKeysMacro::fixMacroSize(
	const std::vector<MacroEvent> & pressedEvents,
	std::vector<MacroEvent> & releasedEvents,
	macro_type & macro)
{
	GK_LOG_FUNC

	GKLog4(trace, "pressed : ", pressedEvents.size(), "released : ", releasedEvents.size())

	/* sanity check */
	if(pressedEvents.size() != releasedEvents.size()) {
		LOG(warning) << "pressed and released events disparity :";
		LOG(warning) << "pressed: " << pressedEvents.size() << " - released: " << releasedEvents.size();
	}

	std::vector<unsigned int> indexes;

	for(const auto & pressed : pressedEvents) {
		for(auto it = releasedEvents.begin(); it != releasedEvents.end(); ++it) {
			if(pressed.key.code == (*it).key.code) {
				if(pressed.index < (*it).index) {
					indexes.push_back( pressed.index );
					indexes.push_back( (*it).index );
					releasedEvents.erase(it);
					break;
				}
			}
		}
	}

	std::sort(indexes.begin(), indexes.end());

	GKLog2(trace, "indexes size : ", indexes.size())

	while( (indexes.size() > 1) and (macro.size() >= MACRO_T_MAX_SIZE) ) {
		auto & index = indexes.back();

		GKLog2(trace, "erasing index : ", index)

		auto it = std::next(macro.begin(), index);
		macro.erase(it);
		indexes.pop_back();

		index = indexes.back();

		GKLog2(trace, "erasing index : ", index)

		it = std::next(macro.begin(), index);
		macro.erase(it);
		indexes.pop_back();
	}

	/* sanity check */
	if( macro.size() >= MACRO_T_MAX_SIZE ) {
		GKLog2(trace, "macro size : ", macro.size())

		LOG(warning) << "macro still greater than MACRO_T_MAX_SIZE, force resize it";
		macro.resize(MACRO_T_MAX_SIZE - 1);
	}

}

} // namespace GLogiK

