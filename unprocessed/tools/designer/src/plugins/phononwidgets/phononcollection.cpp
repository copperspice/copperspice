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

#include "videoplayerplugin.h"
#include "seeksliderplugin.h"
#include "volumesliderplugin.h"

#include <QtDesigner/QDesignerCustomWidgetCollectionInterface>
#include <QtCore/qplugin.h>

QT_BEGIN_NAMESPACE

class PhononCollection: public QObject, public QDesignerCustomWidgetCollectionInterface
{
    Q_OBJECT
    Q_INTERFACES(QDesignerCustomWidgetCollectionInterface)
public:
    explicit PhononCollection(QObject *parent = 0);

    virtual QList<QDesignerCustomWidgetInterface*> customWidgets() const;

private:
    QList<QDesignerCustomWidgetInterface*> m_plugins;
};

PhononCollection::PhononCollection(QObject *parent) :
    QObject(parent)
{
    const QString group = QLatin1String("Phonon");
    m_plugins.push_back(new VideoPlayerPlugin(group, this));
    m_plugins.push_back(new SeekSliderPlugin(group, this));
    m_plugins.push_back(new VolumeSliderPlugin(group, this));
}

QList<QDesignerCustomWidgetInterface*> PhononCollection::customWidgets() const
{
    return m_plugins;
}

Q_EXPORT_PLUGIN(PhononCollection)

QT_END_NAMESPACE

#include "phononcollection.moc"
