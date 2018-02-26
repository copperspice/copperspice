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

#include <qnetworkaccesscache_p.h>

#include <qpointer.h>
#include <qnetworkaccessmanager_p.h>
#include <qnetworkreply_p.h>
#include <qnetworkrequest.h>

enum ExpiryTimeEnum {
   ExpiryTime = 120
};

QNetworkAccessCache::CacheableObject::CacheableObject()
{
   // leave the members uninitialized
   // they must be initialized by the derived class's constructor
}

QNetworkAccessCache::CacheableObject::~CacheableObject()
{
}

void QNetworkAccessCache::CacheableObject::setExpires(bool enable)
{
   expires = enable;
}

void QNetworkAccessCache::CacheableObject::setShareable(bool enable)
{
   shareable = enable;
}

QNetworkAccessCache::QNetworkAccessCache()
   : oldest(nullptr), newest(nullptr)
{
}

QNetworkAccessCache::~QNetworkAccessCache()
{
   clear();
}

void QNetworkAccessCache::clear()
{
   QHash<QByteArray, Node> hashCopy = hash;

   // clear the original hash
   hash.clear();

   // remove all entries
   for (auto &item : hashCopy) {
      item.object->key.clear();
      item.object->dispose();
   }

   // now delete
   hashCopy.clear();

   timer.stop();

   oldest = nullptr;
   newest = nullptr;
}

void QNetworkAccessCache::linkEntry(const QByteArray &key)
{
   auto it = hash.find(key);

   if (it == hash.end()) {
      return;
   }

   // Appends the newest entry entry given by @p key to the end of the linked list.
   Node *const node = &(it.value());

   Q_ASSERT(node != oldest && node != newest);
   Q_ASSERT(node->older == 0 && node->newer == 0);
   Q_ASSERT(node->useCount == 0);

   if (newest) {
      Q_ASSERT(newest->newer == 0);
      newest->newer = node;
      node->older   = newest;
   }

   if (! oldest) {
      // there are no entries, so this is the oldest one too
      oldest = node;
   }

   // use QDateTime::currentDateTimeUtc()
   node->timestamp = QDateTime::currentDateTime().addSecs(ExpiryTime).toUTC();
   newest = node;
}

/*!
    Removes the entry pointed by @p key from the linked list.
    Returns true if the entry removed was the oldest one.
 */
bool QNetworkAccessCache::unlinkEntry(const QByteArray &key)
{
   auto it = hash.find(key);
   if (it == hash.end()) {
      return false;
   }

   Node *const node = &it.value();

   bool wasOldest = false;
   if (node == oldest) {
      oldest = node->newer;
      wasOldest = true;
   }

   if (node == newest) {
      newest = node->older;
   }

   if (node->older) {
      node->older->newer = node->newer;
   }

   if (node->newer) {
      node->newer->older = node->older;
   }

   node->newer = nullptr;
   node->older = nullptr;

   return wasOldest;
}

void QNetworkAccessCache::updateTimer()
{
   timer.stop();

   if (! oldest) {
      return;
   }

   int interval = QDateTime::currentDateTime().secsTo(oldest->timestamp);

   if (interval <= 0) {
      interval = 0;
   } else {
      // round up the interval
      interval = (interval + 15) & ~16;
   }

   timer.start(interval * 1000, this);
}

bool QNetworkAccessCache::emitEntryReady(Node *node, QObject *target, const char *member)
{
   if (! connect(this, SIGNAL(entryReady(QNetworkAccessCache::CacheableObject *)), target,
                 member, Qt::QueuedConnection)) {
      return false;
   }

   emit entryReady(node->object);
   disconnect(SIGNAL(entryReady(QNetworkAccessCache::CacheableObject *)));

   return true;
}

void QNetworkAccessCache::timerEvent(QTimerEvent *)
{
   // expire old items
   const QDateTime now = QDateTime::currentDateTime();

   while (oldest && oldest->timestamp < now) {
      Node *next = oldest->newer;
      oldest->object->dispose();

      hash.remove(oldest->key); // oldest gets deleted
      oldest = next;
   }

   // fixup the list
   if (oldest) {
      oldest->older = 0;
   } else {
      newest = 0;
   }

   updateTimer();
}

void QNetworkAccessCache::addEntry(const QByteArray &key, CacheableObject *entry)
{
   Q_ASSERT(!key.isEmpty());

   if (unlinkEntry(key)) {
      updateTimer();
   }

   Node &node = hash[key];     // create the entry in the hash if it didn't exist
   if (node.useCount) {
      qWarning("QNetworkAccessCache::addEntry: Overriding active cache entry '%s'", key.constData());
   }

   if (node.object) {
      node.object->dispose();
   }

   node.object = entry;
   node.object->key = key;
   node.key = key;
   node.useCount = 1;
}

bool QNetworkAccessCache::hasEntry(const QByteArray &key) const
{
   return hash.contains(key);
}

bool QNetworkAccessCache::requestEntry(const QByteArray &key, QObject *target, const char *member)
{
   auto it = hash.find(key);
   if (it == hash.end()) {
      return false;   // no such entry
   }

   Node *node = &it.value();

   if (node->useCount > 0 && !node->object->shareable) {
      // object is not shareable and is in use
      // queue for later use
      Q_ASSERT(node->older == 0 && node->newer == 0);

      Receiver receiver;
      receiver.object = target;
      receiver.member = member;
      node->receiverQueue.enqueue(receiver);

      // request queued
      return true;

   } else {
      // node not in use or is shareable
      if (unlinkEntry(key)) {
         updateTimer();
      }

      ++node->useCount;
      return emitEntryReady(node, target, member);
   }
}

QNetworkAccessCache::CacheableObject *QNetworkAccessCache::requestEntryNow(const QByteArray &key)
{
   auto it = hash.find(key);
   if (it == hash.end()) {
      return nullptr;
   }

   if (it->useCount > 0) {
      if (it->object->shareable) {
         ++it->useCount;
         return it->object;
      }

      // object in use and not shareable
      return nullptr;
   }

   // entry not in use, let the caller have it
   bool wasOldest = unlinkEntry(key);
   ++it->useCount;

   if (wasOldest) {
      updateTimer();
   }

   return it->object;
}

void QNetworkAccessCache::releaseEntry(const QByteArray &key)
{
   auto it = hash.find(key);
   if (it == hash.end()) {
      qWarning("QNetworkAccessCache::releaseEntry: Trying to release key '%s' which is not in cache", key.constData());
      return;
   }

   Node *node = &it.value();
   Q_ASSERT(node->useCount > 0);

   // are there other objects waiting?
   if (! node->receiverQueue.isEmpty()) {
      // queue another activation
      Receiver receiver;

      do {
         receiver = node->receiverQueue.dequeue();
      } while (receiver.object.isNull() && ! node->receiverQueue.isEmpty());

      if (! receiver.object.isNull()) {
         emitEntryReady(node, receiver.object, receiver.member);
         return;
      }
   }

   if (!--node->useCount) {
      // no objects waiting; add it back to the expiry list
      if (node->object->expires) {
         linkEntry(key);
      }

      if (oldest == node) {
         updateTimer();
      }
   }
}

void QNetworkAccessCache::removeEntry(const QByteArray &key)
{
   auto it = hash.find(key);

   if (it == hash.end()) {
      qWarning("QNetworkAccessCache::removeEntry: Trying to remove key '%s' which is not in cache", key.constData());
      return;
   }

   Node *node = &it.value();

   if (unlinkEntry(key)) {
      updateTimer();
   }

   if (node->useCount > 1) {
      qWarning("QNetworkAccessCache::removeEntry: Removing active cache entry '%s'", key.constData());
   }

   node->object->key.clear();
   hash.remove(node->key);
}

