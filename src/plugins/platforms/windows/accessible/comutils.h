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

#ifndef COMUTILS_H
#define COMUTILS_H

#if ! defined(_WINDOWS_) && ! defined(_WINDOWS_H) && ! defined(__WINDOWS__)
#error Must include windows.h first
#endif

#include <ocidl.h>
#include <qstring.h>

class QVariant;

// Originally QVariantToVARIANT copied from ActiveQt, renamed to avoid conflicts in static builds
bool QVariant2VARIANT(const QVariant &var, VARIANT &arg, const QByteArray &typeName, bool out);

inline BSTR QStringToBSTR(const QString &str)
{
   std::wstring tmp = str.toStdWString();
   return SysAllocStringLen(reinterpret_cast<const OLECHAR *>(tmp.data()), UINT(tmp.size()));
}

#endif

