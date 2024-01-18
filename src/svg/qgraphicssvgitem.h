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

#ifndef QGRAPHICSSVGITEM_H
#define QGRAPHICSSVGITEM_H

#include <qglobal.h>

#if ! defined(QT_NO_GRAPHICSVIEW) && ! defined(QT_NO_SVGWIDGET)

#include <qgraphicsitem.h>

class QSvgRenderer;
class QGraphicsSvgItemPrivate;

class Q_SVG_EXPORT QGraphicsSvgItem : public QGraphicsObject
{
   SVG_CS_OBJECT(QGraphicsSvgItem)
   CS_INTERFACES(QGraphicsItem)

   SVG_CS_PROPERTY_READ(elementId, elementId)
   SVG_CS_PROPERTY_WRITE(elementId, setElementId)

   SVG_CS_PROPERTY_READ(maximumCacheSize, maximumCacheSize)
   SVG_CS_PROPERTY_WRITE(maximumCacheSize, setMaximumCacheSize)

 public:
   QGraphicsSvgItem(QGraphicsItem *parent = nullptr);
   QGraphicsSvgItem(const QString &fileName, QGraphicsItem *parent = nullptr);

   QGraphicsSvgItem(const QGraphicsSvgItem &) = delete;
   QGraphicsSvgItem &operator=(const QGraphicsSvgItem &) = delete;

   void setSharedRenderer(QSvgRenderer *renderer);
   QSvgRenderer *renderer() const;

   void setElementId(const QString &id);
   QString elementId() const;

   void setCachingEnabled(bool caching);
   bool isCachingEnabled() const;

   void setMaximumCacheSize(const QSize &size);
   QSize maximumCacheSize() const;

   QRectF boundingRect() const override;
   void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget = nullptr) override;

   int type() const override;

   static constexpr int Type = 13;

 private:
   Q_DECLARE_PRIVATE_D(QGraphicsItem::d_ptr.data(), QGraphicsSvgItem)

   SVG_CS_SLOT_1(Private, void _q_repaintItem())
   SVG_CS_SLOT_2(_q_repaintItem)
};

#endif // QT_NO_GRAPHICSVIEW or QT_NO_SVGWIDGETS

#endif
