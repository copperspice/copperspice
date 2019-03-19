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

#include "qsvgnode_p.h"
#include "qsvgtinydocument_p.h"

#ifndef QT_NO_SVG

#include "qdebug.h"
#include "qstack.h"

QT_BEGIN_NAMESPACE

QSvgNode::QSvgNode(QSvgNode *parent)
   : m_parent(parent),
     m_visible(true),
     m_displayMode(BlockMode)
{
}

QSvgNode::~QSvgNode()
{

}

void QSvgNode::appendStyleProperty(QSvgStyleProperty *prop, const QString &id)
{
   //qDebug()<<"appending "<<prop->type()<< " ("<< id <<") "<<"to "<<this<<this->type();
   QSvgTinyDocument *doc;
   switch (prop->type()) {
      case QSvgStyleProperty::QUALITY:
         m_style.quality = static_cast<QSvgQualityStyle *>(prop);
         break;
      case QSvgStyleProperty::FILL:
         m_style.fill = static_cast<QSvgFillStyle *>(prop);
         break;
      case QSvgStyleProperty::VIEWPORT_FILL:
         m_style.viewportFill = static_cast<QSvgViewportFillStyle *>(prop);
         break;
      case QSvgStyleProperty::FONT:
         m_style.font = static_cast<QSvgFontStyle *>(prop);
         break;
      case QSvgStyleProperty::STROKE:
         m_style.stroke = static_cast<QSvgStrokeStyle *>(prop);
         break;
      case QSvgStyleProperty::SOLID_COLOR:
         m_style.solidColor = static_cast<QSvgSolidColorStyle *>(prop);
         doc = document();
         if (doc && !id.isEmpty()) {
            doc->addNamedStyle(id, m_style.solidColor);
         }
         break;
      case QSvgStyleProperty::GRADIENT:
         m_style.gradient = static_cast<QSvgGradientStyle *>(prop);
         doc = document();
         if (doc && !id.isEmpty()) {
            doc->addNamedStyle(id, m_style.gradient);
         }
         break;
      case QSvgStyleProperty::TRANSFORM:
         m_style.transform = static_cast<QSvgTransformStyle *>(prop);
         break;
      case QSvgStyleProperty::ANIMATE_COLOR:
         m_style.animateColor = static_cast<QSvgAnimateColor *>(prop);
         break;
      case QSvgStyleProperty::ANIMATE_TRANSFORM:
         m_style.animateTransforms.append(
            static_cast<QSvgAnimateTransform *>(prop));
         break;
      case QSvgStyleProperty::OPACITY:
         m_style.opacity = static_cast<QSvgOpacityStyle *>(prop);
         break;
      case QSvgStyleProperty::COMP_OP:
         m_style.compop = static_cast<QSvgCompOpStyle *>(prop);
         break;
      default:
         qDebug("QSvgNode: Trying to append unknown property!");
         break;
   }
}

void QSvgNode::applyStyle(QPainter *p, QSvgExtraStates &states) const
{
   m_style.apply(p, this, states);
}

void QSvgNode::revertStyle(QPainter *p, QSvgExtraStates &states) const
{
   m_style.revert(p, states);
}

QSvgStyleProperty *QSvgNode::styleProperty(QSvgStyleProperty::Type type) const
{
   const QSvgNode *node = this;
   while (node) {
      switch (type) {
         case QSvgStyleProperty::QUALITY:
            if (node->m_style.quality) {
               return node->m_style.quality;
            }
            break;
         case QSvgStyleProperty::FILL:
            if (node->m_style.fill) {
               return node->m_style.fill;
            }
            break;
         case QSvgStyleProperty::VIEWPORT_FILL:
            if (m_style.viewportFill) {
               return node->m_style.viewportFill;
            }
            break;
         case QSvgStyleProperty::FONT:
            if (node->m_style.font) {
               return node->m_style.font;
            }
            break;
         case QSvgStyleProperty::STROKE:
            if (node->m_style.stroke) {
               return node->m_style.stroke;
            }
            break;
         case QSvgStyleProperty::SOLID_COLOR:
            if (node->m_style.solidColor) {
               return node->m_style.solidColor;
            }
            break;
         case QSvgStyleProperty::GRADIENT:
            if (node->m_style.gradient) {
               return node->m_style.gradient;
            }
            break;
         case QSvgStyleProperty::TRANSFORM:
            if (node->m_style.transform) {
               return node->m_style.transform;
            }
            break;
         case QSvgStyleProperty::ANIMATE_COLOR:
            if (node->m_style.animateColor) {
               return node->m_style.animateColor;
            }
            break;
         case QSvgStyleProperty::ANIMATE_TRANSFORM:
            if (!node->m_style.animateTransforms.isEmpty()) {
               return node->m_style.animateTransforms.first();
            }
            break;
         case QSvgStyleProperty::OPACITY:
            if (node->m_style.opacity) {
               return node->m_style.opacity;
            }
            break;
         case QSvgStyleProperty::COMP_OP:
            if (node->m_style.compop) {
               return node->m_style.compop;
            }
            break;
         default:
            break;
      }
      node = node->parent();
   }

   return 0;
}

QSvgFillStyleProperty *QSvgNode::styleProperty(const QString &id) const
{
   QString rid = id;
   if (rid.startsWith(QLatin1Char('#'))) {
      rid.remove(0, 1);
   }
   QSvgTinyDocument *doc = document();
   return doc ? doc->namedStyle(rid) : 0;
}

QRectF QSvgNode::bounds(QPainter *, QSvgExtraStates &) const
{
   return QRectF(0, 0, 0, 0);
}

QRectF QSvgNode::transformedBounds() const
{
   if (!m_cachedBounds.isEmpty()) {
      return m_cachedBounds;
   }

   QImage dummy(1, 1, QImage::Format_RGB32);
   QPainter p(&dummy);
   QSvgExtraStates states;

   QPen pen(Qt::NoBrush, 1, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin);
   pen.setMiterLimit(4);
   p.setPen(pen);

   QStack<QSvgNode *> parentApplyStack;
   QSvgNode *parent = m_parent;
   while (parent) {
      parentApplyStack.push(parent);
      parent = parent->parent();
   }

   for (int i = parentApplyStack.size() - 1; i >= 0; --i) {
      parentApplyStack[i]->applyStyle(&p, states);
   }

   p.setWorldTransform(QTransform());

   m_cachedBounds = transformedBounds(&p, states);
   return m_cachedBounds;
}

QSvgTinyDocument *QSvgNode::document() const
{
   QSvgTinyDocument *doc = 0;
   QSvgNode *node = const_cast<QSvgNode *>(this);
   while (node && node->type() != QSvgNode::DOC) {
      node = node->parent();
   }
   doc = static_cast<QSvgTinyDocument *>(node);

   return doc;
}

void QSvgNode::setRequiredFeatures(const QStringList &lst)
{
   m_requiredFeatures = lst;
}

const QStringList &QSvgNode::requiredFeatures() const
{
   return m_requiredFeatures;
}

void QSvgNode::setRequiredExtensions(const QStringList &lst)
{
   m_requiredExtensions = lst;
}

const QStringList &QSvgNode::requiredExtensions() const
{
   return m_requiredExtensions;
}

void QSvgNode::setRequiredLanguages(const QStringList &lst)
{
   m_requiredLanguages = lst;
}

const QStringList &QSvgNode::requiredLanguages() const
{
   return m_requiredLanguages;
}

void QSvgNode::setRequiredFormats(const QStringList &lst)
{
   m_requiredFormats = lst;
}

const QStringList &QSvgNode::requiredFormats() const
{
   return m_requiredFormats;
}

void QSvgNode::setRequiredFonts(const QStringList &lst)
{
   m_requiredFonts = lst;
}

const QStringList &QSvgNode::requiredFonts() const
{
   return m_requiredFonts;
}

void QSvgNode::setVisible(bool visible)
{
   //propagate visibility change of true to the parent
   //not propagating false is just a small performance
   //degradation since we'll iterate over children without
   //drawing any of them
   if (m_parent && visible && !m_parent->isVisible()) {
      m_parent->setVisible(true);
   }

   m_visible = visible;
}

QRectF QSvgNode::transformedBounds(QPainter *p, QSvgExtraStates &states) const
{
   applyStyle(p, states);
   QRectF rect = bounds(p, states);
   revertStyle(p, states);
   return rect;
}

void QSvgNode::setNodeId(const QString &i)
{
   m_id = i;
}

void QSvgNode::setXmlClass(const QString &str)
{
   m_class = str;
}

void QSvgNode::setDisplayMode(DisplayMode mode)
{
   m_displayMode = mode;
}

QSvgNode::DisplayMode QSvgNode::displayMode() const
{
   return m_displayMode;
}

qreal QSvgNode::strokeWidth(QPainter *p)
{
   QPen pen = p->pen();
   if (pen.style() == Qt::NoPen || pen.brush().style() == Qt::NoBrush || pen.isCosmetic()) {
      return 0;
   }
   return pen.widthF();
}

QT_END_NAMESPACE

#endif // QT_NO_SVG
