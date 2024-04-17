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

#include <qmacnativewidget_mac.h>

#include <qapplication.h>
#include <qdebug.h>
#include <qplatform_nativeinterface.h>
#include <qwindow.h>

#import  <Cocoa/Cocoa.h>

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


NSView *getEmbeddableView(QWindow *qtWindow)
{
    // Make sure the platform window is created
    qtWindow->create();

    // Inform the window that it's a subwindow of a non-Qt window. This must be
    // done after create() because we need to have a QPlatformWindow instance.
    // The corresponding NSWindow will not be shown and can be deleted later.
    typedef void (*SetEmbeddedInForeignViewFunction)(QPlatformWindow *window, bool embedded);
    reinterpret_cast<SetEmbeddedInForeignViewFunction>(resolvePlatformFunction("setEmbeddedInForeignView"))(qtWindow->handle(), true);

    // Get the Qt content NSView for the QWindow from the Qt platform plugin
    QPlatformNativeInterface *platformNativeInterface = QGuiApplication::platformNativeInterface();
    NSView *qtView = (NSView *)platformNativeInterface->nativeResourceForWindow("nsview", qtWindow);

    return qtView; // qtView is ready for use.
}

QMacNativeWidget::QMacNativeWidget(NSView *parentView)
    : QWidget(nullptr)
{
    //d_func()->topData()->embedded = true;
    setPalette(QPalette(Qt::transparent));
    setAttribute(Qt::WA_SetPalette, false);
    setAttribute(Qt::WA_LayoutUsesWidgetRect);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_NoSystemBackground, false);
}

QMacNativeWidget::~QMacNativeWidget()
{
}

QSize QMacNativeWidget::sizeHint() const
{
    // QMacNativeWidget really does not have any other choice
    // than to fill its designated area.

    if (windowHandle())
        return windowHandle()->size();

    return QWidget::sizeHint();
}
NSView *QMacNativeWidget::nativeView() const
{
    winId();
    return getEmbeddableView(windowHandle());
}

bool QMacNativeWidget::event(QEvent *ev)
{
   return QWidget::event(ev);
}
