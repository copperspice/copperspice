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

#ifndef QDECLARATIVEANCHORS_H
#define QDECLARATIVEANCHORS_H

#include "qdeclarativeitem.h"

#include <qdeclarative.h>
#include <QtCore/QObject>
#include <private/qdeclarativeglobal_p.h>

QT_BEGIN_NAMESPACE

class QDeclarativeAnchorsPrivate;
class QDeclarativeAnchorLine;

class Q_DECLARATIVE_PRIVATE_EXPORT QDeclarativeAnchors : public QObject
{
    CS_OBJECT(QDeclarativeAnchors)

    CS_PROPERTY_READ(left, left)
    CS_PROPERTY_WRITE(left, setLeft)
    CS_PROPERTY_RESET(left, resetLeft)
    CS_PROPERTY_NOTIFY(left, leftChanged)
    CS_PROPERTY_READ(right, right)
    CS_PROPERTY_WRITE(right, setRight)
    CS_PROPERTY_RESET(right, resetRight)
    CS_PROPERTY_NOTIFY(right, rightChanged)
    CS_PROPERTY_READ(horizontalCenter, horizontalCenter)
    CS_PROPERTY_WRITE(horizontalCenter, setHorizontalCenter)
    CS_PROPERTY_RESET(horizontalCenter, resetHorizontalCenter)
    CS_PROPERTY_NOTIFY(horizontalCenter, horizontalCenterChanged)
    CS_PROPERTY_READ(top, top)
    CS_PROPERTY_WRITE(top, setTop)
    CS_PROPERTY_RESET(top, resetTop)
    CS_PROPERTY_NOTIFY(top, topChanged)
    CS_PROPERTY_READ(bottom, bottom)
    CS_PROPERTY_WRITE(bottom, setBottom)
    CS_PROPERTY_RESET(bottom, resetBottom)
    CS_PROPERTY_NOTIFY(bottom, bottomChanged)
    CS_PROPERTY_READ(verticalCenter, verticalCenter)
    CS_PROPERTY_WRITE(verticalCenter, setVerticalCenter)
    CS_PROPERTY_RESET(verticalCenter, resetVerticalCenter)
    CS_PROPERTY_NOTIFY(verticalCenter, verticalCenterChanged)
    CS_PROPERTY_READ(baseline, baseline)
    CS_PROPERTY_WRITE(baseline, setBaseline)
    CS_PROPERTY_RESET(baseline, resetBaseline)
    CS_PROPERTY_NOTIFY(baseline, baselineChanged)
    CS_PROPERTY_READ(margins, margins)
    CS_PROPERTY_WRITE(margins, setMargins)
    CS_PROPERTY_NOTIFY(margins, marginsChanged)
    CS_PROPERTY_READ(leftMargin, leftMargin)
    CS_PROPERTY_WRITE(leftMargin, setLeftMargin)
    CS_PROPERTY_NOTIFY(leftMargin, leftMarginChanged)
    CS_PROPERTY_READ(rightMargin, rightMargin)
    CS_PROPERTY_WRITE(rightMargin, setRightMargin)
    CS_PROPERTY_NOTIFY(rightMargin, rightMarginChanged)
    CS_PROPERTY_READ(horizontalCenterOffset, horizontalCenterOffset)
    CS_PROPERTY_WRITE(horizontalCenterOffset, setHorizontalCenterOffset)
    CS_PROPERTY_NOTIFY(horizontalCenterOffset, horizontalCenterOffsetChanged)
    CS_PROPERTY_READ(topMargin, topMargin)
    CS_PROPERTY_WRITE(topMargin, setTopMargin)
    CS_PROPERTY_NOTIFY(topMargin, topMarginChanged)
    CS_PROPERTY_READ(bottomMargin, bottomMargin)
    CS_PROPERTY_WRITE(bottomMargin, setBottomMargin)
    CS_PROPERTY_NOTIFY(bottomMargin, bottomMarginChanged)
    CS_PROPERTY_READ(verticalCenterOffset, verticalCenterOffset)
    CS_PROPERTY_WRITE(verticalCenterOffset, setVerticalCenterOffset)
    CS_PROPERTY_NOTIFY(verticalCenterOffset, verticalCenterOffsetChanged)
    CS_PROPERTY_READ(baselineOffset, baselineOffset)
    CS_PROPERTY_WRITE(baselineOffset, setBaselineOffset)
    CS_PROPERTY_NOTIFY(baselineOffset, baselineOffsetChanged)
    CS_PROPERTY_READ(*fill, fill)
    CS_PROPERTY_WRITE(*fill, setFill)
    CS_PROPERTY_RESET(*fill, resetFill)
    CS_PROPERTY_NOTIFY(*fill, fillChanged)
    CS_PROPERTY_READ(*centerIn, centerIn)
    CS_PROPERTY_WRITE(*centerIn, setCenterIn)
    CS_PROPERTY_RESET(*centerIn, resetCenterIn)
    CS_PROPERTY_NOTIFY(*centerIn, centerInChanged)

public:
    QDeclarativeAnchors(QObject *parent=0);
    QDeclarativeAnchors(QGraphicsObject *item, QObject *parent=0);
    virtual ~QDeclarativeAnchors();

    enum Anchor {
        LeftAnchor = 0x01,
        RightAnchor = 0x02,
        TopAnchor = 0x04,
        BottomAnchor = 0x08,
        HCenterAnchor = 0x10,
        VCenterAnchor = 0x20,
        BaselineAnchor = 0x40,
        Horizontal_Mask = LeftAnchor | RightAnchor | HCenterAnchor,
        Vertical_Mask = TopAnchor | BottomAnchor | VCenterAnchor | BaselineAnchor
    };
    using Anchors = QFlags<Anchor>;

    QDeclarativeAnchorLine left() const;
    void setLeft(const QDeclarativeAnchorLine &edge);
    void resetLeft();

    QDeclarativeAnchorLine right() const;
    void setRight(const QDeclarativeAnchorLine &edge);
    void resetRight();

    QDeclarativeAnchorLine horizontalCenter() const;
    void setHorizontalCenter(const QDeclarativeAnchorLine &edge);
    void resetHorizontalCenter();

    QDeclarativeAnchorLine top() const;
    void setTop(const QDeclarativeAnchorLine &edge);
    void resetTop();

    QDeclarativeAnchorLine bottom() const;
    void setBottom(const QDeclarativeAnchorLine &edge);
    void resetBottom();

    QDeclarativeAnchorLine verticalCenter() const;
    void setVerticalCenter(const QDeclarativeAnchorLine &edge);
    void resetVerticalCenter();

    QDeclarativeAnchorLine baseline() const;
    void setBaseline(const QDeclarativeAnchorLine &edge);
    void resetBaseline();

    qreal leftMargin() const;
    void setLeftMargin(qreal);

    qreal rightMargin() const;
    void setRightMargin(qreal);

    qreal horizontalCenterOffset() const;
    void setHorizontalCenterOffset(qreal);

    qreal topMargin() const;
    void setTopMargin(qreal);

    qreal bottomMargin() const;
    void setBottomMargin(qreal);

    qreal margins() const;
    void setMargins(qreal);

    qreal verticalCenterOffset() const;
    void setVerticalCenterOffset(qreal);

    qreal baselineOffset() const;
    void setBaselineOffset(qreal);

    QGraphicsObject *fill() const;
    void setFill(QGraphicsObject *);
    void resetFill();

    QGraphicsObject *centerIn() const;
    void setCenterIn(QGraphicsObject *);
    void resetCenterIn();

    Anchors usedAnchors() const;

    void classBegin();
    void componentComplete();

    bool mirrored();

public:
    CS_SIGNAL_1(Public, void leftChanged())
    CS_SIGNAL_2(leftChanged) 
    CS_SIGNAL_1(Public, void rightChanged())
    CS_SIGNAL_2(rightChanged) 
    CS_SIGNAL_1(Public, void topChanged())
    CS_SIGNAL_2(topChanged) 
    CS_SIGNAL_1(Public, void bottomChanged())
    CS_SIGNAL_2(bottomChanged) 
    CS_SIGNAL_1(Public, void verticalCenterChanged())
    CS_SIGNAL_2(verticalCenterChanged) 
    CS_SIGNAL_1(Public, void horizontalCenterChanged())
    CS_SIGNAL_2(horizontalCenterChanged) 
    CS_SIGNAL_1(Public, void baselineChanged())
    CS_SIGNAL_2(baselineChanged) 
    CS_SIGNAL_1(Public, void fillChanged())
    CS_SIGNAL_2(fillChanged) 
    CS_SIGNAL_1(Public, void centerInChanged())
    CS_SIGNAL_2(centerInChanged) 
    CS_SIGNAL_1(Public, void leftMarginChanged())
    CS_SIGNAL_2(leftMarginChanged) 
    CS_SIGNAL_1(Public, void rightMarginChanged())
    CS_SIGNAL_2(rightMarginChanged) 
    CS_SIGNAL_1(Public, void topMarginChanged())
    CS_SIGNAL_2(topMarginChanged) 
    CS_SIGNAL_1(Public, void bottomMarginChanged())
    CS_SIGNAL_2(bottomMarginChanged) 
    CS_SIGNAL_1(Public, void marginsChanged())
    CS_SIGNAL_2(marginsChanged) 
    CS_SIGNAL_1(Public, void verticalCenterOffsetChanged())
    CS_SIGNAL_2(verticalCenterOffsetChanged) 
    CS_SIGNAL_1(Public, void horizontalCenterOffsetChanged())
    CS_SIGNAL_2(horizontalCenterOffsetChanged) 
    CS_SIGNAL_1(Public, void baselineOffsetChanged())
    CS_SIGNAL_2(baselineOffsetChanged) 

private:
    friend class QDeclarativeItem;
    friend class QDeclarativeItemPrivate;
    friend class QDeclarativeGraphicsWidget;
    Q_DISABLE_COPY(QDeclarativeAnchors)
    Q_DECLARE_PRIVATE(QDeclarativeAnchors)

    CS_SLOT_1(Private, void _q_widgetGeometryChanged())
    CS_SLOT_2(_q_widgetGeometryChanged)

    CS_SLOT_1(Private, void _q_widgetDestroyed(QObject * obj))
    CS_SLOT_2(_q_widgetDestroyed)

};
Q_DECLARE_OPERATORS_FOR_FLAGS(QDeclarativeAnchors::Anchors)

QT_END_NAMESPACE

QML_DECLARE_TYPE(QDeclarativeAnchors)

#endif
