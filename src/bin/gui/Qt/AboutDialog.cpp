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

#include <QStyle>
#include <QPushButton>
#include <QString>
#include <QFont>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>

#include "lib/utils/utils.hpp"

#include "AboutDialog.hpp"

namespace GLogiK
{

using namespace NSGKUtils;

AboutDialog::AboutDialog(QWidget* parent)
	:	QDialog(parent)
{
}

AboutDialog::~AboutDialog()
{
	GK_LOG_FUNC

	GKLog(trace, "deleting AboutDialog")
}

void AboutDialog::buildDialog(void)
{
	GK_LOG_FUNC

	QVBoxLayout* vBox = nullptr;

	try {
		vBox = new QVBoxLayout(this);
		GKLog(trace, "allocated QVBoxLayout")

		this->setLayout(vBox);

		vBox->addSpacing(10);

		/* -- -- */
		{
			QString mainText("GKcQt5 ");
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

		vBox->addSpacing(10);

		/* -- -- */

		{
			QString descText("A Qt5 graphical user interface for the\nGLogiK desktop service.");

			QLabel* descLabel = new QLabel(descText);
			vBox->addWidget(descLabel);
			descLabel->setAlignment(Qt::AlignHCenter);

			vBox->setAlignment(descLabel, Qt::AlignHCenter);

			QFont cFont = descLabel->font();
			cFont.setPointSize(10);
			descLabel->setFont(cFont);
		}

		{
			QString descText("GLogiK, daemon to handle special features on\nsome gaming keyboards.");

			QLabel* descLabel = new QLabel(descText);
			vBox->addWidget(descLabel);
			descLabel->setAlignment(Qt::AlignHCenter);

			vBox->setAlignment(descLabel, Qt::AlignHCenter);

			QFont cFont = descLabel->font();
			cFont.setPointSize(10);
			descLabel->setFont(cFont);
		}

		{
			QString descText("Copyright 2016 - 2020 Fabrice Delliaux");

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
			COPYING file for the complete text.");

			QLabel* descLabel = new QLabel(descText);
			vBox->addWidget(descLabel);
			descLabel->setAlignment(Qt::AlignHCenter);

			vBox->setAlignment(descLabel, Qt::AlignHCenter);

			QFont cFont = descLabel->font();
			cFont.setPointSize(8);
			descLabel->setFont(cFont);
		}

		vBox->addStretch();

		/* -- -- */

		QHBoxLayout* hBox = new QHBoxLayout();
		GKLog(trace, "allocated QHBoxLayout")

		vBox->addLayout(hBox);

		hBox->addStretch();

		{
			QStyle* cStyle = this->style();
			QIcon closeIcon = cStyle->standardIcon(QStyle::SP_DialogCloseButton);
			//QIcon infoIcon = cStyle->standardIcon(QStyle::SP_MessageBoxInformation);

			QPushButton* pCloseButton = new QPushButton(closeIcon, "&Close");
			GKLog(trace, "allocated Close button")

			hBox->addWidget(pCloseButton);

			QObject::connect(pCloseButton, &QPushButton::clicked, this, &AboutDialog::closeDialog);
		}

		/* -- -- */


	}
	catch (const std::bad_alloc& e) {
		LOG(error) << "bad allocation : " << e.what();
		throw;
	}
}

void AboutDialog::closeDialog(void)
{
	this->close();
}

} // namespace GLogiK

