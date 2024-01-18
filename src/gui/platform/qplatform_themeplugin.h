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

#ifndef QPLATFORM_THEMEPLUGIN_H
#define QPLATFORM_THEMEPLUGIN_H

#include <qplugin.h>
#include <qfactoryinterface.h>

class QPlatformTheme;

#define QPlatformThemeInterface_ID "com.copperspice.CS.QPlatformThemeInterface"

class Q_GUI_EXPORT QPlatformThemePlugin : public QObject
{
    GUI_CS_OBJECT(QPlatformThemePlugin)

public:
    explicit QPlatformThemePlugin(QObject *parent = nullptr);
    ~QPlatformThemePlugin();

    virtual QPlatformTheme *create(const QString &key, const QStringList &paramList) = 0;
};

#endif
