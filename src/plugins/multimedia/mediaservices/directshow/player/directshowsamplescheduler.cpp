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

#include "directshowsamplescheduler.h"

#include <qcoreapplication.h>
#include <qcoreevent.h>

class DirectShowTimedSample
{
 public:
   DirectShowTimedSample(IMediaSample *sample)
      : m_next(nullptr)
      , m_sample(sample)
      , m_cookie(0)
      , m_lastSample(false) {
      m_sample->AddRef();
   }

   ~DirectShowTimedSample() {
      m_sample->Release();
   }

   IMediaSample *sample() const {
      return m_sample;
   }

   DirectShowTimedSample *nextSample() const {
      return m_next;
   }
   void setNextSample(DirectShowTimedSample *sample) {
      Q_ASSERT(!m_next);
      m_next = sample;
   }

   DirectShowTimedSample *remove() {
      DirectShowTimedSample *next = m_next;
      delete this;
      return next;
   }

   bool schedule(IReferenceClock *clock, REFERENCE_TIME startTime, HANDLE handle);
   void unschedule(IReferenceClock *clock);

   bool isReady(IReferenceClock *clock) const;

   bool isLast() const {
      return m_lastSample;
   }
   void setLast() {
      m_lastSample = true;
   }

 private:
   DirectShowTimedSample *m_next;
   IMediaSample *m_sample;
   DWORD_PTR m_cookie;
   bool m_lastSample;
};

bool DirectShowTimedSample::schedule(
   IReferenceClock *clock, REFERENCE_TIME startTime, HANDLE handle)
{
   REFERENCE_TIME sampleStartTime;
   REFERENCE_TIME sampleEndTime;
   if (m_sample->GetTime(&sampleStartTime, &sampleEndTime) == S_OK) {
      if (clock->AdviseTime(
            startTime, sampleStartTime, reinterpret_cast<HEVENT>(handle), &m_cookie) == S_OK) {
         return true;
      }
   }
   return false;
}

void DirectShowTimedSample::unschedule(IReferenceClock *clock)
{
   clock->Unadvise(m_cookie);
}

bool DirectShowTimedSample::isReady(IReferenceClock *clock) const
{
   REFERENCE_TIME sampleStartTime;
   REFERENCE_TIME sampleEndTime;
   REFERENCE_TIME currentTime;
   if (m_sample->GetTime(&sampleStartTime, &sampleEndTime) == S_OK) {
      if (clock->GetTime(&currentTime) == S_OK) {
         return currentTime >= sampleStartTime;
      }
   }
   return true;
}

DirectShowSampleScheduler::DirectShowSampleScheduler(IUnknown *pin, QObject *parent)
   : QObject(parent)
   , m_pin(pin)
   , m_clock(nullptr)
   , m_allocator(nullptr)
   , m_head(nullptr)
   , m_tail(nullptr)
   , m_maximumSamples(1)
   , m_state(Stopped)
   , m_startTime(0)
   , m_timeoutEvent(::CreateEvent(nullptr, 0, 0, nullptr))
   , m_flushEvent(::CreateEvent(nullptr, 0, 0, nullptr))
{
   m_semaphore.release(m_maximumSamples);
}

DirectShowSampleScheduler::~DirectShowSampleScheduler()
{
   ::CloseHandle(m_timeoutEvent);
   ::CloseHandle(m_flushEvent);

   Q_ASSERT(!m_clock);
   Q_ASSERT(!m_allocator);
}

HRESULT DirectShowSampleScheduler::QueryInterface(REFIID riid, void **ppvObject)
{
   return m_pin->QueryInterface(riid, ppvObject);
}

ULONG DirectShowSampleScheduler::AddRef()
{
   return m_pin->AddRef();
}

ULONG DirectShowSampleScheduler::Release()
{
   return m_pin->Release();
}

// IMemInputPin
HRESULT DirectShowSampleScheduler::GetAllocator(IMemAllocator **ppAllocator)
{
   if (!ppAllocator) {
      return E_POINTER;
   } else {
      QMutexLocker locker(&m_mutex);

      if (!m_allocator) {
         return VFW_E_NO_ALLOCATOR;
      } else {
         *ppAllocator = m_allocator;

         return S_OK;
      }
   }
}

HRESULT DirectShowSampleScheduler::NotifyAllocator(IMemAllocator *pAllocator, BOOL bReadOnly)
{
   (void) bReadOnly;

   HRESULT hr;
   ALLOCATOR_PROPERTIES properties;

   if (!pAllocator) {
      if (m_allocator) {
         m_allocator->Release();
      }

      m_allocator = nullptr;

      return S_OK;
   } else if ((hr = pAllocator->GetProperties(&properties)) != S_OK) {
      return hr;
   } else {
      if (properties.cBuffers == 1) {
         ALLOCATOR_PROPERTIES actual;

         properties.cBuffers = 2;
         if ((hr = pAllocator->SetProperties(&properties, &actual)) != S_OK) {
            return hr;
         }
      }

      QMutexLocker locker(&m_mutex);

      if (m_allocator) {
         m_allocator->Release();
      }

      m_allocator = pAllocator;
      m_allocator->AddRef();

      return S_OK;
   }
}

HRESULT DirectShowSampleScheduler::GetAllocatorRequirements(ALLOCATOR_PROPERTIES *pProps)
{
   if (!pProps) {
      return E_POINTER;
   }

   pProps->cBuffers = 2;

   return S_OK;
}

HRESULT DirectShowSampleScheduler::Receive(IMediaSample *pSample)
{
   if (!pSample) {
      return E_POINTER;
   }

   m_semaphore.acquire(1);

   QMutexLocker locker(&m_mutex);

   if (m_state & Flushing) {
      m_semaphore.release(1);

      return S_FALSE;
   } else if (m_state == Stopped) {
      m_semaphore.release();

      return VFW_E_WRONG_STATE;
   } else {
      DirectShowTimedSample *timedSample = new DirectShowTimedSample(pSample);

      if (m_tail) {
         m_tail->setNextSample(timedSample);
      } else {
         m_head = timedSample;
      }

      m_tail = timedSample;

      if (m_state == Running) {
         if (!timedSample->schedule(m_clock, m_startTime, m_timeoutEvent)) {
            // Timing information is unavailable, so schedule frames immediately.
            QCoreApplication::postEvent(this, new QEvent(QEvent::UpdateRequest));
         } else {
            locker.unlock();
            HANDLE handles[] = { m_flushEvent, m_timeoutEvent };
            DWORD result = ::WaitForMultipleObjects(2, handles, FALSE, INFINITE);
            locker.relock();

            if (result == WAIT_OBJECT_0 + 1) {
               QCoreApplication::postEvent(this, new QEvent(QEvent::UpdateRequest));
            }
         }
      } else if (m_tail == m_head) {
         // If this is the first frame make it available.
         QCoreApplication::postEvent(this, new QEvent(QEvent::UpdateRequest));

         if (m_state == Paused) {
            ::ResetEvent(m_timeoutEvent);

            locker.unlock();
            HANDLE handles[] = { m_flushEvent, m_timeoutEvent };
            ::WaitForMultipleObjects(2, handles, FALSE, INFINITE);
            locker.relock();
         }
      }

      return S_OK;
   }
}

HRESULT DirectShowSampleScheduler::ReceiveMultiple(
   IMediaSample **pSamples, long nSamples, long *nSamplesProcessed)
{
   if (!pSamples || !nSamplesProcessed) {
      return E_POINTER;
   }

   for (*nSamplesProcessed = 0; *nSamplesProcessed < nSamples; ++(*nSamplesProcessed)) {
      HRESULT hr = Receive(pSamples[*nSamplesProcessed]);

      if (hr != S_OK) {
         return hr;
      }
   }
   return S_OK;
}

HRESULT DirectShowSampleScheduler::ReceiveCanBlock()
{
   return S_OK;
}

void DirectShowSampleScheduler::run(REFERENCE_TIME startTime)
{
   QMutexLocker locker(&m_mutex);

   m_state = (m_state & Flushing) | Running;
   m_startTime = startTime;

   for (DirectShowTimedSample *sample = m_head; sample; sample = sample->nextSample()) {
      sample->schedule(m_clock, m_startTime, m_timeoutEvent);
   }

   if (!(m_state & Flushing)) {
      ::ResetEvent(m_flushEvent);
   }

   if (!m_head) {
      ::SetEvent(m_timeoutEvent);
   }

}

void DirectShowSampleScheduler::pause()
{
   QMutexLocker locker(&m_mutex);

   m_state = (m_state & Flushing) | Paused;

   for (DirectShowTimedSample *sample = m_head; sample; sample = sample->nextSample()) {
      sample->unschedule(m_clock);
   }

   if (!(m_state & Flushing)) {
      ::ResetEvent(m_flushEvent);
   }
}

void DirectShowSampleScheduler::stop()
{
   QMutexLocker locker(&m_mutex);

   m_state = m_state & Flushing;

   for (DirectShowTimedSample *sample = m_head; sample; sample = sample->remove()) {
      sample->unschedule(m_clock);

      m_semaphore.release(1);
   }

   m_head = nullptr;
   m_tail = nullptr;

   ::SetEvent(m_flushEvent);
}

void DirectShowSampleScheduler::setFlushing(bool flushing)
{
   QMutexLocker locker(&m_mutex);

   const bool isFlushing = m_state & Flushing;

   if (isFlushing != flushing) {
      if (flushing) {
         m_state |= Flushing;

         for (DirectShowTimedSample *sample = m_head; sample; sample = sample->remove()) {
            sample->unschedule(m_clock);

            m_semaphore.release(1);
         }

         m_head = nullptr;
         m_tail = nullptr;

         ::SetEvent(m_flushEvent);
      } else {
         m_state &= ~Flushing;

         if (m_state != Stopped) {
            ::ResetEvent(m_flushEvent);
         }
      }
   }
}

void DirectShowSampleScheduler::setClock(IReferenceClock *clock)
{
   QMutexLocker locker(&m_mutex);

   if (m_clock) {
      m_clock->Release();
   }

   m_clock = clock;

   if (m_clock) {
      m_clock->AddRef();
   }
}

IMediaSample *DirectShowSampleScheduler::takeSample(bool *eos)
{
   QMutexLocker locker(&m_mutex);

   if (m_head && m_head->isReady(m_clock)) {
      IMediaSample *sample = m_head->sample();
      sample->AddRef();

      *eos   =  m_head->isLast();
      m_head = m_head->remove();

      if (!m_head) {
         m_tail = nullptr;
      }

      m_semaphore.release(1);

      return sample;

   } else {
      return nullptr;
   }
}

bool DirectShowSampleScheduler::scheduleEndOfStream()
{
   QMutexLocker locker(&m_mutex);

   if (m_tail) {
      m_tail->setLast();

      return true;
   } else {
      return false;
   }
}

bool DirectShowSampleScheduler::event(QEvent *event)
{
   if (event->type() == QEvent::UpdateRequest) {
      emit sampleReady();

      return true;

   } else {
      return QObject::event(event);
   }
}
