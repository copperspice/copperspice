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

#ifndef QDECLARATIVEFOCUSPANEL_P_H
#define QDECLARATIVEFOCUSPANEL_P_H

#include <qdeclarativeitem.h>

QT_BEGIN_NAMESPACE

class QDeclarativeFocusPanel : public QDeclarativeItem
{
   DECL_CS_OBJECT(QDeclarativeFocusPanel)

   DECL_CS_PROPERTY_READ(active, isActive)
   DECL_CS_PROPERTY_WRITE(active, setActive)
   DECL_CS_PROPERTY_NOTIFY(active, activeChanged)

 public:
   QDeclarativeFocusPanel(QDeclarativeItem *parent = 0);
   virtual ~QDeclarativeFocusPanel();

   DECL_CS_SIGNAL_1(Public, void activeChanged())
   DECL_CS_SIGNAL_2(activeChanged)

 protected:
   bool sceneEvent(QEvent *event);

 private:
   Q_DISABLE_COPY(QDeclarativeFocusPanel)
   Q_DECLARE_PRIVATE_D(QGraphicsItem::d_ptr.data(), QDeclarativeItem)
};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QDeclarativeFocusPanel)

#endif // QDECLARATIVEFOCUSPANEL_H
