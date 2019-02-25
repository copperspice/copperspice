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

#ifndef QMEDIAPLUGINLOADER_H
#define QMEDIAPLUGINLOADER_H

#include <qobject.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qmap.h>

class QFactoryLoader;
class QMediaServiceProviderPlugin;

class Q_MULTIMEDIA_EXPORT QMediaPluginLoader
{
public:
    QMediaPluginLoader(const QString &iid, const QString &suffix = QString(), Qt::CaseSensitivity = Qt::CaseSensitive);

    ~QMediaPluginLoader();

    QStringList keys() const;
    QObject *instanceForKey(QString const &key);

    QList<QObject*> instances(const QString &key);

private:
    QString m_iid;
    QString m_location;

    QFactoryLoader *m_factoryLoader;
};

#endif
