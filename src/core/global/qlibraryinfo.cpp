/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
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
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#include <qdir.h>
#include <qfile.h>
#include <qlibraryinfo.h>
#include <qscopedpointer.h>
#include <qcoreapplication.h>
#include <qregularexpression.h>
#include <cs_build_info.h>

#ifdef Q_OS_DARWIN
#  include "qcore_mac_p.h"
#endif

#ifndef QT_NO_SETTINGS

QLibraryInfo::QLibraryInfo()
{
}

QSettings *QLibraryInfo::configuration()
{
   return qt_library_settings();
}

QSettings *QLibraryInfo::findConfiguration()
{
   QString qtconfig(":/cs/etc/cs.conf");

   if (! QFile::exists(qtconfig) && QCoreApplication::instance()) {

#ifdef Q_OS_DARWIN
      CFBundleRef bundleRef = CFBundleGetMainBundle();

      if (bundleRef) {
         QCFType<CFURLRef> urlRef = CFBundleCopyResourceURL(bundleRef, QCFString("cs.conf"), 0, 0);

         if (urlRef) {
            QCFString path = CFURLCopyFileSystemPath(urlRef, kCFURLPOSIXPathStyle);
            qtconfig = QDir::cleanPath(path);
         }
      }

      if (qtconfig.isEmpty())
#endif
      {
         QDir pwd(QCoreApplication::applicationDirPath());
         qtconfig = pwd.filePath("cs.conf");
      }
   }

   if (QFile::exists(qtconfig)) {
      return new QSettings(qtconfig, QSettings::IniFormat);
   }

   return 0;     // no luck
}

#ifndef QT_NO_DATESTRING
QDate QLibraryInfo::buildDate()
{
   return QDate::fromString(QString::fromLatin1(BUILD_DATE), Qt::ISODate);
}
#endif

QString QLibraryInfo::licensee()
{
   return QString("Open Source");
}

QString QLibraryInfo::licensedProducts()
{
   return QString("Open Source");
}

QString QLibraryInfo::location(LibraryLocation loc)
{
   QString retval;

   QSettings *config = QLibraryInfo::configuration();

   if (config) {

      QString key;
      QString defaultValue;

      switch (loc) {

         case PluginsPath:
            key = "Plugins";
            defaultValue = "plugins";
            break;

         case ImportsPath:
            key = "Imports";
            defaultValue = "imports";
            break;

         case Qml2ImportsPath:
            defaultValue = "qml";
            break;

         case TranslationsPath:
            key = "Translations";
            defaultValue = "translations";
            break;

         case SettingsPath:
            key = "Settings";
            break;

         default:
            break;
      }

      if (! key.isEmpty()) {
         config->beginGroup("Paths");
         retval = config->value(key, defaultValue).toString();

         // expand environment variables in the form $(ENVVAR)
         QRegularExpression8 reg_var("\\$\\((.*?)\\)");

         QRegularExpressionMatch8 match = reg_var.match(retval);

         while (match.hasMatch()) {
            QString newStr = QString::fromUtf8( qgetenv(match.captured(1).toLatin1().constData()) );

            retval.replace(match.capturedStart(0), match.capturedEnd(0), newStr);
            match = reg_var.match(retval);
         }

         config->endGroup();
      }

   }  else {
      // no configuration file, use defaults

      switch (loc) {

         case PluginsPath:
            retval = "plugins";
            break;

         case ImportsPath:
            retval = "imports";
            break;

         case Qml2ImportsPath:
            retval = "qml";
            break;

         case TranslationsPath:
            retval = "translations";
            break;

         case SettingsPath:
            // key = "Settings";
            break;

         default:
            break;
      }
   }

   return retval;
}

QSettings *QLibraryInfo::qt_library_settings()
{
   static QScopedPointer<QSettings> settings(QLibraryInfo::findConfiguration());
   return settings.data();
}

#endif // QT_NO_SETTINGS

Q_CORE_EXPORT void cs_print_build_info()
{
   QDate build_Date = QLibraryInfo::buildDate();

   printf("CopperSpice Build Information: \n"
          "   Version:          %s\n"
          "   Build Date:       %s\n"
          "   Install Prefix:   %s\n"
          "   Built For:        %s\n",
          CS_VERSION_STR,
          build_Date.toString("MM/dd/yyyy").toLatin1().constData(),
          CsLibraryInfo::install_prefix,
          CsLibraryInfo::built_for);

   fflush(stdout);
}

