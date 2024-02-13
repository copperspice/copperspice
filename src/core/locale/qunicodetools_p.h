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

#ifndef QUNICODETOOLS_P_H
#define QUNICODETOOLS_P_H

#include <qchar.h>
#include <qstring.h>
#include <qvector.h>

struct QCharAttributes {
   uchar graphemeBoundary : 1;
   uchar wordBreak        : 1;
   uchar sentenceBoundary : 1;
   uchar lineBreak        : 1;
   uchar whiteSpace       : 1;
   uchar wordStart        : 1;
   uchar wordEnd          : 1;
   uchar mandatoryBreak   : 1;
};

namespace QUnicodeTools {

struct ScriptItem {
   int position;
   QChar::Script script;
};

enum CharAttributeOption {
   GraphemeBreaks       = 0x01,
   WordBreaks           = 0x02,
   SentenceBreaks       = 0x04,
   LineBreaks           = 0x08,
   WhiteSpaces          = 0x10,
   DefaultOptionsCompat = GraphemeBreaks | LineBreaks | WhiteSpaces,

   DontClearAttributes = 0x1000
};
using CharAttributeOptions = QFlags<CharAttributeOption>;

// attributes buffer has to have a length of string length + 1
Q_CORE_EXPORT void initCharAttributes(const QString &str, QVector<QUnicodeTools::ScriptItem> &scriptItems,
      QCharAttributes *attributes, CharAttributeOptions options = DefaultOptionsCompat);

Q_CORE_EXPORT void initScripts(const QString &str, QVector<QChar::Script> &scriptIds);

} // namespace

#endif
