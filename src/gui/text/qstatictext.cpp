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

#include <qstatictext.h>
#include <qstatictext_p.h>
#include <qtextengine_p.h>
#include <qfontengine_p.h>
#include <qabstracttextdocumentlayout.h>

#include <qapplication.h>

QStaticText::QStaticText()
   : data(new QStaticTextPrivate)
{
}

QStaticText::QStaticText(const QString &text)
   : data(new QStaticTextPrivate)
{
   data->text = text;
   data->invalidate();
}

QStaticText::QStaticText(const QStaticText &other)
{
   data = other.data;
}

QStaticText::~QStaticText()
{
   Q_ASSERT(!data || data->ref.load() >= 1);
}

/*!
    \internal
*/
void QStaticText::detach()
{
   if (data->ref.load() != 1) {
      data.detach();
   }
}

void QStaticText::prepare(const QTransform &matrix, const QFont &font)
{
   data->matrix = matrix;
   data->font = font;
   data->init();
}

QStaticText &QStaticText::operator=(const QStaticText &other)
{
   data = other.data;
   return *this;
}

bool QStaticText::operator==(const QStaticText &other) const
{
   return (data == other.data || (data->text == other.data->text
            && data->font == other.data->font && data->textWidth == other.data->textWidth));
}

bool QStaticText::operator!=(const QStaticText &other) const
{
   return !(*this == other);
}

void QStaticText::setText(const QString &text)
{
   detach();
   data->text = text;
   data->invalidate();
}

/*!
   Sets the text format of the QStaticText to \a textFormat. If \a textFormat is set to
   Qt::AutoText (the default), the format of the text will try to be determined using the
   function Qt::mightBeRichText(). If the text format is Qt::PlainText, then the text will be
   displayed as is, whereas it will be interpreted as HTML if the format is Qt::RichText. HTML tags
   that alter the font of the text, its color, or its layout are supported by QStaticText.

   \note This function will cause the layout of the text to require recalculation.

   \sa textFormat(), setText(), text()
*/
void QStaticText::setTextFormat(Qt::TextFormat textFormat)
{
   detach();
   data->textFormat = textFormat;
   data->invalidate();
}

/*!
  Returns the text format of the QStaticText.

  \sa setTextFormat(), setText(), text()
*/
Qt::TextFormat QStaticText::textFormat() const
{
   return Qt::TextFormat(data->textFormat);
}

/*!
    Returns the text of the QStaticText.

    \sa setText()
*/
QString QStaticText::text() const
{
   return data->text;
}

/*!
  Sets the performance hint of the QStaticText according to the \a
  performanceHint provided. The \a performanceHint is used to
  customize how much caching is done internally to improve
  performance.

  The default is QStaticText::ModerateCaching.

  \note This function will cause the layout of the text to require recalculation.

  \sa performanceHint()
*/
void QStaticText::setPerformanceHint(PerformanceHint performanceHint)
{
   if ((performanceHint == ModerateCaching && !data->useBackendOptimizations)
      || (performanceHint == AggressiveCaching && data->useBackendOptimizations)) {
      return;
   }
   detach();
   data->useBackendOptimizations = (performanceHint == AggressiveCaching);
   data->invalidate();
}

/*!
  Returns which performance hint is set for the QStaticText.

  \sa setPerformanceHint()
*/
QStaticText::PerformanceHint QStaticText::performanceHint() const
{
   return data->useBackendOptimizations ? AggressiveCaching : ModerateCaching;
}

/*!
   Sets the text option structure that controls the layout process to the given \a textOption.

   \sa textOption()
*/
void QStaticText::setTextOption(const QTextOption &textOption)
{
   detach();
   data->textOption = textOption;
   data->invalidate();
}

/*!
    Returns the current text option used to control the layout process.
*/
QTextOption QStaticText::textOption() const
{
   return data->textOption;
}

/*!
    Sets the preferred width for this QStaticText. If the text is wider than the specified width,
    it will be broken into multiple lines and grow vertically. If the text cannot be split into
    multiple lines, it will be larger than the specified \a textWidth.

    Setting the preferred text width to a negative number will cause the text to be unbounded.

    Use size() to get the actual size of the text.

    \note This function will cause the layout of the text to require recalculation.

    \sa textWidth(), size()
*/
void QStaticText::setTextWidth(qreal textWidth)
{
   detach();
   data->textWidth = textWidth;
   data->invalidate();
}

qreal QStaticText::textWidth() const
{
   return data->textWidth;
}

QSizeF QStaticText::size() const
{
   if (data->needsRelayout) {
      data->init();
   }
   return data->actualSize;
}

QStaticTextPrivate::QStaticTextPrivate()
   : textWidth(-1.0), items(nullptr), itemCount(0), glyphPool(nullptr), positionPool(nullptr),
     needsRelayout(true), useBackendOptimizations(false), textFormat(Qt::AutoText),
     untransformedCoordinates(false)
{
}

QStaticTextPrivate::QStaticTextPrivate(const QStaticTextPrivate &other)
   : text(other.text), font(other.font), textWidth(other.textWidth), matrix(other.matrix),
     items(nullptr), itemCount(0), glyphPool(nullptr), positionPool(nullptr), textOption(other.textOption),
     needsRelayout(true), useBackendOptimizations(other.useBackendOptimizations),
     textFormat(other.textFormat), untransformedCoordinates(other.untransformedCoordinates)
{
}

QStaticTextPrivate::~QStaticTextPrivate()
{
   delete[] items;
   delete[] glyphPool;
   delete[] positionPool;
}

QStaticTextPrivate *QStaticTextPrivate::get(const QStaticText *q)
{
   return q->data.data();
}

namespace {

class DrawTextItemRecorder: public QPaintEngine
{
 public:
   DrawTextItemRecorder(bool untransformedCoordinates, bool useBackendOptimizations)
      : m_dirtyPen(false), m_useBackendOptimizations(useBackendOptimizations),
        m_untransformedCoordinates(untransformedCoordinates), m_currentColor(Qt::black) {
   }

   void updateState(const QPaintEngineState &newState) override {
      if (newState.state() & QPaintEngine::DirtyPen && newState.pen().color() != m_currentColor) {
         m_dirtyPen = true;
         m_currentColor = newState.pen().color();
      }
   }

   void drawTextItem(const QPointF &position, const QTextItem &textItem) override {
      const QTextItemInt &ti = static_cast<const QTextItemInt &>(textItem);

      QStaticTextItem currentItem;
      currentItem.setFontEngine(ti.fontEngine);

      currentItem.font           = ti.font();
      currentItem.glyphOffset    = m_glyphs.size();    // Store offset into glyph pool
      currentItem.positionOffset = m_glyphs.size();    // Offset into position pool

      currentItem.useBackendOptimizations = m_useBackendOptimizations;

      if (m_dirtyPen) {
         currentItem.color = m_currentColor;
      }

      QTransform matrix = m_untransformedCoordinates ? QTransform() : state->transform();
      matrix.translate(position.x(), position.y());

      QVarLengthArray<glyph_t> glyphs;
      QVarLengthArray<QFixedPoint> positions;
      ti.fontEngine->getGlyphPositions(ti.glyphs, matrix, ti.flags, glyphs, positions);

      int size = glyphs.size();
      Q_ASSERT(size == positions.size());

      currentItem.numGlyphs = size;
      m_glyphs.resize(m_glyphs.size() + size);
      m_positions.resize(m_glyphs.size());

      glyph_t *glyphsDestination = m_glyphs.data() + currentItem.glyphOffset;
      memcpy(glyphsDestination, glyphs.constData(), sizeof(glyph_t) * currentItem.numGlyphs);

      QFixedPoint *positionsDestination = m_positions.data() + currentItem.positionOffset;
      memcpy(positionsDestination, positions.constData(), sizeof(QFixedPoint) * currentItem.numGlyphs);

      m_items.append(currentItem);
   }

   void drawPolygon(const QPointF *, int, PolygonDrawMode)  override {
      /* intentionally empty */
   }

   bool begin(QPaintDevice *)  override {
      return true;
   }

   bool end()  override {
      return true;
   }

   void drawPixmap(const QRectF &, const QPixmap &, const QRectF &)  override {}

   virtual Type type() const override {
      return User;
   }

   QVector<QStaticTextItem> items() const {
      return m_items;
   }

   QVector<QFixedPoint> positions() const {
      return m_positions;
   }

   QVector<glyph_t> glyphs() const {
      return m_glyphs;
   }

 private:
   QVector<QStaticTextItem> m_items;
   QVector<QFixedPoint> m_positions;
   QVector<glyph_t> m_glyphs;

   bool m_dirtyPen;
   bool m_useBackendOptimizations;
   bool m_untransformedCoordinates;
   QColor m_currentColor;
};

class DrawTextItemDevice: public QPaintDevice
{
 public:
   DrawTextItemDevice(bool untransformedCoordinates, bool useBackendOptimizations) {
      m_paintEngine = new DrawTextItemRecorder(untransformedCoordinates, useBackendOptimizations);
   }

   ~DrawTextItemDevice() {
      delete m_paintEngine;
   }

   int metric(PaintDeviceMetric m) const  override {
      int val;
      switch (m) {
         case PdmWidth:
         case PdmHeight:
         case PdmWidthMM:
         case PdmHeightMM:
            val = 0;
            break;
         case PdmDpiX:
         case PdmPhysicalDpiX:
            val = qt_defaultDpiX();
            break;
         case PdmDpiY:
         case PdmPhysicalDpiY:
            val = qt_defaultDpiY();
            break;
         case PdmNumColors:
            val = 16777216;
            break;
         case PdmDepth:
            val = 24;
            break;
         case PdmDevicePixelRatio:
            val = 1;
            break;
         case PdmDevicePixelRatioScaled:
            val = devicePixelRatioFScale();
            break;
         default:
            val = 0;
            qWarning("DrawTextItemDevice::metric() Invalid metric command");
      }
      return val;
   }

   virtual QPaintEngine *paintEngine() const  override {
      return m_paintEngine;
   }

   QVector<glyph_t> glyphs() const {
      return m_paintEngine->glyphs();
   }

   QVector<QFixedPoint> positions() const {
      return m_paintEngine->positions();
   }

   QVector<QStaticTextItem> items() const {
      return m_paintEngine->items();
   }

 private:
   DrawTextItemRecorder *m_paintEngine;
};
}

void QStaticTextPrivate::paintText(const QPointF &topLeftPosition, QPainter *p)
{
   bool preferRichText = textFormat == Qt::RichText
      || (textFormat == Qt::AutoText && Qt::mightBeRichText(text));

   if (! preferRichText) {
      QTextLayout textLayout;
      textLayout.setText(text);
      textLayout.setFont(font);
      textLayout.setTextOption(textOption);
      textLayout.setCacheEnabled(true);

      qreal leading = QFontMetricsF(font).leading();
      qreal height = -leading;

      textLayout.beginLayout();
      while (true) {
         QTextLine line = textLayout.createLine();

         if (! line.isValid()) {
            break;
         }

         if (textWidth >= 0.0) {
            line.setLineWidth(textWidth);
         }

         height += leading;
         line.setPosition(QPointF(0.0, height));
         height += line.height();
      }
      textLayout.endLayout();

      actualSize = textLayout.boundingRect().size();
      textLayout.draw(p, topLeftPosition);

   } else {
      QTextDocument document;

#ifndef QT_NO_CSSPARSER
      QColor color = p->pen().color();
      document.setDefaultStyleSheet(QString::fromLatin1("body { color: #%1%2%3 }")
         .formatArg(QString::number(color.red(), 16),   2, QLatin1Char('0'))
         .formatArg(QString::number(color.green(), 16), 2, QLatin1Char('0'))
         .formatArg(QString::number(color.blue(), 16),  2, QLatin1Char('0')));
#endif
      document.setDefaultFont(font);
      document.setDocumentMargin(0.0);
#ifndef QT_NO_TEXTHTMLPARSER
      document.setHtml(text);
#else
      document.setPlainText(text);
#endif
      if (textWidth >= 0.0) {
         document.setTextWidth(textWidth);
      } else {
         document.adjustSize();
      }
      document.setDefaultTextOption(textOption);

      p->save();
      p->translate(topLeftPosition);
      QAbstractTextDocumentLayout::PaintContext ctx;
      ctx.palette.setColor(QPalette::Text, p->pen().color());
      document.documentLayout()->draw(p, ctx);
      p->restore();

      if (textWidth >= 0.0) {
         document.adjustSize();   // Find optimal size
      }

      actualSize = document.size();
   }
}

void QStaticTextPrivate::init()
{
   delete[] items;
   delete[] glyphPool;
   delete[] positionPool;

   position = QPointF(0, 0);

   DrawTextItemDevice device(untransformedCoordinates, useBackendOptimizations);
   {
      QPainter painter(&device);
      painter.setFont(font);
      painter.setTransform(matrix);

      paintText(QPointF(0, 0), &painter);
   }

   QVector<QStaticTextItem> deviceItems = device.items();
   QVector<QFixedPoint> positions = device.positions();
   QVector<glyph_t> glyphs = device.glyphs();

   itemCount = deviceItems.size();
   items     = new QStaticTextItem[itemCount];

   glyphPool = new glyph_t[glyphs.size()];
   memcpy(glyphPool, glyphs.constData(), glyphs.size() * sizeof(glyph_t));

   positionPool = new QFixedPoint[positions.size()];
   memcpy(positionPool, positions.constData(), positions.size() * sizeof(QFixedPoint));

   for (int i = 0; i < itemCount; ++i) {
      items[i] = deviceItems.at(i);

      items[i].glyphs = glyphPool + items[i].glyphOffset;
      items[i].glyphPositions = positionPool + items[i].positionOffset;
   }

   needsRelayout = false;
}

QStaticTextItem::~QStaticTextItem()
{
   if (m_userData != nullptr && !m_userData->ref.deref()) {
      delete m_userData;
   }

   setFontEngine(nullptr);
}

void QStaticTextItem::setFontEngine(QFontEngine *fe)
{
   if (m_fontEngine == fe) {
      return;
   }

   if (m_fontEngine != nullptr && ! m_fontEngine->m_refCount.deref()) {
      delete m_fontEngine;
   }

   m_fontEngine = fe;

   if (m_fontEngine != nullptr) {
      m_fontEngine->m_refCount.ref();
   }
}

