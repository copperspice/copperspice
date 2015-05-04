/***********************************************************************
*
* Copyright (c) 2012-2015 Barbara Geller
* Copyright (c) 2012-2015 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or 
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see 
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef GSTREAMER_BACKEND_H
#define GSTREAMER_BACKEND_H

#include "common.h"
#include "devicemanager.h"
#include "medianode.h"
#include "message.h"

#include <phonon/objectdescription.h>
#include <phonon/backendinterface.h>

#include <QtCore/QList>
#include <QtCore/QPointer>
#include <QtCore/QStringList>
#include <QtCore/QTimer>

#include <gst/gst.h>

QT_BEGIN_NAMESPACE

namespace Phonon
{
namespace Gstreamer
{
class AudioOutput;
class MediaNode;
class MediaObject;
class EffectManager;

class Backend : public QObject, public BackendInterface
{
    CS_OBJECT(Backend)
    CS_INTERFACES(Phonon::BackendInterface)

public:

    enum DebugLevel {NoDebug, Warning, Info, Debug};
    Backend(QObject *parent = 0, const QVariantList & = QVariantList());
    virtual ~Backend();

    DeviceManager* deviceManager() const;
    EffectManager* effectManager() const;

    QObject *createObject(BackendInterface::Class, QObject *parent, const QList<QVariant> &args);

    bool isValid() const;
    bool supportsVideo() const;
    QStringList availableMimeTypes() const;

    QList<int> objectDescriptionIndexes(ObjectDescriptionType type) const;
    QHash<QByteArray, QVariant> objectDescriptionProperties(ObjectDescriptionType type, int index) const;

    bool startConnectionChange(QSet<QObject *>);
    bool connectNodes(QObject *, QObject *);
    bool disconnectNodes(QObject *, QObject *);
    bool endConnectionChange(QSet<QObject *>);

    DebugLevel debugLevel() const;

    void addBusWatcher(MediaObject* node);
    void removeBusWatcher(MediaObject* node);
    void logMessage(const QString &message, int priority = 2, QObject *obj=0) const;
    bool checkDependencies() const;

    GSTRM_CS_SIGNAL_1(Public, void objectDescriptionChanged(ObjectDescriptionType un_named_arg1))
    GSTRM_CS_SIGNAL_2(objectDescriptionChanged,un_named_arg1) 

private :
    GSTRM_CS_SLOT_1(Private, void handleBusMessage(Message un_named_arg1))
    GSTRM_CS_SLOT_2(handleBusMessage) 

    static gboolean busCall(GstBus *bus, GstMessage *msg, gpointer data);

    DeviceManager *m_deviceManager;
    EffectManager *m_effectManager;
    DebugLevel m_debugLevel;
    bool m_isValid;
};
}
} // namespace Phonon::Gstreamer

QT_END_NAMESPACE

#endif // Phonon_GSTREAMER_BACKEND_H
