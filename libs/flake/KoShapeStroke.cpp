/* This file is part of the KDE project
 *
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2006-2008 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2007,2009 Thorsten Zachmann <zachmann@kde.org>
 * Copyright (C) 2012      Inge Wallin <inge@lysator.liu.se>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

// Own
#include "KoShapeStroke.h"

// Posix
#include <math.h>

// Qt
#include <QPainterPath>
#include <QPainter>

// Calligra
#include <KoGenStyles.h>
#include <KoOdfGraphicStyles.h>

// Flake
#include "KoViewConverter.h"
#include "KoShape.h"
#include "KoShapeSavingContext.h"
#include "KoPathShape.h"
#include "KoMarker.h"
#include "KoInsets.h"
#include <KoPathSegment.h>
#include <KoPathPoint.h>
#include <cmath>

#include "kis_global.h"

class Q_DECL_HIDDEN KoShapeStroke::Private
{
public:
    Private(KoShapeStroke *_q) : q(_q) {}
    KoShapeStroke *q;

    void paintBorder(KoShape *shape, QPainter &painter, const QPen &pen) const;
    QColor color;
    QPen pen;
    QBrush brush;
};

namespace {
QPair<qreal, qreal> anglesForSegment(KoPathSegment segment) {
    const qreal eps = 1e-6;

    if (segment.degree() < 3) {
        segment = segment.toCubic();
    }

    QList<QPointF> points = segment.controlPoints();
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(points.size() == 4, qMakePair(0.0, 0.0));
    QPointF vec1 = points[1] - points[0];
    QPointF vec2 = points[3] - points[2];

    if (vec1.manhattanLength() < eps) {
        points[1] = segment.pointAt(eps);
        vec1 = points[1] - points[0];
    }

    if (vec2.manhattanLength() < eps) {
        points[2] = segment.pointAt(1.0 - eps);
        vec2 = points[3] - points[2];
    }

    const qreal angle1 = std::atan2(vec1.y(), vec1.x());
    const qreal angle2 = std::atan2(vec2.y(), vec2.x());
    return qMakePair(angle1, angle2);
}
}

void KoShapeStroke::Private::paintBorder(KoShape *shape, QPainter &painter, const QPen &pen) const
{
    if (!pen.isCosmetic() && pen.style() != Qt::NoPen) {
        KoPathShape *pathShape = dynamic_cast<KoPathShape *>(shape);
        if (pathShape) {
            QPainterPath path = pathShape->pathStroke(pen);
            painter.fillPath(path, pen.brush());

            if (!pathShape->hasMarkers()) return;

            const bool autoFillMarkers = pathShape->autoFillMarkers();
            KoMarker *startMarker = pathShape->marker(KoFlake::StartMarker);
            KoMarker *midMarker = pathShape->marker(KoFlake::MidMarker);
            KoMarker *endMarker = pathShape->marker(KoFlake::EndMarker);

            for (int i = 0; i < pathShape->subpathCount(); i++) {
                const int numSubPoints = pathShape->subpathPointCount(i);
                if (numSubPoints < 2) continue;

                const bool isClosedSubpath = pathShape->isClosedSubpath(i);

                qreal firstAngle = 0.0;
                {
                    KoPathSegment segment = pathShape->segmentByIndex(KoPathPointIndex(i, 0));
                    firstAngle= anglesForSegment(segment).first;
                }

                const int numSegments = isClosedSubpath ? numSubPoints : numSubPoints - 1;

                qreal lastAngle = 0.0;
                {
                    KoPathSegment segment = pathShape->segmentByIndex(KoPathPointIndex(i, numSegments - 1));
                    lastAngle = anglesForSegment(segment).second;
                }

                qreal previousAngle = 0.0;

                for (int j = 0; j < numSegments; j++) {
                    KoPathSegment segment = pathShape->segmentByIndex(KoPathPointIndex(i, j));
                    QPair<qreal, qreal> angles = anglesForSegment(segment);

                    const qreal angle1 = angles.first;
                    const qreal angle2 = angles.second;

                    if (j == 0 && startMarker) {
                        const qreal angle = isClosedSubpath ? bisectorAngle(firstAngle, lastAngle) : firstAngle;
                        if (autoFillMarkers) {
                            startMarker->applyShapeStroke(shape, q, segment.first()->point(), pen.widthF(), angle);
                        }
                        startMarker->paintAtPosition(&painter, segment.first()->point(), pen.widthF(), angle);
                    }

                    if (j > 0 && midMarker) {
                        const qreal angle = bisectorAngle(previousAngle, angle1);
                        if (autoFillMarkers) {
                            midMarker->applyShapeStroke(shape, q, segment.first()->point(), pen.widthF(), angle);
                        }
                        midMarker->paintAtPosition(&painter, segment.first()->point(), pen.widthF(), angle);
                    }

                    if (j == numSegments - 1 && endMarker) {
                        const qreal angle = isClosedSubpath ? bisectorAngle(firstAngle, lastAngle) : lastAngle;
                        if (autoFillMarkers) {
                            endMarker->applyShapeStroke(shape, q, segment.second()->point(), pen.widthF(), angle);
                        }
                        endMarker->paintAtPosition(&painter, segment.second()->point(), pen.widthF(), angle);
                    }

                    previousAngle = angle2;
                }
            }

            return;
        }

        painter.strokePath(shape->outline(), pen);
    }
}


KoShapeStroke::KoShapeStroke()
        : d(new Private(this))
{
    d->color = QColor(Qt::black);
    // we are not rendering stroke with zero width anymore
    // so lets use a default width of 1.0
    d->pen.setWidthF(1.0);
}

KoShapeStroke::KoShapeStroke(const KoShapeStroke &other)
        : KoShapeStrokeModel(), d(new Private(this))
{
    d->color = other.d->color;
    d->pen = other.d->pen;
    d->brush = other.d->brush;
}

KoShapeStroke::KoShapeStroke(qreal lineWidth, const QColor &color)
        : d(new Private(this))
{
    d->pen.setWidthF(qMax(qreal(0.0), lineWidth));
    d->pen.setJoinStyle(Qt::MiterJoin);
    d->color = color;
}

KoShapeStroke::~KoShapeStroke()
{
    delete d;
}

KoShapeStroke &KoShapeStroke::operator = (const KoShapeStroke &rhs)
{
    if (this == &rhs)
        return *this;

    d->pen = rhs.d->pen;
    d->color = rhs.d->color;
    d->brush = rhs.d->brush;

    return *this;
}

void KoShapeStroke::fillStyle(KoGenStyle &style, KoShapeSavingContext &context) const
{
    QPen pen = d->pen;
    if (d->brush.gradient())
        pen.setBrush(d->brush);
    else
        pen.setColor(d->color);
    KoOdfGraphicStyles::saveOdfStrokeStyle(style, context.mainStyles(), pen);
}

void KoShapeStroke::strokeInsets(const KoShape *shape, KoInsets &insets) const
{
    Q_UNUSED(shape);
    qreal lineWidth = d->pen.widthF();
    if (lineWidth < 0) {
        lineWidth = 1;
    }
    lineWidth *= 0.5; // since we draw a line half inside, and half outside the object.

    // if we have square cap, we need a little more space
    // -> sqrt((0.5*penWidth)^2 + (0.5*penWidth)^2)
    if (capStyle() == Qt::SquareCap) {
        lineWidth *= M_SQRT2;
    }

    if (joinStyle() == Qt::MiterJoin) {
        lineWidth = qMax(lineWidth, miterLimit());
    }

    insets.top = lineWidth;
    insets.bottom = lineWidth;
    insets.left = lineWidth;
    insets.right = lineWidth;
}

qreal KoShapeStroke::strokeMaxMarkersInset(const KoShape *shape) const
{
    qreal result = 0.0;

    const KoPathShape *pathShape = dynamic_cast<const KoPathShape *>(shape);
    if (pathShape && pathShape->hasMarkers()) {
        const qreal lineWidth = d->pen.widthF();

        QVector<const KoMarker*> markers;
        markers << pathShape->marker(KoFlake::StartMarker);
        markers << pathShape->marker(KoFlake::MidMarker);
        markers << pathShape->marker(KoFlake::EndMarker);

        Q_FOREACH (const KoMarker *marker, markers) {
            if (marker) {
                result = qMax(result, marker->maxInset(lineWidth));
            }
        }
    }

    return result;
}

bool KoShapeStroke::hasTransparency() const
{
    return d->color.alpha() > 0;
}

void KoShapeStroke::paint(KoShape *shape, QPainter &painter, const KoViewConverter &converter)
{
    KoShape::applyConversion(painter, converter);

    QPen pen = d->pen;

    if (d->brush.gradient())
        pen.setBrush(d->brush);
    else
        pen.setColor(d->color);

    d->paintBorder(shape, painter, pen);
}

bool KoShapeStroke::compareFillTo(const KoShapeStrokeModel *other)
{
    if (!other) return false;

    const KoShapeStroke *stroke = dynamic_cast<const KoShapeStroke*>(other);
    if (!stroke) return false;

    return (d->brush.gradient() && d->brush == stroke->d->brush) ||
            (!d->brush.gradient() && d->color == stroke->d->color);
}

bool KoShapeStroke::compareStyleTo(const KoShapeStrokeModel *other)
{
    if (!other) return false;

    const KoShapeStroke *stroke = dynamic_cast<const KoShapeStroke*>(other);
    if (!stroke) return false;

    QPen pen1 = d->pen;
    QPen pen2 = stroke->d->pen;

    // just a random color top avoid comparison of that property
    pen1.setColor(Qt::magenta);
    pen2.setColor(Qt::magenta);

    return pen1 == pen2;
}

bool KoShapeStroke::isVisible() const
{
    return d->pen.widthF() > 0 &&
        (d->brush.gradient() || d->color.alpha() > 0);
}

void KoShapeStroke::setCapStyle(Qt::PenCapStyle style)
{
    d->pen.setCapStyle(style);
}

Qt::PenCapStyle KoShapeStroke::capStyle() const
{
    return d->pen.capStyle();
}

void KoShapeStroke::setJoinStyle(Qt::PenJoinStyle style)
{
    d->pen.setJoinStyle(style);
}

Qt::PenJoinStyle KoShapeStroke::joinStyle() const
{
    return d->pen.joinStyle();
}

void KoShapeStroke::setLineWidth(qreal lineWidth)
{
    d->pen.setWidthF(qMax(qreal(0.0), lineWidth));
}

qreal KoShapeStroke::lineWidth() const
{
    return d->pen.widthF();
}

void KoShapeStroke::setMiterLimit(qreal miterLimit)
{
    d->pen.setMiterLimit(miterLimit);
}

qreal KoShapeStroke::miterLimit() const
{
    return d->pen.miterLimit();
}

QColor KoShapeStroke::color() const
{
    return d->color;
}

void KoShapeStroke::setColor(const QColor &color)
{
    d->color = color;
}

void KoShapeStroke::setLineStyle(Qt::PenStyle style, const QVector<qreal> &dashes)
{
    if (style < Qt::CustomDashLine) {
        d->pen.setStyle(style);
    } else {
        d->pen.setDashPattern(dashes);
    }
}

Qt::PenStyle KoShapeStroke::lineStyle() const
{
    return d->pen.style();
}

QVector<qreal> KoShapeStroke::lineDashes() const
{
    return d->pen.dashPattern();
}

void KoShapeStroke::setDashOffset(qreal dashOffset)
{
    d->pen.setDashOffset(dashOffset);
}

qreal KoShapeStroke::dashOffset() const
{
    return d->pen.dashOffset();
}

void KoShapeStroke::setLineBrush(const QBrush &brush)
{
    d->brush = brush;
}

QBrush KoShapeStroke::lineBrush() const
{
    return d->brush;
}
