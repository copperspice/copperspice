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

#ifndef GSTREAMER_EFFECTMANAGER_H
#define GSTREAMER_EFFECTMANAGER_H

#include "common.h"

#include <QtCore/QObject>
#include <QtCore/QTimer>
#include <QtCore/QStringList>

#include <gst/gst.h>

QT_BEGIN_NAMESPACE

namespace Phonon
{
namespace Gstreamer
{
class Backend;
class EffectManager;

class EffectInfo
{
public :
    EffectInfo(const QString &name,
               const QString &description,
               const QString &author);

    QString name() const
    {
        return m_name;
    }
    QString description() const
    {
        return m_description;
    }
    QString author() const
    {
        return m_author;
    }
    QStringList properties() const
    {
        return m_properties;
    }
    void addProperty(QString propertyName)
    {
        m_properties.append(propertyName);
    }

private:
    QString m_name;
    QString m_description;
    QString m_author;
    QStringList m_properties;
};

class EffectManager : public QObject
{
    GSTRM_CS_OBJECT(EffectManager)
public:
    EffectManager(Backend *parent);
    virtual ~EffectManager();
    const QList<EffectInfo*> audioEffects() const;

private:
    Backend *m_backend;
    QList <EffectInfo*> m_audioEffectList;
    QList <EffectInfo*> m_visualizationList;
};
}
} // namespace Phonon::Gstreamer

QT_END_NAMESPACE

#endif // Phonon_GSTREAMER_EFFECTMANAGER_H
