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

#ifndef QWINDOWSINTEGRATION_H
#define QWINDOWSINTEGRATION_H

#include <qplatform_integration.h>
#include <qscopedpointer.h>

class QWindowsWindow;
class QWindowsStaticOpenGLContext;

struct QWindowsIntegrationPrivate;
struct QWindowsWindowData;

class QWindowsIntegration : public QPlatformIntegration
{
 public:
   enum Options {
      // Options to be passed on command line.
      FontDatabaseFreeType = 0x1,
      FontDatabaseNative = 0x2,
      DisableArb = 0x4,
      NoNativeDialogs = 0x8,
      XpNativeDialogs = 0x10,
      DontPassOsMouseEventsSynthesizedFromTouch = 0x20 // Do not pass OS-generated mouse events from touch.
   };

   explicit QWindowsIntegration(const QStringList &paramList);
   virtual ~QWindowsIntegration();

   bool hasCapability(QPlatformIntegration::Capability cap) const override;

   QPlatformWindow *createPlatformWindow(QWindow *window) const override;

#ifndef QT_NO_OPENGL
   QPlatformOpenGLContext *createPlatformOpenGLContext(QOpenGLContext *context) const override;
   QOpenGLContext::OpenGLModuleType openGLModuleType() override;
   static QWindowsStaticOpenGLContext *staticOpenGLContext();
#endif

   QAbstractEventDispatcher *createEventDispatcher() const override;
   void initialize() override;

#ifndef QT_NO_CLIPBOARD
   QPlatformClipboard *clipboard() const override;
#  ifndef QT_NO_DRAGANDDROP
   QPlatformDrag *drag() const override;
#  endif
#endif

   QPlatformInputContext *inputContext() const override;

#ifndef QT_NO_ACCESSIBILITY
   QPlatformAccessibility *accessibility() const override;
#endif

   QPlatformFontDatabase *fontDatabase() const override;
   QStringList themeNames() const override;
   QPlatformTheme *createPlatformTheme(const QString &name) const override;
   QPlatformServices *services() const override;
   QVariant styleHint(StyleHint hint) const override;

   Qt::KeyboardModifiers queryKeyboardModifiers() const override;
   QList<int> possibleKeys(const QKeyEvent *e) const override;

   static QWindowsIntegration *instance() {
      return m_instance;
   }

   inline void emitScreenAdded(QPlatformScreen *s, bool isPrimary = false) {
      screenAdded(s, isPrimary);
   }

   inline void emitDestroyScreen(QPlatformScreen *s) {
      destroyScreen(s);
   }

   unsigned options() const;

#if ! defined(QT_NO_SESSIONMANAGER)
   QPlatformSessionManager *createPlatformSessionManager(const QString &id, const QString &key) const override;
#endif

 protected:
   virtual QWindowsWindow *createPlatformWindowHelper(QWindow *window, const QWindowsWindowData &) const;

 private:
   QScopedPointer<QWindowsIntegrationPrivate> d;

   static QWindowsIntegration *m_instance;
};

#endif
