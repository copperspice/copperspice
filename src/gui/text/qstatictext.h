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

#ifndef QSTATICTEXT_H
#define QSTATICTEXT_H

#include <qsize.h>
#include <qstring.h>
#include <qtransform.h>
#include <qfont.h>
#include <qtextoption.h>

class QStaticTextPrivate;

class Q_GUI_EXPORT QStaticText
{

 public:
   enum PerformanceHint {
      ModerateCaching,
      AggressiveCaching
   };

   QStaticText();
   QStaticText(const QString &text);
   QStaticText(const QStaticText &other);

   ~QStaticText();



   void setText(const QString &text);
   QString text() const;

   void setTextFormat(Qt::TextFormat textFormat);
   Qt::TextFormat textFormat() const;

   void setTextWidth(qreal textWidth);
   qreal textWidth() const;

   void setTextOption(const QTextOption &textOption);
   QTextOption textOption() const;

   QSizeF size() const;

   void swap(QStaticText &other) {
      qSwap(data, other.data);
   }

   void prepare(const QTransform &matrix = QTransform(), const QFont &font = QFont());

   void setPerformanceHint(PerformanceHint performanceHint);
   PerformanceHint performanceHint() const;

   bool operator==(const QStaticText &other) const;
   bool operator!=(const QStaticText &other) const;

   QStaticText &operator=(const QStaticText &other);

   QStaticText &operator=(QStaticText &&other) {
      swap(other);
      return *this;
   }

 private:
   void detach();

   QExplicitlySharedDataPointer<QStaticTextPrivate> data;
   friend class QStaticTextPrivate;
};

#endif // QSTATICTEXT_H
