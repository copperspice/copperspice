/***********************************************************************
*
* Copyright (c) 2012-2015 Barbara Geller
* Copyright (c) 2012-2015 Ansel Sermersheim
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

#ifndef QDECLARATIVEBEHAVIOR_P_H
#define QDECLARATIVEBEHAVIOR_P_H

#include <qdeclarativestate_p.h>
#include <qdeclarativepropertyvaluesource.h>
#include <qdeclarativepropertyvalueinterceptor.h>
#include <qdeclarative.h>
#include <QtCore/QAbstractAnimation>

QT_BEGIN_NAMESPACE

class QDeclarativeAbstractAnimation;
class QDeclarativeBehaviorPrivate;

class Q_DECLARATIVE_PRIVATE_EXPORT QDeclarativeBehavior : public QObject, public QDeclarativePropertyValueInterceptor
{
   DECL_CS_OBJECT(QDeclarativeBehavior)
   Q_DECLARE_PRIVATE(QDeclarativeBehavior)

   CS_INTERFACES(QDeclarativePropertyValueInterceptor)
   CS_CLASSINFO("DefaultProperty", "animation")
   CS_PROPERTY_READ(*animation, animation)
   CS_PROPERTY_WRITE(*animation, setAnimation)
   CS_PROPERTY_READ(enabled, enabled)
   CS_PROPERTY_WRITE(enabled, setEnabled)
   CS_PROPERTY_NOTIFY(enabled, enabledChanged)
   CS_CLASSINFO("DeferredPropertyNames", "animation")

 public:
   QDeclarativeBehavior(QObject *parent = 0);
   ~QDeclarativeBehavior();

   virtual void setTarget(const QDeclarativeProperty &);
   virtual void write(const QVariant &value);

   QDeclarativeAbstractAnimation *animation();
   void setAnimation(QDeclarativeAbstractAnimation *);

   bool enabled() const;
   void setEnabled(bool enabled);

   CS_SIGNAL_1(Public, void enabledChanged())
   CS_SIGNAL_2(enabledChanged)

 private :
   CS_SLOT_1(Private, void componentFinalized())
   CS_SLOT_2(componentFinalized)
   CS_SLOT_1(Private, void qtAnimationStateChanged(QAbstractAnimation::State un_named_arg1,
             QAbstractAnimation::State un_named_arg2))
   CS_SLOT_2(qtAnimationStateChanged)
};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QDeclarativeBehavior)

#endif // QDECLARATIVEBEHAVIOR_H
