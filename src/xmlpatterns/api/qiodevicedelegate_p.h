/***********************************************************************
*
* Copyright (c) 2012-2015 Barbara Geller
* Copyright (c) 2012-2015 Ansel Sermersheim
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

#ifndef QIODEVICEDELEGATE_P_H
#define QIODEVICEDELEGATE_P_H

#include <QtCore/QTimer>
#include <QtNetwork/QNetworkReply>

QT_BEGIN_NAMESPACE

namespace QPatternist {
/**
 * This is read-only currently.
 */
class QIODeviceDelegate : public QNetworkReply
{
   CS_OBJECT(QIODeviceDelegate)
 public:
   QIODeviceDelegate(QIODevice *const source);

   virtual void abort();

   virtual bool atEnd() const;
   virtual qint64 bytesAvailable() const;
   virtual qint64 bytesToWrite() const;
   virtual bool canReadLine() const;
   virtual void close();
   virtual bool isSequential() const;
   virtual bool open(OpenMode mode);
   virtual qint64 pos() const;
   virtual bool reset();
   virtual bool seek(qint64 pos);
   virtual qint64 size() const;
   virtual bool waitForBytesWritten(int msecs);
   virtual bool waitForReadyRead(int msecs);

 protected:
   virtual qint64 readData(char *data, qint64 maxSize);

 private :
   XMLP_CS_SLOT_1(Private, void networkTimeout())
   XMLP_CS_SLOT_2(networkTimeout)

 private:
   enum {
      /**
       * 20 seconds expressed in milliseconds.
       */
      Timeout = 20000
   };

   QIODevice *const m_source;
   QTimer m_timeout;
};
}

QT_END_NAMESPACE

#endif
