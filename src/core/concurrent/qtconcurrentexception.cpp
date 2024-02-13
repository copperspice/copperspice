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

#include <qtconcurrentexception.h>

namespace QtConcurrent {

void Exception::raise() const
{
   Exception e = *this;
   throw e;
}

Exception *Exception::clone() const
{
   return new Exception(*this);
}

void UnhandledException::raise() const
{
   UnhandledException e = *this;
   throw e;
}

Exception *UnhandledException::clone() const
{
   return new UnhandledException(*this);
}

namespace cs_internal {

class Base
{
 public:
   Base(Exception *exception)
      : exception(exception), refCount(1), hasThrown(false)
   { }

   ~Base() {
      delete exception;
   }

   Exception *exception;
   QAtomicInt refCount;
   bool hasThrown;
};

ExceptionHolder::ExceptionHolder(Exception *exception)
   : base(new Base(exception))
{
}

ExceptionHolder::ExceptionHolder(const ExceptionHolder &other)
   : base(other.base)
{
   base->refCount.ref();
}

void ExceptionHolder::operator=(const ExceptionHolder &other)
{
   if (base == other.base) {
      return;
   }

   if (base->refCount.deref() == false) {
      delete base;
   }

   base = other.base;
   base->refCount.ref();
}

ExceptionHolder::~ExceptionHolder()
{
   if (base->refCount.deref() == false) {
      delete base;
   }
}

Exception *ExceptionHolder::exception() const
{
   return base->exception;
}

void ExceptionStore::setException(const Exception &e)
{
   if (hasException() == false) {
      exceptionHolder = ExceptionHolder(e.clone());
   }
}

bool ExceptionStore::hasException() const
{
   return (exceptionHolder.exception() != nullptr);
}

ExceptionHolder ExceptionStore::exception()
{
   return exceptionHolder;
}

void ExceptionStore::throwPossibleException()
{
   if (hasException() ) {
      exceptionHolder.base->hasThrown = true;
      exceptionHolder.exception()->raise();
   }
}

bool ExceptionStore::hasThrown() const
{
   return exceptionHolder.base->hasThrown;
}

}   // end namespace

} // namespace QtConcurrent

