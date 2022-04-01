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

#include <QHBoxLayout>
#include <QGroupBox>
#include <QLayoutItem>
#include <QSpacerItem>
#include <QPushButton>

#include "GKeysTab.hpp"

namespace GLogiK
{

using namespace NSGKUtils;

const std::map<const MKeysID, c_str> GKeysTab::bankNames = {
	{MKeysID::MKEY_M0, M_KEY_M0},
	{MKeysID::MKEY_M1, M_KEY_M1},
	{MKeysID::MKEY_M2, M_KEY_M2},
	{MKeysID::MKEY_M3, M_KEY_M3},
};

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

	unsigned int layoutNum = 0;

	/* -- -- -- */

	auto clearLayout = [] (QLayout* mainLayout) -> void {
		std::function<void (QLayout*)> clearQLayout =
			[&clearQLayout](QLayout* parentLayout) -> void
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

	auto nextLayoutName = [&layoutNum] () -> const QString
	{
		QString layoutName("hBox ");
		layoutName += std::to_string(layoutNum).c_str();
		layoutNum++;
		return layoutName;
	};

	auto newButton = [] (const QString & text) -> QPushButton* {
		QPushButton* b = new QPushButton(text);
		b->setObjectName(text);
		b->setFixedWidth(32);
		GKLog2(trace, "allocated QPushButton ", text.toStdString())
		return b;
	};

	auto newBanksLayout = [&nextLayoutName, &newButton] (
		const std::vector<MKeysID> & banks ) -> QHBoxLayout*
	{
		QHBoxLayout* hBox = new QHBoxLayout();

		hBox->setObjectName( nextLayoutName() );
		GKLog2(trace, "allocated QHBoxLayout ", hBox->objectName().toStdString())

		for(const auto & id : banks) {
			if(id == MKeysID::MKEY_M0)
				continue; // skip virtual M0 key

			auto n = bankNames.at(id);
			hBox->addWidget( newButton( n ) );
			GKLog2(trace, "allocated M-key bank button: ", n)
		}

		return hBox;
	};

	auto newButtonsLayout = [&nextLayoutName, &newButton] (
				const QString & button1,
				const QString & button2,
				const QString & button3) -> QHBoxLayout*
	{
		QHBoxLayout* hBox = new QHBoxLayout();

		hBox->setObjectName( nextLayoutName() );
		GKLog2(trace, "allocated QHBoxLayout ", hBox->objectName().toStdString())

		hBox->addWidget( newButton(button1) );
		hBox->addWidget( newButton(button2) );
		hBox->addWidget( newButton(button3) );

		return hBox;
	};

	/* -- -- -- */

	const MKeysID currentID = MKeysID::MKEY_M0; // TODO
	const unsigned short keysPerLine = 3;	// TODO

	const banksMap_type & banks = device.getMacrosBanks();

	const mBank_type & bank = banks.at(currentID);

	if( (bank.size() % keysPerLine) != 0 )
		throw GLogiKExcept("G-Keys modulo not null");

	/* -- -- -- */

	clearLayout(_pKeysBoxLayout);

	try {
		{	// initialize M-keys layout
			std::vector<MKeysID> ids;
			for(const auto & idBankPair : banks) {
				const MKeysID & bankID = idBankPair.first;
				ids.push_back(bankID);
			}

			_pKeysBoxLayout->addLayout( newBanksLayout(ids) );
		}

		_pKeysBoxLayout->addSpacing(10);

		/* G-keys */
		for(unsigned short i = 0; i < (bank.size() / keysPerLine); ++i)
		{
			mBank_type::const_iterator it1 = bank.begin();
			if(safeAdvance(it1, bank.end(), i*keysPerLine) != 0)
				throw GLogiKExcept("wrong G-Keys offset");

			auto it2 = it1;
			if(safeAdvance(it2, bank.end(), 1) != 0)
				throw GLogiKExcept("wrong second G-Key");

			auto it3 = it2;
			if(safeAdvance(it3, bank.end(), 1) != 0)
				throw GLogiKExcept("wrong third G-Key");

			const QString key1(getGKeyName(it1->first).c_str());
			const QString key2(getGKeyName(it2->first).c_str());
			const QString key3(getGKeyName(it3->first).c_str());

			_pKeysBoxLayout->addLayout(
				newButtonsLayout(key1, key2, key3)
			);
		}

		_pKeysBoxLayout->addStretch();
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

			QGroupBox* keysBox = new QGroupBox();
			keysBox->setTitle("");
			//keysBox->setFlat(true);

			_pKeysBoxLayout = new QVBoxLayout();
			_pKeysBoxLayout->setObjectName("KeysBoxLayout");

			keysBox->setLayout(_pKeysBoxLayout);

			hBox->addWidget(keysBox);

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

