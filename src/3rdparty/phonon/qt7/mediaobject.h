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

#ifndef QT7_MEDIAOBJECT_H
#define QT7_MEDIAOBJECT_H

#include <QtCore/QStringList>
#include <QtCore/QTime>
#include <phonon/mediaobjectinterface.h>
#include <phonon/addoninterface.h>
#include "medianode.h"

QT_BEGIN_NAMESPACE

namespace Phonon
{
namespace QT7
{
    class QuickTimeVideoPlayer;
    class QuickTimeAudioPlayer;
    class QuickTimeMetaData;
    class AudioGraph;
    class MediaObjectAudioNode;

    class MediaObject : public MediaNode, public Phonon::MediaObjectInterface, public Phonon::AddonInterface
    {
        CS_OBJECT_MULTIPLE(MediaObject, MediaNode)
        CS_INTERFACES(Phonon::MediaObjectInterface, Phonon::AddonInterface)

    public:
        MediaObject(QObject *parent);
        ~MediaObject();

        QStringList availableAudioStreams() const;
        QStringList availableVideoStreams() const;
        QStringList availableSubtitleStreams() const;
        QString currentAudioStream(const QObject *audioPath) const;
        QString currentVideoStream(const QObject *videoPath) const;
        QString currentSubtitleStream(const QObject *videoPath) const;

        void setCurrentAudioStream(const QString &streamName,const QObject *audioPath);
        void setCurrentVideoStream(const QString &streamName,const QObject *videoPath);
        void setCurrentSubtitleStream(const QString &streamName,const QObject *videoPath);

        void play() override;
        void pause() override;
        void stop() override;
        void seek(qint64 milliseconds) override;

        qint32 tickInterval() const override;
        void setTickInterval(qint32 interval) override;
        bool hasVideo() const override;
        bool isSeekable() const override;
        qint64 currentTime() const override;
        Phonon::State state() const override;

        QString errorString() const override;
        Phonon::ErrorType errorType() const override;

        qint64 totalTime() const override;
        MediaSource source() const override;
        void setSource(const MediaSource &) override;
        void setNextSource(const MediaSource &source) override;
        qint32 prefinishMark() const override;
        void setPrefinishMark(qint32) override;
        qint32 transitionTime() const override;
        void setTransitionTime(qint32) override;
        bool hasInterface(Interface interface) const override;
        QVariant interfaceCall(Interface interface, int command,
                  const QList<QVariant> &arguments = QList<QVariant>()) override;

        QuickTimeVideoPlayer* videoPlayer() const;
        QuickTimeAudioPlayer* audioPlayer() const;

        void setVolumeOnMovie(float volume);
        bool setAudioDeviceOnMovie(int id);

        int videoOutputCount();

        QT7_CS_SIGNAL_1(Public, void stateChanged(Phonon::State un_named_arg1,Phonon::State un_named_arg2))
        QT7_CS_SIGNAL_2(stateChanged,un_named_arg1,un_named_arg2)

        QT7_CS_SIGNAL_1(Public, void tick(qint64 un_named_arg1))
        QT7_CS_SIGNAL_2(tick,un_named_arg1)

        QT7_CS_SIGNAL_1(Public, void seekableChanged(bool un_named_arg1))
        QT7_CS_SIGNAL_2(seekableChanged,un_named_arg1)

        QT7_CS_SIGNAL_1(Public, void hasVideoChanged(bool un_named_arg1))
        QT7_CS_SIGNAL_2(hasVideoChanged,un_named_arg1)

        QT7_CS_SIGNAL_1(Public, void bufferStatus(int un_named_arg1))
        QT7_CS_SIGNAL_2(bufferStatus,un_named_arg1)

        QT7_CS_SIGNAL_1(Public, void finished())
        QT7_CS_SIGNAL_2(finished)

        QT7_CS_SIGNAL_1(Public, void aboutToFinish())
        QT7_CS_SIGNAL_2(aboutToFinish)

        QT7_CS_SIGNAL_1(Public, void prefinishMarkReached(qint32 un_named_arg1))
        QT7_CS_SIGNAL_2(prefinishMarkReached,un_named_arg1)

        QT7_CS_SIGNAL_1(Public, void totalTimeChanged(qint64 un_named_arg1))
        QT7_CS_SIGNAL_2(totalTimeChanged,un_named_arg1)

        QT7_CS_SIGNAL_1(Public, void metaDataChanged(const QMultiMap <QString,QString> &un_named_arg1))
        QT7_CS_SIGNAL_2(metaDataChanged,un_named_arg1)

        QT7_CS_SIGNAL_1(Public, void currentSourceChanged(const MediaSource & newSource))
        QT7_CS_SIGNAL_2(currentSourceChanged,newSource)

    protected:
        void mediaNodeEvent(const MediaNodeEvent *event) override;
        bool event(QEvent *event) override;

    private:
        enum AudioSystem {AS_Unset, AS_Video, AS_Graph, AS_Silent} m_audioSystem;
        Phonon::State m_state;

        QuickTimeVideoPlayer *m_videoPlayer;
        QuickTimeAudioPlayer *m_audioPlayer;
        QuickTimeVideoPlayer *m_nextVideoPlayer;
        QuickTimeAudioPlayer *m_nextAudioPlayer;
        MediaObjectAudioNode *m_mediaObjectAudioNode;
        QuickTimeMetaData *m_metaData;

        qint32 m_tickInterval;
        qint32 m_transitionTime;
        quint32 m_prefinishMark;
        quint32 m_currentTime;
        float m_percentageLoaded;

        int m_tickTimer;
        int m_bufferTimer;
        int m_rapidTimer;

        bool m_waitNextSwap;
        int m_swapTimeLeft;
        QTime m_swapTime;

        void synchAudioVideo();
        void updateCurrentTime();
        void swapCurrentWithNext(qint32 transitionTime);
        bool setState(Phonon::State state);
        void pause_internal();
        void play_internal();
        void setupAudioSystem();
        void updateTimer(int &timer, int interval);
        void bufferAudioVideo();
        void updateRapidly();
        void updateCrossFade();
        void updateAudioBuffers();
        void updateLipSynch(int allowedOffset);
        void updateVideoFrames();
        void updateBufferStatus();
        void setMute(bool mute);
        void inspectAudioGraphRecursive(AudioConnection *connection, int &effectCount, int &outputCount);
        void inspectVideoGraphRecursive(MediaNode *node, int &effectCount, int &outputCount);
        void inspectGraph();
        bool isCrossFading();

        QString m_errorString;
        Phonon::ErrorType m_errorType;
        bool checkForError();

       int m_audioEffectCount;
       int m_audioOutputCount;
       int m_videoEffectCount;
       int m_videoOutputCount;
    };

}} //namespace Phonon::QT7

QT_END_NAMESPACE

#endif
