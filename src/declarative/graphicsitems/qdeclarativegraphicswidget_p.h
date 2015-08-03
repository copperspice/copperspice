/***********************************************************************
*
* Copyright (c) 2012-2015 Barbara Geller
* Copyright (c) 2012-2015 Ansel Sermersheim
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

   CS_PROPERTY_READ(anchors, anchors)
   CS_PROPERTY_DESIGNABLE(anchors, false)
   CS_PROPERTY_CONSTANT(anchors)
   CS_PROPERTY_FINAL(anchors)

   CS_PROPERTY_READ(left, left)
   CS_PROPERTY_CONSTANT(left)
   CS_PROPERTY_FINAL(left)

   CS_PROPERTY_READ(right, right)
   CS_PROPERTY_CONSTANT(right)
   CS_PROPERTY_FINAL(right)

   CS_PROPERTY_READ(horizontalCenter, horizontalCenter)
   CS_PROPERTY_CONSTANT(horizontalCenter)
   CS_PROPERTY_FINAL(horizontalCenter)

   CS_PROPERTY_READ(top, top)
   CS_PROPERTY_CONSTANT(top)
   CS_PROPERTY_FINAL(top)

   CS_PROPERTY_READ(bottom, bottom)
   CS_PROPERTY_CONSTANT(bottom)
   CS_PROPERTY_FINAL(bottom)

   CS_PROPERTY_READ(verticalCenter, verticalCenter)
   CS_PROPERTY_CONSTANT(verticalCenter)
   CS_PROPERTY_FINAL(verticalCenter)

   // ### TODO : QGraphicsWidget don't have a baseline concept yet.
   //Q_PROPERTY(QDeclarativeAnchorLine baseline READ baseline CONSTANT FINAL)

 public:
   QDeclarativeGraphicsWidget(QObject *parent = 0);
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
