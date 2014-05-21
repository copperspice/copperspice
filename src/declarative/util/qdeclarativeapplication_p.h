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

#ifndef QDECLARATIVEAPPLICATION_P_H
#define QDECLARATIVEAPPLICATION_P_H

#include <QtCore/QObject>
#include <qdeclarative.h>
#include <private/qdeclarativeglobal_p.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

class QDeclarativeApplicationPrivate;
class Q_DECLARATIVE_PRIVATE_EXPORT QDeclarativeApplication : public QObject
{
    CS_OBJECT(QDeclarativeApplication)
    CS_PROPERTY_READ(active, active)
    CS_PROPERTY_NOTIFY(active, activeChanged)
    CS_PROPERTY_READ(layoutDirection, layoutDirection)
    CS_PROPERTY_NOTIFY(layoutDirection, layoutDirectionChanged)

public:
    explicit QDeclarativeApplication(QObject *parent = 0);
    virtual ~QDeclarativeApplication();
    bool active() const;
    Qt::LayoutDirection layoutDirection() const;

protected:
    bool eventFilter(QObject *obj, QEvent *event);

public:
    CS_SIGNAL_1(Public, void activeChanged())
    CS_SIGNAL_2(activeChanged) 
    CS_SIGNAL_1(Public, void layoutDirectionChanged())
    CS_SIGNAL_2(layoutDirectionChanged) 

private:
    Q_DISABLE_COPY(QDeclarativeApplication)
    Q_DECLARE_PRIVATE(QDeclarativeApplication)
};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QDeclarativeApplication)

QT_END_HEADER

#endif // QDECLARATIVEAPPLICATION_P_H
