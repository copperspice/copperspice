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
   DECL_CS_CLASSINFO("DefaultProperty", "animation")
   DECL_CS_PROPERTY_READ(*animation, animation)
   DECL_CS_PROPERTY_WRITE(*animation, setAnimation)
   DECL_CS_PROPERTY_READ(enabled, enabled)
   DECL_CS_PROPERTY_WRITE(enabled, setEnabled)
   DECL_CS_PROPERTY_NOTIFY(enabled, enabledChanged)
   DECL_CS_CLASSINFO("DeferredPropertyNames", "animation")

 public:
   QDeclarativeBehavior(QObject *parent = nullptr);
   ~QDeclarativeBehavior();

   virtual void setTarget(const QDeclarativeProperty &);
   virtual void write(const QVariant &value);

   QDeclarativeAbstractAnimation *animation();
   void setAnimation(QDeclarativeAbstractAnimation *);

   bool enabled() const;
   void setEnabled(bool enabled);

   DECL_CS_SIGNAL_1(Public, void enabledChanged())
   DECL_CS_SIGNAL_2(enabledChanged)

 private :
   DECL_CS_SLOT_1(Private, void componentFinalized())
   DECL_CS_SLOT_2(componentFinalized)
   DECL_CS_SLOT_1(Private, void qtAnimationStateChanged(QAbstractAnimation::State un_named_arg1,
             QAbstractAnimation::State un_named_arg2))
   DECL_CS_SLOT_2(qtAnimationStateChanged)
};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QDeclarativeBehavior)

#endif // QDECLARATIVEBEHAVIOR_H
