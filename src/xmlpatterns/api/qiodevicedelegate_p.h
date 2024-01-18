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

#ifndef QIODEVICEDELEGATE_P_H
#define QIODEVICEDELEGATE_P_H

#include <qtimer.h>
#include <qnetwork_reply.h>

namespace QPatternist {
/**
 * This is read-only currently.
 */
class QIODeviceDelegate : public QNetworkReply
{
   XMLP_CS_OBJECT(QIODeviceDelegate)

 public:
   QIODeviceDelegate(QIODevice *const source);

   void abort() override;

   bool atEnd() const override;
   qint64 bytesAvailable() const override;
   qint64 bytesToWrite() const override;
   bool canReadLine() const override;
   void close() override;
   bool isSequential() const override;
   bool open(OpenMode mode) override;
   qint64 pos() const override;
   bool reset() override;
   bool seek(qint64 pos) override;
   qint64 size() const override;
   bool waitForBytesWritten(int msecs) override;
   bool waitForReadyRead(int msecs) override;

 protected:
   qint64 readData(char *data, qint64 maxSize) override;

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

#endif
