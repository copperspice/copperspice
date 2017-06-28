/***********************************************************************
*
* Copyright (c) 2012-2017 Barbara Geller
* Copyright (c) 2012-2017 Ansel Sermersheim
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

#ifndef QSTRING8_H
#define QSTRING8_H

#include <cs_string.h>

#include <qchar32.h>
#include <qbytearray.h>

class QRegExp;
class QString8;

void cs_swapFunc(QString8 &a, QString8 &b);

class Q_CORE_EXPORT QString8 : public CsString::CsString
{
   public:
      using Iterator        = iterator;
      using ConstIterator   = const_iterator;

      using difference_type = ssize_t;
      using size_type       = ssize_t;
      using value_type      = QChar32;

      // broom, following four iterators need to return an iterator to a QChar32
      using iterator        = CsString::CsString::iterator;
      using const_iterator  = CsString::CsString::const_iterator;

      using reverse_iterator        = CsString::CsString::reverse_iterator;
      using const_reverse_iterator  = CsString::CsString::const_reverse_iterator;

      QString8() = default;
      QString8(const QString8 &other) = default;
      QString8(QString8 &&other) = default;

      QString8(QChar32 c);
      QString8(size_type size, QChar32 c);

      QString8(const QChar32 *data, size_type size = -1)  {

         if (size == -1) {
            const QChar32 *p = data;

            while (p->unicode() != 0) {
               ++p;
            }

            CsString::CsString::append(data, p);

         } else {
            CsString::CsString::append(data, data + size);

         }
      }

      QString8(const CsString::CsString &other)
         : CsString::CsString(other)
      {
      }

      QString8(CsString::CsString &&other)
         : CsString::CsString(std::move(other))
      {
      }

      template <typename Iterator>
      QString8(Iterator begin, Iterator end)
         : CsString::CsString(begin, end)
      { }

      ~QString8() = default;

      // methods
      QString8 &append(QChar32 c)  {
         CsString::CsString::append(c);
         return *this;
      }

      QString8 &append(const QChar32 *data, size_type size)  {
         CsString::CsString::append(data, data + size);
         return *this;
      }

      QString8 &append(const QString8 &other)  {
         CsString::CsString::append(other);
         return *this;
      }

      QChar32 at(size_type index) const;

      void chop(size_type n);

      void clear() {
         CsString::CsString::clear();
      }

      size_type count() const {
         return CsString::CsString::size();
      }

      const char *constData() const {
         return reinterpret_cast<const char *>(CsString::CsString::constData());
      }

      const char *data() const {
         return constData();
      }

      QString8 &fill(QChar32 c, size_type size = -1);

      QString8 &insert (size_type index, const QString8 & other)  {
         CsString::CsString::insert(index, other);
         return *this;
      }

      QString8 &insert(size_type index, QChar32 c) {
         CsString::CsString::insert(index, 1, c);
         return *this;
      }

      QString8 &insert(size_type index, const QChar32 *data, size_type size) {
         CsString::CsString::insert(begin() + index, data, data + size);
         return *this;
      }

      bool isEmpty() const;

      QString8 left(size_type numOfChars) const Q_REQUIRED_RESULT;

      size_type length() const {
         return CsString::CsString::size();
      }

      QString8 mid(size_type index, size_type numOfChars = -1) const Q_REQUIRED_RESULT;

      QString8 &prepend(const QString8 &other) {
         CsString::CsString::insert(begin(), other);
         return *this;
      }

      QString8 &prepend(QChar32 c) {
         CsString::CsString::insert(begin(), c);
         return *this;
      }

      QString8 &prepend(const QChar32 *data, size_type size)  {
         CsString::CsString::insert(begin(), data, data + size);
         return *this;
      }

      void push_back(QChar32 c) {
         append(c);
      }

      void push_back(const QString8 &other) {
         append(other);
      }

      void push_front(QChar32 c) {
         prepend(c);
      }

      void push_front(const QString8 &other) {
         prepend(other);
      }

      QString8 repeated(size_type count) const Q_REQUIRED_RESULT;

      QString8 &remove(size_type index, size_type size);

      QString8 &replace(size_type index, size_type numOfChars, QChar32 c) {
         CsString::CsString::replace(index, numOfChars, 1, c);
         return *this;
      }

      QString8 &replace(size_type index, size_type numOfChars, const QString8 &other) {
         CsString::CsString::replace(index, numOfChars, other);
         return *this;
      }

      void resize(size_type size) {
         return CsString::CsString::resize(size);
      }

      void resize(size_type size, QChar32 c) {
         return CsString::CsString::resize(size, c);
      }

      QString8 right(size_type numOfChars) const Q_REQUIRED_RESULT;

      size_type size() const {
         return CsString::CsString::size();
      }

      void squeeze() {
         return CsString::CsString::shrink_to_fit();
      }

/*
      void swap(QString8 &other) {
         cs_swapFunc(*this, other);
      }
*/

      void truncate(size_type length);

      const uint8_t *utf8() const {
         return CsString::CsString::constData();
      }

      // iterators
      iterator begin() {
         return CsString::CsString::begin();
      }

      const_iterator begin() const {
         return CsString::CsString::cbegin();
      }

      const_iterator cbegin() const {
         return CsString::CsString::cbegin();
      }

      const_iterator constBegin() const {
         return CsString::CsString::cbegin();
      }

      iterator end() {
         return CsString::CsString::end();
      }

      const_iterator end() const {
         return CsString::CsString::cend();
      }

      const_iterator cend() const {
         return CsString::CsString::cend();
      }

      const_iterator constEnd() const {
         return CsString::CsString::cend();
      }

      reverse_iterator rbegin()  {
         return CsString::CsString::rbegin();
      }

      const_reverse_iterator rbegin() const {
         return CsString::CsString::rbegin();
      }

      reverse_iterator rend()  {
         return CsString::CsString::rend();
      }

      const_reverse_iterator rend() const {
         return CsString::CsString::rend();
      }

      const_reverse_iterator crbegin() const {
         return CsString::CsString::crbegin();
      }

      const_reverse_iterator crend() const {
         return CsString::CsString::crend();
      }

      // operators
      using CsString::CsString::operator=;      // check QString doc if in
      using CsString::CsString::operator+=;     // check QString doc if in

      QString8 &operator=(const QString8 &other) = default;
      QString8 &operator=(QString8 && other) = default;

   private:

};

Q_DECLARE_TYPEINFO(QString8, Q_MOVABLE_TYPE);      // broom - verify

/*
void cs_swapFunc(QString8 &a, QString8 &b) {
   swap(a, b);
}
*/

#endif