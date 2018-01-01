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

#ifndef PHONON_MEDIAOBJECT_H
#define PHONON_MEDIAOBJECT_H

#include <phonon/mediaobjectinterface.h>

#include <QtCore/QHash>
#include <QtCore/QObject>
#include <QtCore/QQueue>
#include <QtCore/QBasicTimer>
#include <QtCore/QWaitCondition>
#include <QtCore/QMutex>
#include <QtCore/QThread>
#include <QFile>
#include <QIODevice>

#include <windows.h>


QT_BEGIN_NAMESPACE

namespace Phonon
{
    class MediaSource;

    namespace WaveOut
    {
        class WorkerThread;
        class AudioOutput;

        class MediaObject : public QObject, public Phonon::MediaObjectInterface {

           PHN_CS_OBJECT_MULTIPLE(MediaObject, QObject)
           CS_INTERFACES(Phonon::MediaObjectInterface)

         public:
            MediaObject(QObject *parent);
            ~MediaObject();
            Phonon::State state() const;
            bool hasVideo() const;
            bool isSeekable() const;
            qint64 currentTime() const;
            qint32 tickInterval() const;

            void setTickInterval(qint32 newTickInterval);
            void play();
            void pause();
            void stop();
            void seek(qint64 time);

            QString errorString() const;
            Phonon::ErrorType errorType() const;
            qint64 totalTime() const;
            qint32 prefinishMark() const;
            void setPrefinishMark(qint32 newPrefinishMark);
            qint32 transitionTime() const;
            void setTransitionTime(qint32);
            qint64 remainingTime() const;
            MediaSource source() const;
            void setSource(const MediaSource &source);
            void setNextSource(const MediaSource &source);
         

         public:
            PHN_CS_SIGNAL_1(Public, void stateChanged(Phonon::State newstate,Phonon::State oldstate))
            PHN_CS_SIGNAL_2(stateChanged,newstate,oldstate) 

            PHN_CS_SIGNAL_1(Public, void tick(qint64 time))
            PHN_CS_SIGNAL_2(tick,time) 

            PHN_CS_SIGNAL_1(Public, void metaDataChanged(const QMultiMap<QString,QString> &un_named_arg1))
            PHN_CS_SIGNAL_2(metaDataChanged,un_named_arg1) 

            PHN_CS_SIGNAL_1(Public, void seekableChanged(bool un_named_arg1))
            PHN_CS_SIGNAL_2(seekableChanged,un_named_arg1) 

            PHN_CS_SIGNAL_1(Public, void hasVideoChanged(bool un_named_arg1))
            PHN_CS_SIGNAL_2(hasVideoChanged,un_named_arg1) 

            PHN_CS_SIGNAL_1(Public, void bufferStatus(int un_named_arg1))
            PHN_CS_SIGNAL_2(bufferStatus,un_named_arg1) 

            PHN_CS_SIGNAL_1(Public, void finished())
            PHN_CS_SIGNAL_2(finished) 

            PHN_CS_SIGNAL_1(Public, void prefinishMarkReached(qint32 un_named_arg1))
            PHN_CS_SIGNAL_2(prefinishMarkReached,un_named_arg1) 

            PHN_CS_SIGNAL_1(Public, void aboutToFinish())
            PHN_CS_SIGNAL_2(aboutToFinish) 

            PHN_CS_SIGNAL_1(Public, void totalTimeChanged(qint64 length)const)
            PHN_CS_SIGNAL_2(totalTimeChanged,length) 

            PHN_CS_SIGNAL_1(Public, void currentSourceChanged(const MediaSource & un_named_arg1))
            PHN_CS_SIGNAL_2(currentSourceChanged,un_named_arg1) 

            PHN_CS_SIGNAL_1(Public, void outOfData(QIODevice * ioStream,QByteArray * buffer,bool * m_bufferingFinshed))
            PHN_CS_SIGNAL_2(outOfData,ioStream,buffer,m_bufferingFinshed) 

        protected:
            void setAudioOutput(QObject *audioOutput);

        private :
            PHN_CS_SLOT_1(Private, void setVolume(qreal newVolume))
            PHN_CS_SLOT_2(setVolume) 
      
            bool m_nextBufferIndex;
            bool prepareBuffers();
            void unPrepareBuffers();
            bool getWaveOutDevice();
            bool openWaveFile(QString fileName);
            bool readHeader();
            bool boolUpdateBuffer();
            bool fillBuffers();
            void swapBuffers();
            void setState(Phonon::State newState);
            void setError(ErrorType errorType, QString errorMessage);
            void deleteValidWaveOutDevice();
            void playBuffer(WAVEHDR *waveHeader);

            static void QT_WIN_CALLBACK WaveOutCallBack(HWAVEOUT hWaveOut, UINT uMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2);

             struct {
            WAVEHDR  *waveHeader;
            QByteArray data;
            } m_soundBuffer1, m_soundBuffer2;

            WAVEFORMATEX m_waveFormatEx;
            HWAVEOUT m_hWaveOut;

            QFile *m_file;
            QIODevice *m_stream;
            QString m_errorString;

            WorkerThread *m_thread;

            MediaSource m_source;
            MediaSource m_nextSource;
            AudioOutput *m_audioOutput;
            ErrorType m_errorType;

            qreal m_volume;
            qint64 m_mediaSize;
            qint64 m_totalTime;
            qint64 m_currentTime;
            qint64 m_transitionTime;
            qint64 m_prefinishMark;
            qint64 m_tickIntervalResolution;
            qint32 m_tickInterval;
            qint32 m_tick;
            Phonon::State m_state;

            bool m_bufferingFinished;
            bool m_paused;
            bool m_stopped;            
            bool m_hasNextSource; 
            bool m_hasSource;
            bool m_sourceIsValid;
            bool m_bufferPrepared;

        friend class Backend;
        };
    }
}

QT_END_NAMESPACE

#endif // PHONON_MEDIAOBJECT_H
