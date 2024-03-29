/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2023  Fabrice Delliaux <netbox253@gmail.com>
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

#ifndef SRC_BIN_GUI_QT_ABOUT_DIALOG_HPP_
#define SRC_BIN_GUI_QT_ABOUT_DIALOG_HPP_

#include <QWidget>
#include <QDialog>
#include <QPushButton>

#include "include/DepsMap.hpp"

namespace GLogiK
{

class AboutDialog
	:	public QDialog
{
	public:
		AboutDialog(QWidget* parent = 0);
		~AboutDialog();

		void buildDialog(const GKDepsMap_type* const pDepsMap);

	protected:

	private:
		QPushButton* _pCopyButton;

		void closeDialog(void);
		void setCopyButtonVisibility(int index);
		void copyToClipboard(const GKDepsMap_type* const pDepsMap);

};

} // namespace GLogiK

#endif
