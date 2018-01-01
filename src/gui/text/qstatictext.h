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

#ifndef QSTATICTEXT_H
#define QSTATICTEXT_H

#include <QtCore/qsize.h>
#include <QtCore/qstring.h>
#include <QtCore/qmetatype.h>
#include <QtGui/qtransform.h>
#include <QtGui/qfont.h>
#include <QtGui/qtextoption.h>

QT_BEGIN_NAMESPACE

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

   void prepare(const QTransform &matrix = QTransform(), const QFont &font = QFont());

   void setPerformanceHint(PerformanceHint performanceHint);
   PerformanceHint performanceHint() const;

   QStaticText &operator=(const QStaticText &);
   bool operator==(const QStaticText &) const;
   bool operator!=(const QStaticText &) const;

 private:
   void detach();

   QExplicitlySharedDataPointer<QStaticTextPrivate> data;
   friend class QStaticTextPrivate;
};

QT_END_NAMESPACE

Q_DECLARE_METATYPE(QStaticText)

#endif // QSTATICTEXT_H
