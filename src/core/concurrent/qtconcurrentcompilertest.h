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

#ifndef QTCONCURRENTCOMPILERTEST_H
#define QTCONCURRENTCOMPILERTEST_H

#include <QtCore/qglobal.h>

QT_BEGIN_NAMESPACE

#define QT_TYPENAME typename

namespace QtPrivate {

template<class T>
class HasResultType
{
   typedef char Yes;
   typedef void *No;
   template<typename U> static Yes test(int, const typename U::result_type * = 0);
   template<typename U> static No test(double);

 public:
   enum { Value = (sizeof(test<T>(0)) == sizeof(Yes)) };
};

}

QT_END_NAMESPACE

#endif
