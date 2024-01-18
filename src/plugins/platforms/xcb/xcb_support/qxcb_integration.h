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

#ifndef QXCB_INTEGRATION_H
#define QXCB_INTEGRATION_H

#include <qplatform_integration.h>
#include <qplatform_screen.h>
#include <qxcb_export.h>

#include <xcb/xcb.h>

class QXcbConnection;
class QAbstractEventDispatcher;
class QXcbNativeInterface;
class QXcbScreen;

class Q_XCB_EXPORT QXcbIntegration : public QPlatformIntegration
{
 public:
   QXcbIntegration(const QStringList &parameters, int &argc, char **argv);
   ~QXcbIntegration();

   QPlatformWindow *createPlatformWindow(QWindow *window) const override;

#ifndef QT_NO_OPENGL
   QPlatformOpenGLContext *createPlatformOpenGLContext(QOpenGLContext *context) const override;
#endif

   QPlatformBackingStore *createPlatformBackingStore(QWindow *window) const override;

   QPlatformOffscreenSurface *createPlatformOffscreenSurface(QOffscreenSurface *surface) const override;

   bool hasCapability(Capability cap) const override;
   QAbstractEventDispatcher *createEventDispatcher() const override;
   void initialize() override;

   void moveToScreen(QWindow *window, int screen);

   QPlatformFontDatabase *fontDatabase() const override;

   QPlatformNativeInterface *nativeInterface()const override;

#ifndef QT_NO_CLIPBOARD
   QPlatformClipboard *clipboard() const override;
#endif

#ifndef QT_NO_DRAGANDDROP
   QPlatformDrag *drag() const override;
#endif

   QPlatformInputContext *inputContext() const override;

#ifndef QT_NO_ACCESSIBILITY
   QPlatformAccessibility *accessibility() const override;
#endif

   QPlatformServices *services() const override;

   Qt::KeyboardModifiers queryKeyboardModifiers() const override;
   QList<int> possibleKeys(const QKeyEvent *e) const override;

   QStringList themeNames() const override;
   QPlatformTheme *createPlatformTheme(const QString &name) const override;
   QVariant styleHint(StyleHint hint) const override;

   QXcbConnection *defaultConnection() const {
      return m_connections.first();
   }

   QByteArray wmClass() const;

#if ! defined(QT_NO_SESSIONMANAGER) && defined(XCB_USE_SM)
   QPlatformSessionManager *createPlatformSessionManager(const QString &id, const QString &key) const override;
#endif

   void sync() override;

   static QXcbIntegration *instance() {
      return m_instance;
   }

 private:
   QList<QXcbConnection *> m_connections;

   QScopedPointer<QPlatformFontDatabase> m_fontDatabase;
   QScopedPointer<QXcbNativeInterface> m_nativeInterface;

   QScopedPointer<QPlatformInputContext> m_inputContext;

#ifndef QT_NO_ACCESSIBILITY
   mutable QScopedPointer<QPlatformAccessibility> m_accessibility;
#endif

   QScopedPointer<QPlatformServices> m_services;

   friend class QXcbConnection; // access QPlatformIntegration::screenAdded()

   mutable QByteArray m_wmClass;
   const char *m_instanceName;
   bool m_canGrab;
   xcb_visualid_t m_defaultVisualId;

   static QXcbIntegration *m_instance;
};

#endif
