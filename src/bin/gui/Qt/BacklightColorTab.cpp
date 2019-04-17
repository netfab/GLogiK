/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2019  Fabrice Delliaux <netbox253@gmail.com>
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

#include <QVBoxLayout>
#include <QHBoxLayout>

#include "lib/utils/utils.hpp"

#include "BacklightColorTab.hpp"

namespace GLogiK
{

using namespace NSGKUtils;

BacklightColorTab::BacklightColorTab(
	NSGKDBus::GKDBus* pDBus,
	const QString & name
)	:	Tab(pDBus),
		_pCurrentColorLabel(nullptr),
		_pNewColorLabel(nullptr),
		_pApplyButton(nullptr)
{
	this->setObjectName(name);
}

void BacklightColorTab::build(void)
{
	QVBoxLayout* vBox = nullptr;
	QHBoxLayout* hBox = nullptr;

	try {
		vBox = new QVBoxLayout(this);
#if DEBUGGING_ON
		LOG(DEBUG1) << "allocated QVBoxLayout";
#endif
		this->setLayout(vBox);

		/* -- -- -- */
		vBox->addWidget( this->getHLine() );
		/* -- -- -- */

		_colorDialog = new QColorDialog();
		vBox->addWidget(_colorDialog);

		_colorDialog->setWindowFlags(Qt::Widget);
		_colorDialog->setOptions(
			QColorDialog::DontUseNativeDialog
			| QColorDialog::NoButtons
		);

		/* -- -- -- */

		vBox->addStretch();

		/* -- -- -- */
		vBox->addWidget( this->getHLine() );
		/* -- -- -- */

		hBox = new QHBoxLayout();
#if DEBUGGING_ON
		LOG(DEBUG1) << "allocated QHBoxLayout";
#endif

		vBox->addLayout(hBox);

		hBox->addWidget(new QLabel("Current backlight color : "));

		_pCurrentColorLabel = new QLabel();
		hBox->addWidget(_pCurrentColorLabel);
		_pCurrentColorLabel->setFixedSize(26, 26);

		QColor initialColor(255, 255, 255);
		this->setCurrentColorLabel(initialColor);

		hBox->addStretch();

		hBox->addWidget(new QLabel("New color : "));

		_pNewColorLabel = new QLabel();
		hBox->addWidget(_pNewColorLabel);
		_pNewColorLabel->setFixedSize(26, 26);

		hBox->addSpacing(10);

		_pApplyButton = new QPushButton("Appl&y Color");
#if DEBUGGING_ON
		LOG(DEBUG1) << "allocated Apply button";
#endif
		hBox->addWidget(_pApplyButton);

		/* Default visual properties for widgets are defined by QStyle
		 * styleSheet() returns empty QString */
		QString newStyle("padding:3px 12px 3px 12px;");
		_pApplyButton->setStyleSheet(newStyle);

		hBox->addSpacing(10);

		/* -- -- -- */
		vBox->addWidget( this->getHLine() );
		/* -- -- -- */

		this->setNewColorLabel(initialColor);

		connect(_colorDialog, &QColorDialog::currentColorChanged, this, &BacklightColorTab::setNewColorLabel);
	}
	catch (const std::bad_alloc& e) {
		LOG(ERROR) << e.what();
		throw;
	}
}

BacklightColorTab::~BacklightColorTab()
{
}

const QPushButton* BacklightColorTab::getApplyButton(void) const
{
	return _pApplyButton;
}

const QColor & BacklightColorTab::getAndSetNewColor(void)
{
	this->setCurrentColorLabel(_newColor);
	_pApplyButton->setEnabled( ! (_newColor == _currentColor) );
	return _newColor;
}

void BacklightColorTab::disableApplyButton(void)
{
	_pApplyButton->setEnabled(false);
}

void BacklightColorTab::updateTab(
	const DeviceProperties & device
) {
	uint8_t r, g, b = 0; device.getRGBBytes(r, g, b);
	QColor color(r, g, b);

#if DEBUGGING_ON
	LOG(DEBUG1) << "updating BacklightColorTab";
	LOG(DEBUG2) << "color : " << color.name().toStdString();
#endif
	this->setCurrentColorLabel(color);
	_colorDialog->setCurrentColor(color);
}

const QString BacklightColorTab::getBlackBorderedStyleSheetColor(const QColor & color) const
{
	QString style("border:1px solid black;");
	style += " background-color:" + color.name() + ";";
	return style;
}

void BacklightColorTab::setCurrentColorLabel(const QColor & color)
{
	_pCurrentColorLabel->setStyleSheet(this->getBlackBorderedStyleSheetColor(color));
	_currentColor = color;
}

void BacklightColorTab::setNewColorLabel(const QColor & color)
{
	_pNewColorLabel->setStyleSheet(this->getBlackBorderedStyleSheetColor(color));
	_newColor = color;
	_pApplyButton->setEnabled( ! (_newColor == _currentColor) );
}

} // namespace GLogiK

