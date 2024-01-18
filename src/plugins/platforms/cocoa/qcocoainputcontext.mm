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

#include <qnsview.h>
#include <qcocoainputcontext.h>
#include <qcocoanativeinterface.h>
#include <qcocoawindow.h>

#include <Carbon/Carbon.h>

#include <QRect>
#include <QApplication>
#include <QWindow>

QCocoaInputContext::QCocoaInputContext()
   : QPlatformInputContext(), mWindow(QApplication::focusWindow())
{
   QMetaObject::invokeMethod(this, "connectSignals", Qt::QueuedConnection);
   updateLocale();
}

QCocoaInputContext::~QCocoaInputContext()
{
}

void QCocoaInputContext::reset()
{
   QPlatformInputContext::reset();

   if (! mWindow) {
      return;
   }

   QNSView *view = static_cast<QCocoaWindow *>(mWindow->handle())->qtView();
   if (!view) {
      return;
   }

   QMacAutoReleasePool pool;
   if (NSTextInputContext *ctxt = [NSTextInputContext currentInputContext]) {
      [ctxt discardMarkedText];
      [view unmarkText];
   }
}

void QCocoaInputContext::connectSignals()
{
   connect(qApp, &QApplication::focusObjectChanged, this, &QCocoaInputContext::focusObjectChanged);
   focusObjectChanged(qApp->focusObject());
}

void QCocoaInputContext::focusObjectChanged(QObject *focusObject)
{
   mWindow = QApplication::focusWindow();
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
