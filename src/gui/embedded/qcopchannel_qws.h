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

#ifndef QCOPCHANNEL_QWS_H
#define QCOPCHANNEL_QWS_H

#include <QtCore/qobject.h>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_COP

class QWSClient;
class QCopChannelPrivate;

class Q_GUI_EXPORT QCopChannel : public QObject
{
   GUI_CS_OBJECT(QCopChannel)

 public:
   explicit QCopChannel(const QString &channel, QObject *parent = nullptr);
   virtual ~QCopChannel();

   QString channel() const;

   static bool isRegistered(const QString  &channel);
   static bool send(const QString &channel, const QString &msg);
   static bool send(const QString &channel, const QString &msg,
                    const QByteArray &data);

   static bool flush();

   static void sendLocally( const QString &ch, const QString &msg,
                            const QByteArray &data);
   static void reregisterAll();

   virtual void receive(const QString &msg, const QByteArray &data);

   GUI_CS_SIGNAL_1(Public, void received(const QString &msg, const QByteArray &data))
   GUI_CS_SIGNAL_2(received, msg, data)

 private:
   void init(const QString &channel);

   // server side
   static void registerChannel(const QString &ch, QWSClient *cl);
   static void detach(QWSClient *cl);
   static void answer(QWSClient *cl, const QString &ch,
                      const QString &msg, const QByteArray &data);
   // client side
   QCopChannelPrivate *d;

   friend class QWSServer;
   friend class QWSServerPrivate;
   friend class QApplication;
};

#endif // QT_NO_COP

QT_END_NAMESPACE

#endif // QCOPCHANNEL_QWS_H
