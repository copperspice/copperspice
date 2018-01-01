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

#ifndef QSEQUENTIALANIMATIONGROUP_P_H
#define QSEQUENTIALANIMATIONGROUP_P_H

#include <qsequentialanimationgroup.h>
#include <qanimationgroup_p.h>

#ifndef QT_NO_ANIMATION

QT_BEGIN_NAMESPACE

class QSequentialAnimationGroupPrivate : public QAnimationGroupPrivate
{
   Q_DECLARE_PUBLIC(QSequentialAnimationGroup)

 public:
   QSequentialAnimationGroupPrivate()
      : currentAnimation(0), currentAnimationIndex(-1), lastLoop(0) {
   }


   struct AnimationIndex {
      AnimationIndex() : index(0), timeOffset(0) {}
      // index points to the animation at timeOffset, skipping 0 duration animations.
      // Note that the index semantic is slightly different depending on the direction.
      int index; // the index of the animation in timeOffset
      int timeOffset; // time offset when the animation at index starts.
   };

   int animationActualTotalDuration(int index) const;
   AnimationIndex indexForCurrentTime() const;

   void setCurrentAnimation(int index, bool intermediate = false);
   void activateCurrentAnimation(bool intermediate = false);

   void animationInsertedAt(int index) override;
   void animationRemoved(int index, QAbstractAnimation *anim) override;

   bool atEnd() const;

   QAbstractAnimation *currentAnimation;
   int currentAnimationIndex;

   // this is the actual duration of uncontrolled animations
   // it helps seeking and even going forward
   QList<int> actualDuration;

   void restart();
   int lastLoop;

   // handle time changes
   void rewindForwards(const AnimationIndex &newAnimationIndex);
   void advanceForwards(const AnimationIndex &newAnimationIndex);

   void _q_uncontrolledAnimationFinished();
};

QT_END_NAMESPACE

#endif //QT_NO_ANIMATION

#endif //QSEQUENTIALANIMATIONGROUP_P_H
