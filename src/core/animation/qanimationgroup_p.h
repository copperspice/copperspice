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

#ifndef QANIMATIONGROUP_P_H
#define QANIMATIONGROUP_P_H

#include <qanimationgroup.h>
#include <QtCore/qlist.h>
#include <qabstractanimation_p.h>

#ifndef QT_NO_ANIMATION

QT_BEGIN_NAMESPACE

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
      // nullptr as the signal, might be called from the animation destructor
      QObject::disconnect(anim, nullptr, q_func(), SLOT(_q_uncontrolledAnimationFinished()));
   }

   void connectUncontrolledAnimation(QAbstractAnimation *anim) {
      QObject::connect(anim, SIGNAL(finished()), q_func(), SLOT(_q_uncontrolledAnimationFinished()));
   }

   QList<QAbstractAnimation *> animations;
};

QT_END_NAMESPACE

#endif //QT_NO_ANIMATION

#endif //QANIMATIONGROUP_P_H
