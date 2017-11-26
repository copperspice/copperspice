/***********************************************************************
*
* Copyright (c) 2012-2017 Barbara Geller
* Copyright (c) 2012-2017 Ansel Sermersheim
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
#include <cs_build_info.h>

#ifdef Q_OS_MAC
#  include "qcore_mac_p.h"
#endif

QT_BEGIN_NAMESPACE

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
   QString qtconfig = QLatin1String(":/cs/etc/cs.conf");

   if (! QFile::exists(qtconfig) && QCoreApplication::instance()) {

#ifdef Q_OS_MAC
      CFBundleRef bundleRef = CFBundleGetMainBundle();

      if (bundleRef) {
         QCFType<CFURLRef> urlRef = CFBundleCopyResourceURL(bundleRef, QCFString(QLatin1String("cs.conf")), 0, 0);

         if (urlRef) {
            QCFString path = CFURLCopyFileSystemPath(urlRef, kCFURLPOSIXPathStyle);
            qtconfig = QDir::cleanPath(path);
         }
      }

      if (qtconfig.isEmpty())
#endif
      {
         QDir pwd(QCoreApplication::applicationDirPath());
         qtconfig = pwd.filePath(QLatin1String("cs.conf"));
      }
   }

   if (QFile::exists(qtconfig)) {
      return new QSettings(qtconfig, QSettings::IniFormat);
   }

   return 0;     // no luck
}

QString QLibraryInfo::buildKey()
{
   return QString::fromLatin1(QT_BUILD_KEY);
}

#ifndef QT_NO_DATESTRING
QDate QLibraryInfo::buildDate()
{
   return QDate::fromString(QString::fromLatin1(BUILD_DATE), Qt::ISODate);
}
#endif

QString QLibraryInfo::licensee()
{
   const char *str = "Open Source";
   return QString::fromLocal8Bit(str);
}

QString QLibraryInfo::licensedProducts()
{
   const char *str = "Open Source";
   return QString::fromLatin1(str);
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
            key = QLatin1String("Plugins");
            defaultValue = QLatin1String("plugins");
            break;

         case ImportsPath:
            key = QLatin1String("Imports");
            defaultValue = QLatin1String("imports");
            break;

         case Qml2ImportsPath:
            QLatin1String("Qml2Imports");
            defaultValue = QLatin1String("qml");
            break;

         case TranslationsPath:
            key = QLatin1String("Translations");
            defaultValue = QLatin1String("translations");
            break;

         case SettingsPath:
            key = QLatin1String("Settings");
            break;

         default:
            break;
      }

      if (! key.isNull()) {
         config->beginGroup(QLatin1String("Paths"));

         retval = config->value(key, defaultValue).toString();

         // expand environment variables in the form $(ENVVAR)
         int rep;
         QRegExp reg_var(QLatin1String("\\$\\(.*\\)"));
         reg_var.setMinimal(true);

         while ((rep = reg_var.indexIn(retval)) != -1) {
            retval.replace(rep, reg_var.matchedLength(), QString::fromLocal8Bit(qgetenv(retval.mid(rep + 2,
                           reg_var.matchedLength() - 3).toLatin1().constData()).constData()));
         }

         config->endGroup();
      }

   }  else {
      // no configuration file, use defaults

      switch (loc) {

         case PluginsPath:
            retval = QLatin1String("plugins");
            break;

         case ImportsPath:
            retval = QLatin1String("imports");
            break;

         case Qml2ImportsPath:
            retval = QLatin1String("qml");
            break;

         case TranslationsPath:
            retval = QLatin1String("translations");
            break;

         case SettingsPath:
            // key = QLatin1String("Settings");
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

QT_END_NAMESPACE
