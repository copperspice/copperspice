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

#define Q_UUIDIMPL

#include <qsqldriverplugin.h>
#include <qstringlist.h>

#ifdef Q_OS_WIN32

// assume MS SQL Server is used, set Q_USE_SYBASE to force Sybase.
#include <qsql_tds.h>

// Conflicting declarations of LPCBYTE in sqlfront.h and winscard.h
#define _WINSCARD_H_
#include <windows.h>
#endif

class QTDSDriverPlugin : public QSqlDriverPlugin
{
   CS_OBJECT(QTDSDriverPlugin)

   CS_PLUGIN_IID(QSqlDriverInterface_ID)
   CS_PLUGIN_KEY("QTDS")

 public:
   QTDSDriverPlugin();

   QSqlDriver *create(const QString &) override;
};

CS_PLUGIN_REGISTER(QTDSDriverPlugin)

QTDSDriverPlugin::QTDSDriverPlugin()
   : QSqlDriverPlugin()
{
}

QSqlDriver *QTDSDriverPlugin::create(const QString &name)
{
   if (name == "QTDS") {
      QTDSDriver *driver = new QTDSDriver();
      return driver;
   }

   return nullptr;
}

