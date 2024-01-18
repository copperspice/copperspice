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

#ifndef QNMDBUSHELPERPRIVATE_H
#define QNMDBUSHELPERPRIVATE_H

#include <QDBusObjectPath>
#include <QDBusContext>
#include <QMap>

#ifndef QT_NO_DBUS

QT_BEGIN_NAMESPACE

class QNmDBusHelper: public QObject, protected QDBusContext
 {
     Q_OBJECT
 public:
    QNmDBusHelper(QObject *parent = nullptr);
    ~QNmDBusHelper();

 public slots:
    void deviceStateChanged(quint32);
    void slotAccessPointAdded( QDBusObjectPath );
    void slotAccessPointRemoved( QDBusObjectPath );
    void slotPropertiesChanged( QMap<QString,QVariant>);
    void slotSettingsRemoved();

Q_SIGNALS:
    void pathForStateChanged(const QString &, quint32);
    void pathForAccessPointAdded(const QString &,  QDBusObjectPath );
    void pathForAccessPointRemoved(const QString &,  QDBusObjectPath );
    void pathForPropertiesChanged(const QString &, QMap<QString,QVariant>);
    void pathForSettingsRemoved(const QString &);
};

QT_END_NAMESPACE

#endif // QT_NO_DBUS

#endif// QNMDBUSHELPERPRIVATE_H
