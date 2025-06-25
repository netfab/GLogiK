/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2025  Fabrice Delliaux <netbox253@gmail.com>
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

#include <QFont>
#include <QString>
#include <QLabel>
#include <QVBoxLayout>

#include "lib/utils/utils.hpp"

#include "AboutTab.hpp"

namespace GLogiK
{

using namespace NSGKUtils;

void AboutTab::buildTab(void)
{
	GK_LOG_FUNC

	try {
		QVBoxLayout* vBox = new QVBoxLayout(this);
		GKLog(trace, "allocated QVBoxLayout")

		this->setLayout(vBox);

		vBox->addWidget( this->getHLine() );

		{
			QString mainText("GKcQt ");
			mainText += VERSION;

			QLabel* mainLabel = new QLabel(mainText);
			vBox->addWidget(mainLabel);

			vBox->setAlignment(mainLabel, Qt::AlignHCenter);

			QFont cFont = mainLabel->font();
			cFont.setPointSize(18);
			cFont.setBold(true);
			mainLabel->setFont(cFont);
		}

		/* -- -- */

		vBox->addWidget( this->getHLine() );

		/* -- -- */

		{
			QString descText("A Qt graphical user interface for the\nGLogiK desktop service.");

			QLabel* descLabel = new QLabel(descText);
			vBox->addWidget(descLabel);
			descLabel->setAlignment(Qt::AlignHCenter);

			vBox->setAlignment(descLabel, Qt::AlignHCenter);

			QFont cFont = descLabel->font();
			cFont.setPointSize(10);
			descLabel->setFont(cFont);
		}

		{
			QString descText("GLogiK, daemon and utilities to handle\nspecial features on some gaming keyboards.");

			QLabel* descLabel = new QLabel(descText);
			vBox->addWidget(descLabel);
			descLabel->setAlignment(Qt::AlignHCenter);

			vBox->setAlignment(descLabel, Qt::AlignHCenter);

			QFont cFont = descLabel->font();
			cFont.setPointSize(10);
			descLabel->setFont(cFont);
		}

		{
			QString descText("Copyright 2016 - 2025 Fabrice Delliaux");

			QLabel* descLabel = new QLabel(descText);
			vBox->addWidget(descLabel);
			descLabel->setAlignment(Qt::AlignHCenter);

			vBox->setAlignment(descLabel, Qt::AlignHCenter);

			QFont cFont = descLabel->font();
			cFont.setPointSize(8);
			descLabel->setFont(cFont);
		}

		{
			QString descText("<a href=\"");
			descText += PACKAGE_URL;
			descText += "\">";
			descText += PACKAGE_URL;
			descText += "</a>";

			QLabel* descLabel = new QLabel(descText);
			vBox->addWidget(descLabel);
			descLabel->setAlignment(Qt::AlignHCenter);

			vBox->setAlignment(descLabel, Qt::AlignHCenter);

			QFont cFont = descLabel->font();
			cFont.setPointSize(8);
			descLabel->setFont(cFont);

			descLabel->setTextFormat(Qt::RichText);
			descLabel->setTextInteractionFlags(Qt::TextBrowserInteraction);
			descLabel->setOpenExternalLinks(true);
		}

		{
			QString descText("GLogiK is distributed under the GNU General Public Licence\n\
			version 3, or (at your option) any later version. Please see\n\
			COPYING file (or the license tab) for the complete text.");

			QLabel* descLabel = new QLabel(descText);
			vBox->addWidget(descLabel);
			descLabel->setAlignment(Qt::AlignHCenter);

			vBox->setAlignment(descLabel, Qt::AlignHCenter);

			QFont cFont = descLabel->font();
			cFont.setPointSize(8);
			descLabel->setFont(cFont);
		}

		vBox->addWidget( this->getHLine() );
	}
	catch (const std::bad_alloc& e) {
		LOG(error) << "bad allocation : " << e.what();
		throw;
	}
}

} // namespace GLogiK
