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

#ifndef QBYTEDATA_P_H
#define QBYTEDATA_P_H

#include <qbytearray.h>
#include <qlist.h>

QT_BEGIN_NAMESPACE

// this class handles a list of QByteArrays, it is a variant of QRingBuffer
// which avoids malloc/realloc/memcpy

class QByteDataBuffer
{
   QList<QByteArray> buffers;
   qint64 bufferCompleteSize;

 public:
   QByteDataBuffer() : bufferCompleteSize(0) {
   }

   ~QByteDataBuffer() {
      clear();
   }

   inline void append(QByteDataBuffer &other) {
      if (other.isEmpty()) {
         return;
      }

      buffers.append(other.buffers);
      bufferCompleteSize += other.byteAmount();
   }


   inline void append(const QByteArray &bd) {
      if (bd.isEmpty()) {
         return;
      }

      buffers.append(bd);
      bufferCompleteSize += bd.size();
   }

   inline void prepend(const QByteArray &bd) {
      if (bd.isEmpty()) {
         return;
      }

      buffers.prepend(bd);
      bufferCompleteSize += bd.size();
   }

   // return the first QByteData. User of this function has to qFree() its .data!
   // preferably use this function to read data.
   inline QByteArray read() {
      bufferCompleteSize -= buffers.first().size();
      return buffers.takeFirst();
   }

   // return everything. User of this function has to qFree() its .data!
   // avoid to use this, it might malloc and memcpy.
   inline QByteArray readAll() {
      return read(byteAmount());
   }

   // return amount. User of this function has to qFree() its .data!
   // avoid to use this, it might malloc and memcpy.
   inline QByteArray read(qint64 amount) {
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

   inline char getChar() {
      char c;
      read(&c, 1);
      return c;
   }

   inline void clear() {
      buffers.clear();
      bufferCompleteSize = 0;
   }

   // The byte count of all QByteArrays
   inline qint64 byteAmount() const {
      return bufferCompleteSize;
   }

   // the number of QByteArrays
   inline qint64 bufferCount() const {
      return buffers.length();
   }

   inline bool isEmpty() const {
      return byteAmount() == 0;
   }

   inline qint64 sizeNextBlock() const {
      if (buffers.isEmpty()) {
         return 0;
      } else {
         return buffers.first().size();
      }
   }

   inline QByteArray &operator[](int i) {
      return buffers[i];
   }

   inline bool canReadLine() const {
      for (int i = 0; i < buffers.length(); i++)
         if (buffers.at(i).contains('\n')) {
            return true;
         }
      return false;
   }
};

QT_END_NAMESPACE

#endif // QBYTEDATA_H
