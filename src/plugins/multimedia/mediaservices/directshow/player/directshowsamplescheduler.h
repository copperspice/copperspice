/***********************************************************************
*
* Copyright (c) 2012-2024 Barbara Geller
* Copyright (c) 2012-2024 Ansel Sermersheim
*
* Copyright (c) 2015 The Qt Company Ltd.
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

#ifndef DIRECTSHOWSAMPLESCHEDULER_H
#define DIRECTSHOWSAMPLESCHEDULER_H

#include <dshow.h>

#include <qmutex.h>
#include <qobject.h>
#include <qsemaphore.h>

class DirectShowTimedSample;

class DirectShowSampleScheduler : public QObject, public IMemInputPin
{
   CS_OBJECT(DirectShowSampleScheduler)
 public:

   enum State {
      Stopped  = 0x00,
      Running  = 0x01,
      Paused   = 0x02,
      RunMask  = 0x03,
      Flushing = 0x04
   };

   DirectShowSampleScheduler(IUnknown *pin, QObject *parent = nullptr);
   ~DirectShowSampleScheduler();

   // IUnknown
   HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void **ppvObject) override;
   ULONG STDMETHODCALLTYPE AddRef() override;
   ULONG STDMETHODCALLTYPE Release() override;

   // IMemInputPin
   HRESULT STDMETHODCALLTYPE GetAllocator(IMemAllocator **ppAllocator) override;
   HRESULT STDMETHODCALLTYPE NotifyAllocator(IMemAllocator *pAllocator, BOOL bReadOnly) override;
   HRESULT STDMETHODCALLTYPE GetAllocatorRequirements(ALLOCATOR_PROPERTIES *pProps) override;

   HRESULT STDMETHODCALLTYPE Receive(IMediaSample *pSample) override;
   HRESULT STDMETHODCALLTYPE ReceiveMultiple(IMediaSample **pSamples, long nSamples, long *nSamplesProcessed) override;
   HRESULT STDMETHODCALLTYPE ReceiveCanBlock() override;

   void run(REFERENCE_TIME startTime);
   void pause();
   void stop();
   void setFlushing(bool flushing);

   IReferenceClock *clock() const {
      return m_clock;
   }
   void setClock(IReferenceClock *clock);

   bool schedule(IMediaSample *sample);
   bool scheduleEndOfStream();

   IMediaSample *takeSample(bool *eos);

   bool event(QEvent *event) override;

 public:
   CS_SIGNAL_1(Public, void sampleReady())
   CS_SIGNAL_2(sampleReady)

 private:
   IUnknown *m_pin;
   IReferenceClock *m_clock;
   IMemAllocator *m_allocator;
   DirectShowTimedSample *m_head;
   DirectShowTimedSample *m_tail;
   int m_maximumSamples;
   int m_state;
   REFERENCE_TIME m_startTime;
   HANDLE m_timeoutEvent;
   HANDLE m_flushEvent;
   QSemaphore m_semaphore;
   QMutex m_mutex;
};

#endif
