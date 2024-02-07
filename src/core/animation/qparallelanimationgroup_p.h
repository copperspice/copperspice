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

#ifndef QPARALLELANIMATIONGROUP_P_H
#define QPARALLELANIMATIONGROUP_P_H

#include <qparallelanimationgroup.h>

#include <qhash.h>

#include <qanimationgroup_p.h>

#ifndef QT_NO_ANIMATION

class QParallelAnimationGroupPrivate : public QAnimationGroupPrivate
{
   Q_DECLARE_PUBLIC(QParallelAnimationGroup)

 public:
   QParallelAnimationGroupPrivate()
      : lastLoop(0), lastCurrentTime(0) {
   }

   bool shouldAnimationStart(QAbstractAnimation *animation, bool startIfAtEnd) const;
   void applyGroupState(QAbstractAnimation *animation);
   bool isUncontrolledAnimationFinished(QAbstractAnimation *anim) const;
   void connectUncontrolledAnimations();
   void disconnectUncontrolledAnimations();

   void animationRemoved(int index, QAbstractAnimation *) override;

   void _q_uncontrolledAnimationFinished();

   QHash<QAbstractAnimation *, int> uncontrolledFinishTime;
   int lastLoop;
   int lastCurrentTime;
};

#endif //QT_NO_ANIMATION

#endif
