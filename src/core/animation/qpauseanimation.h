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

#ifndef QPAUSEANIMATION_H
#define QPAUSEANIMATION_H

#include <qanimationgroup.h>

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

   QPauseAnimation(const QPauseAnimation &) = delete;
   QPauseAnimation &operator=(const QPauseAnimation &) = delete;

   ~QPauseAnimation();

   int duration() const override;
   void setDuration(int msecs);

 protected:
   bool event(QEvent *event) override;
   void updateCurrentTime(int) override;

 private:
   Q_DECLARE_PRIVATE(QPauseAnimation)
};

#endif //QT_NO_ANIMATION

#endif
