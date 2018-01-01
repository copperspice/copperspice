/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
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
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QPROPERTYANIMATION_H
#define QPROPERTYANIMATION_H

#include <QtCore/qvariantanimation.h>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_ANIMATION

class QPropertyAnimationPrivate;

class Q_CORE_EXPORT QPropertyAnimation : public QVariantAnimation
{
   CORE_CS_OBJECT(QPropertyAnimation)

   CORE_CS_PROPERTY_READ(propertyName, propertyName)
   CORE_CS_PROPERTY_WRITE(propertyName, setPropertyName)

   CORE_CS_PROPERTY_READ(targetObject, targetObject)
   CORE_CS_PROPERTY_WRITE(targetObject, setTargetObject)

 public:
   QPropertyAnimation(QObject *parent = nullptr);
   QPropertyAnimation(QObject *target, const QByteArray &propertyName, QObject *parent = nullptr);
   ~QPropertyAnimation();

   QObject *targetObject() const;
   void setTargetObject(QObject *target);

   QByteArray propertyName() const;
   void setPropertyName(const QByteArray &propertyName);

 protected:
   bool event(QEvent *event) override;
   void updateCurrentValue(const QVariant &value) override;
   void updateState(QAbstractAnimation::State newState, QAbstractAnimation::State oldState) override; 

 private:
   Q_DISABLE_COPY(QPropertyAnimation)
   Q_DECLARE_PRIVATE(QPropertyAnimation)
};

#endif //QT_NO_ANIMATION

QT_END_NAMESPACE

#endif // QPROPERTYANIMATION_H
