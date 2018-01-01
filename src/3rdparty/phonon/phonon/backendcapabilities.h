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

#ifndef Phonon_BACKENDCAPABILITIES_H
#define Phonon_BACKENDCAPABILITIES_H

#include "phonon_export.h"
#include "objectdescription.h"
#include <QtCore/QObject>

QT_BEGIN_NAMESPACE

template<class T> class QList;
class QStringList;

namespace Phonon
{

namespace BackendCapabilities
{   
    class Notifier : public QObject
    {
        PHN_CS_OBJECT(Notifier)

        public:           
            PHN_CS_SIGNAL_1(Public, void capabilitiesChanged())
            PHN_CS_SIGNAL_2(capabilitiesChanged) 
            
            PHN_CS_SIGNAL_1(Public, void availableAudioOutputDevicesChanged())
            PHN_CS_SIGNAL_2(availableAudioOutputDevicesChanged) 
            
#ifndef QT_NO_PHONON_AUDIOCAPTURE
            PHN_CS_SIGNAL_1(Public, void availableAudioCaptureDevicesChanged())
            PHN_CS_SIGNAL_2(availableAudioCaptureDevicesChanged) 
#endif

    };
  
    PHONON_EXPORT Notifier *notifier();  
    PHONON_EXPORT QStringList availableMimeTypes();   
    PHONON_EXPORT bool isMimeTypeAvailable(const QString &mimeType);
    PHONON_EXPORT QList<AudioOutputDevice> availableAudioOutputDevices();

   
#ifndef QT_NO_PHONON_AUDIOCAPTURE
    PHONON_EXPORT QList<AudioCaptureDevice> availableAudioCaptureDevices();
#endif

    /**
     * Returns the video output devices the backend supports.
     *
     * \return A list of VideoOutputDevice objects that give a name and
     * description for every supported video output device.
     */
//    PHONON_EXPORT QList<VideoOutputDevice> availableVideoOutputDevices();

    /**
     * Returns the video capture devices the backend supports.
     *
     * \return A list of VideoCaptureDevice objects that give a name and
     * description for every supported video capture device.
     */
//    PHONON_EXPORT QList<VideoCaptureDevice> availableVideoCaptureDevices();

    /**
     * Returns the visualization effects the backend supports.
     *
     * \return A list of VisualizationEffect objects that give a name and
     * description for every supported visualization effect.
     */
//    PHONON_EXPORT QList<VisualizationDescription> availableVisualizations();

    /**
     * Returns descriptions for the audio effects the backend supports.
     *
     * \return A list of AudioEffectDescription objects that give a name and
     * description for every supported audio effect.
     */

#ifndef QT_NO_PHONON_EFFECT
    PHONON_EXPORT QList<EffectDescription> availableAudioEffects();
#endif

//X     /**
//X      * Returns descriptions for the video effects the backend supports.
//X      *
//X      * \return A list of VideoEffectDescription objects that give a name and
//X      * description for every supported video effect.
//X      */
//X     PHONON_EXPORT QList<EffectDescription> availableVideoEffects();

    /**
     * Returns descriptions for the audio codecs the backend supports.
     *
     * \return A list of AudioCodec objects that give a name and
     * description for every supported audio codec.
     */
//    PHONON_EXPORT QList<AudioCodecDescription> availableAudioCodecs();

    /**
     * Returns descriptions for the video codecs the backend supports.
     *
     * \return A list of VideoCodec objects that give a name and
     * description for every supported video codec.
     */
//    PHONON_EXPORT QList<VideoCodecDescription> availableVideoCodecs();

    /**
     * Returns descriptions for the container formats the backend supports.
     *
     * \return A list of ContainerFormat objects that give a name and
     * description for every supported container format.
     */
//    PHONON_EXPORT QList<ContainerFormatDescription> availableContainerFormats();
} // namespace BackendCapabilities
} // namespace Phonon

QT_END_NAMESPACE

#endif
