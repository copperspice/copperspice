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

#ifndef PHONON_MEDIACONTROLLER_H
#define PHONON_MEDIACONTROLLER_H

#include "phonon_export.h"
#include "objectdescription.h"

#include <QtCore/QObject>
#include <QtCore/QtGlobal>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_PHONON_MEDIACONTROLLER

namespace Phonon
{
class MediaControllerPrivate;
class MediaObject;

class PHONON_EXPORT MediaController : public QObject
{
    PHN_CS_OBJECT(MediaController)

    PHN_CS_ENUM(Feature)
    PHN_CS_FLAG(Feature,Features)

    public:
        enum Feature {
            Angles = 1,
            Chapters = 2,
            Titles = 4
        };
        using Features = QFlags<Feature>;

        MediaController(MediaObject *parent);
        ~MediaController();

        Features supportedFeatures() const;

        int availableAngles() const;
        int currentAngle() const;

        int availableChapters() const;
        int currentChapter() const;

        int availableTitles() const;
        int currentTitle() const;

        bool autoplayTitles() const;
       
        AudioChannelDescription currentAudioChannel() const;      
        SubtitleDescription currentSubtitle() const;

        QList<AudioChannelDescription> availableAudioChannels() const;
        QList<SubtitleDescription> availableSubtitles() const;

        void setCurrentAudioChannel(const Phonon::AudioChannelDescription &stream);
        void setCurrentSubtitle(const Phonon::SubtitleDescription &stream);
  
        PHN_CS_SLOT_1(Public, void setCurrentAngle(int angleNumber))
        PHN_CS_SLOT_2(setCurrentAngle) 

        PHN_CS_SLOT_1(Public, void setCurrentChapter(int chapterNumber))
        PHN_CS_SLOT_2(setCurrentChapter) 
      
        PHN_CS_SLOT_1(Public, void setCurrentTitle(int titleNumber))
        PHN_CS_SLOT_2(setCurrentTitle) 

        PHN_CS_SLOT_1(Public, void setAutoplayTitles(bool un_named_arg1))
        PHN_CS_SLOT_2(setAutoplayTitles) 
     
        PHN_CS_SLOT_1(Public, void nextTitle())
        PHN_CS_SLOT_2(nextTitle) 
      
        PHN_CS_SLOT_1(Public, void previousTitle())
        PHN_CS_SLOT_2(previousTitle) 

        PHN_CS_SIGNAL_1(Public, void availableSubtitlesChanged())
        PHN_CS_SIGNAL_2(availableSubtitlesChanged) 
        PHN_CS_SIGNAL_1(Public, void availableAudioChannelsChanged())
        PHN_CS_SIGNAL_2(availableAudioChannelsChanged) 
        PHN_CS_SIGNAL_1(Public, void availableAnglesChanged(int availableAngles))
        PHN_CS_SIGNAL_2(availableAnglesChanged,availableAngles) 
        PHN_CS_SIGNAL_1(Public, void angleChanged(int angleNumber))
        PHN_CS_SIGNAL_2(angleChanged,angleNumber) 
        PHN_CS_SIGNAL_1(Public, void availableChaptersChanged(int availableChapters))
        PHN_CS_SIGNAL_2(availableChaptersChanged,availableChapters) 
        PHN_CS_SIGNAL_1(Public, void chapterChanged(int chapterNumber))
        PHN_CS_SIGNAL_2(chapterChanged,chapterNumber) 
        PHN_CS_SIGNAL_1(Public, void availableTitlesChanged(int availableTitles))
        PHN_CS_SIGNAL_2(availableTitlesChanged,availableTitles) 
        PHN_CS_SIGNAL_1(Public, void titleChanged(int titleNumber))
        PHN_CS_SIGNAL_2(titleChanged,titleNumber) 

    protected:
        MediaControllerPrivate *const d;
};

} // namespace Phonon

Q_DECLARE_OPERATORS_FOR_FLAGS(Phonon::MediaController::Features)

#endif //QT_NO_PHONON_MEDIACONTROLLER

QT_END_NAMESPACE

#endif
