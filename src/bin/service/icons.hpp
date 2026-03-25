/*
 *
 *	This file is part of GLogiK project.
 *	GLogiK, daemon to handle special features on gaming keyboards
 *	Copyright (C) 2016-2026  Fabrice Delliaux <netbox253@gmail.com>
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

#include <QHash>
#include <QIconEngine>
#include <QIcon>
#include <QPainter>
#include <QPalette>
#include <QPixmap>
#include <QRect>
#include <QSize>
#include <QString>

#ifndef SRC_BIN_SERVICE_ICONS_HPP_
#define SRC_BIN_SERVICE_ICONS_HPP_

namespace GLogiK
{

class AdaptiveIconEngine
	:	public QIconEngine
{
	public:
		explicit AdaptiveIconEngine(
			QIcon baseIcon,
			const QPalette & palette
		);

		void paint(
			QPainter* painter,
			const QRect & rect,
			QIcon::Mode mode,
			QIcon::State state
		) override;

		QPixmap pixmap(
			const QSize & size,
			QIcon::Mode mode,
			QIcon::State state
		) override;

		QIconEngine* clone(void) const override;

	private:
		QIcon _baseIcon;
		QPalette _palette;
};

class Icons
{
	public:
		Icons(void);
		~Icons(void);

		QIcon icon(
			const QString & name,
			const QPalette & palette,
			bool recolor = true
		);

	protected:
	private:
		QHash<QString, QIcon> _iconsCache;
};

} // namespace GLogiK

#endif
