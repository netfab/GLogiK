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

#ifndef SRC_BIN_GUI_QT_TAB_HPP_
#define SRC_BIN_GUI_QT_TAB_HPP_

#include <QWidget>
#include <QFrame>
#include <QPushButton>

#include "lib/dbus/GKDBus.hpp"

#define LogRemoteCallFailure \
	FATALERROR << remoteMethod.c_str() << CONST_STRING_METHOD_CALL_FAILURE << e.what();
#define LogRemoteCallGetReplyFailure \
	LOG(ERROR) << remoteMethod.c_str() << CONST_STRING_METHOD_REPLY_FAILURE << e.what();

namespace GLogiK
{

class Tab
	:	public QWidget
{
	public:
		Tab(NSGKDBus::GKDBus* pDBus)
			:	_pDBus(pDBus),
				_pApplyButton(nullptr)
			{};
		virtual ~Tab() = default;
		Tab() = delete;

		virtual void buildTab(void) = 0;
		virtual void updateTab(void) {};

		const QPushButton* getApplyButton(void) const;
		QFrame* getHLine(void);

	protected:
		NSGKDBus::GKDBus* _pDBus;

		QPushButton* _pApplyButton;

		void prepareApplyButton(void);

	private:
};

} // namespace GLogiK

#endif
