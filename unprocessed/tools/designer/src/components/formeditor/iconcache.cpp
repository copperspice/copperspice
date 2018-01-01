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

#include "iconcache.h"
#include <QtGui/QPixmap>
#include <QtGui/QIcon>
#include <QtCore/QDebug>

QT_BEGIN_NAMESPACE

using namespace qdesigner_internal;

IconCache::IconCache(QObject *parent)
    : QDesignerIconCacheInterface(parent)
{
}

QIcon IconCache::nameToIcon(const QString &path, const QString &resourcePath)
{
    Q_UNUSED(path)
    Q_UNUSED(resourcePath)
    qWarning() << "IconCache::nameToIcon(): IconCache is obsoleted";
    return QIcon();
}

QString IconCache::iconToFilePath(const QIcon &pm) const
{
    Q_UNUSED(pm)
    qWarning() << "IconCache::iconToFilePath(): IconCache is obsoleted";
    return QString();
}

QString IconCache::iconToQrcPath(const QIcon &pm) const
{
    Q_UNUSED(pm)
    qWarning() << "IconCache::iconToQrcPath(): IconCache is obsoleted";
    return QString();
}

QPixmap IconCache::nameToPixmap(const QString &path, const QString &resourcePath)
{
    Q_UNUSED(path)
    Q_UNUSED(resourcePath)
    qWarning() << "IconCache::nameToPixmap(): IconCache is obsoleted";
    return QPixmap();
}

QString IconCache::pixmapToFilePath(const QPixmap &pm) const
{
    Q_UNUSED(pm)
    qWarning() << "IconCache::pixmapToFilePath(): IconCache is obsoleted";
    return QString();
}

QString IconCache::pixmapToQrcPath(const QPixmap &pm) const
{
    Q_UNUSED(pm)
    qWarning() << "IconCache::pixmapToQrcPath(): IconCache is obsoleted";
    return QString();
}

QList<QPixmap> IconCache::pixmapList() const
{
    qWarning() << "IconCache::pixmapList(): IconCache is obsoleted";
    return QList<QPixmap>();
}

QList<QIcon> IconCache::iconList() const
{
    qWarning() << "IconCache::iconList(): IconCache is obsoleted";
    return QList<QIcon>();
}

QString IconCache::resolveQrcPath(const QString &filePath, const QString &qrcPath, const QString &wd) const
{
    Q_UNUSED(filePath)
    Q_UNUSED(qrcPath)
    Q_UNUSED(wd)
    qWarning() << "IconCache::resolveQrcPath(): IconCache is obsoleted";
    return QString();
}

QT_END_NAMESPACE
