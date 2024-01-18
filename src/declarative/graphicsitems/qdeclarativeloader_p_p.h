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

#ifndef QDECLARATIVELOADER_P_P_H
#define QDECLARATIVELOADER_P_P_H

#include <qdeclarativeloader_p.h>
#include <qdeclarativeimplicitsizeitem_p_p.h>
#include <qdeclarativeitemchangelistener_p.h>

QT_BEGIN_NAMESPACE

class QDeclarativeContext;
class QDeclarativeLoaderPrivate : public QDeclarativeImplicitSizeItemPrivate, public QDeclarativeItemChangeListener
{
   Q_DECLARE_PUBLIC(QDeclarativeLoader)

 public:
   QDeclarativeLoaderPrivate();
   ~QDeclarativeLoaderPrivate();

   void itemGeometryChanged(QDeclarativeItem *item, const QRectF &newGeometry, const QRectF &oldGeometry);
   void clear();
   void initResize();
   void load();

   QUrl source;
   QGraphicsObject *item;
   QDeclarativeComponent *component;
   bool ownComponent : 1;
   bool updatingSize: 1;
   bool itemWidthValid : 1;
   bool itemHeightValid : 1;

   void _q_sourceLoaded();
   void _q_updateSize(bool loaderGeometryChanged = true);
};

QT_END_NAMESPACE

#endif // QDECLARATIVELOADER_P_H
