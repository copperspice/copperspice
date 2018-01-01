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

#ifndef QTEXTLAYOUT_H
#define QTEXTLAYOUT_H

#include <QtCore/qstring.h>
#include <QtCore/qnamespace.h>
#include <QtCore/qrect.h>
#include <QtCore/qvector.h>
#include <QtGui/qcolor.h>
#include <QtCore/qobject.h>
#include <QtGui/qevent.h>
#include <QtGui/qtextformat.h>
#include <QtGui/qglyphrun.h>
#include <QtGui/qtextcursor.h>

QT_BEGIN_NAMESPACE

class QTextEngine;
class QFont;
class QRect;
class QRegion;
class QTextFormat;
class QPalette;
class QPainter;
class QPaintDevice;
class QTextFormat;
class QTextLine;
class QTextBlock;
class QTextOption;

class Q_GUI_EXPORT QTextInlineObject
{

 public:
   QTextInlineObject(int i, QTextEngine *e) : itm(i), eng(e) {}
   inline QTextInlineObject() : itm(0), eng(0) {}
   inline bool isValid() const {
      return eng;
   }

   QRectF rect() const;
   qreal width() const;
   qreal ascent() const;
   qreal descent() const;
   qreal height() const;

   Qt::LayoutDirection textDirection() const;

   void setWidth(qreal w);
   void setAscent(qreal a);
   void setDescent(qreal d);

   int textPosition() const;

   int formatIndex() const;
   QTextFormat format() const;

 private:
   friend class QTextLayout;
   int itm;
   QTextEngine *eng;
};


class Q_GUI_EXPORT QTextLayout
{

 public:
   // does itemization
   QTextLayout();
   QTextLayout(const QString &text);
   QTextLayout(const QString &text, const QFont &font, QPaintDevice *paintdevice = 0);
   QTextLayout(const QTextBlock &b);
   ~QTextLayout();

   void setFont(const QFont &f);
   QFont font() const;

   void setText(const QString &string);
   QString text() const;

   void setTextOption(const QTextOption &option);
   QTextOption textOption() const;

   void setPreeditArea(int position, const QString &text);
   int preeditAreaPosition() const;
   QString preeditAreaText() const;

   struct FormatRange {
      int start;
      int length;
      QTextCharFormat format;
   };
   void setAdditionalFormats(const QList<FormatRange> &overrides);
   QList<FormatRange> additionalFormats() const;
   void clearAdditionalFormats();

   void setCacheEnabled(bool enable);
   bool cacheEnabled() const;

   void setCursorMoveStyle(Qt::CursorMoveStyle style);
   Qt::CursorMoveStyle cursorMoveStyle() const;

   void beginLayout();
   void endLayout();
   void clearLayout();

   QTextLine createLine();

   int lineCount() const;
   QTextLine lineAt(int i) const;
   QTextLine lineForTextPosition(int pos) const;

   enum CursorMode {
      SkipCharacters,
      SkipWords
   };
   bool isValidCursorPosition(int pos) const;
   int nextCursorPosition(int oldPos, CursorMode mode = SkipCharacters) const;
   int previousCursorPosition(int oldPos, CursorMode mode = SkipCharacters) const;
   int leftCursorPosition(int oldPos) const;
   int rightCursorPosition(int oldPos) const;

   void draw(QPainter *p, const QPointF &pos, const QVector<FormatRange> &selections = QVector<FormatRange>(),
             const QRectF &clip = QRectF()) const;
   void drawCursor(QPainter *p, const QPointF &pos, int cursorPosition) const;
   void drawCursor(QPainter *p, const QPointF &pos, int cursorPosition, int width) const;

   QPointF position() const;
   void setPosition(const QPointF &p);

   QRectF boundingRect() const;

   qreal minimumWidth() const;
   qreal maximumWidth() const;

#if !defined(QT_NO_RAWFONT)
   QList<QGlyphRun> glyphRuns() const;
#endif

   QTextEngine *engine() const {
      return d;
   }
   void setFlags(int flags);

 private:
   QTextLayout(QTextEngine *e) : d(e) {}
   Q_DISABLE_COPY(QTextLayout)

   friend class QPainter;
   friend class QPSPrinter;
   friend class QGraphicsSimpleTextItemPrivate;
   friend class QGraphicsSimpleTextItem;

   friend void qt_format_text(const QFont &font, const QRectF &_r, int tf, const QTextOption *, const QString &str,
                              QRectF *brect, int tabstops, int *tabarray, int tabarraylen, QPainter *painter);
   QTextEngine *d;
};


class Q_GUI_EXPORT QTextLine
{
 public:
   inline QTextLine() : i(0), eng(0) {}
   inline bool isValid() const {
      return eng;
   }

   QRectF rect() const;
   qreal x() const;
   qreal y() const;
   qreal width() const;
   qreal ascent() const;
   qreal descent() const;
   qreal height() const;
   qreal leading() const;

   void setLeadingIncluded(bool included);
   bool leadingIncluded() const;

   qreal naturalTextWidth() const;
   qreal horizontalAdvance() const;
   QRectF naturalTextRect() const;

   enum Edge {
      Leading,
      Trailing
   };
   enum CursorPosition {
      CursorBetweenCharacters,
      CursorOnCharacter
   };

   /* cursorPos gets set to the valid position */
   qreal cursorToX(int *cursorPos, Edge edge = Leading) const;
   inline qreal cursorToX(int cursorPos, Edge edge = Leading) const {
      return cursorToX(&cursorPos, edge);
   }
   int xToCursor(qreal x, CursorPosition = CursorBetweenCharacters) const;

   void setLineWidth(qreal width);
   void setNumColumns(int columns);
   void setNumColumns(int columns, qreal alignmentWidth);

   void setPosition(const QPointF &pos);
   QPointF position() const;

   int textStart() const;
   int textLength() const;

   int lineNumber() const {
      return i;
   }

   void draw(QPainter *p, const QPointF &point, const QTextLayout::FormatRange *selection = 0) const;

 private:
   QTextLine(int line, QTextEngine *e) : i(line), eng(e) {}
   void layout_helper(int numGlyphs);

#if !defined(QT_NO_RAWFONT)
   QList<QGlyphRun> glyphs(int from, int length) const;
#endif

   friend class QTextLayout;
   friend class QTextFragment;
   int i;
   QTextEngine *eng;
};

QT_END_NAMESPACE

#endif // QTEXTLAYOUT_H
