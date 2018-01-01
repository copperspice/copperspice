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

#ifndef QMOUSEVFB_QWS_H
#define QMOUSEVFB_QWS_H

#include <QtGui/qmouse_qws.h>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_QWS_MOUSE_QVFB

class QSocketNotifier;

class QVFbMouseHandler : public QObject, public QWSMouseHandler
{
   GUI_CS_OBJECT_MULTIPLE(QVFbMouseHandler, QObject)

 public:
   QVFbMouseHandler(const QString &driver = QString(),
                    const QString &device = QString());
   ~QVFbMouseHandler();

   void resume();
   void suspend();

 private:
   int mouseFD;
   int mouseIdx;
   enum {mouseBufSize = 128};
   uchar mouseBuf[mouseBufSize];
   QSocketNotifier *mouseNotifier;

   GUI_CS_SLOT_1(Private, void readMouseData())
   GUI_CS_SLOT_2(readMouseData)
};
#endif // QT_NO_QWS_MOUSE_QVFB

QT_END_NAMESPACE

#endif // QMOUSEVFB_QWS_H
