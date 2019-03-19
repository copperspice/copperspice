/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

#include "qbasefilter.h"
#include "qpin.h"
#include "compointer.h"

#include <QtCore/QMutex>

QT_BEGIN_NAMESPACE

namespace Phonon
{
    namespace DS9
    {

        static const AM_MEDIA_TYPE defaultMediaType = { MEDIATYPE_NULL, MEDIASUBTYPE_NULL, TRUE, FALSE, 1, GUID_NULL, 0, 0, 0};

        class QEnumMediaTypes : public IEnumMediaTypes
        {
        public:
            QEnumMediaTypes(QPin *pin) :  m_refCount(1), m_pin(pin), m_index(0)
            {
                m_pin->AddRef();
            }

            ~QEnumMediaTypes()
            {
                m_pin->Release();
            }

            STDMETHODIMP QueryInterface(const IID &iid,void **out) override {
                if (!out) {
                    return E_POINTER;
                }

                HRESULT hr = S_OK;
                if (iid == IID_IEnumMediaTypes) {
                    *out = static_cast<IEnumMediaTypes*>(this);
                } else if (iid == IID_IUnknown) {
                    *out = static_cast<IUnknown*>(this);
                } else {
                    *out = 0;
                    hr = E_NOINTERFACE;
                }

                if (hr == S_OK) {
                    AddRef();
                }
                return hr;
            }

            STDMETHODIMP_(ULONG) AddRef() override
            {
                return InterlockedIncrement(&m_refCount);
            }

            STDMETHODIMP_(ULONG) Release() override
            {
                ULONG refCount = InterlockedDecrement(&m_refCount);
                if (refCount == 0) {
                    delete this;
                }

                return refCount;
            }

            STDMETHODIMP Next(ULONG count, AM_MEDIA_TYPE **out, ULONG *fetched) override
            {
                QMutexLocker locker(&m_mutex);
                if (!out) {
                    return E_POINTER;
                }

                if (!fetched && count > 1) {
                    return E_INVALIDARG;
                }

                uint nbFetched = 0;
                while (nbFetched < count && m_index < m_pin->mediaTypes().count()) {
                    //the caller will deallocate the memory
                    *out = static_cast<AM_MEDIA_TYPE *>(::CoTaskMemAlloc(sizeof(AM_MEDIA_TYPE)));
                    const AM_MEDIA_TYPE original = m_pin->mediaTypes().at(m_index);
                    **out = QPin::copyMediaType(original);
                    nbFetched++;
                    m_index++;
                    out++;
                }

                if (fetched) {
                    *fetched = nbFetched;
                }

                return nbFetched == count ? S_OK : S_FALSE;
            }

            STDMETHODIMP Skip(ULONG count) override
            {
                QMutexLocker locker(&m_mutex);
                m_index = qMin(m_index + int(count), m_pin->mediaTypes().count());
                return  (m_index == m_pin->mediaTypes().count()) ? S_FALSE : S_OK;
            }

            STDMETHODIMP Reset() override
            {
                QMutexLocker locker(&m_mutex);
                m_index = 0;
                return S_OK;
            }

            STDMETHODIMP Clone(IEnumMediaTypes **out) override
            {
                QMutexLocker locker(&m_mutex);
                if (!out) {
                    return E_POINTER;
                }

                *out = new QEnumMediaTypes(m_pin);
                (*out)->Skip(m_index);
                return S_OK;
            }


        private:
            LONG m_refCount;
            QPin *m_pin;
            int m_index;
            QMutex m_mutex;
        };


        QPin::QPin(QBaseFilter *parent, PIN_DIRECTION dir, const QVector<AM_MEDIA_TYPE> &mt) :
            m_parent(parent), m_flushing(false), m_refCount(1),  m_connected(0),
            m_direction(dir), m_mediaTypes(mt), m_connectedType(defaultMediaType),
            m_memAlloc(0)
        {
            Q_ASSERT(m_parent);
            m_parent->addPin(this);
        }

        QPin::~QPin()
        {
            m_parent->removePin(this);
            setMemoryAllocator(0);
            freeMediaType(m_connectedType);
        }

        //reimplementation from IUnknown
        STDMETHODIMP QPin::QueryInterface(REFIID iid, void**out)
        {
            if (!out) {
                return E_POINTER;
            }

            HRESULT hr = S_OK;

            if (iid == IID_IPin) {
                *out = static_cast<IPin*>(this);
            } else if (iid == IID_IUnknown) {
                *out = static_cast<IUnknown*>(this);
            } else if (m_direction == PINDIR_OUTPUT && (iid == IID_IMediaSeeking || iid == IID_IMediaPosition)) {
                return m_parent->QueryInterface(iid, out);
            } else {
                *out = 0;
                hr = E_NOINTERFACE;
            }

            if (hr == S_OK) {
                AddRef();
            }
            return hr;
        }

        STDMETHODIMP_(ULONG) QPin::AddRef()
        {
            return InterlockedIncrement(&m_refCount);
        }

        STDMETHODIMP_(ULONG) QPin::Release()
        {
            ULONG refCount = InterlockedDecrement(&m_refCount);
            if (refCount == 0) {
                delete this;
            }

            return refCount;
        }

        //this is called on the input pins
        STDMETHODIMP QPin::ReceiveConnection(IPin *pin, const AM_MEDIA_TYPE *type)
        {
            if (!pin ||!type) {
                return E_POINTER;
            }

            if (connected()) {
                return VFW_E_ALREADY_CONNECTED;
            }

            if (filterState() != State_Stopped) {
                return VFW_E_NOT_STOPPED;
            }

            if (QueryAccept(type) != S_OK) {
                return VFW_E_TYPE_NOT_ACCEPTED;
            }

            setConnected(pin);
            setConnectedType(*type);

            return S_OK;
        }

        //this is called on the output pins
        STDMETHODIMP QPin::Connect(IPin *pin, const AM_MEDIA_TYPE *type)
        {
            if (!pin) {
                return E_POINTER;
            }

            if (connected()) {
                return VFW_E_ALREADY_CONNECTED;
            }

            if (filterState() != State_Stopped) {
                return VFW_E_NOT_STOPPED;
            }

            HRESULT hr = S_OK;

            setConnected(pin);
            if (!type) {

                //let(s first try the output pin's mediaTypes
                if (checkOutputMediaTypesConnection(pin) != S_OK &&
                    checkOwnMediaTypesConnection(pin) != S_OK) {
                        hr = VFW_E_NO_ACCEPTABLE_TYPES;
                }
            } else if (QueryAccept(type) == S_OK) {
                setConnectedType(*type);
                hr = pin->ReceiveConnection(this, type);
            } else {
                hr = VFW_E_TYPE_NOT_ACCEPTED;
            }

            if (FAILED(hr)) {
                setConnected(0);
                setConnectedType(defaultMediaType);
            } else {
                ComPointer<IMemInputPin> input(pin, IID_IMemInputPin);
                if (input) {
                    ComPointer<IMemAllocator> alloc;
                    input->GetAllocator(alloc.pparam());
                    if (alloc) {
                        //be default we take the allocator from the input pin
                        //we have no reason to force using our own
                        setMemoryAllocator(alloc);
                    }
                }
                if (memoryAllocator() == 0) {
                    ALLOCATOR_PROPERTIES prop;
                    if (input && input->GetAllocatorRequirements(&prop) == S_OK) {
                        createDefaultMemoryAllocator(&prop);
                    } else {
                        createDefaultMemoryAllocator();
                    }
                }

                Q_ASSERT(memoryAllocator() != 0);
                if (input) {
                    input->NotifyAllocator(memoryAllocator(), TRUE); //TRUE is arbitrarily chosen here
                }

            }

            return hr;
        }

        STDMETHODIMP QPin::Disconnect()
        {
            if (!connected()) {
                return S_FALSE;
            }

            if (filterState() != State_Stopped) {
                return VFW_E_NOT_STOPPED;
            }

            setConnected(0);
            setConnectedType(defaultMediaType);
            setMemoryAllocator(0);
            return S_OK;
        }

        STDMETHODIMP QPin::ConnectedTo(IPin **other)
        {
            if (!other) {
                return E_POINTER;
            }

            *other = connected(true);
            if (!(*other)) {
                return VFW_E_NOT_CONNECTED;
            }

            return S_OK;
        }

        STDMETHODIMP QPin::ConnectionMediaType(AM_MEDIA_TYPE *type)
        {
            QMutexLocker locker(&m_mutex);
            if (!type) {
                return E_POINTER;
            }

            *type = copyMediaType(m_connectedType);
            if (!m_connected) {
                return VFW_E_NOT_CONNECTED;
            } else {
                return S_OK;
            }
        }

        STDMETHODIMP QPin::QueryPinInfo(PIN_INFO *info)
        {
            if (!info) {
                return E_POINTER;
            }

            info->dir = m_direction;
            info->pFilter = m_parent;
            m_parent->AddRef();
            info->achName[0] = 0;
            return S_OK;
        }

        STDMETHODIMP QPin::QueryDirection(PIN_DIRECTION *dir)
        {
            if (!dir) {
                return E_POINTER;
            }

            *dir = m_direction;
            return S_OK;
        }

        STDMETHODIMP QPin::QueryId(LPWSTR *id)
        {
            if (!id) {
                return E_POINTER;
            }

            *id = static_cast<LPWSTR>(::CoTaskMemAlloc(2));
            *id[0] = 0;
            return S_OK;
        }

        STDMETHODIMP QPin::QueryAccept(const AM_MEDIA_TYPE *type)
        {
            QMutexLocker locker(&m_mutex);
            if (!type) {
                return E_POINTER;
            }

            for (int i = 0; i < m_mediaTypes.count(); ++i) {
                const AM_MEDIA_TYPE &current = m_mediaTypes.at(i);
                if ( (type->majortype == current.majortype) &&
                    (current.subtype == MEDIASUBTYPE_NULL || type->subtype == current.subtype) &&
                    (type->majortype == MEDIATYPE_Stream || type->formattype != GUID_NULL || current.formattype != GUID_NULL) &&
                    (current.formattype == GUID_NULL || type->formattype == current.formattype)
                    ) {
                        return S_OK;
                }
            }
            return S_FALSE;
        }


        STDMETHODIMP QPin::EnumMediaTypes(IEnumMediaTypes **emt)
        {
            if (!emt) {
                return E_POINTER;
            }

            *emt = new QEnumMediaTypes(this);
            return S_OK;
        }


        STDMETHODIMP QPin::EndOfStream()
        {
            return E_UNEXPECTED;
        }

        STDMETHODIMP QPin::BeginFlush()
        {
            return E_UNEXPECTED;
        }

        STDMETHODIMP QPin::EndFlush()
        {
            return E_UNEXPECTED;
        }

        STDMETHODIMP QPin::NewSegment(REFERENCE_TIME start, REFERENCE_TIME stop, double rate)
        {
            QMutexLocker locker(&m_mutex);
            if (m_direction == PINDIR_OUTPUT && m_connected) {
                //we deliver this downstream
                m_connected->NewSegment(start, stop, rate);
            }
            return S_OK;
        }

        STDMETHODIMP QPin::QueryInternalConnections(IPin **, ULONG*)
        {
            //this is not implemented on purpose (the input pins are connected to all the output pins)
            return E_NOTIMPL;
        }


        HRESULT QPin::checkOutputMediaTypesConnection(IPin *pin)
        {
            ComPointer<IEnumMediaTypes> emt;
            HRESULT hr = pin->EnumMediaTypes(emt.pparam());
            if (hr != S_OK) {
                return hr;
            }

            AM_MEDIA_TYPE *type = 0;
            while (emt->Next(1, &type, 0) == S_OK) {
                if (QueryAccept(type) == S_OK) {
                    setConnectedType(*type);
                    if (pin->ReceiveConnection(this, type) == S_OK) {
                        freeMediaType(type);
                        return S_OK;
                    } else {
                        setConnectedType(defaultMediaType);
                        freeMediaType(type);
                    }
                }
            }

            //we didn't find a suitable type
            return S_FALSE;
        }

        HRESULT QPin::checkOwnMediaTypesConnection(IPin *pin)
        {
            for(int i = 0; i < m_mediaTypes.count(); ++i) {
                const AM_MEDIA_TYPE &current = m_mediaTypes.at(i);
                setConnectedType(current);
                HRESULT hr = pin->ReceiveConnection(this, &current);
                if (hr == S_OK) {
                    return S_OK;
                }
            }

            //we didn't find a suitable type
            return S_FALSE;
        }

        void QPin::freeMediaType(const AM_MEDIA_TYPE &type)
        {
            if (type.cbFormat) {
                ::CoTaskMemFree(type.pbFormat);
            }
            if (type.pUnk) {
                type.pUnk->Release();
            }
        }

        void QPin::freeMediaType(AM_MEDIA_TYPE *type)
        {
            freeMediaType(*type);
            ::CoTaskMemFree(type);
        }

        //addition

        PIN_DIRECTION QPin::direction() const
        {
            return m_direction;
        }

        void QPin::setConnectedType(const AM_MEDIA_TYPE &type)
        {
            QMutexLocker locker(&m_mutex);

            //1st we free memory
            freeMediaType(m_connectedType);

            m_connectedType = copyMediaType(type);
        }

        const AM_MEDIA_TYPE &QPin::connectedType() const
        {
            QMutexLocker locker(&m_mutex);
            return m_connectedType;
        }

        void QPin::setConnected(IPin *pin)
        {
            QMutexLocker locker(&m_mutex);
            if (pin) {
                pin->AddRef();
            }
            if (m_connected) {
                m_connected->Release();
            }
            m_connected = pin;
        }

        IPin *QPin::connected(bool addref) const
        {
            QMutexLocker locker(&m_mutex);
            if (addref && m_connected) {
                m_connected->AddRef();
            }
            return m_connected;
        }

        bool QPin::isFlushing() const
        {
            QMutexLocker locker(&m_mutex);
            return m_flushing;
        }

        FILTER_STATE QPin::filterState() const
        {
            FILTER_STATE fstate = State_Stopped;
            m_parent->GetState(0, &fstate);
            return fstate;
        }

        QVector<AM_MEDIA_TYPE> QPin::mediaTypes() const
        {
            QMutexLocker locker(&m_mutex);
            return m_mediaTypes;
        }

        HRESULT QPin::setAcceptedMediaType(const AM_MEDIA_TYPE &mt)
        {
            const QVector<AM_MEDIA_TYPE> oldMediaTypes = m_mediaTypes;
            m_mediaTypes = QVector<AM_MEDIA_TYPE>() << mt;

            HRESULT hr = S_OK;

            IPin *conn = connected();
            if (conn) {
                //try to reconnect to redefine the media type
                conn->Disconnect();
                Disconnect();
                hr = Connect(conn, 0);
                if (FAILED(hr)) {
                    m_mediaTypes = oldMediaTypes;
                    Connect(conn, 0); //just redo the connection with the old media types
                }
            }
            return hr;
        }

        void QPin::createDefaultMemoryAllocator(ALLOCATOR_PROPERTIES *prop)
        {
            ComPointer<IMemAllocator> alloc(CLSID_MemoryAllocator, IID_IMemAllocator);
            if (prop) {
                alloc->SetProperties(prop, 0);
            }
            setMemoryAllocator(alloc);
        }

        void QPin::setMemoryAllocator(IMemAllocator *alloc)
        {
            QMutexLocker locker(&m_mutex);
            if (alloc) {
                alloc->AddRef();
            }
            if (m_memAlloc) {
                m_memAlloc->Release();
            }
            m_memAlloc = alloc;
        }

        IMemAllocator *QPin::memoryAllocator(bool addref) const
        {
            QMutexLocker locker(&m_mutex);
            if (addref && m_memAlloc) {
                m_memAlloc->AddRef();
            }
            return m_memAlloc;
        }

        AM_MEDIA_TYPE QPin::copyMediaType(const AM_MEDIA_TYPE &type)
        {
            AM_MEDIA_TYPE ret = type;

            //make a deep copy here
            if (ret.cbFormat == 0 || ret.pbFormat == 0) {
                ret.cbFormat = 0;
                ret.pbFormat = 0;
                ret.formattype = GUID_NULL;
            } else {
                ret.pbFormat = reinterpret_cast<BYTE*>(::CoTaskMemAlloc(type.cbFormat));
                memcpy(ret.pbFormat, type.pbFormat, type.cbFormat);
            }

            if (type.pUnk) {
                type.pUnk->AddRef();
            }
            return ret;
        }


    }
}

QT_END_NAMESPACE
