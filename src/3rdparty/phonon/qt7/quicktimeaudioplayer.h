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

#ifndef QT7_QUICKTIMEAUDIOPLAYER_H
#define QT7_QUICKTIMEAUDIOPLAYER_H

#include "backendheader.h"
#include <phonon/mediasource.h>
#include <QtCore/QString>
#include "audionode.h"

QT_BEGIN_NAMESPACE

namespace Phonon
{
namespace QT7
{
    class AudioGraph;
    class MediaNodeEvent;
    class QuickTimeVideoPlayer;

    class QuickTimeAudioPlayer : public AudioNode
    {
        public:
            enum State {Playing, Paused, NoMedia, NoState};

            QuickTimeAudioPlayer();
            virtual ~QuickTimeAudioPlayer();

            void play();
            void pause();
            void seek(quint64 milliseconds);
            void enableAudio(bool enable);
	        bool audioEnabled();
            void flush();

            void setVideoPlayer(QuickTimeVideoPlayer *videoPlayer);
            void unsetVideoPlayer();

            bool hasAudio();
            bool isPlaying();
            void scheduleAudioToGraph();
            long regularTaskFrequency();
            quint64 currentTime();
            QString currentTimeString();
            QuickTimeVideoPlayer *videoPlayer();

            ComponentDescription getAudioNodeDescription() const;
            void initializeAudioUnit();
            bool fillInStreamSpecification(AudioConnection *connection, ConnectionSide side);
            void mediaNodeEvent(const MediaNodeEvent *event);

            static bool soundPlayerIsAwailable();

        private:
            void initSoundExtraction();
            void newGraphNotification();
            void allocateSoundSlices();
            void scheduleSoundSlices();

            State m_state;
            QuickTimeVideoPlayer *m_videoPlayer;

            ScheduledAudioSlice *m_sliceList;
            AudioChannelLayout *m_audioChannelLayout;
            UInt32 m_audioChannelLayoutSize;
            AudioStreamBasicDescription m_audioStreamDescription;

            bool m_discrete;
            bool m_playerUnitStarted;
            bool m_audioExtractionComplete;
            bool m_audioEnabled;
            bool m_audioUnitIsReset;

            long m_samplesRemaining;
            int m_sliceCount;
            int m_maxExtractionPacketCount;

            Float64 m_sampleTimeStamp;
            quint64 m_startTime;
    };

}} // namespace Phonon::QT7

QT_END_NAMESPACE

#endif // Phonon_QT7_QUICKTIMEAUDIOPLAYER_H
