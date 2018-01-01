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

#ifndef PHONON_MEDIAOBJECT_H
#define PHONON_MEDIAOBJECT_H

#include "medianode.h"
#include "mediasource.h"
#include "phonon_export.h"
#include "phonondefs.h"
#include "phononnamespace.h"

QT_BEGIN_NAMESPACE

namespace Phonon
{
    class MediaObjectPrivate;
 

    class PHONON_EXPORT MediaObject : public QObject, public MediaNode
    {
        CS_OBJECT_MULTIPLE(MediaObject, QObject)
        PHONON_OBJECT(MediaObject)
        K_DECLARE_PRIVATE(MediaObject)
             
        PHN_CS_PROPERTY_READ(transitionTime, transitionTime)
        PHN_CS_PROPERTY_WRITE(transitionTime, setTransitionTime)
      
        PHN_CS_PROPERTY_READ(prefinishMark, prefinishMark)
        PHN_CS_PROPERTY_WRITE(prefinishMark, setPrefinishMark)

        PHN_CS_PROPERTY_READ(tickInterval, tickInterval)
        PHN_CS_PROPERTY_WRITE(tickInterval, setTickInterval)

        friend class FrontendInterfacePrivate;

        public:          
            ~MediaObject();
          
            State state() const;           
            bool hasVideo() const;
            bool isSeekable() const;

            qint32 tickInterval() const;
            QStringList metaData(const QString &key) const;
            QStringList metaData(Phonon::MetaData key) const;
         
            QMultiMap<QString, QString> metaData() const;
            QString errorString() const;
            ErrorType errorType() const;

            MediaSource currentSource() const;
            void setCurrentSource(const MediaSource &source);

            QList<MediaSource> queue() const;

            void setQueue(const QList<MediaSource> &sources);
            void setQueue(const QList<QUrl> &urls); 
            void enqueue(const MediaSource &source);
            void enqueue(const QList<MediaSource> &sources);
            void enqueue(const QList<QUrl> &urls);
            void clearQueue();

            qint64 currentTime() const;
            qint64 totalTime() const;
            qint64 remainingTime() const;

            qint32 prefinishMark() const;
            void setPrefinishMark(qint32 msecToEnd);

            qint32 transitionTime() const;
            void setTransitionTime(qint32 msec);

            PHN_CS_SLOT_1(Public, void setTickInterval(qint32 newTickInterval))
            PHN_CS_SLOT_2(setTickInterval) 

            PHN_CS_SLOT_1(Public, void play())
            PHN_CS_SLOT_2(play) 

            PHN_CS_SLOT_1(Public, void pause())
            PHN_CS_SLOT_2(pause) 

            PHN_CS_SLOT_1(Public, void stop())
            PHN_CS_SLOT_2(stop) 

            PHN_CS_SLOT_1(Public, void seek(qint64 time))
            PHN_CS_SLOT_2(seek) 

            PHN_CS_SLOT_1(Public, void clear())
            PHN_CS_SLOT_2(clear) 

            PHN_CS_SIGNAL_1(Public, void stateChanged(Phonon::State newstate,Phonon::State oldstate))
            PHN_CS_SIGNAL_2(stateChanged,newstate,oldstate) 

            PHN_CS_SIGNAL_1(Public, void tick(qint64 time))
            PHN_CS_SIGNAL_2(tick,time) 

            PHN_CS_SIGNAL_1(Public, void metaDataChanged())
            PHN_CS_SIGNAL_2(metaDataChanged) 

            PHN_CS_SIGNAL_1(Public, void seekableChanged(bool isSeekable))
            PHN_CS_SIGNAL_2(seekableChanged,isSeekable) 

#ifndef QT_NO_PHONON_VIDEO
            PHN_CS_SIGNAL_1(Public, void hasVideoChanged(bool hasVideo))
            PHN_CS_SIGNAL_2(hasVideoChanged,hasVideo) 
#endif
            
            PHN_CS_SIGNAL_1(Public, void bufferStatus(int percentFilled))
            PHN_CS_SIGNAL_2(bufferStatus,percentFilled) 

            PHN_CS_SIGNAL_1(Public, void finished())
            PHN_CS_SIGNAL_2(finished) 
 
            PHN_CS_SIGNAL_1(Public, void currentSourceChanged(const Phonon::MediaSource & newSource))
            PHN_CS_SIGNAL_2(currentSourceChanged,newSource) 

            PHN_CS_SIGNAL_1(Public, void aboutToFinish())
            PHN_CS_SIGNAL_2(aboutToFinish) 

            PHN_CS_SIGNAL_1(Public, void prefinishMarkReached(qint32 msecToEnd))
            PHN_CS_SIGNAL_2(prefinishMarkReached,msecToEnd) 

            PHN_CS_SIGNAL_1(Public, void totalTimeChanged(qint64 newTotalTime))
            PHN_CS_SIGNAL_2(totalTimeChanged,newTotalTime) 

        protected:
            //MediaObject(Phonon::MediaObjectPrivate &dd, QObject *parent);

        private:
            PHN_CS_SLOT_1(Private, void _k_resumePlay())
            PHN_CS_SLOT_2(_k_resumePlay)

            PHN_CS_SLOT_1(Private, void _k_resumePause())
            PHN_CS_SLOT_2(_k_resumePause)

            PHN_CS_SLOT_1(Private, void _k_metaDataChanged(const QMultiMap<QString,QString> & un_named_arg1))
            PHN_CS_SLOT_2(_k_metaDataChanged)

#ifndef QT_NO_PHONON_ABSTRACTMEDIASTREAM
            PHN_CS_SLOT_1(Private, void _k_stateChanged(Phonon::State un_named_arg1,Phonon::State un_named_arg2))
            PHN_CS_SLOT_2(_k_stateChanged)
#endif 
            PHN_CS_SLOT_1(Private, void _k_aboutToFinish())
            PHN_CS_SLOT_2(_k_aboutToFinish)

            PHN_CS_SLOT_1(Private, void _k_currentSourceChanged(const MediaSource & un_named_arg1))
            PHN_CS_SLOT_2(_k_currentSourceChanged)
    };
   
    PHONON_EXPORT MediaObject *createPlayer(Phonon::Category category, const MediaSource &source = MediaSource());

} //namespace Phonon

QT_END_NAMESPACE

#endif
