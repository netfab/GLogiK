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

#include <QHBoxLayout>
#include <QGroupBox>
#include <QLayoutItem>
#include <QSpacerItem>
#include <QPushButton>

#include "GKeysTab.hpp"

namespace GLogiK
{

using namespace NSGKUtils;

GKeysTab::GKeysTab(
	NSGKDBus::GKDBus* pDBus,
	const QString & name)
	:	Tab(pDBus)
{
	this->setObjectName(name);
}

GKeysTab::~GKeysTab()
{
}

void GKeysTab::updateTab(const DeviceProperties & device)
{
	GK_LOG_FUNC

	GKLog(trace, "updating GKeysTab")

	auto clearLayout = [] (QLayout* mainLayout) -> void {
		std::function<void (QLayout*) > clearQLayout = [&](QLayout* parentLayout)
		{
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
					clearQLayout(layout);

					itemName = "layout ";
					itemName += layout->objectName().toStdString();

					layout = nullptr;
				}

				QWidget* widget = item->widget();
				if( widget != nullptr ) {
					itemName = "widget ";
					itemName += widget->objectName().toStdString();
					GKLog2(trace, "deleting ", itemName)
					delete widget; widget = nullptr;
				}

				GKLog2(trace, "deleting layout item : ", itemName)
				delete item;
			}
		};

		clearQLayout(mainLayout);
	};

	auto newButtonsLayout = [] (
				const QString & layout, const QString & button1,
				const QString & button2, const QString & button3) -> QHBoxLayout*
	{
		auto newButton = [] (const QString & text) -> QPushButton* {
			QPushButton* b = new QPushButton(text);
			b->setObjectName(text);
			b->setFixedWidth(32);
			GKLog2(trace, "allocated QPushButton ", text.toStdString())
			return b;
		};

		QHBoxLayout* hBox = new QHBoxLayout();
		hBox->setObjectName(layout);
		GKLog2(trace, "allocated QHBoxLayout ", layout.toStdString())

		hBox->addWidget( newButton(button1) );
		hBox->addWidget( newButton(button2) );
		hBox->addWidget( newButton(button3) );

		return hBox;
	};

	/* -- -- -- */

	clearLayout(_pGroupBoxLayout);

	try {
		_pGroupBoxLayout->addLayout( newButtonsLayout("hBox 1", "G1", "G2", "G3") );
		_pGroupBoxLayout->addLayout( newButtonsLayout("hBox 2", "G4", "G5", "G6") );
		_pGroupBoxLayout->addStretch();
	}
	catch (const std::bad_alloc& e) {
		LOG(error) << "bad allocation : " << e.what();
		throw GLogiKBadAlloc("bad allocation");
	}
}

void GKeysTab::buildTab(void)
{
	GK_LOG_FUNC

	try {
		QVBoxLayout* vBox = new QVBoxLayout(this);
		GKLog(trace, "allocated QVBoxLayout")

		this->setLayout(vBox);

		/* -- -- -- */
		vBox->addWidget( this->getHLine() );

		/* -- -- -- */

		{
			QHBoxLayout* hBox = new QHBoxLayout();
			GKLog(trace, "allocated QHBoxLayout")

			vBox->addLayout(hBox);

			/* -- -- -- */

			QGroupBox* GKeysBox = new QGroupBox();
			GKeysBox->setTitle("");
			//GKeysBox->setFlat(true);

			_pGroupBoxLayout = new QVBoxLayout();
			_pGroupBoxLayout->setObjectName("GroupBoxLayout");

			GKeysBox->setLayout(_pGroupBoxLayout);

			hBox->addWidget(GKeysBox);

			/* -- -- -- */
			hBox->addWidget( this->getVLine() );
			/* -- -- -- */

			hBox->addStretch();
		}

		/* -- -- -- */
		vBox->addWidget( this->getHLine() );

		{
			/* -- -- -- */
			QHBoxLayout* hBox = new QHBoxLayout();
			GKLog(trace, "allocated QHBoxLayout")

			vBox->addLayout(hBox);

			hBox->addSpacing(10);
			hBox->addStretch();

			this->prepareApplyButton();
			hBox->addWidget(_pApplyButton);
			//_pApplyButton->setEnabled(false);

			hBox->addSpacing(10);
		}

		/* -- -- -- */
		vBox->addWidget( this->getHLine() );
	}
	catch (const std::bad_alloc& e) {
		LOG(error) << "bad allocation : " << e.what();
		throw;
	}
}

} // namespace GLogiK

