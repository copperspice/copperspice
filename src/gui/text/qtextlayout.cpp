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

#include <qtextlayout.h>
#include <qtextengine_p.h>

#include <qdebug.h>
#include <qfont.h>
#include <qmath.h>
#include <qpainter.h>
#include <qvarlengtharray.h>
#include <qtextformat.h>
#include <qabstracttextdocumentlayout.h>
#include <qrawfont.h>
#include <qglyphrun.h>

#include <qtextdocument_p.h>
#include <qtextformat_p.h>
#include <qpainterpath.h>
#include <qglyphrun_p.h>
#include <qrawfont_p.h>
#include <qfontengine_p.h>
#include <qpainter_p.h>

#include <limits.h>

#define ObjectSelectionBrush (QTextFormat::ForegroundBrush + 1)
#define SuppressText       0x5012
#define SuppressBackground 0x513

QRectF QTextInlineObject::rect() const
{
   QScriptItem &si = eng->layoutData->items[itm];
   return QRectF(0, -si.ascent.toReal(), si.width.toReal(), si.height().toReal());
}

qreal QTextInlineObject::width() const
{
   return eng->layoutData->items[itm].width.toReal();
}

qreal QTextInlineObject::ascent() const
{
   return eng->layoutData->items[itm].ascent.toReal();
}

qreal QTextInlineObject::descent() const
{
   return eng->layoutData->items[itm].descent.toReal();
}

qreal QTextInlineObject::height() const
{
   return eng->layoutData->items[itm].height().toReal();
}

void QTextInlineObject::setWidth(qreal w)
{
   eng->layoutData->items[itm].width = QFixed::fromReal(w);
}

void QTextInlineObject::setAscent(qreal a)
{
   eng->layoutData->items[itm].ascent = QFixed::fromReal(a);
}

void QTextInlineObject::setDescent(qreal d)
{
   eng->layoutData->items[itm].descent = QFixed::fromReal(d);
}

int QTextInlineObject::textPosition() const
{
   return eng->layoutData->items[itm].position;
}

int QTextInlineObject::formatIndex() const
{
   return eng->formatIndex(&eng->layoutData->items[itm]);
}

QTextFormat QTextInlineObject::format() const
{
   return eng->format(&eng->layoutData->items[itm]);
}

Qt::LayoutDirection QTextInlineObject::textDirection() const
{
   return (eng->layoutData->items[itm].analysis.bidiLevel % 2 ? Qt::RightToLeft : Qt::LeftToRight);
}

QTextLayout::QTextLayout()
{
   d = new QTextEngine();
}

QTextLayout::QTextLayout(const QString &text)
{
   d = new QTextEngine();
   d->text = text;
}

QTextLayout::QTextLayout(const QString &text, const QFont &font, QPaintDevice *paintdevice)
{
   QFont f(font);

   if (paintdevice) {
      f = QFont(font, paintdevice);
   }

   d = new QTextEngine(text, f);
}

QTextLayout::QTextLayout(const QTextBlock &block)
{
   d = new QTextEngine();
   d->block = block;
}

QTextLayout::~QTextLayout()
{
   if (! d->stackEngine) {
      delete d;
   }
}

void QTextLayout::setRawFont(const QRawFont &rawFont)
{
   d->rawFont    = rawFont;
   d->useRawFont = true;
   d->resetFontEngineCache();
}

void QTextLayout::setFont(const QFont &font)
{
   d->fnt = font;
   d->useRawFont = false;
   d->resetFontEngineCache();
}

QFont QTextLayout::font() const
{
   return d->font();
}

void QTextLayout::setText(const QString &string)
{
   d->invalidate();
   d->clearLineData();
   d->text = string;
}

QString QTextLayout::text() const
{
   return d->text;
}

void QTextLayout::setTextOption(const QTextOption &option)
{
   d->option = option;
}

const QTextOption &QTextLayout::textOption() const
{
   return d->option;
}

void QTextLayout::setPreeditArea(int position, const QString &text)
{
   if (d->preeditAreaPosition() == position && d->preeditAreaText() == text) {
      return;
   }

   d->setPreeditArea(position, text);

   if (d->block.docHandle()) {
      d->block.docHandle()->documentChange(d->block.position(), d->block.length());
   }
}

int QTextLayout::preeditAreaPosition() const
{
   return d->preeditAreaPosition();
}

QString QTextLayout::preeditAreaText() const
{
   return d->preeditAreaText();
}

void QTextLayout::setFormats(const QVector<FormatRange> &formats)
{
   d->setFormats(formats);

   if (d->block.docHandle()) {
      d->block.docHandle()->documentChange(d->block.position(), d->block.length());
   }
}

QVector<QTextLayout::FormatRange> QTextLayout::formats() const
{
   return d->formats();
}

void QTextLayout::clearFormats()
{
   setFormats(QVector<FormatRange>());
}

void QTextLayout::setCacheEnabled(bool enable)
{
   d->cacheGlyphs = enable;
}

bool QTextLayout::cacheEnabled() const
{
   return d->cacheGlyphs;
}

void QTextLayout::setCursorMoveStyle(Qt::CursorMoveStyle style)
{
   d->visualMovement = (style == Qt::VisualMoveStyle);
}

Qt::CursorMoveStyle QTextLayout::cursorMoveStyle() const
{
   return d->visualMovement ? Qt::VisualMoveStyle : Qt::LogicalMoveStyle;
}

void QTextLayout::beginLayout()
{
#if defined(CS_SHOW_DEBUG_GUI_TEXT)
   if (d->layoutData && d->layoutData->layoutState == QTextEngine::InLayout) {
      qDebug("QTextLayout::beginLayout() Layout was already in progress");
      return;
   }
#endif

   d->invalidate();
   d->clearLineData();
   d->itemize();
   d->layoutData->layoutState = QTextEngine::InLayout;
}

void QTextLayout::endLayout()
{
#if defined(CS_SHOW_DEBUG_GUI_TEXT)
   if (! d->layoutData || d->layoutData->layoutState == QTextEngine::LayoutEmpty) {
      qDebug("QTextLayout::endLayout() No layout in progress");
      return;
   }
#endif

   int l = d->lines.size();
   if (l && d->lines.at(l - 1).length < 0) {
      QTextLine(l - 1, d).setNumColumns(INT_MAX);
   }

   d->layoutData->layoutState = QTextEngine::LayoutEmpty;
   if (! d->cacheGlyphs) {
      d->freeMemory();
   }
}

void QTextLayout::clearLayout()
{
   d->clearLineData();
}

int QTextLayout::nextCursorPosition(int oldPos, CursorMode mode) const
{
   const QCharAttributes *attributes = d->attributes();

   int len = d->block.isValid() ? d->block.length() - 1
      : d->layoutData->string.length();

   Q_ASSERT(len <= d->layoutData->string.length());

   if (! attributes || oldPos < 0 || oldPos >= len) {
      return oldPos;
   }

   if (mode == SkipCharacters) {
      oldPos++;
      while (oldPos < len && !attributes[oldPos].graphemeBoundary) {
         oldPos++;
      }

   } else {
      if (oldPos < len && d->atWordSeparator(oldPos)) {
         oldPos++;
         while (oldPos < len && d->atWordSeparator(oldPos)) {
            oldPos++;
         }

      } else {
         while (oldPos < len && ! attributes[oldPos].whiteSpace && ! d->atWordSeparator(oldPos)) {
            oldPos++;
         }
      }

      while (oldPos < len && attributes[oldPos].whiteSpace) {
         oldPos++;
      }

   }

   return oldPos;
}

int QTextLayout::previousCursorPosition(int oldPos, CursorMode mode) const
{
   const QCharAttributes *attributes = d->attributes();

   int len = d->block.isValid() ? d->block.length() - 1
      : d->layoutData->string.length();

   Q_ASSERT(len <= d->layoutData->string.length());

   if (! attributes || oldPos <= 0 || oldPos > len) {
      return oldPos;
   }

   if (mode == SkipCharacters) {
      oldPos--;
      while (oldPos && ! attributes[oldPos].graphemeBoundary) {
         oldPos--;
      }

   } else {
      while (oldPos > 0 && attributes[oldPos - 1].whiteSpace) {
         oldPos--;
      }

      if (oldPos && d->atWordSeparator(oldPos - 1)) {
         oldPos--;
         while (oldPos && d->atWordSeparator(oldPos - 1)) {
            oldPos--;
         }

      } else {
         while (oldPos > 0 && !attributes[oldPos - 1].whiteSpace && !d->atWordSeparator(oldPos - 1)) {

            oldPos--;
         }
      }
   }

   return oldPos;
}

int QTextLayout::rightCursorPosition(int oldPos) const
{
   int newPos = d->positionAfterVisualMovement(oldPos, QTextCursor::Right);

   return newPos;
}

int QTextLayout::leftCursorPosition(int oldPos) const
{
   int newPos = d->positionAfterVisualMovement(oldPos, QTextCursor::Left);

   return newPos;
}

bool QTextLayout::isValidCursorPosition(int pos) const
{
   const QCharAttributes *attributes = d->attributes();

   if (! attributes || pos < 0 || pos > (int)d->layoutData->string.length()) {
      return false;
   }

   return attributes[pos].graphemeBoundary;
}

QTextLine QTextLayout::createLine()
{
#if defined(CS_SHOW_DEBUG_GUI_TEXT)
   if (! d->layoutData || d->layoutData->layoutState == QTextEngine::LayoutEmpty) {
      qDebug("QTextLayout::createLine() No layout in progress");
      return QTextLine();
   }
#endif

   if (d->layoutData->layoutState == QTextEngine::LayoutFailed) {
      return QTextLine();
   }

   int max = d->lines.size();
   if (max && d->lines.at(max - 1).length < 0) {
      QTextLine(max - 1, d).setNumColumns(INT_MAX);
   }

   int from   = max > 0 ? d->lines.at(max - 1).from + d->lines.at(max - 1).length + d->lines.at(max - 1).trailingSpaces : 0;
   int strlen = d->layoutData->string.length();

   if (max && from >= strlen) {
      if (! d->lines.at(max - 1).length || d->layoutData->string.at(strlen - 1) != QChar::LineSeparator) {
         return QTextLine();
      }
   }

   QScriptLine line;
   line.from       = from;
   line.length     = -1;
   line.justified  = false;
   line.gridfitted = false;

   d->lines.append(line);

   return QTextLine(max, d);
}


int QTextLayout::lineCount() const
{
   return d->lines.size();
}

QTextLine QTextLayout::lineAt(int i) const
{
   return i < lineCount() ? QTextLine(i, d) : QTextLine();
}

QTextLine QTextLayout::lineForTextPosition(int pos) const
{
   int lineNum = d->lineNumberForTextPosition(pos);

   return lineNum >= 0 ? lineAt(lineNum) : QTextLine();
}

QPointF QTextLayout::position() const
{
   return d->position;
}

void QTextLayout::setPosition(const QPointF &p)
{
   d->position = p;
}

QRectF QTextLayout::boundingRect() const
{
   if (d->lines.isEmpty()) {
      return QRectF();
   }

   QFixed xmax;
   QFixed ymax;
   QFixed xmin = d->lines.at(0).x;
   QFixed ymin = d->lines.at(0).y;

   for (int i = 0; i < d->lines.size(); ++i) {
      const QScriptLine &si = d->lines[i];
      xmin = qMin(xmin, si.x);
      ymin = qMin(ymin, si.y);

      QFixed lineWidth = si.width < QFIXED_MAX ? qMax(si.width, si.textWidth) : si.textWidth;
      xmax = qMax(xmax, si.x + lineWidth);

      // ### should the ascent be used in ymin?
      ymax = qMax(ymax, si.y + si.height().ceil());
   }

   return QRectF(xmin.toReal(), ymin.toReal(), (xmax - xmin).toReal(), (ymax - ymin).toReal());
}

qreal QTextLayout::minimumWidth() const
{
   return d->minWidth.toReal();
}


qreal QTextLayout::maximumWidth() const
{
   return d->maxWidth.toReal();
}

void QTextLayout::setFlags(int flags)
{
   if (flags & Qt::TextJustificationForced) {
      d->option.setAlignment(Qt::AlignJustify);
      d->forceJustification = true;
   }

   if (flags & (Qt::TextForceLeftToRight | Qt::TextForceRightToLeft)) {
      d->ignoreBidi = true;
      d->option.setTextDirection((flags & Qt::TextForceLeftToRight) ? Qt::LeftToRight : Qt::RightToLeft);
   }
}

static void addSelectedRegionsToPath(QTextEngine *eng, int lineNumber, const QPointF &pos,
   QTextLayout::FormatRange *selection, QPainterPath *region, QRectF boundingRect)
{
   const QScriptLine &line = eng->lines[lineNumber];

   QTextLineItemIterator iterator(eng, lineNumber, pos, selection);

   const qreal selectionY = pos.y() + line.y.toReal();
   const qreal lineHeight = line.height().toReal();

   QFixed lastSelectionX = iterator.x;
   QFixed lastSelectionWidth;

   while (!iterator.atEnd()) {
      iterator.next();

      QFixed selectionX, selectionWidth;
      if (iterator.getSelectionBounds(&selectionX, &selectionWidth)) {
         if (selectionX == lastSelectionX + lastSelectionWidth) {
            lastSelectionWidth += selectionWidth;
            continue;
         }

         if (lastSelectionWidth > 0) {
            QRectF rect = boundingRect & QRectF(lastSelectionX.toReal(), selectionY, lastSelectionWidth.toReal(), lineHeight);
            rect.moveLeft(qFloor(rect.left()));
            rect.moveTop(qFloor(rect.top()));
            region->addRect(rect);
         }

         lastSelectionX = selectionX;
         lastSelectionWidth = selectionWidth;
      }
   }
   if (lastSelectionWidth > 0) {
      QRectF rect = boundingRect & QRectF(lastSelectionX.toReal(), selectionY, lastSelectionWidth.toReal(), lineHeight);
      rect.moveLeft(qFloor(rect.left()));
      rect.moveTop(qFloor(rect.top()));
      region->addRect(rect);
   }
}

static inline QRectF clipIfValid(const QRectF &rect, const QRectF &clip)
{
   return clip.isValid() ? (rect & clip) : rect;
}

QList<QGlyphRun> QTextLayout::glyphRuns(int from, int length) const
{
   if (from < 0) {
      from = 0;
   }

   if (length < 0) {
      length = text().length();
   }

   QHash<QPair<QFontEngine *, int>, QGlyphRun> glyphRunHash;

   for (int i = 0; i < d->lines.size(); ++i) {

      if (d->lines[i].from > from + length) {
         break;

      } else if (d->lines[i].from + d->lines[i].length >= from) {
         QList<QGlyphRun> glyphRuns = QTextLine(i, d).glyphRuns(from, length);

         for (int j = 0; j < glyphRuns.size(); j++) {
            const QGlyphRun &glyphRun = glyphRuns.at(j);
            QRawFont rawFont = glyphRun.rawFont();

            QFontEngine *fontEngine = rawFont.m_fontPrivate->fontEngine;

            QGlyphRun::GlyphRunFlags flags = glyphRun.flags();
            QPair<QFontEngine *, int> key(fontEngine, int(flags));

            // merge the glyph runs using the same font
            if (glyphRunHash.contains(key)) {
               QGlyphRun &oldGlyphRun = glyphRunHash[key];

               QVector<quint32> indexes   = oldGlyphRun.glyphIndexes();
               QVector<QPointF> positions = oldGlyphRun.positions();
               QRectF boundingRect = oldGlyphRun.boundingRect();

               indexes   += glyphRun.glyphIndexes();
               positions += glyphRun.positions();
               boundingRect = boundingRect.united(glyphRun.boundingRect());

               oldGlyphRun.setGlyphIndexes(indexes);
               oldGlyphRun.setPositions(positions);
               oldGlyphRun.setBoundingRect(boundingRect);

            } else {
               glyphRunHash[key] = glyphRun;
            }
         }
      }
   }

   return glyphRunHash.values();
}

void QTextLayout::draw(QPainter *p, const QPointF &pos, const QVector<FormatRange> &selections,
   const QRectF &clip) const
{
   if (d->lines.isEmpty()) {
      return;
   }

   if (! d->layoutData) {
      d->itemize();
   }

   QPointF position = pos + d->position;

   QFixed clipy = (INT_MIN / 256);
   QFixed clipe = (INT_MAX / 256);
   if (clip.isValid()) {
      clipy = QFixed::fromReal(clip.y() - position.y());
      clipe = clipy + QFixed::fromReal(clip.height());
   }

   int firstLine = 0;
   int lastLine = d->lines.size();
   for (int i = 0; i < d->lines.size(); ++i) {
      QTextLine l(i, d);
      const QScriptLine &sl = d->lines[i];

      if (sl.y > clipe) {
         lastLine = i;
         break;
      }
      if ((sl.y + sl.height()) < clipy) {
         firstLine = i;
         continue;
      }
   }

   QPainterPath excludedRegion;
   QPainterPath textDoneRegion;
   for (int i = 0; i < selections.size(); ++i) {
      FormatRange selection = selections.at(i);
      const QBrush bg = selection.format.background();

      QPainterPath region;
      region.setFillRule(Qt::WindingFill);

      for (int line = firstLine; line < lastLine; ++line) {
         const QScriptLine &sl = d->lines[line];
         QTextLine tl(line, d);

         QRectF lineRect(tl.naturalTextRect());
         lineRect.translate(position);
         lineRect.adjust(0, 0, d->leadingSpaceWidth(sl).toReal(), 0);

         bool isLastLineInBlock = (line == d->lines.size() - 1);
         int sl_length = sl.length + (isLastLineInBlock ? 1 : 0); // the infamous newline


         if (sl.from > selection.start + selection.length || sl.from + sl_length <= selection.start) {
            continue;   // no actual intersection
         }

         const bool selectionStartInLine = sl.from <= selection.start;
         const bool selectionEndInLine = selection.start + selection.length < sl.from + sl_length;

         if (sl.length && (selectionStartInLine || selectionEndInLine)) {
            addSelectedRegionsToPath(d, line, position, &selection, &region, clipIfValid(lineRect, clip));
         } else {
            region.addRect(clipIfValid(lineRect, clip));
         }

         if (selection.format.boolProperty(QTextFormat::FullWidthSelection)) {
            QRectF fullLineRect(tl.rect());
            fullLineRect.translate(position);
            fullLineRect.setRight(QFIXED_MAX);

            if (!selectionEndInLine) {
               region.addRect(clipIfValid(QRectF(lineRect.topRight(), fullLineRect.bottomRight()), clip));
            }

            if (!selectionStartInLine) {
               region.addRect(clipIfValid(QRectF(fullLineRect.topLeft(), lineRect.bottomLeft()), clip));
            }

         } else if (! selectionEndInLine && isLastLineInBlock
                  && !(d->option.flags() & QTextOption::ShowLineAndParagraphSeparators)) {

            region.addRect(clipIfValid(QRectF(lineRect.right(), lineRect.top(),
                     lineRect.height() / 4, lineRect.height()), clip));
         }

      }

      {
         const QPen oldPen = p->pen();
         const QBrush oldBrush = p->brush();

         p->setPen(selection.format.penProperty(QTextFormat::OutlinePen));
         p->setBrush(selection.format.brushProperty(QTextFormat::BackgroundBrush));
         p->drawPath(region);

         p->setPen(oldPen);
         p->setBrush(oldBrush);
      }

      bool hasText = (selection.format.foreground().style() != Qt::NoBrush);
      bool hasBackground = (selection.format.background().style() != Qt::NoBrush);

      if (hasBackground) {
         selection.format.setProperty(ObjectSelectionBrush, selection.format.property(QTextFormat::BackgroundBrush));
         // don't just clear the property, set an empty brush that overrides a potential
         // background brush specified in the text
         selection.format.setProperty(QTextFormat::BackgroundBrush, QBrush());
         selection.format.clearProperty(QTextFormat::OutlinePen);
      }

      selection.format.setProperty(SuppressText, !hasText);

      if (hasText && !hasBackground && !(textDoneRegion & region).isEmpty()) {
         continue;
      }

      p->save();
      p->setClipPath(region, Qt::IntersectClip);

      for (int line = firstLine; line < lastLine; ++line) {
         QTextLine l(line, d);
         l.draw(p, position, &selection);
      }
      p->restore();

      if (hasText) {
         textDoneRegion += region;
      } else {
         if (hasBackground) {
            textDoneRegion -= region;
         }
      }

      excludedRegion += region;
   }

   QPainterPath needsTextButNoBackground = excludedRegion - textDoneRegion;
   if (!needsTextButNoBackground.isEmpty()) {
      p->save();
      p->setClipPath(needsTextButNoBackground, Qt::IntersectClip);
      FormatRange selection;
      selection.start = 0;
      selection.length = INT_MAX;
      selection.format.setProperty(SuppressBackground, true);
      for (int line = firstLine; line < lastLine; ++line) {
         QTextLine l(line, d);
         l.draw(p, position, &selection);
      }
      p->restore();
   }

   if (!excludedRegion.isEmpty()) {
      p->save();
      QPainterPath path;
      QRectF br = boundingRect().translated(position);
      br.setRight(QFIXED_MAX);

      if (! clip.isNull()) {
         br = br.intersected(clip);
      }

      path.addRect(br);
      path -= excludedRegion;
      p->setClipPath(path, Qt::IntersectClip);
   }

   for (int i = firstLine; i < lastLine; ++i) {
      QTextLine l(i, d);
      l.draw(p, position);
   }

   if (! excludedRegion.isEmpty()) {
      p->restore();
   }

   if (! d->cacheGlyphs) {
      d->freeMemory();
   }
}

void QTextLayout::drawCursor(QPainter *painter, const QPointF &pos, int cursorPosition, int width) const
{
   if (d->lines.isEmpty()) {
      return;
   }

   if (! d->layoutData) {
      d->itemize();
   }

   QPointF position = pos + d->position;

   cursorPosition = qBound(0, cursorPosition, d->layoutData->string.length());
   int line = d->lineNumberForTextPosition(cursorPosition);
   if (line < 0) {
      line = 0;
   }

   if (line >= d->lines.size()) {
      return;
   }

   QTextLine l(line, d);
   const QScriptLine &sl = d->lines[line];

   qreal x = position.x() + l.cursorToX(cursorPosition);

   int itm;

   if (d->visualCursorMovement()) {
      if (cursorPosition == sl.from + sl.length) {
         cursorPosition--;
      }

      itm = d->findItem(cursorPosition);

   } else {
      itm = d->findItem(cursorPosition - 1);
   }

   QFixed base = sl.base();
   QFixed descent = sl.descent;
   bool rightToLeft = d->isRightToLeft();

   if (itm >= 0) {
      const QScriptItem &si = d->layoutData->items.at(itm);
      if (si.ascent > 0) {
         base = si.ascent;
      }
      if (si.descent > 0) {
         descent = si.descent;
      }
      rightToLeft = si.analysis.bidiLevel % 2;
   }

   qreal y = position.y() + (sl.y + sl.base() - base).toReal();
   bool toggleAntialiasing = ! (painter->renderHints() & QPainter::Antialiasing)
         && (painter->transform().type() > QTransform::TxTranslate);

   if (toggleAntialiasing) {
      painter->setRenderHint(QPainter::Antialiasing);
   }

   painter->fillRect(QRectF(x, y, qreal(width), (base + descent).toReal()), painter->pen().brush());
   if (toggleAntialiasing) {
      painter->setRenderHint(QPainter::Antialiasing, false);
   }

   if (d->layoutData->hasBidi) {
      const int arrow_extent = 4;
      int sign = rightToLeft ? -1 : 1;
      painter->drawLine(QLineF(x, y, x + (sign * arrow_extent / 2), y + arrow_extent / 2));
      painter->drawLine(QLineF(x, y + arrow_extent, x + (sign * arrow_extent / 2), y + arrow_extent / 2));
   }

   return;
}

QRectF QTextLine::rect() const
{
   const QScriptLine &sl = m_textEngine->lines[index];
   return QRectF(sl.x.toReal(), sl.y.toReal(), sl.width.toReal(), sl.height().toReal());
}

QRectF QTextLine::naturalTextRect() const
{
   const QScriptLine &sl = m_textEngine->lines[index];
   QFixed x = sl.x + m_textEngine->alignLine(sl);

   QFixed width = sl.textWidth;
   if (sl.justified) {
      width = sl.width;
   }

   return QRectF(x.toReal(), sl.y.toReal(), width.toReal(), sl.height().toReal());
}

qreal QTextLine::x() const
{
   return m_textEngine->lines[index].x.toReal();
}

qreal QTextLine::y() const
{
   return m_textEngine->lines[index].y.toReal();
}

qreal QTextLine::width() const
{
   return m_textEngine->lines[index].width.toReal();
}

qreal QTextLine::ascent() const
{
   return m_textEngine->lines[index].ascent.toReal();
}

qreal QTextLine::descent() const
{
   return m_textEngine->lines[index].descent.toReal();
}

qreal QTextLine::height() const
{
   return m_textEngine->lines[index].height().ceil().toReal();
}

qreal QTextLine::leading() const
{
   return m_textEngine->lines[index].leading.toReal();
}

void QTextLine::setLeadingIncluded(bool included)
{
   m_textEngine->lines[index].leadingIncluded = included;
}

bool QTextLine::leadingIncluded() const
{
   return m_textEngine->lines[index].leadingIncluded;
}

qreal QTextLine::naturalTextWidth() const
{
   return m_textEngine->lines[index].textWidth.toReal();
}

qreal QTextLine::horizontalAdvance() const
{
   return m_textEngine->lines[index].textAdvance.toReal();
}

void QTextLine::setLineWidth(qreal width)
{
   QScriptLine &line = m_textEngine->lines[index];

   if (! m_textEngine->layoutData) {
      qWarning("QTextLine::setLineWidth() Unable to set a line width when there is no layout");
      return;
   }

   if (width > QFIXED_MAX) {
      width = QFIXED_MAX;
   }

   line.width = QFixed::fromReal(width);

   if (line.length && line.textWidth <= line.width && line.from + line.length == m_textEngine->layoutData->string.length())  {
      // no need to do anything if the line is already layed out and the last line. This optimization helps
      // when using things in a single line layout.

      return;
   }

   line.length    = 0;
   line.textWidth = 0;

   layout_helper(INT_MAX);
}

void QTextLine::setNumColumns(int numColumns, std::optional<qreal> alignmentWidth)
{
   QScriptLine &line = m_textEngine->lines[index];

   if (alignmentWidth.has_value()) {
      line.width        = QFixed::fromReal(alignmentWidth.value());
   } else {
      line.width        = QFIXED_MAX;
   }

   line.length       = 0;
   line.textWidth    = 0;
   layout_helper(numColumns);
}

namespace {

struct LineBreakHelper {
   LineBreakHelper()
      : glyphCount(0), maxGlyphs(0), m_currentPosition(0), fontEngine(nullptr), logClusters(nullptr),
        manualWrap(false), whiteSpaceOrObject(true) {
   }

   QScriptLine tmpData;
   QScriptLine spaceData;

   QGlyphLayout glyphs;

   int glyphCount;
   int maxGlyphs;
   int m_currentPosition;
   glyph_t previousGlyph;

   QFixed minw;
   QFixed softHyphenWidth;
   QFixed rightBearing;
   QFixed minimumRightBearing;

   QFontEngine *fontEngine;

   const unsigned short *logClusters;

   bool manualWrap;
   bool whiteSpaceOrObject;

   bool checkFullOtherwiseExtend(QScriptLine &line);

   QFixed calculateNewWidth(const QScriptLine &line) const {
      return line.textWidth + tmpData.textWidth + spaceData.textWidth + softHyphenWidth + negativeRightBearing();
   }

   inline glyph_t currentGlyph() const {
      Q_ASSERT(m_currentPosition > 0);
      Q_ASSERT(logClusters[m_currentPosition - 1] < glyphs.numGlyphs);

      return glyphs.glyphs[logClusters[m_currentPosition - 1]];
   }

   inline void saveCurrentGlyph() {
      previousGlyph = 0;

      if (m_currentPosition > 0 && (logClusters[m_currentPosition - 1] < glyphs.numGlyphs)) {
         // needed to calculate right bearing later
         previousGlyph = currentGlyph();
      }
   }

   inline void calculateRightBearing(glyph_t glyph) {
      qreal rb;
      fontEngine->getGlyphBearings(glyph, nullptr, &rb);
      rightBearing = qMin(QFixed::fromReal(rb), QFixed(0));
   }

   inline void calculateRightBearing() {
      if (m_currentPosition <= 0) {
         return;
      }

      calculateRightBearing(currentGlyph());
   }

   inline void calculateRightBearingForPreviousGlyph() {
      if (previousGlyph > 0) {
         calculateRightBearing(previousGlyph);
      }
   }

   static const QFixed RightBearingNotCalculated;

   inline void resetRightBearing() {
      rightBearing = RightBearingNotCalculated;
   }

   inline QFixed negativeRightBearing() const {
      if (rightBearing == RightBearingNotCalculated) {
         return QFixed(0);
      }

      return qAbs(rightBearing);
   }
};

const QFixed LineBreakHelper::RightBearingNotCalculated = QFixed(1);

inline bool LineBreakHelper::checkFullOtherwiseExtend(QScriptLine &line)
{
#if defined(CS_SHOW_DEBUG_GUI_TEXT)
   qDebug("checkFullOtherwiseExtend() Possible break width %f, space w = %f",
         tmpData.textWidth.toReal(), spaceData.textWidth.toReal());
#endif

   QFixed newWidth = calculateNewWidth(line);
   if (line.length && ! manualWrap && (newWidth > line.width || glyphCount > maxGlyphs)) {
      return true;
   }

   minw = qMax(minw, tmpData.textWidth);
   line += tmpData;
   line.textWidth += spaceData.textWidth;

   line.length += spaceData.length;
   tmpData.textWidth   = 0;
   tmpData.length      = 0;
   spaceData.textWidth = 0;
   spaceData.length    = 0;

   return false;
}

} // anonymous namespace

static void addNextCluster(int &pos, int end, QScriptLine &line, int &glyphCount,
   const QScriptItem &current, const QGlyphLayout &glyphs, const ushort *logClusters)
{
   int currentCluster = logClusters[pos];

   do {
      // at the first next cluster
      ++pos;
      ++line.length;
   } while (pos < end && currentCluster == logClusters[pos]);

   do {
      // calculate the textWidth for the rest of the current cluster

      if (! glyphs.attributes[currentCluster].dontPrint) {
         line.textWidth += glyphs.advances[currentCluster];
      }

      ++currentCluster;

   } while (currentCluster < current.num_glyphs && ! glyphs.attributes[currentCluster].clusterStart);

   if (pos == end) {
      // extra unprocessed glyphs
      Q_ASSERT(currentCluster == current.num_glyphs);

   } else {
      // out of sync (example: on cluster 2 and glyph 3 )
      Q_ASSERT(currentCluster == logClusters[pos]);
   }

   ++glyphCount;
}

// fill QScriptLine
void QTextLine::layout_helper(int maxGlyphs)
{
   QScriptLine &line      = m_textEngine->lines[index];
   line.length            = 0;
   line.trailingSpaces    = 0;
   line.textWidth         = 0;
   line.hasTrailingSpaces = false;

   if (! m_textEngine->layoutData->items.size() || line.from >= m_textEngine->layoutData->string.length()) {
      line.setDefaultHeight(m_textEngine);
      return;
   }

   Q_ASSERT(line.from < m_textEngine->layoutData->string.length());

   LineBreakHelper lbh;
   lbh.maxGlyphs = maxGlyphs;

   QTextOption::WrapMode wrapMode = m_textEngine->option.wrapMode();
   bool breakany  = (wrapMode == QTextOption::WrapAnywhere);
   lbh.manualWrap = (wrapMode == QTextOption::ManualWrap || wrapMode == QTextOption::NoWrap);

   int item = -1;
   int newItem = m_textEngine->findItem(line.from);
   Q_ASSERT(newItem >= 0);

#if defined(CS_SHOW_DEBUG_GUI_TEXT)
   qDebug("QTextLine::layout_helper() From = %d: Item = %d, Total = %zd, Width available = %f",
         line.from, newItem, m_textEngine->layoutData->items.size(), line.width.toReal());
#endif

   Qt::Alignment alignment = m_textEngine->option.alignment();

   const QCharAttributes *attributes = m_textEngine->attributes();
   if (! attributes) {
      return;
   }

   lbh.m_currentPosition = line.from;
   int end = 0;

   lbh.logClusters   = m_textEngine->layoutData->logClustersPtr;
   lbh.previousGlyph = 0;

   while (newItem < m_textEngine->layoutData->items.size()) {
      lbh.resetRightBearing();
      lbh.softHyphenWidth = 0;

      if (newItem != item) {
         item = newItem;

         const QScriptItem &current = m_textEngine->layoutData->items[item];

         if (! current.num_glyphs) {
            m_textEngine->shape(item);

            attributes = m_textEngine->attributes();
            if (! attributes) {
               return;
            }

            lbh.logClusters = m_textEngine->layoutData->logClustersPtr;
         }

         lbh.m_currentPosition = qMax(line.from, current.position);
         end = current.position + m_textEngine->length(item);

         lbh.glyphs = m_textEngine->shapedGlyphs(&current);

         QFontEngine *fontEngine = m_textEngine->fontEngine(current);

         if (lbh.fontEngine != fontEngine) {
            lbh.fontEngine = fontEngine;
            lbh.minimumRightBearing = qMin(QFixed(), QFixed::fromReal(fontEngine->minRightBearing()));
         }
      }

      const QScriptItem &current = m_textEngine->layoutData->items[item];

      lbh.tmpData.leading = qMax(lbh.tmpData.leading + lbh.tmpData.ascent,
            current.leading + current.ascent) - qMax(lbh.tmpData.ascent, current.ascent);

      lbh.tmpData.ascent  = qMax(lbh.tmpData.ascent,  current.ascent);
      lbh.tmpData.descent = qMax(lbh.tmpData.descent, current.descent);

      if (current.analysis.flags == QScriptAnalysis::Tab &&
         (alignment & (Qt::AlignLeft | Qt::AlignRight | Qt::AlignCenter | Qt::AlignJustify))) {
         lbh.whiteSpaceOrObject = true;

         if (lbh.checkFullOtherwiseExtend(line)) {
            goto found;
         }

         QFixed x = line.x + line.textWidth + lbh.tmpData.textWidth + lbh.spaceData.textWidth;
         QFixed tabWidth = m_textEngine->calculateTabWidth(item, x);

         attributes = m_textEngine->attributes();
         if (! attributes) {
            return;
         }

         lbh.logClusters = m_textEngine->layoutData->logClustersPtr;
         lbh.glyphs = m_textEngine->shapedGlyphs(&current);

         lbh.spaceData.textWidth += tabWidth;
         lbh.spaceData.length++;
         newItem = item + 1;

         QFixed averageCharWidth = m_textEngine->fontEngine(current)->averageCharWidth();
         lbh.glyphCount += qRound(tabWidth / averageCharWidth);

         if (lbh.checkFullOtherwiseExtend(line)) {
            goto found;
         }

      } else if (current.analysis.flags == QScriptAnalysis::LineOrParagraphSeparator) {
         lbh.whiteSpaceOrObject = true;

         // if the line consists only of the line separator make sure there is a sane height
         if (! line.length && ! lbh.tmpData.length) {
            line.setDefaultHeight(m_textEngine);
         }

         if (m_textEngine->option.flags() & QTextOption::ShowLineAndParagraphSeparators) {
            if (lbh.checkFullOtherwiseExtend(line)) {
               goto found;
            }

            addNextCluster(lbh.m_currentPosition, end, lbh.tmpData, lbh.glyphCount,
                  current, lbh.glyphs, lbh.logClusters);

         } else {
            lbh.tmpData.length++;
            lbh.calculateRightBearingForPreviousGlyph();
         }

         line += lbh.tmpData;
         goto found;

      } else if (current.analysis.flags == QScriptAnalysis::Object) {
         lbh.whiteSpaceOrObject = true;
         lbh.tmpData.length++;

         if (m_textEngine->block.docHandle()) {
            QTextInlineObject inlineObject(item, m_textEngine);
            m_textEngine->docLayout()->positionInlineObject(inlineObject,
                  m_textEngine->block.position() + current.position, inlineObject.format());
         }

         lbh.tmpData.textWidth += current.width;

         newItem = item + 1;
         ++lbh.glyphCount;

         if (lbh.checkFullOtherwiseExtend(line)) {
            goto found;
         }

      } else if (attributes[lbh.m_currentPosition].whiteSpace
                  && m_textEngine->layoutData->string.at(lbh.m_currentPosition).decompositionTag() != QChar::NoBreak) {

         lbh.whiteSpaceOrObject = true;

         while (lbh.m_currentPosition < end && attributes[lbh.m_currentPosition].whiteSpace
                  && m_textEngine->layoutData->string.at(lbh.m_currentPosition).decompositionTag() != QChar::NoBreak) {

            addNextCluster(lbh.m_currentPosition, end, lbh.spaceData, lbh.glyphCount,
               current, lbh.glyphs, lbh.logClusters);
         }

         if (! lbh.manualWrap && lbh.spaceData.textWidth > line.width) {
            lbh.spaceData.textWidth = line.width; // ignore spaces that fall out of the line.
            goto found;
         }

      } else {
         lbh.whiteSpaceOrObject = false;
         bool sb_or_ws = false;

         lbh.saveCurrentGlyph();

         do {
            addNextCluster(lbh.m_currentPosition, end, lbh.tmpData, lbh.glyphCount,
               current, lbh.glyphs, lbh.logClusters);

            bool isBreakableSpace = lbh.m_currentPosition < m_textEngine->layoutData->string.length()
               && attributes[lbh.m_currentPosition].whiteSpace
               && m_textEngine->layoutData->string.at(lbh.m_currentPosition).decompositionTag() != QChar::NoBreak;

            if (lbh.m_currentPosition >= m_textEngine->layoutData->string.length()
                  || isBreakableSpace || attributes[lbh.m_currentPosition].lineBreak) {

               sb_or_ws = true;
               break;

            } else if (breakany && attributes[lbh.m_currentPosition].graphemeBoundary) {
               break;
            }

         } while (lbh.m_currentPosition < end);

         lbh.minw = qMax(lbh.tmpData.textWidth, lbh.minw);

         if (lbh.m_currentPosition > 0 && lbh.m_currentPosition < end
            && attributes[lbh.m_currentPosition].lineBreak
            && m_textEngine->layoutData->string.at(lbh.m_currentPosition - 1).unicode() == QChar::SoftHyphen) {

            // if we are splitting up a word because of a soft hyphen then:
            //
            //  a) have to take the width of the soft hyphen into account to see if the
            //     first syllable(s) /and/ the soft hyphen fit into the line
            //
            //  b) if we are so short of available width that the soft hyphen is the first
            //     breakable position, then we do no want to show it. However we initially have
            //     to take the width for it into account so that the text document layout
            //     sees the overflow and switch to break-anywhere mode, in which we want the
            //     soft-hyphen to slip into the next line and thus become invisible again.

            if (line.length) {
               lbh.softHyphenWidth = lbh.glyphs.advances[lbh.logClusters[lbh.m_currentPosition - 1]];

            } else if (breakany) {
               lbh.tmpData.textWidth += lbh.glyphs.advances[lbh.logClusters[lbh.m_currentPosition - 1]];

            }
         }

         if (sb_or_ws | breakany) {
            // To compute the final width of the text we need to take negative right bearing
            // into account (negative right bearing means the glyph has pixel data past the
            // advance length). Note that the negative right bearing is an absolute number,
            // so that we can apply it to the width using straight forward addition.

            // Store previous right bearing (for the already accepted glyph) in case we
            // end up breaking due to the current glyph being too wide.

            QFixed previousRightBearing = lbh.rightBearing;

            // We skip calculating the right bearing if the minimum negative bearing is too
            // small to possibly expand the text beyond the edge. Note that this optimization
            // will in some cases fail, as the minimum right bearing reported by the font
            // engine may not cover all the glyphs in the font. The result is that we think
            // we don't need to break at the current glyph (because the right bearing is 0),
            // and when we then end up breaking on the next glyph we compute the right bearing
            // and end up with a line width that is slightly larger width than what was requested.
            // Unfortunately we can't remove this optimization as it will slow down text
            // layouting significantly, so we accept the slight correctnes issue.

            if ((lbh.calculateNewWidth(line) + qAbs(lbh.minimumRightBearing)) > line.width) {
               lbh.calculateRightBearing();
            }

            if (lbh.checkFullOtherwiseExtend(line)) {
               // We are too wide to accept the next glyph with its bearing, so we restore the
               // right bearing to that of the previous glyph (the one that was already accepted),
               // so that the bearing can be be applied to the final width of the text below.

               if (previousRightBearing != LineBreakHelper::RightBearingNotCalculated) {
                  lbh.rightBearing = previousRightBearing;
               } else {
                  lbh.calculateRightBearingForPreviousGlyph();
               }

               if (!breakany) {
                  line.textWidth += lbh.softHyphenWidth;
               }

               goto found;
            }
         }

         lbh.saveCurrentGlyph();
      }

      if (lbh.m_currentPosition == end) {
         newItem = item + 1;
      }
   }

   lbh.checkFullOtherwiseExtend(line);

found:
   line.textAdvance = line.textWidth;

   // If right bearing has not been calculated yet, do that now
   if (lbh.rightBearing == LineBreakHelper::RightBearingNotCalculated && ! lbh.whiteSpaceOrObject) {
      lbh.calculateRightBearing();
   }

   // apply any negative right bearing
   line.textWidth += lbh.negativeRightBearing();

   if (line.length == 0) {

#if defined(CS_SHOW_DEBUG_GUI_TEXT)
      qDebug("QTextLine::layout_helper() No break available, adding a new line");
#endif

      line += lbh.tmpData;
   }

#if defined(CS_SHOW_DEBUG_GUI_TEXT)
   qDebug("QTextLine::layout_helper() Line length = %d, Ascent = %f, Descent = %f\n   textWidth = %f, spaceWidth = %f : %s",
         line.length, line.ascent.toReal(), line.descent.toReal(), line.textWidth.toReal(),
         lbh.spaceData.width.toReal(), csPrintable(m_textEngine->layoutData->string.mid(line.from, line.length)) );
#endif


   if (lbh.manualWrap) {
      m_textEngine->minWidth = qMax(m_textEngine->minWidth, line.textWidth);
      m_textEngine->maxWidth = qMax(m_textEngine->maxWidth, line.textWidth);
   } else {
      m_textEngine->minWidth = qMax(m_textEngine->minWidth, lbh.minw);
      m_textEngine->maxWidth += line.textWidth;
   }

   if (line.textWidth > 0 && item < m_textEngine->layoutData->items.size()) {
      m_textEngine->maxWidth += lbh.spaceData.textWidth;
   }

   if (m_textEngine->option.flags() & QTextOption::IncludeTrailingSpaces) {
      line.textWidth += lbh.spaceData.textWidth;
   }

   if (lbh.spaceData.length) {
      line.trailingSpaces = lbh.spaceData.length;
      line.hasTrailingSpaces = true;
   }

   line.justified = false;
   line.gridfitted = false;

   if (m_textEngine->option.wrapMode() == QTextOption::WrapAtWordBoundaryOrAnywhere) {
      if ((lbh.maxGlyphs != INT_MAX && lbh.glyphCount > lbh.maxGlyphs)
         || (lbh.maxGlyphs == INT_MAX && line.textWidth > line.width)) {

         m_textEngine->option.setWrapMode(QTextOption::WrapAnywhere);
         line.length    = 0;
         line.textWidth = 0;

         layout_helper(lbh.maxGlyphs);
         m_textEngine->option.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
      }
   }
}

void QTextLine::setPosition(const QPointF &pos)
{
   m_textEngine->lines[index].x = QFixed::fromReal(pos.x());
   m_textEngine->lines[index].y = QFixed::fromReal(pos.y());
}

QPointF QTextLine::position() const
{
   return QPointF(m_textEngine->lines[index].x.toReal(), m_textEngine->lines[index].y.toReal());
}

// Create a text layout with a string of text. Once laid it out it contains a number of QTextLines.
// from() returns the position inside the text string where each line starts.
// Given the text, "This is a string", laid out into two lines (the second starting at the word 'a'),
// layout.lineAt(0).from() == 0 and layout.lineAt(1).from() == 8

int QTextLine::textStart() const
{
   return m_textEngine->lines[index].from;
}

int QTextLine::textLength() const
{
   if (m_textEngine->option.flags() & QTextOption::ShowLineAndParagraphSeparators
               && m_textEngine->block.isValid() && index == m_textEngine->lines.count() - 1) {
      return m_textEngine->lines[index].length - 1;
   }

   return m_textEngine->lines[index].length + m_textEngine->lines[index].trailingSpaces;
}

static void setPenAndDrawBackground(QPainter *p, const QPen &defaultPen, const QTextCharFormat &chf, const QRectF &r)
{
   QBrush c = chf.foreground();

   if (c.style() == Qt::NoBrush) {
      p->setPen(defaultPen);
   }

   QBrush bg = chf.background();
   if (bg.style() != Qt::NoBrush && !chf.property(SuppressBackground).toBool()) {
      p->fillRect(QRectF(qFloor(r.x()), qFloor(r.y()), r.width(), r.height()), bg);
   }

   if (c.style() != Qt::NoBrush) {
      p->setPen(QPen(c, 0));
   }
}

static QGlyphRun glyphRunWithInfo(QFontEngine *fontEngine, const QGlyphLayout &glyphLayout, const QPointF &pos,
   const QGlyphRun::GlyphRunFlags &flags, const QFixed &selectionX, const QFixed &selectionWidth,
   int glyphsStart, int glyphsEnd, unsigned short *logClusters, int textPosition, int textLength)
{
   Q_ASSERT(logClusters != nullptr);

   QGlyphRun glyphRun;

   QGlyphRunPrivate *d = QGlyphRunPrivate::get(glyphRun);

   int rangeStart = textPosition;
   while (*logClusters != glyphsStart && rangeStart < textPosition + textLength) {
      ++logClusters;
      ++rangeStart;
   }

   int rangeEnd = rangeStart;
   while (*logClusters != glyphsEnd && rangeEnd < textPosition + textLength) {
      ++logClusters;
      ++rangeEnd;
   }

   d->textRangeStart = rangeStart;
   d->textRangeEnd   = rangeEnd;

   // Make a font for this particular engine
   QRawFont font;
   std::shared_ptr<QRawFontPrivate> fontD = QRawFontPrivate::get(font);
   fontD->setFontEngine(fontEngine);

   QVarLengthArray<glyph_t> glyphsArray;
   QVarLengthArray<QFixedPoint> positionsArray;

   QTextItem::RenderFlags renderFlags;
   if (flags.testFlag(QGlyphRun::Overline)) {
      renderFlags |= QTextItem::Overline;
   }

   if (flags.testFlag(QGlyphRun::Underline)) {
      renderFlags |= QTextItem::Underline;
   }

   if (flags.testFlag(QGlyphRun::StrikeOut)) {
      renderFlags |= QTextItem::StrikeOut;
   }

   if (flags.testFlag(QGlyphRun::RightToLeft)) {
      renderFlags |= QTextItem::RightToLeft;
   }

   fontEngine->getGlyphPositions(glyphLayout, QTransform(), renderFlags, glyphsArray, positionsArray);
   Q_ASSERT(glyphsArray.size() == positionsArray.size());

   qreal fontHeight = font.ascent() + font.descent();
   qreal minY = 0;
   qreal maxY = 0;

   QVector<quint32> glyphs;
   glyphs.reserve(glyphsArray.size());

   QVector<QPointF> positions;
   positions.reserve(glyphsArray.size());

   for (int i = 0; i < glyphsArray.size(); ++i) {
      glyphs.append(glyphsArray.at(i) & 0xffffff);

      QPointF position = positionsArray.at(i).toPointF() + pos;
      positions.append(position);

      if (i == 0) {
         maxY = minY = position.y();
      } else {
         minY = qMin(minY, position.y());
         maxY = qMax(maxY, position.y());
      }
   }

   qreal height = maxY + fontHeight - minY;

   glyphRun.setGlyphIndexes(glyphs);
   glyphRun.setPositions(positions);
   glyphRun.setFlags(flags);
   glyphRun.setRawFont(font);

   glyphRun.setBoundingRect(QRectF(selectionX.toReal(), minY - font.ascent(),
         selectionWidth.toReal(), height));

   return glyphRun;
}

QList<QGlyphRun> QTextLine::glyphRuns(int from, int length) const
{
   const QScriptLine &line = m_textEngine->lines[index];

   if (line.length == 0) {
      return QList<QGlyphRun>();
   }

   if (from < 0) {
      from = textStart();
   }

   if (length < 0) {
      length = textLength();
   }

   if (length == 0) {
      return QList<QGlyphRun>();
   }

   QTextLayout::FormatRange selection;
   selection.start  = from;
   selection.length = length;

   QTextLineItemIterator iterator(m_textEngine, index, QPointF(), &selection);
   qreal y = line.y.toReal() + line.base().toReal();
   QList<QGlyphRun> glyphRuns;

   while (! iterator.atEnd()) {
      QScriptItem &si = iterator.next();

      if (si.analysis.flags >= QScriptAnalysis::TabOrObject) {
         continue;
      }

      if (from >= 0 && length >= 0 && (from >= iterator.itemEnd || from + length <= iterator.itemStart)) {
         continue;
      }

      QPointF pos(iterator.x.toReal(), y);
      QFont font;

      QGlyphRun::GlyphRunFlags flags;

      if (! m_textEngine->useRawFont) {
         font = m_textEngine->font(si);
         if (font.overline()) {
            flags |= QGlyphRun::Overline;
         }

         if (font.underline()) {
            flags |= QGlyphRun::Underline;
         }

         if (font.strikeOut()) {
            flags |= QGlyphRun::StrikeOut;
         }
      }

      bool rtl = false;
      if (si.analysis.bidiLevel % 2) {
         flags |= QGlyphRun::RightToLeft;
         rtl = true;
      }

      int relativeFrom = qMax(iterator.itemStart, from) - si.position;
      int relativeTo   = qMin(iterator.itemEnd, from + length) - 1 - si.position;

      unsigned short *logClusters = m_textEngine->logClusters(&si);
      int glyphsStart = logClusters[relativeFrom];
      int glyphsEnd   = (relativeTo == iterator.itemLength) ? si.num_glyphs - 1 : logClusters[relativeTo];

      // the glyph index right next to the requested range
      int nextGlyphIndex = (relativeTo < iterator.itemLength - 1) ? logClusters[relativeTo + 1] : si.num_glyphs;
      if (nextGlyphIndex - 1 > glyphsEnd) {
         glyphsEnd = nextGlyphIndex - 1;
      }

      bool startsInsideLigature = relativeFrom > 0 && logClusters[relativeFrom - 1] == glyphsStart;
      bool endsInsideLigature = nextGlyphIndex == glyphsEnd;

      int itemGlyphsStart = logClusters[iterator.itemStart - si.position];
      int itemGlyphsEnd   = logClusters[iterator.itemEnd - 1 - si.position];

      QGlyphLayout glyphLayout = m_textEngine->shapedGlyphs(&si);

      if (relativeFrom != (iterator.itemStart - si.position) && !rtl) {
         for (int i = itemGlyphsStart; i < glyphsStart; ++i) {
            QFixed justification = QFixed::fromFixed(glyphLayout.justifications[i].space_18d6);
            pos.rx() += (glyphLayout.advances[i] + justification).toReal();
         }

      } else if (relativeTo != (iterator.itemEnd - si.position - 1) && rtl) {
         for (int i = itemGlyphsEnd; i > glyphsEnd; --i) {
            QFixed justification = QFixed::fromFixed(glyphLayout.justifications[i].space_18d6);
            pos.rx() += (glyphLayout.advances[i] + justification).toReal();
         }
      }

      glyphLayout = glyphLayout.mid(glyphsStart, glyphsEnd - glyphsStart + 1);

      QFixed x;
      QFixed width;
      iterator.getSelectionBounds(&x, &width);

      if (glyphLayout.numGlyphs > 0) {
         QFontEngine *mainFontEngine;

         if (m_textEngine->useRawFont && m_textEngine->rawFont.isValid()) {
            mainFontEngine = m_textEngine->fontEngine(si);

         } else {
            mainFontEngine = font.d->engineForScript(si.analysis.script);
         }

         if (mainFontEngine->type() == QFontEngine::Multi) {
            QFontEngineMulti *multiFontEngine = static_cast<QFontEngineMulti *>(mainFontEngine);

            int start = rtl ? glyphLayout.numGlyphs : 0;
            int end = start - 1;
            int which = glyphLayout.glyphs[rtl ? start - 1 : end + 1] >> 24;

            for (; (rtl && start > 0) || (!rtl && end < glyphLayout.numGlyphs - 1);
               rtl ? --start : ++end) {
               const int e = glyphLayout.glyphs[rtl ? start - 1 : end + 1] >> 24;
               if (e == which) {
                  continue;
               }

               QGlyphLayout subLayout = glyphLayout.mid(start, end - start + 1);
               multiFontEngine->ensureEngineAt(which);

               QGlyphRun::GlyphRunFlags subFlags = flags;
               if (start == 0 && startsInsideLigature) {
                  subFlags |= QGlyphRun::SplitLigature;
               }

               glyphRuns.append(glyphRunWithInfo(multiFontEngine->engine(which),
                     subLayout, pos, subFlags, x, width, glyphsStart + start, glyphsStart + end,
                     logClusters + relativeFrom, relativeFrom + si.position,
                     relativeTo - relativeFrom + 1));

               for (int i = 0; i < subLayout.numGlyphs; ++i) {
                  QFixed justification = QFixed::fromFixed(subLayout.justifications[i].space_18d6);
                  pos.rx() += (subLayout.advances[i] + justification).toReal();
               }

               if (rtl) {
                  end = start - 1;
               } else {
                  start = end + 1;
               }
               which = e;
            }

            QGlyphLayout subLayout = glyphLayout.mid(start, end - start + 1);
            multiFontEngine->ensureEngineAt(which);

            QGlyphRun::GlyphRunFlags subFlags = flags;
            if ((start == 0 && startsInsideLigature) || endsInsideLigature) {
               subFlags |= QGlyphRun::SplitLigature;
            }

            QGlyphRun glyphRun = glyphRunWithInfo(multiFontEngine->engine(which),
                  subLayout, pos, subFlags, x, width, glyphsStart + start, glyphsStart + end,
                  logClusters + relativeFrom, relativeFrom + si.position,
                  relativeTo - relativeFrom + 1);

            if (! glyphRun.isEmpty()) {
               glyphRuns.append(glyphRun);
            }

         } else {
            if (startsInsideLigature || endsInsideLigature) {
               flags |= QGlyphRun::SplitLigature;
            }
            QGlyphRun glyphRun = glyphRunWithInfo(mainFontEngine,
                  glyphLayout, pos, flags, x, width, glyphsStart, glyphsEnd,
                  logClusters + relativeFrom, relativeFrom + si.position,
                  relativeTo - relativeFrom + 1);

            if (! glyphRun.isEmpty()) {
               glyphRuns.append(glyphRun);
            }
         }
      }
   }

   return glyphRuns;
}

void QTextLine::draw(QPainter *p, const QPointF &pos, const QTextLayout::FormatRange *selection) const
{
   // can not use this method with rawfont
   Q_ASSERT(! m_textEngine->useRawFont);

   const QScriptLine &line = m_textEngine->lines[index];
   QPen pen = p->pen();

   bool noText = (selection && selection->format.property(SuppressText).toBool());

   if (! line.length) {
      if (selection && selection->start <= line.from && selection->start + selection->length > line.from) {
         const qreal lineHeight = line.height().toReal();

         QRectF r(pos.x() + line.x.toReal(), pos.y() + line.y.toReal(),
            lineHeight / 2, QFontMetrics(m_textEngine->font()).width(QChar(' ')));

         setPenAndDrawBackground(p, QPen(), selection->format, r);
         p->setPen(pen);
      }
      return;
   }

   QTextLineItemIterator iterator(m_textEngine, index, pos, selection);

   QFixed lineBase = line.base();
   m_textEngine->clearDecorations();
   m_textEngine->enableDelayDecorations();

   const QFixed y = QFixed::fromReal(pos.y()) + line.y + lineBase;

   bool suppressColors = (m_textEngine->option.flags() & QTextOption::SuppressColors);

   while (! iterator.atEnd()) {
      QScriptItem &si = iterator.next();

      if (selection && selection->start >= 0 && iterator.isOutsideSelection()) {
         continue;
      }

      if (si.analysis.flags == QScriptAnalysis::LineOrParagraphSeparator
                  && ! (m_textEngine->option.flags() & QTextOption::ShowLineAndParagraphSeparators)) {
         continue;
      }

      QFixed itemBaseLine = y;
      QFont f = m_textEngine->font(si);
      QTextCharFormat format;

      if (m_textEngine->hasFormats() || selection) {
         format = m_textEngine->format(&si);

         if (suppressColors) {
            format.clearForeground();
            format.clearBackground();
            format.clearProperty(QTextFormat::TextUnderlineColor);
         }

         if (selection) {
            format.merge(selection->format);
         }

         setPenAndDrawBackground(p, pen, format, QRectF(iterator.x.toReal(), (y - lineBase).toReal(),
               iterator.itemWidth.toReal(), line.height().toReal()));

         QTextCharFormat::VerticalAlignment valign = format.verticalAlignment();

         if (valign == QTextCharFormat::AlignSuperScript || valign == QTextCharFormat::AlignSubScript) {
            QFontEngine *fe = f.d->engineForScript(si.analysis.script);
            QFixed height = fe->ascent() + fe->descent();

            if (valign == QTextCharFormat::AlignSubScript) {
               itemBaseLine += height / 6;

            } else if (valign == QTextCharFormat::AlignSuperScript) {
               itemBaseLine -= height / 2;
            }
         }
      }

      if (si.analysis.flags >= QScriptAnalysis::TabOrObject) {

         if (m_textEngine->hasFormats()) {
            p->save();

            if (si.analysis.flags == QScriptAnalysis::Object && m_textEngine->block.docHandle()) {
               QFixed itemY = y - si.ascent;

               if (format.verticalAlignment() == QTextCharFormat::AlignTop) {
                  itemY = y - lineBase;
               }

               QRectF itemRect(iterator.x.toReal(), itemY.toReal(), iterator.itemWidth.toReal(),
                  si.height().toReal());

               m_textEngine->docLayout()->drawInlineObject(p, itemRect, QTextInlineObject(iterator.item, m_textEngine),
                  si.position + m_textEngine->block.position(), format);

               if (selection) {
                  QBrush bg = format.brushProperty(ObjectSelectionBrush);

                  if (bg.style() != Qt::NoBrush) {
                     QColor c = bg.color();
                     c.setAlpha(128);
                     p->fillRect(itemRect, c);
                  }
               }

            } else {
               // si.isTab

               QFont f = m_textEngine->font(si);

               QTextItemInt gf(si, &f, format);
               gf.width = iterator.itemWidth;

               QPainterPrivate::get(p)->drawTextItem(QPointF(iterator.x.toReal(), y.toReal()), gf, m_textEngine);

               if (m_textEngine->option.flags() & QTextOption::ShowTabsAndSpaces) {
                  QChar visualTab(0x2192);

                  int w   = QFontMetrics(f).width(visualTab);
                  qreal x = iterator.itemWidth.toReal() - w; // Right-aligned

                  if (x < 0) {
                     p->setClipRect(QRectF(iterator.x.toReal(), line.y.toReal(),
                           iterator.itemWidth.toReal(), line.height().toReal()), Qt::IntersectClip);
                  } else {
                     x /= 2;   // Centered
                  }

                  p->drawText(QPointF(iterator.x.toReal() + x, y.toReal()), visualTab);
               }

            }

            p->restore();
         }

         continue;
      }

      unsigned short *logClusters = m_textEngine->logClusters(&si);
      QGlyphLayout glyphs = m_textEngine->shapedGlyphs(&si);

      QStringView tmp = m_textEngine->layoutData->string.midView(iterator.itemStart, iterator.itemEnd - iterator.itemStart);

      QTextItemInt gf(glyphs.mid(iterator.glyphsStart, iterator.glyphsEnd - iterator.glyphsStart),
         &f, tmp.begin(), tmp.end(), m_textEngine->fontEngine(si), format);

      gf.logClusters = logClusters + iterator.itemStart - si.position;
      gf.width       = iterator.itemWidth;
      gf.justified   = line.justified;
      gf.initWithScriptItem(si);

      Q_ASSERT(gf.fontEngine);

      QPointF pos(iterator.x.toReal(), itemBaseLine.toReal());

      if (format.penProperty(QTextFormat::TextOutline).style() != Qt::NoPen) {
         QPainterPath path;
         path.setFillRule(Qt::WindingFill);

         if (gf.glyphs.numGlyphs) {
            gf.fontEngine->addOutlineToPath(pos.x(), pos.y(), gf.glyphs, &path, gf.flags);
         }

         if (gf.flags) {
            const QFontEngine *fe = gf.fontEngine;
            const qreal lw = fe->lineThickness().toReal();

            if (gf.flags & QTextItem::Underline) {
               qreal offs = fe->underlinePosition().toReal();
               path.addRect(pos.x(), pos.y() + offs, gf.width.toReal(), lw);
            }

            if (gf.flags & QTextItem::Overline) {
               qreal offs = fe->ascent().toReal() + 1;
               path.addRect(pos.x(), pos.y() - offs, gf.width.toReal(), lw);
            }

            if (gf.flags & QTextItem::StrikeOut) {
               qreal offs = fe->ascent().toReal() / 3;
               path.addRect(pos.x(), pos.y() - offs, gf.width.toReal(), lw);
            }
         }

         p->save();
         p->setRenderHint(QPainter::Antialiasing);

         // Currently QPen with a Qt::NoPen style still returns a default
         // QBrush which != Qt::NoBrush so we need this specialcase to reset it

         if (p->pen().style() == Qt::NoPen) {
            p->setBrush(Qt::NoBrush);
         } else {
            p->setBrush(p->pen().brush());
         }

         p->setPen(format.textOutline());
         p->drawPath(path);
         p->restore();

      } else {
         if (noText) {
            gf.glyphs.numGlyphs = 0;   // slightly less elegant than it should be
         }

         QPainterPrivate::get(p)->drawTextItem(pos, gf, m_textEngine);
      }

      if (si.analysis.flags == QScriptAnalysis::Space && (m_textEngine->option.flags() & QTextOption::ShowTabsAndSpaces)) {
         QBrush c = format.foreground();

         if (c.style() != Qt::NoBrush) {
            p->setPen(c.color());
         }

         QChar visualSpace((ushort)0xb7);
         p->drawText(QPointF(iterator.x.toReal(), itemBaseLine.toReal()), visualSpace);
         p->setPen(pen);
      }
   }

   m_textEngine->drawDecorations(p);

   if (m_textEngine->hasFormats()) {
      p->setPen(pen);
   }
}

qreal QTextLine::cursorToX(int *cursorPos, Edge edge) const
{
   const QScriptLine &line = m_textEngine->lines[index];
   bool lastLine = index >= m_textEngine->lines.size() - 1;

   QFixed x = line.x + m_textEngine->alignLine(line) - m_textEngine->leadingSpaceWidth(line);
   if (! m_textEngine->layoutData) {
      m_textEngine->itemize();
   }

   if (! m_textEngine->layoutData->items.size()) {
      *cursorPos = line.from;
      return x.toReal();
   }

   int lineEnd = line.from + line.length + line.trailingSpaces;
   int pos = qBound(line.from, *cursorPos, lineEnd);
   int itm;

   const QCharAttributes *attributes = m_textEngine->attributes();
   if (!attributes) {
      *cursorPos = line.from;
      return x.toReal();
   }

   while (pos < lineEnd && ! attributes[pos].graphemeBoundary) {
      pos++;
   }

   if (pos == lineEnd) {
      // end of line ensure we have the last item on the line
      itm = m_textEngine->findItem(pos - 1);
   } else {
      itm = m_textEngine->findItem(pos);
   }

   if (itm < 0) {
      *cursorPos = line.from;
      return x.toReal();
   }

   m_textEngine->shapeLine(line);

   const QScriptItem *si = &m_textEngine->layoutData->items[itm];
   if (! si->num_glyphs) {
      m_textEngine->shape(itm);
   }

   const int max = m_textEngine->length(itm);
   pos = qBound(0, pos - si->position, max);

   QGlyphLayout glyphs = m_textEngine->shapedGlyphs(si);
   unsigned short *logClusters = m_textEngine->logClusters(si);
   Q_ASSERT(logClusters);

   int glyph_pos = pos == max ? si->num_glyphs : logClusters[pos];

   if (edge == Trailing && glyph_pos < si->num_glyphs) {
      // trailing edge is leading edge of next cluster
      glyph_pos++;

      while (glyph_pos < si->num_glyphs && !glyphs.attributes[glyph_pos].clusterStart) {
         glyph_pos++;
      }
   }

   bool reverse = si->analysis.bidiLevel % 2;

   // add the items left of the cursor
   int firstItem = m_textEngine->findItem(line.from);
   int lastItem  = m_textEngine->findItem(lineEnd - 1, itm);
   int nItems = (firstItem >= 0 && lastItem >= firstItem) ? (lastItem - firstItem + 1) : 0;

   QVarLengthArray<int> visualOrder(nItems);
   QVarLengthArray<uchar> levels(nItems);

   for (int i = 0; i < nItems; ++i) {
      levels[i] = m_textEngine->layoutData->items[i + firstItem].analysis.bidiLevel;
   }
   QTextEngine::bidiReorder(nItems, levels.data(), visualOrder.data());

   for (int i = 0; i < nItems; ++i) {
      int item = visualOrder[i] + firstItem;
      if (item == itm) {
         break;
      }

      QScriptItem &si = m_textEngine->layoutData->items[item];
      if (! si.num_glyphs) {
         m_textEngine->shape(item);
      }

      if (si.analysis.flags >= QScriptAnalysis::TabOrObject) {
         x += si.width;
         continue;
      }
      const int itemLength = m_textEngine->length(item);
      int start = qMax(line.from, si.position);
      int end   = qMin(lineEnd, si.position + itemLength);

      logClusters = m_textEngine->logClusters(&si);

      int gs = logClusters[start - si.position];
      int ge = (end == si.position + itemLength) ? si.num_glyphs - 1 : logClusters[end - si.position - 1];

      QGlyphLayout glyphs = m_textEngine->shapedGlyphs(&si);

      while (gs <= ge) {
         x += glyphs.effectiveAdvance(gs);
         ++gs;
      }
   }

   logClusters = m_textEngine->logClusters(si);
   glyphs      = m_textEngine->shapedGlyphs(si);

   if (si->analysis.flags >= QScriptAnalysis::TabOrObject) {
      if (pos == (reverse ? 0 : max)) {
         x += si->width;
      }

   } else {
      bool rtl    = m_textEngine->isRightToLeft();
      bool visual = m_textEngine->visualCursorMovement();
      int end     = qMin(lineEnd, si->position + max) - si->position;

      if (reverse) {
         int glyph_end;

         if (end == max) {
             glyph_end  = si->num_glyphs;
         } else {
            glyph_end = logClusters[end];
         }

         int glyph_start = glyph_pos;

         if (visual && ! rtl && ! (lastLine && itm == (visualOrder[nItems - 1] + firstItem))) {
            ++glyph_start;
         }

         for (int i = glyph_end - 1; i >= glyph_start; i--) {
            x += glyphs.effectiveAdvance(i);
         }

      } else {
         int start       = qMax(line.from - si->position, 0);
         int glyph_start = logClusters[start];
         int glyph_end   = glyph_pos;

         if (! visual || ! rtl || (lastLine && itm == visualOrder[0] + firstItem)) {
            --glyph_end;
         }

         for (int i = glyph_start; i <= glyph_end; i++) {
            x += glyphs.effectiveAdvance(i);
         }
      }

      x += m_textEngine->offsetInLigature(si, pos, end, glyph_pos);
   }

   if (m_textEngine->option.wrapMode() != QTextOption::NoWrap && x > line.x + line.width) {
      x = line.x + line.width;
   }

   if (m_textEngine->option.wrapMode() != QTextOption::NoWrap && x < 0) {
      x = 0;
   }

   *cursorPos = pos + si->position;

   return x.toReal();
}

int QTextLine::xToCursor(qreal _x, CursorPosition cpos) const
{
   QFixed x = QFixed::fromReal(_x);
   const QScriptLine &line = m_textEngine->lines[index];
   bool lastLine = index  >= m_textEngine->lines.size() - 1;
   int lineNum = index;

   if (! m_textEngine->layoutData) {
      m_textEngine->itemize();
   }

   int line_length = textLength();

   if (! line_length) {
      return line.from;
   }

   int firstItem = m_textEngine->findItem(line.from);
   int lastItem  = m_textEngine->findItem(line.from + line_length - 1, firstItem);
   int nItems = (firstItem >= 0 && lastItem >= firstItem) ? (lastItem - firstItem + 1) : 0;

   if (! nItems) {
      return 0;
   }

   x -= line.x;
   x -= m_textEngine->alignLine(line);

   QVarLengthArray<int> visualOrder(nItems);
   QVarLengthArray<unsigned char> levels(nItems);

   for (int i = 0; i < nItems; ++i) {
      levels[i] = m_textEngine->layoutData->items[i + firstItem].analysis.bidiLevel;
   }

   QTextEngine::bidiReorder(nItems, levels.data(), visualOrder.data());

   bool visual = m_textEngine->visualCursorMovement();
   if (x <= 0) {
      // left of first item
      int item = visualOrder[0] + firstItem;
      QScriptItem &si = m_textEngine->layoutData->items[item];

      if (!si.num_glyphs) {
         m_textEngine->shape(item);
      }

      int pos = si.position;
      if (si.analysis.bidiLevel % 2) {
         pos += m_textEngine->length(item);
      }

      pos = qMax(line.from, pos);
      pos = qMin(line.from + line_length, pos);
      return pos;

   } else if (x < line.textWidth || (line.justified && x < line.width)) {
      // has to be in one of the runs
      QFixed pos;
      bool rtl = m_textEngine->isRightToLeft();

      m_textEngine->shapeLine(line);
      QVector<int> insertionPoints;
      if (visual && rtl) {
         m_textEngine->insertionPointsForLine(lineNum, insertionPoints);
      }

      int nchars = 0;

      for (int i = 0; i < nItems; ++i) {
         int item = visualOrder[i] + firstItem;
         QScriptItem &si = m_textEngine->layoutData->items[item];

         int item_length = m_textEngine->length(item);
         int start = qMax(line.from - si.position, 0);
         int end   = qMin(line.from + line_length - si.position, item_length);

         unsigned short *logClusters = m_textEngine->logClusters(&si);

         int gs = logClusters[start];
         int ge = (end == item_length ? si.num_glyphs : logClusters[end]) - 1;
         QGlyphLayout glyphs = m_textEngine->shapedGlyphs(&si);

         QFixed item_width = 0;
         if (si.analysis.flags >= QScriptAnalysis::TabOrObject) {
            item_width = si.width;

         } else {
            int g = gs;
            while (g <= ge) {
               item_width += glyphs.effectiveAdvance(g);
               ++g;
            }
         }

         if (pos + item_width < x) {
            pos += item_width;
            nchars += end;
            continue;
         }

         if (si.analysis.flags >= QScriptAnalysis::TabOrObject) {
            if (cpos == QTextLine::CursorOnCharacter) {
               return si.position;
            }
            bool left_half = (x - pos) < item_width / 2;

            if (bool(si.analysis.bidiLevel % 2) != left_half) {
               return si.position;
            }
            return si.position + 1;
         }

         int glyph_pos = -1;
         QFixed edge;

         // has to be inside run
         if (cpos == QTextLine::CursorOnCharacter) {
            if (si.analysis.bidiLevel % 2) {
               pos += item_width;
               glyph_pos = gs;

               while (gs <= ge) {
                  if (glyphs.attributes[gs].clusterStart) {
                     if (pos < x) {
                        break;
                     }
                     glyph_pos = gs;
                     edge = pos;

                  }
                  pos -= glyphs.effectiveAdvance(gs);
                  ++gs;
               }

            } else {
               glyph_pos = gs;
               while (gs <= ge) {
                  if (glyphs.attributes[gs].clusterStart) {
                     if (pos > x) {
                        break;
                     }
                     glyph_pos = gs;
                     edge = pos;
                  }
                  pos += glyphs.effectiveAdvance(gs);
                  ++gs;
               }
            }

         } else {
            QFixed dist = INT_MAX / 256;
            if (si.analysis.bidiLevel % 2) {
               if (!visual || rtl || (lastLine && i == nItems - 1)) {
                  pos += item_width;
                  while (gs <= ge) {
                     if (glyphs.attributes[gs].clusterStart && qAbs(x - pos) < dist) {
                        glyph_pos = gs;
                        edge = pos;
                        dist = qAbs(x - pos);
                     }
                     pos -= glyphs.effectiveAdvance(gs);
                     ++gs;
                  }

               } else {
                  while (ge >= gs) {
                     if (glyphs.attributes[ge].clusterStart && qAbs(x - pos) < dist) {
                        glyph_pos = ge;
                        edge = pos;
                        dist = qAbs(x - pos);
                     }
                     pos += glyphs.effectiveAdvance(ge);
                     --ge;
                  }
               }

            } else {
               if (!visual || !rtl || (lastLine && i == 0)) {
                  while (gs <= ge) {
                     if (glyphs.attributes[gs].clusterStart && qAbs(x - pos) < dist) {
                        glyph_pos = gs;
                        edge = pos;
                        dist = qAbs(x - pos);
                     }
                     pos += glyphs.effectiveAdvance(gs);
                     ++gs;
                  }
               } else {
                  QFixed oldPos = pos;
                  while (gs <= ge) {
                     pos += glyphs.effectiveAdvance(gs);
                     if (glyphs.attributes[gs].clusterStart && qAbs(x - pos) < dist) {
                        glyph_pos = gs;
                        edge = pos;
                        dist = qAbs(x - pos);
                     }
                     ++gs;
                  }
                  pos = oldPos;
               }
            }

            if (qAbs(x - pos) < dist) {
               if (visual) {
                  if (! rtl && i < nItems - 1) {
                     nchars += end;
                     continue;
                  }
                  if (rtl && nchars > 0) {
                     return insertionPoints[lastLine ? nchars : nchars - 1];
                  }
               }
               return m_textEngine->positionInLigature(&si, end, x, pos, -1, cpos == QTextLine::CursorOnCharacter);
            }
         }

         Q_ASSERT(glyph_pos != -1);
         return m_textEngine->positionInLigature(&si, end, x, edge, glyph_pos, cpos == QTextLine::CursorOnCharacter);
      }
   }

   // right of last item
   int item = visualOrder[nItems - 1] + firstItem;
   QScriptItem &si = m_textEngine->layoutData->items[item];

   if (! si.num_glyphs) {
      m_textEngine->shape(item);
   }

   int pos = si.position;
   if (! (si.analysis.bidiLevel % 2)) {
      pos += m_textEngine->length(item);
   }

   pos = qMax(line.from, pos);
   int maxPos = line.from + line_length;

   // except for the last line we assume that the character between lines is a space and we want
   // to position the cursor to the left of that character.

   // ### breaks with japanese for example
   if (this->index < m_textEngine->lines.count() - 1) {
      --maxPos;
   }

   pos = qMin(pos, maxPos);

   return pos;
}
