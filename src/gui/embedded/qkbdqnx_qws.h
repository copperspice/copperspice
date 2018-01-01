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

#ifndef QKBDQNX_QWS_H
#define QKBDQNX_QWS_H

#include <QtGui/qapplication.h>
#include <QtGui/qkbd_qws.h>

QT_BEGIN_NAMESPACE

#if ! defined(QT_NO_QWS_KEYBOARD) && ! defined(QT_NO_QWS_KBD_QNX)

class Q_GUI_EXPORT QWSQnxKeyboardHandler : public QObject, public QWSKeyboardHandler
{
   GUI_CS_OBJECT_MULTIPLE(QWSQnxKeyboardHandler, QObject)

 public:
   QWSQnxKeyboardHandler(const QString &device);
   ~QWSQnxKeyboardHandler();

 private :
   GUI_CS_SLOT_1(Private, void socketActivated())
   GUI_CS_SLOT_2(socketActivated)

   int keyboardFD;
};

#endif // QT_NO_QWS_KEYBOARD

QT_END_NAMESPACE

#endif // QKBDQNX_QWS_H
