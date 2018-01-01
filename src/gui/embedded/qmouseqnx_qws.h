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

#ifndef QMOUSEQNX_QWS_H
#define QMOUSEQNX_QWS_H

#include <QtCore/qobject.h>
#include <QtGui/qmouse_qws.h>

QT_BEGIN_NAMESPACE

class QSocketNotifier;

class Q_GUI_EXPORT QQnxMouseHandler : public QObject, public QWSMouseHandler
{
   GUI_CS_OBJECT(QQnxMouseHandler)

 public:
   explicit QQnxMouseHandler(const QString &driver = QString(), const QString &device = QString());
   ~QQnxMouseHandler();

   void resume();
   void suspend();

 private :
   GUI_CS_SLOT_1(Private, void socketActivated())
   GUI_CS_SLOT_2(socketActivated)

   QSocketNotifier *mouseNotifier;
   int mouseFD;
   int mouseButtons;
   bool absolutePositioning;
};

QT_END_NAMESPACE

#endif // QMOUSE_QWS_H
