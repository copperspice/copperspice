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

#ifndef DS9_QPIN_H
#define DS9_QPIN_H

#include <phononds9_namespace.h>
#include <QtCore/QString>
#include <QtCore/QVector>
#include <QtCore/QMutex>
#include <dshow.h>

QT_BEGIN_NAMESPACE

namespace Phonon
{
    namespace DS9
    {
        class QBaseFilter;

        //this is the base class for our self-implemented Pins
        class QPin : public IPin
        {
        public:
            QPin(QBaseFilter *parent, PIN_DIRECTION dir, const QVector<AM_MEDIA_TYPE> &mt);
            virtual ~QPin();

            //reimplementation from IUnknown
            STDMETHODIMP QueryInterface(REFIID iid, void** out) override;
            STDMETHODIMP_(ULONG) AddRef() override;
            STDMETHODIMP_(ULONG) Release() override;

            //reimplementation from IPin
            STDMETHODIMP Connect(IPin *,const AM_MEDIA_TYPE *) override;
            STDMETHODIMP ReceiveConnection(IPin *,const AM_MEDIA_TYPE *) override;
            STDMETHODIMP Disconnect() override;
            STDMETHODIMP ConnectedTo(IPin **) override;
            STDMETHODIMP ConnectionMediaType(AM_MEDIA_TYPE *) override;
            STDMETHODIMP QueryPinInfo(PIN_INFO *) override;
            STDMETHODIMP QueryDirection(PIN_DIRECTION *) override;
            STDMETHODIMP QueryId(LPWSTR*) override;
            STDMETHODIMP QueryAccept(const AM_MEDIA_TYPE*) override;
            STDMETHODIMP EnumMediaTypes(IEnumMediaTypes **) override;
            STDMETHODIMP QueryInternalConnections(IPin **, ULONG*) override;
            STDMETHODIMP EndOfStream() override;
            STDMETHODIMP BeginFlush() override;
            STDMETHODIMP EndFlush() override;
            STDMETHODIMP NewSegment(REFERENCE_TIME, REFERENCE_TIME, double) override;

            QVector<AM_MEDIA_TYPE> mediaTypes() const;

            HRESULT setAcceptedMediaType(const AM_MEDIA_TYPE &);

            bool isFlushing() const;
            void setConnectedType(const AM_MEDIA_TYPE &type);
            const AM_MEDIA_TYPE &connectedType() const;
            void setConnected(IPin *pin);
            IPin *connected(bool = false) const;
            void setMemoryAllocator(IMemAllocator *alloc);
            IMemAllocator *memoryAllocator(bool = false) const;
            void createDefaultMemoryAllocator(ALLOCATOR_PROPERTIES * = 0);
            PIN_DIRECTION direction() const;

            FILTER_STATE filterState() const;

            static AM_MEDIA_TYPE copyMediaType(const AM_MEDIA_TYPE &type);
            static void freeMediaType(AM_MEDIA_TYPE *type);
            static void freeMediaType(const AM_MEDIA_TYPE &type);

        protected:
            //this can be used by sub-classes
            mutable QMutex m_mutex;
            QBaseFilter * const m_parent;
            bool m_flushing;

        private:
            HRESULT checkOutputMediaTypesConnection(IPin *pin);
            HRESULT checkOwnMediaTypesConnection(IPin *pin);

            LONG m_refCount;
            IPin *m_connected;
            const PIN_DIRECTION m_direction;
            QVector<AM_MEDIA_TYPE> m_mediaTypes; //accepted media types
            AM_MEDIA_TYPE m_connectedType;
            IMemAllocator *m_memAlloc;
        };

        //utility function
        class QAMMediaType : public AM_MEDIA_TYPE
        {
        public:
            ~QAMMediaType() 
            {
                QPin::freeMediaType(*this);
            }

        };

    }
}

QT_END_NAMESPACE

#endif //PHONON_QPIN_H
