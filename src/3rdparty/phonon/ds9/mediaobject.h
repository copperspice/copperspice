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

#ifndef DS9_MEDIAOBJECT_H
#define DS9_MEDIAOBJECT_H

#include <phonon/mediaobjectinterface.h>
#include <phonon/addoninterface.h>

#include <QtCore/QHash>
#include <QtCore/QObject>
#include <QtCore/QQueue>
#include <QtCore/QBasicTimer>
#include <QtCore/QMutex>
#include <QtCore/QThread>

#include "backendnode.h"
#include "mediagraph.h"

QT_BEGIN_NAMESPACE

namespace Phonon
{
    class MediaSource;

    namespace DS9
    {
        class VideoWidget;
        class AudioOutput;

        class QWinWaitCondition
        {
        public:
            QWinWaitCondition() : m_handle(::CreateEvent(0,0,0,0))
            {
            }

            ~QWinWaitCondition()
            {
                ::CloseHandle(m_handle);
            }

            void reset()
            {
                //will block
                ::ResetEvent(m_handle);
            }

            void set()
            {
                //will unblock
                ::SetEvent(m_handle);
            }

            operator HANDLE()
            {
                return m_handle;
            }

            operator HEVENT()
            {
                return reinterpret_cast<HEVENT>(m_handle);
            }


        private:
            HANDLE m_handle;
        };

        class WorkerThread : public QThread
        {
            DS9_CS_OBJECT(WorkerThread)

        public:
            WorkerThread();
            ~WorkerThread();

            void run() override;

            //wants to know as soon as the state is set
            void addStateChangeRequest(Graph graph, OAFilterState, QList<Filter> = QList<Filter>());

            quint16 addSeekRequest(Graph graph, qint64 time);
            quint16 addUrlToRender(const QString &url);
            quint16 addFilterToRender(const Filter &filter);

            void replaceGraphForEventManagement(Graph newGraph, Graph oldGraph);

    		void abortCurrentRender(qint16 renderId);

            //tells the thread to stop processing
            void signalStop();

        public:
            DS9_CS_SIGNAL_1(Public, void asyncRenderFinished(quint16 un_named_arg1,HRESULT un_named_arg2,Graph un_named_arg3))
            DS9_CS_SIGNAL_2(asyncRenderFinished,un_named_arg1,un_named_arg2,un_named_arg3) 

            DS9_CS_SIGNAL_1(Public, void asyncSeekingFinished(quint16 un_named_arg1,qint64 un_named_arg2))
            DS9_CS_SIGNAL_2(asyncSeekingFinished,un_named_arg1,un_named_arg2) 

            DS9_CS_SIGNAL_1(Public, void stateReady(Graph un_named_arg1,Phonon::State un_named_arg2))
            DS9_CS_SIGNAL_2(stateReady,un_named_arg1,un_named_arg2) 

            DS9_CS_SIGNAL_1(Public, void eventReady(Graph graph,long eventCode,long param1))
            DS9_CS_SIGNAL_2(eventReady,graph,eventCode,param1) 

        private:

            enum Task
            {
                None,
                Render,
                Seek,
                ChangeState,
                ReplaceGraph //just updates recalls WaitForMultipleObject
            };

            struct Work
            {
                Work() : task(None), id(0), time(0) { }
                Task task;
                quint16 id;
                Graph graph;
                Graph oldGraph;
                Filter filter;
                QString url;
                union
                {
                    qint64 time;
                    OAFilterState state;
                };
                QList<Filter> decoders; //for the state change requests
            };
            void handleTask();

            Work m_currentWork;
            QQueue<Work> m_queue;
            bool m_finished;
            quint16 m_currentWorkId;
            QWinWaitCondition m_waitCondition;
            QMutex m_mutex; // mutex for the m_queue, m_finished and m_currentWorkId

            //this is for WaitForMultipleObjects
            struct
            {
                Graph graph;
                HANDLE handle;
            } m_graphHandle[FILTER_COUNT];
        };


        class MediaObject : public BackendNode, public Phonon::MediaObjectInterface
#ifndef QT_NO_PHONON_ABSTRACTMEDIASTREAM
            , public Phonon::AddonInterface
#endif
        {
            CS_OBJECT_MULTIPLE(MediaObject, BackendNode)
            friend class Stream;            
                        
#ifndef QT_NO_PHONON_ABSTRACTMEDIASTREAM
            CS_INTERFACES(Phonon::AddonInterface, Phonon::MediaObjectInterface)
#else
            CS_INTERFACES(Phonon::MediaObjectInterface)
#endif

        public:
            MediaObject(QObject *parent);
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
            void ensureStopped();
            void seek(qint64 time) override;

            QString errorString() const override;
            Phonon::ErrorType errorType() const override;

#ifndef QT_NO_PHONON_ABSTRACTMEDIASTREAM
            bool hasInterface(Interface) const override;
            QVariant interfaceCall(Interface iface, int command, const QList<QVariant> &params) override;
#endif

            qint64 totalTime() const override;
            qint32 prefinishMark() const override;
            void setPrefinishMark(qint32 newPrefinishMark) override;

            qint32 transitionTime() const override;
            void setTransitionTime(qint32) override;

            qint64 remainingTime() const override;

            MediaSource source() const override;
            void setSource(const MediaSource &source) override;
            void setNextSource(const MediaSource &source) override;

            //COM error management
            bool catchComError(HRESULT hr);

            void grabNode(BackendNode *node);
            bool connectNodes(BackendNode *source, BackendNode *sink);
            bool disconnectNodes(BackendNode *source, BackendNode *sink);

            void switchFilters(int index, Filter oldFilter, Filter newFilter);

            WorkerThread *workerThread();
            void loadingFinished(MediaGraph *mg);
            void seekingFinished(MediaGraph *mg);
            MediaGraph *currentGraph() const;

            //this is used by the backend only
            Phonon::State transactionState;

         private :
            DS9_CS_SLOT_1(Private, void switchToNextSource())
            DS9_CS_SLOT_2(switchToNextSource) 

            DS9_CS_SLOT_1(Private, void slotStateReady(Graph un_named_arg1,Phonon::State un_named_arg2))
            DS9_CS_SLOT_2(slotStateReady) 

            DS9_CS_SLOT_1(Private, void handleEvents(Graph un_named_arg1,long eventCode,long param1))
            DS9_CS_SLOT_2(handleEvents) 

            DS9_CS_SLOT_1(Private, void finishLoading(quint16 workId,HRESULT hr,Graph un_named_arg3))
            DS9_CS_SLOT_2(finishLoading) 

            DS9_CS_SLOT_1(Private, void finishSeeking(quint16 workId,qint64 time))
            DS9_CS_SLOT_2(finishSeeking) 

         public:
            DS9_CS_SIGNAL_1(Public, void stateChanged(Phonon::State newstate,Phonon::State oldstate))
            DS9_CS_SIGNAL_2(stateChanged,newstate,oldstate) 

            DS9_CS_SIGNAL_1(Public, void tick(qint64 time))
            DS9_CS_SIGNAL_2(tick,time) 

            DS9_CS_SIGNAL_1(Public, void metaDataChanged(const QMultiMap<QString,QString> &un_named_arg1))
            DS9_CS_SIGNAL_2(metaDataChanged,un_named_arg1) 

            DS9_CS_SIGNAL_1(Public, void seekableChanged(bool un_named_arg1))
            DS9_CS_SIGNAL_2(seekableChanged,un_named_arg1) 

            DS9_CS_SIGNAL_1(Public, void hasVideoChanged(bool un_named_arg1))
            DS9_CS_SIGNAL_2(hasVideoChanged,un_named_arg1) 

            DS9_CS_SIGNAL_1(Public, void bufferStatus(int un_named_arg1))
            DS9_CS_SIGNAL_2(bufferStatus,un_named_arg1) 

            // AddonInterface:
            DS9_CS_SIGNAL_1(Public, void titleChanged(int un_named_arg1))
            DS9_CS_SIGNAL_2(titleChanged,un_named_arg1) 

            DS9_CS_SIGNAL_1(Public, void availableTitlesChanged(int un_named_arg1))
            DS9_CS_SIGNAL_2(availableTitlesChanged,un_named_arg1) 

            DS9_CS_SIGNAL_1(Public, void chapterChanged(int un_named_arg1))
            DS9_CS_SIGNAL_2(chapterChanged,un_named_arg1) 

            DS9_CS_SIGNAL_1(Public, void availableChaptersChanged(int un_named_arg1))
            DS9_CS_SIGNAL_2(availableChaptersChanged,un_named_arg1) 

            DS9_CS_SIGNAL_1(Public, void angleChanged(int un_named_arg1))
            DS9_CS_SIGNAL_2(angleChanged,un_named_arg1) 

            DS9_CS_SIGNAL_1(Public, void availableAnglesChanged(int un_named_arg1))
            DS9_CS_SIGNAL_2(availableAnglesChanged,un_named_arg1) 

            DS9_CS_SIGNAL_1(Public, void finished())
            DS9_CS_SIGNAL_2(finished) 

            DS9_CS_SIGNAL_1(Public, void prefinishMarkReached(qint32 un_named_arg1))
            DS9_CS_SIGNAL_2(prefinishMarkReached,un_named_arg1) 

            DS9_CS_SIGNAL_1(Public, void aboutToFinish())
            DS9_CS_SIGNAL_2(aboutToFinish) 

            DS9_CS_SIGNAL_1(Public, void totalTimeChanged(qint64 length))
            DS9_CS_SIGNAL_2(totalTimeChanged,length) 

            DS9_CS_SIGNAL_1(Public, void currentSourceChanged(const MediaSource & un_named_arg1))
            DS9_CS_SIGNAL_2(currentSourceChanged,un_named_arg1) 

        protected:
            void setState(Phonon::State);
            void timerEvent(QTimerEvent *e) override;

        private:
#ifndef QT_NO_PHONON_VIDEO
            void updateVideoGeometry();
#endif
            void handleComplete(IGraphBuilder *graph);
            MediaGraph *nextGraph() const;

            void updateTargetTick();
            void updateStopPosition();

            mutable QString m_errorString;
            mutable Phonon::ErrorType m_errorType;

            Phonon::State m_state;
            Phonon::State m_nextState;
            qint32 m_transitionTime;

            qint32 m_prefinishMark;

            QBasicTimer m_tickTimer;
            qint32 m_tickInterval;

            //the graph(s)
            MediaGraph *m_graphs[FILTER_COUNT];

            //...the videowidgets in the graph
            QList<VideoWidget*> m_videoWidgets;
            QList<AudioOutput*> m_audioOutputs;

            bool m_buffering:1;
            bool m_oldHasVideo:1;
            bool m_prefinishMarkSent:1;
            bool m_aboutToFinishSent:1;
            bool m_nextSourceReadyToStart:1;

            //for TitleInterface (and commands)
#ifndef QT_NO_PHONON_MEDIACONTROLLER
            bool m_autoplayTitles:1;
            QList<qint64> m_titles;
            int m_currentTitle;
            int _iface_availableTitles() const;
            int _iface_currentTitle() const;
            void _iface_setCurrentTitle(int title, bool bseek = true);
            void setTitles(const QList<qint64> &titles);
            qint64 titleAbsolutePosition(int title) const;
#endif

            qint64 m_targetTick;
            WorkerThread m_thread;
        };
    }
}

QT_END_NAMESPACE

#endif // PHONON_MEDIAOBJECT_H
