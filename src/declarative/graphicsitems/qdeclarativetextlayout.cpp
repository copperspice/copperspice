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

#include <qdeclarativetextlayout_p.h>
#include <qstatictext_p.h>
#include <qfontengine_p.h>
#include <qtextengine_p.h>
#include <qpainter_p.h>
#include <qpaintengineex_p.h>

QT_BEGIN_NAMESPACE

class QDeclarativeTextLayoutPrivate
{
 public:
   QDeclarativeTextLayoutPrivate()
      : cached(false) {}

   QPointF position;

   bool cached;
   QVector<QStaticTextItem> items;
   QVector<QFixedPoint> positions;
   QVector<glyph_t> glyphs;
   QVector<QChar> chars;
};

namespace {
class DrawTextItemRecorder: public QPaintEngine
{
 public:
   DrawTextItemRecorder(bool untransformedCoordinates, bool useBackendOptimizations)
      : m_inertText(0), m_dirtyPen(false), m_useBackendOptimizations(useBackendOptimizations),
        m_untransformedCoordinates(untransformedCoordinates), m_currentColor(Qt::black) {
   }

   virtual void updateState(const QPaintEngineState &newState) {
      if (newState.state() & QPaintEngine::DirtyPen
            && newState.pen().color() != m_currentColor) {
         m_dirtyPen = true;
         m_currentColor = newState.pen().color();
      }
   }

   virtual void drawTextItem(const QPointF &position, const QTextItem &textItem) {
      int glyphOffset = m_inertText->glyphs.size(); // Store offset into glyph pool
      int positionOffset = m_inertText->glyphs.size(); // Offset into position pool
      int charOffset = m_inertText->chars.size();

      const QTextItemInt &ti = static_cast<const QTextItemInt &>(textItem);

      bool needFreshCurrentItem = true;
      if (!m_inertText->items.isEmpty()) {
         QStaticTextItem &last = m_inertText->items[m_inertText->items.count() - 1];

         if (last.fontEngine() == ti.fontEngine && last.font == ti.font() &&
               (!m_dirtyPen || last.color == state->pen().color())) {
            needFreshCurrentItem = false;

            last.numChars += ti.num_chars;

         }
      }

      if (needFreshCurrentItem) {
         QStaticTextItem currentItem;

         currentItem.setFontEngine(ti.fontEngine);
         currentItem.font = ti.font();
         currentItem.charOffset = charOffset;
         currentItem.numChars = ti.num_chars;
         currentItem.numGlyphs = 0;
         currentItem.glyphOffset = glyphOffset;
         currentItem.positionOffset = positionOffset;
         currentItem.useBackendOptimizations = m_useBackendOptimizations;
         if (m_dirtyPen) {
            currentItem.color = m_currentColor;
         }

         m_inertText->items.append(currentItem);
      }

      QStaticTextItem &currentItem = m_inertText->items.last();

      QTransform matrix = m_untransformedCoordinates ? QTransform() : state->transform();
      matrix.translate(position.x(), position.y());

      QVarLengthArray<glyph_t> glyphs;
      QVarLengthArray<QFixedPoint> positions;
      ti.fontEngine->getGlyphPositions(ti.glyphs, matrix, ti.flags, glyphs, positions);

      int size = glyphs.size();
      Q_ASSERT(size == positions.size());
      currentItem.numGlyphs += size;

      m_inertText->glyphs.resize(m_inertText->glyphs.size() + size);
      m_inertText->positions.resize(m_inertText->glyphs.size());
      m_inertText->chars.resize(m_inertText->chars.size() + ti.num_chars);

      glyph_t *glyphsDestination = m_inertText->glyphs.data() + glyphOffset;
      memcpy(glyphsDestination, glyphs.constData(), sizeof(glyph_t) * size);

      QFixedPoint *positionsDestination = m_inertText->positions.data() + positionOffset;
      memcpy(positionsDestination, positions.constData(), sizeof(QFixedPoint) * size);

      QChar *charsDestination = m_inertText->chars.data() + charOffset;
      memcpy(charsDestination, ti.chars, sizeof(QChar) * ti.num_chars);

   }

   virtual void drawPolygon(const QPointF *, int , PolygonDrawMode ) {
      /* intentionally empty */
   }

   virtual bool begin(QPaintDevice *)  {
      return true;
   }
   virtual bool end() {
      return true;
   }
   virtual void drawPixmap(const QRectF &, const QPixmap &, const QRectF &) {}
   virtual Type type() const {
      return User;
   }

   void begin(QDeclarativeTextLayoutPrivate *t) {
      m_inertText = t;
      m_dirtyPen = false;
   }

 private:
   QDeclarativeTextLayoutPrivate *m_inertText;

   bool m_dirtyPen;
   bool m_useBackendOptimizations;
   bool m_untransformedCoordinates;
   QColor m_currentColor;
};

class DrawTextItemDevice: public QPaintDevice
{
 public:
   DrawTextItemDevice(bool untransformedCoordinates, bool useBackendOptimizations) {
      m_paintEngine = new DrawTextItemRecorder(untransformedCoordinates,
            useBackendOptimizations);
   }

   ~DrawTextItemDevice() {
      delete m_paintEngine;
   }

   void begin(QDeclarativeTextLayoutPrivate *t) {
      m_paintEngine->begin(t);
   }

   int metric(PaintDeviceMetric m) const {
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
         default:
            val = 0;
            qWarning("DrawTextItemDevice::metric: Invalid metric command");
      }
      return val;
   }

   virtual QPaintEngine *paintEngine() const {
      return m_paintEngine;
   }

 private:
   DrawTextItemRecorder *m_paintEngine;
};

struct InertTextPainter {
   InertTextPainter()
      : device(true, true), painter(&device) {
      painter.setPen(QPen(QColor())); // explicitly invalid color.
   }

   DrawTextItemDevice device;
   QPainter painter;
};
}

Q_GLOBAL_STATIC(InertTextPainter, inertTextPainter);

/*!
\class QDeclarativeTextLayout
\brief The QDeclarativeTextLayout class is a version of QStaticText that works with QTextLayouts.
\internal

This class is basically a copy of the QStaticText code, but it is adapted to source its text from
QTextLayout.

It is also considerably faster to create a QDeclarativeTextLayout than a QStaticText because it uses
a single, shared QPainter instance.  QStaticText by comparison creates a new QPainter per instance.
As a consequence this means that QDeclarativeTextLayout is not re-enterant.  Adding a lock around
the shared painter solves this, and only introduces a minor performance penalty, but is unnecessary
for QDeclarativeTextLayout's current use (QDeclarativeText is already tied to the GUI thread).
*/

QDeclarativeTextLayout::QDeclarativeTextLayout()
   : d(0)
{
}

QDeclarativeTextLayout::QDeclarativeTextLayout(const QString &text)
   : QTextLayout(text), d(0)
{
}

QDeclarativeTextLayout::~QDeclarativeTextLayout()
{
   if (d) {
      delete d;
   }
}

void QDeclarativeTextLayout::beginLayout()
{
   if (d && d->cached) {
      d->cached = false;
      d->items.clear();
      d->positions.clear();
      d->glyphs.clear();
      d->chars.clear();
      d->position = QPointF();
   }
   QTextLayout::beginLayout();
}

void QDeclarativeTextLayout::clearLayout()
{
   if (d && d->cached) {
      d->cached = false;
      d->items.clear();
      d->positions.clear();
      d->glyphs.clear();
      d->chars.clear();
      d->position = QPointF();
   }
   QTextLayout::clearLayout();
}

void QDeclarativeTextLayout::prepare()
{
   if (!d || !d->cached) {

      if (!d) {
         d = new QDeclarativeTextLayoutPrivate;
      }

      InertTextPainter *itp = inertTextPainter();
      itp->device.begin(d);
      QTextLayout::draw(&itp->painter, QPointF(0, 0));

      glyph_t *glyphPool = d->glyphs.data();
      QFixedPoint *positionPool = d->positions.data();
      QChar *charPool = d->chars.data();

      int itemCount = d->items.count();
      for (int ii = 0; ii < itemCount; ++ii) {
         QStaticTextItem &item = d->items[ii];
         item.glyphs = glyphPool + item.glyphOffset;
         item.glyphPositions = positionPool + item.positionOffset;
         item.chars = charPool + item.charOffset;
      }

      d->cached = true;
   }
}

// Defined in qpainter.cpp
extern Q_GUI_EXPORT void qt_draw_decoration_for_glyphs(QPainter *painter, const glyph_t *glyphArray,
      const QFixedPoint *positions, int glyphCount,
      QFontEngine *fontEngine, const QFont &font,
      const QTextCharFormat &charFormat);

void QDeclarativeTextLayout::draw(QPainter *painter, const QPointF &p)
{
   QPainterPrivate *priv = QPainterPrivate::get(painter);

   bool paintEngineSupportsTransformations = priv->extended &&
         (priv->extended->type() == QPaintEngine::OpenGL2 ||
          priv->extended->type() == QPaintEngine::OpenVG ||
          priv->extended->type() == QPaintEngine::OpenGL);

   if (!paintEngineSupportsTransformations || !priv->state->matrix.isAffine()) {
      QTextLayout::draw(painter, p);
      return;
   }

   prepare();

   int itemCount = d->items.count();

   if (p != d->position) {
      QFixed fx = QFixed::fromReal(p.x());
      QFixed fy = QFixed::fromReal(p.y());
      QFixed oldX = QFixed::fromReal(d->position.x());
      QFixed oldY = QFixed::fromReal(d->position.y());
      for (int item = 0; item < itemCount; ++item) {
         QStaticTextItem &textItem = d->items[item];

         for (int ii = 0; ii < textItem.numGlyphs; ++ii) {
            textItem.glyphPositions[ii].x += fx - oldX;
            textItem.glyphPositions[ii].y += fy - oldY;
         }
         textItem.userDataNeedsUpdate = true;
      }

      d->position = p;
   }

   QPen oldPen = priv->state->pen;
   QColor currentColor = oldPen.color();
   QColor defaultColor = currentColor;
   for (int ii = 0; ii < itemCount; ++ii) {
      QStaticTextItem &item = d->items[ii];
      if (item.color.isValid() && currentColor != item.color) {
         // up-edge of a <font color="">text</font> tag
         // we set the painter pen to the text item's specified color.
         painter->setPen(item.color);
         currentColor = item.color;
      } else if (!item.color.isValid() && currentColor != defaultColor) {
         // down-edge of a <font color="">text</font> tag
         // we reset the painter pen back to the default color.
         currentColor = defaultColor;
         painter->setPen(currentColor);
      }
      priv->extended->drawStaticTextItem(&item);

      qt_draw_decoration_for_glyphs(painter, item.glyphs, item.glyphPositions,
                                    item.numGlyphs, item.fontEngine(), painter->font(),
                                    QTextCharFormat());
   }
   if (currentColor != oldPen.color()) {
      painter->setPen(oldPen);
   }
}

QT_END_NAMESPACE

