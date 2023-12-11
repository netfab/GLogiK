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

#include <QVBoxLayout>
#include <QFrame>
#include <QStringList>
#include <QScrollArea>
#include <QHeaderView>
#include <QTableWidget>
#include <QTableWidgetItem>

#include "lib/utils/utils.hpp"

#include "DependenciesTab.hpp"

namespace GLogiK
{

using namespace NSGKUtils;

DependenciesTab::DependenciesTab(const GKDepsMap_type* const pDepsMap)
	:	_pDepsMap(pDepsMap)
{
}

void DependenciesTab::buildTab(void)
{
	GK_LOG_FUNC

	try {
		QVBoxLayout* vBox = new QVBoxLayout(this);
		GKLog(trace, "allocated QVBoxLayout")

		this->setLayout(vBox);

		vBox->addSpacing(10);

		/* -- -- -- */
		QFrame* mainFrame = new QFrame();
		vBox->addWidget(mainFrame);

		QVBoxLayout* scrollLayout = new QVBoxLayout();
		mainFrame->setLayout(scrollLayout);

		{
			QScrollArea* scrollArea = new QScrollArea();
			scrollLayout->addWidget(scrollArea);

			scrollArea->setWidgetResizable(true);
			scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
			scrollLayout->setContentsMargins(0, 0, 0, 0);

			QTableWidget* depsTable = new QTableWidget(0, 4);

			{
				const QStringList header = { "", "Built Against", "Run With", "" };
				depsTable->setHorizontalHeaderLabels(header);
				depsTable->horizontalHeader()->setStretchLastSection(true);
				depsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
				depsTable->horizontalHeader()->setSectionsClickable(false);
				depsTable->verticalHeader()->setVisible(false);
				depsTable->setShowGrid(false);
				depsTable->setSelectionMode(QAbstractItemView::NoSelection);
				depsTable->setFocusPolicy(Qt::NoFocus);
			}

			int row = 0;
			for(const auto & depPair : (*_pDepsMap))
			{
				//const auto & GKBin = depPair.first;
				const auto & GKDeps = depPair.second;

				/* increasing rows */
				depsTable->setRowCount( (depsTable->rowCount() + GKDeps.size()) );

				for(const auto & GKDep : GKDeps)
				{
					QTableWidgetItem* item = nullptr;

					const int alignment = Qt::AlignVCenter | Qt::AlignHCenter;
					const Qt::ItemFlags flags = Qt::NoItemFlags | Qt::ItemIsEnabled;

					item = new QTableWidgetItem(GKDep.getDependency().c_str());
					item->setTextAlignment(alignment);
					item->setFlags(flags);
					depsTable->setItem(row, 0, item);

					item = new QTableWidgetItem(GKDep.getCompileTimeVersion().c_str());
					item->setTextAlignment(alignment);
					item->setFlags(flags);
					depsTable->setItem(row, 1, item);

					item = new QTableWidgetItem(GKDep.getRunTimeVersion().c_str());
					item->setTextAlignment(alignment);
					item->setFlags(flags);
					depsTable->setItem(row, 2, item);

					++row;
				}
			}

			depsTable->resizeColumnsToContents();

			scrollArea->setWidget(depsTable);
		}
	}
	catch (const std::bad_alloc& e) {
		LOG(error) << "bad allocation : " << e.what();
		throw;
	}
}

} // namespace GLogiK
