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

#ifndef QFUTURE_H
#define QFUTURE_H

#include <qglobal.h>
#include <qfutureinterface.h>
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

   // internal
   explicit QFuture(QFutureInterface<T> *p)
      : d(*p)
   {
   }

   QFuture(const QFuture &other)
      : d(other.d)
   {
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

   bool isResultReadyAt(int index) const {
      return d.isResultReadyAt(index);
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
      using iterator_category = std::bidirectional_iterator_tag;
      using difference_type   = qptrdiff;
      using size_type         = difference_type;
      using value_type        = T;
      using pointer           = const T *;
      using reference         = const T &;

      const_iterator() {
      }

      const_iterator(QFuture const *const _future, int _index) : future(_future), index(_index) {
      }

      const_iterator(const const_iterator &other) : future(other.future), index(other.index)  {
      }

      const_iterator &operator=(const const_iterator &other) {
         future = other.future;
         index  = other.index;
         return *this;
      }

      const T &operator*() const {
         return future->d.resultReference(index);
      }

      const T *operator->() const {
         return future->d.resultPointer(index);
      }

      bool operator!=(const const_iterator &other) const {
         if (index == -1 && other.index == -1) {
            // comparing end != end?
            return false;
         }

         if (other.index == -1) {
            return (future->isRunning() || (index < future->resultCount()));
         }

         return (index != other.index);
      }

      bool operator==(const const_iterator &other) const {
         return !operator!=(other);
      }

      const_iterator &operator++() {
         ++index;
         return *this;
      }

      const_iterator operator++(int) {
         const_iterator r = *this;
         ++index;
         return r;
      }

      const_iterator &operator--() {
         --index;
         return *this;
      }

      const_iterator operator--(int) {
         const_iterator r = *this;
         --index;
         return r;
      }

      const_iterator operator+(size_type n) const {
         return const_iterator(future, index + n);
      }

      const_iterator operator-(size_type n) const {
         return const_iterator(future, index - n);
      }

      const_iterator &operator+=(size_type n) {
         index += n;
         return *this;
      }

      const_iterator &operator-=(size_type n) {
         index -= n;
         return *this;
      }

    private:
      QFuture const *future;
      int index;
   };

   friend class const_iterator;

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

 public:
   // Warning: the d pointer is not documented and is considered private.
   mutable QFutureInterface<T> d;

 private:
   friend class QFutureWatcher<T>;
};

template <typename T>
inline QFuture<T> &QFuture<T>::operator=(const QFuture<T> &other)
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
   using const_iterator = typename QFuture<T>::const_iterator;

   QFuture<T> c;
   const_iterator i;

 public:
   QFutureIterator(const QFuture<T> &future)
      : c(future), i(c.constBegin())
   { }

   QFutureIterator &operator=(const QFuture<T> &future) {
      c = future;
      i = c.constBegin();
      return *this;
   }

   void toFront() {
      i = c.constBegin();
   }

   void toBack() {
      i = c.constEnd();
   }

   bool hasNext() const {
      return i != c.constEnd();
   }

   const T &next() {
      return *i++;
   }

   const T &peekNext() const {
      return *i;
   }

   bool hasPrevious() const {
      return i != c.constBegin();
   }

   const T &previous() {
      return *--i;
   }

   const T &peekPrevious() const {
      const_iterator p = i;
      return *--p;
   }

   bool findNext(const T &value)  {
      while (i != c.constEnd()) {
         if (*i++ == value) {
            return true;
         }
      }

      return false;
   }

   bool findPrevious(const T &value)   {
      while (i != c.constBegin()) {
         if (*(--i) == value)  {
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
      : d(QFutureInterface<void>::canceledResult())
   {
   }

   // internal
   explicit QFuture(QFutureInterfaceBase *p)
      : d(*p)
   {
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