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

#ifndef SRC_BIN_GUI_QT_BACKLIGHT_COLOR_TAB_HPP_
#define SRC_BIN_GUI_QT_BACKLIGHT_COLOR_TAB_HPP_

#include <QColor>
#include <QLabel>
#include <QString>
#include <QPushButton>
#include <QColorDialog>

#include "lib/shared/deviceProperties.hpp"

#include "Tab.hpp"

namespace GLogiK
{

class BacklightColorTab
	:	public Tab
{
	public:
		BacklightColorTab(
			NSGKDBus::GKDBus* pDBus,
			const QString & name
		);
		~BacklightColorTab();

		void buildTab(void);
		void updateTab(
			const DeviceProperties & device
		);

		const QColor & getAndSetNewColor(void);
		void disableApplyButton(void);

	private:
		BacklightColorTab() = delete;

		QColor _currentColor;
		QColor _newColor;

		QLabel* _pCurrentColorLabel;
		QLabel* _pNewColorLabel;

		QColorDialog* _colorDialog;

		const QString getBlackBorderedStyleSheetColor(const QColor & color) const;
		void setCurrentColorLabel(const QColor & color);
		void setNewColorLabel(const QColor & color);
};

} // namespace GLogiK

#endif
