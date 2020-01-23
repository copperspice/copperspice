/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
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

#ifndef QDESKTOPSERVICES_H
#define QDESKTOPSERVICES_H

#include <qstring.h>
#include <qstandardpaths.h>

#ifndef QT_NO_DESKTOPSERVICES

class QStringList;
class QUrl;
class QObject;

class Q_GUI_EXPORT QDesktopServices
{
 public:
   static bool openUrl(const QUrl &url);
   static void setUrlHandler(const QString &scheme, QObject *receiver, const char *method);
   static void unsetUrlHandler(const QString &scheme);

 private:
   static QString storageLocationImpl(QStandardPaths::StandardLocation type);
};

#endif // QT_NO_DESKTOPSERVICES

#endif

