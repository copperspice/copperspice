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

#ifndef QKBDLINUXINPUT_QWS_H
#define QKBDLINUXINPUT_QWS_H

#include <QtGui/qkbd_qws.h>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_QWS_KEYBOARD

#ifndef QT_NO_QWS_KBD_LINUXINPUT

class QWSLinuxInputKbPrivate;

class QWSLinuxInputKeyboardHandler : public QWSKeyboardHandler
{

 public:
   QWSLinuxInputKeyboardHandler(const QString &);
   virtual ~QWSLinuxInputKeyboardHandler();

   virtual bool filterInputEvent(quint16 &input_code, qint32 &input_value);

 private:
   QWSLinuxInputKbPrivate *d;
};

#endif // QT_NO_QWS_KBD_LINUXINPUT

#endif // QT_NO_QWS_KEYBOARD

QT_END_NAMESPACE

#endif // QKBDLINUXINPUT_QWS_H
