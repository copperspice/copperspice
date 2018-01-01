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

#ifndef DS9_QMEMINPUTPIN_H
#define DS9_QMEMINPUTPIN_H

#include <QtCore/QList>
#include <QtCore/QMutex>
#include <qpin.h>
#include <dshow.h>

QT_BEGIN_NAMESPACE

namespace Phonon
{
    namespace DS9
    {
        class QBaseFilter;

        //this class will be used for our effects
        class QMemInputPin : public QPin, public IMemInputPin
        {
        public:
            QMemInputPin(QBaseFilter *, const QVector<AM_MEDIA_TYPE> &, bool transform, QPin *output);
            ~QMemInputPin();

            //reimplementation from IUnknown
            STDMETHODIMP QueryInterface(REFIID iid, void** out) override;
            STDMETHODIMP_(ULONG) AddRef() override;
            STDMETHODIMP_(ULONG) Release() override;

            //reimplementation from IPin
            STDMETHODIMP ReceiveConnection(IPin *,const AM_MEDIA_TYPE *) override;
            STDMETHODIMP BeginFlush() override;
            STDMETHODIMP EndFlush() override;
            STDMETHODIMP EndOfStream() override;
            STDMETHODIMP NewSegment(REFERENCE_TIME start, REFERENCE_TIME stop, double rate) override;

            //reimplementation from IMemAllocator
            STDMETHODIMP GetAllocator(IMemAllocator **) override;
            STDMETHODIMP NotifyAllocator(IMemAllocator *,BOOL) override;
            STDMETHODIMP GetAllocatorRequirements(ALLOCATOR_PROPERTIES *) override;
            STDMETHODIMP Receive(IMediaSample *) override;
            STDMETHODIMP ReceiveMultiple(IMediaSample **,long,long *) override;
            STDMETHODIMP ReceiveCanBlock() override;

        private:
            IMediaSample *duplicateSampleForOutput(IMediaSample *, IMemAllocator *);
            ALLOCATOR_PROPERTIES getDefaultAllocatorProperties() const;

            bool m_shouldDuplicateSamples;
            const bool m_transform; //defines if the pin is transforming the samples
            QPin* const m_output;
            QMutex m_mutexReceive;
        };
    }
}

QT_END_NAMESPACE

#endif
