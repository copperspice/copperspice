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

#include "qplatform_printplugin.h"
#include "qplatform_printersupport.h"

#include "qprinterinfo.h"
#include "qfactoryloader_p.h"
#include <qcoreapplication.h>

Q_GLOBAL_STATIC_WITH_ARGS(QFactoryLoader, loader, (QPlatformPrinterSupportFactoryInterface_iid, "/printsupport", Qt::CaseInsensitive))

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

    printerSupport = 0;
}

/*!
    \internal

    Returns a lazily-initialized singleton. Ownership is granted to the
    QPlatformPrinterSupportPlugin, which is never unloaded or destroyed until
    application exit, i.e. you can expect this pointer to always be valid and
    multiple calls to this function will always return the same pointer.
*/
QPlatformPrinterSupport *QPlatformPrinterSupportPlugin::get()
{
    if (! printerSupport) {
        auto keySet = loader()->keySet();

        if (! keySet.isEmpty()) {
            printerSupport = cs_load_plugin<QPlatformPrinterSupport, QPlatformPrinterSupportPlugin>(loader(), *(keySet.constBegin()));
        }

        if (printerSupport) {
            qAddPostRoutine(cleanupPrinterSupport);
        }
    }

    return printerSupport;
}

