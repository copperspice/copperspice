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

#include <qgenericunix_services_p.h>

#include <qstandardpaths.h>
#include <qprocess.h>
#include <qurl.h>
#include <qdebug.h>

#include <stdlib.h>

static inline QByteArray detectDesktopEnvironment()
{
   const QByteArray xdgCurrentDesktop = qgetenv("XDG_CURRENT_DESKTOP");
   if (! xdgCurrentDesktop.isEmpty()) {
      return xdgCurrentDesktop.toUpper();   // KDE, GNOME, UNITY, LXDE, MATE, XFCE...
   }

   // Classic fallbacks
   if (! qgetenv("KDE_FULL_SESSION").isEmpty()) {
      return "KDE";
   }

   if (! qgetenv("GNOME_DESKTOP_SESSION_ID").isEmpty()) {
      return "GNOME";
   }

   // Fallback to checking $DESKTOP_SESSION (unreliable)
   const QByteArray desktopSession = qgetenv("DESKTOP_SESSION");

   if (desktopSession == "gnome") {
      return "GNOME";
   }

   if (desktopSession == "xfce") {
      return "XFCE";
   }

   return "UNKNOWN";
}

static inline bool checkExecutable(const QString &candidate, QString *result)
{
   *result = QStandardPaths::findExecutable(candidate);
   return !result->isEmpty();
}

static inline bool detectWebBrowser(const QByteArray &desktop, bool checkBrowserVariable, QString *browser)
{
   const QString browsers[] = {"google-chrome", "firefox", "mozilla", "opera"};

   browser->clear();
   if (checkExecutable("xdg-open", browser)) {
      return true;
   }

   if (checkBrowserVariable) {
      QByteArray browserVariable = qgetenv("DEFAULT_BROWSER");

      if (browserVariable.isEmpty()) {
         browserVariable = qgetenv("BROWSER");
      }

      if (! browserVariable.isEmpty() && checkExecutable(QString::fromUtf8(browserVariable), browser)) {
         return true;
      }
   }

   if (desktop == "KDE") {
      // Konqueror launcher

      if (checkExecutable("kfmclient", browser)) {
         browser->append(" exec");
         return true;
      }

   } else if (desktop == "GNOME") {
      if (checkExecutable("gnome-open", browser)) {
         return true;
      }
   }

   for (auto &item : browsers) {
      if (checkExecutable(item, browser)) {
         return true;
      }
   }

   return false;
}

static inline bool launch(const QString &launcher, const QUrl &url)
{
   const QString command = launcher + QLatin1Char(' ') + url.toEncoded();

#if defined(CS_SHOW_DEBUG_PLATFORM)
   qDebug("launch() Starting process %s", csPrintable(command));
#endif

#if defined(QT_NO_PROCESS)
   const bool ok = ::system(csPrintable(command + " &"));
#else
   const bool ok = QProcess::startDetached(command);
#endif

   if (!ok) {
      qWarning("launch() Failed to start process %s", csPrintable(command));
   }

   return ok;
}

QByteArray QGenericUnixServices::desktopEnvironment() const
{
   static const QByteArray result = detectDesktopEnvironment();
   return result;
}

bool QGenericUnixServices::openUrl(const QUrl &url)
{
   if (url.scheme() == "mailto") {
      return openDocument(url);
   }

   if (m_webBrowser.isEmpty() && ! detectWebBrowser(desktopEnvironment(), true, &m_webBrowser)) {
      qWarning("Unable to detect a web browser to launch '%s'", csPrintable(url.toString()));
      return false;
   }
   return launch(m_webBrowser, url);
}

bool QGenericUnixServices::openDocument(const QUrl &url)
{
   if (m_documentLauncher.isEmpty() && ! detectWebBrowser(desktopEnvironment(), false, &m_documentLauncher)) {
      qWarning("Unable to detect a launcher for '%s'", csPrintable(url.toString()));
      return false;
   }

   return launch(m_documentLauncher, url);
}

