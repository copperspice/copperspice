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

#ifndef QSOCKETNOTIFIER_H
#define QSOCKETNOTIFIER_H

#include <QtCore/qobject.h>

QT_BEGIN_NAMESPACE

class Q_CORE_EXPORT QSocketNotifier : public QObject
{
   CORE_CS_OBJECT(QSocketNotifier)

 public:
   enum Type { Read, Write, Exception };

   QSocketNotifier(int socket, Type, QObject *parent = nullptr);
   ~QSocketNotifier();

   inline int socket() const {
      return sockfd;
   }
   inline Type type() const {
      return sntype;
   }

   inline bool isEnabled() const {
      return snenabled;
   }

   CORE_CS_SLOT_1(Public, void setEnabled(bool un_named_arg1))
   CORE_CS_SLOT_2(setEnabled)

   CORE_CS_SIGNAL_1(Public, void activated(int socket))
   CORE_CS_SIGNAL_2(activated, socket)

 protected:
   bool event(QEvent *) override;

 private:
   Q_DISABLE_COPY(QSocketNotifier)

   int sockfd;
   Type sntype;
   bool snenabled;
};

QT_END_NAMESPACE

#endif // QSOCKETNOTIFIER_H
