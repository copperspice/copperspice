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

#include <qfont.h>
#include <qpaintdevice.h>
#include <qfontmetrics.h>

#include <qfont_p.h>
#include <qfontengine_p.h>
#include <qunicodetables_p.h>

#include <math.h>

extern void qt_format_text(const QFont &font, const QRectF &_r, int tf, const QString &text,
            QRectF *brect, int tabStops, int *tabArray, int tabArrayLen, QPainter *painter);

QFontMetrics::QFontMetrics(const QFont &font)
   : d(font.d.data())
{
}

QFontMetrics::QFontMetrics(const QFont &font, QPaintDevice *paintdevice)
{
   int dpi = paintdevice ? paintdevice->logicalDpiY() : qt_defaultDpi();

   const int screen = 0;

   if (font.d->dpi != dpi || font.d->screen != screen ) {
      d = new QFontPrivate(*font.d);
      d->dpi = dpi;
      d->screen = screen;
   } else {
      d = font.d.data();
   }

}

QFontMetrics::QFontMetrics(const QFontMetrics &other)
   : d(other.d.data())
{
}

QFontMetrics::~QFontMetrics()
{
}

QFontMetrics &QFontMetrics::operator=(const QFontMetrics &other)
{
   d = other.d.data();
   return *this;
}

bool QFontMetrics::operator==(const QFontMetrics &other) const
{
   return d == other.d;
}

int QFontMetrics::ascent() const
{
   QFontEngine *engine = d->engineForScript(QChar::Script_Common);
   Q_ASSERT(engine != nullptr);

   return qRound(engine->ascent());
}

int QFontMetrics::descent() const
{
   QFontEngine *engine = d->engineForScript(QChar::Script_Common);
   Q_ASSERT(engine != nullptr);

   return qRound(engine->descent());
}

/*!
    Returns the height of the font.

    This is always equal to ascent()+descent()+1 (the 1 is for the
    base line).

    \sa leading(), lineSpacing()
*/
int QFontMetrics::height() const
{
   QFontEngine *engine = d->engineForScript(QChar::Script_Common);
   Q_ASSERT(engine != nullptr);

   return qRound(engine->ascent()) + qRound(engine->descent());
}

/*!
    Returns the leading of the font.

    This is the natural inter-line spacing.

    \sa height(), lineSpacing()
*/
int QFontMetrics::leading() const
{
   QFontEngine *engine = d->engineForScript(QChar::Script_Common);
   Q_ASSERT(engine != nullptr);

   return qRound(engine->leading());
}

/*!
    Returns the distance from one base line to the next.

    This value is always equal to leading()+height().

    \sa height(), leading()
*/
int QFontMetrics::lineSpacing() const
{
   QFontEngine *engine = d->engineForScript(QChar::Script_Common);
   Q_ASSERT(engine != nullptr);

   return qRound(engine->leading()) + qRound(engine->ascent()) + qRound(engine->descent());
}

/*!
    Returns the minimum left bearing of the font.

    This is the smallest leftBearing(char) of all characters in the
    font.

    Note that this function can be very slow if the font is large.

    \sa minRightBearing(), leftBearing()
*/
int QFontMetrics::minLeftBearing() const
{
   QFontEngine *engine = d->engineForScript(QChar::Script_Common);
   Q_ASSERT(engine != nullptr);

   return qRound(engine->minLeftBearing());
}

/*!
    Returns the minimum right bearing of the font.

    This is the smallest rightBearing(char) of all characters in the
    font.

    Note that this function can be very slow if the font is large.

    \sa minLeftBearing(), rightBearing()
*/
int QFontMetrics::minRightBearing() const
{
   QFontEngine *engine = d->engineForScript(QChar::Script_Common);
   Q_ASSERT(engine != nullptr);

   return qRound(engine->minRightBearing());
}

/*!
    Returns the width of the widest character in the font.
*/
int QFontMetrics::maxWidth() const
{
   QFontEngine *engine = d->engineForScript(QChar::Script_Common);
   Q_ASSERT(engine != nullptr);

   return qRound(engine->maxCharWidth());
}

/*!
    Returns the 'x' height of the font. This is often but not always
    the same as the height of the character 'x'.
*/
int QFontMetrics::xHeight() const
{
   QFontEngine *engine = d->engineForScript(QChar::Script_Common);
   Q_ASSERT(engine != nullptr);

   if (d->capital == QFont::SmallCaps) {
      return qRound(d->smallCapsFontPrivate()->engineForScript(QChar::Script_Common)->ascent());
   }
   return qRound(engine->xHeight());
}

int QFontMetrics::averageCharWidth() const
{
   QFontEngine *engine = d->engineForScript(QChar::Script_Common);
   Q_ASSERT(engine != nullptr);

   return qRound(engine->averageCharWidth());
}

bool QFontMetrics::inFont(QChar ch) const
{
   return inFontUcs4(ch.unicode());
}

bool QFontMetrics::inFontUcs4(char32_t ch) const
{
   const int script_id = QChar(ch).script();

   QFontEngine *engine = d->engineForScript(script_id);
   Q_ASSERT(engine != nullptr);

   if (engine->type() == QFontEngine::Box) {
      return false;
   }

   return engine->canRender(make_view(QString(ch)));
}

int QFontMetrics::leftBearing(QChar ch) const
{
   const int script_id = ch.script();
   QFontEngine *engine;

   if (d->capital == QFont::SmallCaps && ch.isLower()) {
      engine = d->smallCapsFontPrivate()->engineForScript(script_id);
   } else {
      engine = d->engineForScript(script_id);
   }

   Q_ASSERT(engine != nullptr);

   if (engine->type() == QFontEngine::Box) {
      return 0;
   }

   d->alterCharForCapitalization(ch);
   glyph_t glyph = engine->glyphIndex(ch.unicode());

   qreal lb;
   engine->getGlyphBearings(glyph, &lb);

   return qRound(lb);
}

int QFontMetrics::rightBearing(QChar ch) const
{
   const int script = ch.script();
   QFontEngine *engine;

   if (d->capital == QFont::SmallCaps && ch.isLower()) {
      engine = d->smallCapsFontPrivate()->engineForScript(script);
   } else {
      engine = d->engineForScript(script);
   }

   Q_ASSERT(engine != nullptr);
   if (engine->type() == QFontEngine::Box) {
      return 0;
   }

   d->alterCharForCapitalization(ch);

   glyph_t glyph = engine->glyphIndex(ch.unicode());

   qreal rb;
   engine->getGlyphBearings(glyph, nullptr, &rb);

   return qRound(rb);
}

int QFontMetrics::width(const QString &text, int len) const
{
   return width(text, len, 0);
}

/*!
    \internal
*/
int QFontMetrics::width(const QString &text, int len, int flags) const
{
   int pos = text.indexOf(QLatin1Char('\x9c'));

   if (pos != -1) {
      len = (len < 0) ? pos : qMin(pos, len);
   } else if (len < 0) {
      len = text.length();
   }

   if (len == 0) {
      return 0;
   }

   if (flags & Qt::TextBypassShaping) {
      // Skip harfbuzz complex shaping, only use advances
      int numGlyphs = len;
      QVarLengthGlyphLayoutArray glyphs(numGlyphs);
      QFontEngine *engine = d->engineForScript(QChar::Script_Common);

      if (! engine->stringToCMap(text, &glyphs, &numGlyphs, Qt::EmptyFlag)) {
         // error, may want to throw
         glyphs.resize(numGlyphs);

         if (! engine->stringToCMap(text, &glyphs, &numGlyphs, Qt::EmptyFlag)) {
            Q_ASSERT_X(false, Q_FUNC_INFO, "stringToCMap should not fail twice");
         }
      }

      QFixed width;

      for (int i = 0; i < numGlyphs; ++i) {
         width += glyphs.advances[i];
      }

      return qRound(width);
   }

   QStackTextEngine layout(text, QFont(d.data()));
   return qRound(layout.width(0, len));
}

int QFontMetrics::width(QChar ch) const
{
   if (ch.category() == QChar::Mark_NonSpacing) {
      return 0;
   }

   const int script_id = ch.script();
   QFontEngine *engine;

   if (d->capital == QFont::SmallCaps && ch.isLower()) {
      engine = d->smallCapsFontPrivate()->engineForScript(script_id);
   } else {
      engine = d->engineForScript(script_id);
   }

   Q_ASSERT(engine != nullptr);
   d->alterCharForCapitalization(ch);

   glyph_t glyph = engine->glyphIndex(ch.unicode());

   QFixed advance;
   QGlyphLayout glyphs;
   glyphs.numGlyphs = 1;
   glyphs.glyphs    = &glyph;
   glyphs.advances  = &advance;
   engine->recalcAdvances(&glyphs, Qt::EmptyFlag);

   return qRound(advance);
}

QRect QFontMetrics::boundingRect(const QString &text) const
{
   if (text.length() == 0) {
      return QRect();
   }

   QStackTextEngine layout(text, QFont(d.data()));
   layout.itemize();
   glyph_metrics_t gm = layout.boundingBox(0, text.length());

   return QRect(qRound(gm.x), qRound(gm.y), qRound(gm.width), qRound(gm.height));
}

QRect QFontMetrics::boundingRect(QChar ch) const
{
   const int script = ch.script();
   QFontEngine *engine;

   if (d->capital == QFont::SmallCaps && ch.isLower()) {
      engine = d->smallCapsFontPrivate()->engineForScript(script);
   } else {
      engine = d->engineForScript(script);
   }

   Q_ASSERT(engine != nullptr);
   d->alterCharForCapitalization(ch);

   glyph_t glyph = engine->glyphIndex(ch.unicode());
   glyph_metrics_t gm = engine->boundingBox(glyph);

   return QRect(qRound(gm.x), qRound(gm.y), qRound(gm.width), qRound(gm.height));
}


QRect QFontMetrics::boundingRect(const QRect &rect, int flags, const QString &text, int tabStops,
   int *tabArray) const
{
   int tabArrayLen = 0;
   if (tabArray)
      while (tabArray[tabArrayLen]) {
         tabArrayLen++;
      }

   QRectF rb;
   QRectF rr(rect);
   qt_format_text(QFont(d.data()), rr, flags | Qt::TextDontPrint, text, &rb, tabStops,
               tabArray, tabArrayLen, nullptr);

   return rb.toAlignedRect();
}

QSize QFontMetrics::size(int flags, const QString &text, int tabStops, int *tabArray) const
{
   return boundingRect(QRect(0, 0, 0, 0), flags | Qt::TextLongestVariant, text, tabStops, tabArray).size();
}

QRect QFontMetrics::tightBoundingRect(const QString &text) const
{
   if (text.length() == 0) {
      return QRect();
   }

   QStackTextEngine layout(text, QFont(d.data()));
   layout.itemize();
   glyph_metrics_t gm = layout.tightBoundingBox(0, text.length());

   return QRect(qRound(gm.x), qRound(gm.y), qRound(gm.width), qRound(gm.height));
}

QString QFontMetrics::elidedText(const QString &text, Qt::TextElideMode mode, int width, int flags) const
{
   QString _text = text;
   if (!(flags & Qt::TextLongestVariant)) {
      int posA = 0;
      int posB = _text.indexOf(QLatin1Char('\x9c'));
      while (posB >= 0) {
         QString portion = _text.mid(posA, posB - posA);
         if (size(flags, portion).width() <= width) {
            return portion;
         }
         posA = posB + 1;
         posB = _text.indexOf(QLatin1Char('\x9c'), posA);
      }
      _text = _text.mid(posA);
   }
   QStackTextEngine engine(_text, QFont(d.data()));
   return engine.elidedText(mode, width, flags);
}

/*!
    Returns the distance from the base line to where an underscore
    should be drawn.

    \sa overlinePos(), strikeOutPos(), lineWidth()
*/
int QFontMetrics::underlinePos() const
{
   QFontEngine *engine = d->engineForScript(QChar::Script_Common);
   Q_ASSERT(engine != nullptr);
   return qRound(engine->underlinePosition());
}

/*!
    Returns the distance from the base line to where an overline
    should be drawn.

    \sa underlinePos(), strikeOutPos(), lineWidth()
*/
int QFontMetrics::overlinePos() const
{
   return ascent() + 1;
}

/*!
    Returns the distance from the base line to where the strikeout
    line should be drawn.

    \sa underlinePos(), overlinePos(), lineWidth()
*/
int QFontMetrics::strikeOutPos() const
{
   int pos = ascent() / 3;
   return pos > 0 ? pos : 1;
}

/*!
    Returns the width of the underline and strikeout lines, adjusted
    for the point size of the font.

    \sa underlinePos(), overlinePos(), strikeOutPos()
*/
int QFontMetrics::lineWidth() const
{
   QFontEngine *engine = d->engineForScript(QChar::Script_Common);
   Q_ASSERT(engine != nullptr);

   return qRound(engine->lineThickness());
}

QFontMetricsF::QFontMetricsF(const QFontMetrics &fontMetrics)
   : d(fontMetrics.d.data())
{
}

QFontMetricsF &QFontMetricsF::operator=(const QFontMetrics &other)
{
   d = other.d.data();
   return *this;
}

QFontMetricsF::QFontMetricsF(const QFont &font)
   : d(font.d.data())
{
}

QFontMetricsF::QFontMetricsF(const QFont &font, QPaintDevice *paintdevice)
{
   int dpi = paintdevice ? paintdevice->logicalDpiY() : qt_defaultDpi();

   const int screen = 0;

   if (font.d->dpi != dpi || font.d->screen != screen ) {
      d = new QFontPrivate(*font.d);
      d->dpi = dpi;
      d->screen = screen;
   } else {
      d = font.d.data();
   }
}

QFontMetricsF::QFontMetricsF(const QFontMetricsF &fm)
   : d(fm.d.data())
{
}

QFontMetricsF::~QFontMetricsF()
{
}

QFontMetricsF &QFontMetricsF::operator=(const QFontMetricsF &fm)
{
   d = fm.d.data();
   return *this;
}


bool QFontMetricsF::operator ==(const QFontMetricsF &other) const
{
   return d == other.d;
}

qreal QFontMetricsF::ascent() const
{
   QFontEngine *engine = d->engineForScript(QChar::Script_Common);
   Q_ASSERT(engine != nullptr);

   return engine->ascent().toReal();
}


qreal QFontMetricsF::descent() const
{
   QFontEngine *engine = d->engineForScript(QChar::Script_Common);
   Q_ASSERT(engine != nullptr);

   return engine->descent().toReal();
}

/*!
    Returns the height of the font.

    This is always equal to ascent()+descent()+1 (the 1 is for the
    base line).

    \sa leading(), lineSpacing()
*/
qreal QFontMetricsF::height() const
{
   QFontEngine *engine = d->engineForScript(QChar::Script_Common);
   Q_ASSERT(engine != nullptr);

   return (engine->ascent() + engine->descent()).toReal();
}

/*!
    Returns the leading of the font.

    This is the natural inter-line spacing.

    \sa height(), lineSpacing()
*/
qreal QFontMetricsF::leading() const
{
   QFontEngine *engine = d->engineForScript(QChar::Script_Common);
   Q_ASSERT(engine != nullptr);

   return engine->leading().toReal();
}

/*!
    Returns the distance from one base line to the next.

    This value is always equal to leading()+height().

    \sa height(), leading()
*/
qreal QFontMetricsF::lineSpacing() const
{
   QFontEngine *engine = d->engineForScript(QChar::Script_Common);
   Q_ASSERT(engine != nullptr);

   return (engine->leading() + engine->ascent() + engine->descent()).toReal();
}

/*!
    Returns the minimum left bearing of the font.

    This is the smallest leftBearing(char) of all characters in the
    font.

    Note that this function can be very slow if the font is large.

    \sa minRightBearing(), leftBearing()
*/
qreal QFontMetricsF::minLeftBearing() const
{
   QFontEngine *engine = d->engineForScript(QChar::Script_Common);
   Q_ASSERT(engine != nullptr);

   return engine->minLeftBearing();
}

/*!
    Returns the minimum right bearing of the font.

    This is the smallest rightBearing(char) of all characters in the
    font.

    Note that this function can be very slow if the font is large.

    \sa minLeftBearing(), rightBearing()
*/
qreal QFontMetricsF::minRightBearing() const
{
   QFontEngine *engine = d->engineForScript(QChar::Script_Common);
   Q_ASSERT(engine != nullptr);

   return engine->minRightBearing();
}

/*!
    Returns the width of the widest character in the font.
*/
qreal QFontMetricsF::maxWidth() const
{
   QFontEngine *engine = d->engineForScript(QChar::Script_Common);
   Q_ASSERT(engine != nullptr);

   return engine->maxCharWidth();
}

/*!
    Returns the 'x' height of the font. This is often but not always
    the same as the height of the character 'x'.
*/
qreal QFontMetricsF::xHeight() const
{
   QFontEngine *engine = d->engineForScript(QChar::Script_Common);
   Q_ASSERT(engine != nullptr);

   if (d->capital == QFont::SmallCaps) {
      return d->smallCapsFontPrivate()->engineForScript(QChar::Script_Common)->ascent().toReal();
   }

   return engine->xHeight().toReal();
}

qreal QFontMetricsF::averageCharWidth() const
{
   QFontEngine *engine = d->engineForScript(QChar::Script_Common);
   Q_ASSERT(engine != nullptr);

   return engine->averageCharWidth().toReal();
}

bool QFontMetricsF::inFont(QChar ch) const
{
   return inFontUcs4(ch.unicode());
}

bool QFontMetricsF::inFontUcs4(char32_t ch) const
{
   const int script = QChar(ch).script();
   QFontEngine *engine = d->engineForScript(script);

   Q_ASSERT(engine != nullptr);

   if (engine->type() == QFontEngine::Box) {
      return false;
   }

   return engine->canRender(make_view(QString(ch)));
}

qreal QFontMetricsF::leftBearing(QChar ch) const
{
   const int script = ch.script();
   QFontEngine *engine;

   if (d->capital == QFont::SmallCaps && ch.isLower()) {
      engine = d->smallCapsFontPrivate()->engineForScript(script);
   } else {
      engine = d->engineForScript(script);
   }

   Q_ASSERT(engine != nullptr);
   if (engine->type() == QFontEngine::Box) {
      return 0;
   }

   d->alterCharForCapitalization(ch);

   glyph_t glyph = engine->glyphIndex(ch.unicode());
   qreal lb;
   engine->getGlyphBearings(glyph, &lb);

   return lb;
}

qreal QFontMetricsF::rightBearing(QChar ch) const
{
   const int script = ch.script();
   QFontEngine *engine;

   if (d->capital == QFont::SmallCaps && ch.isLower()) {
      engine = d->smallCapsFontPrivate()->engineForScript(script);
   } else {
      engine = d->engineForScript(script);
   }

   Q_ASSERT(engine != nullptr);
   if (engine->type() == QFontEngine::Box) {
      return 0;
   }

   d->alterCharForCapitalization(ch);

   glyph_t glyph = engine->glyphIndex(ch.unicode());

   qreal rb;
   engine->getGlyphBearings(glyph, nullptr, &rb);

   return rb;
}

qreal QFontMetricsF::width(const QString &text) const
{
   int pos = text.indexOf('\x9c');
   int len = (pos != -1) ? pos : text.length();

   QStackTextEngine layout(text, QFont(d.data()));
   layout.itemize();

   return layout.width(0, len).toReal();
}

qreal QFontMetricsF::width(QChar ch) const
{
   if (ch.category() == QChar::Mark_NonSpacing) {
      return 0.;
   }

   const int script = ch.script();
   QFontEngine *engine;

   if (d->capital == QFont::SmallCaps && ch.isLower()) {
      engine = d->smallCapsFontPrivate()->engineForScript(script);
   } else {
      engine = d->engineForScript(script);
   }
   Q_ASSERT(engine != nullptr);

   d->alterCharForCapitalization(ch);

   glyph_t glyph = engine->glyphIndex(ch.unicode());
   QFixed advance;

   QGlyphLayout glyphs;
   glyphs.numGlyphs = 1;
   glyphs.glyphs = &glyph;
   glyphs.advances = &advance;
   engine->recalcAdvances(&glyphs, Qt::EmptyFlag);
   return advance.toReal();
}

QRectF QFontMetricsF::boundingRect(const QString &text) const
{
   int len = text.length();

   if (len == 0) {
      return QRectF();
   }

   QStackTextEngine layout(text, QFont(d.data()));
   layout.itemize();
   glyph_metrics_t gm = layout.boundingBox(0, len);

   return QRectF(gm.x.toReal(), gm.y.toReal(), gm.width.toReal(), gm.height.toReal());
}

QRectF QFontMetricsF::boundingRect(QChar ch) const
{
   const int script = ch.script();
   QFontEngine *engine;

   if (d->capital == QFont::SmallCaps && ch.isLower()) {
      engine = d->smallCapsFontPrivate()->engineForScript(script);
   } else {
      engine = d->engineForScript(script);
   }

   Q_ASSERT(engine != nullptr);
   d->alterCharForCapitalization(ch);

   glyph_t glyph = engine->glyphIndex(ch.unicode());

   glyph_metrics_t gm = engine->boundingBox(glyph);

   return QRectF(gm.x.toReal(), gm.y.toReal(), gm.width.toReal(), gm.height.toReal());
}

QRectF QFontMetricsF::boundingRect(const QRectF &rect, int flags, const QString &text,
   int tabStops, int *tabArray) const
{
   int tabArrayLen = 0;

   if (tabArray)
      while (tabArray[tabArrayLen]) {
         tabArrayLen++;
      }

   QRectF rb;
   qt_format_text(QFont(d.data()), rect, flags | Qt::TextDontPrint, text, &rb, tabStops, tabArray, tabArrayLen, nullptr);
   return rb;
}

QSizeF QFontMetricsF::size(int flags, const QString &text, int tabStops, int *tabArray) const
{
   return boundingRect(QRectF(), flags | Qt::TextLongestVariant, text, tabStops, tabArray).size();
}

QRectF QFontMetricsF::tightBoundingRect(const QString &text) const
{
   if (text.length() == 0) {
      return QRect();
   }

   QStackTextEngine layout(text, QFont(d.data()));
   layout.itemize();
   glyph_metrics_t gm = layout.tightBoundingBox(0, text.length());

   return QRectF(gm.x.toReal(), gm.y.toReal(), gm.width.toReal(), gm.height.toReal());
}

QString QFontMetricsF::elidedText(const QString &text, Qt::TextElideMode mode, qreal width, int flags) const
{
   QString _text = text;

   if (!(flags & Qt::TextLongestVariant)) {
      int posA = 0;
      int posB = _text.indexOf(QLatin1Char('\x9c'));

      while (posB >= 0) {
         QString portion = _text.mid(posA, posB - posA);
         if (size(flags, portion).width() <= width) {
            return portion;
         }
         posA = posB + 1;
         posB = _text.indexOf(QLatin1Char('\x9c'), posA);
      }
      _text = _text.mid(posA);
   }

   QStackTextEngine engine(_text, QFont(d.data()));
   return engine.elidedText(mode, QFixed::fromReal(width), flags);
}

/*!
    Returns the distance from the base line to where an underscore
    should be drawn.

    \sa overlinePos(), strikeOutPos(), lineWidth()
*/
qreal QFontMetricsF::underlinePos() const
{
   QFontEngine *engine = d->engineForScript(QChar::Script_Common);
   Q_ASSERT(engine != nullptr);

   return engine->underlinePosition().toReal();
}

/*!
    Returns the distance from the base line to where an overline
    should be drawn.

    \sa underlinePos(), strikeOutPos(), lineWidth()
*/
qreal QFontMetricsF::overlinePos() const
{
   return ascent() + 1;
}

/*!
    Returns the distance from the base line to where the strikeout
    line should be drawn.

    \sa underlinePos(), overlinePos(), lineWidth()
*/
qreal QFontMetricsF::strikeOutPos() const
{
   return ascent() / 3.;
}

/*!
    Returns the width of the underline and strikeout lines, adjusted
    for the point size of the font.

    \sa underlinePos(), overlinePos(), strikeOutPos()
*/
qreal QFontMetricsF::lineWidth() const
{
   QFontEngine *engine = d->engineForScript(QChar::Script_Common);
   Q_ASSERT(engine != nullptr);

   return engine->lineThickness().toReal();
}
