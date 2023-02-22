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

#include <QStyle>
#include <QPushButton>
#include <QString>
#include <QFont>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTabWidget>
#include <QFile>
#include <QPlainTextEdit>
#include <QCryptographicHash>

#include "lib/utils/utils.hpp"

#include "AboutDialog.hpp"

namespace GLogiK
{

using namespace NSGKUtils;

void FirstTab::buildTab(void)
{
	GK_LOG_FUNC

	try {
		QVBoxLayout* vBox = new QVBoxLayout(this);
		GKLog(trace, "allocated QVBoxLayout")

		this->setLayout(vBox);

		vBox->addSpacing(10);

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
			QString descText("Copyright 2016 - 2023 Fabrice Delliaux");

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
	}
	catch (const std::bad_alloc& e) {
		LOG(error) << "bad allocation : " << e.what();
		throw;
	}
}

void LicenseTab::buildTab(void)
{
	GK_LOG_FUNC

	try {
		QVBoxLayout* vBox = new QVBoxLayout(this);
		GKLog(trace, "allocated QVBoxLayout")

		this->setLayout(vBox);

		vBox->addSpacing(10);

		QPlainTextEdit* licenseWidget = new QPlainTextEdit();
		GKLog(trace, "allocated QPlainTextEdit")

		licenseWidget->setObjectName("LicenseWidget");
		licenseWidget->setReadOnly(true);

		QString licensePath(DOC_DIR); licensePath += "/COPYING";
		QFile licenseFile(licensePath);

		if(licenseFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
			auto get_hash = [] (QFile & file, QCryptographicHash::Algorithm algo)
				-> const std::string
			{
				QCryptographicHash hash(algo);
				QString hashString;
				if( hash.addData(&file) ) {
					QByteArray result = hash.result();
					hashString = result.toHex();
				}
				else {
					LOG(error) << "addData failure";
				}

				/* reset QFile before return */
				const std::string boolret( file.reset() ? "true" : "false" );
				GKLog2(trace, "file reset: ", boolret)

				return hashString.toStdString();
			};

			const std::string sha1( get_hash(licenseFile, QCryptographicHash::Sha1) );
			/* found license file */
			if( sha1 == LICENSE_FILE_SHA1 ) {
				QByteArray data(licenseFile.readAll());
				licenseWidget->setPlainText(QString(data));
			}
			else {
				GKLog2(trace, "license sha1: ", sha1)
				const std::string wrongError("error: license file sha1 does not match");
				licenseWidget->setPlainText(wrongError.c_str());
				LOG(error) << wrongError;
			}
		}
		else {
			const std::string openError("error: cannot open license file");
			licenseWidget->setPlainText(openError.c_str());
			LOG(error) << openError;
		}

		vBox->addWidget(licenseWidget);
	}
	catch (const std::bad_alloc& e) {
		LOG(error) << "bad allocation : " << e.what();
		throw;
	}
}

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

	try {
		QVBoxLayout* vBox = new QVBoxLayout(this);
		GKLog(trace, "allocated QVBoxLayout")

		this->setLayout(vBox);

		vBox->addSpacing(10);

		/* -- -- */

		QTabWidget* tabbedWidgets = new QTabWidget();
		GKLog(trace, "allocated QTabWidget")

		tabbedWidgets->setObjectName("AboutTabs");

		vBox->addWidget(tabbedWidgets);

		/* -- -- */
		FirstTab* firstTab = new FirstTab();
		tabbedWidgets->addTab(firstTab, tr("About"));
		firstTab->buildTab();

		/* -- -- */
		LicenseTab* licenseTab = new LicenseTab();
		tabbedWidgets->addTab(licenseTab, tr("License"));
		licenseTab->buildTab();

		/* -- -- */

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

