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

#ifndef QTEXTCODECPLUGIN_H
#define QTEXTCODECPLUGIN_H

#include <qbytearray.h>
#include <qfactoryinterface.h>
#include <qlist.h>
#include <qplugin.h>

#ifndef QT_NO_TEXTCODECPLUGIN

class QTextCodec;

struct Q_CORE_EXPORT QTextCodecFactoryInterface : public QFactoryInterface {
   virtual QTextCodec *create(const QString &key) = 0;
};

#define QTextCodecInterface_ID "com.copperspice.CS.QTextCodecInterface"
CS_DECLARE_INTERFACE(QTextCodecFactoryInterface, QTextCodecInterface_ID)

class Q_CORE_EXPORT QTextCodecPlugin : public QObject, public QTextCodecFactoryInterface
{
   CORE_CS_OBJECT_MULTIPLE(QTextCodecPlugin, QObject)

   CS_INTERFACES(QTextCodecFactoryInterface, QFactoryInterface)

 public:
   explicit QTextCodecPlugin(QObject *parent = nullptr);
   ~QTextCodecPlugin();

   virtual QStringList names() const = 0;
   virtual QStringList aliases() const = 0;
   virtual QTextCodec *createForName(const QString &name) = 0;

   virtual QList<int> mibEnums() const = 0;
   virtual QTextCodec *createForMib(int mib) = 0;

 private:
   QStringList keys() const override;
   QTextCodec *create(const QString &name) override;
};

#endif // QT_NO_TEXTCODECPLUGIN

#endif // QTEXTCODECPLUGIN_H
