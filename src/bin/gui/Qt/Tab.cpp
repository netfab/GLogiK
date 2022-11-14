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

#include <string>

#include <QLayoutItem>
#include <QSpacerItem>

#include "lib/utils/utils.hpp"

#include "Tab.hpp"

namespace GLogiK
{

using namespace NSGKUtils;

const QPushButton* Tab::getApplyButton(void) const
{
	return _pApplyButton;
}

QFrame* Tab::getHLine(void)
{
	GK_LOG_FUNC

	QFrame* line = new QFrame();
	GKLog(trace, "allocated QFrame")

	line->setFrameShape(QFrame::HLine);
	line->setFrameShadow(QFrame::Sunken);
	return line;
}

QFrame* Tab::getVLine(void)
{
	GK_LOG_FUNC

	QFrame* line = new QFrame();
	GKLog(trace, "allocated QFrame")

	line->setFrameShape(QFrame::VLine);
	line->setFrameShadow(QFrame::Sunken);
	return line;
}

void Tab::prepareApplyButton(void)
{
	GK_LOG_FUNC

	_pApplyButton = new QPushButton("Appl&y Changes");
	GKLog(trace, "allocated Apply button")

	/* Default visual properties for widgets are defined by QStyle
	 * styleSheet() returns empty QString */
	_pApplyButton->setStyleSheet("padding:3px 12px 3px 12px;");
}

void Tab::clearLayout(QLayout* parentLayout)
{
	GK_LOG_FUNC

	if(parentLayout == nullptr)
		return;

	QLayoutItem* item;
	while( (item = parentLayout->takeAt(0) ) != nullptr)
	{
		std::string itemName("spacer");

		/*
		 * From the (QLayoutItem) Qt documentation.
		 *
		 *   If this item *is* a QLayout, ->layout() returns
		 *     it as a QLayout; otherwise nullptr is returned.
		 *   If this item *is* a QSpacerItem, ->spacerItem() returns
		 *     it as a QSpacerItem; otherwise nullptr is returned.
		 *   If this item *manages* a QWidget, ->widget() returns
		 *     that widget; otherwise nullptr is returned.
		 */

		QLayout* layout = item->layout();
		if( layout != nullptr ) {
			this->clearLayout(layout);

			itemName = "layout ";
			itemName += layout->objectName().toStdString();

			layout = nullptr;
		}

		QWidget* widget = item->widget();
		if( widget != nullptr ) {
			itemName = "widget ";
			itemName += widget->objectName().toStdString();
			GKLog2(trace, "deleting ", itemName)

			widget->disconnect();
			delete widget; widget = nullptr;
		}

		GKLog2(trace, "deleting layout item : ", itemName)
		delete item;
	}
}

} // namespace GLogiK

