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

#ifndef QMOUSEPC_QWS_H
#define QMOUSEPC_QWS_H

#include <QtGui/qmouse_qws.h>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_QWS_MOUSE_PC

class QWSPcMouseHandlerPrivate;

class QWSPcMouseHandler : public QWSMouseHandler
{

 public:
   explicit QWSPcMouseHandler(const QString & = QString(), const QString & = QString());
   ~QWSPcMouseHandler();

   void suspend();
   void resume();

 protected:
   QWSPcMouseHandlerPrivate *d;
};

#endif // QT_NO_QWS_MOUSE_PC

QT_END_NAMESPACE

#endif // QMOUSEPC_QWS_H
