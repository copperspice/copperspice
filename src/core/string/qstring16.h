/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
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

#ifndef QSTRING16_H
#define QSTRING16_H

#define CS_STRING_ALLOW_UNSAFE

#include <cstddef>
#include <string>

#include <qglobal.h>
#include <qbytearray.h>

#include <cs_string.h>
#include <qchar32.h>
#include <qstringview.h>

class QStringParser;

template <typename S>
class QRegularExpression;

class Q_CORE_EXPORT QString16 : public CsString::CsString_utf16
{
   public:

};

Q_CORE_EXPORT QDataStream &operator<<(QDataStream &, const QString16 &);
Q_CORE_EXPORT QDataStream &operator>>(QDataStream &, QString16 &);

#endif