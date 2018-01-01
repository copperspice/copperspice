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

#ifndef QSEQUENTIALANIMATIONGROUP_H
#define QSEQUENTIALANIMATIONGROUP_H

#include <QtCore/qanimationgroup.h>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_ANIMATION

class QPauseAnimation;
class QSequentialAnimationGroupPrivate;

class Q_CORE_EXPORT QSequentialAnimationGroup : public QAnimationGroup
{
   CORE_CS_OBJECT(QSequentialAnimationGroup)

   CORE_CS_PROPERTY_READ(currentAnimation, currentAnimation)
   CORE_CS_PROPERTY_NOTIFY(currentAnimation, currentAnimationChanged)

 public:
   QSequentialAnimationGroup(QObject *parent = nullptr);
   ~QSequentialAnimationGroup();

   QPauseAnimation *addPause(int msecs);
   QPauseAnimation *insertPause(int index, int msecs);

   QAbstractAnimation *currentAnimation() const;
   int duration() const override;

   CORE_CS_SIGNAL_1(Public, void currentAnimationChanged(QAbstractAnimation *current))
   CORE_CS_SIGNAL_2(currentAnimationChanged, current)

 protected:
   QSequentialAnimationGroup(QSequentialAnimationGroupPrivate &dd, QObject *parent);

   bool event(QEvent *event) override;
   void updateCurrentTime(int) override;
   void updateState(QAbstractAnimation::State newState, QAbstractAnimation::State oldState) override;
   void updateDirection(QAbstractAnimation::Direction direction) override;

 private:
   Q_DISABLE_COPY(QSequentialAnimationGroup)
   Q_DECLARE_PRIVATE(QSequentialAnimationGroup)

   CORE_CS_SLOT_1(Private, void _q_uncontrolledAnimationFinished())
   CORE_CS_SLOT_2(_q_uncontrolledAnimationFinished)
};

#endif //QT_NO_ANIMATION

QT_END_NAMESPACE

#endif //QSEQUENTIALANIMATIONGROUP_H
