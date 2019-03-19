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

#include <qfont.h>
#include <qpaintdevice.h>
#include <qfontmetrics.h>
#include <qfont_p.h>
#include <qfontengine_p.h>
#include <qunicodetables_p.h>
#include <math.h>

#ifdef Q_WS_X11
#include <qx11info_x11.h>
#endif

QT_BEGIN_NAMESPACE

#ifdef Q_WS_X11
extern const QX11Info *qt_x11Info(const QPaintDevice *pd);
#endif

extern void qt_format_text(const QFont &font, const QRectF &_r,
                           int tf, const QString &text, QRectF *brect,
                           int tabStops, int *tabArray, int tabArrayLen,
                           QPainter *painter);

QFontMetrics::QFontMetrics(const QFont &font)
   : d(font.d.data())
{
}

QFontMetrics::QFontMetrics(const QFont &font, QPaintDevice *paintdevice)
{
   int dpi = paintdevice ? paintdevice->logicalDpiY() : qt_defaultDpi();
#ifdef Q_WS_X11
   const QX11Info *info = qt_x11Info(paintdevice);
   int screen = info ? info->screen() : 0;
#else
   const int screen = 0;
#endif
   if (font.d->dpi != dpi || font.d->screen != screen ) {
      d = new QFontPrivate(*font.d);
      d->dpi = dpi;
      d->screen = screen;
   } else {
      d = font.d.data();
   }

}

/*!
    Constructs a copy of \a fm.
*/
QFontMetrics::QFontMetrics(const QFontMetrics &fm)
   : d(fm.d.data())
{
}

/*!
    Destroys the font metrics object and frees all allocated
    resources.
*/
QFontMetrics::~QFontMetrics()
{
}

/*!
    Assigns the font metrics \a fm.
*/
QFontMetrics &QFontMetrics::operator=(const QFontMetrics &fm)
{
   d = fm.d.data();
   return *this;
}

/*!
    \overload
    Returns true if \a other is equal to this object; otherwise
    returns false.

    Two font metrics are considered equal if they were constructed
    from the same QFont and the paint devices they were constructed
    for are considered compatible.

    \sa operator!=()
*/
bool QFontMetrics::operator ==(const QFontMetrics &other) const
{
   return d == other.d;
}

/*!
    Returns true if \a other is equal to this object; otherwise
    returns false.

    Two font metrics are considered equal if they were constructed
    from the same QFont and the paint devices they were constructed
    for are considered compatible.

    \sa operator!=()
*/
bool QFontMetrics::operator ==(const QFontMetrics &other)
{
   return d == other.d;
}

/*!
    \fn bool QFontMetrics::operator!=(const QFontMetrics &other)

    Returns true if \a other is not equal to this object; otherwise returns false.

    Two font metrics are considered equal if they were constructed
    from the same QFont and the paint devices they were constructed
    for are considered compatible.

    \sa operator==()
*/

/*!
    \fn bool QFontMetrics::operator !=(const QFontMetrics &other) const

    Returns true if \a other is not equal to this object; otherwise returns false.

    Two font metrics are considered equal if they were constructed
    from the same QFont and the paint devices they were constructed
    for are considered compatible.

    \sa operator==()
*/

/*!
    Returns the ascent of the font.

    The ascent of a font is the distance from the baseline to the
    highest position characters extend to. In practice, some font
    designers break this rule, e.g. when they put more than one accent
    on top of a character, or to accommodate an unusual character in
    an exotic language, so it is possible (though rare) that this
    value will be too small.

    \sa descent()
*/
int QFontMetrics::ascent() const
{
   QFontEngine *engine = d->engineForScript(QChar::Script_Common);
   Q_ASSERT(engine != 0);
   return qRound(engine->ascent());
}


/*!
    Returns the descent of the font.

    The descent is the distance from the base line to the lowest point
    characters extend to. In practice, some font designers break this rule,
    e.g. to accommodate an unusual character in an exotic language, so
    it is possible (though rare) that this value will be too small.

    \sa ascent()
*/
int QFontMetrics::descent() const
{
   QFontEngine *engine = d->engineForScript(QChar::Script_Common);
   Q_ASSERT(engine != 0);
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
   Q_ASSERT(engine != 0);
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
   Q_ASSERT(engine != 0);
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
   Q_ASSERT(engine != 0);
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
   Q_ASSERT(engine != 0);
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
   Q_ASSERT(engine != 0);
   return qRound(engine->minRightBearing());
}

/*!
    Returns the width of the widest character in the font.
*/
int QFontMetrics::maxWidth() const
{
   QFontEngine *engine = d->engineForScript(QChar::Script_Common);
   Q_ASSERT(engine != 0);
   return qRound(engine->maxCharWidth());
}

/*!
    Returns the 'x' height of the font. This is often but not always
    the same as the height of the character 'x'.
*/
int QFontMetrics::xHeight() const
{
   QFontEngine *engine = d->engineForScript(QChar::Script_Common);
   Q_ASSERT(engine != 0);
   if (d->capital == QFont::SmallCaps) {
      return qRound(d->smallCapsFontPrivate()->engineForScript(QChar::Script_Common)->ascent());
   }
   return qRound(engine->xHeight());
}

int QFontMetrics::averageCharWidth() const
{
   QFontEngine *engine = d->engineForScript(QChar::Script_Common);
   Q_ASSERT(engine != 0);
   return qRound(engine->averageCharWidth());
}

bool QFontMetrics::inFont(QChar ch) const
{
   const int script = ch.script();
   QFontEngine *engine = d->engineForScript(script);
   Q_ASSERT(engine != 0);

   if (engine->type() == QFontEngine::Box) {
      return false;
   }

   return engine->canRender(QString(ch));
}

bool QFontMetrics::inFontUcs4(uint ucs4) const
{
   QChar ch = QChar(char32_t(ucs4));
   const int script_id = ch.script();

   QFontEngine *engine = d->engineForScript(script_id);
   Q_ASSERT(engine != 0);

   if (engine->type() == QFontEngine::Box) {
      return false;
   }

   return engine->canRender(QString(ch));
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

   Q_ASSERT(engine != 0);
   if (engine->type() == QFontEngine::Box) {
      return 0;
   }

   d->alterCharForCapitalization(ch);

   QGlyphLayoutArray<10> glyphs;
   int nglyphs = 9;

   engine->stringToCMap(QString(ch), &glyphs, &nglyphs, 0);

   // ### can nglyphs != 1 happen at all? Not currently I think
   qreal lb;
   engine->getGlyphBearings(glyphs.glyphs[0], &lb);

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

   Q_ASSERT(engine != 0);
   if (engine->type() == QFontEngine::Box) {
      return 0;
   }

   d->alterCharForCapitalization(ch);

   QGlyphLayoutArray<10> glyphs;
   int nglyphs = 9;
   engine->stringToCMap(QString(ch), &glyphs, &nglyphs, 0);

   // ### can nglyphs != 1 happen at all? Not currently I think
   qreal rb;
   engine->getGlyphBearings(glyphs.glyphs[0], 0, &rb);

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

      if (! engine->stringToCMap(text, &glyphs, &numGlyphs, 0)) {
         glyphs.resize(numGlyphs);

         if (! engine->stringToCMap(text, &glyphs, &numGlyphs, 0)) {
            Q_ASSERT_X(false, Q_FUNC_INFO, "stringToCMap should not fail twice");
         }
      }

      QFixed width;

      for (int i = 0; i < numGlyphs; ++i) {
         width += glyphs.advances_x[i];
      }

      return qRound(width);
   }

   QStackTextEngine layout(text, d.data());
   layout.ignoreBidi = true;
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

   Q_ASSERT(engine != 0);
   d->alterCharForCapitalization(ch);

   QGlyphLayoutArray<8> glyphs;
   int nglyphs = 7;
   engine->stringToCMap(QString(ch), &glyphs, &nglyphs, 0);

   return qRound(glyphs.advances_x[0]);
}

// obsolete
int QFontMetrics::charWidth(const QString &text, int pos) const
{
   if (pos < 0 || pos > text.length()) {
      return 0;
   }

   QChar ch = text[pos];

   const int script = ch.script();
   int width;

   if (script != QChar::Script_Common) {
      // complex script shaping. Have to do some hard work
      int from = qMax(0, pos - 8);
      int to   = qMin(text.length(), pos + 8);

      QString cstr = text.mid(from, to - from);
      QStackTextEngine layout(cstr, d.data());

      layout.ignoreBidi = true;
      layout.itemize();
      width = qRound(layout.width(pos - from, 1));

   } else if (ch.category() == QChar::Mark_NonSpacing) {
      width = 0;

   } else {
      QFontEngine *engine;

      if (d->capital == QFont::SmallCaps && ch.isLower()) {
         engine = d->smallCapsFontPrivate()->engineForScript(script);
      } else {
         engine = d->engineForScript(script);
      }

      Q_ASSERT(engine != 0);

      d->alterCharForCapitalization(ch);

      QGlyphLayoutArray<8> glyphs;
      int nglyphs = 7;
      engine->stringToCMap(QString(ch), &glyphs, &nglyphs, 0);
      width = qRound(glyphs.advances_x[0]);
   }

   return width;
}

QRect QFontMetrics::boundingRect(const QString &text) const
{
   if (text.length() == 0) {
      return QRect();
   }

   QStackTextEngine layout(text, d.data());
   layout.ignoreBidi = true;
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

   Q_ASSERT(engine != 0);
   d->alterCharForCapitalization(ch);

   QGlyphLayoutArray<10> glyphs;
   int nglyphs = 9;
   engine->stringToCMap(QString(ch), &glyphs, &nglyphs, 0);
   glyph_metrics_t gm = engine->boundingBox(glyphs.glyphs[0]);
   return QRect(qRound(gm.x), qRound(gm.y), qRound(gm.width), qRound(gm.height));
}

/*!
    \overload

    Returns the bounding rectangle of the characters in the string
    specified by \a text, which is the set of pixels the text would
    cover if drawn at (0, 0). The drawing, and hence the bounding
    rectangle, is constrained to the rectangle \a rect.

    The \a flags argument is the bitwise OR of the following flags:
    \list
    \o Qt::AlignLeft aligns to the left border, except for
          Arabic and Hebrew where it aligns to the right.
    \o Qt::AlignRight aligns to the right border, except for
          Arabic and Hebrew where it aligns to the left.
    \o Qt::AlignJustify produces justified text.
    \o Qt::AlignHCenter aligns horizontally centered.
    \o Qt::AlignTop aligns to the top border.
    \o Qt::AlignBottom aligns to the bottom border.
    \o Qt::AlignVCenter aligns vertically centered
    \o Qt::AlignCenter (== \c{Qt::AlignHCenter | Qt::AlignVCenter})
    \o Qt::TextSingleLine ignores newline characters in the text.
    \o Qt::TextExpandTabs expands tabs (see below)
    \o Qt::TextShowMnemonic interprets "&x" as \underline{x}; i.e., underlined.
    \o Qt::TextWordWrap breaks the text to fit the rectangle.
    \endlist

    Qt::Horizontal alignment defaults to Qt::AlignLeft and vertical
    alignment defaults to Qt::AlignTop.

    If several of the horizontal or several of the vertical alignment
    flags are set, the resulting alignment is undefined.

    If Qt::TextExpandTabs is set in \a flags, then: if \a tabArray is
    non-null, it specifies a 0-terminated sequence of pixel-positions
    for tabs; otherwise if \a tabStops is non-zero, it is used as the
    tab spacing (in pixels).

    Note that the bounding rectangle may extend to the left of (0, 0),
    e.g. for italicized fonts, and that the text output may cover \e
    all pixels in the bounding rectangle.

    Newline characters are processed as linebreaks.

    Despite the different actual character heights, the heights of the
    bounding rectangles of "Yes" and "yes" are the same.

    The bounding rectangle returned by this function is somewhat larger
    than that calculated by the simpler boundingRect() function. This
    function uses the \link minLeftBearing() maximum left \endlink and
    \link minRightBearing() right \endlink font bearings as is
    necessary for multi-line text to align correctly. Also,
    fontHeight() and lineSpacing() are used to calculate the height,
    rather than individual character heights.

    \sa width(), QPainter::boundingRect(), Qt::Alignment
*/
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
   qt_format_text(QFont(d.data()), rr, flags | Qt::TextDontPrint, text, &rb, tabStops, tabArray,
                  tabArrayLen, 0);

   return rb.toAlignedRect();
}

/*!
    Returns the size in pixels of \a text.

    The \a flags argument is the bitwise OR of the following flags:
    \list
    \o Qt::TextSingleLine ignores newline characters.
    \o Qt::TextExpandTabs expands tabs (see below)
    \o Qt::TextShowMnemonic interprets "&x" as \underline{x}; i.e., underlined.
    \o Qt::TextWordBreak breaks the text to fit the rectangle.
    \endlist

    If Qt::TextExpandTabs is set in \a flags, then: if \a tabArray is
    non-null, it specifies a 0-terminated sequence of pixel-positions
    for tabs; otherwise if \a tabStops is non-zero, it is used as the
    tab spacing (in pixels).

    Newline characters are processed as linebreaks.

    Despite the different actual character heights, the heights of the
    bounding rectangles of "Yes" and "yes" are the same.

    \sa boundingRect()
*/
QSize QFontMetrics::size(int flags, const QString &text, int tabStops, int *tabArray) const
{
   return boundingRect(QRect(0, 0, 0, 0), flags | Qt::TextLongestVariant, text, tabStops, tabArray).size();
}

/*!
  \since 4.3

    Returns a tight bounding rectangle around the characters in the
    string specified by \a text. The bounding rectangle always covers
    at least the set of pixels the text would cover if drawn at (0,
    0).

    Note that the bounding rectangle may extend to the left of (0, 0),
    e.g. for italicized fonts, and that the width of the returned
    rectangle might be different than what the width() method returns.

    If you want to know the advance width of the string (to layout
    a set of strings next to each other), use width() instead.

    Newline characters are processed as normal characters, \e not as
    linebreaks.

    \warning Calling this method is very slow on Windows.

    \sa width(), height(), boundingRect()
*/
QRect QFontMetrics::tightBoundingRect(const QString &text) const
{
   if (text.length() == 0) {
      return QRect();
   }

   QStackTextEngine layout(text, d.data());
   layout.ignoreBidi = true;
   layout.itemize();
   glyph_metrics_t gm = layout.tightBoundingBox(0, text.length());
   return QRect(qRound(gm.x), qRound(gm.y), qRound(gm.width), qRound(gm.height));
}


/*!
    \since 4.2

    If the string \a text is wider than \a width, returns an elided
    version of the string (i.e., a string with "..." in it).
    Otherwise, returns the original string.

    The \a mode parameter specifies whether the text is elided on the
    left (e.g., "...tech"), in the middle (e.g., "Tr...ch"), or on
    the right (e.g., "Trol...").

    The \a width is specified in pixels, not characters.

    The \a flags argument is optional and currently only supports
    Qt::TextShowMnemonic as value.

    The elide mark will follow the \l{Qt::LayoutDirection}{layout
    direction}; it will be on the right side of the text for
    right-to-left layouts, and on the left side for right-to-left
    layouts. Note that this behavior is independent of the text
    language.
*/
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
   Q_ASSERT(engine != 0);
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
   Q_ASSERT(engine != 0);
   return qRound(engine->lineThickness());
}




/*****************************************************************************
  QFontMetricsF member functions
 *****************************************************************************/

/*!
    \class QFontMetricsF
    \reentrant

    \brief The QFontMetricsF class provides font metrics information.

    \ingroup painting
    \ingroup shared

    QFontMetricsF functions calculate the size of characters and
    strings for a given font. You can construct a QFontMetricsF object
    with an existing QFont to obtain metrics for that font. If the
    font is changed later, the font metrics object is \e not updated.

    Once created, the object provides functions to access the
    individual metrics of the font, its characters, and for strings
    rendered in the font.

    There are several functions that operate on the font: ascent(),
    descent(), height(), leading() and lineSpacing() return the basic
    size properties of the font. The underlinePos(), overlinePos(),
    strikeOutPos() and lineWidth() functions, return the properties of
    the line that underlines, overlines or strikes out the
    characters. These functions are all fast.

    There are also some functions that operate on the set of glyphs in
    the font: minLeftBearing(), minRightBearing() and maxWidth().
    These are by necessity slow, and we recommend avoiding them if
    possible.

    For each character, you can get its width(), leftBearing() and
    rightBearing() and find out whether it is in the font using
    inFont(). You can also treat the character as a string, and use
    the string functions on it.

    The string functions include width(), to return the width of a
    string in pixels (or points, for a printer), boundingRect(), to
    return a rectangle large enough to contain the rendered string,
    and size(), to return the size of that rectangle.

    Example:
    \snippet doc/src/snippets/code/src_gui_text_qfontmetrics.cpp 1

    \sa QFont QFontInfo QFontDatabase
*/

/*!
    \since 4.2

    Constructs a font metrics object with floating point precision
    from the given \a fontMetrics object.
*/
QFontMetricsF::QFontMetricsF(const QFontMetrics &fontMetrics)
   : d(fontMetrics.d.data())
{
}

/*!
    \since 4.2

    Assigns \a other to this object.
*/
QFontMetricsF &QFontMetricsF::operator=(const QFontMetrics &other)
{
   d = other.d.data();
   return *this;
}

/*!
    Constructs a font metrics object for \a font.

    The font metrics will be compatible with the paintdevice used to
    create \a font.

    The font metrics object holds the information for the font that is
    passed in the constructor at the time it is created, and is not
    updated if the font's attributes are changed later.

    Use QFontMetricsF(const QFont &, QPaintDevice *) to get the font
    metrics that are compatible with a certain paint device.
*/
QFontMetricsF::QFontMetricsF(const QFont &font)
   : d(font.d.data())
{
}

/*!
    Constructs a font metrics object for \a font and \a paintdevice.

    The font metrics will be compatible with the paintdevice passed.
    If the \a paintdevice is 0, the metrics will be screen-compatible,
    ie. the metrics you get if you use the font for drawing text on a
    \link QWidget widgets\endlink or \link QPixmap pixmaps\endlink,
    not on a QPicture or QPrinter.

    The font metrics object holds the information for the font that is
    passed in the constructor at the time it is created, and is not
    updated if the font's attributes are changed later.
*/
QFontMetricsF::QFontMetricsF(const QFont &font, QPaintDevice *paintdevice)
{
   int dpi = paintdevice ? paintdevice->logicalDpiY() : qt_defaultDpi();
#ifdef Q_WS_X11
   const QX11Info *info = qt_x11Info(paintdevice);
   int screen = info ? info->screen() : 0;
#else
   const int screen = 0;
#endif
   if (font.d->dpi != dpi || font.d->screen != screen ) {
      d = new QFontPrivate(*font.d);
      d->dpi = dpi;
      d->screen = screen;
   } else {
      d = font.d.data();
   }

}

/*!
    Constructs a copy of \a fm.
*/
QFontMetricsF::QFontMetricsF(const QFontMetricsF &fm)
   : d(fm.d.data())
{
}

/*!
    Destroys the font metrics object and frees all allocated
    resources.
*/
QFontMetricsF::~QFontMetricsF()
{
}

/*!
    Assigns the font metrics \a fm to this font metrics object.
*/
QFontMetricsF &QFontMetricsF::operator=(const QFontMetricsF &fm)
{
   d = fm.d.data();
   return *this;
}

/*!
  \overload
  Returns true if the font metrics are equal to the \a other font
  metrics; otherwise returns false.

  Two font metrics are considered equal if they were constructed from the
  same QFont and the paint devices they were constructed for are
  considered to be compatible.
*/
bool QFontMetricsF::operator ==(const QFontMetricsF &other) const
{
   return d == other.d;
}

/*!
  Returns true if the font metrics are equal to the \a other font
  metrics; otherwise returns false.

  Two font metrics are considered equal if they were constructed from the
  same QFont and the paint devices they were constructed for are
  considered to be compatible.
*/
bool QFontMetricsF::operator ==(const QFontMetricsF &other)
{
   return d == other.d;
}

/*!
    \fn bool QFontMetricsF::operator!=(const QFontMetricsF &other)

    Returns true if the font metrics are not equal to the \a other font
    metrics; otherwise returns false.

    \sa operator==()
*/

/*!
    \fn bool QFontMetricsF::operator !=(const QFontMetricsF &other) const
    \overload

    Returns true if the font metrics are not equal to the \a other font
    metrics; otherwise returns false.

    \sa operator==()
*/

/*!
    Returns the ascent of the font.

    The ascent of a font is the distance from the baseline to the
    highest position characters extend to. In practice, some font
    designers break this rule, e.g. when they put more than one accent
    on top of a character, or to accommodate an unusual character in
    an exotic language, so it is possible (though rare) that this
    value will be too small.

    \sa descent()
*/
qreal QFontMetricsF::ascent() const
{
   QFontEngine *engine = d->engineForScript(QChar::Script_Common);
   Q_ASSERT(engine != 0);
   return engine->ascent().toReal();
}


/*!
    Returns the descent of the font.

    The descent is the distance from the base line to the lowest point
    characters extend to. (Note that this is different from X, which
    adds 1 pixel.) In practice, some font designers break this rule,
    e.g. to accommodate an unusual character in an exotic language, so
    it is possible (though rare) that this value will be too small.

    \sa ascent()
*/
qreal QFontMetricsF::descent() const
{
   QFontEngine *engine = d->engineForScript(QChar::Script_Common);
   Q_ASSERT(engine != 0);
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
   Q_ASSERT(engine != 0);

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
   Q_ASSERT(engine != 0);
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
   Q_ASSERT(engine != 0);
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
   Q_ASSERT(engine != 0);
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
   Q_ASSERT(engine != 0);
   return engine->minRightBearing();
}

/*!
    Returns the width of the widest character in the font.
*/
qreal QFontMetricsF::maxWidth() const
{
   QFontEngine *engine = d->engineForScript(QChar::Script_Common);
   Q_ASSERT(engine != 0);
   return engine->maxCharWidth();
}

/*!
    Returns the 'x' height of the font. This is often but not always
    the same as the height of the character 'x'.
*/
qreal QFontMetricsF::xHeight() const
{
   QFontEngine *engine = d->engineForScript(QChar::Script_Common);
   Q_ASSERT(engine != 0);

   if (d->capital == QFont::SmallCaps) {
      return d->smallCapsFontPrivate()->engineForScript(QChar::Script_Common)->ascent().toReal();
   }

   return engine->xHeight().toReal();
}

qreal QFontMetricsF::averageCharWidth() const
{
   QFontEngine *engine = d->engineForScript(QChar::Script_Common);
   Q_ASSERT(engine != 0);

   return engine->averageCharWidth().toReal();
}

bool QFontMetricsF::inFont(QChar ch) const
{
   const int script = ch.script();
   QFontEngine *engine = d->engineForScript(script);

   Q_ASSERT(engine != 0);
   if (engine->type() == QFontEngine::Box) {
      return false;
   }

   return engine->canRender(QString(ch));
}

bool QFontMetricsF::inFontUcs4(uint ucs4) const
{
   QChar ch = QChar(char32_t(ucs4));

   const int script = ch.script();
   QFontEngine *engine = d->engineForScript(script);

   Q_ASSERT(engine != 0);

   if (engine->type() == QFontEngine::Box) {
      return false;
   }

   return engine->canRender(QString(ch));
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

   Q_ASSERT(engine != 0);
   if (engine->type() == QFontEngine::Box) {
      return 0;
   }

   d->alterCharForCapitalization(ch);

   QGlyphLayoutArray<10> glyphs;
   int nglyphs = 9;
   engine->stringToCMap(QString(ch), &glyphs, &nglyphs, 0);

   // ### can nglyphs != 1 happen at all? Not currently I think
   qreal lb;
   engine->getGlyphBearings(glyphs.glyphs[0], &lb);

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

   Q_ASSERT(engine != 0);
   if (engine->type() == QFontEngine::Box) {
      return 0;
   }

   d->alterCharForCapitalization(ch);

   QGlyphLayoutArray<10> glyphs;
   int nglyphs = 9;

   engine->stringToCMap(QString(ch), &glyphs, &nglyphs, 0);

   // ### can nglyphs != 1 happen at all? Not currently I think
   qreal rb;
   engine->getGlyphBearings(glyphs.glyphs[0], 0, &rb);

   return rb;
}

qreal QFontMetricsF::width(const QString &text) const
{
   int pos = text.indexOf('\x9c');
   int len = (pos != -1) ? pos : text.length();

   QStackTextEngine layout(text, d.data());
   layout.ignoreBidi = true;
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
   Q_ASSERT(engine != 0);

   d->alterCharForCapitalization(ch);

   QGlyphLayoutArray<8> glyphs;
   int nglyphs = 7;

   engine->stringToCMap(QString(ch), &glyphs, &nglyphs, 0);
   return glyphs.advances_x[0].toReal();
}

QRectF QFontMetricsF::boundingRect(const QString &text) const
{
   int len = text.length();

   if (len == 0) {
      return QRectF();
   }

   QStackTextEngine layout(text, d.data());
   layout.ignoreBidi = true;
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

   Q_ASSERT(engine != 0);
   d->alterCharForCapitalization(ch);

   QGlyphLayoutArray<10> glyphs;
   int nglyphs = 9;

   engine->stringToCMap(QString(ch), &glyphs, &nglyphs, 0);
   glyph_metrics_t gm = engine->boundingBox(glyphs.glyphs[0]);

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
   qt_format_text(QFont(d.data()), rect, flags | Qt::TextDontPrint, text, &rb, tabStops, tabArray, tabArrayLen, 0);
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

   QStackTextEngine layout(text, d.data());
   layout.ignoreBidi = true;
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
   Q_ASSERT(engine != 0);
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
   Q_ASSERT(engine != 0);
   return engine->lineThickness().toReal();
}

/*!
    \fn QSize QFontMetrics::size(int flags, const QString &text, int len,
                                 int tabStops, int *tabArray) const
    \compat

    Use the size() function in combination with QString::left()
    instead.

    \oldcode
        QSize size = size(flags, str, len, tabstops, tabarray);
    \newcode
        QSize size = size(flags, str.left(len), tabstops, tabarray);
    \endcode
*/

/*!
    \fn QRect QFontMetrics::boundingRect(int x, int y, int w, int h, int flags,
        const QString& text, int len, int tabStops, int *tabArray) const
    \compat

    Use the boundingRect() function in combination with
    QString::left() and a QRect constructor instead.

    \oldcode
        QRect rect = boundingRect(x, y, w, h , flags, text, len,
                                  tabStops, tabArray);
    \newcode
        QRect rect = boundingRect(QRect(x, y, w, h), flags, text.left(len),
                                  tabstops, tabarray);
    \endcode

*/

/*!
    \fn QRect QFontMetrics::boundingRect(const QString &text, int len) const
    \compat

    Use the boundingRect() function in combination with
    QString::left() instead.

    \oldcode
        QRect rect = boundingRect(text, len);
    \newcode
        QRect rect = boundingRect(text.left(len));
    \endcode
*/

QT_END_NAMESPACE
