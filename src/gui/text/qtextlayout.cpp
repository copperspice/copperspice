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

#include <qtextlayout.h>
#include <qtextengine_p.h>

#include <qfont.h>
#include <qapplication.h>
#include <qmath.h>
#include <qpainter.h>
#include <qvarlengtharray.h>
#include <qtextformat.h>
#include <qabstracttextdocumentlayout.h>
#include <qtextdocument_p.h>
#include <qtextformat_p.h>
#include <qstyleoption.h>
#include <qpainterpath.h>
#include <qglyphrun.h>
#include <qglyphrun_p.h>
#include <qrawfont.h>
#include <qrawfont_p.h>
#include <limits.h>

#include <qdebug.h>

#include <qfontengine_p.h>

#if !defined(QT_NO_FREETYPE)
#  include <qfontengine_ft_p.h>
#endif

QT_BEGIN_NAMESPACE

#define ObjectSelectionBrush (QTextFormat::ForegroundBrush + 1)
#define SuppressText 0x5012
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
   if (!eng->block.docHandle()) {
      return QTextFormat();
   }
   return eng->formats()->format(eng->formatIndex(&eng->layoutData->items[itm]));
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
   d = new QTextEngine((text.isNull() ? (const QString &)QString::fromLatin1("") : text), f.d.data());
}

QTextLayout::QTextLayout(const QTextBlock &block)
{
   d = new QTextEngine();
   d->block = block;
}

QTextLayout::~QTextLayout()
{
   if (!d->stackEngine) {
      delete d;
   }
}

void QTextLayout::setFont(const QFont &font)
{
   d->fnt = font;
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

QTextOption QTextLayout::textOption() const
{
   return d->option;
}

void QTextLayout::setPreeditArea(int position, const QString &text)
{
   if (text.isEmpty()) {
      if (!d->specialData) {
         return;
      }
      if (d->specialData->addFormats.isEmpty()) {
         delete d->specialData;
         d->specialData = 0;
      } else {
         d->specialData->preeditText = QString();
         d->specialData->preeditPosition = -1;
      }
   } else {
      if (!d->specialData) {
         d->specialData = new QTextEngine::SpecialData;
      }
      d->specialData->preeditPosition = position;
      d->specialData->preeditText = text;
   }
   d->invalidate();
   d->clearLineData();
   if (d->block.docHandle()) {
      d->block.docHandle()->documentChange(d->block.position(), d->block.length());
   }
}

int QTextLayout::preeditAreaPosition() const
{
   return d->specialData ? d->specialData->preeditPosition : -1;
}

QString QTextLayout::preeditAreaText() const
{
   return d->specialData ? d->specialData->preeditText : QString();
}

void QTextLayout::setAdditionalFormats(const QList<FormatRange> &formatList)
{
   if (formatList.isEmpty()) {
      if (!d->specialData) {
         return;
      }
      if (d->specialData->preeditText.isEmpty()) {
         delete d->specialData;
         d->specialData = 0;
      } else {
         d->specialData->addFormats = formatList;
         d->specialData->addFormatIndices.clear();
      }
   } else {
      if (!d->specialData) {
         d->specialData = new QTextEngine::SpecialData;
         d->specialData->preeditPosition = -1;
      }
      d->specialData->addFormats = formatList;
      d->indexAdditionalFormats();
   }
   if (d->block.docHandle()) {
      d->block.docHandle()->documentChange(d->block.position(), d->block.length());
   }
   d->resetFontEngineCache();
}

QList<QTextLayout::FormatRange> QTextLayout::additionalFormats() const
{
   QList<FormatRange> formats;
   if (!d->specialData) {
      return formats;
   }

   formats = d->specialData->addFormats;

   if (d->specialData->addFormatIndices.isEmpty()) {
      return formats;
   }

   const QTextFormatCollection *collection = d->formats();

   for (int i = 0; i < d->specialData->addFormatIndices.count(); ++i) {
      formats[i].format = collection->charFormat(d->specialData->addFormatIndices.at(i));
   }

   return formats;
}


void QTextLayout::clearAdditionalFormats()
{
   setAdditionalFormats(QList<FormatRange>());
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
   d->visualMovement = style == Qt::VisualMoveStyle ? true : false;
}

Qt::CursorMoveStyle QTextLayout::cursorMoveStyle() const
{
   return d->visualMovement ? Qt::VisualMoveStyle : Qt::LogicalMoveStyle;
}

void QTextLayout::beginLayout()
{
#ifndef QT_NO_DEBUG
   if (d->layoutData && d->layoutData->layoutState == QTextEngine::InLayout) {
      qWarning("QTextLayout::beginLayout: Called while already doing layout");
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
#ifndef QT_NO_DEBUG
   if (!d->layoutData || d->layoutData->layoutState == QTextEngine::LayoutEmpty) {
      qWarning("QTextLayout::endLayout: Called without beginLayout()");
      return;
   }
#endif
   int l = d->lines.size();
   if (l && d->lines.at(l - 1).length < 0) {
      QTextLine(l - 1, d).setNumColumns(INT_MAX);
   }
   d->layoutData->layoutState = QTextEngine::LayoutEmpty;
   if (!d->cacheGlyphs) {
      d->freeMemory();
   }
}

void QTextLayout::clearLayout()
{
   d->clearLineData();
}

int QTextLayout::nextCursorPosition(int oldPos, CursorMode mode) const
{
   const HB_CharAttributes *attributes = d->attributes();
   int len = d->block.isValid() ? d->block.length() - 1
             : d->layoutData->string.length();
   Q_ASSERT(len <= d->layoutData->string.length());
   if (!attributes || oldPos < 0 || oldPos >= len) {
      return oldPos;
   }

   if (mode == SkipCharacters) {
      oldPos++;
      while (oldPos < len && !attributes[oldPos].charStop) {
         oldPos++;
      }
   } else {
      if (oldPos < len && d->atWordSeparator(oldPos)) {
         oldPos++;
         while (oldPos < len && d->atWordSeparator(oldPos)) {
            oldPos++;
         }
      } else {
         while (oldPos < len && !d->atSpace(oldPos) && !d->atWordSeparator(oldPos)) {
            oldPos++;
         }
      }
      while (oldPos < len && d->atSpace(oldPos)) {
         oldPos++;
      }
   }

   return oldPos;
}

int QTextLayout::previousCursorPosition(int oldPos, CursorMode mode) const
{
   const HB_CharAttributes *attributes = d->attributes();
   if (!attributes || oldPos <= 0 || oldPos > d->layoutData->string.length()) {
      return oldPos;
   }

   if (mode == SkipCharacters) {
      oldPos--;
      while (oldPos && !attributes[oldPos].charStop) {
         oldPos--;
      }
   } else {
      while (oldPos && d->atSpace(oldPos - 1)) {
         oldPos--;
      }

      if (oldPos && d->atWordSeparator(oldPos - 1)) {
         oldPos--;
         while (oldPos && d->atWordSeparator(oldPos - 1)) {
            oldPos--;
         }
      } else {
         while (oldPos && !d->atSpace(oldPos - 1) && !d->atWordSeparator(oldPos - 1)) {
            oldPos--;
         }
      }
   }

   return oldPos;
}

int QTextLayout::rightCursorPosition(int oldPos) const
{
   int newPos = d->positionAfterVisualMovement(oldPos, QTextCursor::Right);
   //    qDebug("%d -> %d", oldPos, newPos);
   return newPos;
}

int QTextLayout::leftCursorPosition(int oldPos) const
{
   int newPos = d->positionAfterVisualMovement(oldPos, QTextCursor::Left);
   //    qDebug("%d -> %d", oldPos, newPos);
   return newPos;
}

bool QTextLayout::isValidCursorPosition(int pos) const
{
   const HB_CharAttributes *attributes = d->attributes();
   if (!attributes || pos < 0 || pos > (int)d->layoutData->string.length()) {
      return false;
   }
   return attributes[pos].charStop;
}

QTextLine QTextLayout::createLine()
{
#ifndef QT_NO_DEBUG
   if (!d->layoutData || d->layoutData->layoutState == QTextEngine::LayoutEmpty) {
      qWarning("QTextLayout::createLine: Called without layouting");
      return QTextLine();
   }
#endif
   if (d->layoutData->layoutState == QTextEngine::LayoutFailed) {
      return QTextLine();
   }

   int l = d->lines.size();
   if (l && d->lines.at(l - 1).length < 0) {
      QTextLine(l - 1, d).setNumColumns(INT_MAX);
   }
   int from = l > 0 ? d->lines.at(l - 1).from + d->lines.at(l - 1).length + d->lines.at(l - 1).trailingSpaces : 0;
   int strlen = d->layoutData->string.length();
   if (l && from >= strlen) {
      if (!d->lines.at(l - 1).length || d->layoutData->string.at(strlen - 1) != QChar::LineSeparator) {
         return QTextLine();
      }
   }

   QScriptLine line;
   line.from = from;
   line.length = -1;
   line.justified = false;
   line.gridfitted = false;

   d->lines.append(line);
   return QTextLine(l, d);
}


int QTextLayout::lineCount() const
{
   return d->lines.size();
}

QTextLine QTextLayout::lineAt(int i) const
{
   return QTextLine(i, d);
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

   QFixed xmax, ymax;
   QFixed xmin = d->lines.at(0).x;
   QFixed ymin = d->lines.at(0).y;

   for (int i = 0; i < d->lines.size(); ++i) {
      const QScriptLine &si = d->lines[i];
      xmin = qMin(xmin, si.x);
      ymin = qMin(ymin, si.y);
      QFixed lineWidth = si.width < QFIXED_MAX ? qMax(si.width, si.textWidth) : si.textWidth;
      xmax = qMax(xmax, si.x + lineWidth);
      // ### shouldn't the ascent be used in ymin???
      ymax = qMax(ymax, si.y + si.height());
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


#if !defined(QT_NO_RAWFONT)
QList<QGlyphRun> QTextLayout::glyphRuns() const
{
   QList<QGlyphRun> glyphs;
   for (int i = 0; i < d->lines.size(); ++i) {
      glyphs += QTextLine(i, d).glyphs(-1, -1);
   }

   return glyphs;
}
#endif // QT_NO_RAWFONT

void QTextLayout::draw(QPainter *p, const QPointF &pos, const QVector<FormatRange> &selections,
                       const QRectF &clip) const
{
   if (d->lines.isEmpty()) {
      return;
   }

   if (!d->layoutData) {
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
         } else if (!selectionEndInLine
                    && isLastLineInBlock
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
      if (!clip.isNull()) {
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
   if (!excludedRegion.isEmpty()) {
      p->restore();
   }


   if (!d->cacheGlyphs) {
      d->freeMemory();
   }
}

void QTextLayout::drawCursor(QPainter *p, const QPointF &pos, int cursorPosition) const
{
   drawCursor(p, pos, cursorPosition, 1);
}

void QTextLayout::drawCursor(QPainter *p, const QPointF &pos, int cursorPosition, int width) const
{
   if (d->lines.isEmpty()) {
      return;
   }

   if (!d->layoutData) {
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
   bool toggleAntialiasing = !(p->renderHints() & QPainter::Antialiasing)
                             && (p->transform().type() > QTransform::TxTranslate);
   if (toggleAntialiasing) {
      p->setRenderHint(QPainter::Antialiasing);
   }

#ifdef Q_OS_MAC
   // Always draw the cursor aligned to pixel boundary.
   x = qRound(x);
#endif

   p->fillRect(QRectF(x, y, qreal(width), (base + descent + 1).toReal()), p->pen().brush());
   if (toggleAntialiasing) {
      p->setRenderHint(QPainter::Antialiasing, false);
   }
   if (d->layoutData->hasBidi) {
      const int arrow_extent = 4;
      int sign = rightToLeft ? -1 : 1;
      p->drawLine(QLineF(x, y, x + (sign * arrow_extent / 2), y + arrow_extent / 2));
      p->drawLine(QLineF(x, y + arrow_extent, x + (sign * arrow_extent / 2), y + arrow_extent / 2));
   }
   return;
}


QRectF QTextLine::rect() const
{
   const QScriptLine &sl = eng->lines[i];
   return QRectF(sl.x.toReal(), sl.y.toReal(), sl.width.toReal(), sl.height().toReal());
}

/*!
    Returns the rectangle covered by the line.
*/
QRectF QTextLine::naturalTextRect() const
{
   const QScriptLine &sl = eng->lines[i];
   QFixed x = sl.x + eng->alignLine(sl);

   QFixed width = sl.textWidth;
   if (sl.justified) {
      width = sl.width;
   }

   return QRectF(x.toReal(), sl.y.toReal(), width.toReal(), sl.height().toReal());
}

qreal QTextLine::x() const
{
   return eng->lines[i].x.toReal();
}

qreal QTextLine::y() const
{
   return eng->lines[i].y.toReal();
}

qreal QTextLine::width() const
{
   return eng->lines[i].width.toReal();
}

qreal QTextLine::ascent() const
{
   return eng->lines[i].ascent.toReal();
}

qreal QTextLine::descent() const
{
   return eng->lines[i].descent.toReal();
}

qreal QTextLine::height() const
{
   return eng->lines[i].height().toReal();
}

qreal QTextLine::leading() const
{
   return eng->lines[i].leading.toReal();
}

void QTextLine::setLeadingIncluded(bool included)
{
   eng->lines[i].leadingIncluded = included;

}

bool QTextLine::leadingIncluded() const
{
   return eng->lines[i].leadingIncluded;
}

qreal QTextLine::naturalTextWidth() const
{
   return eng->lines[i].textWidth.toReal();
}

qreal QTextLine::horizontalAdvance() const
{
   return eng->lines[i].textAdvance.toReal();
}

void QTextLine::setLineWidth(qreal width)
{
   QScriptLine &line = eng->lines[i];
   if (!eng->layoutData) {
      qWarning("QTextLine: Can't set a line width while not layouting.");
      return;
   }

   if (width > QFIXED_MAX) {
      width = QFIXED_MAX;
   }

   line.width = QFixed::fromReal(width);
   if (line.length
         && line.textWidth <= line.width
         && line.from + line.length == eng->layoutData->string.length())
      // no need to do anything if the line is already layouted and the last one. This optimization helps
      // when using things in a single line layout.
   {
      return;
   }
   line.length = 0;
   line.textWidth = 0;

   layout_helper(INT_MAX);
}

void QTextLine::setNumColumns(int numColumns)
{
   QScriptLine &line = eng->lines[i];
   line.width = QFIXED_MAX;
   line.length = 0;
   line.textWidth = 0;
   layout_helper(numColumns);
}

void QTextLine::setNumColumns(int numColumns, qreal alignmentWidth)
{
   QScriptLine &line = eng->lines[i];
   line.width = QFixed::fromReal(alignmentWidth);
   line.length = 0;
   line.textWidth = 0;
   layout_helper(numColumns);
}

#if 0
#define LB_DEBUG qDebug
#else
#define LB_DEBUG if (0) qDebug
#endif

namespace {

struct LineBreakHelper {
   LineBreakHelper()
      : glyphCount(0), maxGlyphs(0), currentPosition(0), fontEngine(0), logClusters(0),
        manualWrap(false), whiteSpaceOrObject(true) {
   }

   QScriptLine tmpData;
   QScriptLine spaceData;

   QGlyphLayout glyphs;

   int glyphCount;
   int maxGlyphs;
   int currentPosition;
   glyph_t previousGlyph;

   QFixed minw;
   QFixed softHyphenWidth;
   QFixed rightBearing;
   QFixed minimumRightBearing;

   QFontEngine *fontEngine;
   QFontEngine *previousFontEngine;
   const unsigned short *logClusters;

   bool manualWrap;
   bool whiteSpaceOrObject;

   bool checkFullOtherwiseExtend(QScriptLine &line);

   QFixed calculateNewWidth(const QScriptLine &line) const {
      return line.textWidth + tmpData.textWidth + spaceData.textWidth + softHyphenWidth
             - qMin(rightBearing, QFixed());
   }

   inline glyph_t currentGlyph() const {
      Q_ASSERT(currentPosition > 0);
      Q_ASSERT(logClusters[currentPosition - 1] < glyphs.numGlyphs);

      return glyphs.glyphs[logClusters[currentPosition - 1]];
   }

   inline void resetPreviousGlyph() {
      previousGlyph = 0;
      previousFontEngine = 0;
   }

   inline void saveCurrentGlyph() {
      resetPreviousGlyph();
      if (currentPosition > 0 &&
            logClusters[currentPosition - 1] < glyphs.numGlyphs) {
         previousGlyph = currentGlyph(); // needed to calculate right bearing later
         previousFontEngine = fontEngine;
      }
   }

   inline void adjustRightBearing(glyph_t glyph) {
      qreal rb;
      fontEngine->getGlyphBearings(glyph, 0, &rb);
      rightBearing = qMin(QFixed(), QFixed::fromReal(rb));
   }

   inline void adjustRightBearing() {
      if (currentPosition <= 0) {
         return;
      }
      adjustRightBearing(currentGlyph());
   }

   inline void adjustPreviousRightBearing() {
      if (previousGlyph > 0 && previousFontEngine) {
         qreal rb;
         previousFontEngine->getGlyphBearings(previousGlyph, 0, &rb);
         rightBearing = qMin(QFixed(), QFixed::fromReal(rb));
      }
   }

   inline void resetRightBearing() {
      rightBearing = QFixed(1); // Any positive number is defined as invalid since only
      // negative right bearings are interesting to us.
   }
};

inline bool LineBreakHelper::checkFullOtherwiseExtend(QScriptLine &line)
{
   LB_DEBUG("possible break width %f, spacew=%f", tmpData.textWidth.toReal(), spaceData.textWidth.toReal());

   QFixed newWidth = calculateNewWidth(line);
   if (line.length && !manualWrap && (newWidth > line.width || glyphCount > maxGlyphs)) {
      return true;
   }

   minw = qMax(minw, tmpData.textWidth);
   line += tmpData;
   line.textWidth += spaceData.textWidth;

   line.length += spaceData.length;
   tmpData.textWidth = 0;
   tmpData.length = 0;
   spaceData.textWidth = 0;
   spaceData.length = 0;

   return false;
}

} // anonymous namespace

static inline void addNextCluster(int &pos, int end, QScriptLine &line, int &glyphCount,
                  const QScriptItem &current, const unsigned short *logClusters, const QGlyphLayout &glyphs)
{
   int glyphPosition = logClusters[pos];
   do { // got to the first next cluster
      ++pos;
      ++line.length;
   } while (pos < end && logClusters[pos] == glyphPosition);

   do { // calculate the textWidth for the rest of the current cluster.
      if (!glyphs.attributes[glyphPosition].dontPrint) {
         line.textWidth += glyphs.advances_x[glyphPosition];
      }

      ++glyphPosition;

   } while (glyphPosition < current.num_glyphs && !glyphs.attributes[glyphPosition].clusterStart);

   Q_ASSERT((pos == end && glyphPosition == current.num_glyphs) || logClusters[pos] == glyphPosition);

   ++glyphCount;
}

// fill QScriptLine
void QTextLine::layout_helper(int maxGlyphs)
{
   QScriptLine &line = eng->lines[i];
   line.length = 0;
   line.trailingSpaces = 0;
   line.textWidth = 0;
   line.hasTrailingSpaces = false;

   if (!eng->layoutData->items.size() || line.from >= eng->layoutData->string.length()) {
      line.setDefaultHeight(eng);
      return;
   }

   Q_ASSERT(line.from < eng->layoutData->string.length());

   LineBreakHelper lbh;

   lbh.maxGlyphs = maxGlyphs;

   QTextOption::WrapMode wrapMode = eng->option.wrapMode();
   bool breakany = (wrapMode == QTextOption::WrapAnywhere);
   lbh.manualWrap = (wrapMode == QTextOption::ManualWrap || wrapMode == QTextOption::NoWrap);

   int item = -1;
   int newItem = eng->findItem(line.from);

   LB_DEBUG("from: %d: item=%d, total %d, width available %f", line.from, newItem, eng->layoutData->items.size(),
            line.width.toReal());

   Qt::Alignment alignment = eng->option.alignment();

   const HB_CharAttributes *attributes = eng->attributes();
   if (!attributes) {
      return;
   }
   lbh.currentPosition = line.from;
   int end = 0;
   lbh.logClusters = eng->layoutData->logClustersPtr;
   lbh.resetPreviousGlyph();

   while (newItem < eng->layoutData->items.size()) {
      lbh.resetRightBearing();
      lbh.softHyphenWidth = 0;
      if (newItem != item) {
         item = newItem;
         const QScriptItem &current = eng->layoutData->items[item];
         if (!current.num_glyphs) {
            eng->shape(item);
            attributes = eng->attributes();
            if (!attributes) {
               return;
            }
            lbh.logClusters = eng->layoutData->logClustersPtr;
         }
         lbh.currentPosition = qMax(line.from, current.position);
         end = current.position + eng->length(item);
         lbh.glyphs = eng->shapedGlyphs(&current);
         QFontEngine *fontEngine = eng->fontEngine(current);
         if (lbh.fontEngine != fontEngine) {
            lbh.fontEngine = fontEngine;
            lbh.minimumRightBearing = qMin(QFixed(),
                                           QFixed::fromReal(fontEngine->minRightBearing()));
         }
      }
      const QScriptItem &current = eng->layoutData->items[item];

      lbh.tmpData.leading = qMax(lbh.tmpData.leading + lbh.tmpData.ascent,
                                 current.leading + current.ascent) - qMax(lbh.tmpData.ascent,
                                       current.ascent);
      lbh.tmpData.ascent = qMax(lbh.tmpData.ascent, current.ascent);
      lbh.tmpData.descent = qMax(lbh.tmpData.descent, current.descent);

      if (current.analysis.flags == QScriptAnalysis::Tab &&
            (alignment & (Qt::AlignLeft | Qt::AlignRight | Qt::AlignCenter | Qt::AlignJustify))) {
         lbh.whiteSpaceOrObject = true;
         if (lbh.checkFullOtherwiseExtend(line)) {
            goto found;
         }

         QFixed x = line.x + line.textWidth + lbh.tmpData.textWidth + lbh.spaceData.textWidth;
         QFixed tabWidth = eng->calculateTabWidth(item, x);

         lbh.spaceData.textWidth += tabWidth;
         lbh.spaceData.length++;
         newItem = item + 1;

         QFixed averageCharWidth = eng->fontEngine(current)->averageCharWidth();
         lbh.glyphCount += qRound(tabWidth / averageCharWidth);

         if (lbh.checkFullOtherwiseExtend(line)) {
            goto found;
         }
      } else if (current.analysis.flags == QScriptAnalysis::LineOrParagraphSeparator) {
         lbh.whiteSpaceOrObject = true;
         // if the line consists only of the line separator make sure
         // we have a sane height
         if (!line.length && !lbh.tmpData.length) {
            line.setDefaultHeight(eng);
         }
         if (eng->option.flags() & QTextOption::ShowLineAndParagraphSeparators) {
            if (lbh.checkFullOtherwiseExtend(line)) {
               goto found;
            }

            addNextCluster(lbh.currentPosition, end, lbh.tmpData, lbh.glyphCount,
                           current, lbh.logClusters, lbh.glyphs);
         } else {
            lbh.tmpData.length++;
            lbh.adjustPreviousRightBearing();
         }
         line += lbh.tmpData;
         goto found;
      } else if (current.analysis.flags == QScriptAnalysis::Object) {
         lbh.whiteSpaceOrObject = true;
         lbh.tmpData.length++;

         QTextFormat format = eng->formats()->format(eng->formatIndex(&eng->layoutData->items[item]));
         if (eng->block.docHandle()) {
            eng->docLayout()->positionInlineObject(QTextInlineObject(item, eng), eng->block.position() + current.position, format);
         }

         lbh.tmpData.textWidth += current.width;

         newItem = item + 1;
         ++lbh.glyphCount;
         if (lbh.checkFullOtherwiseExtend(line)) {
            goto found;
         }
      } else if (attributes[lbh.currentPosition].whiteSpace) {
         lbh.whiteSpaceOrObject = true;
         while (lbh.currentPosition < end && attributes[lbh.currentPosition].whiteSpace)
            addNextCluster(lbh.currentPosition, end, lbh.spaceData, lbh.glyphCount,
                           current, lbh.logClusters, lbh.glyphs);

         if (!lbh.manualWrap && lbh.spaceData.textWidth > line.width) {
            lbh.spaceData.textWidth = line.width; // ignore spaces that fall out of the line.
            goto found;
         }
      } else {
         lbh.whiteSpaceOrObject = false;
         bool sb_or_ws = false;
         lbh.saveCurrentGlyph();
         do {
            addNextCluster(lbh.currentPosition, end, lbh.tmpData, lbh.glyphCount,
                           current, lbh.logClusters, lbh.glyphs);

            // This is a hack to fix a regression caused by the introduction of the
            // whitespace flag to non-breakable spaces and will cause the non-breakable
            // spaces to behave as in previous Qt versions in the line breaking algorithm.
            // The line breaks do not currently follow the Unicode specs, but fixing this would
            // require refactoring the code and would cause behavioral regressions.
            bool isBreakableSpace = lbh.currentPosition < eng->layoutData->string.length()
                  && attributes[lbh.currentPosition].whiteSpace
                  && eng->layoutData->string.at(lbh.currentPosition).decompositionTag() != QChar::NoBreak;

            bool isNotBreak = attributes[lbh.currentPosition - 1].lineBreakType != HB_NoBreak;

            if (lbh.currentPosition >= eng->layoutData->string.length() || isBreakableSpace || isNotBreak ) {
               sb_or_ws = true;
               break;

            } else if (breakany && attributes[lbh.currentPosition].charStop) {
               break;
            }

         } while (lbh.currentPosition < end);

         lbh.minw = qMax(lbh.tmpData.textWidth, lbh.minw);

         if (lbh.currentPosition > 0 && lbh.currentPosition < end
                   && attributes[lbh.currentPosition - 1].lineBreakType == HB_SoftHyphen
                   && eng->layoutData->string.at(lbh.currentPosition - 1).unicode() == QChar::SoftHyphen) {

            // if we are splitting up a word because of
            // a soft hyphen then we ...
            //
            //  a) have to take the width of the soft hyphen into
            //     account to see if the first syllable(s) /and/
            //     the soft hyphen fit into the line
            //
            //  b) if we are so short of available width that the
            //     soft hyphen is the first breakable position, then
            //     we don't want to show it. However we initially
            //     have to take the width for it into account so that
            //     the text document layout sees the overflow and
            //     switch to break-anywhere mode, in which we
            //     want the soft-hyphen to slip into the next line
            //     and thus become invisible again.
            //
            if (line.length) {
               lbh.softHyphenWidth = lbh.glyphs.advances_x[lbh.logClusters[lbh.currentPosition - 1]];
            } else if (breakany) {
               lbh.tmpData.textWidth += lbh.glyphs.advances_x[lbh.logClusters[lbh.currentPosition - 1]];
            }
         }

         // The actual width of the text needs to take the right bearing into account. The
         // right bearing is left-ward, which means that if the rightmost pixel is to the right
         // of the advance of the glyph, the bearing will be negative. We flip the sign
         // for the code to be more readable. Logic borrowed from qfontmetrics.cpp.
         // We ignore the right bearing if the minimum negative bearing is too little to
         // expand the text beyond the edge.

         if (sb_or_ws | breakany) {
            QFixed rightBearing = lbh.rightBearing; // store previous right bearing

#if ! defined(Q_OS_MAC)
            if (lbh.calculateNewWidth(line) - lbh.minimumRightBearing > line.width)
#endif

               lbh.adjustRightBearing();

            if (lbh.checkFullOtherwiseExtend(line)) {
               // we are too wide, fix right bearing
               if (rightBearing <= 0) {
                  lbh.rightBearing = rightBearing;   // take from cache
               } else {
                  lbh.adjustPreviousRightBearing();
               }

               if (!breakany) {
                  line.textWidth += lbh.softHyphenWidth;
               }

               goto found;
            }
         }
         lbh.saveCurrentGlyph();
      }
      if (lbh.currentPosition == end) {
         newItem = item + 1;
      }
   }

   LB_DEBUG("reached end of line");
   lbh.checkFullOtherwiseExtend(line);

found:
   if (lbh.rightBearing > 0 && !lbh.whiteSpaceOrObject) { // If right bearing has not yet been adjusted
      lbh.adjustRightBearing();
   }
   line.textAdvance = line.textWidth;
   line.textWidth -= qMin(QFixed(), lbh.rightBearing);

   if (line.length == 0) {
      LB_DEBUG("no break available in line, adding temp: length %d, width %f, space: length %d, width %f",
               lbh.tmpData.length, lbh.tmpData.textWidth.toReal(),
               lbh.spaceData.length, lbh.spaceData.textWidth.toReal());
      line += lbh.tmpData;
   }

   LB_DEBUG("line length = %d, ascent=%f, descent=%f, textWidth=%f (spacew=%f)", line.length, line.ascent.toReal(),
            line.descent.toReal(), line.textWidth.toReal(), lbh.spaceData.width.toReal());
   LB_DEBUG("        : '%s'", eng->layoutData->string.mid(line.from, line.length).toUtf8().data());

   if (lbh.manualWrap) {
      eng->minWidth = qMax(eng->minWidth, line.textWidth);
      eng->maxWidth = qMax(eng->maxWidth, line.textWidth);
   } else {
      eng->minWidth = qMax(eng->minWidth, lbh.minw);
      eng->maxWidth += line.textWidth;
   }

   if (line.textWidth > 0 && item < eng->layoutData->items.size()) {
      eng->maxWidth += lbh.spaceData.textWidth;
   }

   if (eng->option.flags() & QTextOption::IncludeTrailingSpaces) {
      line.textWidth += lbh.spaceData.textWidth;
   }
   if (lbh.spaceData.length) {
      line.trailingSpaces = lbh.spaceData.length;
      line.hasTrailingSpaces = true;
   }

   line.justified = false;
   line.gridfitted = false;

   if (eng->option.wrapMode() == QTextOption::WrapAtWordBoundaryOrAnywhere) {
      if ((lbh.maxGlyphs != INT_MAX && lbh.glyphCount > lbh.maxGlyphs)
            || (lbh.maxGlyphs == INT_MAX && line.textWidth > line.width)) {

         eng->option.setWrapMode(QTextOption::WrapAnywhere);
         line.length = 0;
         line.textWidth = 0;
         layout_helper(lbh.maxGlyphs);
         eng->option.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
      }
   }
}

/*!
    Moves the line to position \a pos.
*/
void QTextLine::setPosition(const QPointF &pos)
{
   eng->lines[i].x = QFixed::fromReal(pos.x());
   eng->lines[i].y = QFixed::fromReal(pos.y());
}

/*!
    Returns the line's position relative to the text layout's position.
*/
QPointF QTextLine::position() const
{
   return QPointF(eng->lines[i].x.toReal(), eng->lines[i].y.toReal());
}

// ### DOC: I have no idea what this means/does.
// You create a text layout with a string of text. Once you laid
// it out, it contains a number of QTextLines. from() returns the position
// inside the text string where this line starts. If you e.g. has a
// text of "This is a string", laid out into two lines (the second
// starting at the word 'a'), layout.lineAt(0).from() == 0 and
// layout.lineAt(1).from() == 8.

/*!
    Returns the start of the line from the beginning of the string
    passed to the QTextLayout.
*/
int QTextLine::textStart() const
{
   return eng->lines[i].from;
}

int QTextLine::textLength() const
{
   if (eng->option.flags() & QTextOption::ShowLineAndParagraphSeparators
         && eng->block.isValid() && i == eng->lines.count() - 1) {
      return eng->lines[i].length - 1;
   }
   return eng->lines[i].length + eng->lines[i].trailingSpaces;
}

static void drawMenuText(QPainter *p, QFixed x, QFixed y, const QScriptItem &si, QTextItemInt &gf,
                  QTextEngine *eng, int start, int glyph_start)
{
   int ge = glyph_start + gf.glyphs.numGlyphs;
   int gs = glyph_start;
   int end = start + gf.num_chars;
   unsigned short *logClusters = eng->logClusters(&si);
   QGlyphLayout glyphs = eng->shapedGlyphs(&si);
   QFixed orig_width = gf.width;

   int *ul = eng->underlinePositions;
   if (ul)
      while (*ul != -1 && *ul < start) {
         ++ul;
      }
   bool rtl = si.analysis.bidiLevel % 2;
   if (rtl) {
      x += si.width;
   }

   do {
      int gtmp = ge;
      int stmp = end;
      if (ul && *ul != -1 && *ul < end) {
         stmp = *ul;
         gtmp = logClusters[*ul - si.position];
      }

      gf.glyphs = glyphs.mid(gs, gtmp - gs);
      gf.num_chars = stmp - start;
      gf.chars = eng->layoutData->string.unicode() + start;
      QFixed w = 0;
      while (gs < gtmp) {
         w += glyphs.effectiveAdvance(gs);
         ++gs;
      }
      start = stmp;
      gf.width = w;
      if (rtl) {
         x -= w;
      }
      if (gf.num_chars) {
         p->drawTextItem(QPointF(x.toReal(), y.toReal()), gf);
      }
      if (!rtl) {
         x += w;
      }
      if (ul && *ul != -1 && *ul < end) {
         // draw underline
         gtmp = (*ul == end - 1) ? ge : logClusters[*ul + 1 - si.position];
         ++stmp;
         gf.glyphs = glyphs.mid(gs, gtmp - gs);
         gf.num_chars = stmp - start;
         gf.chars = eng->layoutData->string.unicode() + start;
         gf.logClusters = logClusters + start - si.position;
         w = 0;
         while (gs < gtmp) {
            w += glyphs.effectiveAdvance(gs);
            ++gs;
         }
         ++start;
         gf.width = w;
         gf.underlineStyle = QTextCharFormat::SingleUnderline;
         if (rtl) {
            x -= w;
         }
         p->drawTextItem(QPointF(x.toReal(), y.toReal()), gf);
         if (!rtl) {
            x += w;
         }
         gf.underlineStyle = QTextCharFormat::NoUnderline;
         ++gf.chars;
         ++ul;
      }
   } while (gs < ge);

   gf.width = orig_width;
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

namespace {
struct GlyphInfo {
   GlyphInfo(const QGlyphLayout &layout, const QPointF &position,
             const QTextItemInt::RenderFlags &renderFlags)
      : glyphLayout(layout), itemPosition(position), flags(renderFlags) {
   }

   QGlyphLayout glyphLayout;
   QPointF itemPosition;
   QTextItem::RenderFlags flags;
};
}

/*!
    \internal

    Returns the glyph indexes and positions for all glyphs in this QTextLine which reside in
    QScriptItems that overlap with the range defined by \a from and \a length. The arguments
    specify characters, relative to the text in the layout. Note that it is not possible to
    use this function to retrieve a subset of the glyphs in a QScriptItem.
*/
#if ! defined(QT_NO_RAWFONT)
QList<QGlyphRun> QTextLine::glyphs(int from, int length) const
{
   const QScriptLine &line = eng->lines[i];

   if (line.length == 0) {
      return QList<QGlyphRun>();
   }

   QMultiHash<QFontEngine *, GlyphInfo> glyphLayoutHash;

   QTextLineItemIterator iterator(eng, i);
   qreal y = line.y.toReal() + line.base().toReal();

   while (!iterator.atEnd()) {
      QScriptItem &si = iterator.next();
      if (si.analysis.flags >= QScriptAnalysis::TabOrObject) {
         continue;
      }

      QPointF pos(iterator.x.toReal(), y);
      if (from >= 0 && length >= 0 && (from >= si.position + eng->length(&si) || from + length <= si.position)) {
         continue;
      }

      QFont font = eng->font(si);

      QTextItem::RenderFlags flags;
      if (font.overline()) {
         flags |= QTextItem::Overline;
      }
      if (font.underline()) {
         flags |= QTextItem::Underline;
      }
      if (font.strikeOut()) {
         flags |= QTextItem::StrikeOut;
      }
      if (si.analysis.bidiLevel % 2) {
         flags |= QTextItem::RightToLeft;
      }

      QGlyphLayout glyphLayout = eng->shapedGlyphs(&si).mid(iterator.glyphsStart, iterator.glyphsEnd - iterator.glyphsStart);

      if (glyphLayout.numGlyphs > 0) {
         QFontEngine *mainFontEngine = font.d->engineForScript(si.analysis.script);

         if (mainFontEngine->type() == QFontEngine::Multi) {
            QFontEngineMulti *multiFontEngine = static_cast<QFontEngineMulti *>(mainFontEngine);

            int start = 0;
            int end;
            int which = glyphLayout.glyphs[0] >> 24;

            for (end = 0; end < glyphLayout.numGlyphs; ++end) {
               const int e = glyphLayout.glyphs[end] >> 24;

               if (e == which) {
                  continue;
               }

               QGlyphLayout subLayout = glyphLayout.mid(start, end - start);
               glyphLayoutHash.insertMulti(multiFontEngine->engine(which), GlyphInfo(subLayout, pos, flags));

               for (int i = 0; i < subLayout.numGlyphs; i++)
                  pos += QPointF(subLayout.advances_x[i].toReal(), subLayout.advances_y[i].toReal());

               start = end;
               which = e;
            }

            QGlyphLayout subLayout = glyphLayout.mid(start, end - start);
            glyphLayoutHash.insertMulti(multiFontEngine->engine(which), GlyphInfo(subLayout, pos, flags));

         } else {
            glyphLayoutHash.insertMulti(mainFontEngine, GlyphInfo(glyphLayout, pos, flags));
         }
      }
   }

   QHash<QPair<QFontEngine *, int>, QGlyphRun> glyphsHash;

   QList<QFontEngine *> keys = glyphLayoutHash.uniqueKeys();

   for (int i = 0; i < keys.size(); ++i) {
      QFontEngine *fontEngine = keys.at(i);

      // Make a font for this particular engine
      QRawFont font;
      QRawFontPrivate *fontD = QRawFontPrivate::get(font);
      fontD->fontEngine = fontEngine;
      fontD->fontEngine->ref.ref();

#if defined(Q_OS_WIN)
      if (fontEngine->supportsSubPixelPositions()) {
         fontD->hintingPreference = QFont::PreferVerticalHinting;
      } else {
         fontD->hintingPreference = QFont::PreferFullHinting;
      }

#elif defined(Q_OS_MAC)
      fontD->hintingPreference = QFont::PreferNoHinting;

#elif ! defined(QT_NO_FREETYPE)
      if (fontEngine->type() == QFontEngine::Freetype) {
         QFontEngineFT *freeTypeEngine = static_cast<QFontEngineFT *>(fontEngine);

         switch (freeTypeEngine->defaultHintStyle()) {
            case QFontEngineFT::HintNone:
               fontD->hintingPreference = QFont::PreferNoHinting;
               break;

            case QFontEngineFT::HintLight:
               fontD->hintingPreference = QFont::PreferVerticalHinting;
               break;

            case QFontEngineFT::HintMedium:
            case QFontEngineFT::HintFull:
               fontD->hintingPreference = QFont::PreferFullHinting;
               break;
         };
      }
#endif

      QList<GlyphInfo> glyphLayouts = glyphLayoutHash.values(fontEngine);

      for (int j = 0; j < glyphLayouts.size(); ++j) {
         const QPointF &pos = glyphLayouts.at(j).itemPosition;
         const QGlyphLayout &glyphLayout = glyphLayouts.at(j).glyphLayout;
         const QTextItem::RenderFlags &flags = glyphLayouts.at(j).flags;

         QVarLengthArray<glyph_t> glyphsArray;
         QVarLengthArray<QFixedPoint> positionsArray;

         fontEngine->getGlyphPositions(glyphLayout, QTransform(), flags, glyphsArray, positionsArray);
         Q_ASSERT(glyphsArray.size() == positionsArray.size());

         QVector<quint32> glyphs;
         QVector<QPointF> positions;

         for (int i = 0; i < glyphsArray.size(); ++i) {
            glyphs.append(glyphsArray.at(i) & 0xffffff);
            positions.append(positionsArray.at(i).toPointF() + pos);
         }

         QGlyphRun glyphIndexes;
         glyphIndexes.setGlyphIndexes(glyphs);
         glyphIndexes.setPositions(positions);

         glyphIndexes.setOverline(flags.testFlag(QTextItem::Overline));
         glyphIndexes.setUnderline(flags.testFlag(QTextItem::Underline));
         glyphIndexes.setStrikeOut(flags.testFlag(QTextItem::StrikeOut));
         glyphIndexes.setRawFont(font);

         QPair<QFontEngine *, int> key(fontEngine, int(flags));

         if (! glyphsHash.contains(key)) {
            glyphsHash.insert(key, glyphIndexes);

         } else {
            QGlyphRun &glyphRun = glyphsHash[key];

            QVector<quint32> indexes = glyphRun.glyphIndexes();
            QVector<QPointF> positions = glyphRun.positions();

            indexes += glyphIndexes.glyphIndexes();
            positions += glyphIndexes.positions();

            glyphRun.setGlyphIndexes(indexes);
            glyphRun.setPositions(positions);
         }
      }
   }

   return glyphsHash.values();
}
#endif // QT_NO_RAWFONT

void QTextLine::draw(QPainter *p, const QPointF &pos, const QTextLayout::FormatRange *selection) const
{
   const QScriptLine &line = eng->lines[i];
   QPen pen = p->pen();

   bool noText = (selection && selection->format.property(SuppressText).toBool());

   if (!line.length) {
      if (selection
            && selection->start <= line.from
            && selection->start + selection->length > line.from) {

         const qreal lineHeight = line.height().toReal();
         QRectF r(pos.x() + line.x.toReal(), pos.y() + line.y.toReal(),
                  lineHeight / 2, QFontMetrics(eng->font()).width(QLatin1Char(' ')));

         setPenAndDrawBackground(p, QPen(), selection->format, r);
         p->setPen(pen);
      }
      return;
   }

   QTextLineItemIterator iterator(eng, i, pos, selection);
   QFixed lineBase = line.base();

   const QFixed y = QFixed::fromReal(pos.y()) + line.y + lineBase;

   bool suppressColors = (eng->option.flags() & QTextOption::SuppressColors);
   while (!iterator.atEnd()) {
      QScriptItem &si = iterator.next();

      if (selection && selection->start >= 0 && iterator.isOutsideSelection()) {
         continue;
      }

      if (si.analysis.flags == QScriptAnalysis::LineOrParagraphSeparator
            && !(eng->option.flags() & QTextOption::ShowLineAndParagraphSeparators)) {
         continue;
      }

      QFixed itemBaseLine = y;
      QFont f = eng->font(si);
      QTextCharFormat format;

      if (eng->hasFormats() || selection) {
         format = eng->format(&si);
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

         if (eng->hasFormats()) {
            p->save();
            if (si.analysis.flags == QScriptAnalysis::Object && eng->block.docHandle()) {
               QFixed itemY = y - si.ascent;
               if (format.verticalAlignment() == QTextCharFormat::AlignTop) {
                  itemY = y - lineBase;
               }

               QRectF itemRect(iterator.x.toReal(), itemY.toReal(), iterator.itemWidth.toReal(), si.height().toReal());

               eng->docLayout()->drawInlineObject(p, itemRect,
                                                  QTextInlineObject(iterator.item, eng),
                                                  si.position + eng->block.position(),
                                                  format);
               if (selection) {
                  QBrush bg = format.brushProperty(ObjectSelectionBrush);
                  if (bg.style() != Qt::NoBrush) {
                     QColor c = bg.color();
                     c.setAlpha(128);
                     p->fillRect(itemRect, c);
                  }
               }
            } else { // si.isTab
               QFont f = eng->font(si);
               QTextItemInt gf(si, &f, format);
               gf.chars = 0;
               gf.num_chars = 0;
               gf.width = iterator.itemWidth;
               p->drawTextItem(QPointF(iterator.x.toReal(), y.toReal()), gf);
               if (eng->option.flags() & QTextOption::ShowTabsAndSpaces) {
                  QChar visualTab(0x2192);
                  int w = QFontMetrics(f).width(visualTab);
                  qreal x = iterator.itemWidth.toReal() - w; // Right-aligned
                  if (x < 0)
                     p->setClipRect(QRectF(iterator.x.toReal(), line.y.toReal(),
                                           iterator.itemWidth.toReal(), line.height().toReal()),
                                    Qt::IntersectClip);
                  else {
                     x /= 2;   // Centered
                  }
                  p->drawText(QPointF(iterator.x.toReal() + x,
                                      y.toReal()), visualTab);
               }

            }
            p->restore();
         }

         continue;
      }

      unsigned short *logClusters = eng->logClusters(&si);
      QGlyphLayout glyphs = eng->shapedGlyphs(&si);

      QTextItemInt gf(glyphs.mid(iterator.glyphsStart, iterator.glyphsEnd - iterator.glyphsStart),
                      &f, eng->layoutData->string.unicode() + iterator.itemStart,
                      iterator.itemEnd - iterator.itemStart, eng->fontEngine(si), format);
      gf.logClusters = logClusters + iterator.itemStart - si.position;
      gf.width = iterator.itemWidth;
      gf.justified = line.justified;
      gf.initWithScriptItem(si);

      Q_ASSERT(gf.fontEngine);

      if (eng->underlinePositions) {
         // can't have selections in this case
         drawMenuText(p, iterator.x, itemBaseLine, si, gf, eng, iterator.itemStart, iterator.glyphsStart);
      } else {
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
            //Currently QPen with a Qt::NoPen style still returns a default
            //QBrush which != Qt::NoBrush so we need this specialcase to reset it
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
            p->drawTextItem(pos, gf);
         }
      }
      if (si.analysis.flags == QScriptAnalysis::Space
            && (eng->option.flags() & QTextOption::ShowTabsAndSpaces)) {
         QBrush c = format.foreground();
         if (c.style() != Qt::NoBrush) {
            p->setPen(c.color());
         }
         QChar visualSpace((ushort)0xb7);
         p->drawText(QPointF(iterator.x.toReal(), itemBaseLine.toReal()), visualSpace);
         p->setPen(pen);
      }
   }


   if (eng->hasFormats()) {
      p->setPen(pen);
   }
}

qreal QTextLine::cursorToX(int *cursorPos, Edge edge) const
{
   if (!eng->layoutData) {
      eng->itemize();
   }

   const QScriptLine &line = eng->lines[i];
   bool lastLine = i >= eng->lines.size() - 1;

   QFixed x = line.x;
   x += eng->alignLine(line) - eng->leadingSpaceWidth(line);

   if (!i && !eng->layoutData->items.size()) {
      *cursorPos = 0;
      return x.toReal();
   }

   int lineEnd = line.from + line.length + line.trailingSpaces;
   int pos = *cursorPos;
   int itm;
   const HB_CharAttributes *attributes = eng->attributes();
   if (!attributes) {
      *cursorPos = 0;
      return x.toReal();
   }
   while (pos < lineEnd && !attributes[pos].charStop) {
      pos++;
   }
   if (pos == lineEnd) {
      // end of line ensure we have the last item on the line
      itm = eng->findItem(pos - 1);
   } else {
      itm = eng->findItem(pos);
   }
   eng->shapeLine(line);

   const QScriptItem *si = &eng->layoutData->items[itm];
   if (!si->num_glyphs) {
      eng->shape(itm);
   }
   pos -= si->position;

   QGlyphLayout glyphs = eng->shapedGlyphs(si);
   unsigned short *logClusters = eng->logClusters(si);
   Q_ASSERT(logClusters);

   int l = eng->length(itm);
   if (pos > l) {
      pos = l;
   }
   if (pos < 0) {
      pos = 0;
   }

   int glyph_pos = pos == l ? si->num_glyphs : logClusters[pos];
   if (edge == Trailing) {
      // trailing edge is leading edge of next cluster
      while (glyph_pos < si->num_glyphs && !glyphs.attributes[glyph_pos].clusterStart) {
         glyph_pos++;
      }
   }

   bool reverse = eng->layoutData->items[itm].analysis.bidiLevel % 2;

   // add the items left of the cursor
   int firstItem = eng->findItem(line.from);
   int lastItem = eng->findItem(lineEnd - 1);
   int nItems = (firstItem >= 0 && lastItem >= firstItem) ? (lastItem - firstItem + 1) : 0;

   QVarLengthArray<int> visualOrder(nItems);
   QVarLengthArray<uchar> levels(nItems);
   for (int i = 0; i < nItems; ++i) {
      levels[i] = eng->layoutData->items[i + firstItem].analysis.bidiLevel;
   }
   QTextEngine::bidiReorder(nItems, levels.data(), visualOrder.data());

   for (int i = 0; i < nItems; ++i) {
      int item = visualOrder[i] + firstItem;
      if (item == itm) {
         break;
      }
      QScriptItem &si = eng->layoutData->items[item];
      if (!si.num_glyphs) {
         eng->shape(item);
      }

      if (si.analysis.flags >= QScriptAnalysis::TabOrObject) {
         x += si.width;
         continue;
      }
      int start = qMax(line.from, si.position);
      int end = qMin(lineEnd, si.position + eng->length(item));

      logClusters = eng->logClusters(&si);

      int gs = logClusters[start - si.position];
      int ge = (end == si.position + eng->length(item)) ? si.num_glyphs - 1 : logClusters[end - si.position - 1];

      QGlyphLayout glyphs = eng->shapedGlyphs(&si);

      while (gs <= ge) {
         x += glyphs.effectiveAdvance(gs);
         ++gs;
      }
   }

   logClusters = eng->logClusters(si);
   glyphs = eng->shapedGlyphs(si);
   if (si->analysis.flags >= QScriptAnalysis::TabOrObject) {
      if (pos == (reverse ? 0 : l)) {
         x += si->width;
      }
   } else {
      bool rtl = eng->isRightToLeft();
      bool visual = eng->visualCursorMovement();
      int end = qMin(lineEnd, si->position + l) - si->position;
      if (reverse) {
         int glyph_end = end == l ? si->num_glyphs : logClusters[end];
         int glyph_start = glyph_pos;
         if (visual && !rtl && !(lastLine && itm == (visualOrder[nItems - 1] + firstItem))) {
            glyph_start++;
         }
         for (int i = glyph_end - 1; i >= glyph_start; i--) {
            x += glyphs.effectiveAdvance(i);
         }
      } else {
         int start = qMax(line.from - si->position, 0);
         int glyph_start = logClusters[start];
         int glyph_end = glyph_pos;
         if (!visual || !rtl || (lastLine && itm == visualOrder[0] + firstItem)) {
            glyph_end--;
         }
         for (int i = glyph_start; i <= glyph_end; i++) {
            x += glyphs.effectiveAdvance(i);
         }
      }
      x += eng->offsetInLigature(si, pos, end, glyph_pos);
   }

   if (eng->option.wrapMode() != QTextOption::NoWrap && x > line.x + line.width) {
      x = line.x + line.width;
   }

   if (eng->option.wrapMode() != QTextOption::NoWrap && x < 0) {
      x = 0;
   }

   *cursorPos = pos + si->position;
   return x.toReal();
}

int QTextLine::xToCursor(qreal _x, CursorPosition cpos) const
{
   QFixed x = QFixed::fromReal(_x);
   const QScriptLine &line = eng->lines[i];
   bool lastLine = i >= eng->lines.size() - 1;
   int lineNum = i;

   if (!eng->layoutData) {
      eng->itemize();
   }

   int line_length = textLength();

   if (!line_length) {
      return line.from;
   }

   int firstItem = eng->findItem(line.from);
   int lastItem = eng->findItem(line.from + line_length - 1);
   int nItems = (firstItem >= 0 && lastItem >= firstItem) ? (lastItem - firstItem + 1) : 0;

   if (!nItems) {
      return 0;
   }

   x -= line.x;
   x -= eng->alignLine(line);
   //     qDebug("xToCursor: x=%f, cpos=%d", x.toReal(), cpos);

   QVarLengthArray<int> visualOrder(nItems);
   QVarLengthArray<unsigned char> levels(nItems);
   for (int i = 0; i < nItems; ++i) {
      levels[i] = eng->layoutData->items[i + firstItem].analysis.bidiLevel;
   }
   QTextEngine::bidiReorder(nItems, levels.data(), visualOrder.data());

   bool visual = eng->visualCursorMovement();
   if (x <= 0) {
      // left of first item
      int item = visualOrder[0] + firstItem;
      QScriptItem &si = eng->layoutData->items[item];
      if (!si.num_glyphs) {
         eng->shape(item);
      }
      int pos = si.position;
      if (si.analysis.bidiLevel % 2) {
         pos += eng->length(item);
      }
      pos = qMax(line.from, pos);
      pos = qMin(line.from + line_length, pos);
      return pos;
   } else if (x < line.textWidth
              || (line.justified && x < line.width)) {
      // has to be in one of the runs
      QFixed pos;
      bool rtl = eng->isRightToLeft();

      eng->shapeLine(line);
      QVector<int> insertionPoints;
      if (visual && rtl) {
         eng->insertionPointsForLine(lineNum, insertionPoints);
      }
      int nchars = 0;
      for (int i = 0; i < nItems; ++i) {
         int item = visualOrder[i] + firstItem;
         QScriptItem &si = eng->layoutData->items[item];
         int item_length = eng->length(item);
         //             qDebug("    item %d, visual %d x_remain=%f", i, item, x.toReal());

         int start = qMax(line.from - si.position, 0);
         int end = qMin(line.from + line_length - si.position, item_length);

         unsigned short *logClusters = eng->logClusters(&si);

         int gs = logClusters[start];
         int ge = (end == item_length ? si.num_glyphs : logClusters[end]) - 1;
         QGlyphLayout glyphs = eng->shapedGlyphs(&si);

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
         //             qDebug("      start=%d, end=%d, gs=%d, ge=%d item_width=%f", start, end, gs, ge, item_width.toReal());

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
                     break;
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
                  if (!rtl && i < nItems - 1) {
                     nchars += end;
                     continue;
                  }
                  if (rtl && nchars > 0) {
                     return insertionPoints[lastLine ? nchars : nchars - 1];
                  }
               }
               return eng->positionInLigature(&si, end, x, pos, -1, cpos == QTextLine::CursorOnCharacter);
            }
         }

         Q_ASSERT(glyph_pos != -1);
         return eng->positionInLigature(&si, end, x, edge, glyph_pos, cpos == QTextLine::CursorOnCharacter);
      }
   }

   // right of last item
   int item = visualOrder[nItems - 1] + firstItem;
   QScriptItem &si = eng->layoutData->items[item];

   if (!si.num_glyphs) {
      eng->shape(item);
   }
   int pos = si.position;
   if (!(si.analysis.bidiLevel % 2)) {
      pos += eng->length(item);
   }
   pos = qMax(line.from, pos);

   int maxPos = line.from + line_length;

   // except for the last line we assume that the character between lines is a space and we want
   // to position the cursor to the left of that character.

   // ###### breaks with japanese for example
   if (this->i < eng->lines.count() - 1) {
      --maxPos;
   }

   pos = qMin(pos, maxPos);
   return pos;
}

QT_END_NAMESPACE
