/*
 *  Copyright (c) 2002 Patrick Julien  <freak@codepimps.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#include <limits.h>

#include <qimage.h>
#include <qpainter.h>
#include <qpixmap.h>

#include <kdebug.h>

#include "kis_image.h"
#include "kis_strategy_colorspace_rgb.h"
#include "tiles/kispixeldata.h"

namespace {
	const Q_INT32 MAX_CHANNEL_RGB = 3;
	const Q_INT32 MAX_CHANNEL_RGBA = 4;
}

KisStrategyColorSpaceRGB::KisStrategyColorSpaceRGB() : 	m_pixmap(RENDER_WIDTH * 2, RENDER_HEIGHT * 2)
{
	m_buf = new QUANTUM[RENDER_WIDTH * RENDER_HEIGHT * MAX_CHANNEL_RGBA];
}

KisStrategyColorSpaceRGB::~KisStrategyColorSpaceRGB()
{
	delete[] m_buf;
}

void KisStrategyColorSpaceRGB::nativeColor(const KoColor& c, QUANTUM *dst)
{
	dst[PIXEL_RED] = upscale(c.R());
	dst[PIXEL_GREEN] = upscale(c.G());
	dst[PIXEL_BLUE] = upscale(c.B());
}

void KisStrategyColorSpaceRGB::nativeColor(const KoColor& c, QUANTUM opacity, QUANTUM *dst)
{
	dst[PIXEL_RED] = upscale(c.R());
	dst[PIXEL_GREEN] = upscale(c.G());
	dst[PIXEL_BLUE] = upscale(c.B());
	dst[PIXEL_ALPHA] = opacity;
}

void KisStrategyColorSpaceRGB::nativeColor(const QColor& c, QUANTUM *dst)
{
	dst[PIXEL_RED] = upscale(c.red());
	dst[PIXEL_GREEN] = upscale(c.green());
	dst[PIXEL_BLUE] = upscale(c.blue());
}

void KisStrategyColorSpaceRGB::nativeColor(const QColor& c, QUANTUM opacity, QUANTUM *dst)
{
	dst[PIXEL_RED] = upscale(c.red());
	dst[PIXEL_GREEN] = upscale(c.green());
	dst[PIXEL_BLUE] = upscale(c.blue());
	dst[PIXEL_ALPHA] = opacity;
}

void KisStrategyColorSpaceRGB::nativeColor(QRgb rgb, QUANTUM *dst)
{
	dst[PIXEL_RED] = qRed(rgb);
	dst[PIXEL_GREEN] = qGreen(rgb);
	dst[PIXEL_BLUE] = qBlue(rgb);
}

void KisStrategyColorSpaceRGB::nativeColor(QRgb rgb, QUANTUM opacity, QUANTUM *dst)
{
	dst[PIXEL_RED] = qRed(rgb);
	dst[PIXEL_GREEN] = qGreen(rgb);
	dst[PIXEL_BLUE] = qBlue(rgb);
	dst[PIXEL_ALPHA] = opacity;
}

void KisStrategyColorSpaceRGB::composite(QUANTUM *dst, QUANTUM *src, Q_INT32 opacity, CompositeOp op) const
{
	QUANTUM alpha;
	QUANTUM invAlpha;
	QUANTUM *d;
	QUANTUM *s;

	d = dst;
	s = src;

	switch (op) {
	case COMPOSITE_CLEAR:
		if ( opacity == OPACITY_OPAQUE ) {
			memset(d, 0, MAX_CHANNEL_RGBA);
		}
		break;
	case COMPOSITE_COPY:
		if ( opacity == OPACITY_OPAQUE ) {
			memcpy(d, s, MAX_CHANNEL_RGBA);
		}
		break;
	case COMPOSITE_OVER:
		if (s[PIXEL_ALPHA] == OPACITY_TRANSPARENT)
			break;

		if ( opacity == OPACITY_OPAQUE ) {
			if (s[PIXEL_ALPHA] == OPACITY_TRANSPARENT ||
			    (d[PIXEL_ALPHA] == OPACITY_OPAQUE && s[PIXEL_ALPHA] == OPACITY_OPAQUE))
			{
				memcpy(d, s, MAX_CHANNEL_RGBA * sizeof(QUANTUM));
				break;
			}
		}
		alpha = (s[PIXEL_ALPHA] * opacity) / QUANTUM_MAX;
		invAlpha = QUANTUM_MAX - alpha;

		d[PIXEL_RED] = (d[PIXEL_RED] * invAlpha + s[PIXEL_RED] * alpha) / QUANTUM_MAX;
		d[PIXEL_GREEN] = (d[PIXEL_GREEN] * invAlpha + s[PIXEL_GREEN] * alpha) / QUANTUM_MAX;
		d[PIXEL_BLUE] = (d[PIXEL_BLUE] * invAlpha + s[PIXEL_BLUE] * alpha) / QUANTUM_MAX;
		alpha = (d[PIXEL_ALPHA] * (QUANTUM_MAX - s[PIXEL_ALPHA]) + s[PIXEL_ALPHA]) / QUANTUM_MAX;
		d[PIXEL_ALPHA] = (d[PIXEL_ALPHA] * (QUANTUM_MAX - alpha) + s[PIXEL_ALPHA]) / QUANTUM_MAX;

		break;
	case COMPOSITE_IN:
	case COMPOSITE_ATOP:
	case COMPOSITE_XOR:
	case COMPOSITE_PLUS:
	case COMPOSITE_MINUS:
	case COMPOSITE_ADD:
	case COMPOSITE_SUBTRACT:
	case COMPOSITE_DIFF:
	case COMPOSITE_MULT:
	case COMPOSITE_BUMPMAP:
	case COMPOSITE_COPY_RED:
	case COMPOSITE_COPY_GREEN:
	case COMPOSITE_COPY_BLUE:
	case COMPOSITE_COPY_OPACITY:
	case COMPOSITE_DISSOLVE:
	case COMPOSITE_DISPLACE:
	case COMPOSITE_MODULATE:
	case COMPOSITE_THRESHOLD:
	default:
		kdDebug() << "Not Implemented.\n";
		return;
	}
}

void KisStrategyColorSpaceRGB::render(KisImageSP projection, QPainter& painter, Q_INT32 x, Q_INT32 y, Q_INT32 width, Q_INT32 height)
{
	if (projection) {
		KisTileMgrSP tm = projection -> tiles();
		KisPixelDataSP pd = new KisPixelData;
		QImage img;

		pd -> mgr = 0;
		pd -> tile = 0;
		pd -> mode = TILEMODE_READ;
		pd -> x1 = x;
		pd -> x2 = x + width - 1;
		pd -> y1 = y;
		pd -> y2 = y + height - 1;
		pd -> width = pd -> x2 - pd -> x1 + 1;
		pd -> height = pd -> y2 - pd -> y1 + 1;
		pd -> depth = projection -> depth();
		pd -> stride = pd -> depth * pd -> width;
		pd -> owner = false;
		pd -> data = m_buf;
		tm -> readPixelData(pd);


		if (QImage::systemByteOrder() == QImage::LittleEndian) {
			img = QImage(pd -> data, pd -> width, pd -> height, pd -> depth * CHAR_BIT, 0, 0, QImage::LittleEndian);
		}
		else {
			img = QImage(pd->width,  pd->height, 32, 0, QImage::LittleEndian);
			Q_INT32 i = 0;

			uchar *j = img.bits();
			QString s;
			while ( i < pd ->stride * pd -> height ) {

				// Swap the bytes
				*( j + PIXEL_ALPHA ) = *( pd->data + i + PIXEL_BLUE );
				*( j + PIXEL_RED )   = *( pd->data + i + PIXEL_GREEN );
				*( j + PIXEL_GREEN ) = *( pd->data + i + PIXEL_RED );
				*( j + PIXEL_BLUE )  = *( pd->data + i + PIXEL_ALPHA );

				i += MAX_CHANNEL_RGBA;
				j += MAX_CHANNEL_RGBA; // Because we're hard-coded 32 bits deep, 4 bytes

			}
		}

		m_pixio.putImage(&m_pixmap, 0, 0, &img);
		painter.drawPixmap(x, y, m_pixmap, 0, 0, width, height);
	}
}


