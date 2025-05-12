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

#include <qplatform_accessibility.h>
#include <qplatform_clipboard.h>
#include <qplatform_cursor.h>
#include <qplatform_drag.h>
#include <qplatform_inputcontext.h>
#include <qplatform_services.h>

#include <qapplication_p.h>
#include <qgenericunix_eventdispatcher_p.h>
#include <qgenericunix_fontdatabase_p.h>
#include <qgenericunix_theme_p.h>

namespace QtWaylandClient {

class GenericWaylandTheme: public QGenericUnixTheme
{
 public:
   static QStringList themeNames() {
      QStringList result;

      if (QGuiApplication::desktopSettingsAware()) {
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
   : mFontDb(new QGenericUnixFontDatabase()), mNativeInterface(nullptr)
{
   mClipboard = nullptr;
   mDrag      = nullptr;
}

QWaylandIntegration::~QWaylandIntegration()
{
   delete mClipboard;
   delete mDrag;
   delete mFontDb;
   delete mNativeInterface;
}

QAbstractEventDispatcher *QWaylandIntegration::createEventDispatcher() const
{
   return createUnixEventDispatcher();
}

QPlatformBackingStore *QWaylandIntegration::createPlatformBackingStore(QWindow *window) const
{
   return nullptr;
}

QPlatformTheme *QWaylandIntegration::createPlatformTheme(const QString &name) const
{
   return GenericWaylandTheme::createUnixTheme(name);
}

QPlatformWindow *QWaylandIntegration::createPlatformWindow(QWindow *window) const
{
   return nullptr;
}

QStringList QWaylandIntegration::themeNames() const
{
   return GenericWaylandTheme::themeNames();
}

}
