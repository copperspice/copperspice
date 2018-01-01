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

#include <QIntValidator>
#include <QSettings>

#include "proxysettings.h"

QT_BEGIN_NAMESPACE

ProxySettings::ProxySettings (QWidget * parent)
    : QDialog (parent), Ui::ProxySettings()
{
    setupUi (this);

#if !defined Q_WS_MAEMO_5
    // the onscreen keyboard can't cope with masks
    proxyServerEdit->setInputMask(QLatin1String("000.000.000.000;_"));
#endif
    QIntValidator *validator = new QIntValidator (0, 9999, this);
    proxyPortEdit->setValidator(validator);

    QSettings settings;
    proxyCheckBox->setChecked(settings.value(QLatin1String("http_proxy/use"), 0).toBool());
    proxyServerEdit->insert(settings.value(QLatin1String("http_proxy/hostname")).toString());
    proxyPortEdit->insert(settings.value(QLatin1String("http_proxy/port"), QLatin1String("80")).toString ());
    usernameEdit->insert(settings.value(QLatin1String("http_proxy/username")).toString ());
    passwordEdit->insert(settings.value(QLatin1String("http_proxy/password")).toString ());
}

ProxySettings::~ProxySettings()
{
}

void ProxySettings::accept ()
{
    QSettings settings;

    settings.setValue(QLatin1String("http_proxy/use"), proxyCheckBox->isChecked());
    settings.setValue(QLatin1String("http_proxy/hostname"), proxyServerEdit->text());
    settings.setValue(QLatin1String("http_proxy/port"), proxyPortEdit->text());
    settings.setValue(QLatin1String("http_proxy/username"), usernameEdit->text());
    settings.setValue(QLatin1String("http_proxy/password"), passwordEdit->text());

    QDialog::accept ();
}

QNetworkProxy ProxySettings::httpProxy ()
{
    QSettings settings;
    QNetworkProxy proxy;

    bool proxyInUse = settings.value(QLatin1String("http_proxy/use"), 0).toBool();
    if (proxyInUse) {
        proxy.setType (QNetworkProxy::HttpProxy);
        proxy.setHostName (settings.value(QLatin1String("http_proxy/hostname")).toString());// "192.168.220.5"
        proxy.setPort (settings.value(QLatin1String("http_proxy/port"), 80).toInt());  // 8080
        proxy.setUser (settings.value(QLatin1String("http_proxy/username")).toString());
        proxy.setPassword (settings.value(QLatin1String("http_proxy/password")).toString());
        //QNetworkProxy::setApplicationProxy (proxy);
    }
    else {
        proxy.setType (QNetworkProxy::NoProxy);
    }
    return proxy;
}

bool ProxySettings::httpProxyInUse()
{
    QSettings settings;
    return settings.value(QLatin1String("http_proxy/use"), 0).toBool();
}

QT_END_NAMESPACE
