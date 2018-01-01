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
**  This file is part of the KDE project.
********************************************************/

#import <QTKit/QTMovie.h>

#include "quicktimeaudioplayer.h"
#include "quicktimevideoplayer.h"
#include "audiograph.h"
#include "medianodeevent.h"
#include "medianode.h"

QT_BEGIN_NAMESPACE

namespace Phonon
{
namespace QT7
{

QuickTimeAudioPlayer::QuickTimeAudioPlayer() : AudioNode(0, 1)
{
    m_state = NoMedia;
    m_videoPlayer = 0;
    m_audioChannelLayout = 0;
    m_sliceList = 0;
    m_sliceCount = 30;
    m_maxExtractionPacketCount = 4096;
    m_audioExtractionComplete = false;
    m_audioEnabled = true;
    m_samplesRemaining = -1;
    m_startTime = 0;
    m_sampleTimeStamp = 0;
    m_audioUnitIsReset = true;
}

QuickTimeAudioPlayer::~QuickTimeAudioPlayer()
{
    unsetVideoPlayer();
}

void QuickTimeAudioPlayer::unsetVideoPlayer()
{
    if (m_audioUnit){
        OSStatus err = AudioUnitReset(m_audioUnit, kAudioUnitScope_Global, 0);
        BACKEND_ASSERT2(err == noErr, "Could not reset audio player unit when unsetting movie", FATAL_ERROR)
    }

    if (m_audioChannelLayout){
        free(m_audioChannelLayout);
        m_audioChannelLayout = 0;
    }
    
    if (m_sliceList){
        for (int i=0; i<m_sliceCount; i++)
	        free(m_sliceList[i].mBufferList);
        free(m_sliceList);
        m_sliceList = 0;
    }
    
    m_videoPlayer = 0;
    m_audioExtractionComplete = false;
    m_samplesRemaining = -1;
    m_sampleTimeStamp = 0;
    m_state = NoMedia;
}

void QuickTimeAudioPlayer::enableAudio(bool enable)
{
    // Remember to seek after enabling audio.
    if (enable == m_audioEnabled)
        return;
        
    m_audioEnabled = enable;
    if (!enable)
        flush();
}

bool QuickTimeAudioPlayer::audioEnabled()
{
    return m_audioEnabled;
}

void QuickTimeAudioPlayer::setVideoPlayer(QuickTimeVideoPlayer *videoPlayer)
{
    unsetVideoPlayer();
    if (videoPlayer && videoPlayer->hasMovie()){
        m_videoPlayer = videoPlayer;
        initSoundExtraction();
        allocateSoundSlices();
        m_state = Paused;
        seek(0);
    }
}

QuickTimeVideoPlayer *QuickTimeAudioPlayer::videoPlayer()
{
    return m_videoPlayer;
}

void QuickTimeAudioPlayer::scheduleAudioToGraph()
{
    if (!m_videoPlayer || !m_audioEnabled || m_audioExtractionComplete || m_state != Playing)
        return;

    // Schedule audio slices, and detect if everything went OK.
    // If not, flag the need for another audio system, but let
    // the end app know about it:
    gClearError();
    scheduleSoundSlices();    
    if (gGetErrorType() != NO_ERROR){
        gClearError();
        if (m_audioGraph)
            m_audioGraph->setStatusCannotPlay();
    }
}

void QuickTimeAudioPlayer::flush()
{
    // Empty scheduled audio data, so playback
    // will stop. Call seek to refill data again.
    if (m_audioUnit){
        m_startTime = currentTime();
        OSStatus err = AudioUnitReset(m_audioUnit, kAudioUnitScope_Global, 0);
        BACKEND_ASSERT2(err == noErr, "Could not reset audio player unit on pause", FATAL_ERROR)
        m_audioUnitIsReset = true;
    }
}

void QuickTimeAudioPlayer::pause()
{
    m_state = Paused;
    flush();
}

void QuickTimeAudioPlayer::play()
{
    m_state = Playing;
    if (!m_audioEnabled)
        return;
    if (m_audioUnitIsReset)
        seek(m_startTime);
    else
        scheduleAudioToGraph();
}

bool QuickTimeAudioPlayer::isPlaying()
{
    return m_videoPlayer && m_state == Playing;
}

void QuickTimeAudioPlayer::seek(quint64 milliseconds)
{
    if (!m_videoPlayer || !m_videoPlayer->hasMovie())
        return;    
    if (milliseconds > m_videoPlayer->duration())
        milliseconds = m_videoPlayer->duration();
    if (!m_audioUnitIsReset && milliseconds == currentTime())
        return;
        
    m_startTime = milliseconds;
    
    // Since the graph may be running (advancing time), there is
    // no point in seeking if were not going to play immidiatly:
    if (m_state != Playing)
        return;
    if (!m_audioUnit)
        return;
    if (!m_audioEnabled || !m_videoPlayer->isSeekable())
        return;

    // Reset (and stop playing):
    OSStatus err;
    if (!m_audioUnitIsReset){
        err = AudioUnitReset(m_audioUnit, kAudioUnitScope_Global, 0);
        BACKEND_ASSERT2(err == noErr, "Could not reset audio player unit before seek", FATAL_ERROR)
    }
    m_sampleTimeStamp = 0;
    for (int i = 0; i < m_sliceCount; i++)
	    m_sliceList[i].mFlags = kScheduledAudioSliceFlag_Complete;

    // Start to play again immidiatly:
    AudioTimeStamp timeStamp;
    memset(&timeStamp, 0, sizeof(timeStamp));
	 timeStamp.mFlags = kAudioTimeStampSampleTimeValid;
    timeStamp.mSampleTime = -1;
	 err = AudioUnitSetProperty(m_audioUnit,
        kAudioUnitProperty_ScheduleStartTimeStamp, kAudioUnitScope_Global,
        0, &timeStamp, sizeof(timeStamp));
    BACKEND_ASSERT2(err == noErr, "Could not set schedule start time stamp on audio player unit", FATAL_ERROR)

    // Seek back to 'now' in the movie:
    TimeRecord timeRec;
	 timeRec.scale = m_videoPlayer->timeScale();
    timeRec.base = 0;
	 timeRec.value.hi = 0;
	 timeRec.value.lo = (milliseconds / 1000.0f) * timeRec.scale;

    float durationLeftSec = float(m_videoPlayer->duration() - milliseconds) / 1000.0f;
    m_samplesRemaining = (durationLeftSec > 0) ? (durationLeftSec * m_audioStreamDescription.mSampleRate) : -1;
    m_audioExtractionComplete = false;
    m_audioUnitIsReset = false;    
    scheduleAudioToGraph();

}

quint64 QuickTimeAudioPlayer::currentTime()
{
    if (!m_audioUnit){
        if (m_videoPlayer)
            return m_videoPlayer->currentTime();
        else
            return m_startTime;
    }

    Float64 currentUnitTime = getTimeInSamples(kAudioUnitProperty_CurrentPlayTime);
    if (currentUnitTime == -1)
        currentUnitTime = 0;

    quint64 cTime = quint64(m_startTime +
        float(currentUnitTime / float(m_audioStreamDescription.mSampleRate)) * 1000.0f);
    return (m_videoPlayer && cTime > m_videoPlayer->duration()) ? m_videoPlayer->duration() : cTime;
}

QString QuickTimeAudioPlayer::currentTimeString()
{
    return QuickTimeVideoPlayer::timeToString(currentTime());
}

bool QuickTimeAudioPlayer::hasAudio()
{
    if (!m_videoPlayer)
        return false;
        
    return m_videoPlayer->hasAudio();
}

bool QuickTimeAudioPlayer::soundPlayerIsAwailable()
{
    QuickTimeAudioPlayer player;
    ComponentDescription d = player.getAudioNodeDescription();
    return FindNextComponent(0, &d);
}

ComponentDescription QuickTimeAudioPlayer::getAudioNodeDescription() const
{
	ComponentDescription description;
	description.componentType = kAudioUnitType_Generator;
	description.componentSubType = kAudioUnitSubType_ScheduledSoundPlayer;
	description.componentManufacturer = kAudioUnitManufacturer_Apple;
	description.componentFlags = 0;
	description.componentFlagsMask = 0;
    return description;
}

void QuickTimeAudioPlayer::initializeAudioUnit()
{
}

bool QuickTimeAudioPlayer::fillInStreamSpecification(AudioConnection *connection, ConnectionSide side)
{
    if (!m_videoPlayer){
        if (side == Source)
            DEBUG_AUDIO_STREAM("QuickTimeAudioPlayer" << int(this) << "is source, but has no movie to use for stream spec fill.")
        return true;
    }

    if (side == Source){
        DEBUG_AUDIO_STREAM("QuickTimeAudioPlayer" << int(this) << "is source, and fills in stream spec from movie.")
        connection->m_sourceStreamDescription = m_audioStreamDescription;
        connection->m_sourceChannelLayout = (AudioChannelLayout *) malloc(m_audioChannelLayoutSize);
        memcpy(connection->m_sourceChannelLayout, m_audioChannelLayout, m_audioChannelLayoutSize);
        connection->m_sourceChannelLayoutSize = m_audioChannelLayoutSize;
        connection->m_hasSourceSpecification = true;
    }
    return true;
}

long QuickTimeAudioPlayer::regularTaskFrequency(){
    if (!m_audioEnabled || !m_audioUnit || (m_audioGraph && m_audioGraph->graphCannotPlay()))
        return INT_MAX;

    // Calculate how much audio in
    // milliseconds our slices can hold:
    int packetNeedPerSecond = m_audioStreamDescription.mSampleRate / m_maxExtractionPacketCount;
    long bufferTimeLengthSec = float(m_sliceCount) / float(packetNeedPerSecond);
    // Make sure we also get some time to fill the
    // buffer, so divide the time by two:
    return (bufferTimeLengthSec * (1000 / 2));
}

void QuickTimeAudioPlayer::initSoundExtraction()
{
}

void QuickTimeAudioPlayer::allocateSoundSlices()
{
}

void QuickTimeAudioPlayer::scheduleSoundSlices()
{
}

void QuickTimeAudioPlayer::mediaNodeEvent(const MediaNodeEvent *event)
{
    switch (event->type()){
        case MediaNodeEvent::AudioGraphAboutToBeDeleted:
        case MediaNodeEvent::AboutToRestartAudioStream:
        case MediaNodeEvent::StartConnectionChange:
            m_startTime = currentTime();
            break;
        case MediaNodeEvent::AudioGraphInitialized:
        case MediaNodeEvent::RestartAudioStreamRequest:
        case MediaNodeEvent::EndConnectionChange:
            if (m_state == Playing)
                seek(m_startTime);
            break;
       default:
            break;
    }
}

}
}

QT_END_NAMESPACE

