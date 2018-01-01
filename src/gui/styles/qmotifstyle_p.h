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

#ifndef QMOTIFSTYLE_P_H
#define QMOTIFSTYLE_P_H

#include <qlist.h>
#include <qdatetime.h>
#include <qprogressbar.h>
#include <qmotifstyle.h>
#include <qcommonstyle_p.h>

QT_BEGIN_NAMESPACE

class QMotifStylePrivate : public QCommonStylePrivate
{
   Q_DECLARE_PUBLIC(QMotifStyle)

 public:
   QMotifStylePrivate();

#ifndef QT_NO_PROGRESSBAR
   QList<QProgressBar *> bars;
   int animationFps;
   int animateTimer;
   QTime startTime;
   int animateStep;
#endif

};

QT_END_NAMESPACE

#endif //QMOTIFSTYLE_P_H
