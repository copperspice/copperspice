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

#include <qsvgstyle_p.h>

#include <qcolor.h>
#include <qdebug.h>
#include <qmath.h>
#include <qnumeric.h>
#include <qpainter.h>
#include <qpair.h>

#include <qsvgfont_p.h>
#include <qsvggraphics_p.h>
#include <qsvgnode_p.h>
#include <qsvgtinydocument_p.h>

QSvgExtraStates::QSvgExtraStates()
   : fillOpacity(1.0), strokeOpacity(1.0), svgFont(nullptr), textAnchor(Qt::AlignLeft), fontWeight(400),
     fillRule(Qt::WindingFill), strokeDashOffset(0), vectorEffect(false)
{
}

QSvgStyleProperty::~QSvgStyleProperty()
{
}

void QSvgFillStyleProperty::apply(QPainter *, const QSvgNode *, QSvgExtraStates &)
{
   Q_ASSERT(! "This method should not be called");
}

void QSvgFillStyleProperty::revert(QPainter *, QSvgExtraStates &)
{
   Q_ASSERT(! "Revert() method should not be called");
}

QSvgQualityStyle::QSvgQualityStyle(int color)
{
   (void) color;
}

void QSvgQualityStyle::apply(QPainter *, const QSvgNode *, QSvgExtraStates &)
{
}

void QSvgQualityStyle::revert(QPainter *, QSvgExtraStates &)
{
}

QSvgFillStyle::QSvgFillStyle()
   : m_style(nullptr), m_fillRule(Qt::WindingFill), m_oldFillRule(Qt::WindingFill), m_fillOpacity(1.0),
     m_oldFillOpacity(0), m_gradientResolved(1), m_fillRuleSet(0), m_fillOpacitySet(0), m_fillSet(0)
{
}

void QSvgFillStyle::setFillRule(Qt::FillRule f)
{
   m_fillRuleSet = 1;
   m_fillRule = f;
}

void QSvgFillStyle::setFillOpacity(qreal opacity)
{
   m_fillOpacitySet = 1;
   m_fillOpacity = opacity;
}

void QSvgFillStyle::setFillStyle(QSvgFillStyleProperty *style)
{
   m_style = style;
   m_fillSet = 1;
}

void QSvgFillStyle::setBrush(QBrush brush)
{
   m_fill    = brush;
   m_style   = nullptr;
   m_fillSet = 1;
}

void QSvgFillStyle::apply(QPainter *p, const QSvgNode *, QSvgExtraStates &states)
{
   m_oldFill = p->brush();
   m_oldFillRule = states.fillRule;
   m_oldFillOpacity = states.fillOpacity;

   if (m_fillRuleSet) {
      states.fillRule = m_fillRule;
   }

   if (m_fillSet) {
      if (m_style) {
         p->setBrush(m_style->brush(p, states));
      } else {
         p->setBrush(m_fill);
      }
   }   if (m_fillOpacitySet) {
      states.fillOpacity = m_fillOpacity;
   }
}

void QSvgFillStyle::revert(QPainter *p, QSvgExtraStates &states)
{
   if (m_fillOpacitySet) {
      states.fillOpacity = m_oldFillOpacity;
   }
   if (m_fillSet) {
      p->setBrush(m_oldFill);
   }
   if (m_fillRuleSet) {
      states.fillRule = m_oldFillRule;
   }
}

QSvgViewportFillStyle::QSvgViewportFillStyle(const QBrush &brush)
   : m_viewportFill(brush)
{
}

void QSvgViewportFillStyle::apply(QPainter *p, const QSvgNode *, QSvgExtraStates &)
{
   m_oldFill = p->brush();
   p->setBrush(m_viewportFill);
}

void QSvgViewportFillStyle::revert(QPainter *p, QSvgExtraStates &)
{
   p->setBrush(m_oldFill);
}

QSvgFontStyle::QSvgFontStyle(QSvgFont *font, QSvgTinyDocument *doc)
   : m_svgFont(font), m_doc(doc), m_familySet(0), m_sizeSet(0), m_styleSet(0),
     m_variantSet(0), m_weightSet(0), m_textAnchorSet(0)
{
}

QSvgFontStyle::QSvgFontStyle()
   : m_svgFont(nullptr), m_doc(nullptr), m_familySet(0), m_sizeSet(0), m_styleSet(0),
     m_variantSet(0), m_weightSet(0), m_textAnchorSet(0)
{
}

int QSvgFontStyle::SVGToQtWeight(int weight)
{
   switch (weight) {
      case 100:
      case 200:
         return QFont::Light;

      case 300:
      case 400:
         return QFont::Normal;

      case 500:
      case 600:
         return QFont::DemiBold;

      case 700:
      case 800:
         return QFont::Bold;

      case 900:
         return QFont::Black;
   }

   return QFont::Normal;
}

void QSvgFontStyle::apply(QPainter *p, const QSvgNode *, QSvgExtraStates &states)
{
   m_oldQFont = p->font();
   m_oldSvgFont = states.svgFont;
   m_oldTextAnchor = states.textAnchor;
   m_oldWeight = states.fontWeight;

   if (m_textAnchorSet) {
      states.textAnchor = m_textAnchor;
   }

   QFont font = m_oldQFont;
   if (m_familySet) {
      states.svgFont = m_svgFont;
      font.setFamily(m_qfont.family());
   }

   if (m_sizeSet) {
      font.setPointSizeF(m_qfont.pointSizeF());
   }

   if (m_styleSet) {
      font.setStyle(m_qfont.style());
   }

   if (m_variantSet) {
      font.setCapitalization(m_qfont.capitalization());
   }

   if (m_weightSet) {
      if (m_weight == BOLDER) {
         states.fontWeight = qMin(states.fontWeight + 100, 900);
      } else if (m_weight == LIGHTER) {
         states.fontWeight = qMax(states.fontWeight - 100, 100);
      } else {
         states.fontWeight = m_weight;
      }
      font.setWeight(SVGToQtWeight(states.fontWeight));
   }

   p->setFont(font);
}

void QSvgFontStyle::revert(QPainter *p, QSvgExtraStates &states)
{
   p->setFont(m_oldQFont);
   states.svgFont = m_oldSvgFont;
   states.textAnchor = m_oldTextAnchor;
   states.fontWeight = m_oldWeight;
}

QSvgStrokeStyle::QSvgStrokeStyle()
   : m_strokeOpacity(1.0), m_oldStrokeOpacity(0.0), m_strokeDashOffset(0), m_oldStrokeDashOffset(0),
     m_style(nullptr), m_gradientResolved(1), m_vectorEffect(0), m_oldVectorEffect(0), m_strokeSet(0),
     m_strokeDashArraySet(0), m_strokeDashOffsetSet(0), m_strokeLineCapSet(0), m_strokeLineJoinSet(0),
     m_strokeMiterLimitSet(0), m_strokeOpacitySet(0), m_strokeWidthSet(0), m_vectorEffectSet(0)
{
}

void QSvgStrokeStyle::apply(QPainter *p, const QSvgNode *, QSvgExtraStates &states)
{
   m_oldStroke = p->pen();
   m_oldStrokeOpacity = states.strokeOpacity;
   m_oldStrokeDashOffset = states.strokeDashOffset;
   m_oldVectorEffect = states.vectorEffect;

   QPen pen = p->pen();

   qreal oldWidth = pen.widthF();
   qreal width = m_stroke.widthF();
   if (oldWidth == 0) {
      oldWidth = 1;
   }
   if (width == 0) {
      width = 1;
   }
   qreal scale = oldWidth / width;

   if (m_strokeOpacitySet) {
      states.strokeOpacity = m_strokeOpacity;
   }

   if (m_vectorEffectSet) {
      states.vectorEffect = m_vectorEffect;
   }

   if (m_strokeSet) {
      if (m_style) {
         pen.setBrush(m_style->brush(p, states));
      } else {
         pen.setBrush(m_stroke.brush());
      }
   }

   if (m_strokeWidthSet) {
      pen.setWidthF(m_stroke.widthF());
   }

   bool setDashOffsetNeeded = false;

   if (m_strokeDashOffsetSet) {
      states.strokeDashOffset = m_strokeDashOffset;
      setDashOffsetNeeded = true;
   }

   if (m_strokeDashArraySet) {
      if (m_stroke.style() == Qt::SolidLine) {
         pen.setStyle(Qt::SolidLine);
      } else if (m_strokeWidthSet || oldWidth == 1) {
         // If both width and dash array was set, the dash array is already scaled correctly.
         pen.setDashPattern(m_stroke.dashPattern());
         setDashOffsetNeeded = true;
      } else {
         // If dash array was set, but not the width, the dash array has to be scaled with respect to the old width.
         QVector<qreal> dashes = m_stroke.dashPattern();
         for (int i = 0; i < dashes.size(); ++i) {
            dashes[i] /= oldWidth;
         }
         pen.setDashPattern(dashes);
         setDashOffsetNeeded = true;
      }

   } else if (m_strokeWidthSet && pen.style() != Qt::SolidLine && scale != 1) {
      // If the width was set, but not the dash array, the old dash array must be scaled with respect to the new width.
      QVector<qreal> dashes = pen.dashPattern();
      for (int i = 0; i < dashes.size(); ++i) {
         dashes[i] *= scale;
      }
      pen.setDashPattern(dashes);
      setDashOffsetNeeded = true;
   }

   if (m_strokeLineCapSet) {
      pen.setCapStyle(m_stroke.capStyle());
   }
   if (m_strokeLineJoinSet) {
      pen.setJoinStyle(m_stroke.joinStyle());
   }
   if (m_strokeMiterLimitSet) {
      pen.setMiterLimit(m_stroke.miterLimit());
   }

   // You can have dash offset on solid strokes in SVG files, but not in Qt.
   // QPen::setDashOffset() will set the pen style to Qt::CustomDashLine,
   // so don't call the method if the pen is solid.
   if (setDashOffsetNeeded && pen.style() != Qt::SolidLine) {
      qreal currentWidth = pen.widthF();
      if (currentWidth == 0) {
         currentWidth = 1;
      }
      pen.setDashOffset(states.strokeDashOffset / currentWidth);
   }

   pen.setCosmetic(states.vectorEffect);

   p->setPen(pen);
}

void QSvgStrokeStyle::revert(QPainter *p, QSvgExtraStates &states)
{
   p->setPen(m_oldStroke);
   states.strokeOpacity = m_oldStrokeOpacity;
   states.strokeDashOffset = m_oldStrokeDashOffset;
   states.vectorEffect = m_oldVectorEffect;
}

void QSvgStrokeStyle::setDashArray(const QVector<qreal> &dashes)
{
   if (m_strokeWidthSet) {
      QVector<qreal> d = dashes;
      qreal w = m_stroke.widthF();
      if (w != 0 && w != 1) {
         for (int i = 0; i < d.size(); ++i) {
            d[i] /= w;
         }
      }
      m_stroke.setDashPattern(d);

   } else {
      m_stroke.setDashPattern(dashes);
   }

   m_strokeDashArraySet = 1;
}

QSvgSolidColorStyle::QSvgSolidColorStyle(const QColor &color)
   : m_solidColor(color)
{
}

QSvgGradientStyle::QSvgGradientStyle(QGradient *grad)
   : m_gradient(grad), m_gradientStopsSet(false)
{
}

QBrush QSvgGradientStyle::brush(QPainter *, QSvgExtraStates &)
{
   if (! m_link.isEmpty()) {
      resolveStops();
   }

   // If the gradient is marked as empty, insert transparent black
   if (!m_gradientStopsSet) {
      m_gradient->setStops(QVector<QPair<qreal, QColor>>() << QPair<qreal, QColor>(0.0, QColor(0, 0, 0, 0)));
      m_gradientStopsSet = true;
   }

   QBrush b(*m_gradient);

   if (!m_matrix.isIdentity()) {
      b.setMatrix(m_matrix);
   }

   return b;
}

void QSvgGradientStyle::setMatrix(const QMatrix &mat)
{
   m_matrix = mat;
}

QSvgTransformStyle::QSvgTransformStyle(const QTransform &trans)
   : m_transform(trans)
{
}

void QSvgTransformStyle::apply(QPainter *p, const QSvgNode *, QSvgExtraStates &)
{
   m_oldWorldTransform = p->worldTransform();
   p->setWorldTransform(m_transform, true);
}

void QSvgTransformStyle::revert(QPainter *p, QSvgExtraStates &)
{
   p->setWorldTransform(m_oldWorldTransform, false /* don't combine */);
}

QSvgStyleProperty::Type QSvgQualityStyle::type() const
{
   return QUALITY;
}

QSvgStyleProperty::Type QSvgFillStyle::type() const
{
   return FILL;
}

QSvgStyleProperty::Type QSvgViewportFillStyle::type() const
{
   return VIEWPORT_FILL;
}

QSvgStyleProperty::Type QSvgFontStyle::type() const
{
   return FONT;
}

QSvgStyleProperty::Type QSvgStrokeStyle::type() const
{
   return STROKE;
}

QSvgStyleProperty::Type QSvgSolidColorStyle::type() const
{
   return SOLID_COLOR;
}

QSvgStyleProperty::Type QSvgGradientStyle::type() const
{
   return GRADIENT;
}

QSvgStyleProperty::Type QSvgTransformStyle::type() const
{
   return TRANSFORM;
}

QSvgCompOpStyle::QSvgCompOpStyle(QPainter::CompositionMode mode)
   : m_mode(mode)
{

}

void QSvgCompOpStyle::apply(QPainter *p, const QSvgNode *, QSvgExtraStates &)
{
   m_oldMode = p->compositionMode();
   p->setCompositionMode(m_mode);
}

void QSvgCompOpStyle::revert(QPainter *p, QSvgExtraStates &)
{
   p->setCompositionMode(m_oldMode);
}

QSvgStyleProperty::Type QSvgCompOpStyle::type() const
{
   return COMP_OP;
}

QSvgStyle::~QSvgStyle()
{
}

void QSvgStyle::apply(QPainter *p, const QSvgNode *node, QSvgExtraStates &states)
{
   if (quality) {
      quality->apply(p, node, states);
   }

   if (fill) {
      fill->apply(p, node, states);
   }

   if (viewportFill) {
      viewportFill->apply(p, node, states);
   }

   if (font) {
      font->apply(p, node, states);
   }

   if (stroke) {
      stroke->apply(p, node, states);
   }

   if (transform) {
      transform->apply(p, node, states);
   }

   if (animateColor) {
      animateColor->apply(p, node, states);
   }

   //animated transforms have to be applied
   //_after_ the original object transformations
   if (! animateTransforms.isEmpty()) {
      qreal totalTimeElapsed = node->document()->currentElapsed();
      // Find the last animateTransform with additive="replace", since this will override all
      // previous animateTransforms.

      QList<QSvgRefCounter<QSvgAnimateTransform> >::const_iterator itr = animateTransforms.constEnd();
      do {
         --itr;

         if ((*itr)->animActive(totalTimeElapsed)
               && (*itr)->additiveType() == QSvgAnimateTransform::Replace) {
            // An animateTransform with additive="replace" will replace the transform attribute.
            if (transform) {
               transform->revert(p, states);
            }
            break;
         }
      } while (itr != animateTransforms.constBegin());

      // Apply the animateTransforms after and including the last one with additive="replace".
      for (; itr != animateTransforms.constEnd(); ++itr) {
         if ((*itr)->animActive(totalTimeElapsed)) {
            (*itr)->apply(p, node, states);
         }
      }
   }

   if (opacity) {
      opacity->apply(p, node, states);
   }

   if (compop) {
      compop->apply(p, node, states);
   }
}

void QSvgStyle::revert(QPainter *p, QSvgExtraStates &states)
{
   if (quality) {
      quality->revert(p, states);
   }

   if (fill) {
      fill->revert(p, states);
   }

   if (viewportFill) {
      viewportFill->revert(p, states);
   }

   if (font) {
      font->revert(p, states);
   }

   if (stroke) {
      stroke->revert(p, states);
   }

   //animated transforms need to be reverted _before_
   //the native transforms
   if (!animateTransforms.isEmpty()) {
      QList<QSvgRefCounter<QSvgAnimateTransform> >::const_iterator itr = animateTransforms.constBegin();
      for (; itr != animateTransforms.constEnd(); ++itr) {
         if ((*itr)->transformApplied()) {
            (*itr)->revert(p, states);
            break;
         }
      }
      for (; itr != animateTransforms.constEnd(); ++itr) {
         (*itr)->clearTransformApplied();
      }
   }

   if (transform) {
      transform->revert(p, states);
   }

   if (animateColor) {
      animateColor->revert(p, states);
   }

   if (opacity) {
      opacity->revert(p, states);
   }

   if (compop) {
      compop->revert(p, states);
   }
}

QSvgAnimateTransform::QSvgAnimateTransform(int startMs, int endMs, int byMs)
   : QSvgStyleProperty(), m_from(startMs),m_totalRunningTime(endMs - startMs), m_type(Empty),
      m_additive(Replace), m_count(0), m_finished(false), m_freeze(false),
      m_repeatCount(-1.), m_transformApplied(false)
{
   (void) byMs;
}

void QSvgAnimateTransform::setArgs(TransformType type, Additive additive, const QVector<qreal> &args)
{
   m_type = type;
   m_args = args;
   m_additive = additive;
   Q_ASSERT(!(args.count() % 3));
   m_count = args.count() / 3;
}

void QSvgAnimateTransform::apply(QPainter *p, const QSvgNode *node, QSvgExtraStates &)
{
   m_oldWorldTransform = p->worldTransform();
   resolveMatrix(node);
   p->setWorldTransform(m_transform, true);
   m_transformApplied = true;
}

void QSvgAnimateTransform::revert(QPainter *p, QSvgExtraStates &)
{
   p->setWorldTransform(m_oldWorldTransform, false /* don't combine */);
   m_transformApplied = false;
}

void QSvgAnimateTransform::resolveMatrix(const QSvgNode *node)
{
   static const qreal deg2rad = qreal(0.017453292519943295769);
   qreal totalTimeElapsed = node->document()->currentElapsed();
   if (totalTimeElapsed < m_from || m_finished) {
      return;
   }

   qreal animationFrame = 0;
   if (m_totalRunningTime != 0) {
      animationFrame = (totalTimeElapsed - m_from) / m_totalRunningTime;

      if (m_repeatCount >= 0 && m_repeatCount < animationFrame) {
         m_finished = true;
         animationFrame = m_repeatCount;
      }
   }

   qreal percentOfAnimation = animationFrame;
   if (percentOfAnimation > 1) {
      percentOfAnimation -= ((int)percentOfAnimation);
   }

   qreal currentPosition = percentOfAnimation * (m_count - 1);
   int startElem = qFloor(currentPosition);
   int endElem   = qCeil(currentPosition);

   switch (m_type) {
      case Translate: {
         startElem *= 3;
         endElem   *= 3;
         qreal from1, from2;
         qreal to1, to2;
         from1 = m_args[startElem++];
         from2 = m_args[startElem++];
         to1   = m_args[endElem++];
         to2   = m_args[endElem++];

         qreal transXDiff = (to1 - from1) * percentOfAnimation;
         qreal transX = from1 + transXDiff;
         qreal transYDiff = (to2 - from2) * percentOfAnimation;
         qreal transY = from2 + transYDiff;
         m_transform = QTransform();
         m_transform.translate(transX, transY);
         break;
      }
      case Scale: {
         startElem *= 3;
         endElem   *= 3;
         qreal from1, from2;
         qreal to1, to2;
         from1 = m_args[startElem++];
         from2 = m_args[startElem++];
         to1   = m_args[endElem++];
         to2   = m_args[endElem++];

         qreal transXDiff = (to1 - from1) * percentOfAnimation;
         qreal transX = from1 + transXDiff;
         qreal transYDiff = (to2 - from2) * percentOfAnimation;
         qreal transY = from2 + transYDiff;
         if (transY == 0) {
            transY = transX;
         }
         m_transform = QTransform();
         m_transform.scale(transX, transY);
         break;
      }
      case Rotate: {
         startElem *= 3;
         endElem   *= 3;
         qreal from1, from2, from3;
         qreal to1, to2, to3;
         from1 = m_args[startElem++];
         from2 = m_args[startElem++];
         from3 = m_args[startElem++];
         to1   = m_args[endElem++];
         to2   = m_args[endElem++];
         to3   = m_args[endElem++];

         qreal rotationDiff = (to1 - from1) * percentOfAnimation;
         //qreal rotation = from1 + rotationDiff;

         qreal transXDiff = (to2 - from2) * percentOfAnimation;
         qreal transX = from2 + transXDiff;
         qreal transYDiff = (to3 - from3) * percentOfAnimation;
         qreal transY = from3 + transYDiff;
         m_transform = QTransform();
         m_transform.translate(transX, transY);
         m_transform.rotate(rotationDiff);
         m_transform.translate(-transX, -transY);
         break;
      }
      case SkewX: {
         startElem *= 3;
         endElem   *= 3;
         qreal from1;
         qreal to1;
         from1 = m_args[startElem++];
         to1   = m_args[endElem++];

         qreal transXDiff = (to1 - from1) * percentOfAnimation;
         qreal transX = from1 + transXDiff;
         m_transform = QTransform();
         m_transform.shear(qTan(transX * deg2rad), 0);
         break;
      }
      case SkewY: {
         startElem *= 3;
         endElem   *= 3;
         qreal from1;
         qreal to1;
         from1 = m_args[startElem++];
         to1   = m_args[endElem++];


         qreal transYDiff = (to1 - from1) * percentOfAnimation;
         qreal transY = from1 + transYDiff;
         m_transform = QTransform();
         m_transform.shear(0, qTan(transY * deg2rad));
         break;
      }
      default:
         break;
   }
}

QSvgStyleProperty::Type QSvgAnimateTransform::type() const
{
   return ANIMATE_TRANSFORM;
}

void QSvgAnimateTransform::setFreeze(bool freeze)
{
   m_freeze = freeze;
}

void QSvgAnimateTransform::setRepeatCount(qreal repeatCount)
{
   m_repeatCount = repeatCount;
}

QSvgAnimateColor::QSvgAnimateColor(int startMs, int endMs, int byMs)
   : QSvgStyleProperty(), m_from(startMs), m_totalRunningTime(endMs - startMs),
      m_fill(false), m_finished(false), m_freeze(false), m_repeatCount(-1.)
{
   (void) byMs;
}

void QSvgAnimateColor::setArgs(bool fill, const QList<QColor> &colors)
{
   m_fill   = fill;
   m_colors = colors;
}

void QSvgAnimateColor::setFreeze(bool freeze)
{
   m_freeze = freeze;
}

void QSvgAnimateColor::setRepeatCount(qreal repeatCount)
{
   m_repeatCount = repeatCount;
}

void QSvgAnimateColor::apply(QPainter *p, const QSvgNode *node, QSvgExtraStates &)
{
   qreal totalTimeElapsed = node->document()->currentElapsed();
   if (totalTimeElapsed < m_from || m_finished) {
      return;
   }

   qreal animationFrame = 0;
   if (m_totalRunningTime != 0) {
      animationFrame = (totalTimeElapsed - m_from) / m_totalRunningTime;
   }

   if (m_repeatCount >= 0 && m_repeatCount < animationFrame) {
      m_finished = true;
      animationFrame = m_repeatCount;
   }

   qreal percentOfAnimation = animationFrame;
   if (percentOfAnimation > 1) {
      percentOfAnimation -= ((int)percentOfAnimation);
   }

   qreal currentPosition = percentOfAnimation * (m_colors.count() - 1);

   int startElem = qFloor(currentPosition);
   int endElem   = qCeil(currentPosition);
   QColor start = m_colors[startElem];
   QColor end = m_colors[endElem];

   qreal percentOfColorMorph = currentPosition;
   if (percentOfColorMorph > 1) {
      percentOfColorMorph -= ((int)percentOfColorMorph);
   }

   // Interpolate between the two fixed colors start and end
   qreal aDiff = (end.alpha() - start.alpha()) * percentOfColorMorph;
   qreal rDiff = (end.red()   - start.red()) * percentOfColorMorph;
   qreal gDiff = (end.green() - start.green()) * percentOfColorMorph;
   qreal bDiff = (end.blue()  - start.blue()) * percentOfColorMorph;

   int alpha  = int(start.alpha() + aDiff);
   int red    = int(start.red() + rDiff);
   int green  = int(start.green() + gDiff);
   int blue   = int(start.blue() + bDiff);

   QColor color(red, green, blue, alpha);

   if (m_fill) {
      QBrush b = p->brush();
      m_oldBrush = b;
      b.setColor(color);
      p->setBrush(b);
   } else {
      QPen pen = p->pen();
      m_oldPen = pen;
      pen.setColor(color);
      p->setPen(pen);
   }
}

void QSvgAnimateColor::revert(QPainter *p, QSvgExtraStates &)
{
   if (m_fill) {
      p->setBrush(m_oldBrush);
   } else {
      p->setPen(m_oldPen);
   }
}

QSvgStyleProperty::Type QSvgAnimateColor::type() const
{
   return ANIMATE_COLOR;
}

QSvgOpacityStyle::QSvgOpacityStyle(qreal opacity)
   : m_opacity(opacity), m_oldOpacity(0)
{

}

void QSvgOpacityStyle::apply(QPainter *p, const QSvgNode *, QSvgExtraStates &)
{
   m_oldOpacity = p->opacity();
   p->setOpacity(m_opacity * m_oldOpacity);
}

void QSvgOpacityStyle::revert(QPainter *p, QSvgExtraStates &)
{
   p->setOpacity(m_oldOpacity);
}

QSvgStyleProperty::Type QSvgOpacityStyle::type() const
{
   return OPACITY;
}

void QSvgGradientStyle::setStopLink(const QString &link, QSvgTinyDocument *doc)
{
   m_link = link;
   m_doc  = doc;
}

void QSvgGradientStyle::resolveStops()
{
   if (!m_link.isEmpty() && m_doc) {
      QSvgStyleProperty *prop = m_doc->styleProperty(m_link);

      if (prop && prop != this) {
         if (prop->type() == QSvgStyleProperty::GRADIENT) {
            QSvgGradientStyle *st =
               static_cast<QSvgGradientStyle *>(prop);
            st->resolveStops();
            m_gradient->setStops(st->qgradient()->stops());
            m_gradientStopsSet = st->gradientStopsSet();
         }
      } else {
         qWarning("Could not resolve property : %s", csPrintable(m_link));
      }
      m_link = QString();
   }
}


