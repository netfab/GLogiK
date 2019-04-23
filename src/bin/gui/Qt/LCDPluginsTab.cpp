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

#include <QFrame>
#include <QScrollArea>
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
	:	Tab(pDBus)
{
	this->setObjectName(name);
}

LCDPluginsTab::~LCDPluginsTab()
{
}

void LCDPluginsTab::buildTab(void)
{
	QVBoxLayout* vBox = nullptr;

	try {
		vBox = new QVBoxLayout(this);
#if DEBUGGING_ON
		LOG(DEBUG1) << "allocated QVBoxLayout";
#endif
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

		_checkboxesLayout = new QVBoxLayout();
		try {
			QFrame* checkboxesFrame = new QFrame();
			checkboxesFrame->setLayout(_checkboxesLayout);

			scrollArea->setWidget(checkboxesFrame);
		}
		catch (const std::bad_alloc& e) {
			delete _checkboxesLayout;
			throw;
		}
	}
	catch (const std::bad_alloc& e) {
		LOG(ERROR) << e.what();
		throw;
	}
}

void LCDPluginsTab::updateTab(const std::string & devID)
{
	/*  removing layout items */
	QLayoutItem* child;
	while ((child = _checkboxesLayout->takeAt(0)) != 0) {
		delete child->widget();
		delete child;
	}

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
			for(const auto & plugin : array) {
				_checkboxesLayout->addWidget(new QCheckBox( plugin.getName().c_str() ));
			}

			_checkboxesLayout->addStretch();
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

}

