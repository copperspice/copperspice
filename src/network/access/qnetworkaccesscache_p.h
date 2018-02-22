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

#ifndef QNETWORKACCESSCACHE_P_H
#define QNETWORKACCESSCACHE_P_H

#include <qobject.h>
#include <qbasictimer.h>
#include <qbytearray.h>
#include <qdatetime.h>
#include <qhash.h>
#include <qmetatype.h>
#include <qqueue.h>

QT_BEGIN_NAMESPACE

class QNetworkRequest;
class QUrl;

// this class is not about caching files, but rather about caching objects used by
// QNetworkAccessManager, e.g. existing TCP connections or credentials.

class QNetworkAccessCache : public QObject
{
   NET_CS_OBJECT(QNetworkAccessCache)

 public:


   class CacheableObject
   {
    public:
       CacheableObject();
       virtual ~CacheableObject();

       virtual void dispose() = 0;

       inline QByteArray cacheKey() const {
          return key;
       }

    protected:
      void setExpires(bool enable);
      void setShareable(bool enable);

    private:
      QByteArray key;
      bool expires;
      bool shareable;

      friend class QNetworkAccessCache;
   };

   struct Receiver {
      QPointer<QObject> object;
      const char *member;
   };

   struct Node {
      QDateTime timestamp;
      QQueue<Receiver> receiverQueue;
      QByteArray key;

      Node *older;
      Node *newer;
      CacheableObject *object;

      int useCount;

      Node()
         : older(nullptr), newer(nullptr), object(nullptr), useCount(0) {
      }
   };

   QNetworkAccessCache();
   ~QNetworkAccessCache();

   void clear();

   void addEntry(const QByteArray &key, CacheableObject *entry);
   bool hasEntry(const QByteArray &key) const;
   bool requestEntry(const QByteArray &key, QObject *target, const char *member);

   CacheableObject *requestEntryNow(const QByteArray &key);
   void releaseEntry(const QByteArray &key);
   void removeEntry(const QByteArray &key);

   NET_CS_SIGNAL_1(Public, void entryReady(QNetworkAccessCache::CacheableObject *un_named_arg1))
   NET_CS_SIGNAL_2(entryReady, un_named_arg1)

 protected:
   void timerEvent(QTimerEvent *) override;

 private:
   // idea copied from qcache.h
   QHash<QByteArray, Node> hash;
   Node *oldest;
   Node *newest;

   QBasicTimer timer;

   void linkEntry(const QByteArray &key);
   bool unlinkEntry(const QByteArray &key);
   void updateTimer();
   bool emitEntryReady(Node *node, QObject *target, const char *member);
};

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QNetworkAccessCache::CacheableObject *)

#endif
