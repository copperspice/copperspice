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

#include <qmediastoragelocation_p.h>

#include <qstandardpaths.h>

QMediaStorageLocation::QMediaStorageLocation()
{
}

void QMediaStorageLocation::addStorageLocation(MediaType type, const QString &location)
{
    m_customLocations[type].append(location);
}

QDir QMediaStorageLocation::defaultLocation(MediaType type) const
{
    QStringList dirCandidates = {m_customLocations.value(type)};

    switch (type) {
       case Movies:
           dirCandidates << QStandardPaths::writableLocation(QStandardPaths::MoviesLocation);
           break;

       case Music:
           dirCandidates << QStandardPaths::writableLocation(QStandardPaths::MusicLocation);
           break;

       case Pictures:
           dirCandidates << QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
           break;

       default:
           break;
    }

    dirCandidates << QDir::homePath();
    dirCandidates << QDir::currentPath();
    dirCandidates << QDir::tempPath();

    for (const QString &path : dirCandidates) {
        if (QFileInfo(path).isWritable())
            return QDir(path);
    }

    return QDir();
}

QString QMediaStorageLocation::generateFileName(const QString &requestedName,
            MediaType type, const QString &prefix, const QString &extension) const
{
    if (requestedName.isEmpty())
        return generateFileName(prefix, defaultLocation(type), extension);

    QString path = requestedName;

    if (QFileInfo(path).isRelative())
        path = defaultLocation(type).absoluteFilePath(path);

    if (QFileInfo(path).isDir())
        return generateFileName(prefix, QDir(path), extension);

    if (! path.endsWith(extension))
        path.append(QString(".%1").formatArg(extension));

    return path;
}

QString QMediaStorageLocation::generateFileName(const QString &prefix, const QDir &dir, const QString &extension) const
{
    QMutexLocker lock(&m_mutex);

    const QString lastMediaKey = dir.absolutePath() + ' ' + prefix + ' ' + extension;
    qint64 lastMediaIndex = m_lastUsedIndex.value(lastMediaKey, 0);

    if (lastMediaIndex == 0) {
        // first run, find the maximum media number during the fist capture

        for (const QString &fileName : dir.entryList(QStringList() << QString("%1*.%2").formatArg(prefix).formatArg(extension))) {
            const qint64 mediaIndex = fileName.mid(prefix.length(), fileName.size() - prefix.length() - extension.length() - 1).toInteger<qint64>();
            lastMediaIndex = qMax(lastMediaIndex, mediaIndex);
        }
    }

    // do not just rely on cached lastMediaIndex value since
    // someone else may create a file after camera started
    while (true) {
        const QString name = QString("%1%2.%3").formatArg(prefix).formatArg(lastMediaIndex + 1, 8, 10, '0').formatArg(extension);
        const QString path = dir.absoluteFilePath(name);

        if (! QFileInfo(path).exists()) {
            m_lastUsedIndex[lastMediaKey] = lastMediaIndex + 1;
            return path;
        }

        lastMediaIndex++;
    }

    return QString();
}

