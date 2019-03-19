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

#define Q_TEST_QPIXMAPCACHE

#include <qpixmapcache.h>
#include <qobject.h>
#include <qdebug.h>
#include <qpixmapcache_p.h>

#include <stdlib.h>

QT_BEGIN_NAMESPACE

#if defined(Q_WS_QWS)
static int cache_limit = 2048;     // 2048 KB cache limit for embedded
#else
static int cache_limit = 10240;    // 10 MB cache limit for desktop
#endif

QPixmapCache::Key::Key() : d(0)
{
}

QPixmapCache::Key::Key(const Key &other)
{
   if (other.d) {
      ++(other.d->ref);
   }
   d = other.d;
}

/*!
    Destroys the key.
*/
QPixmapCache::Key::~Key()
{
   if (d && --(d->ref) == 0) {
      delete d;
   }
}

/*!
    \internal

    Returns true if this key is the same as the given \a key; otherwise returns
    false.
*/
bool QPixmapCache::Key::operator ==(const Key &key) const
{
   return (d == key.d);
}

/*!
    \fn bool QPixmapCache::Key::operator !=(const Key &key) const
    \internal
*/

/*!
    \internal
*/
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

   QList< QPair<QString, QPixmap> > allPixmaps() const;
   bool flushDetachedPixmaps(bool nt);

 private:
   enum { soon_time = 10000, flush_time = 30000 };
   int *keyArray;
   int theid;
   int ps;
   int keyArraySize;
   int freeKey;
   QHash<QString, QPixmapCache::Key> cacheKeys;
   bool t;
};

uint qHash(const QPixmapCache::Key &k)
{
   return qHash(QPMCache::get(k)->key);
}

QPMCache::QPMCache()
   : QObject(0),
     QCache<QPixmapCache::Key, QPixmapCacheEntry>(cache_limit * 1024),
     keyArray(0), theid(0), ps(0), keyArraySize(0), freeKey(0), t(false)
{
}
QPMCache::~QPMCache()
{
   clear();
   free(keyArray);
}

/*
  This is supposed to cut the cache size down by about 25% in a
  minute once the application becomes idle, to let any inserted pixmap
  remain in the cache for some time before it becomes a candidate for
  cleaning-up, and to not cut down the size of the cache while the
  cache is in active use.

  When the last detached pixmap has been deleted from the cache, kill the
  timer so Qt won't keep the CPU from going into sleep mode. Currently
  the timer is not restarted when the pixmap becomes unused, but it does
  restart once something else is added (i.e. the cache space is actually needed).

  Returns true if any were removed.
*/
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
      theid = startTimer(nt ? soon_time : flush_time);
      t = nt;
   }
}


QPixmap *QPMCache::object(const QString &key) const
{
   QPixmapCache::Key cacheKey = cacheKeys.value(key);
   if (!cacheKey.d || !cacheKey.d->isValid) {
      const_cast<QPMCache *>(this)->cacheKeys.remove(key);
      return 0;
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
         theid = startTimer(flush_time);
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
   bool success = QCache<QPixmapCache::Key, QPixmapCacheEntry>::insert(cacheKey, new QPixmapCacheEntry(cacheKey, pixmap),
                  cost);
   if (success) {
      if (!theid) {
         theid = startTimer(flush_time);
         t = false;
      }
   } else {
      //Insertion failed we released the key and return an invalid one
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

   bool success = QCache<QPixmapCache::Key, QPixmapCacheEntry>::insert(cacheKey, new QPixmapCacheEntry(cacheKey, pixmap),
                  cost);
   if (success) {
      if (!theid) {
         theid = startTimer(flush_time);
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
   //The key was not in the cache
   if (!cacheKey.d) {
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
   keyArray = 0;
   freeKey = 0;
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

QList< QPair<QString, QPixmap> > QPMCache::allPixmaps() const
{
   QList< QPair<QString, QPixmap> > r;
   QHash<QString, QPixmapCache::Key>::const_iterator it = cacheKeys.begin();
   while (it != cacheKeys.end()) {
      QPixmap *ptr = QCache<QPixmapCache::Key, QPixmapCacheEntry>::object(it.value());
      if (ptr) {
         r.append(QPair<QString, QPixmap>(it.key(), *ptr));
      }
      ++it;
   }
   return r;
}


Q_GLOBAL_STATIC(QPMCache, pm_cache)

int q_QPixmapCache_keyHashSize()
{
   return pm_cache()->size();
}

QPixmapCacheEntry::~QPixmapCacheEntry()
{
   pm_cache()->releaseKey(key);
}

/*!
    \obsolete
    \overload

    Returns the pixmap associated with the \a key in the cache, or
    null if there is no such pixmap.

    \warning If valid, you should copy the pixmap immediately (this is
    fast). Subsequent insertions into the cache could cause the
    pointer to become invalid. For this reason, we recommend you use
    bool find(const QString&, QPixmap*) instead.

    Example:
    \snippet doc/src/snippets/code/src_gui_image_qpixmapcache.cpp 0
*/

QPixmap *QPixmapCache::find(const QString &key)
{
   return pm_cache()->object(key);
}


/*!
    \obsolete

    Use bool find(const QString&, QPixmap*) instead.
*/

bool QPixmapCache::find(const QString &key, QPixmap &pixmap)
{
   return find(key, &pixmap);
}

/*!
    Looks for a cached pixmap associated with the given \a key in the cache.
    If the pixmap is found, the function sets \a pixmap to that pixmap and
    returns true; otherwise it leaves \a pixmap alone and returns false.

    \since 4.6

    Example:
    \snippet doc/src/snippets/code/src_gui_image_qpixmapcache.cpp 1
*/

bool QPixmapCache::find(const QString &key, QPixmap *pixmap)
{
   QPixmap *ptr = pm_cache()->object(key);
   if (ptr && pixmap) {
      *pixmap = *ptr;
   }
   return ptr != 0;
}

/*!
    Looks for a cached pixmap associated with the given \a key in the cache.
    If the pixmap is found, the function sets \a pixmap to that pixmap and
    returns true; otherwise it leaves \a pixmap alone and returns false. If
    the pixmap is not found, it means that the \a key is no longer valid,
    so it will be released for the next insertion.

    \since 4.6
*/
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
   return ptr != 0;
}

/*!
    Inserts a copy of the pixmap \a pixmap associated with the \a key into
    the cache.

    All pixmaps inserted by the Qt library have a key starting with
    "$qt", so your own pixmap keys should never begin "$qt".

    When a pixmap is inserted and the cache is about to exceed its
    limit, it removes pixmaps until there is enough room for the
    pixmap to be inserted.

    The oldest pixmaps (least recently accessed in the cache) are
    deleted when more space is needed.

    The function returns true if the object was inserted into the
    cache; otherwise it returns false.

    \sa setCacheLimit()
*/

bool QPixmapCache::insert(const QString &key, const QPixmap &pixmap)
{
   return pm_cache()->insert(key, pixmap, pixmap.width() * pixmap.height() * pixmap.depth() / 8);
}

/*!
    Inserts a copy of the given \a pixmap into the cache and returns a key
    that can be used to retrieve it.

    When a pixmap is inserted and the cache is about to exceed its
    limit, it removes pixmaps until there is enough room for the
    pixmap to be inserted.

    The oldest pixmaps (least recently accessed in the cache) are
    deleted when more space is needed.

    \sa setCacheLimit(), replace()

    \since 4.6
*/
QPixmapCache::Key QPixmapCache::insert(const QPixmap &pixmap)
{
   return pm_cache()->insert(pixmap, pixmap.width() * pixmap.height() * pixmap.depth() / 8);
}

/*!
    Replaces the pixmap associated with the given \a key with the \a pixmap
    specified. Returns true if the \a pixmap has been correctly inserted into
    the cache; otherwise returns false.

    \sa setCacheLimit(), insert()

    \since 4.6
*/
bool QPixmapCache::replace(const Key &key, const QPixmap &pixmap)
{
   //The key is not valid anymore, a flush happened before probably
   if (!key.d || !key.d->isValid) {
      return false;
   }
   return pm_cache()->replace(key, pixmap, pixmap.width() * pixmap.height() * pixmap.depth() / 8);
}

/*!
    Returns the cache limit (in kilobytes).

    The default cache limit is 2048 KB on embedded platforms, 10240 KB on
    desktop platforms.

    \sa setCacheLimit()
*/

int QPixmapCache::cacheLimit()
{
   return cache_limit;
}

/*!
    Sets the cache limit to \a n kilobytes.

    The default setting is 2048 KB on embedded platforms, 10240 KB on
    desktop platforms.

    \sa cacheLimit()
*/

void QPixmapCache::setCacheLimit(int n)
{
   cache_limit = n;
   pm_cache()->setMaxCost(1024 * cache_limit);
}

/*!
  Removes the pixmap associated with \a key from the cache.
*/
void QPixmapCache::remove(const QString &key)
{
   pm_cache()->remove(key);
}

/*!
  Removes the pixmap associated with \a key from the cache and releases
  the key for a future insertion.

  \since 4.6
*/
void QPixmapCache::remove(const Key &key)
{
   //The key is not valid anymore, a flush happened before probably
   if (!key.d || !key.d->isValid) {
      return;
   }
   pm_cache()->remove(key);
}

/*!
    Removes all pixmaps from the cache.
*/

void QPixmapCache::clear()
{
   QT_TRY {
      pm_cache()->clear();
   } QT_CATCH(const std::bad_alloc &) {
      // if we ran out of memory during pm_cache(), it's no leak,
      // so just ignore it.
   }
}

void QPixmapCache::flushDetachedPixmaps()
{
   pm_cache()->flushDetachedPixmaps(true);
}

int QPixmapCache::totalUsed()
{
   return (pm_cache()->totalCost() + 1023) / 1024;
}

QList< QPair<QString, QPixmap> > QPixmapCache::allPixmaps()
{
   return pm_cache()->allPixmaps();
}

QT_END_NAMESPACE
