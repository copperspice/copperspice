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

#ifndef QFONTINFO_H
#define QFONTINFO_H

#include <QtGui/qfont.h>
#include <QtCore/qsharedpointer.h>

QT_BEGIN_NAMESPACE

class Q_GUI_EXPORT QFontInfo
{
 public:
   QFontInfo(const QFont &);
   QFontInfo(const QFontInfo &);
   ~QFontInfo();

   QFontInfo &operator=(const QFontInfo &);

   QString family() const;
   QString styleName() const;
   int pixelSize() const;
   int pointSize() const;
   qreal pointSizeF() const;
   bool italic() const;
   QFont::Style style() const;
   int weight() const;
   inline bool bold() const {
      return weight() > QFont::Normal;
   }
   bool underline() const;
   bool overline() const;
   bool strikeOut() const;
   bool fixedPitch() const;
   QFont::StyleHint styleHint() const;
   bool rawMode() const;

   bool exactMatch() const;

 private:
   QExplicitlySharedDataPointer<QFontPrivate> d;
};

QT_END_NAMESPACE


#endif // QFONTINFO_H
