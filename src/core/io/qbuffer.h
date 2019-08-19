/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

#ifndef QBUFFER_H
#define QBUFFER_H

#include <qiodevice.h>
#include <qbytearray.h>

class QObject;
class QBufferPrivate;

class Q_CORE_EXPORT QBuffer : public QIODevice
{
   CORE_CS_OBJECT(QBuffer)

 public:
   explicit QBuffer(QObject *parent = nullptr);
   QBuffer(QByteArray *buf, QObject *parent = nullptr);

   ~QBuffer();

   QByteArray &buffer();
   const QByteArray &buffer() const;
   void setBuffer(QByteArray *a);

   void setData(const QByteArray &data);
   inline void setData(const char *data, int len);
   const QByteArray &data() const;

   bool open(OpenMode openMode) override;
   void close() override;

   qint64 size() const override;
   qint64 pos() const override;
   bool seek(qint64 off) override;
   bool atEnd() const override;
   bool canReadLine() const override;

 protected:
   void connectNotify(const QMetaMethod &signalMethod) const override;
   void disconnectNotify(const QMetaMethod &signalMethod) const override;

   qint64 readData(char *data, qint64 maxlen) override;
   qint64 writeData(const char *data, qint64 len) override;

 private:
   Q_DECLARE_PRIVATE(QBuffer)
   Q_DISABLE_COPY(QBuffer)

   CORE_CS_SLOT_1(Private, void _q_emitSignals())
   CORE_CS_SLOT_2(_q_emitSignals)
};

inline void QBuffer::setData(const char *adata, int alen)
{
   setData(QByteArray(adata, alen));
}

#endif // QBUFFER_H
