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

#include <qwayland_integration_p.h>

#include <qdebug.h>
#include <qopenglcontext.h>
#include <qplatform_accessibility.h>
#include <qplatform_clipboard.h>
#include <qplatform_cursor.h>
#include <qplatform_drag.h>
#include <qplatform_inputcontext.h>
#include <qplatform_services.h>
#include <qsocketnotifier.h>

#include <qapplication_p.h>
#include <qgenericunix_eventdispatcher_p.h>
#include <qgenericunix_fontdatabase_p.h>
#include <qgenericunix_theme_p.h>
#include <qplatform_inputcontextfactory_p.h>
#include <qwayland_clientbuffer_integration_p.h>
#include <qwayland_clientbuffer_integrationfactory_p.h>
#include <qwayland_clipboard_p.h>
#include <qwayland_display_p.h>
#include <qwayland_dnd_p.h>
#include <qwayland_hardware_integration_p.h>
#include <qwayland_inputcontext_p.h>
#include <qwayland_inputdevice_integration_p.h>
#include <qwayland_inputdevice_integrationfactory_p.h>
#include <qwayland_inputdevice_p.h>
#include <qwayland_native_interface_p.h>
#include <qwayland_screen_p.h>
#include <qwayland_serverbuffer_integration_p.h>
#include <qwayland_serverbuffer_integrationfactory_p.h>
#include <qwayland_shell_integration_p.h>
#include <qwayland_shell_integrationfactory_p.h>
#include <qwayland_shm_backingstore_p.h>
#include <qwayland_shm_window_p.h>
#include <qwayland_windowmanager_integration_p.h>

namespace QtWaylandClient {

class GenericWaylandTheme: public QGenericUnixTheme
{
 public:
   static QStringList themeNames() {
      QStringList result;

      if (QApplication::desktopSettingsAware()) {
         const QByteArray desktopEnvironment = QApplicationPrivate::platformIntegration()->services()->desktopEnvironment();

         if (desktopEnvironment == "KDE") {
            result.push_back("kde");

         } else if (! desktopEnvironment.isEmpty() && desktopEnvironment != "UNKNOWN" &&
            desktopEnvironment != "GNOME" && desktopEnvironment != "UNITY" &&
            desktopEnvironment != "MATE"  && desktopEnvironment != "XFCE" && desktopEnvironment != "LXDE") {
            // Ignore X11 desktop environments

            result.push_back(QString::fromUtf8(desktopEnvironment.toLower()));
         }
      }

      if (result.isEmpty()) {
         result.push_back(QGenericUnixTheme::m_name);
      }

      return result;
   }
};

QWaylandIntegration::QWaylandIntegration()
   : m_clientBufferIntegration(nullptr), m_serverBufferIntegration(nullptr), m_shellIntegration(nullptr), m_inputDeviceIntegration(nullptr),
     m_clientBufferIntegrationInitialized(false), m_serverBufferIntegrationInitialized(false), m_shellIntegrationInitialized(false),

#ifndef QT_NO_ACCESSIBILITY
     m_accessibility(new QPlatformAccessibility()),
#endif

     m_fontDb(new QGenericUnixFontDatabase()), m_nativeInterface(new QWaylandNativeInterface(this))
{
   m_display   = new QWaylandDisplay(this);
   m_clipboard = new QWaylandClipboard(m_display);
   m_drag      = new QWaylandDrag(m_display);
}

QWaylandIntegration::~QWaylandIntegration()
{
#ifndef QT_NO_ACCESSIBILITY
   delete m_accessibility;
#endif

   delete m_display;
   delete m_clipboard;
   delete m_drag;
   delete m_fontDb;
   delete m_nativeInterface;
}

void QWaylandIntegration::initialize()
{
   QAbstractEventDispatcher *dispatcher = QApplicationPrivate::eventDispatcher;

   QObject::connect(dispatcher, &QAbstractEventDispatcher::aboutToBlock, m_display, &QWaylandDisplay::flushRequests);
   QObject::connect(dispatcher, &QAbstractEventDispatcher::awake,        m_display, &QWaylandDisplay::flushRequests);

   int fd = wl_display_get_fd(m_display->wl_display());

   QSocketNotifier *sn = new QSocketNotifier(fd, QSocketNotifier::Read, m_display);
   QObject::connect(sn, &QSocketNotifier::activated, m_display, &QWaylandDisplay::flushRequests);
}

#ifndef QT_NO_ACCESSIBILITY
QPlatformAccessibility *QWaylandIntegration::accessibility() const
{
   return m_accessibility;
}
#endif

QPlatformClipboard *QWaylandIntegration::clipboard() const
{
   return m_clipboard;
}

#ifndef QT_NO_OPENGL
QPlatformOpenGLContext *QWaylandIntegration::createPlatformOpenGLContext(QOpenGLContext *context) const
{
   // pending implementation

   return nullptr;
}
#endif

QWaylandClientBufferIntegration *QWaylandIntegration::clientBufferIntegration() const
{
   if (! m_clientBufferIntegrationInitialized) {
      const_cast<QWaylandIntegration *>(this)->initializeClientBufferIntegration();
   }

   return m_clientBufferIntegration != nullptr && m_clientBufferIntegration->isValid() ? m_clientBufferIntegration : nullptr;
}

QAbstractEventDispatcher *QWaylandIntegration::createEventDispatcher() const
{
   return createUnixEventDispatcher();
}

QWaylandInputDevice *QWaylandIntegration::createInputDevice(QWaylandDisplay *display, int version, uint32_t id)
{
   if (m_inputDeviceIntegration != nullptr) {
      return m_inputDeviceIntegration->createInputDevice(display, version, id);
   }

   return new QWaylandInputDevice(display, version, id);
}

QPlatformBackingStore *QWaylandIntegration::createPlatformBackingStore(QWindow *window) const
{
   return new QWaylandShmBackingStore(window);
}

QPlatformTheme *QWaylandIntegration::createPlatformTheme(const QString &name) const
{
   return GenericWaylandTheme::createUnixTheme(name);
}

QPlatformWindow *QWaylandIntegration::createPlatformWindow(QWindow *window) const
{
   // pending implementation

   return nullptr;
}

QWaylandDisplay *QWaylandIntegration::display() const
{
   return m_display;
}

QPlatformDrag *QWaylandIntegration::drag() const
{
   return m_drag;
}

QPlatformFontDatabase *QWaylandIntegration::fontDatabase() const
{
   return m_fontDb;
}

QPlatformNativeInterface *QWaylandIntegration::nativeInterface() const
{
   return m_nativeInterface;
}

QStringList QWaylandIntegration::themeNames() const
{
   return GenericWaylandTheme::themeNames();
}

QWaylandServerBufferIntegration *QWaylandIntegration::serverBufferIntegration() const
{
   if (! m_serverBufferIntegrationInitialized) {
      const_cast<QWaylandIntegration *>(this)->initializeServerBufferIntegration();
   }

   return m_serverBufferIntegration;
}

QWaylandShellIntegration *QWaylandIntegration::shellIntegration() const
{
   if (m_shellIntegrationInitialized) {
      const_cast<QWaylandIntegration *>(this)->initializeShellIntegration();
   }

   return m_shellIntegration;
}

void QWaylandIntegration::initializeClientBufferIntegration()
{
   m_clientBufferIntegrationInitialized = true;

   // pending implementation

   if (m_clientBufferIntegration == nullptr) {
      qWarning("Unable to load client buffer integration\n");
   } else {
      m_clientBufferIntegration->initialize(m_display);
   }
}

void QWaylandIntegration::initializeServerBufferIntegration()
{
   m_serverBufferIntegrationInitialized = true;

   // pending implementation

   if (m_serverBufferIntegration == nullptr) {
      qWarning("Unable to load server buffer integration\n");
   } else {
      m_serverBufferIntegration->initialize(m_display);
   }
}

void QWaylandIntegration::initializeShellIntegration()
{
   // pending implementation
}
}

