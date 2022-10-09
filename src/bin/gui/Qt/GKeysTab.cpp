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
		_pKeysBox(nullptr),
		_pInputsBox(nullptr),
		_pHeaderHBoxLayout(nullptr),
		_pHelpLabel(nullptr),
		_currentBankID(MKeysID::MKEY_M0),
		_helpLabel("Click on a G-Key and/or a M-Key")
{
	this->setObjectName(name);
}

GKeysTab::~GKeysTab()
{
}

void GKeysTab::updateTab(const DeviceProperties & device, const bool resetCurrentBankID)
{
	GK_LOG_FUNC

	GKLog(trace, "updating GKeysTab")

	unsigned int layoutNum = 0;

	const unsigned short keysPerLine = 3;	// TODO

	uint8_t r, g, b = 0; device.getRGBBytes(r, g, b);
	const QColor color(r, g, b);
	const QString colorName = color.name();
	GKLog2(trace, "got color: ", colorName.toStdString())

	/* -- -- -- */

	auto nextLayoutName = [&layoutNum] () -> const QString
	{
		QString layoutName("hBox ");
		layoutName += std::to_string(layoutNum).c_str();
		layoutNum++;
		return layoutName;
	};

	auto newMButton = [this, &device] (
			const MKeysID & bankID,
			const QString & keyName) -> QPushButton*
	{
		QPushButton* button = new QPushButton(keyName);

		if(bankID == _currentBankID)
			button->setProperty("class", QVariant("currentBank")); // css class
		else
			button->setProperty("class", QVariant("inactiveBank")); // css class

		button->setObjectName(keyName);
		button->setFixedWidth(32);

		QObject::connect( button, &QPushButton::clicked, std::bind(&GKeysTab::updateCurrentBankID, this, device, bankID) );

		GKLog2(trace, "allocated M-Key QPushButton ", keyName.toStdString())
		return button;
	};

	auto newGButton = [this, &device, &colorName] (mBank_type::const_iterator & it) -> QPushButton*
	{
		const GKeysID GKeyID = it->first;
		const GKeyEventType eventType = (it->second).getEventType();

		QPushButton* button = this->newGKeyButton(GKeyID, eventType, colorName);
		QObject::connect( button, &QPushButton::clicked, std::bind(&GKeysTab::updateInputsBox, this, device, GKeyID) );
		return button;
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

	auto newGKeysLayout = [&nextLayoutName, &newGButton] (
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

	if( resetCurrentBankID )
		_currentBankID = MKeysID::MKEY_M0;

	const banksMap_type & banks = device.getBanks();

	const mBank_type & bank = banks.at(_currentBankID);

	//for(const auto & GMacroPair : bank) {
	//	LOG(trace)	<< "key|size: " << getGKeyName(GMacroPair.first)
	//				<< "|" << (GMacroPair.second).size();
	//}

	if( (bank.size() % keysPerLine) != 0 )
		throw GLogiKExcept("G-Keys modulo not null");

	QVBoxLayout* keysBoxLayout = static_cast<QVBoxLayout*>(_pKeysBox->layout());

	this->clearLayout(keysBoxLayout);

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
				newGKeysLayout(it1, it2, it3)
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

				_pHelpLabel = new QLabel(_helpLabel);
				inputsBoxLayout->addWidget( _pHelpLabel );

				inputsBoxLayout->addWidget( this->getHLine() );

				{ // header
					_pHeaderHBoxLayout = new QHBoxLayout();
					inputsBoxLayout->addLayout(_pHeaderHBoxLayout);

					QPushButton* button = this->newBlankButton();
					_pHeaderHBoxLayout->addWidget(button);

					_pHeaderHBoxLayout->addStretch();

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

QPushButton* GKeysTab::newBlankButton(void)
{
	QPushButton* button = new QPushButton("");

	button->setProperty("class", QVariant("blankButton")); // css class
	button->setObjectName("BlankButton");
	button->setFixedWidth(40);
	button->setEnabled(false);

	return button;
}

QPushButton* GKeysTab::newGKeyButton(
	const GKeysID GKeyID,
	const GKeyEventType eventType,
	const QString & colorName)
{
	const QString buttonText(getGKeyName(GKeyID).c_str());

	QPushButton* button = new QPushButton(buttonText);

	button->setObjectName(buttonText);
	button->setFixedWidth(40);

	if( eventType == GKeyEventType::GKEY_MACRO ) {
		button->setProperty("class", QVariant("macroGKey")); // css class
	}
	else if( eventType == GKeyEventType::GKEY_INACTIVE ) {
		button->setProperty("class", QVariant("inactiveGKey")); // css class
		QString style = "color:";
		style += colorName;
		style += ";";

		button->setStyleSheet(style);
	}

	GKLog2(trace, "allocated G-Key QPushButton ", buttonText.toStdString())
	return button;
}

void GKeysTab::updateInputsBox(const DeviceProperties & device, const GKeysID GKeyID)
{
	const banksMap_type & banks = device.getBanks();
	const mBank_type & bank = banks.at(_currentBankID);
	const GKeyEventType eventType = bank.at(GKeyID).getEventType();

	uint8_t r, g, b = 0; device.getRGBBytes(r, g, b);
	const QColor color(r, g, b);
	const QString colorName = color.name();
	GKLog2(trace, "got color: ", colorName.toStdString())

	/* -- -- -- */

	/* ->clear() is producing a visual artefact */
	//_pHelpLabel->clear();
	_pHelpLabel->setText("");

	this->clearLayout(_pHeaderHBoxLayout);

	/* -- -- -- */

	QPushButton* button = this->newGKeyButton(GKeyID, eventType, colorName);

	button->setEnabled(false);

	_pHeaderHBoxLayout->addWidget(button);

	_pHeaderHBoxLayout->addStretch();
}

QPushButton* GKeysTab::findButtonIn(QObject* parentWidget, const QString & buttonName)
{
	QPushButton* button = nullptr;
	if(! parentWidget) {
		GKLog(error, "null parent widget")
	}
	else {
		button = parentWidget->findChild<QPushButton *>(buttonName);
	}
	return button;
}

void GKeysTab::setStubButtonText(const QString & buttonName, const QString & buttonText)
{
	QPushButton* button = this->findButtonIn(_pInputsBox, buttonName);
	if(! button) {
		GKLog(warning, "cannot find stub key")
	}
	else {
		button->setText(buttonText);
	}
}

void GKeysTab::updateCurrentBankID(const DeviceProperties & device, const MKeysID bankID)
{
	_currentBankID = (_currentBankID == bankID) ? MKeysID::MKEY_M0 : bankID;

	/* -- -- -- */

	_pHelpLabel->setText(_helpLabel);

	this->clearLayout(_pHeaderHBoxLayout);

	/* -- -- -- */

	QPushButton* button = this->newBlankButton();
	_pHeaderHBoxLayout->addWidget(button);
	_pHeaderHBoxLayout->addStretch();

	/* -- -- -- */

	this->updateTab(device);
}

} // namespace GLogiK

