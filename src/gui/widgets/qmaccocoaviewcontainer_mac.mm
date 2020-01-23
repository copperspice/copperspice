/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
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

#import  <Cocoa/Cocoa.h>

#include <qmaccocoaviewcontainer_mac.h>

#include <qdebug.h>
#include <qplatform_nativeinterface.h>
#include <qwindow.h>

#include <qwidget_p.h>




namespace {
// TODO use QtMacExtras copy of this function when available.

inline QPlatformNativeInterface::FP_Integration resolvePlatformFunction(const QByteArray &functionName)
{
    QPlatformNativeInterface *nativeInterface = QGuiApplication::platformNativeInterface();

    QPlatformNativeInterface::FP_Integration function =
        nativeInterface->nativeResourceFunctionForIntegration(functionName);

    if (! function)
         qWarning() << "Unable to resolve function" << functionName
                    << "from QGuiApplication::platformNativeInterface()->nativeResourceFunctionForIntegration()";
    return function;
}

} //namespsace
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

QMacCocoaViewContainer::QMacCocoaViewContainer(NSView *view, QWidget *parent)
   : QWidget(*new QMacCocoaViewContainerPrivate, parent, 0)
{
    if (view) {
        setCocoaView(view);
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
NSView *QMacCocoaViewContainer::cocoaView() const
{
   Q_D(const QMacCocoaViewContainer);
    return d->nsview;
}

/*!
    Sets the NSView to contain to be \a cocoaViewToWrap and retains it. If this
    container already had a view set, it will release the previously set view.
*/
void QMacCocoaViewContainer::setCocoaView(NSView *view)
{
    Q_D(QMacCocoaViewContainer);
    NSView *oldView = d->nsview;
    [view retain];
    d->nsview = view;

    // Create window and platformwindow
    winId();
    QPlatformWindow *platformWindow = this->windowHandle()->handle();

    // Set the new view as the content view for the window.
    typedef void (*SetWindowContentViewFunction)(QPlatformWindow *window, NSView *nsview);
    reinterpret_cast<SetWindowContentViewFunction>(resolvePlatformFunction("setwindowcontentview"))(platformWindow, view);

    [oldView release];
}

