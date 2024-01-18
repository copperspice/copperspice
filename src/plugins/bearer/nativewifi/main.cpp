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

#include <qnativewifiengine.h>
#include <platformdefs.h>

#include <qsystemlibrary_p.h>
#include <qbearerplugin_p.h>

#ifndef QT_NO_BEARERMANAGEMENT

QT_BEGIN_NAMESPACE

static void resolveLibrary()
{
    static bool triedResolve = false;
    if (!triedResolve) {
        QSystemLibrary wlanapi(QLatin1String("wlanapi"));
        if (wlanapi.load()) {
            local_WlanOpenHandle = (WlanOpenHandleProto)
                wlanapi.resolve("WlanOpenHandle");
            local_WlanRegisterNotification = (WlanRegisterNotificationProto)
                wlanapi.resolve("WlanRegisterNotification");
            local_WlanEnumInterfaces = (WlanEnumInterfacesProto)
                wlanapi.resolve("WlanEnumInterfaces");
            local_WlanGetAvailableNetworkList = (WlanGetAvailableNetworkListProto)
                wlanapi.resolve("WlanGetAvailableNetworkList");
            local_WlanQueryInterface = (WlanQueryInterfaceProto)
                wlanapi.resolve("WlanQueryInterface");
            local_WlanConnect = (WlanConnectProto)
                wlanapi.resolve("WlanConnect");
            local_WlanDisconnect = (WlanDisconnectProto)
                wlanapi.resolve("WlanDisconnect");
            local_WlanScan = (WlanScanProto)
                wlanapi.resolve("WlanScan");
            local_WlanFreeMemory = (WlanFreeMemoryProto)
                wlanapi.resolve("WlanFreeMemory");
            local_WlanCloseHandle = (WlanCloseHandleProto)
                wlanapi.resolve("WlanCloseHandle");
        }
        triedResolve = true;
    }
}

class QNativeWifiEnginePlugin : public QBearerEnginePlugin
{
public:
    QNativeWifiEnginePlugin();
    ~QNativeWifiEnginePlugin();

    QStringList keys() const;
    QBearerEngine *create(const QString &key) const;
};

QNativeWifiEnginePlugin::QNativeWifiEnginePlugin()
{
}

QNativeWifiEnginePlugin::~QNativeWifiEnginePlugin()
{
}

QStringList QNativeWifiEnginePlugin::keys() const
{
    return QStringList() << QLatin1String("nativewifi");
}

QBearerEngine *QNativeWifiEnginePlugin::create(const QString &key) const
{
    if (key != QLatin1String("nativewifi"))
        return 0;

    resolveLibrary();

    // native wifi dll not available
    if (!local_WlanOpenHandle)
        return 0;

    QNativeWifiEngine *engine = new QNativeWifiEngine;

    // could not initialise subsystem
    if (engine && !engine->available()) {
        delete engine;
        return 0;
    }

    return engine;
}

Q_EXPORT_STATIC_PLUGIN(QNativeWifiEnginePlugin)
Q_EXPORT_PLUGIN2(qnativewifibearer, QNativeWifiEnginePlugin)

QT_END_NAMESPACE

#endif // QT_NO_BEARERMANAGEMENT
