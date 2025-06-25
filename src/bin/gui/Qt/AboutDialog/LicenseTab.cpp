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

#include <QVBoxLayout>
#include <QPlainTextEdit>
#include <QFont>
#include <QString>
#include <QFile>
#include <QCryptographicHash>

#include "lib/utils/utils.hpp"

#include "LicenseTab.hpp"

namespace GLogiK
{

using namespace NSGKUtils;

void LicenseTab::buildTab(void)
{
	GK_LOG_FUNC

	try {
		QVBoxLayout* vBox = new QVBoxLayout(this);
		GKLog(trace, "allocated QVBoxLayout")

		this->setLayout(vBox);

		vBox->addWidget( this->getHLine() );

		QPlainTextEdit* licenseWidget = new QPlainTextEdit();
		GKLog(trace, "allocated QPlainTextEdit")

		licenseWidget->setObjectName("LicenseWidget");
		licenseWidget->setReadOnly(true);

		{
			QFont cFont = licenseWidget->font();
			cFont.setPointSize(10);
			licenseWidget->setFont(cFont);
		}

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

		vBox->addWidget( this->getHLine() );
	}
	catch (const std::bad_alloc& e) {
		LOG(error) << "bad allocation : " << e.what();
		throw;
	}
}

} // namespace GLogiK
