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
#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStandardItemModel>

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
		_pKeysBoxLayout(nullptr),
		_pInputsBoxHeaderLayout(nullptr),
		_pHelpLabel(nullptr),
		_GKeyEventTypeComboBox(nullptr),
		_currentBankID(MKeysID::MKEY_M0),
		_helpLabel("Click on a G-Key and/or a M-Key"),
		_updateGKeyEvent(false)
{
	this->setObjectName(name);
}

GKeysTab::~GKeysTab()
{
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
				QGroupBox* keysBox = new QGroupBox();
				keysBox->setTitle("");
				keysBox->setObjectName("keysBox"); // css ID

				_pKeysBoxLayout = new QVBoxLayout();
				_pKeysBoxLayout->setObjectName("KeysBoxLayout");

				keysBox->setLayout(_pKeysBoxLayout);

				hBox->addWidget(keysBox);
				GKLog(trace, "keysBox added")
			}

			/* -- -- -- */
			hBox->addWidget( this->getVLine() );
			/* -- -- -- */

			/* -- -- -- */
			{ // inputsBox
				QGroupBox* inputsBox = new QGroupBox();
				inputsBox->setTitle("");
				inputsBox->setObjectName("inputsBox"); // css ID

				QVBoxLayout* inputsBoxLayout = new QVBoxLayout();
				inputsBoxLayout->setObjectName("InputsVBoxLayout");

				inputsBox->setLayout(inputsBoxLayout);

				_pHelpLabel = new QLabel(_helpLabel);
				inputsBoxLayout->addWidget( _pHelpLabel );

				inputsBoxLayout->addWidget( this->getHLine() );

				{ // header
					_pInputsBoxHeaderLayout = new QHBoxLayout();
					inputsBoxLayout->addLayout(_pInputsBoxHeaderLayout);

					GKLog(trace, "inputsBox header added")
				}

				inputsBoxLayout->addWidget( this->getHLine() );

				// --
				inputsBoxLayout->addStretch();

				// --
				hBox->addWidget(inputsBox);

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
			_pApplyButton->setEnabled(false);

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

void GKeysTab::updateTab(const DeviceProperties & device, const MKeysID bankID)
{
	GK_LOG_FUNC

	_currentBankID = (_currentBankID == bankID) ? MKeysID::MKEY_M0 : bankID;

	this->setApplyButtonStatus(false);

	/* exceptions can be thrown from QPushButton::clicked events
	 * see newMButton() lambda in ::redrawTab()
	 */
	try {
		this->redrawTab(device);
	}
	catch (const GLogiKExcept & e) {
		LOG(error) << "redrawing tab failure: " << e.what();
	}
}

void GKeysTab::getGKeyEventParams(MKeysID & bankID, GKeysID & keyID, GKeyEventType & eventType)
{
	if(! _updateGKeyEvent)
		throw GLogiKExcept("internal logic error");

	bankID = _currentBankID;
	keyID  = _currentGKeyID;
	eventType = _newEventType;

	this->setApplyButtonStatus(false);
}

/*
 * -- private
 */

void GKeysTab::setApplyButtonStatus(const bool status)
{
	_updateGKeyEvent = status;
	_pApplyButton->setEnabled(status);
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
	GK_LOG_FUNC

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

void GKeysTab::clearInputsBoxHeaderLayout(void)
{
	GK_LOG_FUNC

	if(_GKeyEventTypeComboBox) {
		GKLog(trace, "disconnecting GKeyEventTypeComboBox currentIndexChanged signal")
		// make sure combobox's currentIndexChanged signal is disconnected
		// before clearing layout and deleting widget
		QObject::disconnect(_GKeyEventTypeComboBox, nullptr, nullptr, nullptr);
	}

	this->clearLayout(_pInputsBoxHeaderLayout);

	_GKeyEventTypeComboBox = nullptr;
}

void GKeysTab::clearKeysBoxLayout(void)
{
	GK_LOG_FUNC

	for(auto button : _buttonsSignalsToClear) {
		if(button) {
			GKLog2(trace, "disconnecting ::clicked signal for button: ", button->objectName().toStdString())
			QObject::disconnect(button, nullptr, nullptr, nullptr);
		}
	}
	_buttonsSignalsToClear.clear();

	this->clearLayout(_pKeysBoxLayout);
}

void GKeysTab::updateInputsBox(const DeviceProperties & device, const GKeysID GKeyID)
{
	GK_LOG_FUNC

	this->setApplyButtonStatus(false);

	try {
		const banksMap_type & banks = device.getBanks();
		const mBank_type & bank = banks.at(_currentBankID);
		const GKeysEvent & event = bank.at(GKeyID);
		const GKeyEventType eventType = event.getEventType();

		uint8_t r, g, b = 0; device.getRGBBytes(r, g, b);
		const QColor color(r, g, b);
		const QString colorName = color.name();
		GKLog2(trace, "got color: ", colorName.toStdString())

		/* -- -- -- */

		/* ->clear() is producing a visual artefact */
		//_pHelpLabel->clear();
		_pHelpLabel->setText("");

		this->clearInputsBoxHeaderLayout();

		/* -- -- -- */

		QPushButton* button = this->newGKeyButton(GKeyID, eventType, colorName);
		button->setEnabled(false);

		_GKeyEventTypeComboBox = new QComboBox();
		GKLog(trace, "allocated QComboBox")
		_GKeyEventTypeComboBox->setObjectName("GKeyEventTypeComboBox");

		{
			/* see QVariant::QVariant(uint val) constructor */
			uint v = static_cast<unsigned int>(GKeyEventType::GKEY_INACTIVE);
			const QVariant eType1(v);
			v = static_cast<unsigned int>(GKeyEventType::GKEY_MACRO);
			const QVariant eType2(v);

			_GKeyEventTypeComboBox->addItem("inactive", eType1);
			_GKeyEventTypeComboBox->addItem("macro", eType2);

			GKeyEventType itemEventType = eventType;
			const bool emptyMacro = event.getMacro().empty();

			if(emptyMacro) {
				QStandardItemModel* model = qobject_cast<QStandardItemModel*>(_GKeyEventTypeComboBox->model());
				if(model == nullptr)
					throw GLogiKExcept("can't get QStandardItemModel pointer");

				QStandardItem *item = model->item(1);
				/* disabling combobox item since macro is empty */
				item->setFlags( item->flags() & ~Qt::ItemIsEnabled );
			}

			if(itemEventType == GKeyEventType::GKEY_MACRO) {
				if(emptyMacro) { // sanity check
					LOG(warning) << "empty macro event";
					itemEventType = GKeyEventType::GKEY_INACTIVE;
				}
				else
					_GKeyEventTypeComboBox->setCurrentIndex(1);
			}

			if(itemEventType == GKeyEventType::GKEY_INACTIVE)
				_GKeyEventTypeComboBox->setCurrentIndex(0);

			_GKeyEventTypeComboBox->setEnabled(true);
		}

		_pInputsBoxHeaderLayout->addWidget(button);
		_pInputsBoxHeaderLayout->addWidget(_GKeyEventTypeComboBox);
		_pInputsBoxHeaderLayout->addStretch();

		QObject::connect(
			_GKeyEventTypeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged),
			std::bind(&GKeysTab::switchGKeyEventType, this, device, GKeyID)
		);
	}
	catch (const std::bad_alloc& e) {
		LOG(error) << "bad allocation detected: " << e.what();
	}
	catch (const std::out_of_range& oor) {
		LOG(error) << "out of range detected: " << oor.what();
	}
	catch (const GLogiKExcept & e) {
		LOG(error) << e.what();
	}
}

void GKeysTab::switchGKeyEventType(const DeviceProperties & device, const GKeysID GKeyID)
{
	GK_LOG_FUNC

	try {
		auto getDataEventType = [] (const QVariant & itemData) -> const GKeyEventType
		{
			bool ok = false;
			const uint value = itemData.toUInt(&ok);
			if(ok and value <= 255) {
				if(value >= (static_cast<unsigned int>(GKeyEventType::GKEY_INVALID))) {
					throw GLogiKExcept("invalid GKeyEventType value");
				}

				return static_cast<GKeyEventType>(value);
			}
			throw GLogiKExcept("invalid value or conversion failure");
		};

		const int index = _GKeyEventTypeComboBox->currentIndex();
		GKLog2(trace, "GKey ComboBox index: ", index)

		const banksMap_type & banks = device.getBanks();
		const mBank_type & bank = banks.at(_currentBankID);
		const GKeysEvent & event = bank.at(GKeyID);
		const QVariant data = _GKeyEventTypeComboBox->itemData(index).value<QVariant>();

		_newEventType = getDataEventType(data);
		_currentGKeyID = GKeyID;

		this->setApplyButtonStatus( (event.getEventType() != _newEventType) );
	}
	catch (const std::out_of_range& oor) {
		LOG(error) << "out of range detected: " << oor.what();
	}
	catch (const GLogiKExcept & e) {
		LOG(error) << "error getting event type: " << e.what();
	}
}

void GKeysTab::redrawTab(const DeviceProperties & device)
{
	GK_LOG_FUNC

	GKLog(trace, "redrawing GKeysTab")

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

		QObject::connect( button, &QPushButton::clicked, std::bind(&GKeysTab::updateTab, this, device, bankID) );
		_buttonsSignalsToClear.push_back(button);

		GKLog2(trace, "allocated M-Key QPushButton ", keyName.toStdString())
		return button;
	};

	auto newGButton = [this, &device, &colorName] (mBank_type::const_iterator & it) -> QPushButton*
	{
		const GKeysID GKeyID = it->first;
		const GKeyEventType eventType = (it->second).getEventType();

		QPushButton* button = this->newGKeyButton(GKeyID, eventType, colorName);
		QObject::connect( button, &QPushButton::clicked, std::bind(&GKeysTab::updateInputsBox, this, device, GKeyID) );
		_buttonsSignalsToClear.push_back(button);
		return button;
	};

	auto newBanksLayout = [&nextLayoutName, &newMButton] (
		const std::vector<MKeysID> & banks ) -> QHBoxLayout*
	{
		QHBoxLayout* hBox = new QHBoxLayout();

		hBox->setObjectName( nextLayoutName() );
		GKLog2(trace, "allocated QHBoxLayout ", hBox->objectName().toStdString())

		try {
			for(const auto & id : banks) {
				if(id == MKeysID::MKEY_M0)
					continue; // skip virtual M0 key

				auto n = bankNames.at(id);
				hBox->addWidget( newMButton( id, n ) );
				GKLog2(trace, "allocated M-key bank button: ", n)
			}
		}
		catch (const std::out_of_range& oor) {
			throw GLogiKExcept("newBanksLayout: id not found");
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

	try {
		{ // resetting right panel
			_pHelpLabel->setText(_helpLabel);

			this->clearInputsBoxHeaderLayout();

			/* -- -- -- */

			QPushButton* button = this->newBlankButton();
			_pInputsBoxHeaderLayout->addWidget(button);
			_pInputsBoxHeaderLayout->addStretch();
		} //

		/* -- -- -- */
		// redrawing left panel

		const banksMap_type & banks = device.getBanks();
		const mBank_type & bank = banks.at(_currentBankID);

		//for(const auto & GMacroPair : bank) {
		//	LOG(trace)	<< "key|size: " << getGKeyName(GMacroPair.first)
		//				<< "|" << (GMacroPair.second).size();
		//}

		if( (bank.size() % keysPerLine) != 0 )
			throw GLogiKExcept("G-Keys modulo not null");

		this->clearKeysBoxLayout();

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

				_pKeysBoxLayout->addLayout(
					newGKeysLayout(it1, it2, it3)
				);

				if((++c % 2) == 0)
					_pKeysBoxLayout->addSpacing(20);
			}

			_pKeysBoxLayout->addStretch();
		}
		catch (const std::bad_alloc& e) {
			LOG(error) << "bad allocation : " << e.what();
			throw GLogiKBadAlloc("bad allocation");
		}
	}
	catch (const std::out_of_range& oor) {
		throw GLogiKExcept("currentBankID not found");
	}
}

} // namespace GLogiK

