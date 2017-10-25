/***********************************************************************
*
* Copyright (c) 2012-2017 Barbara Geller
* Copyright (c) 2012-2017 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
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
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QGRAPHICSITEMANIMATION_H
#define QGRAPHICSITEMANIMATION_H

#include <QtCore/qobject.h>
#include <qcontainerfwd.h>

#if ! defined(QT_NO_GRAPHICSVIEW)

QT_BEGIN_NAMESPACE

class QGraphicsItem;
class QMatrix;
class QPointF;
class QTimeLine;
class QGraphicsItemAnimationPrivate;

class Q_GUI_EXPORT QGraphicsItemAnimation : public QObject
{
   GUI_CS_OBJECT(QGraphicsItemAnimation)

 public:
   QGraphicsItemAnimation(QObject *parent = nullptr);
   virtual ~QGraphicsItemAnimation();

   QGraphicsItem *item() const;
   void setItem(QGraphicsItem *item);

   QTimeLine *timeLine() const;
   void setTimeLine(QTimeLine *timeLine);

   QPointF posAt(qreal step) const;
   QList<QPair<qreal, QPointF> > posList() const;
   void setPosAt(qreal step, const QPointF &pos);

   QMatrix matrixAt(qreal step) const;

   qreal rotationAt(qreal step) const;
   QList<QPair<qreal, qreal> > rotationList() const;
   void setRotationAt(qreal step, qreal angle);

   qreal xTranslationAt(qreal step) const;
   qreal yTranslationAt(qreal step) const;
   QList<QPair<qreal, QPointF> > translationList() const;
   void setTranslationAt(qreal step, qreal dx, qreal dy);

   qreal verticalScaleAt(qreal step) const;
   qreal horizontalScaleAt(qreal step) const;
   QList<QPair<qreal, QPointF> > scaleList() const;
   void setScaleAt(qreal step, qreal sx, qreal sy);

   qreal verticalShearAt(qreal step) const;
   qreal horizontalShearAt(qreal step) const;
   QList<QPair<qreal, QPointF> > shearList() const;
   void setShearAt(qreal step, qreal sh, qreal sv);

   void clear();

   GUI_CS_SLOT_1(Public, void setStep(qreal x))
   GUI_CS_SLOT_2(setStep)
   GUI_CS_SLOT_1(Public, void reset())
   GUI_CS_SLOT_2(reset)

 protected:
   virtual void beforeAnimationStep(qreal step);
   virtual void afterAnimationStep(qreal step);

 private:
   Q_DISABLE_COPY(QGraphicsItemAnimation)
   QGraphicsItemAnimationPrivate *d;
};

QT_END_NAMESPACE

#endif // QT_NO_GRAPHICSVIEW
#endif
