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

#ifndef QDECLARATIVESMOOTHEDANIMATION_P_H
#define QDECLARATIVESMOOTHEDANIMATION_P_H

#include <qdeclarative.h>
#include <qdeclarativeanimation_p.h>
#include <QtCore/qobject.h>

QT_BEGIN_NAMESPACE

class QDeclarativeProperty;
class QDeclarativeSmoothedAnimationPrivate;

class QDeclarativeSmoothedAnimation : public QDeclarativeNumberAnimation
{
   DECL_CS_OBJECT(QDeclarativeSmoothedAnimation)

   Q_DECLARE_PRIVATE(QDeclarativeSmoothedAnimation)
   CS_ENUM(ReversingMode)

   DECL_CS_PROPERTY_READ(velocity, velocity)
   DECL_CS_PROPERTY_WRITE(velocity, setVelocity)
   DECL_CS_PROPERTY_NOTIFY(velocity, velocityChanged)
   DECL_CS_PROPERTY_READ(reversingMode, reversingMode)
   DECL_CS_PROPERTY_WRITE(reversingMode, setReversingMode)
   DECL_CS_PROPERTY_NOTIFY(reversingMode, reversingModeChanged)
   DECL_CS_PROPERTY_READ(maximumEasingTime, maximumEasingTime)
   DECL_CS_PROPERTY_WRITE(maximumEasingTime, setMaximumEasingTime)
   DECL_CS_PROPERTY_NOTIFY(maximumEasingTime, maximumEasingTimeChanged)

 public:
   enum ReversingMode { Eased, Immediate, Sync };

   QDeclarativeSmoothedAnimation(QObject *parent = nullptr);
   ~QDeclarativeSmoothedAnimation();

   ReversingMode reversingMode() const;
   void setReversingMode(ReversingMode);

   virtual int duration() const;
   virtual void setDuration(int);

   qreal velocity() const;
   void setVelocity(qreal);

   int maximumEasingTime() const;
   void setMaximumEasingTime(int);

   virtual void transition(QDeclarativeStateActions &actions, QDeclarativeProperties &modified,
                           TransitionDirection direction);
   QAbstractAnimation *qtAnimation();

   DECL_CS_SIGNAL_1(Public, void velocityChanged())
   DECL_CS_SIGNAL_2(velocityChanged)
   DECL_CS_SIGNAL_1(Public, void reversingModeChanged())
   DECL_CS_SIGNAL_2(reversingModeChanged)
   DECL_CS_SIGNAL_1(Public, void maximumEasingTimeChanged())
   DECL_CS_SIGNAL_2(maximumEasingTimeChanged)
};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QDeclarativeSmoothedAnimation)

#endif // QDECLARATIVESMOOTHEDANIMATION_H
