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

#ifndef QPARALLELANIMATIONGROUP_H
#define QPARALLELANIMATIONGROUP_H

#include <QtCore/qanimationgroup.h>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_ANIMATION

class QParallelAnimationGroupPrivate;

class Q_CORE_EXPORT QParallelAnimationGroup : public QAnimationGroup
{
   CORE_CS_OBJECT(QParallelAnimationGroup)

 public:
   QParallelAnimationGroup(QObject *parent = nullptr);
   ~QParallelAnimationGroup();

   int duration() const override;

 protected:
   QParallelAnimationGroup(QParallelAnimationGroupPrivate &dd, QObject *parent);

   bool event(QEvent *event) override;
   void updateCurrentTime(int currentTime) override;
   void updateState(QAbstractAnimation::State newState, QAbstractAnimation::State oldState) override;
   void updateDirection(QAbstractAnimation::Direction direction) override;

 private:
   Q_DISABLE_COPY(QParallelAnimationGroup)
   Q_DECLARE_PRIVATE(QParallelAnimationGroup)

   CORE_CS_SLOT_1(Private, void _q_uncontrolledAnimationFinished())
   CORE_CS_SLOT_2(_q_uncontrolledAnimationFinished)
};

#endif //QT_NO_ANIMATION

QT_END_NAMESPACE

#endif // QPARALLELANIMATIONGROUP
