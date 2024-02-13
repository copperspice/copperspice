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

#include <qstringlist.h>
#include <qstringparser.h>
#include <qtextcodecplugin.h>

#ifndef QT_NO_TEXTCODECPLUGIN

QTextCodecPlugin::QTextCodecPlugin(QObject *parent)
   : QObject(parent)
{
}

QTextCodecPlugin::~QTextCodecPlugin()
{
}

QStringList QTextCodecPlugin::keys() const
{
   QStringList keys = names();
   keys += aliases();

   QList<int> mibs = mibEnums();

   for (int i = 0; i < mibs.count(); ++i) {
      keys += "MIB: " + QString::number(mibs.at(i));
   }

   return keys;
}

QTextCodec *QTextCodecPlugin::create(const QString &name)
{
   if (name.startsWith("MIB: ")) {
      return createForMib(name.mid(5).toInteger<int>());
   }

   return createForName(name);
}

#endif // QT_NO_TEXTCODECPLUGIN
