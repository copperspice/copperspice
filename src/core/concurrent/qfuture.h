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

#ifndef QFUTURE_H
#define QFUTURE_H

#include <qglobal.h>
#include <qfutureinterface.h>
#include <qstring.h>
#include <qtconcurrentcompilertest.h>

template <typename T>
class QFutureWatcher;

template <>
class QFutureWatcher<void>;

template <typename T>
class QFuture
{
 public:
   QFuture()
      : d(QFutureInterface<T>::canceledResult()) {
   }

   explicit QFuture(QFutureInterface<T> *p)
      // internal
      : d(*p) {
   }

   QFuture(const QFuture &other)
      : d(other.d) {
   }

   ~QFuture() {
   }

   inline QFuture<T> &operator=(const QFuture &other);

   bool operator==(const QFuture &other) const {
      return (d == other.d);
   }

   bool operator!=(const QFuture &other) const {
      return (d != other.d);
   }

   void cancel() {
      d.cancel();
   }
   bool isCanceled() const {
      return d.isCanceled();
   }

   void setPaused(bool paused) {
      d.setPaused(paused);
   }
   bool isPaused() const {
      return d.isPaused();
   }
   void pause() {
      setPaused(true);
   }
   void resume() {
      setPaused(false);
   }
   void togglePaused() {
      d.togglePaused();
   }

   bool isStarted() const {
      return d.isStarted();
   }
   bool isFinished() const {
      return d.isFinished();
   }
   bool isRunning() const {
      return d.isRunning();
   }

   int resultCount() const {
      return d.resultCount();
   }
   int progressValue() const {
      return d.progressValue();
   }
   int progressMinimum() const {
      return d.progressMinimum();
   }
   int progressMaximum() const {
      return d.progressMaximum();
   }
   QString progressText() const {
      return d.progressText();
   }
   void waitForFinished() {
      d.waitForFinished();
   }

   inline T result() const;
   inline T resultAt(int index) const;
   bool isResultReadyAt(int resultIndex) const {
      return d.isResultReadyAt(resultIndex);
   }

   operator T() const {
      return result();
   }
   QList<T> results() const {
      return d.results();
   }

   class const_iterator
   {
    public:
      typedef std::bidirectional_iterator_tag iterator_category;
      typedef qptrdiff difference_type;
      typedef T value_type;
      typedef const T *pointer;
      typedef const T &reference;

      inline const_iterator() {}
      inline const_iterator(QFuture const *const _future, int _index) : future(_future), index(_index) {}

      inline const_iterator(const const_iterator &other) : future(other.future), index(other.index)  {}

      inline const_iterator &operator=(const const_iterator &other) {
         future = other.future;
         index  = other.index;
         return *this;
      }

      inline const T &operator*() const {
         return future->d.resultReference(index);
      }

      inline const T *operator->() const {
         return future->d.resultPointer(index);
      }

      inline bool operator!=(const const_iterator &other) const {
         if (index == -1 && other.index == -1) {
            // comparing end != end?
            return false;
         }

         if (other.index == -1) {
            return (future->isRunning() || (index < future->resultCount()));
         }
         return (index != other.index);
      }

      inline bool operator==(const const_iterator &other) const {
         return !operator!=(other);
      }

      inline const_iterator &operator++() {
         ++index;
         return *this;
      }


      inline const_iterator operator++(int) {
         const_iterator r = *this;
         ++index;
         return r;
      }

      inline const_iterator &operator--() {
         --index;
         return *this;
      }

      inline const_iterator operator--(int) {
         const_iterator r = *this;
         --index;
         return r;
      }

      inline const_iterator operator+(int n) const {
         return const_iterator(future, index + n);
      }

      inline const_iterator operator-(int n) const {
         return const_iterator(future, index - n);
      }

      inline const_iterator &operator+=(int n) {
         index += n;
         return *this;
      }

      inline const_iterator &operator-=(int n) {
         index -= n;
         return *this;
      }

    private:
      QFuture const *future;
      int index;
   };

   friend class const_iterator;
   typedef const_iterator ConstIterator;

   const_iterator begin() const {
      return  const_iterator(this, 0);
   }

   const_iterator constBegin() const {
      return  const_iterator(this, 0);
   }

   const_iterator end() const {
      return const_iterator(this, -1);
   }

   const_iterator constEnd() const {
      return const_iterator(this, -1);
   }

 private:
   friend class QFutureWatcher<T>;

 public:
   // Warning: the d pointer is not documented and is considered private.
   mutable QFutureInterface<T> d;
};

template <typename T>
inline QFuture<T> & QFuture<T>::operator=(const QFuture<T> &other)
{
   d = other.d;
   return *this;
}

template <typename T>
inline T QFuture<T>::result() const
{
   d.waitForResult(0);
   return d.resultReference(0);
}

template <typename T>
inline T QFuture<T>::resultAt(int index) const
{
   d.waitForResult(index);
   return d.resultReference(index);
}

template <typename T>
inline QFuture<T> QFutureInterface<T>::future()
{
   return QFuture<T>(this);
}

template <class T>
class QFutureIterator
{
   typedef typename QFuture<T>::const_iterator const_iterator;
   QFuture<T> c;
   const_iterator i;

   public:
      inline QFutureIterator(const QFuture<T> &container)
         : c(container), i(c.constBegin()) {}

      inline QFutureIterator &operator=(const QFuture<T> &container)
         { c = container; i = c.constBegin(); return *this; }

      inline void toFront() { i = c.constBegin(); }
      inline void toBack() { i = c.constEnd(); }
      inline bool hasNext() const { return i != c.constEnd(); }
      inline const T &next() { return *i++; }
      inline const T &peekNext() const { return *i; }
      inline bool hasPrevious() const { return i != c.constBegin(); }
      inline const T &previous() { return *--i; }
      inline const T &peekPrevious() const { const_iterator p = i; return *--p; }

      inline bool findNext(const T &t)  {
         while (i != c.constEnd()) {
            if (*i++ == t) {
               return true;
            }
         }
         return false;
      }

      inline bool findPrevious(const T &t)   {
         while (i != c.constBegin()) {
            if (*(--i) == t)  {
               return true;
            }
         }
         return false;
      }
};

template <>
class QFuture<void>
{
 public:
   QFuture()
      : d(QFutureInterface<void>::canceledResult()) {
   }
   explicit QFuture(QFutureInterfaceBase *p)
      // internal
      : d(*p) {
   }

   QFuture(const QFuture &other)
      : d(other.d) {
   }

   ~QFuture() {
   }

   QFuture<void> &operator=(const QFuture<void> &other);

   bool operator==(const QFuture &other) const {
      return (d == other.d);
   }

   bool operator!=(const QFuture &other) const {
      return (d != other.d);
   }


   template <typename T>
   QFuture(const QFuture<T> &other)
      : d(other.d) {
   }

   template <typename T>
   QFuture<void> &operator=(const QFuture<T> &other) {
      d = other.d;
      return *this;
   }

   void cancel() {
      d.cancel();
   }
   bool isCanceled() const {
      return d.isCanceled();
   }

   void setPaused(bool paused) {
      d.setPaused(paused);
   }
   bool isPaused() const {
      return d.isPaused();
   }
   void pause() {
      setPaused(true);
   }
   void resume() {
      setPaused(false);
   }
   void togglePaused() {
      d.togglePaused();
   }

   bool isStarted() const {
      return d.isStarted();
   }
   bool isFinished() const {
      return d.isFinished();
   }
   bool isRunning() const {
      return d.isRunning();
   }

   int resultCount() const {
      return d.resultCount();
   }
   int progressValue() const {
      return d.progressValue();
   }
   int progressMinimum() const {
      return d.progressMinimum();
   }
   int progressMaximum() const {
      return d.progressMaximum();
   }
   QString progressText() const {
      return d.progressText();
   }
   void waitForFinished() {
      d.waitForFinished();
   }

 private:
   friend class QFutureWatcher<void>;

   mutable QFutureInterfaceBase d;
};

inline QFuture<void> &QFuture<void>::operator=(const QFuture<void> &other)
{
   d = other.d;
   return *this;
}

inline QFuture<void> QFutureInterface<void>::future()
{
   return QFuture<void>(this);
}

template <typename T>
QFuture<void> qToVoidFuture(const QFuture<T> &future)
{
   return QFuture<void>(future.d);
}

#endif