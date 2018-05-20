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

#ifndef QSTRINGFWD_H
#define QSTRINGFWD_H

class QByteArray;
class QChar32;
class QString8;
class QString16;

using QChar         = QChar32;
using QString       = QString8;
using QLatin1Char   = QChar32;
using QLatin1String = QString8;

#if ! defined (CS_DOXYPRESS)
namespace Cs {
#endif

template <typename S>
class QRegularExpression;

template <typename S>
class QRegularExpressionMatch;

template <typename S>
class QStringView;

#if ! defined (CS_DOXYPRESS)
}
#endif

using QRegularExpression        = Cs::QRegularExpression<QString8>;
using QRegularExpression8       = Cs::QRegularExpression<QString8>;
using QRegularExpression16      = Cs::QRegularExpression<QString16>;

using QRegularExpressionMatch   = Cs::QRegularExpressionMatch<QString8>;
using QRegularExpressionMatch8  = Cs::QRegularExpressionMatch<QString8>;
using QRegularExpressionMatch16 = Cs::QRegularExpressionMatch<QString16>;

#if ! defined (CS_DOXYPRESS)
using QStringView    = Cs::QStringView<QString8>;
#endif

using QStringView8   = Cs::QStringView<QString8>;
using QStringView16  = Cs::QStringView<QString16>;

#endif