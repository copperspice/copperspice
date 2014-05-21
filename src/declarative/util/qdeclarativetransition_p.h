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

#ifndef QDECLARATIVETRANSITION_H
#define QDECLARATIVETRANSITION_H

#include "private/qdeclarativestate_p.h"

#include <qdeclarative.h>

#include <QtCore/qobject.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

class QDeclarativeAbstractAnimation;
class QDeclarativeTransitionPrivate;
class QDeclarativeTransitionManager;

class Q_DECLARATIVE_EXPORT QDeclarativeTransition : public QObject
{
    CS_OBJECT(QDeclarativeTransition)
    Q_DECLARE_PRIVATE(QDeclarativeTransition)

    CS_PROPERTY_READ(from, fromState)
    CS_PROPERTY_WRITE(from, setFromState)
    CS_PROPERTY_NOTIFY(from, fromChanged)
    CS_PROPERTY_READ(to, toState)
    CS_PROPERTY_WRITE(to, setToState)
    CS_PROPERTY_NOTIFY(to, toChanged)
    CS_PROPERTY_READ(reversible, reversible)
    CS_PROPERTY_WRITE(reversible, setReversible)
    CS_PROPERTY_NOTIFY(reversible, reversibleChanged)
    CS_PROPERTY_READ(animations, animations)
    CS_CLASSINFO("DefaultProperty", "animations")
    CS_CLASSINFO("DeferredPropertyNames", "animations")

public:
    QDeclarativeTransition(QObject *parent=0);
    ~QDeclarativeTransition();

    QString fromState() const;
    void setFromState(const QString &);

    QString toState() const;
    void setToState(const QString &);

    bool reversible() const;
    void setReversible(bool);

    QDeclarativeListProperty<QDeclarativeAbstractAnimation> animations();

    void prepare(QDeclarativeStateOperation::ActionList &actions,
                 QList<QDeclarativeProperty> &after,
                 QDeclarativeTransitionManager *end);

    void setReversed(bool r);
    void stop();

public:
    CS_SIGNAL_1(Public, void fromChanged())
    CS_SIGNAL_2(fromChanged) 
    CS_SIGNAL_1(Public, void toChanged())
    CS_SIGNAL_2(toChanged) 
    CS_SIGNAL_1(Public, void reversibleChanged())
    CS_SIGNAL_2(reversibleChanged) 
};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QDeclarativeTransition)

QT_END_HEADER

#endif // QDECLARATIVETRANSITION_H
