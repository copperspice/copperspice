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

#include <qpauseanimation.h>

#include <qabstractanimation_p.h>

#ifndef QT_NO_ANIMATION

class QPauseAnimationPrivate : public QAbstractAnimationPrivate
{
 public:
   QPauseAnimationPrivate() : QAbstractAnimationPrivate(), duration(250) {
      isPause = true;
   }

   int duration;
};

QPauseAnimation::QPauseAnimation(QObject *parent) : QAbstractAnimation(*new QPauseAnimationPrivate, parent)
{
}

QPauseAnimation::QPauseAnimation(int msecs, QObject *parent) : QAbstractAnimation(*new QPauseAnimationPrivate, parent)
{
   setDuration(msecs);
}

QPauseAnimation::~QPauseAnimation()
{
}

int QPauseAnimation::duration() const
{
   Q_D(const QPauseAnimation);
   return d->duration;
}

void QPauseAnimation::setDuration(int msecs)
{
   if (msecs < 0) {
      qWarning("QPauseAnimation::setDuration() Unable to set a negative duration");
      return;
   }

   Q_D(QPauseAnimation);
   d->duration = msecs;
}

bool QPauseAnimation::event(QEvent *e)
{
   return QAbstractAnimation::event(e);
}

void QPauseAnimation::updateCurrentTime(int)
{
}

#endif //QT_NO_ANIMATION
