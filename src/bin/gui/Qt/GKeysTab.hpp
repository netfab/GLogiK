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

#include <QObject>
#include <QString>
#include <QLabel>
#include <QComboBox>
#include <QGroupBox>
#include <QPushButton>
#include <QHBoxLayout>

#include "lib/shared/deviceProperties.hpp"
#include "lib/shared/glogik.hpp"
#include "lib/utils/utils.hpp"

#include "include/base.hpp"

#include "Tab.hpp"

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
		void updateTab(
			const DeviceProperties & device,
			const bool resetCurrentBankID = false
		);


	private:
		GKeysTab() = delete;

		QGroupBox* _pKeysBox;

		QHBoxLayout* _pInputsBoxHeaderLayout;

		QLabel* _pHelpLabel;
		QComboBox* _GKeyEventTypeComboBox;

		MKeysID _currentBankID;

		QString _helpLabel;

		std::vector<QPushButton*> _buttonsSignalsToClear;

		void disconnectAllKeysBoxButtons(void);
		void clearInputsBoxHeaderLayout(void);

		void setRadioButtonsEnabled(const bool status);
		void updateInputsBox(const DeviceProperties & device, const GKeysID GKeyID);
		void switchGKeyEventType(DeviceProperties & device, const GKeysID GKeyID);

		QPushButton* newGKeyButton(
			const GKeysID GKeyID,
			const GKeyEventType eventType,
			const QString & colorName = "#RRGGBB"
		);
		QPushButton* newBlankButton(void);

		void updateCurrentBankID(
			const DeviceProperties & device,
			const MKeysID bankID
		);

		static const std::map<const MKeysID, c_str> bankNames;
};

} // namespace GLogiK

#endif
