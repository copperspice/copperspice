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

#ifndef QICONENGINEPLUGIN_H
#define QICONENGINEPLUGIN_H

#include <qplugin.h>
#include <qfactoryinterface.h>

class QIconEngine;

#define QIconEngineInterface_ID "com.copperspice.CS.IconEngineInterface"

class Q_GUI_EXPORT QIconEnginePlugin : public QObject
{
   GUI_CS_OBJECT(QIconEnginePlugin)

 public:
   QIconEnginePlugin(QObject *parent = nullptr);
   ~QIconEnginePlugin();

   virtual QIconEngine *create(const QString &filename = QString()) = 0;
};

#endif