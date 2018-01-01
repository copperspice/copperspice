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

#ifndef ABSTRACTICONCACHE_H
#define ABSTRACTICONCACHE_H

#include <QtDesigner/sdk_global.h>

#include <QtCore/QObject>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

class QIcon;
class QPixmap;
class QString;

class QDESIGNER_SDK_EXPORT QDesignerIconCacheInterface : public QObject
{
    Q_OBJECT
public:
    QDesignerIconCacheInterface(QObject *parent_)
        : QObject(parent_) {}

    virtual QIcon nameToIcon(const QString &filePath, const QString &qrcPath = QString()) = 0;
    virtual QPixmap nameToPixmap(const QString &filePath, const QString &qrcPath = QString()) = 0;

    virtual QString iconToFilePath(const QIcon &pm) const = 0;
    virtual QString iconToQrcPath(const QIcon &pm) const = 0;

    virtual QString pixmapToFilePath(const QPixmap &pm) const = 0;
    virtual QString pixmapToQrcPath(const QPixmap &pm) const = 0;

    virtual QList<QPixmap> pixmapList() const = 0;
    virtual QList<QIcon> iconList() const = 0;

    virtual QString resolveQrcPath(const QString &filePath, const QString &qrcPath, const QString &workingDirectory = QString()) const = 0;
};

QT_END_NAMESPACE

QT_END_HEADER

#endif // ABSTRACTICONCACHE_H
