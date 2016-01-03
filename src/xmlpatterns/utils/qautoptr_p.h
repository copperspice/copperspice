/***********************************************************************
*
* Copyright (c) 2012-2016 Barbara Geller
* Copyright (c) 2012-2016 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or 
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see 
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QAutoPtr_P_H
#define QAutoPtr_P_H

#include <QtGlobal>

QT_BEGIN_NAMESPACE

namespace QPatternist {
template<typename T>
class AutoPtrRef
{
 public:
   explicit AutoPtrRef(T *const other) : m_ptr(other) {
   }

   T *m_ptr;
};

template<typename T>
class AutoPtr
{
 public:
   explicit inline AutoPtr(T *pointer = 0) : m_ptr(pointer) {
   }

   inline AutoPtr(AutoPtr<T> &other) : m_ptr(other.release()) {
   }

   inline ~AutoPtr() {
      delete m_ptr;
   }

   inline T &operator*() const {
      Q_ASSERT_X(m_ptr, Q_FUNC_INFO, "");
      return *m_ptr;
   }

   T *operator->() const {
      Q_ASSERT_X(m_ptr, Q_FUNC_INFO, "");
      return m_ptr;
   }

   inline bool operator!() const {
      return !m_ptr;
   }

   inline operator bool() const {
      return m_ptr != 0;
   }

   AutoPtr(AutoPtrRef<T> ref) : m_ptr(ref.m_ptr) {
   }

   AutoPtr &operator=(AutoPtrRef<T> ref) {
      if (ref.m_ptr != m_ptr) {
         delete m_ptr;
         m_ptr = ref.m_ptr;
      }
      return *this;
   }

   template<typename L>
   operator AutoPtrRef<L>() {
      return AutoPtrRef<L>(this->release());
   }

   template<typename L>
   operator AutoPtr<L>() {
      return AutoPtr<L>(this->release());
   }

   template<typename L>
   inline AutoPtr(AutoPtr<L> &other) : m_ptr(other.release()) {
   }

   inline T *release() {
      T *const retval = m_ptr;
      m_ptr = 0;
      return retval;
   }

   inline T *data() const {
      return m_ptr;
   }

   void reset(T *other = 0) {
      if (other != m_ptr) {
         delete m_ptr;
         m_ptr = other;
      }
   }

   inline AutoPtr &operator=(AutoPtr &other) {
      reset(other.release());
      return *this;
   }

 private:
   T *m_ptr;
};
}

QT_END_NAMESPACE
#endif
