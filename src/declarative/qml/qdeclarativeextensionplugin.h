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

#ifndef QDECLARATIVEEXTENSIONPLUGIN_H
#define QDECLARATIVEEXTENSIONPLUGIN_H

#include <QtCore/qplugin.h>
#include <QtDeclarative/qdeclarativeextensioninterface.h>

QT_BEGIN_NAMESPACE

class QDeclarativeEngine;

class Q_DECLARATIVE_EXPORT QDeclarativeExtensionPlugin : public QObject, public QDeclarativeExtensionInterface
{
   DECL_CS_OBJECT(QDeclarativeExtensionPlugin)
   CS_INTERFACES(QDeclarativeExtensionInterface)
 public:
   explicit QDeclarativeExtensionPlugin(QObject *parent = nullptr);
   ~QDeclarativeExtensionPlugin();

   virtual void registerTypes(const char *uri) = 0;
   virtual void initializeEngine(QDeclarativeEngine *engine, const char *uri);

 private:
   Q_DISABLE_COPY(QDeclarativeExtensionPlugin)
};

QT_END_NAMESPACE

#endif // QDECLARATIVEEXTENSIONPLUGIN_H
