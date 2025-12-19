/***********************************************************************
*
* Copyright (c) 2012-2025 Barbara Geller
* Copyright (c) 2012-2025 Ansel Sermersheim
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

#ifndef QWAYLAND_INTEGRATION_H
#define QWAYLAND_INTEGRATION_H

#include <qplatform_integration.h>

namespace QtWaylandClient {

class QWaylandBuffer;
class QWaylandClientBufferIntegration;
class QWaylandDisplay;
class QWaylandInputDevice;
class QWaylandInputDeviceIntegration;
class QWaylandServerBufferIntegration;
class QWaylandShellIntegration;

class Q_WAYLAND_CLIENT_EXPORT QWaylandIntegration : public QPlatformIntegration
{
 public:
   QWaylandIntegration();
   ~QWaylandIntegration();

#ifndef QT_NO_ACCESSIBILITY
   QPlatformAccessibility *accessibility() const override;
#endif

#ifndef QT_NO_OPENGL
   QPlatformOpenGLContext *createPlatformOpenGLContext(QOpenGLContext *context) const override;
#endif

   virtual QWaylandClientBufferIntegration *clientBufferIntegration() const;

   QAbstractEventDispatcher *createEventDispatcher() const override;
   QWaylandInputDevice *createInputDevice(QWaylandDisplay *display, int version, uint32_t id);
   QPlatformBackingStore *createPlatformBackingStore(QWindow *window) const override;
   QPlatformTheme *createPlatformTheme(const QString &name) const override;
   QPlatformWindow *createPlatformWindow(QWindow *window) const override;

   QPlatformClipboard *clipboard() const override;

   QPlatformDrag *drag() const override;

   void platform_destroyScreen(QPlatformScreen *screen) {
      QPlatformIntegration::destroyScreen(screen);
   }

   QWaylandDisplay *display() const;

   QPlatformFontDatabase *fontDatabase() const override;

   QWaylandShellIntegration *getShellIntegration() const {
      return m_shellIntegration;
   }

   bool hasCapability(QPlatformIntegration::Capability cap) const override;

   void initialize() override;
   QPlatformInputContext *inputContext() const override;

   QPlatformNativeInterface *nativeInterface() const override;

   QVariant styleHint(StyleHint hint) const override;

   void platform_screenAdded(QPlatformScreen *screen, bool isPrimary = false) {
      QPlatformIntegration::screenAdded(screen, isPrimary);
   }

   QPlatformServices *services() const override;

   virtual QWaylandServerBufferIntegration *serverBufferIntegration() const;
   virtual QWaylandShellIntegration *shellIntegration() const;

   QStringList themeNames() const override;

 protected:
   QWaylandClientBufferIntegration *m_clientBufferIntegration;
   QWaylandServerBufferIntegration *m_serverBufferIntegration;
   QWaylandShellIntegration *m_shellIntegration;
   QWaylandInputDeviceIntegration *m_inputDeviceIntegration;

 private:
   QWaylandShellIntegration *createShellIntegration(const QString &interfaceName);

   void initializeClientBufferIntegration();
   void initializeServerBufferIntegration();
   void initializeShellIntegration();
   void initializeInputDeviceIntegration();

   bool m_clientBufferIntegrationInitialized;
   bool m_serverBufferIntegrationInitialized;
   bool m_shellIntegrationInitialized;

#ifndef QT_NO_ACCESSIBILITY
   QPlatformAccessibility *m_accessibility;
#endif

   QPlatformClipboard *m_clipboard;
   QPlatformDrag *m_drag;
   QPlatformFontDatabase *m_fontDb;
   QWaylandDisplay *m_display;
   QPlatformNativeInterface *m_nativeInterface;

   QScopedPointer<QPlatformInputContext> m_inputContext;
};

}

#endif
