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

#ifndef QANIMATIONGROUP_P_H
#define QANIMATIONGROUP_P_H

#include <qanimationgroup.h>

#include <qlist.h>

#include <qabstractanimation_p.h>

#ifndef QT_NO_ANIMATION

class QAnimationGroupPrivate : public QAbstractAnimationPrivate
{
   Q_DECLARE_PUBLIC(QAnimationGroup)

 public:
   QAnimationGroupPrivate() {
      isGroup = true;
   }

   virtual void animationInsertedAt(int) { }
   virtual void animationRemoved(int, QAbstractAnimation *);

   void disconnectUncontrolledAnimation(QAbstractAnimation *anim) {
      QObject::disconnect(anim, &QAbstractAnimation::finished, q_func(), &QAnimationGroup::_q_uncontrolledAnimationFinished);
   }

   void connectUncontrolledAnimation(QAbstractAnimation *anim) {
      QObject::connect(anim, &QAbstractAnimation::finished, q_func(), &QAnimationGroup::_q_uncontrolledAnimationFinished);
   }

   QList<QAbstractAnimation *> animations;
};

#endif //QT_NO_ANIMATION

#endif
