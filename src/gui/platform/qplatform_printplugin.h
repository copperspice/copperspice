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

#ifndef QPLATFORMPRINTPLUGIN_H
#define QPLATFORMPRINTPLUGIN_H

#include <qplugin.h>
#include <qfactoryinterface.h>

class QPlatformPrinterSupport;

#define QPlatformPrinterSupportPlugin_ID "com.copperspice.CS.QPlatformPrinterSupportInterface"

class Q_GUI_EXPORT QPlatformPrinterSupportPlugin : public QObject
{
    GUI_CS_OBJECT(QPlatformPrinterSupportPlugin)

public:
    explicit QPlatformPrinterSupportPlugin(QObject *parent = nullptr);
    ~QPlatformPrinterSupportPlugin();

    virtual QPlatformPrinterSupport *create(const QString &key) = 0;

    static QPlatformPrinterSupport *get();
};

#endif
