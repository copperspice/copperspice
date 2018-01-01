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

/********************************************************
**  Copyright (C) 2005-2006 Matthias Kretz <kretz@kde.org
********************************************************/

#include "factory_p.h"

#include "backendinterface.h"
#include "medianode_p.h"
#include "mediaobject.h"
#include "audiooutput.h"
#include "globalstatic_p.h"
#include "objectdescription.h"
#include "platformplugin.h"
#include "phononnamespace_p.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QList>
#include <QtCore/QPluginLoader>
#include <QtCore/QPointer>

#ifndef QT_NO_DBUS
#include <QtDBus/QtDBus>
#endif

QT_BEGIN_NAMESPACE

namespace Phonon
{

class PlatformPlugin;

class FactoryPrivate : public Phonon::Factory::Sender
{  
    PHN_CS_OBJECT(FactoryPrivate)
    friend QObject *Factory::backend(bool);

    public:
        FactoryPrivate();
        ~FactoryPrivate();

        bool createBackend();

#ifndef QT_NO_PHONON_PLATFORMPLUGIN
        PlatformPlugin *platformPlugin();

        PlatformPlugin *m_platformPlugin;
        bool m_noPlatformPlugin;
#endif

        QPointer<QObject> m_backendObject;

        QList<QObject *> objects;
        QList<MediaNodePrivate *> mediaNodePrivateList;

    private:
        
#ifndef QT_NO_DBUS        
        // called via DBUS when the user changes the Phonon Backend.
        PHN_CS_SLOT_1(Private,void phononBackendChanged())
        PHN_CS_SLOT_2(phononBackendChanged)

#endif
        // unregisters the backend object
        PHN_CS_SLOT_1(Private,void objectDestroyed(QObject *))
        PHN_CS_SLOT_2(objectDestroyed)

        PHN_CS_SLOT_1(Private,void objectDescriptionChanged(ObjectDescriptionType))
        PHN_CS_SLOT_2(objectDescriptionChanged)                       
};

PHONON_GLOBAL_STATIC(Phonon::FactoryPrivate, globalFactory)

static inline void ensureLibraryPathSet()
{
#ifdef PHONON_LIBRARY_PATH
    static bool done = false;

    if (! done) {
        done = true;
        QCoreApplication::addLibraryPath(QLatin1String(PHONON_LIBRARY_PATH));
    }
#endif
}

void Factory::setBackend(QObject *b)
{
    Q_ASSERT(globalFactory->m_backendObject == 0);
    globalFactory->m_backendObject = b;
}

bool FactoryPrivate::createBackend()
{
    Q_ASSERT(m_backendObject == 0);

#ifndef QT_NO_PHONON_PLATFORMPLUGIN
    PlatformPlugin *f = globalFactory->platformPlugin();

    if (f) {
        m_backendObject = f->createBackend();
    }
#endif 

    if (! m_backendObject) {
        ensureLibraryPathSet();

        // could not load a backend through the platform plugin. Falling back to the default
        // (finding the first loadable backend).
        const QLatin1String suffix("/phonon_backend/");
        const QStringList paths = QCoreApplication::libraryPaths();

        for (int i = 0; i < paths.count(); ++i) {
            const QString libPath = paths.at(i) + suffix;
            const QDir dir(libPath);

            if (! dir.exists()) {
                pDebug() << Q_FUNC_INFO << dir.absolutePath() << "does not exist";
                continue;
            }

            QStringList plugins(dir.entryList(QDir::Files));

            const QStringList files = dir.entryList(QDir::Files);

            for (int i = 0; i < plugins.count(); ++i) {
                QPluginLoader pluginLoader(libPath + plugins.at(i));

                if (! pluginLoader.load()) {
                    pDebug() << Q_FUNC_INFO << "  load failed:"<< pluginLoader.errorString();
                    continue;
                }

                pDebug() << pluginLoader.instance();
                m_backendObject = pluginLoader.instance();

                if (m_backendObject) {
                    break;
                }

                // no backend found, do not leave an unused plugin in memory
                pluginLoader.unload();
            }

            if (m_backendObject) {
                break;
            }
        }

        if (! m_backendObject) {
            pWarning() << Q_FUNC_INFO << "phonon backend plugin could not be loaded";
            return false;
        }
    }

    connect(m_backendObject, SIGNAL(objectDescriptionChanged(ObjectDescriptionType)),
                  this, SLOT(objectDescriptionChanged(ObjectDescriptionType)));

    return true;
}

FactoryPrivate::FactoryPrivate()
    :
#ifndef QT_NO_PHONON_PLATFORMPLUGIN
    m_platformPlugin(nullptr),
    m_noPlatformPlugin(false),
#endif
    m_backendObject(nullptr)
{
    // Add the post routine to make sure that all other global statics (especially the ones from Qt)
    // are still available. If the FactoryPrivate dtor is called too late many bad things can happen
    // as the whole backend might still be alive.
    qAddPostRoutine(globalFactory.destroy);

#ifndef QT_NO_DBUS
    QDBusConnection::sessionBus().connect(QString(), QString(), QLatin1String("org.kde.Phonon.Factory"),
        QLatin1String("phononBackendChanged"), this, SLOT(phononBackendChanged()));
#endif

}

FactoryPrivate::~FactoryPrivate()
{
    for (int i = 0; i < mediaNodePrivateList.count(); ++i) {
        mediaNodePrivateList.at(i)->deleteBackendObject();
    }

    if (objects.size() > 0) {
        pError() << "The backend objects are not deleted as was requested.";
        qDeleteAll(objects);
    }

    delete m_backendObject;

#ifndef QT_NO_PHONON_PLATFORMPLUGIN
    delete m_platformPlugin;
#endif
}

void FactoryPrivate::objectDescriptionChanged(ObjectDescriptionType type)
{

#ifdef PHONON_METHODTEST
    Q_UNUSED(type);
#else
    pDebug() << Q_FUNC_INFO << type;
    switch (type) {
    case AudioOutputDeviceType:
        emit availableAudioOutputDevicesChanged();
        break;
    case AudioCaptureDeviceType:
        emit availableAudioCaptureDevicesChanged();
        break;
    default:
        break;
    }

    //emit capabilitiesChanged();

#endif 
}

Factory::Sender *Factory::sender()
{
    return globalFactory;
}

bool Factory::isMimeTypeAvailable(const QString &mimeType)
{
#ifndef QT_NO_PHONON_PLATFORMPLUGIN
    PlatformPlugin *f = globalFactory->platformPlugin();

    if (f) {
        return f->isMimeTypeAvailable(mimeType);
    }
#else
    Q_UNUSED(mimeType);
#endif

    return true; // the MIME type might be supported, let BackendCapabilities find out
}

void Factory::registerFrontendObject(MediaNodePrivate *bp)
{
    globalFactory->mediaNodePrivateList.prepend(bp); // inserted last => deleted first
}

void Factory::deregisterFrontendObject(MediaNodePrivate *bp)
{
    // The Factory can already be cleaned up while there are other frontend objects still alive.
    // When those are deleted they'll call deregisterFrontendObject through ~BasePrivate
    if (!globalFactory.isDestroyed()) {
        globalFactory->mediaNodePrivateList.removeAll(bp);
    }
}

#ifndef QT_NO_DBUS
void FactoryPrivate::phononBackendChanged()
{
    if (m_backendObject) {
        for (int i = 0; i < mediaNodePrivateList.count(); ++i) {
            mediaNodePrivateList.at(i)->deleteBackendObject();
        }
        if (objects.size() > 0) {
            pDebug() << "WARNING: we were asked to change the backend but the application did\n"
                "not free all references to objects created by the factory. Therefore we can not\n"
                "change the backend without crashing. Now we have to wait for a restart to make\n"
                "backendswitching possible.";

            // in case there were objects deleted give them a chance to recreate them now
            for (int i = 0; i < mediaNodePrivateList.count(); ++i) {
                mediaNodePrivateList.at(i)->createBackendObject();
            }
            return;
        }
        delete m_backendObject;
        m_backendObject = 0;
    }

    createBackend();
    for (int i = 0; i < mediaNodePrivateList.count(); ++i) {
        mediaNodePrivateList.at(i)->createBackendObject();
    }

    emit backendChanged();
}
#endif

//X void Factory::freeSoundcardDevices()
//X {
//X     if (globalFactory->backend) {
//X         globalFactory->backend->freeSoundcardDevices();
//X     }
//X }

void FactoryPrivate::objectDestroyed(QObject * obj)
{
    //pDebug() << Q_FUNC_INFO << obj;
    objects.removeAll(obj);
}

#define FACTORY_IMPL(classname) \
QObject *Factory::create ## classname(QObject *parent) \
{ \
    if (backend()) { \
        return registerQObject(qobject_cast<BackendInterface *>(backend())->createObject(BackendInterface::classname##Class, parent)); \
    } \
    return 0; \
}
#define FACTORY_IMPL_1ARG(classname) \
QObject *Factory::create ## classname(int arg1, QObject *parent) \
{ \
    if (backend()) { \
        return registerQObject(qobject_cast<BackendInterface *>(backend())->createObject(BackendInterface::classname##Class, parent, QList<QVariant>() << arg1)); \
    } \
    return 0; \
}

FACTORY_IMPL(MediaObject)
#ifndef QT_NO_PHONON_EFFECT
FACTORY_IMPL_1ARG(Effect)
#endif //QT_NO_PHONON_EFFECT
#ifndef QT_NO_PHONON_VOLUMEFADEREFFECT
FACTORY_IMPL(VolumeFaderEffect)
#endif //QT_NO_PHONON_VOLUMEFADEREFFECT
FACTORY_IMPL(AudioOutput)
#ifndef QT_NO_PHONON_VIDEO
FACTORY_IMPL(VideoWidget)
#endif //QT_NO_PHONON_VIDEO
FACTORY_IMPL(AudioDataOutput)

#undef FACTORY_IMPL

#ifndef QT_NO_PHONON_PLATFORMPLUGIN
PlatformPlugin *FactoryPrivate::platformPlugin()
{
    if (m_platformPlugin) {
        return m_platformPlugin;
    }

    if (m_noPlatformPlugin) {
        return nullptr;
    }

#ifndef QT_NO_DBUS
    if (! QCoreApplication::instance() || QCoreApplication::applicationName().isEmpty()) {
        pWarning() << "Phonon needs QCoreApplication::applicationName set in order to export audio output names through the DBUS interface";
    }
#endif

    Q_ASSERT(QCoreApplication::instance());
    const QByteArray platform_plugin_env = qgetenv("PHONON_PLATFORMPLUGIN");

    if (! platform_plugin_env.isEmpty()) {
        QPluginLoader pluginLoader(QString::fromLocal8Bit(platform_plugin_env.constData()));

        if (pluginLoader.load()) {
            m_platformPlugin = qobject_cast<PlatformPlugin *>(pluginLoader.instance());

            if (m_platformPlugin) {
                return m_platformPlugin;
            }
        }
    }

    const QString suffix(QLatin1String("/phonon_platform/"));
    ensureLibraryPathSet();
    QDir dir;

    dir.setNameFilters(! qgetenv("KDE_FULL_SESSION").isEmpty() ? QStringList(QLatin1String("kde.*")) :
            (!qgetenv("GNOME_DESKTOP_SESSION_ID").isEmpty() ? QStringList(QLatin1String("gnome.*")) : QStringList())  );

    dir.setFilter(QDir::Files);
    const QStringList libPaths = QCoreApplication::libraryPaths();

    while (true) {
        for (int i = 0; i < libPaths.count(); ++i) {
            const QString libPath = libPaths.at(i) + suffix;
            dir.setPath(libPath);

            if (! dir.exists()) {
                continue;
            }

            const QStringList files = dir.entryList(QDir::Files);
            for (int i = 0; i < files.count(); ++i) {
                QPluginLoader pluginLoader(libPath + files.at(i));

                if (! pluginLoader.load()) {
                    pDebug() << Q_FUNC_INFO << "  platform plugin load failed:" << pluginLoader.errorString();
                    continue;
                }

                pDebug() << pluginLoader.instance();
                QObject *qobj = pluginLoader.instance();
                m_platformPlugin = qobject_cast<PlatformPlugin *>(qobj);
                pDebug() << m_platformPlugin;

                if (m_platformPlugin) {
                    connect(qobj, SIGNAL(objectDescriptionChanged(ObjectDescriptionType)),
                            this, SLOT(objectDescriptionChanged(ObjectDescriptionType)));
                    return m_platformPlugin;

                } else {
                    delete qobj;
                    pDebug() << Q_FUNC_INFO 
                             << dir.absolutePath() 
                             << "exists but the platform plugin was not loadable:" 
                             << pluginLoader.errorString();
                    pluginLoader.unload();
                }
            }
        }

        if (dir.nameFilters().isEmpty()) {
           break;
        }

        dir.setNameFilters(QStringList());
    }

    pDebug() << Q_FUNC_INFO << "platform plugin could not be loaded";
    m_noPlatformPlugin = true;

    return nullptr;
}

PlatformPlugin *Factory::platformPlugin()
{
    return globalFactory->platformPlugin();
}
#endif // QT_NO_PHONON_PLATFORMPLUGIN

QObject *Factory::backend(bool createWhenNull)
{
    if (globalFactory.isDestroyed()) {
        return 0;
    }

    if (createWhenNull && globalFactory->m_backendObject == 0) {
        globalFactory->createBackend();

        // XXX: might create "reentrancy" problems:
        // a method calls this method and is called again because the 
        
        emit globalFactory->backendChanged();
    }
    return globalFactory->m_backendObject;
}

#ifndef QT_NO_PROPERTIES
#define GET_STRING_PROPERTY(name) \
QString Factory::name() \
{ \
    if (globalFactory->m_backendObject) { \
        return globalFactory->m_backendObject->property(#name).toString(); \
    } \
    return QString(); \
} \

GET_STRING_PROPERTY(identifier)
GET_STRING_PROPERTY(backendName)
GET_STRING_PROPERTY(backendComment)
GET_STRING_PROPERTY(backendVersion)
GET_STRING_PROPERTY(backendIcon)
GET_STRING_PROPERTY(backendWebsite)
#endif //QT_NO_PROPERTIES
QObject *Factory::registerQObject(QObject *o)
{
    if (o) {
        QObject::connect(o, SIGNAL(destroyed(QObject *)), globalFactory, SLOT(objectDestroyed(QObject *)), Qt::DirectConnection);
        globalFactory->objects.append(o);
    }
    return o;
}

} //namespace Phonon

QT_END_NAMESPACE

