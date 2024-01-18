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

/*
*
* Copyright (c) 2006 Nikolas Zimmermann <zimmermann@kde.org>
*
*/

#include "config.h"

#include <wtf/StdLibExtras.h>
#include <wtf/text/WTFString.h>

#include <qstring8.h>
#include <qstring16.h>
#include <qstringview.h>

namespace WTF {

String::String(const QString &str)
{
   if (str.isEmpty()) {
      return;
   }

   QString16 tmp = str.toUtf16();
   m_impl = StringImpl::create(reinterpret_cast<const UChar *>(tmp.constData()), tmp.size_storage());
}

String::String(QStringView view)
{
   if (view.isEmpty()) {
      return;
   }

   QString16 tmp(view.begin(), view.end());
   m_impl = StringImpl::create(reinterpret_cast<const UChar*>(tmp.constData()), tmp.size_storage());
}

String::operator QString() const
{
   return QString::fromUtf16(reinterpret_cast<const char16_t *>(characters()), length());
}

QDataStream & operator<<(QDataStream &stream, const String &str)
{
    stream << QString(str);
    return stream;
}

QDataStream& operator>>(QDataStream &stream, String &str)
{
    QString tmp;

    stream >> tmp;
    str = tmp;

    return stream;
}

}

