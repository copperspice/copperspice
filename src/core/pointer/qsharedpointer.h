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

#ifndef QSHAREDPOINTER_H
#define QSHAREDPOINTER_H

#include <cs_enable_shared.h>
#include <cs_shared_pointer.h>
#include <cs_weak_pointer.h>

#include <qglobal.h>

class CSInternalRefCount;
class QObject;

#if ! defined(CS_DOXYPRESS)

template <typename T>
class QSharedPointer : public CsPointer::CsSharedPointer<T>
{
 public:
   using CsPointer::CsSharedPointer<T>::CsSharedPointer;

   QSharedPointer(CsPointer::CsSharedPointer<T> p) noexcept
      : CsPointer::CsSharedPointer<T>(std::move(p))
   {
   }

   QSharedPointer(const QSharedPointer &) = default;
   QSharedPointer &operator=(const QSharedPointer &) = default;

   QSharedPointer(QSharedPointer &&other) = default;
   QSharedPointer &operator=(QSharedPointer && other) = default;

   bool isNull() const {
      return CsPointer::CsSharedPointer<T>::is_null();
   }

   template <typename U>
   QSharedPointer<U> constCast() const {
      return CsPointer::const_pointer_cast<U>(*this);
   }

   template <typename U>
   QSharedPointer<U> dynamicCast() const {
      return CsPointer::dynamic_pointer_cast<U>(*this);
   }

   template <typename U>
   QSharedPointer<U> objectCast() const {
      return CsPointer::dynamic_pointer_cast<U>(*this);
   }

   template <typename U>
   QSharedPointer<U> staticCast() const {
      return CsPointer::static_pointer_cast<U>(*this);
   }
};

#endif

#endif
