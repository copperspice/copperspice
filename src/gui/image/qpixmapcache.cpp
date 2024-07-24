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

#include <qpixmapcache.h>

#include <qdebug.h>
#include <qobject.h>

#include <qpixmapcache_p.h>

static int CacheLimit = 10240;    // 10 MB cache limit for desktop

QPixmapCache::Key::Key()
   : d(nullptr)
{
}

QPixmapCache::Key::Key(const Key &other)
{
   if (other.d) {
      ++(other.d->ref);
   }
   d = other.d;
}

QPixmapCache::Key::~Key()
{
   if (d && --(d->ref) == 0) {
      delete d;
   }
}

bool QPixmapCache::Key::operator ==(const Key &key) const
{
   return (d == key.d);
}

QPixmapCache::Key &QPixmapCache::Key::operator =(const Key &other)
{
   if (d != other.d) {
      if (other.d) {
         ++(other.d->ref);
      }

      if (d && --(d->ref) == 0) {
         delete d;
      }

      d = other.d;
   }

   return *this;
}

class QPMCache : public QObject, public QCache<QPixmapCache::Key, QPixmapCacheEntry>
{
   GUI_CS_OBJECT(QPMCache)

 public:
   QPMCache();
   ~QPMCache();

   void timerEvent(QTimerEvent *) override;
   bool insert(const QString &key, const QPixmap &pixmap, int cost);
   QPixmapCache::Key insert(const QPixmap &pixmap, int cost);
   bool replace(const QPixmapCache::Key &key, const QPixmap &pixmap, int cost);
   bool remove(const QString &key);
   bool remove(const QPixmapCache::Key &key);

   void resizeKeyArray(int size);
   QPixmapCache::Key createKey();
   void releaseKey(const QPixmapCache::Key &key);
   void clear();

   QPixmap *object(const QString &key) const;
   QPixmap *object(const QPixmapCache::Key &key) const;

   static inline QPixmapCache::KeyData *get(const QPixmapCache::Key &key) {
      return key.d;
   }

   static QPixmapCache::KeyData *getKeyData(QPixmapCache::Key *key);

   bool flushDetachedPixmaps(bool nt);

 private:
   static constexpr const int TimeSoon  = 10000;
   static constexpr const int TimeFlush = 30000;

   int *keyArray;
   int theid;
   int ps;
   int keyArraySize;
   int freeKey;
   QHash<QString, QPixmapCache::Key> cacheKeys;
   bool t;
};

static QPMCache *pm_cache()
{
   static QPMCache retval;
   return &retval;
}

uint qHash(const QPixmapCache::Key &k)
{
   return qHash(QPMCache::get(k)->key);
}

QPMCache::QPMCache()
   : QObject(nullptr), QCache<QPixmapCache::Key, QPixmapCacheEntry>(CacheLimit * 1024),
     keyArray(nullptr), theid(0), ps(0), keyArraySize(0), freeKey(0), t(false)
{
}

QPMCache::~QPMCache()
{
   clear();
   free(keyArray);
}

bool QPMCache::flushDetachedPixmaps(bool nt)
{
   int mc = maxCost();
   setMaxCost(nt ? totalCost() * 3 / 4 : totalCost() - 1);
   setMaxCost(mc);
   ps = totalCost();

   bool any = false;
   QHash<QString, QPixmapCache::Key>::iterator it = cacheKeys.begin();
   while (it != cacheKeys.end()) {
      if (!contains(it.value())) {
         releaseKey(it.value());
         it = cacheKeys.erase(it);
         any = true;
      } else {
         ++it;
      }
   }

   return any;
}

void QPMCache::timerEvent(QTimerEvent *)
{
   bool nt = totalCost() == ps;
   if (!flushDetachedPixmaps(nt)) {
      killTimer(theid);
      theid = 0;

   } else if (nt != t) {
      killTimer(theid);
      theid = startTimer(nt ? TimeSoon : TimeFlush);
      t = nt;
   }
}

QPixmap *QPMCache::object(const QString &key) const
{
   QPixmapCache::Key cacheKey = cacheKeys.value(key);
   if (!cacheKey.d || !cacheKey.d->isValid) {
      const_cast<QPMCache *>(this)->cacheKeys.remove(key);
      return nullptr;
   }

   QPixmap *ptr = QCache<QPixmapCache::Key, QPixmapCacheEntry>::object(cacheKey);
   //We didn't find the pixmap in the cache, the key is not valid anymore
   if (!ptr) {
      const_cast<QPMCache *>(this)->cacheKeys.remove(key);
   }

   return ptr;
}

QPixmap *QPMCache::object(const QPixmapCache::Key &key) const
{
   Q_ASSERT(key.d->isValid);
   QPixmap *ptr = QCache<QPixmapCache::Key, QPixmapCacheEntry>::object(key);
   //We didn't find the pixmap in the cache, the key is not valid anymore
   if (!ptr) {
      const_cast<QPMCache *>(this)->releaseKey(key);
   }
   return ptr;
}

bool QPMCache::insert(const QString &key, const QPixmap &pixmap, int cost)
{
   QPixmapCache::Key cacheKey;
   QPixmapCache::Key oldCacheKey = cacheKeys.value(key);
   //If for the same key we add already a pixmap we should delete it
   if (oldCacheKey.d) {
      QCache<QPixmapCache::Key, QPixmapCacheEntry>::remove(oldCacheKey);
      cacheKeys.remove(key);
   }

   //we create a new key the old one has been removed
   cacheKey = createKey();

   bool success = QCache<QPixmapCache::Key, QPixmapCacheEntry>::insert(cacheKey, new QPixmapCacheEntry(cacheKey, pixmap),
         cost);
   if (success) {
      cacheKeys.insert(key, cacheKey);
      if (!theid) {
         theid = startTimer(TimeFlush);
         t = false;
      }
   } else {
      //Insertion failed we released the new allocated key
      releaseKey(cacheKey);
   }
   return success;
}

QPixmapCache::Key QPMCache::insert(const QPixmap &pixmap, int cost)
{
   QPixmapCache::Key cacheKey = createKey();
   bool success = QCache<QPixmapCache::Key, QPixmapCacheEntry>::insert(cacheKey, new QPixmapCacheEntry(cacheKey, pixmap), cost);

   if (success) {
      if (! theid) {
         theid = startTimer(TimeFlush);
         t = false;
      }

   } else {
      // Insertion failed we released the key and return an invalid one
      releaseKey(cacheKey);
   }

   return cacheKey;
}

bool QPMCache::replace(const QPixmapCache::Key &key, const QPixmap &pixmap, int cost)
{
   Q_ASSERT(key.d->isValid);
   //If for the same key we had already an entry so we should delete the pixmap and use the new one
   QCache<QPixmapCache::Key, QPixmapCacheEntry>::remove(key);

   QPixmapCache::Key cacheKey = createKey();

   bool success = QCache<QPixmapCache::Key, QPixmapCacheEntry>::insert(cacheKey, new QPixmapCacheEntry(cacheKey, pixmap), cost);

   if (success) {
      if (! theid) {
         theid = startTimer(TimeFlush);
         t = false;
      }

      const_cast<QPixmapCache::Key &>(key) = cacheKey;

   } else {
      //Insertion failed we released the key
      releaseKey(cacheKey);
   }

   return success;
}

bool QPMCache::remove(const QString &key)
{
   QPixmapCache::Key cacheKey = cacheKeys.value(key);

   // the key was not in the cache
   if (! cacheKey.d) {
      return false;
   }

   cacheKeys.remove(key);
   return QCache<QPixmapCache::Key, QPixmapCacheEntry>::remove(cacheKey);
}

bool QPMCache::remove(const QPixmapCache::Key &key)
{
   return QCache<QPixmapCache::Key, QPixmapCacheEntry>::remove(key);
}

void QPMCache::resizeKeyArray(int size)
{
   if (size <= keyArraySize || size == 0) {
      return;
   }
   keyArray = q_check_ptr(reinterpret_cast<int *>(realloc(keyArray,
               size * sizeof(int))));
   for (int i = keyArraySize; i != size; ++i) {
      keyArray[i] = i + 1;
   }
   keyArraySize = size;
}

QPixmapCache::Key QPMCache::createKey()
{
   if (freeKey == keyArraySize) {
      resizeKeyArray(keyArraySize ? keyArraySize << 1 : 2);
   }
   int id = freeKey;
   freeKey = keyArray[id];
   QPixmapCache::Key key;
   QPixmapCache::KeyData *d = QPMCache::getKeyData(&key);
   d->key = ++id;
   return key;
}

void QPMCache::releaseKey(const QPixmapCache::Key &key)
{
   if (key.d->key > keyArraySize || key.d->key <= 0) {
      return;
   }
   key.d->key--;
   keyArray[key.d->key] = freeKey;
   freeKey = key.d->key;
   key.d->isValid = false;
   key.d->key = 0;
}

void QPMCache::clear()
{
   free(keyArray);
   keyArray     = nullptr;
   freeKey      = 0;
   keyArraySize = 0;

   //Mark all keys as invalid
   QList<QPixmapCache::Key> keys = QCache<QPixmapCache::Key, QPixmapCacheEntry>::keys();
   for (int i = 0; i < keys.size(); ++i) {
      keys.at(i).d->isValid = false;
   }

   QCache<QPixmapCache::Key, QPixmapCacheEntry>::clear();
}

QPixmapCache::KeyData *QPMCache::getKeyData(QPixmapCache::Key *key)
{
   if (!key->d) {
      key->d = new QPixmapCache::KeyData;
   }
   return key->d;
}

int q_QPixmapCache_keyHashSize()
{
   return pm_cache()->size();
}

QPixmapCacheEntry::~QPixmapCacheEntry()
{
   pm_cache()->releaseKey(key);
}

QPixmap *QPixmapCache::find(const QString &key)
{
   return pm_cache()->object(key);
}

bool QPixmapCache::find(const QString &key, QPixmap &pixmap)
{
   return find(key, &pixmap);
}

bool QPixmapCache::find(const QString &key, QPixmap *pixmap)
{
   QPixmap *ptr = pm_cache()->object(key);
   if (ptr && pixmap) {
      *pixmap = *ptr;
   }
   return ptr != nullptr;
}

bool QPixmapCache::find(const Key &key, QPixmap *pixmap)
{
   //The key is not valid anymore, a flush happened before probably
   if (!key.d || !key.d->isValid) {
      return false;
   }

   QPixmap *ptr = pm_cache()->object(key);
   if (ptr && pixmap) {
      *pixmap = *ptr;
   }
   return ptr != nullptr;
}

bool QPixmapCache::insert(const QString &key, const QPixmap &pixmap)
{
   return pm_cache()->insert(key, pixmap, pixmap.width() * pixmap.height() * pixmap.depth() / 8);
}

QPixmapCache::Key QPixmapCache::insert(const QPixmap &pixmap)
{
   return pm_cache()->insert(pixmap, pixmap.width() * pixmap.height() * pixmap.depth() / 8);
}

bool QPixmapCache::replace(const Key &key, const QPixmap &pixmap)
{
   //The key is not valid anymore, a flush happened before probably
   if (!key.d || !key.d->isValid) {
      return false;
   }
   return pm_cache()->replace(key, pixmap, pixmap.width() * pixmap.height() * pixmap.depth() / 8);
}

int QPixmapCache::cacheLimit()
{
   return CacheLimit;
}

void QPixmapCache::setCacheLimit(int n)
{
   CacheLimit = n;
   pm_cache()->setMaxCost(1024 * CacheLimit);
}

void QPixmapCache::remove(const QString &key)
{
   pm_cache()->remove(key);
}

void QPixmapCache::remove(const Key &key)
{
   // key is not valid anymore, a flush happened before
   if (!key.d || !key.d->isValid) {
      return;
   }
   pm_cache()->remove(key);
}

void QPixmapCache::clear()
{
  try {
      pm_cache()->clear();

   } catch (const std::bad_alloc &) {
      // if we ran out of memory during pm_cache(), it's no leak,
      // so just ignore it.
   }
}

