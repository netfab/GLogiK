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

#include <QImage>

#include "lib/utils/utils.hpp"

#include "icons.hpp"

namespace GLogiK
{

using namespace NSGKUtils;

AdaptiveIconEngine::AdaptiveIconEngine(
	QIcon baseIcon,
	const QPalette & palette)
	:	_baseIcon(std::move(baseIcon)),
		_palette(palette)
{
}

void AdaptiveIconEngine::paint(
	QPainter* painter,
	const QRect & rect,
	QIcon::Mode mode,
	QIcon::State state)
{
	GK_LOG_FUNC

	// Temporary image canvas to ensure that the background is transparent and alpha blending works.
	auto scale = painter->device()->devicePixelRatioF();
	QImage img(rect.size() * scale, QImage::Format_ARGB32_Premultiplied);
	img.fill(0);
	QPainter p(&img);

	_baseIcon.paint(&p, img.rect(), Qt::AlignCenter, mode, state);

	p.setCompositionMode(QPainter::CompositionMode_SourceIn);

	switch(mode)
	{
		case QIcon::Active:
			p.fillRect(img.rect(), _palette.color(QPalette::Active, QPalette::ButtonText));
			break;
		case QIcon::Selected:
			p.fillRect(img.rect(), _palette.color(QPalette::Active, QPalette::HighlightedText));
			break;
		case QIcon::Disabled:
			p.fillRect(img.rect(), _palette.color(QPalette::Disabled, QPalette::WindowText));
			break;
		default:
			if(mode != QIcon::Normal)
			{
				LOG(warning) << "unknown QIcon mode: " << mode;
			}
			p.fillRect(img.rect(), _palette.color(QPalette::Normal, QPalette::WindowText));
			break;
	}

	painter->drawImage(rect, img);
}

QPixmap AdaptiveIconEngine::pixmap(
	const QSize & size,
	QIcon::Mode mode,
	QIcon::State state)
{
	QImage img(size, QImage::Format_ARGB32_Premultiplied);
	img.fill(0);

	QPainter painter(&img);
	this->paint(&painter, QRect(0, 0, size.width(), size.height()), mode, state);

	return QPixmap::fromImage(img, Qt::ImageConversionFlag::NoFormatConversion);
}

QIconEngine* AdaptiveIconEngine::clone(void) const
{
	return new AdaptiveIconEngine(_baseIcon, {});
}

Icons::Icons(void)
{
	QIcon::setThemeSearchPaths(QStringList{":/glogik"} << QIcon::themeSearchPaths());
	QIcon::setThemeName("GLogiK");
}

Icons::~Icons(void)
{
}

QIcon Icons::icon(
	const QString & name,
	const QPalette & palette,
	bool recolor)
{
	QString cacheName = QString("%1:%2").arg(recolor ? "1" : "0", name);
	QIcon icon = _iconsCache.value(cacheName);

	if( ! icon.isNull() )
		return icon;

	icon = QIcon::fromTheme(name);
	if(recolor)
	{
		icon = QIcon( new AdaptiveIconEngine(icon, palette) );
	}

	_iconsCache.insert(cacheName, icon);

	return icon;
}

} // namespace GLogiK
