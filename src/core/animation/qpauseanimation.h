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

#ifndef QPAUSEANIMATION_H
#define QPAUSEANIMATION_H

#include <QtCore/qanimationgroup.h>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_ANIMATION

class QPauseAnimationPrivate;

class Q_CORE_EXPORT QPauseAnimation : public QAbstractAnimation
{
   CORE_CS_OBJECT(QPauseAnimation)

   CORE_CS_PROPERTY_READ(duration, duration)
   CORE_CS_PROPERTY_WRITE(duration, setDuration)

 public:
   QPauseAnimation(QObject *parent = nullptr);
   QPauseAnimation(int msecs, QObject *parent = nullptr);
   ~QPauseAnimation();

   int duration() const override;
   void setDuration(int msecs);

 protected:
   bool event(QEvent *e) override;
   void updateCurrentTime(int) override;

 private:
   Q_DISABLE_COPY(QPauseAnimation)
   Q_DECLARE_PRIVATE(QPauseAnimation)
};

#endif //QT_NO_ANIMATION

QT_END_NAMESPACE

#endif // QPAUSEANIMATION_P_H
