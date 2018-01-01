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

#ifndef QSCROLLAREA_P_H
#define QSCROLLAREA_P_H

#ifndef QT_NO_SCROLLAREA

#include <qabstractscrollarea_p.h>
#include <QtGui/qscrollbar.h>

QT_BEGIN_NAMESPACE

class QScrollAreaPrivate: public QAbstractScrollAreaPrivate
{
   Q_DECLARE_PUBLIC(QScrollArea)

 public:
   QScrollAreaPrivate(): resizable(false), alignment(0) {}
   void updateScrollBars();
   void updateWidgetPosition();
   QPointer<QWidget> widget;
   mutable QSize widgetSize;
   bool resizable;
   Qt::Alignment alignment;
};

#endif

QT_END_NAMESPACE

#endif
