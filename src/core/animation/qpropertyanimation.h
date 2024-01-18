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

#ifndef QPROPERTYANIMATION_H
#define QPROPERTYANIMATION_H

#include <qvariantanimation.h>

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
   QPropertyAnimation(QObject *target, const QString &propertyName, QObject *parent = nullptr);

   QPropertyAnimation(const QPropertyAnimation &) = delete;
   QPropertyAnimation &operator=(const QPropertyAnimation &) = delete;

   ~QPropertyAnimation();

   QObject *targetObject() const;
   void setTargetObject(QObject *target);

   QString propertyName() const;
   void setPropertyName(const QString &propertyName);

 protected:
   bool event(QEvent *event) override;
   void updateCurrentValue(const QVariant &value) override;
   void updateState(QAbstractAnimation::State newState, QAbstractAnimation::State oldState) override;

 private:
   Q_DECLARE_PRIVATE(QPropertyAnimation)
};

#endif

#endif // QPROPERTYANIMATION_H
