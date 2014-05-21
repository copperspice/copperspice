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

#import <Cocoa/Cocoa.h>
#import <private/qcocoaview_mac_p.h>
#include "qmacnativewidget_mac.h"
#include <private/qwidget_p.h>

/*!
    \class QMacNativeWidget
    \since 4.5
    \brief The QMacNativeWidget class provides a widget for Mac OS X that provides a way to put Qt widgets into Carbon
    or Cocoa hierarchies depending on how Qt was configured.

    \ingroup advanced

    On Mac OS X, there is a difference between a window and view;
    normally expressed as widgets in Qt.  Qt makes assumptions about its
    parent-child hierarchy that make it complex to put an arbitrary Qt widget
    into a hierarchy of "normal" views from Apple frameworks. QMacNativeWidget
    bridges the gap between views and windows and makes it possible to put a
    hierarchy of Qt widgets into a non-Qt window or view.

    QMacNativeWidget pretends it is a window (i.e. isWindow() will return true),
    but it cannot be shown on its own. It needs to be put into a window
    when it is created or later through a native call.

    QMacNativeWidget works for either Carbon or Cocoa depending on how Qt was configured. If Qt is
    using Carbon, QMacNativeWidget will embed into Carbon hierarchies. If Qt is
    using Cocoa, QMacNativeWidget embeds into Cocoa hierarchies.

    Here is an example of putting a QPushButton into a NSWindow:

    \snippet doc/src/snippets/qmacnativewidget/main.mm 0

    On Carbon, this would do the equivalent:

    \snippet doc/src/snippets/qmacnativewidget/main.mm 1

    Note that QMacNativeWidget requires knowledge of Carbon or Cocoa. All it
    does is get the Qt hierarchy into a window not owned by Qt. It is then up
    to the programmer to ensure it is placed correctly in the window and
    responds correctly to events.
*/

QT_BEGIN_NAMESPACE

class QMacNativeWidgetPrivate : public QWidgetPrivate
{
};

extern OSViewRef qt_mac_create_widget(QWidget *widget, QWidgetPrivate *widgetPrivate, OSViewRef parent);


/*!
    Create a QMacNativeWidget with \a parentView as its "superview" (i.e.,
    parent). The \a parentView is either an HIViewRef if Qt is using Carbon or
    a NSView pointer if Qt is using Cocoa.
*/
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
