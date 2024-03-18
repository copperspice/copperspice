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

#ifndef QSOCKETNOTIFIER_H
#define QSOCKETNOTIFIER_H

#include <qobject.h>

class QSocketNotifierPrivate;

class Q_CORE_EXPORT QSocketNotifier : public QObject
{
   CORE_CS_OBJECT(QSocketNotifier)

 public:
   enum Type {
      Read,
      Write,
      Exception
   };

   QSocketNotifier(qintptr socket, Type type, QObject *parent = nullptr);

   QSocketNotifier(const QSocketNotifier &) = delete;
   QSocketNotifier &operator=(const QSocketNotifier &) = delete;

   ~QSocketNotifier();

   qintptr socket() const;
   Type type() const;
   bool isEnabled() const;

   CORE_CS_SLOT_1(Public, void setEnabled(bool enable))
   CORE_CS_SLOT_2(setEnabled)

   CORE_CS_SIGNAL_1(Public, void activated(int socket))
   CORE_CS_SIGNAL_2(activated, socket)

 protected:
   bool event(QEvent *event) override;

 private:
   qintptr sockfd;
   QSocketNotifier::Type sntype;
   bool snenabled;
};

#endif