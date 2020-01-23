/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
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

#ifndef QSCOPEDVALUEROLLBACK_H
#define QSCOPEDVALUEROLLBACK_H

#include <QtCore/qglobal.h>

template <typename T>
class QScopedValueRollback
{
 public:
   explicit QScopedValueRollback(T &var) :
        varRef(var), oldValue(var)
   { }

    explicit QScopedValueRollback(T &var, T value) :
        varRef(var), oldValue(var)
    {
        varRef = qMove(value);
    }

   ~QScopedValueRollback() {
      varRef = std::move(oldValue);
   }

   void commit() {
      oldValue = varRef;
   }

 private:
   T &varRef;
   T oldValue;

   Q_DISABLE_COPY(QScopedValueRollback)
};

#endif
