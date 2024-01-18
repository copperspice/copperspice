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

#ifndef QDECLARATIVENULLABLEVALUE_P_P_H
#define QDECLARATIVENULLABLEVALUE_P_P_H

QT_BEGIN_NAMESPACE

template<typename T>
struct QDeclarativeNullableValue {
   QDeclarativeNullableValue()
      : isNull(true), value(T()) {}
   QDeclarativeNullableValue(const QDeclarativeNullableValue<T> &o)
      : isNull(o.isNull), value(o.value) {}
   QDeclarativeNullableValue(const T &t)
      : isNull(false), value(t) {}
   QDeclarativeNullableValue<T> &operator=(const T &t) {
      isNull = false;
      value = t;
      return *this;
   }
   QDeclarativeNullableValue<T> &operator=(const QDeclarativeNullableValue<T> &o) {
      isNull = o.isNull;
      value = o.value;
      return *this;
   }
   operator T() const {
      return value;
   }

   void invalidate() {
      isNull = true;
   }
   bool isValid() const {
      return !isNull;
   }
   bool isNull;
   T value;
};

QT_END_NAMESPACE

#endif // QDECLARATIVENULLABLEVALUE_P_H
