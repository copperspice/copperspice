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

#ifndef QDECLARATIVEGRAPHICSWIDGET_P_H
#define QDECLARATIVEGRAPHICSWIDGET_P_H

#include <QObject>
#include <QtDeclarative/qdeclarativecomponent.h>

QT_BEGIN_NAMESPACE

class QDeclarativeAnchorLine;
class QDeclarativeAnchors;
class QGraphicsObject;
class QDeclarativeGraphicsWidgetPrivate;

// ### TODO can the extension object be the anchor directly? We save one allocation -> awesome.
class QDeclarativeGraphicsWidget : public QObject
{
   DECL_CS_OBJECT(QDeclarativeGraphicsWidget)

   DECL_CS_PROPERTY_READ(anchors, anchors)
   DECL_CS_PROPERTY_DESIGNABLE(anchors, false)
   DECL_CS_PROPERTY_CONSTANT(anchors)
   DECL_CS_PROPERTY_FINAL(anchors)

   DECL_CS_PROPERTY_READ(left, left)
   DECL_CS_PROPERTY_CONSTANT(left)
   DECL_CS_PROPERTY_FINAL(left)

   DECL_CS_PROPERTY_READ(right, right)
   DECL_CS_PROPERTY_CONSTANT(right)
   DECL_CS_PROPERTY_FINAL(right)

   DECL_CS_PROPERTY_READ(horizontalCenter, horizontalCenter)
   DECL_CS_PROPERTY_CONSTANT(horizontalCenter)
   DECL_CS_PROPERTY_FINAL(horizontalCenter)

   DECL_CS_PROPERTY_READ(top, top)
   DECL_CS_PROPERTY_CONSTANT(top)
   DECL_CS_PROPERTY_FINAL(top)

   DECL_CS_PROPERTY_READ(bottom, bottom)
   DECL_CS_PROPERTY_CONSTANT(bottom)
   DECL_CS_PROPERTY_FINAL(bottom)

   DECL_CS_PROPERTY_READ(verticalCenter, verticalCenter)
   DECL_CS_PROPERTY_CONSTANT(verticalCenter)
   DECL_CS_PROPERTY_FINAL(verticalCenter)

   // ### TODO : QGraphicsWidget don't have a baseline concept yet.
   //Q_PROPERTY(QDeclarativeAnchorLine baseline READ baseline CONSTANT FINAL)

 public:
   QDeclarativeGraphicsWidget(QObject *parent = nullptr);
   ~QDeclarativeGraphicsWidget();
   QDeclarativeAnchors *anchors();
   QDeclarativeAnchorLine left() const;
   QDeclarativeAnchorLine right() const;
   QDeclarativeAnchorLine horizontalCenter() const;
   QDeclarativeAnchorLine top() const;
   QDeclarativeAnchorLine bottom() const;
   QDeclarativeAnchorLine verticalCenter() const;
   Q_DISABLE_COPY(QDeclarativeGraphicsWidget)
   Q_DECLARE_PRIVATE(QDeclarativeGraphicsWidget)
};

QT_END_NAMESPACE

#endif // QDECLARATIVEGRAPHICSWIDGET_P_H
