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

QT_BEGIN_NAMESPACE

#ifndef QT_NO_LIBRARY
Q_GLOBAL_STATIC_WITH_ARGS(QFactoryLoader, loader,
    (QPlatformPrinterSupportFactoryInterface_iid, QLatin1String("/printsupport"), Qt::CaseInsensitive))
#endif

QPlatformPrinterSupportPlugin::QPlatformPrinterSupportPlugin(QObject *parent)
    : QObject(parent)
{
}

QPlatformPrinterSupportPlugin::~QPlatformPrinterSupportPlugin()
{
}

static QPlatformPrinterSupport *printerSupport = 0;

#ifndef QT_NO_LIBRARY
static void cleanupPrinterSupport()
{
#ifndef QT_NO_PRINTER
    delete printerSupport;
#endif
    printerSupport = 0;
}
#endif // !QT_NO_LIBRARY

/*!
    \internal

    Returns a lazily-initialized singleton. Ownership is granted to the
    QPlatformPrinterSupportPlugin, which is never unloaded or destroyed until
    application exit, i.e. you can expect this pointer to always be valid and
    multiple calls to this function will always return the same pointer.
*/
QPlatformPrinterSupport *QPlatformPrinterSupportPlugin::get()
{
#ifndef QT_NO_LIBRARY
    if (!printerSupport) {
        const QMultiMap<int, QString> keyMap = loader()->keyMap();
        if (!keyMap.isEmpty())
            printerSupport = qLoadPlugin<QPlatformPrinterSupport, QPlatformPrinterSupportPlugin>(loader(), keyMap.constBegin().value());
        if (printerSupport)
            qAddPostRoutine(cleanupPrinterSupport);
    }
#endif // !QT_NO_LIBRARY
    return printerSupport;
}

QT_END_NAMESPACE
