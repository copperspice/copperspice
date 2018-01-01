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

#ifndef PROXYSETTINGS_H
#define PROXYSETTINGS_H

#include <QDialog>
#include <QNetworkProxy>
#ifdef Q_WS_MAEMO_5
#include "ui_proxysettings_maemo5.h"
#else
#include "ui_proxysettings.h"
#endif

QT_BEGIN_NAMESPACE
/**
*/
class ProxySettings : public QDialog, public Ui::ProxySettings
{

Q_OBJECT

public:
    ProxySettings(QWidget * parent = 0);

    ~ProxySettings();

    static QNetworkProxy httpProxy ();
    static bool httpProxyInUse ();

public slots:
    virtual void accept ();
};

QT_END_NAMESPACE

#endif // PROXYSETTINGS_H
