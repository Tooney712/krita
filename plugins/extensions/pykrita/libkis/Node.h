/*
 *  Copyright (c) 2016 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef LIBKIS_NODE_H
#define LIBKIS_NODE_H

#include <QObject>

#include <kis_types.h>

#include "kritalibkis_export.h"
#include "libkis.h"

/**
 * Node
 */
class KRITALIBKIS_EXPORT Node : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY(Node)

public:
    explicit Node(KisNodeSP node, QObject *parent = 0);
    virtual ~Node();

    bool alphaLocked() const;
    void setAlphaLocked(bool value);

    QString blendingMode() const;
    void setBlendingMode(QString value);

    QList<Channel*> channels() const;
    void setChannels(QList<Channel*> value);

    QList<Node*> childNodes() const;
    void setChildNodes(QList<Node*> value);

    ColorDepth* colorDepth() const;
    void setColorDepth(ColorDepth* value);

    QString colorLabel() const;
    void setColorLabel(QString value);

    ColorModel* colorModel() const;
    void setColorModel(ColorModel* value);

    ColorProfile* colorProfile() const;
    void setColorProfile(ColorProfile* value);

    bool inheritAlpha() const;
    void setInheritAlpha(bool value);

    bool locked() const;
    void setLocked(bool value);

    QString name() const;
    void setName(QString value);

    int opacity() const;
    void setOpacity(int value);

    Node* parentNode() const;
    void setParentNode(Node* value);

    QString type() const;
    void setType(QString value);

    bool visible() const;
    void setVisible(bool value);

    InfoObject* metaDataInfo() const;
    void setMetaDataInfo(InfoObject* value);

    Generator* generator() const;
    void setGenerator(Generator* value);

    Filter* filter() const;
    void setFilter(Filter* value);

    Transformation* transformation() const;
    void setTransformation(Transformation* value);

    Selection* selection() const;
    void setSelection(Selection* value);

    QString fileName() const;
    void setFileName(QString value);

    QByteArray pixelData() const;
    void setPixelData(QByteArray value);


public Q_SLOTS:

    void move(int x, int y);

    void moveToParent(Node *parent);

    void remove();

    Node* duplicate();

    bool save(const QString &filename);

Q_SIGNALS:



private:
    struct Private;
    Private *d;

};

#endif // LIBKIS_NODE_H
