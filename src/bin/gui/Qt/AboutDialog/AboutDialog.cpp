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
#include <QHBoxLayout>
#include <QTabWidget>
#include <QClipboard>
#include <QGuiApplication>

#include "lib/utils/utils.hpp"

#include "AboutTab.hpp"
#include "LicenseTab.hpp"
#include "DependenciesTab.hpp"

#include "AboutDialog.hpp"

namespace GLogiK
{

using namespace NSGKUtils;

AboutDialog::AboutDialog(QWidget* parent)
	:	QDialog(parent),
		_pCopyButton(nullptr)
{
}

AboutDialog::~AboutDialog()
{
	GK_LOG_FUNC

	GKLog(trace, "deleting AboutDialog")
}

void AboutDialog::buildDialog(const GKDepsMap_type* const pDepsMap)
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

		{
			/* -- -- */
			AboutTab* aboutTab = new AboutTab();
			tabbedWidgets->addTab(aboutTab, tr("About"));
			aboutTab->buildTab();

			/* -- -- */
			LicenseTab* licenseTab = new LicenseTab();
			tabbedWidgets->addTab(licenseTab, tr("License"));
			licenseTab->buildTab();

			/* -- -- */
			DependenciesTab* dependenciesTab = new DependenciesTab(pDepsMap);
			tabbedWidgets->addTab(dependenciesTab, tr("Dependencies"));
			dependenciesTab->buildTab();
		}

		/* -- -- */

		vBox->addStretch();

		/* -- -- */

		QHBoxLayout* hBox = new QHBoxLayout();
		GKLog(trace, "allocated QHBoxLayout")

		vBox->addLayout(hBox);

		hBox->addStretch();

		{
			QIcon copyIcon = QIcon::fromTheme("edit-copy");
			_pCopyButton = new QPushButton(copyIcon, "&Copy");
			GKLog(trace, "allocated Copy button")
			_pCopyButton->setVisible(false); /* not visible by default */
			hBox->addWidget(_pCopyButton);

			QStyle* cStyle = this->style();
			QIcon closeIcon = cStyle->standardIcon(QStyle::SP_DialogCloseButton);
			QPushButton* pCloseButton = new QPushButton(closeIcon, "&Close");
			GKLog(trace, "allocated Close button")
			hBox->addWidget(pCloseButton);

			QObject::connect(pCloseButton, &QPushButton::clicked, this, &AboutDialog::closeDialog);
		}

		/* -- -- */

		/* connect event to toggle copy button visibility */
		QObject::connect(tabbedWidgets, &QTabWidget::currentChanged, this, &AboutDialog::setCopyButtonVisibility);
		QObject::connect(_pCopyButton, &QPushButton::clicked, std::bind(&AboutDialog::copyToClipboard, this, pDepsMap));
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

void AboutDialog::setCopyButtonVisibility(int index)
{
	GK_LOG_FUNC

	GKLog2(trace, "tab index: ", index)
	/* copy button only visible on dependencies tab */
	const bool visibility = (index == 2) ? true : false;
	_pCopyButton->setVisible(visibility);
}

void AboutDialog::copyToClipboard(const GKDepsMap_type* const pDepsMap)
{
	GK_LOG_FUNC

	QClipboard* clipboard = QGuiApplication::clipboard();

	const int pad = 8;
	QString text = QString("%1 %2 %3\n").arg("", 10).arg("built", pad).arg("run", pad);

	for(const auto & depPair : (*pDepsMap))
	{
		//const auto & GKBin = depPair.first;
		const auto & GKDeps = depPair.second;
			for(const auto & GKDep : GKDeps)
			{
				const char* d  = GKDep.getDependency().c_str();
				const char* ct = GKDep.getCompileTimeVersion().c_str();
				const char* rt = GKDep.getRunTimeVersion().c_str();

				text += QString("%1 %2 %3\n").arg(d, 10).arg(ct, pad).arg(rt, pad);
			}
	}

	clipboard->setText(text);
}

} // namespace GLogiK

