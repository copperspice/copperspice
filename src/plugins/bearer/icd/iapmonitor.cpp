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

#include <QStringList>

#include <conn_settings.h>
#include "iapmonitor.h"

namespace Maemo {


void conn_settings_notify_func (ConnSettingsType type,
                                const char *id,
                                const char *key,
                                ConnSettingsValue *value,
                                void *user_data);

class IAPMonitorPrivate {
private:
    IAPMonitor *monitor;
    ConnSettings *settings;

public:

    IAPMonitorPrivate(IAPMonitor *monitor)
        : monitor(monitor)
    {
        settings = conn_settings_open(CONN_SETTINGS_CONNECTION, NULL);
        conn_settings_add_notify(
                        settings,
                        (ConnSettingsNotifyFunc *)conn_settings_notify_func,
                        this);
    }

    ~IAPMonitorPrivate()
    {
        conn_settings_del_notify(settings);
        conn_settings_close(settings);
    }

    void iapAdded(const QString &iap)
    {
        monitor->iapAdded(iap);
    }

    void iapRemoved(const QString &iap)
    {
        monitor->iapRemoved(iap);
    }
};

void conn_settings_notify_func (ConnSettingsType type,
                                const char *id,
                                const char *key,
                                ConnSettingsValue *value,
                                void *user_data)
{
    Q_UNUSED(id);

    if (type != CONN_SETTINGS_CONNECTION) return;
    IAPMonitorPrivate *priv = (IAPMonitorPrivate *)user_data;

    QString iapId(key);
    iapId = iapId.split("/")[0];
    if (value != 0) {
        priv->iapAdded(iapId);
    } else if (iapId == QString(key)) {
        // IAP is removed only when the directory gets removed
        priv->iapRemoved(iapId);
    }
}

IAPMonitor::IAPMonitor()
    : d_ptr(new IAPMonitorPrivate(this))
{
}

IAPMonitor::~IAPMonitor()
{
    delete d_ptr;
}

void IAPMonitor::iapAdded(const QString &id)
{
    Q_UNUSED(id);
    // By default do nothing
}

void IAPMonitor::iapRemoved(const QString &id)
{
    Q_UNUSED(id);
    // By default do nothing
}

} // namespace Maemo
