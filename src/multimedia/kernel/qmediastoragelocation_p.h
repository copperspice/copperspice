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

#ifndef QMEDIASTORAGELOCATION_H
#define QMEDIASTORAGELOCATION_H

#include <qdir.h>
#include <qmap.h>
#include <qhash.h>
#include <qmutex.h>

class Q_MULTIMEDIA_EXPORT QMediaStorageLocation
{
 public:
    enum MediaType {
        Movies,
        Music,
        Pictures,
        Sounds
    };

    QMediaStorageLocation();

    void addStorageLocation(MediaType type, const QString &location);
    QDir defaultLocation(MediaType type) const;

    QString generateFileName(const QString &requestedName, MediaType type, const QString &prefix, const QString &extension) const;
    QString generateFileName(const QString &prefix, const QDir &dir, const QString &extension) const;

 private:
    mutable QMutex m_mutex;
    mutable QHash<QString, qint64> m_lastUsedIndex;
    QMap<MediaType, QStringList> m_customLocations;
};

#endif
