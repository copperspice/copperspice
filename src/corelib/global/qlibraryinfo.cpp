/***********************************************************************
*
* Copyright (c) 2012-2014 Barbara Geller
* Copyright (c) 2012-2014 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or 
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see 
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#include "qdir.h"
#include "qfile.h"
#include "qconfig.h"
#include "qsettings.h"
#include "qlibraryinfo.h"
#include "qscopedpointer.h"
# include "qcoreapplication.h"

#ifdef Q_OS_MAC
#  include "qcore_mac_p.h"
#endif

QT_BEGIN_NAMESPACE

extern void qDumpCPUFeatures(); // in qsimd.cpp

#ifndef QT_NO_SETTINGS

struct QLibrarySettings
{
    QLibrarySettings();
    QScopedPointer<QSettings> settings;
};
Q_GLOBAL_STATIC(QLibrarySettings, qt_library_settings)

class QLibraryInfoPrivate
{
public:
    static QSettings *findConfiguration();
    static void cleanup()
    {
        QLibrarySettings *ls = qt_library_settings();
        if (ls)
            ls->settings.reset(0);
    }
    static QSettings *configuration()
    {
        QLibrarySettings *ls = qt_library_settings();
        return ls ? ls->settings.data() : 0;
    }
};

QLibrarySettings::QLibrarySettings()
    : settings(QLibraryInfoPrivate::findConfiguration())
{
    qAddPostRoutine(QLibraryInfoPrivate::cleanup);
}

QSettings *QLibraryInfoPrivate::findConfiguration()
{
    QString qtconfig = QLatin1String(":/cs/etc/cs.conf");

    if (!QFile::exists(qtconfig) && QCoreApplication::instance()) {

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

    if (QFile::exists(qtconfig))
        return new QSettings(qtconfig, QSettings::IniFormat);
    return 0;     //no luck
}


QLibraryInfo::QLibraryInfo()
{ }

QString QLibraryInfo::licensee()
{
   const char *str = "Open Source";
   return QString::fromLocal8Bit(str);
}

QString QLibraryInfo::licensedProducts()
{
   const char *str = "OpenSource";
   return QString::fromLatin1(str);
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

QString QLibraryInfo::location(LibraryLocation loc)
{
    QString ret;

    if(! QLibraryInfoPrivate::configuration()) {
        const char *path = 0;

        switch (loc) {

#ifdef QT_CONFIGURE_PREFIX_PATH
        // root dir for CopperSpice install 
        case PrefixPath:
            path = QT_CONFIGURE_PREFIX_PATH;
            break;
#endif

#ifdef QT_CONFIGURE_HEADERS_PATH
        // CS header files
        case HeadersPath:
            path = QT_CONFIGURE_HEADERS_PATH;
            break;
#endif

#ifdef QT_CONFIGURE_LIBRARIES_PATH
        // CS shared libraries 
        case LibrariesPath:
            path = QT_CONFIGURE_LIBRARIES_PATH;
            break;
#endif

#ifdef QT_CONFIGURE_BINARIES_PATH
        // uic, rcc 
        case BinariesPath:
            path = QT_CONFIGURE_BINARIES_PATH;
            break;
#endif

#ifdef QT_CONFIGURE_PLUGINS_PATH
        case PluginsPath:
            path = QT_CONFIGURE_PLUGINS_PATH;
            break;
#endif

#ifdef QT_CONFIGURE_IMPORTS_PATH
        case ImportsPath:
            path = QT_CONFIGURE_IMPORTS_PATH;
            break;
#endif

#ifdef QT_CONFIGURE_TRANSLATIONS_PATH
        case TranslationsPath:
            path = QT_CONFIGURE_TRANSLATIONS_PATH;
            break;
#endif

#ifdef QT_CONFIGURE_SETTINGS_PATH
        // cs.conf file 
        case SettingsPath:
            path = QT_CONFIGURE_SETTINGS_PATH;
            break;
#endif

        default:
            break;
        }

        if (path)
            ret = QString::fromLocal8Bit(path);

    } else {
        QString key;
        QString defaultValue;

        switch(loc) {

        case PrefixPath:
            key = QLatin1String("Prefix");
            break;
       
        case HeadersPath:
            key = QLatin1String("Headers");
            defaultValue = QLatin1String("include");
            break;

        case LibrariesPath:
            key = QLatin1String("Libraries");
            defaultValue = QLatin1String("lib");
            break;

        case BinariesPath:
            key = QLatin1String("Binaries");
            defaultValue = QLatin1String("bin");
            break;

        case PluginsPath:
            key = QLatin1String("Plugins");
            defaultValue = QLatin1String("plugins");
            break;

        case ImportsPath:
            key = QLatin1String("Imports");
            defaultValue = QLatin1String("imports");
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

        if(! key.isNull()) {
            QSettings *config = QLibraryInfoPrivate::configuration();
            config->beginGroup(QLatin1String("Paths"));

            ret = config->value(key, defaultValue).toString();

            // expand environment variables in the form $(ENVVAR)
            int rep;
            QRegExp reg_var(QLatin1String("\\$\\(.*\\)"));
            reg_var.setMinimal(true);

            while((rep = reg_var.indexIn(ret)) != -1) {
                ret.replace(rep, reg_var.matchedLength(),
                            QString::fromLocal8Bit(qgetenv(ret.mid(rep + 2,
                                reg_var.matchedLength() - 3).toLatin1().constData()).constData()));
            }

#ifdef QLIBRARYINFO_EPOCROOT
            // $${EPOCROOT} is a special case, resolve it similarly to qmake.
            QRegExp epocrootMatcher(QLatin1String("\\$\\$\\{EPOCROOT\\}"));
            if ((rep = epocrootMatcher.indexIn(ret)) != -1)
                ret.replace(rep, epocrootMatcher.matchedLength(), qt_epocRoot());
#endif
            config->endGroup();
        }
    }

    if (QDir::isRelativePath(ret)) {
        QString baseDir;
        if (loc == PrefixPath) {
            // we make the prefix path absolute to the executable's directory

            if (QCoreApplication::instance()) {
#ifdef Q_OS_MAC
                CFBundleRef bundleRef = CFBundleGetMainBundle();
                if (bundleRef) {
                    QCFType<CFURLRef> urlRef = CFBundleCopyBundleURL(bundleRef);
                    if (urlRef) {
                        QCFString path = CFURLCopyFileSystemPath(urlRef, kCFURLPOSIXPathStyle);
                        return QDir::cleanPath(QString(path) + QLatin1String("/Contents/") + ret);
                    }
                }
#endif
                baseDir = QCoreApplication::applicationDirPath();
            } else {
                baseDir = QDir::currentPath();
            }

        } else {
            // we make any other path absolute to the prefix directory
            baseDir = location(PrefixPath);
        }
        ret = QDir::cleanPath(baseDir + QLatin1Char('/') + ret);
    }
    return ret;
}

#endif // QT_NO_SETTINGS

QT_END_NAMESPACE

#if defined(Q_CC_GNU) && defined(ELF_INTERPRETER)
#  include <stdio.h>
#  include <stdlib.h>

extern const char qt_core_interpreter[] __attribute__((section(".interp"))) = ELF_INTERPRETER;

#endif
