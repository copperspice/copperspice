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

#ifndef QABSTRACTXMLFORWARDITERATOR_P_H
#define QABSTRACTXMLFORWARDITERATOR_P_H

#include <qcontainerfwd.h>
#include <qlist.h>
#include <qshareddata.h>
#include <qstring.h>
#include <qvector.h>

/* In this file we in some cases do not use QAbstractXmlForwardIterator's Ptr typedef.
 * This is a compiler workaround for MS VS 6.0. */

template <typename T>
class QAbstractXmlForwardIterator;

class QAbstractXmlForwardIteratorPrivate;

template <typename T>
inline bool qIsForwardIteratorEnd(const T &unit)
{
   return !unit;
}

template <>
inline bool qIsForwardIteratorEnd(const QString &unit)
{
   return unit.isEmpty();
}

template <typename T>
class QAbstractXmlForwardIterator : public QSharedData
{
 public:
   typedef QExplicitlySharedDataPointer<QAbstractXmlForwardIterator<T> > Ptr;
   typedef QList<QExplicitlySharedDataPointer<QAbstractXmlForwardIterator<T> > > List;
   typedef QVector<QExplicitlySharedDataPointer<QAbstractXmlForwardIterator<T> > > Vector;

   inline QAbstractXmlForwardIterator()
      : d_ptr(nullptr)
   {
   }

   virtual ~QAbstractXmlForwardIterator() {}

   virtual T next() = 0;
   virtual T current() const = 0;

   virtual qint64 position() const = 0;

   virtual typename QAbstractXmlForwardIterator<T>::Ptr toReversed();
   virtual QList<T> toList();
   virtual typename QAbstractXmlForwardIterator<T>::Ptr copy() const;
   virtual T last();
   virtual bool isEmpty();
   virtual qint64 count();
   virtual qint64 sizeHint() const;

 private:
   QAbstractXmlForwardIterator<T>(const QAbstractXmlForwardIterator<T> &) = delete;
   QAbstractXmlForwardIterator<T> &operator=(const QAbstractXmlForwardIterator<T> &) = delete;

   QAbstractXmlForwardIteratorPrivate *d_ptr; /* Currently not used. */
};

/* The namespace QPatternist and its members are internal, not part of the public API, and
 * unsupported. Using them leads to undefined behavior. */
namespace QPatternist {
class DeduplicateIterator;

template<typename InputType, typename OutputType, typename Derived, typename ListType = QList<InputType> >
class ListIteratorPlatform : public QAbstractXmlForwardIterator<OutputType>
{
   /* This declaration is a workaround for a set of GCC versions on OS X,
    * amongst others powerpc-apple-darwin8-gcc-4.0.1 (GCC) 4.0.1. In
    * DeduplicateIterator, it fails to see the protected inheritance. */
   friend class DeduplicateIterator;

 public:
   OutputType next() override {
      if (m_position == -1) {
         return OutputType();
      }

      if (m_position == m_list.count()) {
         m_position = -1;
         m_current = OutputType();
         return OutputType();
      }

      m_current = static_cast<const Derived *>(this)->inputToOutputItem(m_list.at(m_position));
      ++m_position;
      return m_current;
   }

   OutputType current() const override {
      return m_current;
   }

   qint64 position() const override {
      return m_position;
   }

   qint64 count() override {
      return m_list.count();
   }

   typename QAbstractXmlForwardIterator<OutputType>::Ptr copy() const  override {
      return QExplicitlySharedDataPointer<QAbstractXmlForwardIterator<OutputType> >(new
             ListIteratorPlatform<InputType, OutputType, Derived, ListType>(m_list));
   }

 protected:
   inline ListIteratorPlatform(const ListType &list) : m_list(list), m_position(0) { }

   const ListType  m_list;
   qint64          m_position;
   OutputType      m_current;
};

template<typename T, typename ListType = QList<T> >
class ListIterator : public ListIteratorPlatform<T, T, ListIterator<T, ListType>, ListType>
{
   /*
    * This declaration is needed for MSVC 2005, 14.00.50727.42 for 80x86.
    */
   friend class IteratorVector;

   using ListIteratorPlatform<T, T, ListIterator<T, ListType>, ListType>::m_list;

   static QVector<T> toVector(const QVector<T> &vector) {
      return vector;
   }

   static QVector<T> toVector(const QList<T> &list) {
      return list.toVector();
   }

   static QList<T> toList(const QVector<T> &vector) {
      return vector.toList();
   }

   static QList<T> toList(const QList<T> &list) {
      return list;
   }

 public:
   inline ListIterator(const ListType &list) : ListIteratorPlatform<T, T, ListIterator<T, ListType>, ListType>(list) {
   }

   QList<T> toList() override {
      return toList(m_list);
   }

   virtual QVector<T> toVector() {
      return toVector(m_list);
   }

 private:
   inline const T &inputToOutputItem(const T &inputType) const {
      return inputType;
   }
   friend class ListIteratorPlatform<T, T, ListIterator<T, ListType>, ListType>;

   // needed for MSVC 2005
   friend class DeduplicateIterator;
};

template<typename T>
inline typename QAbstractXmlForwardIterator<T>::Ptr
makeListIterator(const QList<T> &list)
{
   return typename ListIterator<T>::Ptr(new ListIterator<T>(list));
}

template<typename T>
inline typename QAbstractXmlForwardIterator<T>::Ptr
makeVectorIterator(const QVector<T> &vector)
{
   return typename ListIterator<T, QVector<T> >::Ptr(new ListIterator<T, QVector<T> >(vector));
}

}

template<typename T>
QList<T> QAbstractXmlForwardIterator<T>::toList()
{
   QList<T> result;
   T item(next());

   while (!qIsForwardIteratorEnd(item)) {
      result.append(item);
      item = next();
   }

   return result;
}

template<typename T>
qint64 QAbstractXmlForwardIterator<T>::count()
{
   qint64 retval = 0;

   while (!qIsForwardIteratorEnd(next())) {
      ++retval;
   }

   return retval;
}

template<typename T>
typename QAbstractXmlForwardIterator<T>::Ptr QAbstractXmlForwardIterator<T>::toReversed()
{
   T item(next());
   QList<T> result;

   while (!qIsForwardIteratorEnd(item)) {
      result.prepend(item);
      item = next();
   }

   return QExplicitlySharedDataPointer<QAbstractXmlForwardIterator<T> >(new QPatternist::ListIterator<T>(result));
}

template<typename T>
T QAbstractXmlForwardIterator<T>::last()
{
   T item(next());

   while (!qIsForwardIteratorEnd(item)) {
      item = next();
   }

   return item;
}

template<typename T>
typename QAbstractXmlForwardIterator<T>::Ptr QAbstractXmlForwardIterator<T>::copy() const
{
   Q_ASSERT_X(false, Q_FUNC_INFO, "This function is internal, unsupported, and should never be called.");
   return typename QAbstractXmlForwardIterator<T>::Ptr();
}

template<typename T>
bool QAbstractXmlForwardIterator<T>::isEmpty()
{
   return qIsForwardIteratorEnd(next());
}

template<typename T>
qint64 QAbstractXmlForwardIterator<T>::sizeHint() const
{
   Q_ASSERT_X(false, Q_FUNC_INFO, "This function is internal, unsupported, and should never be called.");
   return -1;
}

#endif
