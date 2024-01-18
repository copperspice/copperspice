/***********************************************************************
*
* Copyright (c) 2012-2024 Barbara Geller
* Copyright (c) 2012-2024 Ansel Sermersheim
*
* Copyright (c) 2015 The Qt Company Ltd.
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
*
* This file is part of CopperSpice.
*
* CopperSpice is free software. You can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*
* https://www.gnu.org/licenses/
*
***********************************************************************/

#ifndef QDECLARATIVETRANSITION_P_H
#define QDECLARATIVETRANSITION_P_H

#include <qdeclarativestate_p.h>
#include <qdeclarative.h>
#include <QtCore/qobject.h>

QT_BEGIN_NAMESPACE

class QDeclarativeAbstractAnimation;
class QDeclarativeTransitionPrivate;
class QDeclarativeTransitionManager;

class Q_DECLARATIVE_EXPORT QDeclarativeTransition : public QObject
{
   DECL_CS_OBJECT(QDeclarativeTransition)
   Q_DECLARE_PRIVATE(QDeclarativeTransition)

   DECL_CS_PROPERTY_READ(from, fromState)
   DECL_CS_PROPERTY_WRITE(from, setFromState)
   DECL_CS_PROPERTY_NOTIFY(from, fromChanged)
   DECL_CS_PROPERTY_READ(to, toState)
   DECL_CS_PROPERTY_WRITE(to, setToState)
   DECL_CS_PROPERTY_NOTIFY(to, toChanged)
   DECL_CS_PROPERTY_READ(reversible, reversible)
   DECL_CS_PROPERTY_WRITE(reversible, setReversible)
   DECL_CS_PROPERTY_NOTIFY(reversible, reversibleChanged)
   DECL_CS_PROPERTY_READ(animations, animations)
   DECL_CS_CLASSINFO("DefaultProperty", "animations")
   DECL_CS_CLASSINFO("DeferredPropertyNames", "animations")

 public:
   QDeclarativeTransition(QObject *parent = nullptr);
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
   DECL_CS_SIGNAL_1(Public, void fromChanged())
   DECL_CS_SIGNAL_2(fromChanged)
   DECL_CS_SIGNAL_1(Public, void toChanged())
   DECL_CS_SIGNAL_2(toChanged)
   DECL_CS_SIGNAL_1(Public, void reversibleChanged())
   DECL_CS_SIGNAL_2(reversibleChanged)
};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QDeclarativeTransition)

#endif // QDECLARATIVETRANSITION_H
