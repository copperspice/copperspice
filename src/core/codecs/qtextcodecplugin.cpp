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

#include <qtextcodecplugin.h>
#include <qstringlist.h>
#include <qstringparser.h>

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
   QStringList keys;
   QList<QByteArray> list = names();
   list += aliases();

   for (int i = 0; i < list.size(); ++i) {
      keys += QString::fromLatin1(list.at(i));
   }

   QList<int> mibs = mibEnums();
   for (int i = 0; i < mibs.count(); ++i) {
      keys += "MIB: " + QString::number(mibs.at(i));
   }

   return keys;
}

QTextCodec *QTextCodecPlugin::create(const QString &name)
{
   if (name.startsWith("MIB: ")) {
      return createForMib(name.mid(4).toInteger<int>());
   }
   return createForName(name.toLatin1());
}

#endif // QT_NO_TEXTCODECPLUGIN
