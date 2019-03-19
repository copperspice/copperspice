/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

#ifndef QLIBRARYINFO_H
#define QLIBRARYINFO_H

#include <qstring8.h>
#include <qdate.h>
#include <qsettings.h>

#ifndef QT_NO_SETTINGS

class Q_CORE_EXPORT QLibraryInfo
{
 public:
   static QString licensee();
   static QString licensedProducts();

#ifndef QT_NO_DATESTRING
   static QDate buildDate();
#endif

   enum LibraryLocation {
      PluginsPath,
      ImportsPath,
      Qml2ImportsPath,
      TranslationsPath,
      SettingsPath,
   };
   static QString location(LibraryLocation);

 private:
   QLibraryInfo();

   static QSettings *configuration();
   static QSettings *findConfiguration();
   static QSettings *qt_library_settings();
};

#endif /* QT_NO_SETTINGS */

Q_CORE_EXPORT void cs_print_build_info();

#endif
