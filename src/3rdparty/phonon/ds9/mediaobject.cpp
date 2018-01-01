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

#include <QtCore/QVector>
#include <QtCore/QTimerEvent>
#include <QtCore/QTimer>
#include <QtCore/QTime>
#include <QtCore/QLibrary>

#include <dshow.h>
#include <objbase.h>
#include <initguid.h>
#include <qnetwork.h>
#include <evcode.h>

#include "mediaobject.h"
#include "videowidget.h"
#include "audiooutput.h"

#include <QtCore/QDebug>

#define TIMER_INTERVAL 16 //... ms for the timer that polls the current state (we use the multimedia timer)
#define PRELOAD_TIME 2000 // 2 seconds to load a source

QT_BEGIN_NAMESPACE

namespace Phonon
{
    namespace DS9
    {
        typedef BOOL (WINAPI* LPAMGETERRORTEXT)(HRESULT, WCHAR *, DWORD);

        // define the WorkerThread class
        WorkerThread::WorkerThread()
          : QThread(), m_finished(false), m_currentWorkId(1)
        {
        }

        WorkerThread::~WorkerThread()
        {
        }

        void WorkerThread::run()
        {
            while (m_finished == false) {
                HANDLE handles[FILTER_COUNT +1];
                handles[0] = m_waitCondition;
                int count = 1;

                for(int i = 0; i < FILTER_COUNT; ++i) {
                    if (m_graphHandle[i].graph) {
                        handles[count++] = m_graphHandle[i].handle;
                    }
                }

                DWORD result = ::WaitForMultipleObjects(count, handles, FALSE, INFINITE);
                if (result == WAIT_OBJECT_0) {
                    handleTask();

                } else {
                    //this is the event management
                    const Graph &graph = m_graphHandle[result - WAIT_OBJECT_0 - 1].graph;
                    long eventCode;
                    LONG_PTR param1, param2;

                    ComPointer<IMediaEvent> mediaEvent(graph, IID_IMediaEvent);
                    mediaEvent->GetEvent(&eventCode, &param1, &param2, 0);
                    emit eventReady(graph, eventCode, param1);
                    mediaEvent->FreeEventParams(eventCode, param1, param2);
                }
            }
        }

        // ?? wants to know as soon as the state is set
        void WorkerThread::addStateChangeRequest(Graph graph, OAFilterState state, QList<Filter> decoders)
        {
            QMutexLocker locker(&m_mutex);
            bool found = false;

            //we try to see if there is already an attempt to change the state and we remove it
            for(int i = 0; !found && i < m_queue.size(); ++i) {
                const Work &w = m_queue.at(i);
                if (w.graph == graph && w.task == ChangeState) {
                    found = true;
                    m_queue.removeAt(i);
                }
            }

            // create a new task
            Work w;
            w.task = ChangeState;
            w.id = m_currentWorkId++;
            w.graph = graph;
            w.state = state;
            w.decoders = decoders;
            m_queue.enqueue(w);
            m_waitCondition.set();
        }

        quint16 WorkerThread::addSeekRequest(Graph graph, qint64 time)
        {
            QMutexLocker locker(&m_mutex);
            bool found = false;
            //we try to see if there is already an attempt to seek and we remove it
            for(int i = 0; !found && i < m_queue.size(); ++i) {
                const Work &w = m_queue.at(i);
                if (w.graph == graph && w.task == Seek) {
                    found = true;
                    m_queue.removeAt(i);
                }
            }

            Work w;
            w.task = Seek;
            //we create a new graph
            w.graph = graph;
            w.id = m_currentWorkId++;
            w.time = time;
            m_queue.enqueue(w);
            m_waitCondition.set();
            return w.id;
        }

        quint16 WorkerThread::addUrlToRender(const QString &url)
        {
            QMutexLocker locker(&m_mutex);
            Work w;
            w.task = Render;

            //we create a new graph
            w.graph = Graph(CLSID_FilterGraph, IID_IGraphBuilder);
            w.url = url;
            w.url.detach();
            w.id = m_currentWorkId++;

            m_queue.enqueue(w);
            m_waitCondition.set();
            return w.id;
        }

        quint16 WorkerThread::addFilterToRender(const Filter &filter)
        {
            QMutexLocker locker(&m_mutex);
            Work w;
            w.task = Render;

            //we create a new graph
            w.graph = Graph(CLSID_FilterGraph, IID_IGraphBuilder);
            w.filter = filter;
            w.graph->AddFilter(filter, 0);
            w.id = m_currentWorkId++;

            m_queue.enqueue(w);
            m_waitCondition.set();
            return w.id;
        }

        void WorkerThread::replaceGraphForEventManagement(Graph newGraph, Graph oldGraph)
        {
            QMutexLocker locker(&m_mutex);
            Work w;
            w.task = ReplaceGraph;
            w.graph = newGraph;
            w.oldGraph = oldGraph;
            m_queue.enqueue(w);
            m_waitCondition.set();
        }

        void WorkerThread::handleTask()
        {
            QMutexLocker locker(Backend::directShowMutex);
            {
                QMutexLocker locker(&m_mutex);
                if (m_finished || m_queue.isEmpty()) {
                    return;
                }

                m_currentWork = m_queue.dequeue();

                // ensure to have the wait condition in the right state
                if (m_queue.isEmpty()) {
                    m_waitCondition.reset();

                } else {
                    m_waitCondition.set();
                }
            }

            HRESULT hr = S_OK;

            if (m_currentWork.task == ReplaceGraph) {
                int index = -1;

                for(int i = 0; i < FILTER_COUNT; ++i) {
                    if (m_graphHandle[i].graph == m_currentWork.oldGraph) {
                        m_graphHandle[i].graph = Graph();
                        index = i;
                        break;

                    } else if (index == -1 && m_graphHandle[i].graph == 0) {
                        //this is the first available slot
                        index = i;
                    }
                }

                Q_ASSERT(index != -1);

                //add the new graph
                HANDLE h;
                if (SUCCEEDED(ComPointer<IMediaEvent>(m_currentWork.graph, IID_IMediaEvent)
                    ->GetEventHandle(reinterpret_cast<OAEVENT*>(&h)))) {
                    m_graphHandle[index].graph = m_currentWork.graph;
                    m_graphHandle[index].handle = h;
                }

            } else if (m_currentWork.task == Render) {

                if (m_currentWork.filter) {
                    //let's render pins
                    const QList<OutputPin> outputs = BackendNode::pins(m_currentWork.filter, PINDIR_OUTPUT);
                    for (int i = 0; SUCCEEDED(hr) && i < outputs.count(); ++i) {
                        hr = m_currentWork.graph->Render(outputs.at(i));
                    }

                } else if (!m_currentWork.url.isEmpty()) {
                    //let's render a url (blocking call)
                    hr = m_currentWork.graph->RenderFile(reinterpret_cast<const wchar_t *>(m_currentWork.url.utf16()), 0);
                }

                if (hr != E_ABORT) {
                    emit asyncRenderFinished(m_currentWork.id, hr, m_currentWork.graph);
                }

            } else if (m_currentWork.task == Seek) {
                //that's a seekrequest
                ComPointer<IMediaSeeking> mediaSeeking(m_currentWork.graph, IID_IMediaSeeking);
                qint64 newtime = m_currentWork.time * 10000;
                hr = mediaSeeking->SetPositions(&newtime, AM_SEEKING_AbsolutePositioning,
                    0, AM_SEEKING_NoPositioning);

                emit asyncSeekingFinished(m_currentWork.id, newtime / 10000);
                hr = E_ABORT; //to avoid emitting asyncRenderFinished

            } else if (m_currentWork.task == ChangeState) {

                //remove useless decoders
                QList<Filter> unused;
                for (int i = 0; i < m_currentWork.decoders.count(); ++i) {
                    const Filter &filter = m_currentWork.decoders.at(i);
                    bool used = false;
                    const QList<OutputPin> pins = BackendNode::pins(filter, PINDIR_OUTPUT);
                    for( int i = 0; i < pins.count(); ++i) {
                        InputPin input;
                        if (pins.at(i)->ConnectedTo(input.pparam()) == S_OK) {
                            used = true;
                        }
                    }
                    if (!used) {
                        unused += filter;
                    }
                }

                // the state can be retrieved
                for (int i = 0; i < unused.count(); ++i) {
                    // remove this filter from the graph
                    m_currentWork.graph->RemoveFilter(unused.at(i));
                }


                // can get the state
                ComPointer<IMediaControl> mc(m_currentWork.graph, IID_IMediaControl);

                // change the state here
                switch(m_currentWork.state)
                {
                case State_Stopped:
                    mc->Stop();
                    break;
                case State_Paused:
                    mc->Pause();
                    break;
                case State_Running:
                    mc->Run();
                    break;
                }

                OAFilterState s;

                //blocking call
                HRESULT hr = mc->GetState(INFINITE, &s);         

                if (SUCCEEDED(hr)) {
                    if (s == State_Stopped) {
                        emit stateReady(m_currentWork.graph, Phonon::StoppedState);

                    } else if (s == State_Paused) {
                        emit stateReady(m_currentWork.graph, Phonon::PausedState);

                    } else /*if (s == State_Running)*/ {
                        emit stateReady(m_currentWork.graph, Phonon::PlayingState);
                    }
                }
            }

            {
                QMutexLocker locker(&m_mutex);
                m_currentWork = Work(); //reinitialize
            }
        }

        void WorkerThread::abortCurrentRender(qint16 renderId)
        {
            QMutexLocker locker(&m_mutex);
            if (m_currentWork.id == renderId) {
                m_currentWork.graph->Abort();
            }
            bool found = false;
            for(int i = 0; !found && i < m_queue.size(); ++i) {
                const Work &w = m_queue.at(i);
                if (w.id == renderId) {
                    found = true;
                    m_queue.removeAt(i);
                    if (m_queue.isEmpty()) {
                        m_waitCondition.reset();
                    }
                }
            }
        }

        //tells the thread to stop processing
        void WorkerThread::signalStop()
        {
            QMutexLocker locker(&m_mutex);
            m_queue.clear();
            if (m_currentWork.graph) {
                //in case we're currently rendering something
                m_currentWork.graph->Abort();

            }

            m_finished = true;
            m_waitCondition.set();
        }


        MediaObject::MediaObject(QObject *parent) : BackendNode(parent),
            transactionState(Phonon::StoppedState),
            m_errorType(Phonon::NoError),
            m_state(Phonon::LoadingState),
            m_nextState(Phonon::StoppedState),
            m_prefinishMark(0),
            m_tickInterval(0),
            m_buffering(false),
            m_oldHasVideo(false),
            m_prefinishMarkSent(false),
            m_aboutToFinishSent(false),
            m_nextSourceReadyToStart(false),
#ifndef QT_NO_PHONON_MEDIACONTROLLER
            m_autoplayTitles(true),
            m_currentTitle(0),
#endif
            m_targetTick(INFINITE)
        {

            for(int i = 0; i < FILTER_COUNT; ++i) {
                m_graphs[i] = new MediaGraph(this, i);
            }

            connect(&m_thread, SIGNAL(stateReady(Graph,Phonon::State)),            SLOT(slotStateReady(Graph,Phonon::State)));
            connect(&m_thread, SIGNAL(eventReady(Graph,long,long)),                SLOT(handleEvents(Graph,long,long)));
            connect(&m_thread, SIGNAL(asyncRenderFinished(quint16,HRESULT,Graph)), SLOT(finishLoading(quint16,HRESULT,Graph)));
            connect(&m_thread, SIGNAL(asyncSeekingFinished(quint16,qint64)),       SLOT(finishSeeking(quint16,qint64)));

            //really special case
            m_mediaObject = this;
            m_thread.start();
        }

        MediaObject::~MediaObject()
        {
            //be sure to finish the timer first
            m_tickTimer.stop();

            //we finish the worker thread here
            m_thread.signalStop();
            m_thread.wait();

            //and then we delete the graphs
            for (int i = 0; i < FILTER_COUNT; ++i) {
                delete m_graphs[i];
            }
        }

        WorkerThread *MediaObject::workerThread()
        {
            return &m_thread;
        }

        MediaGraph *MediaObject::currentGraph() const
        {
            return m_graphs[0];
        }

        MediaGraph *MediaObject::nextGraph() const
        {
            return m_graphs[FILTER_COUNT - 1];
        }

        //utility function to save the graph to a file
        void MediaObject::timerEvent(QTimerEvent *e)
        {
            if (e->timerId() == m_tickTimer.timerId()) {

                const qint64 current = currentTime();
                const qint64 total = totalTime();

                if ( m_tickInterval != 0 && current > m_targetTick) {
                    updateTargetTick();
                    emit tick(current);
                }

                //check that the title hasn't changed
#ifndef QT_NO_PHONON_MEDIACONTROLLER
                if (m_autoplayTitles && m_currentTitle < _iface_availableTitles() - 1) {

                    if (current >= total) {
                        //we go to the next title
                        _iface_setCurrentTitle(m_currentTitle + 1, false);
                        emit tick(current);
                    }
                    return;
                }
#endif //QT_NO_PHONON_MEDIACONTROLLER

                if (total) {
                    const qint64 remaining = total - current;

                    if (m_transitionTime < 0 && m_nextSourceReadyToStart) {
                        if (remaining < -m_transitionTime + TIMER_INTERVAL/2) {
                            //we need to switch graphs to run the next source in the queue (with cross-fading)
                            switchToNextSource();
                            return;
                        } else if (current < -m_transitionTime) {
                            //we are currently crossfading
                            for (int i = 0; i < m_audioOutputs.count(); ++i) {
                                m_audioOutputs.at(i)->setCrossFadingProgress( currentGraph()->index(), qMin( qreal(1.), qreal(current) / qreal(-m_transitionTime)));
                            }
                        }
                    }

                    if (m_prefinishMark > 0 && !m_prefinishMarkSent && remaining < m_prefinishMark + TIMER_INTERVAL/2) {
#ifdef GRAPH_DEBUG
                        qDebug() << "DS9: emit prefinishMarkReached" << remaining << QTime::currentTime().toString();
#endif
                        m_prefinishMarkSent = true;

                        emit prefinishMarkReached( remaining );
                    }

                    if (!m_aboutToFinishSent && remaining < PRELOAD_TIME - m_transitionTime  + TIMER_INTERVAL/2) {
                        //let's take a 2 seconds time to actually load the next file
#ifdef GRAPH_DEBUG
                        qDebug() << "DS9: emit aboutToFinish" << remaining << QTime::currentTime().toString();
#endif
                        m_aboutToFinishSent = true;
                        emit aboutToFinish();
                    }
                } else {
                    //total is 0: the stream is probably live (endless)
                }

                if (m_buffering) {
                    ComPointer<IAMNetworkStatus> status(currentGraph()->realSource(), IID_IAMNetworkStatus);
                    if (status) {
                        long l;
                        status->get_BufferingProgress(&l);
                        emit bufferStatus(l);
#ifdef GRAPH_DEBUG
                        qDebug() << "emit bufferStatus(" << l << ")";
#endif
                    }
                }
            }
        }

        void MediaObject::switchToNextSource()
        {
            m_prefinishMarkSent = false;
            m_aboutToFinishSent = false;
            m_nextSourceReadyToStart = false;

            m_oldHasVideo = currentGraph()->hasVideo();

            qSwap(m_graphs[0], m_graphs[1]); //swap the graphs

            if (m_transitionTime >= 0)
                m_graphs[1]->stop(); //make sure we stop the previous graph

            if (currentGraph()->mediaSource().type() != Phonon::MediaSource::Invalid &&
                catchComError(currentGraph()->renderResult())) {
                    setState(Phonon::ErrorState);
                    return;
            }

            //we need to play the next media
            play();

            //we tell the video widgets to switch now to the new source
#ifndef QT_NO_PHONON_VIDEO
            for (int i = 0; i < m_videoWidgets.count(); ++i) {
                m_videoWidgets.at(i)->setCurrentGraph(currentGraph()->index());
            }
#endif 

            emit currentSourceChanged(currentGraph()->mediaSource());
            emit metaDataChanged(currentGraph()->metadata());

            if (nextGraph()->hasVideo() != currentGraph()->hasVideo()) {
                emit hasVideoChanged(currentGraph()->hasVideo());
            }

            emit tick(0);
            emit totalTimeChanged(totalTime());

#ifndef QT_NO_PHONON_MEDIACONTROLLER
            setTitles(currentGraph()->titles());
#endif
        }

        Phonon::State MediaObject::state() const
        {
            if (m_buffering) {
                return Phonon::BufferingState;
            } else {
                return m_state;
            }
        }

        bool MediaObject::hasVideo() const
        {
            return currentGraph()->hasVideo();
        }

        bool MediaObject::isSeekable() const
        {
            return currentGraph()->isSeekable();
        }

        qint64 MediaObject::totalTime() const
        {
#ifndef QT_NO_PHONON_MEDIACONTROLLER
            //1st, check if there is more titles after
            const qint64 ret = (m_currentTitle < _iface_availableTitles() - 1) ?
                titleAbsolutePosition(m_currentTitle+1) : currentGraph()->absoluteTotalTime();

            //this is the duration of the current title
            return ret - titleAbsolutePosition(m_currentTitle);
#else
            return currentGraph()->absoluteTotalTime();
#endif //QT_NO_PHONON_MEDIACONTROLLER
        }

        qint64 MediaObject::currentTime() const
        {
            //this handles inaccuracy when stopping on a title
            return currentGraph()->absoluteCurrentTime()
#ifndef QT_NO_PHONON_MEDIACONTROLLER
                - titleAbsolutePosition(m_currentTitle)
#endif //QT_NO_PHONON_MEDIACONTROLLER
                ;
        }

        qint32 MediaObject::tickInterval() const
        {
            return m_tickInterval;
        }

        void MediaObject::setTickInterval(qint32 newTickInterval)
        {
            m_tickInterval = newTickInterval;
            updateTargetTick();
        }

        void MediaObject::pause()
        {
            if (currentGraph()->isLoading()) {
                m_nextState = Phonon::PausedState;
            } else {
                currentGraph()->pause();
            }
        }

        void MediaObject::stop()
        {
            if (currentGraph()->isLoading()) {
                m_nextState = Phonon::StoppedState;
            } else {
                currentGraph()->stop();
            }
        }

        void MediaObject::ensureStopped()
        {
            currentGraph()->ensureStopped();
            if (m_state == Phonon::ErrorState) {
                // reset the state here
                m_state = Phonon::StoppedState;
            }
        }

        void MediaObject::play()
        {
            if (currentGraph()->isLoading()) {
                m_nextState = Phonon::PlayingState;
            } else {
                currentGraph()->play();
            }
        }

        QString MediaObject::errorString() const
        {
            return m_errorString;
        }

        Phonon::ErrorType MediaObject::errorType() const
        {
            return m_errorType;
        }


        void MediaObject::updateTargetTick()
        {
            if (m_tickInterval) {
                const qint64 current = currentTime();
                m_targetTick = current / m_tickInterval * m_tickInterval;
                if (current == 0 || m_targetTick < current) {
                    m_targetTick += m_tickInterval;
                }
            }
        }

        void MediaObject::setState(Phonon::State newstate)
        {

            if (newstate == Phonon::PlayingState) {
                updateTargetTick();
            }

            if (newstate == m_state) {
                return;
            }

            // manage the timer
            if (newstate == Phonon::PlayingState) {
                m_tickTimer.start(TIMER_INTERVAL, this);

            } else {
                m_tickTimer.stop();

            }

            Phonon::State oldstate = state();
            m_state = newstate;
            emit stateChanged(newstate, oldstate);
        }


        qint32 MediaObject::prefinishMark() const
        {
            return m_prefinishMark;
        }

        void MediaObject::setPrefinishMark(qint32 newPrefinishMark)
        {
            m_prefinishMark = newPrefinishMark;
        }

        qint32 MediaObject::transitionTime() const
        {
            return m_transitionTime;
        }

        void MediaObject::setTransitionTime(qint32 time)
        {
            m_transitionTime = time;
        }

        qint64 MediaObject::remainingTime() const
        {
            return totalTime() - currentTime();
        }


        Phonon::MediaSource MediaObject::source() const
        {
            return currentGraph()->mediaSource();
        }

        void MediaObject::setNextSource(const Phonon::MediaSource &source)
        {
            m_nextSourceReadyToStart = true;
            const bool shouldSwitch = (m_state == Phonon::StoppedState || m_state == Phonon::ErrorState);
            nextGraph()->loadSource(source);    // preload the source

            if (shouldSwitch) {
                switchToNextSource();
            }
        }

        void MediaObject::setSource(const Phonon::MediaSource &source)
        {
            m_nextSourceReadyToStart = false;
            m_prefinishMarkSent = false;
            m_aboutToFinishSent = false;

            m_oldHasVideo = currentGraph()->hasVideo();
            setState(Phonon::LoadingState);

            // after loading go into stopped state
            m_nextState = Phonon::StoppedState;

            catchComError(currentGraph()->loadSource(source));
            emit currentSourceChanged(source);
        }

        void MediaObject::slotStateReady(Graph graph, Phonon::State newState)
        {
            if (graph == currentGraph()->graph() && !currentGraph()->isLoading()) {
                setState(newState);
            }
        }

        void MediaObject::loadingFinished(MediaGraph *mg)
        {                   
            if (mg == currentGraph()) {

#ifndef QT_NO_PHONON_MEDIACONTROLLER
                //Title interface
                m_currentTitle = 0;
                setTitles(currentGraph()->titles());
#endif 

                HRESULT hr = mg->renderResult();

                if (catchComError(hr)) {
                    return;
                }

                if (m_oldHasVideo != currentGraph()->hasVideo()) {
                    emit hasVideoChanged(currentGraph()->hasVideo());
                }

#ifndef QT_NO_PHONON_VIDEO
                if (currentGraph()->hasVideo()) {
                    updateVideoGeometry();
                }
#endif

                emit metaDataChanged(currentGraph()->metadata());
                emit totalTimeChanged(totalTime());

                // put the next state
                switch(m_nextState)
                {
                case Phonon::PausedState:
                    pause();
                    break;

                case Phonon::PlayingState:
                    play();
                    break;

                case Phonon::ErrorState:
                    setState(Phonon::ErrorState);
                    break;

                case Phonon::StoppedState:
                default:
                    stop();
                    break;
                }
            }
        }

        void MediaObject::seek(qint64 time)
        {
            // seek into the current title
            currentGraph()->absoluteSeek(time

#ifndef QT_NO_PHONON_MEDIACONTROLLER
                + titleAbsolutePosition(m_currentTitle)
#endif
                );
        }

        void MediaObject::seekingFinished(MediaGraph *mg)
        {
            if (mg == currentGraph()) {

                updateTargetTick();
                if (currentTime() < totalTime() - m_prefinishMark) {
                    m_prefinishMarkSent = false;
                }

                if (currentTime() < totalTime() - PRELOAD_TIME + m_transitionTime) {
                    m_aboutToFinishSent = false;
                }

                // this helps the update of the application (seekslider for example)
                if (m_state == PausedState || m_state == PlayingState) {
                    emit tick(currentTime());
                }
            }
        }


        bool MediaObject::catchComError(HRESULT hr)
        {
            m_errorString.clear();
            m_errorType = Phonon::NoError;

            if (hr != S_OK) {

#ifdef GRAPH_DEBUG
                qWarning("An error occurred 0x%x",hr);
#endif

                LPAMGETERRORTEXT getErrorText = (LPAMGETERRORTEXT)QLibrary::resolve(QLatin1String("quartz"), "AMGetErrorTextW");

                WCHAR buffer[MAX_ERROR_TEXT_LEN];
                if (getErrorText && getErrorText(hr, buffer, MAX_ERROR_TEXT_LEN)) {
                    m_errorString = QString::fromWCharArray(buffer);
                } else {
                    m_errorString = QString::fromLatin1("Unknown error");
                }

                const QString comError = QString::number(uint(hr), 16);
                if (!m_errorString.toLower().contains(comError.toLower())) {
                    m_errorString += QString::fromLatin1(" (0x%1)").arg(comError);
                }

                if (FAILED(hr)) {
                    m_errorType = Phonon::FatalError;
                    setState(Phonon::ErrorState);
                } else {
                    m_errorType = Phonon::NormalError;
                    m_nextState = Phonon::ErrorState;
                }

            } else {
                m_errorType = Phonon::NoError;

            }

            return m_errorType == Phonon::FatalError;
        }


        void MediaObject::grabNode(BackendNode *node)
        {
            for (int i = 0; i < FILTER_COUNT; ++i) {
                m_graphs[i]->grabNode(node);
            }
            node->setMediaObject(this);
        }

        bool MediaObject::connectNodes(BackendNode *source, BackendNode *sink)
        {
            bool ret = true;

            for (int i = 0; i < FILTER_COUNT; ++i) {
                ret = ret && m_graphs[i]->connectNodes(source, sink);
            }

            if (ret) {

#ifndef QT_NO_PHONON_VIDEO
                if (VideoWidget *video = qobject_cast<VideoWidget*>(sink)) {
                    m_videoWidgets += video;

                } else
#endif

                    if (AudioOutput *audio = qobject_cast<AudioOutput*>(sink)) {
                       m_audioOutputs += audio;
                   }
            }

            return ret;
        }

        bool MediaObject::disconnectNodes(BackendNode *source, BackendNode *sink)
        {
            bool ret = true;
            for (int i = 0; i < FILTER_COUNT; ++i) {
                ret = ret && m_graphs[i]->disconnectNodes(source, sink);
            }

            if (ret) {

#ifndef QT_NO_PHONON_VIDEO
                if (VideoWidget *video = qobject_cast<VideoWidget*>(sink)) {
                    m_videoWidgets.removeOne(video);
                } else
#endif
                    if (AudioOutput *audio = qobject_cast<AudioOutput*>(sink)) {
                        m_audioOutputs.removeOne(audio);
                   }
            }
            return ret;
        }

#ifndef QT_NO_PHONON_VIDEO
        void MediaObject::updateVideoGeometry()
        {
            for (int i = 0; i < m_videoWidgets.count(); ++i) {
                m_videoWidgets.at(i)->notifyVideoLoaded();
            }
        }
#endif //QT_NO_PHONON_VIDEO

        void MediaObject::handleComplete(IGraphBuilder *graph)
        {
            if (graph == currentGraph()->graph()) {
                if (m_transitionTime >= PRELOAD_TIME || m_aboutToFinishSent == false) {
                    emit aboutToFinish(); //give a chance to the frontend to give a next source
                    m_aboutToFinishSent = true;
                }

                if (!m_nextSourceReadyToStart) {
                    //this is the last source, we simply finish
                    const qint64 current = currentTime();
                    const OAFilterState currentState = currentGraph()->syncGetRealState();

                    emit tick(current); //this ensures that the end of the seek slider is reached
                    emit finished();

                    if (currentTime() == current && currentGraph()->syncGetRealState() == currentState) {
                        //no seek operation in-between
                        pause();
                        setState(Phonon::PausedState); //we set it here
                    }

                } else if (m_transitionTime == 0) {
                    //gapless transition
                    switchToNextSource(); //let's call the function immediately
                } else if (m_transitionTime > 0) {
                    //management of the transition (if it is >= 0)
                    QTimer::singleShot(m_transitionTime, this, SLOT(switchToNextSource()));
                }
            } else {
                //it is just the end of the previous source (in case of cross-fading)
                nextGraph()->cleanup();
            }
            for (int i = 0; i < m_audioOutputs.count(); ++i) {
                m_audioOutputs.at(i)->setCrossFadingProgress( currentGraph()->index(), 1.); //cross-fading is in any case finished
            }
        }

        void MediaObject::finishLoading(quint16 workId, HRESULT hr, Graph graph)
        {
            for(int i = 0; i < FILTER_COUNT; ++i) {
                m_graphs[i]->finishLoading(workId, hr, graph);
            }
        }

        void MediaObject::finishSeeking(quint16 workId, qint64 time)
        {
            for(int i = 0; i < FILTER_COUNT; ++i) {
                m_graphs[i]->finishSeeking(workId, time);
            }
        }


        void MediaObject::handleEvents(Graph graph,long eventCode,long param1)
        {
            QString eventDescription;
            switch (eventCode)
            {
            case EC_BUFFERING_DATA:
                if (graph == currentGraph()->graph()) {
                    m_buffering = param1;
                    emit stateChanged(state(), m_state);
                }
                break;
            case EC_LENGTH_CHANGED:
                if (graph == currentGraph()->graph()) {
                    emit totalTimeChanged( totalTime() );
                }
                break;

            case EC_COMPLETE:
                handleComplete(graph);
                break;

#ifndef QT_NO_PHONON_VIDEO
            case EC_VIDEO_SIZE_CHANGED:
                if (graph == currentGraph()->graph()) {
                    updateVideoGeometry();
                }
                break;
#endif //QT_NO_PHONON_VIDEO

#ifdef GRAPH_DEBUG
            case EC_ACTIVATE: qDebug() << "EC_ACTIVATE: A video window is being " << (param1 ? "ACTIVATED" : "DEACTIVATED"); break;
            case EC_BUILT: qDebug() << "EC_BUILT: Send by the Video Control when a graph has been built. Not forwarded to applications."; break;
            case EC_CLOCK_CHANGED: qDebug() << "EC_CLOCK_CHANGED"; break;
            case EC_CLOCK_UNSET: qDebug() << "EC_CLOCK_UNSET: The clock provider was disconnected."; break;
            case EC_CODECAPI_EVENT: qDebug() << "EC_CODECAPI_EVENT: Sent by an encoder to signal an encoding event."; break;
            case EC_DEVICE_LOST: qDebug() << "EC_DEVICE_LOST: A Plug and Play device was removed or has become available again."; break;
            case EC_DISPLAY_CHANGED: qDebug() << "EC_DISPLAY_CHANGED: The display mode has changed."; break;
            case EC_END_OF_SEGMENT: qDebug() << "EC_END_OF_SEGMENT: The end of a segment has been reached."; break;
            case EC_ERROR_STILLPLAYING: qDebug() << "EC_ERROR_STILLPLAYING: An asynchronous command to run the graph has failed."; break;
            case EC_ERRORABORT: qDebug() << "EC_ERRORABORT: An operation was aborted because of an error."; break;
            case EC_EXTDEVICE_MODE_CHANGE: qDebug() << "EC_EXTDEVICE_MODE_CHANGE: Not supported."; break;
            case EC_FULLSCREEN_LOST: qDebug() << "EC_FULLSCREEN_LOST: The video renderer is switching out of full-screen mode."; break;
            case EC_GRAPH_CHANGED: qDebug() << "EC_GRAPH_CHANGED: The filter graph has changed."; break;
            case EC_NEED_RESTART: qDebug() << "EC_NEED_RESTART: A filter is requesting that the graph be restarted."; break;
            case EC_NOTIFY_WINDOW: qDebug() << "EC_NOTIFY_WINDOW: Notifies a filter of the video renderer's window."; break;
            case EC_OLE_EVENT: qDebug() << "EC_OLE_EVENT: A filter is passing a text string to the application."; break;
            case EC_OPENING_FILE: qDebug() << "EC_OPENING_FILE: The graph is opening a file, or has finished opening a file."; break;
            case EC_PALETTE_CHANGED: qDebug() << "EC_PALETTE_CHANGED: The video palette has changed."; break;
            case EC_PAUSED: qDebug() << "EC_PAUSED: A pause request has completed."; break;
            case EC_PREPROCESS_COMPLETE: qDebug() << "EC_PREPROCESS_COMPLETE: Sent by the WM ASF Writer filter when it completes the pre-processing for multipass encoding."; break;
            case EC_QUALITY_CHANGE: qDebug() << "EC_QUALITY_CHANGE: The graph is dropping samples, for quality control."; break;
            case EC_REPAINT: qDebug() << "EC_REPAINT: A video renderer requires a repaint."; break;
            case EC_SEGMENT_STARTED: qDebug() << "EC_SEGMENT_STARTED: A new segment has started."; break;
            case EC_SHUTTING_DOWN: qDebug() << "EC_SHUTTING_DOWN: The filter graph is shutting down, prior to being destroyed."; break;
            case EC_SNDDEV_IN_ERROR: qDebug() << "EC_SNDDEV_IN_ERROR: A device error has occurred in an audio capture filter."; break;
            case EC_SNDDEV_OUT_ERROR: qDebug() << "EC_SNDDEV_OUT_ERROR: A device error has occurred in an audio renderer filter."; break;
            case EC_STARVATION: qDebug() << "EC_STARVATION: A filter is not receiving enough data."; break;
            case EC_STATE_CHANGE: qDebug() << "EC_STATE_CHANGE: The filter graph has changed state."; break;
            case EC_STEP_COMPLETE: qDebug() << "EC_STEP_COMPLETE: A filter performing frame stepping has stepped the specified number of frames."; break;
            case EC_STREAM_CONTROL_STARTED: qDebug() << "EC_STREAM_CONTROL_STARTED: A stream-control start command has taken effect."; break;
            case EC_STREAM_CONTROL_STOPPED: qDebug() << "EC_STREAM_CONTROL_STOPPED: A stream-control stop command has taken effect."; break;
            case EC_STREAM_ERROR_STILLPLAYING: qDebug() << "EC_STREAM_ERROR_STILLPLAYING: An error has occurred in a stream. The stream is still playing."; break;
            case EC_STREAM_ERROR_STOPPED: qDebug() << "EC_STREAM_ERROR_STOPPED: A stream has stopped because of an error."; break;
            case EC_TIMECODE_AVAILABLE: qDebug() << "EC_TIMECODE_AVAILABLE: Not supported."; break;
            case EC_UNBUILT: qDebug() << "Sent by the Video Control when a graph has been torn down. Not forwarded to applications."; break;
            case EC_USERABORT: qDebug() << "EC_USERABORT: Send by the Video Control when a graph has been torn down. Not forwarded to applications."; break;
            case EC_VMR_RECONNECTION_FAILED: qDebug() << "EC_VMR_RECONNECTION_FAILED: Sent by the VMR-7 and the VMR-9 when it was unable to accept a dynamic format change request from the upstream decoder."; break;
            case EC_VMR_RENDERDEVICE_SET: qDebug() << "EC_VMR_RENDERDEVICE_SET: Sent when the VMR has selected its rendering mechanism."; break;
            case EC_VMR_SURFACE_FLIPPED: qDebug() << "EC_VMR_SURFACE_FLIPPED: Sent when the VMR-7's allocator presenter has called the DirectDraw Flip method on the surface being presented."; break;
            case EC_WINDOW_DESTROYED: qDebug() << "EC_WINDOW_DESTROYED: The video renderer was destroyed or removed from the graph"; break;
            case EC_WMT_EVENT: qDebug() << "EC_WMT_EVENT: Sent by the Windows Media Format SDK when an application uses the ASF Reader filter to play ASF files protected by digital rights management (DRM)."; break;
            case EC_WMT_INDEX_EVENT: qDebug() << "EC_WMT_INDEX_EVENT: Sent by the Windows Media Format SDK when an application uses the ASF Writer to index Windows Media Video files."; break;

                //documented by Microsoft but not supported in the Platform SDK
                //              case EC_BANDWIDTHCHANGE : qDebug() << "EC_BANDWIDTHCHANGE: not supported"; break;
                //              case EC_CONTENTPROPERTY_CHANGED: qDebug() << "EC_CONTENTPROPERTY_CHANGED: not supported."; break;
                //              case EC_EOS_SOON: qDebug() << "EC_EOS_SOON: not supported"; break;
                //              case EC_ERRORABORTEX: qDebug() << "EC_ERRORABORTEX: An operation was aborted because of an error."; break;
                //              case EC_FILE_CLOSED: qDebug() << "EC_FILE_CLOSED: The source file was closed because of an unexpected event."; break;
                //              case EC_LOADSTATUS: qDebug() << "EC_LOADSTATUS: Notifies the application of progress when opening a network file."; break;
                //              case EC_MARKER_HIT: qDebug() << "EC_MARKER_HIT: not supported."; break;
                //              case EC_NEW_PIN: qDebug() << "EC_NEW_PIN: not supported."; break;
                //              case EC_PLEASE_REOPEN: qDebug() << "EC_PLEASE_REOPEN: The source file has changed."; break;
                //              case EC_PROCESSING_LATENCY: qDebug() << "EC_PROCESSING_LATENCY: Indicates the amount of time that a component is taking to process each sample."; break;
                //              case EC_RENDER_FINISHED: qDebug() << "EC_RENDER_FINISHED: Not supported."; break;
                //              case EC_SAMPLE_LATENCY: qDebug() << "EC_SAMPLE_LATENCY: Specifies how far behind schedule a component is for processing samples."; break;
                //              case EC_SAMPLE_NEEDED: qDebug() << "EC_SAMPLE_NEEDED: Requests a new input sample from the Enhanced Video Renderer (EVR) filter."; break;
                //              case EC_SCRUB_TIME: qDebug() << "EC_SCRUB_TIME: Specifies the time stamp for the most recent frame step."; break;
                //              case EC_STATUS: qDebug() << "EC_STATUS: Contains two arbitrary status strings."; break;
                //              case EC_VIDEOFRAMEREADY: qDebug() << "EC_VIDEOFRAMEREADY: A video frame is ready for display."; break;

            default:
                qDebug() << "Unknown event" << eventCode << "(" << param1 << ")";
                break;
#else
            default:
                break;
#endif
            }
        }


#ifndef QT_NO_PHONON_MEDIACONTROLLER
        //interface management
        bool MediaObject::hasInterface(Interface iface) const
        {
            return iface == AddonInterface::TitleInterface;
        }

        QVariant MediaObject::interfaceCall(Interface iface, int command, const QList<QVariant> &params)
        {
            if (hasInterface(iface)) {

                switch (iface)
                {
                case TitleInterface:
                    switch (command)
                    {
                    case availableTitles:
                        return _iface_availableTitles();
                    case title:
                        return _iface_currentTitle();
                    case setTitle:
                        _iface_setCurrentTitle(params.first().toInt());
                        break;
                    case autoplayTitles:
                        return m_autoplayTitles;
                    case setAutoplayTitles:
                        m_autoplayTitles = params.first().toBool();
                        updateStopPosition();
                        break;
                    }
                    break;
                default:
                    break;
                }
            }
            return QVariant();
        }


        //TitleInterface

        //this is called to set the time for the different titles
        qint64 MediaObject::titleAbsolutePosition(int title) const
        {
            if (title >= 0 && title < m_titles.count()) {
                return m_titles.at(title);
            } else {
                return 0;
            }
        }

        void MediaObject::setTitles(const QList<qint64> &titles)
        {
            //this is called when the source is loaded
            const bool emitSignal = m_titles.count() != titles.count();
            m_titles = titles;
            if (emitSignal) {
                emit availableTitlesChanged(titles.count());
            }
            updateStopPosition();
        }


        int MediaObject::_iface_availableTitles() const
        {
            return m_titles.count() - 1;
        }

        int MediaObject::_iface_currentTitle() const
        {
            return m_currentTitle;
        }

        void MediaObject::_iface_setCurrentTitle(int title, bool bseek)
        {
#ifdef GRAPH_DEBUG
            qDebug() << "_iface_setCurrentTitle" << title;
#endif
            const int oldTitle = m_currentTitle;
            m_currentTitle = title;
            updateStopPosition();
            if (bseek) {
                //let's seek to the beginning of the song
                seek(0);
            } else {
                updateTargetTick();
            }
            if (oldTitle != title) {
                emit titleChanged(title);
                emit totalTimeChanged(totalTime());
            }

        }

        void MediaObject::updateStopPosition()
        {
            if (!m_autoplayTitles && m_currentTitle < _iface_availableTitles() - 1) {
                //stop position is set to the end of the track
                currentGraph()->setStopPosition(titleAbsolutePosition(m_currentTitle+1));
            } else {
                //stop position is set to the end
                currentGraph()->setStopPosition(-1);
            }
        }
#endif

        void MediaObject::switchFilters(int index, Filter oldFilter, Filter newFilter)
        {

            if (currentGraph()->index() == index) {
                currentGraph()->switchFilters(oldFilter, newFilter);
            } else {
                nextGraph()->switchFilters(oldFilter, newFilter);
            }

        }


    }
}

QT_END_NAMESPACE
