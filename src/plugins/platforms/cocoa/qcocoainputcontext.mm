/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

#include "qnsview.h"
#include "qcocoainputcontext.h"
#include "qcocoanativeinterface.h"
#include "qcocoawindow.h"

#include <Carbon/Carbon.h>

#include <QtCore/QRect>
#include <QtGui/QGuiApplication>
#include <QtGui/QWindow>

QT_BEGIN_NAMESPACE

/*!
    \class QCocoaInputContext
    \brief Cocoa Input context implementation

    Handles input of foreign characters (particularly East Asian)
    languages.

    \section1 Testing

    \list
    \o Select input sources like 'Kotoeri' in Language & Text Preferences
    \o Compile the \a mainwindows/mdi example and open a text window.
    \o In the language bar, switch to 'Hiragana'.
    \o In a text editor control, type the syllable \a 'la'.
       Underlined characters show up, indicating that there is completion
       available. Press the Space key two times. A completion popup occurs
       which shows the options.
    \endlist

    \section1 Interaction

    Input method support in Cocoa uses NSTextInput protorol. Therefore
    almost all functionality is implemented in QNSView.

    \ingroup qt-lighthouse-cocoa
*/



QCocoaInputContext::QCocoaInputContext()
    : QPlatformInputContext()
    , mWindow(QGuiApplication::focusWindow())
{
    QMetaObject::invokeMethod(this, "connectSignals", Qt::QueuedConnection);
    updateLocale();
}

QCocoaInputContext::~QCocoaInputContext()
{
}

/*!
    \brief Cancels a composition.
*/

void QCocoaInputContext::reset()
{
    QPlatformInputContext::reset();

    if (!mWindow)
        return;

    QNSView *view = static_cast<QCocoaWindow *>(mWindow->handle())->qtView();
    if (!view)
        return;

    QMacAutoReleasePool pool;
    if (NSTextInputContext *ctxt = [NSTextInputContext currentInputContext]) {
        [ctxt discardMarkedText];
        [view unmarkText];
    }
}

void QCocoaInputContext::connectSignals()
{
    connect(qApp, SIGNAL(focusObjectChanged(QObject*)), this, SLOT(focusObjectChanged(QObject*)));
    focusObjectChanged(qApp->focusObject());
}

void QCocoaInputContext::focusObjectChanged(QObject *focusObject)
{
    Q_UNUSED(focusObject);
    mWindow = QGuiApplication::focusWindow();
}

void QCocoaInputContext::updateLocale()
{
    TISInputSourceRef source = TISCopyCurrentKeyboardInputSource();
    CFArrayRef languages = (CFArrayRef) TISGetInputSourceProperty(source, kTISPropertyInputSourceLanguages);
    if (CFArrayGetCount(languages) > 0) {
        CFStringRef langRef = (CFStringRef)CFArrayGetValueAtIndex(languages, 0);
        QString name = QCFString::toQString(langRef);
        QLocale locale(name);
        if (m_locale != locale) {
            m_locale = locale;
            emitLocaleChanged();
        }
    }
    CFRelease(source);
}

QT_END_NAMESPACE
