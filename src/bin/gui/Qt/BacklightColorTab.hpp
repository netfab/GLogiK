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

#include <QColor>
#include <QString>

#include "lib/shared/deviceProperties.hpp"

#include "Tab.hpp"

class QLabel;
class QColorDialog;

namespace GLogiK
{

class BacklightColorTab
	:	public Tab
{
	private:
		using DeviceProperties = LIBShared::DeviceProperties;

	public:
		BacklightColorTab(
			GKDBUS::GKDBus* pDBus,
			const QString & name
		);
		~BacklightColorTab();

		void buildTab();
		void updateTab(
			const DeviceProperties & device,
			const std::string & devID
		);

		const QColor & getAndSetNewColor();
		void disableApplyButton();

	private:
		BacklightColorTab() = delete;

		QColor _currentColor;
		QColor _newColor;

		QLabel* _pCurrentColorLabel;
		QLabel* _pNewColorLabel;

		QColorDialog* _colorDialog;

		QString getBlackBorderedStyleSheetColor(const QColor & color) const;
		void setCurrentColorLabel(const QColor & color);
		void setNewColorLabel(const QColor & color);
};

} // namespace GLogiK
