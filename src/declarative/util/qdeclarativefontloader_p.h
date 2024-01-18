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

#ifndef QDECLARATIVEFONTLOADER_P_H
#define QDECLARATIVEFONTLOADER_P_H

#include <qdeclarative.h>
#include <QtCore/qobject.h>
#include <QtCore/qurl.h>

QT_BEGIN_NAMESPACE

class QDeclarativeFontLoaderPrivate;
class QDeclarativeFontLoader : public QObject
{
   DECL_CS_OBJECT(QDeclarativeFontLoader)
   Q_DECLARE_PRIVATE(QDeclarativeFontLoader)

   CS_ENUM(Status)

   DECL_CS_PROPERTY_READ(source, source)
   DECL_CS_PROPERTY_WRITE(source, setSource)
   DECL_CS_PROPERTY_NOTIFY(source, sourceChanged)
   DECL_CS_PROPERTY_READ(name, name)
   DECL_CS_PROPERTY_WRITE(name, setName)
   DECL_CS_PROPERTY_NOTIFY(name, nameChanged)
   DECL_CS_PROPERTY_READ(status, status)
   DECL_CS_PROPERTY_NOTIFY(status, statusChanged)

 public:
   enum Status { Null = 0, Ready, Loading, Error };

   QDeclarativeFontLoader(QObject *parent = nullptr);
   ~QDeclarativeFontLoader();

   QUrl source() const;
   void setSource(const QUrl &url);

   QString name() const;
   void setName(const QString &name);

   Status status() const;

 private :
   DECL_CS_SLOT_1(Private, void updateFontInfo(const QString &un_named_arg1, QDeclarativeFontLoader::Status un_named_arg2))
   DECL_CS_SLOT_2(updateFontInfo)

 public:
   DECL_CS_SIGNAL_1(Public, void sourceChanged())
   DECL_CS_SIGNAL_2(sourceChanged)
   DECL_CS_SIGNAL_1(Public, void nameChanged())
   DECL_CS_SIGNAL_2(nameChanged)
   DECL_CS_SIGNAL_1(Public, void statusChanged())
   DECL_CS_SIGNAL_2(statusChanged)
};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QDeclarativeFontLoader)

#endif // QDECLARATIVEFONTLOADER_H

