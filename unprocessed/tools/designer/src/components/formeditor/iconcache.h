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

#ifndef ICONCACHE_H
#define ICONCACHE_H

#include "formeditor_global.h"

#include <QtDesigner/QDesignerIconCacheInterface>

QT_BEGIN_NAMESPACE

namespace qdesigner_internal {

class QT_FORMEDITOR_EXPORT IconCache : public QDesignerIconCacheInterface
{
    Q_OBJECT
public:
    explicit IconCache(QObject *parent);

    virtual QIcon nameToIcon(const QString &path, const QString &resourcePath = QString());
    virtual QString iconToFilePath(const QIcon &pm) const;
    virtual QString iconToQrcPath(const QIcon &pm) const;
    virtual QPixmap nameToPixmap(const QString &path, const QString &resourcePath = QString());
    virtual QString pixmapToFilePath(const QPixmap &pm) const;
    virtual QString pixmapToQrcPath(const QPixmap &pm) const;

    virtual QList<QPixmap> pixmapList() const;
    virtual QList<QIcon> iconList() const;

    virtual QString resolveQrcPath(const QString &filePath, const QString &qrcPath, const QString &workingDirectory = QString()) const;

private:
};

}  // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // ICONCACHE_H
