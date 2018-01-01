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

#ifndef QXMLUTILS_P_H
#define QXMLUTILS_P_H

#include <QtCore/qstring.h>

QT_BEGIN_NAMESPACE

class QString;
class QChar;
class QXmlCharRange;

//  This class contains helper functions related to XML for validating character classes, productions in the XML specification
class Q_CORE_EXPORT QXmlUtils
{
 public:
   static bool isEncName(const QString &encName);
   static bool isChar(const QChar c);
   static bool isNameChar(const QChar c);
   static bool isLetter(const QChar c);
   static bool isNCName(const QStringRef &ncName);
   static inline bool isNCName(const QString &ncName) {
      return isNCName(&ncName);
   }
   static bool isPublicID(const QString &candidate);

 private:
   typedef const QXmlCharRange *RangeIter;
   static bool rangeContains(RangeIter begin, RangeIter end, const QChar c);
   static bool isBaseChar(const QChar c);
   static bool isDigit(const QChar c);
   static bool isExtender(const QChar c);
   static bool isIdeographic(const QChar c);
   static bool isCombiningChar(const QChar c);
};

QT_END_NAMESPACE

#endif
