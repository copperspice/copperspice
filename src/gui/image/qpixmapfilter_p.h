/***********************************************************************
*
* Copyright (c) 2012-2014 Barbara Geller
* Copyright (c) 2012-2014 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QPIXMAPFILTER_P_H
#define QPIXMAPFILTER_P_H

#include <QtCore/qnamespace.h>
#include <QtGui/qpixmap.h>
#include <QtGui/qgraphicseffect.h>
#include <QScopedPointer>

#ifndef QT_NO_GRAPHICSEFFECT

QT_BEGIN_NAMESPACE

class QPainter;
class QPixmapData;
class QPixmapFilterPrivate;
class QPixmapConvolutionFilterPrivate;
class QPixmapBlurFilterPrivate;
class QPixmapColorizeFilterPrivate;
class QPixmapDropShadowFilterPrivate;

class Q_GUI_EXPORT QPixmapFilter : public QObject
{
   CS_OBJECT(QPixmapFilter)
   Q_DECLARE_PRIVATE(QPixmapFilter)

 public:
   virtual ~QPixmapFilter() = 0;

   enum FilterType {
      ConvolutionFilter,
      ColorizeFilter,
      DropShadowFilter,
      BlurFilter,

      UserFilter = 1024
   };

   FilterType type() const;

   virtual QRectF boundingRectFor(const QRectF &rect) const;

   virtual void draw(QPainter *painter, const QPointF &p, const QPixmap &src, const QRectF &srcRect = QRectF()) const = 0;

 protected:
   QPixmapFilter(QPixmapFilterPrivate &d, FilterType type, QObject *parent);
   QPixmapFilter(FilterType type, QObject *parent);

   QScopedPointer<QPixmapFilterPrivate> d_ptr;
};

class Q_GUI_EXPORT QPixmapConvolutionFilter : public QPixmapFilter
{
   CS_OBJECT(QPixmapConvolutionFilter)
   Q_DECLARE_PRIVATE(QPixmapConvolutionFilter)

 public:
   QPixmapConvolutionFilter(QObject *parent = 0);
   ~QPixmapConvolutionFilter();

   void setConvolutionKernel(const qreal *matrix, int rows, int columns);

   QRectF boundingRectFor(const QRectF &rect) const;
   void draw(QPainter *painter, const QPointF &dest, const QPixmap &src, const QRectF &srcRect = QRectF()) const;

 private:
   friend class QGLPixmapConvolutionFilter;
   friend class QVGPixmapConvolutionFilter;
   const qreal *convolutionKernel() const;
   int rows() const;
   int columns() const;
};

class Q_GUI_EXPORT QPixmapBlurFilter : public QPixmapFilter
{
   CS_OBJECT(QPixmapBlurFilter)
   Q_DECLARE_PRIVATE(QPixmapBlurFilter)

 public:
   QPixmapBlurFilter(QObject *parent = 0);
   ~QPixmapBlurFilter();

   void setRadius(qreal radius);
   void setBlurHints(QGraphicsBlurEffect::BlurHints hints);

   qreal radius() const;
   QGraphicsBlurEffect::BlurHints blurHints() const;

   QRectF boundingRectFor(const QRectF &rect) const;
   void draw(QPainter *painter, const QPointF &dest, const QPixmap &src, const QRectF &srcRect = QRectF()) const;

 private:
   friend class QGLPixmapBlurFilter;
};

class Q_GUI_EXPORT QPixmapColorizeFilter : public QPixmapFilter
{
   CS_OBJECT(QPixmapColorizeFilter)
   Q_DECLARE_PRIVATE(QPixmapColorizeFilter)

 public:
   QPixmapColorizeFilter(QObject *parent = 0);

   void setColor(const QColor &color);
   QColor color() const;

   void setStrength(qreal strength);
   qreal strength() const;

   void draw(QPainter *painter, const QPointF &dest, const QPixmap &src, const QRectF &srcRect = QRectF()) const;
};

class Q_GUI_EXPORT QPixmapDropShadowFilter : public QPixmapFilter
{
   CS_OBJECT(QPixmapDropShadowFilter)
   Q_DECLARE_PRIVATE(QPixmapDropShadowFilter)

 public:
   QPixmapDropShadowFilter(QObject *parent = 0);
   ~QPixmapDropShadowFilter();

   QRectF boundingRectFor(const QRectF &rect) const;
   void draw(QPainter *p, const QPointF &pos, const QPixmap &px, const QRectF &src = QRectF()) const;

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

QT_END_NAMESPACE

#endif //QT_NO_GRAPHICSEFFECT
#endif // QPIXMAPFILTER_H
