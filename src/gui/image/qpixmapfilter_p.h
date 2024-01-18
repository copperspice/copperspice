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

#ifndef QPIXMAPFILTER_H
#define QPIXMAPFILTER_H

#include <qnamespace.h>
#include <qpixmap.h>
#include <qgraphicseffect.h>

#ifndef QT_NO_GRAPHICSEFFECT

class QPainter;
class QPlatformPixmap;
class QPixmapFilterPrivate;

class Q_GUI_EXPORT QPixmapFilter : public QObject
{
   GUI_CS_OBJECT(QPixmapFilter)
   Q_DECLARE_PRIVATE(QPixmapFilter)

 public:
   enum FilterType {
      ConvolutionFilter,
      ColorizeFilter,
      DropShadowFilter,
      BlurFilter,

      UserFilter = 1024
   };

   virtual ~QPixmapFilter() = 0;

   FilterType type() const;

   virtual QRectF boundingRectFor(const QRectF &rect) const;

   virtual void draw(QPainter *painter, const QPointF &p, const QPixmap &src, const QRectF &srcRect = QRectF()) const = 0;

 protected:
   QPixmapFilter(QPixmapFilterPrivate &d, FilterType type, QObject *parent);
   QPixmapFilter(FilterType type, QObject *parent);

   QScopedPointer<QPixmapFilterPrivate> d_ptr;
};

class QPixmapConvolutionFilterPrivate;

class Q_GUI_EXPORT QPixmapConvolutionFilter : public QPixmapFilter
{
   GUI_CS_OBJECT(QPixmapConvolutionFilter)
   Q_DECLARE_PRIVATE(QPixmapConvolutionFilter)

 public:
   QPixmapConvolutionFilter(QObject *parent = nullptr);
   ~QPixmapConvolutionFilter();

   void setConvolutionKernel(const qreal *matrix, int rows, int columns);

   QRectF boundingRectFor(const QRectF &rect) const override;
   void draw(QPainter *painter, const QPointF &dest, const QPixmap &src, const QRectF &srcRect = QRectF()) const override;

 private:
   friend class QGLPixmapConvolutionFilter;
   friend class QVGPixmapConvolutionFilter;
   const qreal *convolutionKernel() const;
   int rows() const;
   int columns() const;
};

class QPixmapBlurFilterPrivate;

class Q_GUI_EXPORT QPixmapBlurFilter : public QPixmapFilter
{
   GUI_CS_OBJECT(QPixmapBlurFilter)
   Q_DECLARE_PRIVATE(QPixmapBlurFilter)

 public:
   QPixmapBlurFilter(QObject *parent = nullptr);
   ~QPixmapBlurFilter();

   void setRadius(qreal radius);
   void setBlurHints(QGraphicsBlurEffect::BlurHints hints);

   qreal radius() const;
   QGraphicsBlurEffect::BlurHints blurHints() const;

   QRectF boundingRectFor(const QRectF &rect) const override;
   void draw(QPainter *painter, const QPointF &dest, const QPixmap &src, const QRectF &srcRect = QRectF()) const override;

 private:
   friend class QGLPixmapBlurFilter;
};

class QPixmapColorizeFilterPrivate;

class Q_GUI_EXPORT QPixmapColorizeFilter : public QPixmapFilter
{
   GUI_CS_OBJECT(QPixmapColorizeFilter)
   Q_DECLARE_PRIVATE(QPixmapColorizeFilter)

 public:
   QPixmapColorizeFilter(QObject *parent = nullptr);

   void setColor(const QColor &color);
   QColor color() const;

   void setStrength(qreal strength);
   qreal strength() const;

   void draw(QPainter *painter, const QPointF &dest, const QPixmap &src, const QRectF &srcRect = QRectF()) const override;
};

class QPixmapDropShadowFilterPrivate;

class Q_GUI_EXPORT QPixmapDropShadowFilter : public QPixmapFilter
{
   GUI_CS_OBJECT(QPixmapDropShadowFilter)
   Q_DECLARE_PRIVATE(QPixmapDropShadowFilter)

 public:
   QPixmapDropShadowFilter(QObject *parent = nullptr);
   ~QPixmapDropShadowFilter();

   QRectF boundingRectFor(const QRectF &rect) const override;
   void draw(QPainter *p, const QPointF &pos, const QPixmap &px, const QRectF &src = QRectF()) const override;

   qreal blurRadius() const;
   void setBlurRadius(qreal radius);

   QColor color() const;
   void setColor(const QColor &color);

   QPointF offset() const;
   void setOffset(const QPointF &offset);
   inline void setOffset(qreal dx, qreal dy) {
      setOffset(QPointF(dx, dy));
   }
};

#endif //QT_NO_GRAPHICSEFFECT

#endif
