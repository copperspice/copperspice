/***********************************************************************
*
* Copyright (c) 2012-2014 Barbara Geller
* Copyright (c) 2012-2014 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or 
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see 
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QGRAPHICSANCHORLAYOUT_H
#define QGRAPHICSANCHORLAYOUT_H

#include <QtGui/qgraphicsitem.h>
#include <QtGui/qgraphicslayout.h>
#include <QScopedPointer>

QT_BEGIN_NAMESPACE

#if !defined(QT_NO_GRAPHICSVIEW) || (QT_EDITION & QT_MODULE_GRAPHICSVIEW) != QT_MODULE_GRAPHICSVIEW

class QGraphicsAnchorPrivate;
class QGraphicsAnchorLayout;
class QGraphicsAnchorLayoutPrivate;

class Q_GUI_EXPORT QGraphicsAnchor : public QObject
{
    CS_OBJECT(QGraphicsAnchor)

    GUI_CS_PROPERTY_READ(spacing, spacing)
    GUI_CS_PROPERTY_WRITE(spacing, setSpacing)
    GUI_CS_PROPERTY_RESET(spacing, unsetSpacing)

    GUI_CS_PROPERTY_READ(sizePolicy, sizePolicy)
    GUI_CS_PROPERTY_WRITE(sizePolicy, setSizePolicy)

public:
    void setSpacing(qreal spacing);
    void unsetSpacing();
    qreal spacing() const;

    void setSizePolicy(QSizePolicy::Policy policy);
    QSizePolicy::Policy sizePolicy() const;

    ~QGraphicsAnchor();

private:
    QGraphicsAnchor(QGraphicsAnchorLayout *parent);
    Q_DECLARE_PRIVATE(QGraphicsAnchor)

    friend class QGraphicsAnchorLayoutPrivate;
    friend struct AnchorData;

protected:
	 QScopedPointer<QGraphicsAnchorPrivate> d_ptr;

};

class Q_GUI_EXPORT QGraphicsAnchorLayout : public QGraphicsLayout
{
public:
    QGraphicsAnchorLayout(QGraphicsLayoutItem *parent = 0);
    virtual ~QGraphicsAnchorLayout();

    QGraphicsAnchor *addAnchor(QGraphicsLayoutItem *firstItem, Qt::AnchorPoint firstEdge,
                               QGraphicsLayoutItem *secondItem, Qt::AnchorPoint secondEdge);

    QGraphicsAnchor *anchor(QGraphicsLayoutItem *firstItem, Qt::AnchorPoint firstEdge,
                            QGraphicsLayoutItem *secondItem, Qt::AnchorPoint secondEdge);

    void addCornerAnchors(QGraphicsLayoutItem *firstItem, Qt::Corner firstCorner,
                          QGraphicsLayoutItem *secondItem, Qt::Corner secondCorner);

    void addAnchors(QGraphicsLayoutItem *firstItem,
                    QGraphicsLayoutItem *secondItem,
                    Qt::Orientations orientations = Qt::Horizontal | Qt::Vertical);

    void setHorizontalSpacing(qreal spacing);
    void setVerticalSpacing(qreal spacing);
    void setSpacing(qreal spacing);
    qreal horizontalSpacing() const;
    qreal verticalSpacing() const;

    void removeAt(int index);
    void setGeometry(const QRectF &rect);
    int count() const;
    QGraphicsLayoutItem *itemAt(int index) const;

    void invalidate();

protected:
    QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint = QSizeF()) const;

private:
    Q_DISABLE_COPY(QGraphicsAnchorLayout)
    Q_DECLARE_PRIVATE(QGraphicsAnchorLayout)

    friend class QGraphicsAnchor;
};

#endif

QT_END_NAMESPACE

#endif
