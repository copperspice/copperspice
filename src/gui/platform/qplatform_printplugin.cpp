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

#include "qplatform_printplugin.h"
#include "qplatform_printersupport.h"

#include <qprinterinfo.h>
#include <qfactoryloader_p.h>
#include <qcoreapplication.h>

static QFactoryLoader *loader()
{
   static QFactoryLoader retval(QPlatformPrinterSupportPlugin_ID, "/printerdrivers", Qt::CaseInsensitive);
   return &retval;
}

QPlatformPrinterSupportPlugin::QPlatformPrinterSupportPlugin(QObject *parent)
    : QObject(parent)
{
}

QPlatformPrinterSupportPlugin::~QPlatformPrinterSupportPlugin()
{
}

static QPlatformPrinterSupport *printerSupport = nullptr;

static void cleanupPrinterSupport()
{
#ifndef QT_NO_PRINTER
    delete printerSupport;
#endif

    printerSupport = nullptr;
}

QPlatformPrinterSupport *QPlatformPrinterSupportPlugin::get()
{
    if (! printerSupport) {
        QFactoryLoader *factoryObj = loader();

        // what keys are available
        auto keySet = factoryObj->keySet();

        if (! keySet.isEmpty()) {
            printerSupport = cs_load_plugin<QPlatformPrinterSupport, QPlatformPrinterSupportPlugin>(factoryObj, *(keySet.constBegin()));
        }

        if (printerSupport) {
            qAddPostRoutine(cleanupPrinterSupport);
        }
    }

    return printerSupport;
}

