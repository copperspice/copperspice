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

#ifndef PRINTOUT_H
#define PRINTOUT_H

#include <qdatetime.h>
#include <qfont.h>
#include <qlist.h>
#include <qpainter.h>
#include <qrect.h>
#include <qtextoption.h>

class QFontMetrics;
class QPrinter;

class PrintOut
{
 public:
   enum Rule { NoRule, ThinRule, ThickRule };
   enum Style { Normal, Strong, Emphasis };

   PrintOut(QPrinter *printer);
   ~PrintOut();

   void setRule(Rule rule);
   void setGuide(const QString &guide);
   void vskip();
   void flushLine(bool mayBreak = false);
   void addBox(int percent, const QString &text = QString(),
               Style style = Normal,
               Qt::Alignment halign = Qt::AlignLeft);

   int pageNum() const {
      return page;
   }

   struct Box {
      QRect rect;
      QString text;
      QFont font;
      QTextOption options;

      Box( const QRect &r, const QString &t, const QFont &f, const QTextOption &o )
         : rect( r ), text( t ), font( f ), options( o ) { }
   };

 private:
   void breakPage(bool init = false);
   void drawRule( Rule rule );

   struct Paragraph {
      QRect rect;
      QList<Box> boxes;

      Paragraph() { }
      Paragraph( QPoint p ) : rect( p, QSize(0, 0) ) { }
   };

   QPrinter *pr;
   QPainter p;
   QFont f8;
   QFont f10;
   QFontMetrics *fmetrics;
   Rule nextRule;
   Paragraph cp;
   int page;
   bool firstParagraph;
   QString g;
   QDateTime dateTime;

   int hmargin;
   int vmargin;
   int voffset;
   int hsize;
   int vsize;
};

#endif
