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

#include <qiconengineplugin.h>
#include <qstringlist.h>

#if !defined(QT_NO_IMAGEFORMATPLUGIN) && !defined(QT_NO_SVG)

#include "qsvgiconengine.h"

#include <qiodevice.h>
#include <qbytearray.h>
#include <qdebug.h>

QT_BEGIN_NAMESPACE

class QSvgIconPlugin : public QIconEnginePluginV2
{
public:
    QStringList keys() const;
    QIconEngineV2 *create(const QString &filename = QString());
};

QStringList QSvgIconPlugin::keys() const
{
    QStringList keys(QLatin1String("svg"));
#ifndef QT_NO_COMPRESS
    keys << QLatin1String("svgz") << QLatin1String("svg.gz");
#endif
    return keys;
}

QIconEngineV2 *QSvgIconPlugin::create(const QString &file)
{
    QSvgIconEngine *engine = new QSvgIconEngine;
    if (!file.isNull())
        engine->addFile(file, QSize(), QIcon::Normal, QIcon::Off);
    return engine;
}

Q_EXPORT_STATIC_PLUGIN(QSvgIconPlugin)
Q_EXPORT_PLUGIN2(qsvgicon, QSvgIconPlugin)

QT_END_NAMESPACE

#endif // !QT_NO_IMAGEFORMATPLUGIN
