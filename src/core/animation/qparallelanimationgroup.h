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

#ifndef QPARALLELANIMATIONGROUP_H
#define QPARALLELANIMATIONGROUP_H

#include <qanimationgroup.h>

#ifndef QT_NO_ANIMATION

class QParallelAnimationGroupPrivate;

class Q_CORE_EXPORT QParallelAnimationGroup : public QAnimationGroup
{
   CORE_CS_OBJECT(QParallelAnimationGroup)

 public:
   QParallelAnimationGroup(QObject *parent = nullptr);

   QParallelAnimationGroup(const QParallelAnimationGroup &) = delete;
   QParallelAnimationGroup &operator=(const QParallelAnimationGroup &) = delete;

   ~QParallelAnimationGroup();

   int duration() const override;

 protected:
   QParallelAnimationGroup(QParallelAnimationGroupPrivate &dd, QObject *parent);

   bool event(QEvent *event) override;
   void updateCurrentTime(int currentTime) override;
   void updateState(QAbstractAnimation::State newState, QAbstractAnimation::State oldState) override;
   void updateDirection(QAbstractAnimation::Direction direction) override;

 private:
   Q_DECLARE_PRIVATE(QParallelAnimationGroup)

   // slot
   void _q_uncontrolledAnimationFinished() override;
};

#endif // QT_NO_ANIMATION

#endif
