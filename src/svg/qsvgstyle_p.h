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

#ifndef QSVGSTYLE_P_H
#define QSVGSTYLE_P_H

#include <qpainter.h>

#ifndef QT_NO_SVG

#include "QtGui/qpen.h"
#include "QtGui/qbrush.h"
#include "QtGui/qmatrix.h"
#include "QtGui/qcolor.h"
#include "QtGui/qfont.h"
#include <qdebug.h>

QT_BEGIN_NAMESPACE

class QPainter;
class QSvgNode;
class QSvgFont;
class QSvgTinyDocument;

template <class T> class QSvgRefCounter
{
 public:
   QSvgRefCounter() {
      t = 0;
   }
   QSvgRefCounter(T *_t) {
      t = _t;
      if (t) {
         t->ref();
      }
   }
   QSvgRefCounter(const QSvgRefCounter &other) {
      t = other.t;
      if (t) {
         t->ref();
      }
   }
   QSvgRefCounter &operator =(T *_t) {
      if (_t) {
         _t->ref();
      }
      if (t) {
         t->deref();
      }
      t = _t;
      return *this;
   }
   QSvgRefCounter &operator =(const QSvgRefCounter &other) {
      if (other.t) {
         other.t->ref();
      }
      if (t) {
         t->deref();
      }
      t = other.t;
      return *this;
   }
   ~QSvgRefCounter() {
      if (t) {
         t->deref();
      }
   }

   inline T *operator->() const {
      return t;
   }
   inline operator T *() const {
      return t;
   }

 private:
   T *t;
};

class QSvgRefCounted
{
 public:
   QSvgRefCounted() {
      _ref = 0;
   }
   virtual ~QSvgRefCounted() {}
   void ref() {
      ++_ref;
      //        qDebug() << this << ": adding ref, now " << _ref;
   }
   void deref() {
      //        qDebug() << this << ": removing ref, now " << _ref;
      if (!--_ref) {
         //            qDebug("     deleting");
         delete this;
      }
   }
 private:
   int _ref;
};

struct QSvgExtraStates {
   QSvgExtraStates();

   qreal fillOpacity;
   qreal strokeOpacity;
   QSvgFont *svgFont;
   Qt::Alignment textAnchor;
   int fontWeight;
   Qt::FillRule fillRule;
   qreal strokeDashOffset;
   bool vectorEffect; // true if pen is cosmetic
};

class QSvgStyleProperty : public QSvgRefCounted
{
 public:
   enum Type {
      QUALITY,
      FILL,
      VIEWPORT_FILL,
      FONT,
      STROKE,
      SOLID_COLOR,
      GRADIENT,
      TRANSFORM,
      ANIMATE_TRANSFORM,
      ANIMATE_COLOR,
      OPACITY,
      COMP_OP
   };
 public:
   virtual ~QSvgStyleProperty();
   virtual void apply(QPainter *p, const QSvgNode *node, QSvgExtraStates &states) = 0;
   virtual void revert(QPainter *p, QSvgExtraStates &states) = 0;
   virtual Type type() const = 0;
};

class QSvgFillStyleProperty : public QSvgStyleProperty
{
 public:
   virtual QBrush brush(QPainter *p, QSvgExtraStates &states) = 0;
   void apply(QPainter *p, const QSvgNode *node, QSvgExtraStates &states) override;
   void revert(QPainter *p, QSvgExtraStates &states) override;
};

class QSvgQualityStyle : public QSvgStyleProperty
{
 public:
   QSvgQualityStyle(int color);
   void apply(QPainter *p, const QSvgNode *node, QSvgExtraStates &states) override;
   void revert(QPainter *p, QSvgExtraStates &states) override;
   Type type() const override;

 private:
   // color-render ing v 	v 	'auto' | 'optimizeSpeed' |
   //                                  'optimizeQuality' | 'inherit'
   int m_colorRendering;

   // shape-rendering v 	v 	'auto' | 'optimizeSpeed' | 'crispEdges' |
   //                                  'geometricPrecision' | 'inherit'
   //QSvgShapeRendering m_shapeRendering;


   // text-rendering    v 	v 	'auto' | 'optimizeSpeed' | 'optimizeLegibility'
   //                                | 'geometricPrecision' | 'inherit'
   //QSvgTextRendering m_textRendering;


   // vector-effect         v 	x 	'default' | 'non-scaling-stroke' | 'inherit'
   //QSvgVectorEffect m_vectorEffect;

   // image-rendering v 	v 	'auto' | 'optimizeSpeed' | 'optimizeQuality' |
   //                                      'inherit'
   //QSvgImageRendering m_imageRendering;
};



class QSvgOpacityStyle : public QSvgStyleProperty
{
 public:
   QSvgOpacityStyle(qreal opacity);

   void apply(QPainter *p, const QSvgNode *node, QSvgExtraStates &states) override;
   void revert(QPainter *p, QSvgExtraStates &states) override;
   Type type() const override;

 private:
   qreal m_opacity;
   qreal m_oldOpacity;
};

class QSvgFillStyle : public QSvgStyleProperty
{
 public:
   QSvgFillStyle();

   void apply(QPainter *p, const QSvgNode *node, QSvgExtraStates &states) override;
   void revert(QPainter *p, QSvgExtraStates &states) override;
   Type type() const override;

   void setFillRule(Qt::FillRule f);
   void setFillOpacity(qreal opacity);
   void setFillStyle(QSvgFillStyleProperty *style);
   void setBrush(QBrush brush);

   const QBrush &qbrush() const {
      return m_fill;
   }

   qreal fillOpacity() const {
      return m_fillOpacity;
   }

   Qt::FillRule fillRule() const {
      return m_fillRule;
   }

   QSvgFillStyleProperty *style() const {
      return m_style;
   }

   void setGradientId(const QString &Id) {
      m_gradientId = Id;
   }

   QString gradientId() const {
      return m_gradientId;
   }

   void setGradientResolved(bool resolved) {
      m_gradientResolved = resolved;
   }

   bool isGradientResolved() const {
      return m_gradientResolved;
   }

 private:
   // fill            v 	v 	'inherit' | <Paint.datatype>
   // fill-opacity    v 	v 	'inherit' | <OpacityValue.datatype>
   QBrush m_fill;
   QBrush m_oldFill;
   QSvgFillStyleProperty *m_style;

   Qt::FillRule m_fillRule;
   Qt::FillRule m_oldFillRule;
   qreal m_fillOpacity;
   qreal m_oldFillOpacity;

   QString m_gradientId;
   uint m_gradientResolved : 1;

   uint m_fillRuleSet : 1;
   uint m_fillOpacitySet : 1;
   uint m_fillSet : 1;
};

class QSvgViewportFillStyle : public QSvgStyleProperty
{
 public:
   QSvgViewportFillStyle(const QBrush &brush);

   void apply(QPainter *p, const QSvgNode *node, QSvgExtraStates &states) override;
   void revert(QPainter *p, QSvgExtraStates &states) override;
   Type type() const override;

   const QBrush &qbrush() const {
      return m_viewportFill;
   }

 private:
   // viewport-fill         v 	x 	'inherit' | <Paint.datatype>
   // viewport-fill-opacity 	v 	x 	'inherit' | <OpacityValue.datatype>
   QBrush m_viewportFill;

   QBrush m_oldFill;
};

class QSvgFontStyle : public QSvgStyleProperty
{
 public:
   static const int LIGHTER = -1;
   static const int BOLDER = 1;

   QSvgFontStyle(QSvgFont *font, QSvgTinyDocument *doc);
   QSvgFontStyle();

   void apply(QPainter *p, const QSvgNode *node, QSvgExtraStates &states) override;
   void revert(QPainter *p, QSvgExtraStates &states) override;
   Type type() const override;

   void setSize(qreal size) {
      // Store the _pixel_ size in the font. Since QFont::setPixelSize() only takes an int, call
      // QFont::SetPointSize() instead. Set proper font size just before rendering.
      m_qfont.setPointSizeF(size);
      m_sizeSet = 1;
   }

   void setTextAnchor(Qt::Alignment anchor) {
      m_textAnchor = anchor;
      m_textAnchorSet = 1;
   }

   void setFamily(const QString &family) {
      m_qfont.setFamily(family);
      m_familySet = 1;
   }

   void setStyle(QFont::Style fontStyle) {
      m_qfont.setStyle(fontStyle);
      m_styleSet = 1;
   }

   void setVariant(QFont::Capitalization fontVariant) {
      m_qfont.setCapitalization(fontVariant);
      m_variantSet = 1;
   }

   static int SVGToQtWeight(int weight);

   void setWeight(int weight) {
      m_weight = weight;
      m_weightSet = 1;
   }

   QSvgFont *svgFont() const {
      return m_svgFont;
   }

   const QFont &qfont() const {
      return m_qfont;
   }

   QSvgTinyDocument *doc() const {
      return m_doc;
   }

 private:
   QSvgFont *m_svgFont;
   QSvgTinyDocument *m_doc;
   QFont m_qfont;

   int m_weight;
   Qt::Alignment m_textAnchor;

   QSvgFont *m_oldSvgFont;
   QFont m_oldQFont;
   Qt::Alignment m_oldTextAnchor;
   int m_oldWeight;

   uint m_familySet : 1;
   uint m_sizeSet : 1;
   uint m_styleSet : 1;
   uint m_variantSet : 1;
   uint m_weightSet : 1;
   uint m_textAnchorSet : 1;
};

class QSvgStrokeStyle : public QSvgStyleProperty
{
 public:
   QSvgStrokeStyle();

   void apply(QPainter *p, const QSvgNode *node, QSvgExtraStates &states) override;
   void revert(QPainter *p, QSvgExtraStates &states) override;
   Type type() const override;

   void setStroke(QBrush brush) {
      m_stroke.setBrush(brush);
      m_style = 0;
      m_strokeSet = 1;
   }

   void setStyle(QSvgFillStyleProperty *style) {
      m_style = style;
      m_strokeSet = 1;
   }

   void setDashArray(const QVector<qreal> &dashes);

   void setDashArrayNone() {
      m_stroke.setStyle(Qt::SolidLine);
      m_strokeDashArraySet = 1;
   }

   void setDashOffset(qreal offset) {
      m_strokeDashOffset = offset;
      m_strokeDashOffsetSet = 1;
   }

   void setLineCap(Qt::PenCapStyle cap) {
      m_stroke.setCapStyle(cap);
      m_strokeLineCapSet = 1;
   }

   void setLineJoin(Qt::PenJoinStyle join) {
      m_stroke.setJoinStyle(join);
      m_strokeLineJoinSet = 1;
   }

   void setMiterLimit(qreal limit) {
      m_stroke.setMiterLimit(limit);
      m_strokeMiterLimitSet = 1;
   }

   void setOpacity(qreal opacity) {
      m_strokeOpacity = opacity;
      m_strokeOpacitySet = 1;
   }

   void setWidth(qreal width) {
      m_stroke.setWidthF(width);
      m_strokeWidthSet = 1;
      Q_ASSERT(!m_strokeDashArraySet); // set width before dash array.
   }

   qreal width() {
      return m_stroke.widthF();
   }

   void setVectorEffect(bool nonScalingStroke) {
      m_vectorEffect = nonScalingStroke;
      m_vectorEffectSet = 1;
   }

   QSvgFillStyleProperty *style() const {
      return m_style;
   }

   void setGradientId(const QString &Id) {
      m_gradientId = Id;
   }

   QString gradientId() const {
      return m_gradientId;
   }

   void setGradientResolved(bool resolved) {
      m_gradientResolved = resolved;
   }

   bool isGradientResolved() const {
      return m_gradientResolved;
   }

   QPen stroke() const {
      return m_stroke;
   }

 private:
   // stroke            v 	v 	'inherit' | <Paint.datatype>
   // stroke-dasharray  v 	v 	'inherit' | <StrokeDashArrayValue.datatype>
   // stroke-dashoffset v 	v 	'inherit' | <StrokeDashOffsetValue.datatype>
   // stroke-linecap    v 	v 	'butt' | 'round' | 'square' | 'inherit'
   // stroke-linejoin   v 	v 	'miter' | 'round' | 'bevel' | 'inherit'
   // stroke-miterlimit v 	v 	'inherit' | <StrokeMiterLimitValue.datatype>
   // stroke-opacity    v 	v 	'inherit' | <OpacityValue.datatype>
   // stroke-width      v 	v 	'inherit' | <StrokeWidthValue.datatype>
   QPen m_stroke;
   QPen m_oldStroke;
   qreal m_strokeOpacity;
   qreal m_oldStrokeOpacity;
   qreal m_strokeDashOffset;
   qreal m_oldStrokeDashOffset;

   QSvgFillStyleProperty *m_style;
   QString m_gradientId;
   uint m_gradientResolved : 1;
   uint m_vectorEffect : 1;
   uint m_oldVectorEffect : 1;

   uint m_strokeSet : 1;
   uint m_strokeDashArraySet : 1;
   uint m_strokeDashOffsetSet : 1;
   uint m_strokeLineCapSet : 1;
   uint m_strokeLineJoinSet : 1;
   uint m_strokeMiterLimitSet : 1;
   uint m_strokeOpacitySet : 1;
   uint m_strokeWidthSet : 1;
   uint m_vectorEffectSet : 1;
};

class QSvgSolidColorStyle : public QSvgFillStyleProperty
{
 public:
   QSvgSolidColorStyle(const QColor &color);

   Type type() const override;

   const QColor &qcolor() const {
      return m_solidColor;
   }

   QBrush brush(QPainter *, QSvgExtraStates &) override {
      return m_solidColor;
   }

 private:
   // solid-color       v 	x 	'inherit' | <SVGColor.datatype>
   // solid-opacity     v 	x 	'inherit' | <OpacityValue.datatype>
   QColor m_solidColor;

   QBrush m_oldFill;
   QPen   m_oldStroke;
};

class QSvgGradientStyle : public QSvgFillStyleProperty
{
 public:
   QSvgGradientStyle(QGradient *grad);
   ~QSvgGradientStyle() {
      delete m_gradient;
   }

   Type type() const override;

   void setStopLink(const QString &link, QSvgTinyDocument *doc);
   QString stopLink() const {
      return m_link;
   }
   void resolveStops();

   void setMatrix(const QMatrix &matrix);
   QMatrix  qmatrix() const {
      return m_matrix;
   }

   QGradient *qgradient() const {
      return m_gradient;
   }

   bool gradientStopsSet() const {
      return m_gradientStopsSet;
   }

   void setGradientStopsSet(bool set) {
      m_gradientStopsSet = set;
   }

   QBrush brush(QPainter *, QSvgExtraStates &) override;

 private:
   QGradient        *m_gradient;
   QMatrix           m_matrix;

   QSvgTinyDocument *m_doc;
   QString           m_link;
   bool              m_gradientStopsSet;
};

class QSvgTransformStyle : public QSvgStyleProperty
{
 public:
   QSvgTransformStyle(const QTransform &transform);

   void apply(QPainter *p, const QSvgNode *node, QSvgExtraStates &states) override;
   void revert(QPainter *p, QSvgExtraStates &states) override;
   Type type() const override;

   const QTransform &qtransform() const {
      return m_transform;
   }

 private:
   //7.6 The transform  attribute
   QTransform m_transform;
   QTransform m_oldWorldTransform;
};


class QSvgAnimateTransform : public QSvgStyleProperty
{
 public:
   enum TransformType {
      Empty,
      Translate,
      Scale,
      Rotate,
      SkewX,
      SkewY
   };
   enum Additive {
      Sum,
      Replace
   };
 public:
   QSvgAnimateTransform(int startMs, int endMs, int by = 0);

   void setArgs(TransformType type, Additive additive, const QVector<qreal> &args);
   void setFreeze(bool freeze);
   void setRepeatCount(qreal repeatCount);

   void apply(QPainter *p, const QSvgNode *node, QSvgExtraStates &states) override;
   void revert(QPainter *p, QSvgExtraStates &states) override;
   Type type() const override;

   QSvgAnimateTransform::Additive additiveType() const {
      return m_additive;
   }

   bool animActive(qreal totalTimeElapsed) {
      if (totalTimeElapsed < m_from) {
         return false;
      }
      if (m_freeze || m_repeatCount < 0) { // fill="freeze" or repeat="indefinite"
         return true;
      }
      if (m_totalRunningTime == 0) {
         return false;
      }
      qreal animationFrame = (totalTimeElapsed - m_from) / m_totalRunningTime;
      if (animationFrame > m_repeatCount) {
         return false;
      }
      return true;
   }

   bool transformApplied() const {
      return m_transformApplied;
   }

   // Call this instead of revert if you know that revert is unnecessary.
   void clearTransformApplied() {
      m_transformApplied = false;
   }

 protected:
   void resolveMatrix(const QSvgNode *node);

 private:
   qreal m_from, m_to, m_by;
   qreal m_totalRunningTime;
   TransformType m_type;
   Additive m_additive;
   QVector<qreal> m_args;
   int m_count;
   QTransform m_transform;
   QTransform m_oldWorldTransform;
   bool m_finished;
   bool m_freeze;
   qreal m_repeatCount;
   bool m_transformApplied;
};


class QSvgAnimateColor : public QSvgStyleProperty
{
 public:
   QSvgAnimateColor(int startMs, int endMs, int by = 0);

   void setArgs(bool fill, const QList<QColor> &colors);
   void setFreeze(bool freeze);
   void setRepeatCount(qreal repeatCount);

   void apply(QPainter *p, const QSvgNode *node, QSvgExtraStates &states) override;
   void revert(QPainter *p, QSvgExtraStates &states) override;
   Type type() const override;

 private:
   qreal m_from;
   qreal m_to;
   qreal m_by;
   qreal m_totalRunningTime;

   QList<QColor> m_colors;
   QBrush m_oldBrush;
   QPen   m_oldPen;
   bool m_fill;
   bool m_finished;
   bool m_freeze;
   qreal m_repeatCount;
};


class QSvgCompOpStyle : public QSvgStyleProperty
{
 public:
   QSvgCompOpStyle(QPainter::CompositionMode mode);

   void apply(QPainter *p, const QSvgNode *node, QSvgExtraStates &states) override;
   void revert(QPainter *p, QSvgExtraStates &states) override;
   Type type() const override;

   const QPainter::CompositionMode &compOp() const {
      return m_mode;
   }

 private:
   //comp-op attribute
   QPainter::CompositionMode m_mode;

   QPainter::CompositionMode m_oldMode;
};


class QSvgStyle
{
 public:
   QSvgStyle()
      : quality(0), fill(0), viewportFill(0), font(0), stroke(0), solidColor(0), gradient(0), transform(0),
        animateColor(0), opacity(0), compop(0) {
   }
   ~QSvgStyle();

   void apply(QPainter *p, const QSvgNode *node, QSvgExtraStates &states);
   void revert(QPainter *p, QSvgExtraStates &states);

   QSvgRefCounter<QSvgQualityStyle>      quality;
   QSvgRefCounter<QSvgFillStyle>         fill;
   QSvgRefCounter<QSvgViewportFillStyle> viewportFill;
   QSvgRefCounter<QSvgFontStyle>         font;
   QSvgRefCounter<QSvgStrokeStyle>       stroke;
   QSvgRefCounter<QSvgSolidColorStyle>   solidColor;
   QSvgRefCounter<QSvgGradientStyle>     gradient;
   QSvgRefCounter<QSvgTransformStyle>    transform;
   QSvgRefCounter<QSvgAnimateColor>      animateColor;
   QList<QSvgRefCounter<QSvgAnimateTransform> >   animateTransforms;
   QSvgRefCounter<QSvgOpacityStyle>      opacity;
   QSvgRefCounter<QSvgCompOpStyle>       compop;
};

/********************************************************/
// NOT implemented:

// color           v 	v 	'inherit' | <Color.datatype>
//QColor m_color;

// display         v 	x 	'inline' | 'block' | 'list-item'
//                                 | 'run-in' | 'compact' | 'marker' |
//                                 'table' | 'inline-table' |
//                                 'table-row-group' | 'table-header-group' |
//                                 'table-footer-group' | 'table-row' |
//                                 'table-column-group' | 'table-column' |
//                                 'table-cell' | 'table-caption' |
//                                 'none' | 'inherit'
//QSvgDisplayStyle m_display;

// display-align   v 	v 	'auto' | 'before' | 'center' | 'after' | 'inherit'
//QSvgDisplayAlign m_displayAlign;

// line-increment  v 	v 	'auto' | 'inherit' | <Number.datatype>
//int m_lineIncrement;

// text-anchor       v 	v 	'start' | 'middle' | 'end' | 'inherit'
//QSvgTextAnchor m_textAnchor;

// visibility 	v 	v 	'visible' | 'hidden' | 'inherit'
//QSvgVisibility m_visibility;

/******************************************************/
// the following do not make sense for us

// pointer-events  v 	v 	'visiblePainted' | 'visibleFill' | 'visibleStroke' |
//                              'visible' | 'painted' | 'fill' | 'stroke' | 'all' |
//                              'none' | 'inherit'
//QSvgPointEvents m_pointerEvents;

// audio-level     v  	x  	'inherit' | <Number.datatype>

QT_END_NAMESPACE

#endif // QT_NO_SVG
#endif // QSVGSTYLE_P_H
