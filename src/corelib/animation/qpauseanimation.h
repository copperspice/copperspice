/***********************************************************************
*
* Copyright (c) 2012-2014 Barbara Geller
* Copyright (c) 2012-2014 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or 
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see 
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
    CS_OBJECT(QPauseAnimation)

    CORE_CS_PROPERTY_READ(duration, duration)
    CORE_CS_PROPERTY_WRITE(duration, setDuration)

public:
    QPauseAnimation(QObject *parent = 0);
    QPauseAnimation(int msecs, QObject *parent = 0);
    ~QPauseAnimation();

    int duration() const;
    void setDuration(int msecs);

protected:
    bool event(QEvent *e);
    void updateCurrentTime(int);

private:
    Q_DISABLE_COPY(QPauseAnimation)
    Q_DECLARE_PRIVATE(QPauseAnimation)
};

#endif //QT_NO_ANIMATION

QT_END_NAMESPACE

#endif // QPAUSEANIMATION_P_H
