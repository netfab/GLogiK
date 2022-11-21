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

#ifndef SRC_BIN_GUI_QT_GKEYS_TAB_HPP_
#define SRC_BIN_GUI_QT_GKEYS_TAB_HPP_

#include <vector>
#include <map>
#include <string>

#include <QString>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include "lib/shared/deviceProperties.hpp"
#include "lib/shared/glogik.hpp"
#include "lib/utils/utils.hpp"

#include "include/base.hpp"
#include "include/MBank.hpp"

#include "Tab.hpp"

#define GKEY_COMMAND_LINE_STRING_MAX_LENGTH 64

namespace GLogiK
{

class GKeysTab
	:	public Tab
{
	public:
		GKeysTab(
			NSGKDBus::GKDBus* pDBus,
			const QString & name
		);
		~GKeysTab();

		void buildTab(void);
		void updateTab(const DeviceProperties & device, const MKeysID bankID);
		void getGKeyEventParams(
			MKeysID & bankID,
			GKeysID & keyID,
			GKeyEventType & eventType,
			std::string & eventCommand
		);

	private:
		GKeysTab() = delete;

		QVBoxLayout* _pKeysBoxLayout;
		QHBoxLayout* _pInputsBoxHeaderLayout;
		QHBoxLayout* _pInputsBoxBodyLayout;

		QLabel* _pHelpLabel;
		QLineEdit* _pCommandLineEdit;
		QComboBox* _GKeyEventTypeComboBox;

		MKeysID _currentBankID;
		GKeysID _currentGKeyID;
		GKeyEventType _newEventType;

		QString _helpLabel;
		std::string _currentEventCommand;

		std::vector<QPushButton*> _buttonsSignalsToClear;

		bool _updateGKeyEvent;

		void setApplyButtonStatus(const bool status);
		void updateApplyButtonStatus(const QString & newString);

		QPushButton* newBlankButton(void);
		QPushButton* newGKeyButton(
			const GKeysID GKeyID,
			const GKeyEventType eventType,
			const QString & colorName = "#RRGGBB"
		);

		void clearInputsBoxHeaderLayout(void);
		void clearInputsBoxBodyLayout(void);
		void clearKeysBoxLayout(void);

		void setGKeyEventParams(
			const std::string & eventCommand,
			const GKeyEventType eventType,
			const GKeysID GKeyID
		);
		void prepareCommandWidget(const GKeysEvent & GKeyEvent, const GKeysID GKeyID);

		void updateInputsBox(const DeviceProperties & device, const GKeysID GKeyID);
		void switchGKeyEventType(const DeviceProperties & device, const GKeysID GKeyID);

		void redrawTab(const DeviceProperties & device);

		static const std::map<const MKeysID, c_str> bankNames;
};

} // namespace GLogiK

#endif
