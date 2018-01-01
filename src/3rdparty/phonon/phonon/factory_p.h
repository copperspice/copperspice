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

#ifndef PHONON_FACTORY_P_H
#define PHONON_FACTORY_P_H

#include "phonon_export.h"
#include <QtCore/QObject>
#include <QtCore/QStringList>

QT_BEGIN_NAMESPACE

class QUrl;
class QIcon;

namespace Phonon
{
    class PlatformPlugin;
    class MediaNodePrivate;
    class AbstractMediaStream;

/**
 * \internal
 * \brief Factory to access the preferred Backend.
 *
 * This class is used internally to get the backend's implementation.
 * It keeps track of the objects that were created. When a
 * request for a backend change comes, it asks all frontend objects to delete
 * their backend objects and then checks whether they were all deleted. Only
 * then the old backend is unloaded and the new backend is loaded.
 *
 * \author Matthias Kretz <kretz@kde.org>
 */
namespace Factory
{    
    class Sender : public QObject
    {
        PHN_CS_OBJECT(Sender)

        public:            
            PHN_CS_SIGNAL_1(Public, void backendChanged())
            PHN_CS_SIGNAL_2(backendChanged) 

            PHN_CS_SIGNAL_1(Public, void availableAudioOutputDevicesChanged())
            PHN_CS_SIGNAL_2(availableAudioOutputDevicesChanged) 
           
            PHN_CS_SIGNAL_1(Public, void availableAudioCaptureDevicesChanged())
            PHN_CS_SIGNAL_2(availableAudioCaptureDevicesChanged) 
    };
   
    PHONON_EXPORT Sender *sender();
   
    QObject *createMediaObject(QObject *parent = nullptr);
  
#ifndef QT_NO_PHONON_EFFECT
    QObject *createEffect(int effectId, QObject *parent = nullptr);
#endif 
   
#ifndef QT_NO_PHONON_VOLUMEFADEREFFECT
    QObject *createVolumeFaderEffect(QObject *parent = nullptr);
#endif 

    QObject *createAudioOutput(QObject *parent = nullptr);
  
#ifndef QT_NO_PHONON_VIDEO
    QObject *createVideoWidget(QObject *parent = nullptr);
#endif 

    PHONON_EXPORT QObject *createAudioDataOutput(QObject *parent = nullptr);
    PHONON_EXPORT QObject *backend(bool createWhenNull = true);

    QString identifier();
    
    PHONON_EXPORT QString backendName();

    QString backendComment();
    QString backendVersion();
    QString backendIcon();
    QString backendWebsite();

    PHONON_EXPORT QObject *registerQObject(QObject *o);

    bool isMimeTypeAvailable(const QString &mimeType);

    PHONON_EXPORT void registerFrontendObject(MediaNodePrivate *);
    PHONON_EXPORT void deregisterFrontendObject(MediaNodePrivate *);

    PHONON_EXPORT void setBackend(QObject *);
    //PHONON_EXPORT void createBackend(const QString &library, const QString &version = QString());

    PHONON_EXPORT PlatformPlugin *platformPlugin();

//X    It is probably better if we can get away with internal handling of
//X    freeing the soundcard device when it's not needed anymore and
//X    providing an IPC method to stop all MediaObjects -> free all
//X    devices
//X    /**
//X     * \internal
//X     * This is called when the application needs to free the soundcard
//X     * device(s).
//X     */
//X    void freeSoundcardDevices();
} // namespace Factory
} // namespace Phonon

QT_END_NAMESPACE

#endif
