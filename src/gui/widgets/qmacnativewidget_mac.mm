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

#import  <Cocoa/Cocoa.h>
#import  <qcocoaview_mac_p.h>

#include <qmacnativewidget_mac.h>
#include <qwidget_p.h>

QT_BEGIN_NAMESPACE

class QMacNativeWidgetPrivate : public QWidgetPrivate
{
};

extern OSViewRef qt_mac_create_widget(QWidget *widget, QWidgetPrivate *widgetPrivate, OSViewRef parent);

QMacNativeWidget::QMacNativeWidget(void *parentView)
   : QWidget(*new QMacNativeWidgetPrivate, 0, Qt::Window)
{
   Q_D(QMacNativeWidget);
   OSViewRef myView = qt_mac_create_widget(this, d, OSViewRef(parentView));

   d->topData()->embedded = true;
   create(WId(myView), false, false);
   setPalette(QPalette(Qt::transparent));
   setAttribute(Qt::WA_SetPalette, false);
   setAttribute(Qt::WA_LayoutUsesWidgetRect);
}

/*!
    Destroy the QMacNativeWidget.
*/
QMacNativeWidget::~QMacNativeWidget()
{
}

/*!
    \reimp
*/
QSize QMacNativeWidget::sizeHint() const
{
   return QSize(200, 200);
}
/*!
    \reimp
*/
bool QMacNativeWidget::event(QEvent *ev)
{
   return QWidget::event(ev);
}

QT_END_NAMESPACE
