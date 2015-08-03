/***********************************************************************
*
* Copyright (c) 2012-2015 Barbara Geller
* Copyright (c) 2012-2015 Ansel Sermersheim
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

   CS_PROPERTY_READ(source, source)
   CS_PROPERTY_WRITE(source, setSource)
   CS_PROPERTY_NOTIFY(source, sourceChanged)
   CS_PROPERTY_READ(name, name)
   CS_PROPERTY_WRITE(name, setName)
   CS_PROPERTY_NOTIFY(name, nameChanged)
   CS_PROPERTY_READ(status, status)
   CS_PROPERTY_NOTIFY(status, statusChanged)

 public:
   enum Status { Null = 0, Ready, Loading, Error };

   QDeclarativeFontLoader(QObject *parent = 0);
   ~QDeclarativeFontLoader();

   QUrl source() const;
   void setSource(const QUrl &url);

   QString name() const;
   void setName(const QString &name);

   Status status() const;

 private :
   CS_SLOT_1(Private, void updateFontInfo(const QString &un_named_arg1, QDeclarativeFontLoader::Status un_named_arg2))
   CS_SLOT_2(updateFontInfo)

 public:
   CS_SIGNAL_1(Public, void sourceChanged())
   CS_SIGNAL_2(sourceChanged)
   CS_SIGNAL_1(Public, void nameChanged())
   CS_SIGNAL_2(nameChanged)
   CS_SIGNAL_1(Public, void statusChanged())
   CS_SIGNAL_2(statusChanged)
};

QT_END_NAMESPACE

QML_DECLARE_TYPE(QDeclarativeFontLoader)

#endif // QDECLARATIVEFONTLOADER_H

