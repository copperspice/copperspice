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

#include <qkbddriverplugin_qws.h>
#include <qkbdlinuxinput_qws.h>

QT_BEGIN_NAMESPACE

class QLinuxInputKbdDriver : public QKbdDriverPlugin
{
public:
    QLinuxInputKbdDriver();

    QStringList keys() const;
    QWSKeyboardHandler* create(const QString &driver, const QString &device);
};

QLinuxInputKbdDriver::QLinuxInputKbdDriver()
    : QKbdDriverPlugin()
{
}

QStringList QLinuxInputKbdDriver::keys() const
{
    return (QStringList() << QLatin1String("LinuxInput"));
}

QWSKeyboardHandler* QLinuxInputKbdDriver::create(const QString &driver,
                                                 const QString &device)
{
    Q_UNUSED(device);
    if (driver.compare(QLatin1String("LinuxInput"), Qt::CaseInsensitive))
        return 0;
    return new QWSLinuxInputKeyboardHandler(device);
}

Q_EXPORT_PLUGIN2(qwslinuxinputkbddriver, QLinuxInputKbdDriver)

QT_END_NAMESPACE
