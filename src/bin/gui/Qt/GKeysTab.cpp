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

#include <QColor>
#include <QVariant>
#include <QLabel>
#include <QList>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLayoutItem>
#include <QSpacerItem>
#include <QPushButton>
#include <QRadioButton>

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
	:	Tab(pDBus),
		_stubGKeyName("stubGKey"),
		_stubCommandKeyName("stubCommandKey"),
		_stubMacroKeyName("stubMacroKey")
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

	const MKeysID currentID = MKeysID::MKEY_M2; // TODO
	const unsigned short keysPerLine = 3;	// TODO

	uint8_t r, g, b = 0; device.getRGBBytes(r, g, b);
	const QColor color(r, g, b);

	GKLog2(trace, "got color: ", color.name().toStdString())

	/* -- -- -- */

	auto setButtonColor = [&color] (QPushButton* button) -> void {
		if( ! button ) {
			GKLog(warning, "null pointer detected")
			return;
		}
		QString style = "color:";
		style += color.name();
		style += ";";
		button->setStyleSheet(style);
	};

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

	auto newMButton = [&currentID] (
			const MKeysID & keyID,
			const QString & keyName) -> QPushButton*
	{
		QPushButton* b = new QPushButton(keyName);

		if(keyID == currentID)
			b->setProperty("class", QVariant("currentBank")); // css class

		b->setObjectName(keyName);
		b->setFixedWidth(32);

		GKLog2(trace, "allocated M-Key QPushButton ", keyName.toStdString())
		return b;
	};

	auto newGButton = [this, &setButtonColor] (mBank_type::const_iterator & it) -> QPushButton*
	{
		const QString keyName(getGKeyName(it->first).c_str());

		QPushButton* b = new QPushButton(keyName);

		// this G-Key is currently used - macro is defined
		if( ! (it->second).empty() )
			b->setProperty("class", QVariant("macroGKey")); // css class
		else {
			setButtonColor(b);
		}

		b->setObjectName(keyName);
		b->setFixedWidth(40);

		connect( b, &QPushButton::clicked, std::bind(&GKeysTab::updateInputsBox, this, it) );

		GKLog2(trace, "allocated G-Key QPushButton ", keyName.toStdString())
		return b;
	};

	auto newBanksLayout = [&nextLayoutName, &newMButton] (
		const std::vector<MKeysID> & banks ) -> QHBoxLayout*
	{
		QHBoxLayout* hBox = new QHBoxLayout();

		hBox->setObjectName( nextLayoutName() );
		GKLog2(trace, "allocated QHBoxLayout ", hBox->objectName().toStdString())

		for(const auto & id : banks) {
			if(id == MKeysID::MKEY_M0)
				continue; // skip virtual M0 key

			auto n = bankNames.at(id);
			hBox->addWidget( newMButton( id, n ) );
			GKLog2(trace, "allocated M-key bank button: ", n)
		}

		return hBox;
	};

	auto newButtonsLayout = [&nextLayoutName, &newGButton] (
		mBank_type::const_iterator & it1,
		mBank_type::const_iterator & it2,
		mBank_type::const_iterator & it3) -> QHBoxLayout*
	{
		QHBoxLayout* hBox = new QHBoxLayout();

		hBox->setObjectName( nextLayoutName() );

		GKLog2(trace, "allocated QHBoxLayout ", hBox->objectName().toStdString())

		hBox->addWidget( newGButton(it1) );
		hBox->addWidget( newGButton(it2) );
		hBox->addWidget( newGButton(it3) );

		return hBox;
	};

	/* -- -- -- */

	const banksMap_type & banks = device.getMacrosBanks();

	const mBank_type & bank = banks.at(currentID);

	//for(const auto & GMacroPair : bank) {
	//	LOG(trace)	<< "key|size: " << getGKeyName(GMacroPair.first)
	//				<< "|" << (GMacroPair.second).size();
	//}

	if( (bank.size() % keysPerLine) != 0 )
		throw GLogiKExcept("G-Keys modulo not null");

	/* -- -- -- */
	{
		QPushButton* button = _pInputsBox->findChild<QPushButton *>(_stubGKeyName);
		setButtonColor(button);
	}

	QVBoxLayout* keysBoxLayout = static_cast<QVBoxLayout*>(_pKeysBox->layout());

	clearLayout(keysBoxLayout);

	try {
		{	// initialize M-keys layout
			std::vector<MKeysID> ids;
			for(const auto & idBankPair : banks) {
				const MKeysID & bankID = idBankPair.first;
				ids.push_back(bankID);
			}

			keysBoxLayout->addLayout( newBanksLayout(ids) );
		}

		keysBoxLayout->addSpacing(10);
		unsigned short c = 0;

		/* G-keys layouts */
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

			keysBoxLayout->addLayout(
				newButtonsLayout(it1, it2, it3)
			);

			if((++c % 2) == 0)
				keysBoxLayout->addSpacing(20);
		}

		keysBoxLayout->addStretch();
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

		{ // keysBox + VLine + inputsBox
			QHBoxLayout* hBox = new QHBoxLayout();
			GKLog(trace, "allocated QHBoxLayout")

			vBox->addLayout(hBox);

			/* -- -- -- */
			{ // keysBox
				_pKeysBox = new QGroupBox();
				_pKeysBox->setTitle("");
				_pKeysBox->setObjectName("keysBox"); // css ID
				//_pKeysBox->setFlat(true);

				QVBoxLayout* keysBoxLayout = new QVBoxLayout();
				keysBoxLayout->setObjectName("KeysBoxLayout");

				_pKeysBox->setLayout(keysBoxLayout);

				hBox->addWidget(_pKeysBox);
				GKLog(trace, "keysBox added")
			}

			/* -- -- -- */
			hBox->addWidget( this->getVLine() );
			/* -- -- -- */

			/* -- -- -- */
			{ // inputsBox
				_pInputsBox = new QGroupBox();
				_pInputsBox->setTitle("");
				_pInputsBox->setObjectName("inputsBox"); // css ID
				//_pInputsBox->setFlat(true);

				QVBoxLayout* inputsBoxLayout = new QVBoxLayout();
				inputsBoxLayout->setObjectName("InputsVBoxLayout");

				_pInputsBox->setLayout(inputsBoxLayout);
				{ // header
					auto newStubGButton = [] (
						const QString & name, const QString & qssClass = "") -> QPushButton*
					{
						const QString keyName("G0");

						QPushButton* b = new QPushButton(keyName);

						if(qssClass != "") {
							b->setProperty("class", QVariant(qssClass)); // css class
						}
						b->setObjectName(name);
						b->setFixedWidth(40);
						b->setEnabled(false);

						GKLog2(trace, "allocated G-Key example QPushButton ", keyName.toStdString())
						return b;
					};

					QHBoxLayout* headerHBoxLayout = new QHBoxLayout();
					inputsBoxLayout->addLayout(headerHBoxLayout);

					_pRadioButtonsGroup = new QButtonGroup(this);
					QRadioButton* button1 = new QRadioButton("Macro");
					QRadioButton* button2 = new QRadioButton("Command");
					_pRadioButtonsGroup->addButton(button1);
					_pRadioButtonsGroup->addButton(button2);

					headerHBoxLayout->addWidget( this->getVLine() );
					headerHBoxLayout->addWidget( newStubGButton(_stubMacroKeyName, "macroGKey") );
					headerHBoxLayout->addWidget(button1);
					headerHBoxLayout->addWidget( this->getVLine() );

					headerHBoxLayout->addStretch();

					headerHBoxLayout->addWidget( newStubGButton(_stubGKeyName) );
					headerHBoxLayout->addWidget( new QLabel("not used") );

					headerHBoxLayout->addStretch();

					headerHBoxLayout->addWidget( this->getVLine() );
					headerHBoxLayout->addWidget( newStubGButton(_stubCommandKeyName, "cmmndGKey") );
					headerHBoxLayout->addWidget(button2);
					headerHBoxLayout->addWidget( this->getVLine() );

					this->setRadioButtonsEnabled(false);

					GKLog(trace, "inputsBox header added")
				}

				inputsBoxLayout->addWidget( this->getHLine() );

				// --
				inputsBoxLayout->addStretch();

				// --
				hBox->addWidget(_pInputsBox);
				GKLog(trace, "inputsBox added")
			}
		} // end (keysBox + VLine + inputsBox)

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

void GKeysTab::setRadioButtonsEnabled(const bool status)
{
	if(_pRadioButtonsGroup == nullptr)
		return;

	using T = QList<QAbstractButton*>;

	const T radioButtons(_pRadioButtonsGroup->buttons());

	for(T::const_iterator it = radioButtons.constBegin();
		it != radioButtons.constEnd(); ++it)
	{
		(*it)->setEnabled(status);
	}

}

void GKeysTab::updateInputsBox(mBank_type::const_iterator & it)
{
	const GKeysID & GKeyID = it->first;

	auto setStubButtonText = [this, &GKeyID] (const QString & stubButtonName) -> void {
		QPushButton* button = _pInputsBox->findChild<QPushButton *>(stubButtonName);
		if(! button) {
			GKLog(warning, "cannot find stub G-Key")
		}
		else {
			const QString keyName(getGKeyName(GKeyID).c_str());
			button->setText(keyName);
		}
	};

	setStubButtonText(_stubMacroKeyName);
	setStubButtonText(_stubGKeyName);
	setStubButtonText(_stubCommandKeyName);
}

} // namespace GLogiK

