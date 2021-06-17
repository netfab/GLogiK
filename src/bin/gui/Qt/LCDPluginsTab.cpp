/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2021  Fabrice Delliaux <netbox253@gmail.com>
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

#include <QStringList>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFrame>
#include <QScrollArea>
#include <QHeaderView>
#include <QTableWidgetItem>
#include <QCheckBox>

#include "lib/shared/glogik.hpp"
#include "lib/utils/utils.hpp"

#include "include/LCDPluginProperties.hpp"

#include "LCDPluginsTab.hpp"

namespace GLogiK
{

using namespace NSGKUtils;

LCDPluginsTab::LCDPluginsTab(
	NSGKDBus::GKDBus* pDBus,
	const QString & name)
	:	Tab(pDBus),
		_LCDPluginsMask(0),
		_newLCDPluginsMask(0),
		_pPluginsTable(nullptr)
{
	this->setObjectName(name);
}

LCDPluginsTab::~LCDPluginsTab()
{
}

void LCDPluginsTab::buildTab(void)
{
	GK_LOG_FUNC

	QVBoxLayout* vBox = nullptr;

	try {
		vBox = new QVBoxLayout(this);
		GKLog(trace, "allocated QVBoxLayout")

		this->setLayout(vBox);

		/* -- -- -- */
		vBox->addWidget( this->getHLine() );

		/* -- -- -- */
		QFrame* mainFrame = new QFrame();
		vBox->addWidget(mainFrame);

		QVBoxLayout* scrollLayout = new QVBoxLayout();
		mainFrame->setLayout(scrollLayout);

		QScrollArea* scrollArea = new QScrollArea();
		scrollLayout->addWidget(scrollArea);

		scrollArea->setWidgetResizable(true);
		scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
		scrollLayout->setContentsMargins(0, 0, 0, 0);

		_pPluginsTable = new QTableWidget(0, 3);
		scrollArea->setWidget(_pPluginsTable);

		const QStringList header = { "enabled", "name", "description" };
		_pPluginsTable->setHorizontalHeaderLabels(header);
		_pPluginsTable->horizontalHeader()->setStretchLastSection(true);
		_pPluginsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
		_pPluginsTable->horizontalHeader()->setSectionsClickable(false);
		_pPluginsTable->verticalHeader()->setVisible(false);
		_pPluginsTable->setShowGrid(false);
		_pPluginsTable->setSelectionMode(QAbstractItemView::NoSelection);
		_pPluginsTable->setFocusPolicy(Qt::NoFocus);

		/* -- -- -- */
		vBox->addWidget( this->getHLine() );

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

		/* -- -- -- */
		vBox->addWidget( this->getHLine() );
	}
	catch (const std::bad_alloc& e) {
		LOG(error) << "bad allocation : " << e.what();
		throw;
	}
}

void LCDPluginsTab::updateTab(
	const std::string & devID,
	const DeviceProperties & device)
{
	GK_LOG_FUNC

	disconnect(_pPluginsTable, &QTableWidget::cellClicked, this, &LCDPluginsTab::toggleCheckbox);

	/* removing table items */
	_pPluginsTable->clearContents();

	const std::string remoteMethod("GetDeviceLCDPluginsProperties");
	try {
		_pDBus->initializeRemoteMethodCall(
			NSGKDBus::BusConnection::GKDBUS_SESSION,
			GLOGIK_DESKTOP_SERVICE_DBUS_BUS_CONNECTION_NAME,
			GLOGIK_DESKTOP_SERVICE_SESSION_DBUS_OBJECT_PATH,
			GLOGIK_DESKTOP_SERVICE_SESSION_DBUS_INTERFACE,
			remoteMethod.c_str()
		);
		_pDBus->appendStringToRemoteMethodCall(devID);
		_pDBus->appendStringToRemoteMethodCall("reserved");

		_pDBus->sendRemoteMethodCall();

		try {
			_pDBus->waitForRemoteMethodCallReply();
			const LCDPluginsPropertiesArray_type array = _pDBus->getNextLCDPluginsArrayArgument();

			_pPluginsTable->setRowCount( array.size() );

			int c = 0;
			_LCDPluginsMask = device.getLCDPluginsMask1();
			_newLCDPluginsMask = _LCDPluginsMask;

			for(const auto & plugin : array) {
				QCheckBox* checkbox =  new QCheckBox();
				_pPluginsTable->setCellWidget(c, 0, checkbox);

				/* centering checkbox */
				checkbox->setStyleSheet("margin:auto;");
				checkbox->setChecked( (_LCDPluginsMask & plugin.getID()) );

				{
					const qulonglong id = plugin.getID();
					const QVariant value(id);
					checkbox->setProperty(_idProperty.c_str(), value);
				}

				/* connecting checkbox */
				connect(checkbox, &QCheckBox::stateChanged, this, &LCDPluginsTab::updateNewLCDPluginsMask);

				QTableWidgetItem* item = nullptr;

				item = new QTableWidgetItem( plugin.getName().c_str() );
				_pPluginsTable->setItem(c, 1, item);
				item->setTextAlignment(Qt::AlignCenter);
				item->setFlags( Qt::NoItemFlags | Qt::ItemIsEnabled );

				item = new QTableWidgetItem( plugin.getDesc().c_str() );
				_pPluginsTable->setItem(c, 2, item);
				item->setTextAlignment(Qt::AlignVCenter);
				item->setFlags( Qt::NoItemFlags | Qt::ItemIsEnabled );

				++c;
			}

			_pPluginsTable->resizeColumnsToContents();

			/* connect cellClicked events to checkbox toggle */
			connect(_pPluginsTable, &QTableWidget::cellClicked, this, &LCDPluginsTab::toggleCheckbox);
		}
		catch (const GLogiKExcept & e) {
			LogRemoteCallGetReplyFailure
			throw GLogiKExcept("failure to get request reply");
		}
	}
	catch (const GKDBusMessageWrongBuild & e) {
		_pDBus->abandonRemoteMethodCall();
		LogRemoteCallFailure
		throw GLogiKExcept("failure to build request");
	}
}

const uint64_t LCDPluginsTab::getAndSetNewLCDPluginsMask(void)
{
	 _LCDPluginsMask = _newLCDPluginsMask;
	_pApplyButton->setEnabled( !(_LCDPluginsMask == _newLCDPluginsMask) );
	return _newLCDPluginsMask;
}

void LCDPluginsTab::toggleCheckbox(int row, int column)
{
	GK_LOG_FUNC

	if(column != 0)
		return;

	QCheckBox* check = dynamic_cast<QCheckBox*>( _pPluginsTable->cellWidget(row, column) );
	if( check )
		check->toggle();
	else {
		LOG(warning) << "wrong dynamic cast";
	}
}

void LCDPluginsTab::updateNewLCDPluginsMask(int checkboxState)
{
	GK_LOG_FUNC

	QObject* cb = sender();
	const QVariant value = cb->property(_idProperty.c_str());
	/* checking that property was found */
	if( value.isValid() ) {
		bool converted = false;
		const qulonglong id = value.toULongLong(&converted);
		if( converted and id > 0 ) {
			const uint64_t pluginID(id);
			if( checkboxState == Qt::Checked )  {
				_newLCDPluginsMask |= pluginID;
			}
			else if( checkboxState == Qt::Unchecked ) {
				_newLCDPluginsMask &= ~(pluginID);
			}
			else {
				LOG(warning) << "unhandled checkbox state : " << checkboxState;
			}

			_pApplyButton->setEnabled( !(_LCDPluginsMask == _newLCDPluginsMask) );
			GKLog4(trace, "id: ", id, "mask: ", _newLCDPluginsMask)
		}
		else {
			LOG(warning) << "conversion failure";
		}
	}
	else {
		LOG(warning) << "invalid QVariant";
	}
}

}

