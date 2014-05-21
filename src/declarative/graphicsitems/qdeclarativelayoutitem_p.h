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

#ifndef QDECLARATIVEGRAPHICSLAYOUTITEM_H
#define QDECLARATIVEGRAPHICSLAYOUTITEM_H
#include "qdeclarativeitem.h"

#include <QGraphicsLayoutItem>
#include <QSizeF>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

class QDeclarativeLayoutItem : public QDeclarativeItem, public QGraphicsLayoutItem
{
    CS_OBJECT(QDeclarativeLayoutItem)

    CS_INTERFACES(QGraphicsLayoutItem)
    CS_PROPERTY_READ(maximumSize, maximumSize)
    CS_PROPERTY_WRITE(maximumSize, setMaximumSize)
    CS_PROPERTY_NOTIFY(maximumSize, maximumSizeChanged)
    CS_PROPERTY_READ(minimumSize, minimumSize)
    CS_PROPERTY_WRITE(minimumSize, setMinimumSize)
    CS_PROPERTY_NOTIFY(minimumSize, minimumSizeChanged)
    CS_PROPERTY_READ(preferredSize, preferredSize)
    CS_PROPERTY_WRITE(preferredSize, setPreferredSize)
    CS_PROPERTY_NOTIFY(preferredSize, preferredSizeChanged)

public:
    QDeclarativeLayoutItem(QDeclarativeItem* parent=0);

    QSizeF maximumSize() const { return m_maximumSize; }
    void setMaximumSize(const QSizeF &s) { if(s==m_maximumSize) return; m_maximumSize = s; emit maximumSizeChanged(); }

    QSizeF minimumSize() const { return m_minimumSize; }
    void setMinimumSize(const QSizeF &s) { if(s==m_minimumSize) return; m_minimumSize = s; emit minimumSizeChanged(); }

    QSizeF preferredSize() const { return m_preferredSize; }
    void setPreferredSize(const QSizeF &s) { if(s==m_preferredSize) return; m_preferredSize = s; emit preferredSizeChanged(); }

    virtual void setGeometry(const QRectF & rect);
protected:
    virtual QSizeF sizeHint(Qt::SizeHint which, const QSizeF &constraint = QSizeF()) const;

public:
    CS_SIGNAL_1(Public, void maximumSizeChanged())
    CS_SIGNAL_2(maximumSizeChanged) 
    CS_SIGNAL_1(Public, void minimumSizeChanged())
    CS_SIGNAL_2(minimumSizeChanged) 
    CS_SIGNAL_1(Public, void preferredSizeChanged())
    CS_SIGNAL_2(preferredSizeChanged) 

private:
    QSizeF m_maximumSize;
    QSizeF m_minimumSize;
    QSizeF m_preferredSize;
};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QDeclarativeLayoutItem)

QT_END_HEADER
#endif
