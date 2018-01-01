/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
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

#ifndef QGRAPHICSTRANSFORM_P_H
#define QGRAPHICSTRANSFORM_P_H

#ifndef QT_NO_GRAPHICSVIEW
QT_BEGIN_NAMESPACE

class QGraphicsItem;

class QGraphicsTransformPrivate
{

 public:
   Q_DECLARE_PUBLIC(QGraphicsTransform)

   QGraphicsTransformPrivate() : item(0) {}
   virtual ~QGraphicsTransformPrivate() {}

   QGraphicsItem *item;

   void setItem(QGraphicsItem *item);
   static void updateItem(QGraphicsItem *item);

 protected:
   QGraphicsTransform *q_ptr;

};

QT_END_NAMESPACE
#endif //QT_NO_GRAPHCISVIEW

#endif // QGRAPHICSTRANSFORM_P_H
