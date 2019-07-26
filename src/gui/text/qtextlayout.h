/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

#ifndef QTEXTLAYOUT_H
#define QTEXTLAYOUT_H

#include <qstring.h>
#include <qnamespace.h>
#include <qrect.h>
#include <qvector.h>
#include <qcolor.h>
#include <qobject.h>
#include <qevent.h>
#include <qtextformat.h>
#include <qglyphrun.h>
#include <qtextcursor.h>

class QTextEngine;
class QFont;
class QRawFont;
class QRect;
class QRegion;
class QTextFormat;
class QPalette;
class QPainter;
class QPaintDevice;
class QTextLine;
class QTextBlock;
class QTextOption;

class Q_GUI_EXPORT QTextInlineObject
{

 public:
   QTextInlineObject(int i, QTextEngine *e) : itm(i), eng(e) {}
   inline QTextInlineObject() : itm(0), eng(nullptr) {}

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
   QTextLayout();
   QTextLayout(const QString &text);
   QTextLayout(const QString &text, const QFont &font, QPaintDevice *paintdevice = nullptr);
   QTextLayout(const QTextBlock &b);

   ~QTextLayout();

   void setFont(const QFont &f);
   QFont font() const;

   void setText(const QString &string);
   QString text() const;

   void setTextOption(const QTextOption &option);
   const QTextOption &textOption() const;

   void setRawFont(const QRawFont &rawFont);

   void setPreeditArea(int position, const QString &text);
   int preeditAreaPosition() const;
   QString preeditAreaText() const;

   struct FormatRange {
      int start;
      int length;
      QTextCharFormat format;

      friend bool operator==(const FormatRange &lhs, const FormatRange &rhs) {
         return lhs.start == rhs.start && lhs.length == rhs.length && lhs.format == rhs.format;
      }

      friend bool operator!=(const FormatRange &lhs, const FormatRange &rhs) {
         return !operator==(lhs, rhs);
      }
   };


   void setFormats(const QVector<FormatRange> &overrides);
   QVector<FormatRange> formats() const;
   void clearFormats();

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

   QList<QGlyphRun> glyphRuns(int from = -1, int length = -1) const;

   QTextEngine *engine() const {
      return d;
   }
   void setFlags(int flags);

 private:
   QTextLayout(QTextEngine *e) : d(e) {}
   Q_DISABLE_COPY(QTextLayout)

    QTextEngine *d;

   friend class QPainter;

   friend class QGraphicsSimpleTextItemPrivate;
   friend class QGraphicsSimpleTextItem;

   friend void qt_format_text(const QFont &font, const QRectF &_r, int tf, const QTextOption *, const QString &str,
                  QRectF *brect, int tabstops, int *tabarray, int tabarraylen, QPainter *painter);

};

class Q_GUI_EXPORT QTextLine
{
 public:
   enum Edge {
      Leading,
      Trailing
   };

   enum CursorPosition {
      CursorBetweenCharacters,
      CursorOnCharacter
   };

   inline QTextLine() : index(0), m_textEngine(nullptr) {}

   inline bool isValid() const {
      return m_textEngine;
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

   // cursorPos gets set to the valid position
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
      return index;
   }

   void draw(QPainter *p, const QPointF &point, const QTextLayout::FormatRange *selection = nullptr) const;

   QList<QGlyphRun> glyphRuns(int from = -1, int length = -1) const;

 private:
   QTextLine(int line, QTextEngine *e) : index(line), m_textEngine(e) {}
   void layout_helper(int numGlyphs);

   int index;
   QTextEngine *m_textEngine;

   friend class QTextLayout;
   friend class QTextFragment;
};

#endif
