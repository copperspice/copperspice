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

#ifndef QBYTEDATA_P_H
#define QBYTEDATA_P_H

#include <qbytearray.h>
#include <qlist.h>

// this class handles a list of QByteArrays, it is a variant of QRingBuffer
// which avoids malloc/realloc/memcpy

class QByteDataBuffer
{
   QList<QByteArray> buffers;
   qint64 bufferCompleteSize;

 public:
   QByteDataBuffer()
      : bufferCompleteSize(0)
   {
   }

   ~QByteDataBuffer()
   {
      clear();
   }

   void append(QByteDataBuffer &other) {
      if (other.isEmpty()) {
         return;
      }

      buffers.append(other.buffers);
      bufferCompleteSize += other.byteAmount();
   }

   void append(const QByteArray &bd) {
      if (bd.isEmpty()) {
         return;
      }

      buffers.append(bd);
      bufferCompleteSize += bd.size();
   }

   void prepend(const QByteArray &bd) {
      if (bd.isEmpty()) {
         return;
      }

      buffers.prepend(bd);
      bufferCompleteSize += bd.size();
   }

   // return the first QByteData. User of this function has to qFree() its .data
   // preferably use this function to read data.
   QByteArray read() {
      bufferCompleteSize -= buffers.first().size();
      return buffers.takeFirst();
   }

   // return everything. User of this function has to qFree() its .data
   // avoid to use this, it might malloc and memcpy.
   QByteArray readAll() {
      return read(byteAmount());
   }

   // return amount. User of this function has to qFree() its .data!
   // avoid to use this, it might malloc and memcpy.
   QByteArray read(qint64 amount) {
      amount = qMin(byteAmount(), amount);
      QByteArray byteData;
      byteData.resize(amount);
      read(byteData.data(), byteData.size());
      return byteData;
   }

   // return amount bytes. User of this function has to qFree() its .data!
   // avoid to use this, it will memcpy.
   qint64 read(char *dst, qint64 amount) {
      amount = qMin(amount, byteAmount());
      qint64 originalAmount = amount;
      char *writeDst = dst;

      while (amount > 0) {
         QByteArray first = buffers.takeFirst();

         if (amount >= first.size()) {
            // take it completely
            bufferCompleteSize -= first.size();
            amount -= first.size();
            memcpy(writeDst, first.constData(), first.size());
            writeDst += first.size();
            first.clear();
         } else {
            // take a part of it & it is the last one to take
            bufferCompleteSize -= amount;
            memcpy(writeDst, first.constData(), amount);

            qint64 newFirstSize = first.size() - amount;
            QByteArray newFirstData;
            newFirstData.resize(newFirstSize);
            memcpy(newFirstData.data(), first.constData() + amount, newFirstSize);
            buffers.prepend(newFirstData);

            amount = 0;
            first.clear();
         }
      }

      return originalAmount;
   }

   char getChar() {
      char c;
      read(&c, 1);
      return c;
   }

   void clear() {
      buffers.clear();
      bufferCompleteSize = 0;
   }

   // byte count of all QByteArrays
   qint64 byteAmount() const {
      return bufferCompleteSize;
   }

   // bumber of QByteArrays
   qint64 bufferCount() const {
      return buffers.length();
   }

   bool isEmpty() const {
      return byteAmount() == 0;
   }

   qint64 sizeNextBlock() const {
      if (buffers.isEmpty()) {
         return 0;
      } else {
         return buffers.first().size();
      }
   }

   QByteArray &operator[](int i) {
      return buffers[i];
   }

   bool canReadLine() const {
      for (int i = 0; i < buffers.length(); i++)
         if (buffers.at(i).contains('\n')) {
            return true;
         }

      return false;
   }
};

#endif
