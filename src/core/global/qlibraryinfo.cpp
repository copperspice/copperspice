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

#include <qcoreapplication.h>
#include <qdir.h>
#include <qfile.h>
#include <qlibraryinfo.h>
#include <qregularexpression.h>
#include <qscopedpointer.h>
#include <qsettings.h>
#include <qstringlist.h>

#include <cs_build_info.h>

#ifdef Q_OS_DARWIN
#  include <qcore_mac_p.h>
#endif

#ifndef QT_NO_SETTINGS

static const char platformsSection[] = "Platforms";

struct QLibrarySettings
{
    QLibrarySettings();
    void load();

    QScopedPointer<QSettings> settings;
    bool reloadOnQAppAvailable;
};

static QLibrarySettings *qt_library_settings()
{
   static QLibrarySettings retval;
   return &retval;
}

class QLibraryInfoPrivate
{
public:
    static QSettings *findConfiguration();

    static QSettings *configuration()
    {
        QLibrarySettings *ls = qt_library_settings();

        if (ls) {
            if (ls->reloadOnQAppAvailable && QCoreApplication::instance() != nullptr) {
                ls->load();
            }

            return ls->settings.data();

        } else {
            return nullptr;
        }
    }
};

QLibrarySettings::QLibrarySettings()
{
   load();
}

void QLibrarySettings::load()
{
   settings.reset(QLibraryInfoPrivate::findConfiguration());
   reloadOnQAppAvailable = (settings.data() == nullptr && QCoreApplication::instance() == nullptr);

   bool haveDevicePaths;
   bool haveEffectivePaths;
   bool havePaths;

   if (settings != nullptr) {

     QStringList children = settings->childGroups();
     haveDevicePaths = children.contains("DevicePaths");

     // EffectiveSourcePaths is for the build only
     bool haveEffectiveSourcePaths = false;
     haveEffectivePaths = haveEffectiveSourcePaths || children.contains("EffectivePaths");

     // an existing but empty file claimed to contain the Paths section
     havePaths = (! haveDevicePaths && ! haveEffectivePaths
                  && ! children.contains(platformsSection)) || children.contains("Paths");

     if (! havePaths) {
        settings.reset(nullptr);
     }
   }
}

QSettings *QLibraryInfoPrivate::findConfiguration()
{
   QString qtconfig(":/cs/etc/cs.conf");

   if (! QFile::exists(qtconfig) && QCoreApplication::instance()) {

#ifdef Q_OS_DARWIN
      CFBundleRef bundleRef = CFBundleGetMainBundle();

      if (bundleRef) {
         // locates the cs.conf file in foo.app/Contents/Resources
         QCFType<CFURLRef> urlRef = CFBundleCopyResourceURL(bundleRef, QCFString("cs.conf").toCFStringRef(), nullptr, nullptr);

         if (urlRef) {
            QCFString path = CFURLCopyFileSystemPath(urlRef, kCFURLPOSIXPathStyle);
            qtconfig = QDir::cleanPath(path.toQString());
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
      QSettings *tmp = new QSettings(qtconfig, QSettings::IniFormat);
      return tmp;
   }

   return nullptr;     // no luck
}

#endif // QT_NO_SETTINGS

QLibraryInfo::QLibraryInfo()
{
}

QDate QLibraryInfo::buildDate()
{
   return QDate::fromString(QString::fromLatin1(BUILD_DATE), Qt::ISODate);
}

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
   QSettings *config = nullptr;

#if ! defined(QT_NO_SETTINGS)
   // raw pointer to a QSettings object, this is a ptr to the cs.conf data
   config = QLibraryInfoPrivate::configuration();
#endif

   if (config != nullptr) {
      QString key;
      QString defaultValue;

      switch (loc) {

         case PrefixPath:
            key = "Prefix";
            defaultValue = ".";
            break;

         case PluginsPath:
            key = "Plugins";
            defaultValue = "plugins";
            break;

         case ImportsPath:
            key = "Imports";
            defaultValue = "imports";
            break;

         case Qml2ImportsPath:
            key = "Qml2Imports";
            defaultValue = "qml";
            break;

         case TranslationsPath:
            key = "Translations";
            defaultValue = "translations";
            break;

         case SettingsPath:
            key = "Settings";
            defaultValue = ".";
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
      // no cs.conf file just use default values

      switch (loc) {

         case PrefixPath:
            retval = ".";
            break;

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
            retval = "settings";
            break;

         default:
            break;
      }
   }

   if (! retval.isEmpty() && QDir::isRelativePath(retval)){
      QString baseDir;

      if (loc == PrefixPath) {
         if (QCoreApplication::instance())  {

#ifdef Q_OS_DARWIN
            CFBundleRef bundleRef = CFBundleGetMainBundle();

            if (bundleRef) {
               QCFType<CFURLRef> urlRef = CFBundleCopyBundleURL(bundleRef);

               if (urlRef) {
                  QCFString path = CFURLCopyFileSystemPath(urlRef, kCFURLPOSIXPathStyle);

                  QString bundleContentsDir = QString(path.toQString()) + "/Contents/";

                  if (QDir(bundleContentsDir).exists()) {
                     return QDir::cleanPath(bundleContentsDir + retval);
                  }
               }
            }
#endif
            // make the prefix path absolute to the exe directory
            baseDir = QCoreApplication::applicationDirPath();

         } else {
            baseDir = QDir::currentPath();
         }

      } else {
         // make any other path absolute to the prefix directory
         baseDir = location(PrefixPath);

      }

      retval = QDir::cleanPath(baseDir + '/' + retval);
   }

   return retval;
}

QStringList QLibraryInfo::platformPluginArguments(const QString &platformName)
{
#if ! defined(QT_NO_SETTINGS)
    QScopedPointer<const QSettings> settings(QLibraryInfoPrivate::findConfiguration());

    if (! settings.isNull()) {
        QString key = QString(platformsSection) + "/" + platformName + "Arguments";

        return settings->value(key).toStringList();
    }
#endif

    return QStringList();
}

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
