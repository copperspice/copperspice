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

#include <avfstoragelocation.h>
#include <qstandardpaths.h>

AVFStorageLocation::AVFStorageLocation()
{
}

AVFStorageLocation::~AVFStorageLocation()
{
}

/*!
 * Generate the actual file name from user requested one.
 * requestedName may be either empty (the default dir and naming theme is used),
 * points to existing dir (the default name used)
 * or specify the full actual path.
 */
QString AVFStorageLocation::generateFileName(const QString &requestedName, QCamera::CaptureMode mode,
      const QString &prefix, const QString &ext) const
{
    if (requestedName.isEmpty())
        return generateFileName(prefix, defaultDir(mode), ext);

    if (QFileInfo(requestedName).isDir())
        return generateFileName(prefix, QDir(requestedName), ext);

    return requestedName;
}

QDir AVFStorageLocation::defaultDir(QCamera::CaptureMode mode) const
{
    QStringList dirCandidates;

    if (mode == QCamera::CaptureVideo) {
        dirCandidates << QStandardPaths::writableLocation(QStandardPaths::MoviesLocation);
    } else {
        dirCandidates << QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    }

    dirCandidates << QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    dirCandidates << QDir::homePath();
    dirCandidates << QDir::currentPath();
    dirCandidates << QDir::tempPath();

    for (const QString &path : dirCandidates) {
        if (QFileInfo(path).isWritable())
            return QDir(path);
    }

    return QDir();
}

QString AVFStorageLocation::generateFileName(const QString &prefix, const QDir &dir, const QString &ext) const
{
    QString lastClipKey = dir.absolutePath() + ' ' + prefix + ' ' + ext;
    int lastClip = m_lastUsedIndex.value(lastClipKey, 0);

    if (lastClip == 0) {
        //first run, find the maximum clip number during the fist capture
        for (const QString &fileName : dir.entryList(QStringList() << QString("%1*.%2").formatArg(prefix).formatArg(ext))) {
            int imgNumber = fileName.mid(prefix.length(), fileName.size()-prefix.length()-ext.length()-1).toInteger<int>();
            lastClip = qMax(lastClip, imgNumber);
        }
    }


    // do not just rely on cached lastClip value,
    // someone else may create a file after camera started
    while (true) {
        QString name = QString("%1%2.%3").formatArg(prefix).formatArg(lastClip+1, 4, 10, QChar('0')).formatArg(ext);
        QString path = dir.absoluteFilePath(name);

        if (! QFileInfo(path).exists()) {
            m_lastUsedIndex[lastClipKey] = lastClip+1;
            return path;
        }

        lastClip++;
    }

    return QString();
}
