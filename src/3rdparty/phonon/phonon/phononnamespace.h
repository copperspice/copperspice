/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2005-2006 Matthias Kretz <kretz@kde.org>
* Copyright (C) 2015 The Qt Company Ltd.
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
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
* https://www.gnu.org/licenses/
*
***********************************************************************/

#ifndef PHONON_PHONONNAMESPACE_H
#define PHONON_PHONONNAMESPACE_H

#include "phonon_export.h"
#include <qstringfwd.h>

#define PHONON_VERSION_CHECK(major, minor, patch) ((major<<16)|(minor<<8)|(patch))
#define PHONON_VERSION PHONON_VERSION_CHECK(4, 4, 0)
#define PHONON_VERSION_STR "4.4.0"

namespace Phonon
{
    PHONON_EXPORT const char *phononVersion();

    /*
     * Enum to identify the media discs supported by MediaObject.
     *
     * \see MediaSource(Phonon::DiscType, const QString &deviceName)
     */
    enum DiscType {
        /*
         * No disc was selected. This is only useful as a return value from
         * MediaSource::distType();
         */
        NoDisc = -1,
        /*
         * Identifies Audio CDs.
         */
        Cd = 0,
        /*
         * Identifies DVDs (not arbitrary data DVDs, only movie DVDs).
         */
        Dvd = 1,
        /*
         * Identifies Video CDs.
         */
        Vcd = 2
    };

    /*
     * Provided as keys for \ref MediaObject::metaData for convenience, in addition to the strings defined in
     * the Ogg Vorbis specification.
     */
    enum MetaData {
        /*
         * The artist generally considered responsible for the work. In popular
         * music this is usually the performing band or singer. For classical
         * music it would be the composer. For an audio book it would be the
         * author of the original text.
         */
        ArtistMetaData,
        /*
         * The collection name to which this track belongs.
         */
        AlbumMetaData,
        /*
         * Track/Work name
         */
        TitleMetaData,
        /*
         * Date the track was recorded
         */
        DateMetaData,
        /*
         * A short text indication of music genre
         */
        GenreMetaData,
        /*
         * The track number of this piece if part of a specific larger
         * collection or album
         */
        TracknumberMetaData,
        /*
         * A short text description of the contents
         */
        DescriptionMetaData,
        MusicBrainzDiscIdMetaData
    };

    enum State
    {
        LoadingState,
        StoppedState,
        PlayingState,
        BufferingState,
        PausedState,
        ErrorState
    };

    /*
     * Set's the category your program should be listed in in the mixer.
     *
     * A Jukebox will set this to Music, a VoIP program to Communication, a
     * DVD player to video, and so on.
     *
     * \note These categories can also become useful for an application that
     * controls the volumes automatically, like turning down the music when a call
     * comes in, or turning down the notifications when the media player knows
     * it's playing classical music.
     *
     * \see AudioOutput::setCategory
     */
    enum Category
    {
        /*
         * Will make use of the default device.
         */
        NoCategory = -1,
        /*
         * If the sounds produced are notifications (bing, beep and such) you
         * should use this category.
         */
        NotificationCategory = 0,
        /*
         * If your application is a music player (like a jukebox or media player
         * playing an audio file).
         */
        MusicCategory = 1,
        /*
         * If the sound is the audio channel of a video.
         */
        VideoCategory = 2,
        /*
         * If your applications produces sounds from communication with somebody
         * else (VoIP, voice chat).
         */
        CommunicationCategory = 3,
        /*
         * Sound produced by a computer game should go into this category.
         */
        GameCategory = 4,
        /*
         * Sounds produced for accessibility (e.g. Text-To-Speech)
         */
        AccessibilityCategory = 5,
        /*
         * \internal
         * Holds the largest value of categories.
         */
        LastCategory = AccessibilityCategory
    };

    /*
     * Tells your program how to recover from an error.
     *
     * \see MediaObject::errorType()
     */
    enum ErrorType {
        /*
         * No error. MediaObject::errorType() returns this if
         * MediaObject::state() != Phonon::ErrorState.
         */
        NoError = 0,
        /*
         * Playback should work, and trying with another URL should work.
         */
        NormalError = 1,
        /*
         * Something important does not work. Your program cannot continue
         * playback or capture or whatever it was trying to do
         * without help from the user.
         */
        FatalError = 2
    };

    /*
     * Returns a (translated) string to show to the user identifying the given
     * Category.
     */
    PHONON_EXPORT QString categoryToString(Category c);

    // TODO: naming
    /*enum MediaStreamType {
        Audio = 1,
        Video = 2,
        StillImage = 4,
        Subtitle = 8,
        AllMedia = 0xFFFFFFFF
    };
    Q_DECLARE_FLAGS(MediaStreamTypes, MediaStreamType)*/
} // namespace Phonon

//Q_DECLARE_OPERATORS_FOR_FLAGS(Phonon::MediaStreamTypes)



//X class kdbgstream;
//X #include <kdebug.h>
//X /**
//X  * Implements needed operator to use Phonon::State with kDebug
//X  */
//X inline PHONON_EXPORT kdbgstream &operator<<(kdbgstream  & stream, const Phonon::State state)
//X {
//X     switch(state)
//X     {
//X     case Phonon::ErrorState:
//X         stream << "ErrorState";
//X         break;
//X     case Phonon::LoadingState:
//X         stream << "LoadingState";
//X         break;
//X     case Phonon::StoppedState:
//X         stream << "StoppedState";
//X         break;
//X     case Phonon::PlayingState:
//X         stream << "PlayingState";
//X         break;
//X     case Phonon::BufferingState:
//X         stream << "BufferingState";
//X         break;
//X     case Phonon::PausedState:
//X         stream << "PausedState";
//X         break;
//X     }
//X     return stream;
//X }

#include <QtCore/QMetaType>

Q_DECLARE_METATYPE(Phonon::State)
Q_DECLARE_METATYPE(Phonon::ErrorType)
Q_DECLARE_METATYPE(Phonon::Category)

#endif
