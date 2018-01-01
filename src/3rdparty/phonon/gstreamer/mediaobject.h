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

#ifndef GSTREAMER_MEDIAOBJECT_H
#define GSTREAMER_MEDIAOBJECT_H

#include "backend.h"
#include "common.h"
#include "medianode.h"

#include <phonon/mediaobjectinterface.h>
#include <phonon/addoninterface.h>
#include <QtCore/QHash>
#include <QtCore/QString>
#include <QtCore/QVariant>
#include <QtCore/QObject>
#include <QtCore/QDate>
#include <QtCore/QUrl>
#include <qcoreevent.h>
#include <gst/gst.h>

QT_BEGIN_NAMESPACE

class QTimer;
typedef QMultiMap<QString, QString> TagMap;

namespace Phonon
{
namespace Gstreamer
{

class VideoWidget;
class AudioPath;
class VideoPath;
class AudioOutput;

class MediaObject : public QObject, public MediaObjectInterface

#ifndef QT_NO_PHONON_MEDIACONTROLLER
        , public AddonInterface
#endif
        , public MediaNode
{
    friend class Stream;
    friend class AudioDataOutput;

    CS_OBJECT_MULTIPLE(MediaObject, QObject)
      
#ifndef QT_NO_PHONON_MEDIACONTROLLER    
   CS_INTERFACES(Phonon::AddonInterface, Phonon::MediaObjectInterface, Phonon::Gstreamer::MediaNode)
#else
   CS_INTERFACES(Phonon::MediaObjectInterface, Phonon::Gstreamer::MediaNode)
#endif
  
public:
    MediaObject(Backend *backend, QObject *parent);
    ~MediaObject();

    Phonon::State state() const override;

    bool hasVideo() const override;
    bool isSeekable() const override;

    qint64 currentTime() const override;
    qint32 tickInterval() const override;
    void setTickInterval(qint32 newTickInterval) override;

    void play() override;
    void pause() override;
    void stop() override;
    void seek(qint64 time) override;

    QString errorString() const override;
    Phonon::ErrorType errorType() const override;

    QUrl url() const;
    qint64 totalTime() const override;

    qint32 prefinishMark() const override;
    void setPrefinishMark(qint32 newPrefinishMark) override;

    qint32 transitionTime() const override;
    void setTransitionTime(qint32) override;
    qint64 remainingTime() const override;

    void setSource(const MediaSource &source) override;
    void setNextSource(const MediaSource &source) override;
    MediaSource source() const override; 

    // No additional interfaces currently supported
#ifndef QT_NO_PHONON_MEDIACONTROLLER
    bool hasInterface(Interface) const override;
    QVariant interfaceCall(Interface, int, const QList<QVariant> &) override;
#endif

    bool isLoading()
    {
        return m_loading;
    }

    bool audioAvailable()
    {
        return m_hasAudio;
    }

    bool videoAvailable()
    {
        return m_hasVideo;
    }

    GstElement *audioGraph()
    {
        return m_audioGraph;
    }

    GstElement *videoGraph()
    {
        return m_videoGraph;
    }

    GstElement *pipeline()
    {
        return m_pipeline;
    };

    gulong capsHandler()
    {
        return m_capsHandler;
    };

    void connectVideo(GstPad *videoPad);
    void connectAudio(GstPad *audioPad);
    void handleBusMessage(const Message &msg);
    void handleEndOfStream();
    void addMissingCodecName(const QString &codec) { m_missingCodecs.append(codec); }
    void invalidateGraph();

    static void cb_newpad (GstElement *decodebin, GstPad *pad, gboolean last, gpointer data);
    static void cb_pad_added (GstElement *decodebin, GstPad *pad, gpointer data);
    static void cb_unknown_type (GstElement *decodebin, GstPad *pad, GstCaps *caps, gpointer data);
    static void cb_no_more_pads (GstElement * decodebin, gpointer data);
    void saveState();
    void resumeState();

public :
    QMultiMap<QString,QString> metaData();

    GSTRM_CS_SLOT_1(Public, void setState(State un_named_arg1))
    GSTRM_CS_SLOT_2(setState)    

    GSTRM_CS_SIGNAL_1(Public, void currentSourceChanged(const MediaSource & newSource))
    GSTRM_CS_SIGNAL_2(currentSourceChanged,newSource) 

    GSTRM_CS_SIGNAL_1(Public, void stateChanged(Phonon::State newstate,Phonon::State oldstate))
    GSTRM_CS_SIGNAL_2(stateChanged,newstate,oldstate) 

    GSTRM_CS_SIGNAL_1(Public, void tick(qint64 time))
    GSTRM_CS_SIGNAL_2(tick,time) 

    GSTRM_CS_SIGNAL_1(Public, void metaDataChanged(const QMultiMap<QString,QString> &un_named_arg1))
    GSTRM_CS_SIGNAL_2(metaDataChanged,un_named_arg1) 

    GSTRM_CS_SIGNAL_1(Public, void seekableChanged(bool un_named_arg1))
    GSTRM_CS_SIGNAL_2(seekableChanged,un_named_arg1) 

    GSTRM_CS_SIGNAL_1(Public, void hasVideoChanged(bool un_named_arg1))
    GSTRM_CS_SIGNAL_2(hasVideoChanged,un_named_arg1) 

    GSTRM_CS_SIGNAL_1(Public, void finished())
    GSTRM_CS_SIGNAL_2(finished) 

    GSTRM_CS_SIGNAL_1(Public, void prefinishMarkReached(qint32 un_named_arg1))
    GSTRM_CS_SIGNAL_2(prefinishMarkReached,un_named_arg1) 

    GSTRM_CS_SIGNAL_1(Public, void aboutToFinish())
    GSTRM_CS_SIGNAL_2(aboutToFinish) 

    GSTRM_CS_SIGNAL_1(Public, void totalTimeChanged(qint64 length))
    GSTRM_CS_SIGNAL_2(totalTimeChanged,length) 

    GSTRM_CS_SIGNAL_1(Public, void bufferStatus(int percentFilled))
    GSTRM_CS_SIGNAL_2(bufferStatus,percentFilled)   
    
    GSTRM_CS_SIGNAL_1(Public, void setMetaData(QMultiMap <QString,QString> newData))
    GSTRM_CS_SIGNAL_2(setMetaData,newData) 

    // AddonInterface:
    GSTRM_CS_SIGNAL_1(Public, void titleChanged(int un_named_arg1))
    GSTRM_CS_SIGNAL_2(titleChanged,un_named_arg1) 

    GSTRM_CS_SIGNAL_1(Public, void availableTitlesChanged(int un_named_arg1))
    GSTRM_CS_SIGNAL_2(availableTitlesChanged,un_named_arg1) 

    // Not implemented
    GSTRM_CS_SIGNAL_1(Public, void chapterChanged(int un_named_arg1))
    GSTRM_CS_SIGNAL_2(chapterChanged,un_named_arg1) 
    GSTRM_CS_SIGNAL_1(Public, void availableChaptersChanged(int un_named_arg1))
    GSTRM_CS_SIGNAL_2(availableChaptersChanged,un_named_arg1) 
    GSTRM_CS_SIGNAL_1(Public, void angleChanged(int un_named_arg1))
    GSTRM_CS_SIGNAL_2(angleChanged,un_named_arg1) 
    GSTRM_CS_SIGNAL_1(Public, void availableAnglesChanged(int un_named_arg1))
    GSTRM_CS_SIGNAL_2(availableAnglesChanged,un_named_arg1) 

    GSTRM_CS_SIGNAL_1(Public, void availableSubtitlesChanged())
    GSTRM_CS_SIGNAL_2(availableSubtitlesChanged) 
    GSTRM_CS_SIGNAL_1(Public, void availableAudioChannelsChanged())
    GSTRM_CS_SIGNAL_2(availableAudioChannelsChanged) 

protected:
    void beginLoad();
    void loadingComplete();
    void newPadAvailable (GstPad *pad);
    void changeState(State);
    void setError(const QString &errorString, Phonon::ErrorType error = NormalError);
    /*
     * @param encodedUrl percent-encoded QString for source compat reasons.  Should change to QUrl
     */
    bool createPipefromURL(const QUrl &url);
    bool createPipefromStream(const MediaSource &);

   GstElement *audioElement() override
    {
        Q_ASSERT(m_audioPipe);
        return m_audioPipe;
    }

    GstElement *videoElement() override
    {
        Q_ASSERT(m_videoPipe);
        return m_videoPipe;
    }

private :
    GSTRM_CS_SLOT_1(Private, void noMorePadsAvailable())
    GSTRM_CS_SLOT_2(noMorePadsAvailable)     
    GSTRM_CS_SLOT_1(Private, void getStreamInfo())
    GSTRM_CS_SLOT_2(getStreamInfo) 
    GSTRM_CS_SLOT_1(Private, void emitTick())
    GSTRM_CS_SLOT_2(emitTick) 
    GSTRM_CS_SLOT_1(Private, void beginPlay())
    GSTRM_CS_SLOT_2(beginPlay) 
    GSTRM_CS_SLOT_1(Private, void setVideoCaps(GstCaps * caps))
    GSTRM_CS_SLOT_2(setVideoCaps) 
    GSTRM_CS_SLOT_1(Private, void notifyStateChange(Phonon::State newstate,Phonon::State oldstate))
    GSTRM_CS_SLOT_2(notifyStateChange) 

    // GStreamer specific 
    void createPipeline();
    bool addToPipeline(GstElement *elem);
    void setTotalTime(qint64 newTime);
    void getStreamsInfo();
    bool updateTotalTime();
    void updateSeekable();
    qint64 getPipelinePos() const;

    int _iface_availableTitles() const;
    int _iface_currentTitle() const;
    void _iface_setCurrentTitle(int title);
    void setTrack(int title);

    bool m_resumeState;
    State m_oldState;
    quint64 m_oldPos;

    State m_state;
    State m_pendingState;
    QTimer *m_tickTimer;
    qint32 m_tickInterval;

    MediaSource m_source;
    MediaSource m_nextSource;
    qint32 m_prefinishMark;
    qint32 m_transitionTime;
	 bool m_isStream;

    qint64 m_posAtSeek;

    bool m_prefinishMarkReachedNotEmitted;
    bool m_aboutToFinishEmitted;
    bool m_loading;
    gulong m_capsHandler;

    GstElement *m_datasource;
    GstElement *m_decodebin;

    GstElement *m_audioPipe;
    GstElement *m_videoPipe;

    qint64 m_totalTime;
    int m_bufferPercent;
    bool m_hasVideo;
    bool m_videoStreamFound;
    bool m_hasAudio;
    bool m_seekable;
    bool m_atEndOfStream;
    bool m_atStartOfStream;
    Phonon::ErrorType m_error;
    QString m_errorString;

    GstElement *m_pipeline;
    GstElement *m_audioGraph;
    GstElement *m_videoGraph;
    int m_previousTickTime;
    bool m_resetNeeded;
    QStringList m_missingCodecs;
    QMultiMap<QString, QString> m_metaData;
    bool m_autoplayTitles;
    int m_availableTitles;
    int m_currentTitle;
    int m_pendingTitle;
};
}

} //namespace Phonon::Gstreamer

QT_END_NAMESPACE

#endif
