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

#include <qdebug.h>

#include <qpatternistlocale_p.h>
#include <qiodevicedelegate_p.h>

using namespace QPatternist;

QIODeviceDelegate::QIODeviceDelegate(QIODevice *const source) : m_source(source)
{
   Q_ASSERT(m_source);

   connect(source, &QIODevice::aboutToClose,         this, &QIODeviceDelegate::aboutToClose);
   connect(source, &QIODevice::bytesWritten,         this, &QIODeviceDelegate::bytesWritten);
   connect(source, &QIODevice::readyRead,            this, &QIODeviceDelegate::readyRead);
   connect(source, &QIODevice::readChannelFinished,  this, &QIODeviceDelegate::readChannelFinished);
   connect(source, &QIODevice::readChannelFinished,  this, &QIODeviceDelegate::finished);

   /* For instance QFile emits no signals, so how do we know if the device has all data available
    * and it therefore is safe and correct to emit finished()? isSequential() tells us whether it's
    * not random access, and whether it's safe to emit finished(). */
   if (m_source->isSequential()) {
      QMetaObject::invokeMethod(this, "readyRead", Qt::QueuedConnection);
   } else {
      QMetaObject::invokeMethod(this, "finished", Qt::QueuedConnection);
   }

   setOpenMode(QIODevice::ReadOnly);

   /* Set up the timeout timer. */
   connect(&m_timeout, &QTimer::timeout, this, &QIODeviceDelegate::networkTimeout);

   m_timeout.setSingleShot(true);
   m_timeout.start(Timeout);
}

void QIODeviceDelegate::networkTimeout()
{
   setErrorString(QtXmlPatterns::tr("Network timeout."));
   error(QNetworkReply::TimeoutError);
}

void QIODeviceDelegate::abort()
{
   /* Do nothing, just to please QNetworkReply's pure virtual. */
}

bool QIODeviceDelegate::atEnd() const
{
   return m_source->atEnd();
}

qint64 QIODeviceDelegate::bytesAvailable() const
{
   return m_source->bytesAvailable();
}

qint64 QIODeviceDelegate::bytesToWrite() const
{
   return m_source->bytesToWrite();
}

bool QIODeviceDelegate::canReadLine() const
{
   return m_source->canReadLine();
}

void QIODeviceDelegate::close()
{
   return m_source->close();
}

bool QIODeviceDelegate::isSequential() const
{
   return m_source->isSequential();
}

bool QIODeviceDelegate::open(OpenMode mode)
{
   const bool success = m_source->open(mode);
   setOpenMode(m_source->openMode());
   return success;
}

qint64 QIODeviceDelegate::pos() const
{
   return m_source->pos();
}

bool QIODeviceDelegate::reset()
{
   return m_source->reset();
}

bool QIODeviceDelegate::seek(qint64 pos)
{
   return m_source->seek(pos);
}

qint64 QIODeviceDelegate::size() const
{
   return m_source->size();
}

bool QIODeviceDelegate::waitForBytesWritten(int msecs)
{
   return m_source->waitForBytesWritten(msecs);
}

bool QIODeviceDelegate::waitForReadyRead(int msecs)
{
   return m_source->waitForReadyRead(msecs);
}

qint64 QIODeviceDelegate::readData(char *data, qint64 maxSize)
{
   return m_source->read(data, maxSize);
}
