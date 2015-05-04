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

#ifndef QDECLARATIVEFOCUSPANEL_P_H
#define QDECLARATIVEFOCUSPANEL_P_H

#include <qdeclarativeitem.h>

QT_BEGIN_NAMESPACE

class QDeclarativeFocusPanel : public QDeclarativeItem
{
   CS_OBJECT(QDeclarativeFocusPanel)

   CS_PROPERTY_READ(active, isActive)
   CS_PROPERTY_WRITE(active, setActive)
   CS_PROPERTY_NOTIFY(active, activeChanged)

 public:
   QDeclarativeFocusPanel(QDeclarativeItem *parent = 0);
   virtual ~QDeclarativeFocusPanel();

   CS_SIGNAL_1(Public, void activeChanged())
   CS_SIGNAL_2(activeChanged)

 protected:
   bool sceneEvent(QEvent *event);

 private:
   Q_DISABLE_COPY(QDeclarativeFocusPanel)
   Q_DECLARE_PRIVATE_D(QGraphicsItem::d_ptr.data(), QDeclarativeItem)
};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QDeclarativeFocusPanel)

#endif // QDECLARATIVEFOCUSPANEL_H
