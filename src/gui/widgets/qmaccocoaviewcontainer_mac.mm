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

#include <qwidget_p.h>
#include <qmaccocoaviewcontainer_mac.h>
#include <qt_mac_p.h>

QT_BEGIN_NAMESPACE

class QMacCocoaViewContainerPrivate : public QWidgetPrivate
{
   Q_DECLARE_PUBLIC(QMacCocoaViewContainer)

 public:
   NSView *nsview;

   QMacCocoaViewContainerPrivate();
   ~QMacCocoaViewContainerPrivate();
};

QMacCocoaViewContainerPrivate::QMacCocoaViewContainerPrivate()
   : nsview(0)
{
}

QMacCocoaViewContainerPrivate::~QMacCocoaViewContainerPrivate()
{
   [nsview release];
}

QMacCocoaViewContainer::QMacCocoaViewContainer(void *cocoaViewToWrap, QWidget *parent)
   : QWidget(*new QMacCocoaViewContainerPrivate, parent, 0)
{
   if (cocoaViewToWrap) {
      setCocoaView(cocoaViewToWrap);
   }

   // QMacCocoaViewContainer requires a native window handle.
   setAttribute(Qt::WA_NativeWindow);
}

/*!
    Destroy the QMacCocoaViewContainer and release the wrapped view.
*/
QMacCocoaViewContainer::~QMacCocoaViewContainer()
{
}

/*!
    Returns the NSView that has been set on this container.  The returned view
    has been autoreleased, so you will need to retain it if you want to make
    use of it.
*/
void *QMacCocoaViewContainer::cocoaView() const
{
   Q_D(const QMacCocoaViewContainer);
   return [[d->nsview retain] autorelease];
}

/*!
    Sets the NSView to contain to be \a cocoaViewToWrap and retains it. If this
    container already had a view set, it will release the previously set view.
*/
void QMacCocoaViewContainer::setCocoaView(void *cocoaViewToWrap)
{
   Q_D(QMacCocoaViewContainer);
   QMacCocoaAutoReleasePool pool;
   NSView *view = static_cast<NSView *>(cocoaViewToWrap);
   NSView *oldView = d->nsview;
   destroy(true, true);
   [view retain];
   d->nsview = view;

   create(WId(d->nsview), false, true);

   [oldView release];
}

QT_END_NAMESPACE
