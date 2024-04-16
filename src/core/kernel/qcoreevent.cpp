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

#include <qcoreevent.h>

#include <qcoreapplication.h>

#include <qcoreapplication_p.h>

#include <limits>
#include <atomic>

QEvent::QEvent(Type type)
   : d(nullptr), t(type), posted(false), spont(false), m_accept(true)
{
}

QEvent::QEvent(const QEvent &other)
   : d(other.d), t(other.t), posted(other.posted), spont(other.spont), m_accept(other.m_accept)
{
   Q_ASSERT_X(! d, "QEvent", "QEventPrivate is not defined anywhere");
}

QEvent &QEvent::operator=(const QEvent &other)
{
   Q_ASSERT_X(! other.d, "QEvent", "QEventPrivate is not defined anywhere");

   t        = other.t;
   posted   = other.posted;
   spont    = other.spont;
   m_accept = other.m_accept;

   return *this;
}

QEvent::~QEvent()
{
   if (posted && QCoreApplication::instance()) {
      QCoreApplicationPrivate::removePostedEvent(this);
   }

   Q_ASSERT_X(! d, "QEvent", "QEventPrivate is not defined anywhere");
}

namespace {

template <size_t N>
struct QBasicAtomicBitField {
   static constexpr const int BitsPerInt = std::numeric_limits<uint>::digits;
   static constexpr const int NumInts = (N + BitsPerInt - 1) / BitsPerInt;
   static constexpr const int NumBits = N;

   // atomic int points to the next (possibly) free ID saving
   // the otherwise necessary scan through 'data':
   std::atomic<uint> next;
   std::atomic<uint> data[NumInts];

   bool allocateSpecific(int which) {
      std::atomic<uint> &entry = data[which / BitsPerInt];

      uint old = entry.load();
      const uint bit = 1U << (which % BitsPerInt);

      return ! (old & bit) && entry.compare_exchange_strong(old, old | bit, std::memory_order_relaxed);
   }

   int allocateNext() {
      // Unroll loop to iterate over ints, then bits? Would save
      // potentially a lot of cmpxchgs, because we can scan the
      // whole int before having to load it again.

      // this should never execute many iterations, so leave like this for now
      for (uint i = next.load(); i < NumBits; ++i) {
         if (allocateSpecific(i)) {
            // remember next (possibly) free id
            uint oldNext = next.load();

            next.compare_exchange_strong(oldNext, qMax(i + 1, oldNext), std::memory_order_relaxed);
            return i;
         }
      }

      return -1;
   }
};

} // end namespace

using UserEventTypeRegistry = QBasicAtomicBitField<QEvent::MaxUser - QEvent::User + 1>;

static UserEventTypeRegistry userEventTypeRegistry;

static inline int registerEventTypeZeroBased(int id)
{
   // if the type hint has not been registered yet, take it
   if (id < UserEventTypeRegistry::NumBits && id >= 0 && userEventTypeRegistry.allocateSpecific(id)) {
      return id;
   }

   // otherwise ignore hint
   return userEventTypeRegistry.allocateNext();
}

int QEvent::registerEventType(int hint)
{
   const int result = registerEventTypeZeroBased(QEvent::MaxUser - hint);
   return result < 0 ? -1 : QEvent::MaxUser - result ;
}

QTimerEvent::QTimerEvent(int timerId)
   : QEvent(Timer), id(timerId)
{
}

QTimerEvent::~QTimerEvent()
{
}

QChildEvent::QChildEvent(Type type, QObject *child)
   : QEvent(type), c(child)
{
}

QChildEvent::~QChildEvent()
{
}

QDynamicPropertyChangeEvent::QDynamicPropertyChangeEvent(const QByteArray &name)
   : QEvent(QEvent::DynamicPropertyChange), n(name)
{
}

QDynamicPropertyChangeEvent::~QDynamicPropertyChangeEvent()
{
}

QDeferredDeleteEvent::QDeferredDeleteEvent()
   : QEvent(QEvent::DeferredDelete), level(0)
{ }

QDeferredDeleteEvent::~QDeferredDeleteEvent()
{ }
