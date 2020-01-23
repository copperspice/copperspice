/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
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

#ifndef QANIMATIONGROUP_H
#define QANIMATIONGROUP_H

#include <QtCore/qabstractanimation.h>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_ANIMATION

class QAnimationGroupPrivate;

class Q_CORE_EXPORT QAnimationGroup : public QAbstractAnimation
{
   CORE_CS_OBJECT(QAnimationGroup)

 public:
   QAnimationGroup(QObject *parent = nullptr);
   ~QAnimationGroup();

   QAbstractAnimation *animationAt(int index) const;
   int animationCount() const;
   int indexOfAnimation(QAbstractAnimation *animation) const;
   void addAnimation(QAbstractAnimation *animation);
   void insertAnimation(int index, QAbstractAnimation *animation);
   void removeAnimation(QAbstractAnimation *animation);
   QAbstractAnimation *takeAnimation(int index);
   void clear();

 protected:
   QAnimationGroup(QAnimationGroupPrivate &dd, QObject *parent);
   bool event(QEvent *event) override;

 private:
   Q_DISABLE_COPY(QAnimationGroup)
   Q_DECLARE_PRIVATE(QAnimationGroup)
};

#endif //QT_NO_ANIMATION

QT_END_NAMESPACE

#endif //QANIMATIONGROUP_H
