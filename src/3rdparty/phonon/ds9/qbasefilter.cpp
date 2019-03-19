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

#include <qmutex.h>
#include <qstring16.h>

QT_BEGIN_NAMESPACE

namespace Phonon
{
    namespace DS9
    {

        class QEnumPins : public IEnumPins
        {
        public:
            QEnumPins(QBaseFilter *filter) : m_refCount(1),
                m_filter(filter), m_pins(filter->pins()), m_index(0)
            {
                m_filter->AddRef();
            }

            virtual ~QEnumPins()
            {
                m_filter->Release();
            }

            STDMETHODIMP QueryInterface(const IID &iid,void **out) override {
                if (!out) {
                    return E_POINTER;
                }

                HRESULT hr = S_OK;
                if (iid == IID_IEnumPins) {
                    *out = static_cast<IEnumPins*>(this);
                } else if (iid == IID_IUnknown) {
                    *out = static_cast<IUnknown*>(this);
                } else {
                    *out = 0;
                    hr = E_NOINTERFACE;
                }

                if (S_OK)
                    AddRef();
                return hr;
            }

            STDMETHODIMP_(ULONG) AddRef() override {
                return InterlockedIncrement(&m_refCount);
            }

            STDMETHODIMP_(ULONG) Release() override {
                ULONG refCount = InterlockedDecrement(&m_refCount);
                if (refCount == 0) {
                    delete this;
                }

                return refCount;
            }

            STDMETHODIMP Next(ULONG count,IPin **ret,ULONG *fetched) override {
                QMutexLocker locker(&m_mutex);
                if (m_filter->pins() != m_pins) {
                    return VFW_E_ENUM_OUT_OF_SYNC;
                }

                if (fetched == 0 && count > 1) {
                    return E_INVALIDARG;
                }

                if (!ret) {
                    return E_POINTER;
                }

                uint nbfetched = 0;
                while (nbfetched < count && m_index < m_pins.count()) {
                    IPin *current = m_pins[m_index];
                    current->AddRef();
                    ret[nbfetched] = current;
                    nbfetched++;
                    m_index++;
                }

                if (fetched) {
                    *fetched = nbfetched;
                }

                return nbfetched == count ? S_OK : S_FALSE;
            }

            STDMETHODIMP Skip(ULONG count) override {
                QMutexLocker locker(&m_mutex);
                if (m_filter->pins() != m_pins) {
                    return VFW_E_ENUM_OUT_OF_SYNC;
                }

                m_index = qMin(m_index + int(count), m_pins.count());
                return m_index == m_pins.count() ? S_FALSE : S_OK;
            }

            STDMETHODIMP Reset() override             {
                QMutexLocker locker(&m_mutex);
                m_index = 0;
                return S_OK;
            }

            STDMETHODIMP Clone(IEnumPins **out) override             {
                QMutexLocker locker(&m_mutex);
                if (m_filter->pins() != m_pins) {
                    return VFW_E_ENUM_OUT_OF_SYNC;
                }

                if (!out) {
                    return E_POINTER;
                }

                *out = new QEnumPins(m_filter);
                (*out)->Skip(m_index);
                return S_OK;
            }


        private:
            LONG m_refCount;
            QBaseFilter *m_filter;
            QList<QPin*> m_pins;
            int m_index;
            QMutex m_mutex;
        };


        QBaseFilter::QBaseFilter(const CLSID &clsid):
        m_refCount(1), m_clsid(clsid), m_clock(0), m_graph(0), m_state(State_Stopped)
        {
        }

        QBaseFilter::~QBaseFilter()
        {
            while (!m_pins.isEmpty()) {
                delete m_pins.first();
            }
        }

        const QList<QPin *> QBaseFilter::pins() const
        {
            QMutexLocker locker(&m_mutex);
            return m_pins;
        }

        void QBaseFilter::addPin(QPin *pin)
        {
            QMutexLocker locker(&m_mutex);
            m_pins.append(pin);
        }

        void QBaseFilter::removePin(QPin *pin)
        {
            QMutexLocker locker(&m_mutex);
            m_pins.removeAll(pin);
        }

        FILTER_STATE QBaseFilter::state() const
        {
            return m_state;
        }

        IFilterGraph *QBaseFilter::graph() const
        {
            return m_graph;
        }

        STDMETHODIMP QBaseFilter::QueryInterface(REFIID iid, void **out)
        {
            if (!out) {
                return E_POINTER;
            }

            HRESULT hr = S_OK;

            if (iid == IID_IBaseFilter) {
                *out = static_cast<IBaseFilter*>(this);
            } else if (iid == IID_IMediaFilter) {
                *out = static_cast<IMediaFilter*>(this);
            } else if (iid == IID_IPersist) {
                *out = static_cast<IPersist*>(this);
            } else if (iid == IID_IUnknown) {
                *out = static_cast<IUnknown*>(static_cast<IBaseFilter*>(this));
            }
            else if (iid == IID_IMediaPosition || iid == IID_IMediaSeeking) {
                if (inputPins().isEmpty()) {
                    *out = getUpStreamInterface(iid);
                    if (*out) {
                        return S_OK; //we return here to avoid adding a reference
                    } else {
                        hr = E_NOINTERFACE;
                    }
                } else if (iid == IID_IMediaSeeking) {
                    *out = static_cast<IMediaSeeking*>(this);
                } else if (iid == IID_IMediaPosition ||iid == IID_IDispatch) {
                    *out = static_cast<IMediaPosition*>(this);
                }
            } else {
                *out = 0;
                hr = E_NOINTERFACE;
            }

            if (hr == S_OK) {
                AddRef();
            }

            return hr;
        }

        STDMETHODIMP_(ULONG) QBaseFilter::AddRef()
        {
            return InterlockedIncrement(&m_refCount);
        }

        STDMETHODIMP_(ULONG) QBaseFilter::Release()
        {
            ULONG refCount = InterlockedDecrement(&m_refCount);
            if (refCount == 0) {
                delete this;
            }

            return refCount;
        }

        STDMETHODIMP QBaseFilter::GetClassID(CLSID *clsid)
        {
            QMutexLocker locker(&m_mutex);
            *clsid = m_clsid;
            return S_OK;
        }

        STDMETHODIMP QBaseFilter::Stop()
        {
            QMutexLocker locker(&m_mutex);
            m_state = State_Stopped;
            return S_OK;
        }

        STDMETHODIMP QBaseFilter::Pause()
        {
            QMutexLocker locker(&m_mutex);
            m_state = State_Paused;
            return S_OK;
        }

        STDMETHODIMP QBaseFilter::Run(REFERENCE_TIME)
        {
            QMutexLocker locker(&m_mutex);
            m_state = State_Running;
            return S_OK;
        }

        STDMETHODIMP QBaseFilter::GetState(DWORD, FILTER_STATE *state)
        {
            QMutexLocker locker(&m_mutex);
            if (!state) {
                return E_POINTER;
            }

            *state = m_state;
            return S_OK;
        }

        STDMETHODIMP QBaseFilter::SetSyncSource(IReferenceClock *clock)
        {
            QMutexLocker locker(&m_mutex);
            if (clock) {
                clock->AddRef();
            }
            if (m_clock) {
                m_clock->Release();
            }
            m_clock = clock;
            return S_OK;
        }

        STDMETHODIMP QBaseFilter::GetSyncSource(IReferenceClock **clock)
        {
            QMutexLocker locker(&m_mutex);
            if (!clock) {
                return E_POINTER;
            }

            if (m_clock) {
                m_clock->AddRef();
            }

            *clock = m_clock;
            return S_OK;
        }

        STDMETHODIMP QBaseFilter::FindPin(LPCWSTR name, IPin**pin)
        {
            if (!pin) {
                return E_POINTER;
            }

            for (int i = 0; i < m_pins.count(); ++i) {
                IPin * current = m_pins.at(i);
                PIN_INFO info;
                current->QueryPinInfo(&info);
                if (info.pFilter) {
                    info.pFilter->Release();
                }
                if ( wcscmp(info.achName, name) == 0) {
                    *pin = current;
                    current->AddRef();
                    return S_OK;
                }
            }

            *pin = 0;
            return VFW_E_NOT_FOUND;
        }

        STDMETHODIMP QBaseFilter::QueryFilterInfo(FILTER_INFO *info )
        {
            QMutexLocker locker(&m_mutex);

            if (! info) {
                return E_POINTER;
            }

            info->pGraph = m_graph;

            if (m_graph) {
                m_graph->AddRef();
            }

            QString16 tmp = m_name.toUtf16();
            memcpy(info->achName,tmp.constData(), qMin(MAX_FILTER_NAME, m_name.size_storage() + 1) * 2);

            return S_OK;
        }

        STDMETHODIMP QBaseFilter::JoinFilterGraph(IFilterGraph *graph, LPCWSTR name)
        {
            QMutexLocker locker(&m_mutex);

            m_graph = graph;

            if (name == nullptr) {
               m_name = "";
            } else {
               m_name  = QString::fromStdWString(std::wstring(name));
            }

            return S_OK;
        }

        STDMETHODIMP QBaseFilter::EnumPins(IEnumPins **ep)
        {
            if (! ep) {
                return E_POINTER;
            }

            *ep = new QEnumPins(this);
            return S_OK;
        }


        STDMETHODIMP QBaseFilter::QueryVendorInfo(LPWSTR *)
        {
            //we give no information on that
            return E_NOTIMPL;
        }

                //implementation from IMediaSeeking
        STDMETHODIMP QBaseFilter::GetCapabilities(DWORD *pCapabilities)
        {
            IMediaSeeking *ms = getUpstreamMediaSeeking();
            if (!ms) {
                return E_NOTIMPL;
            }

            HRESULT hr = ms->GetCapabilities(pCapabilities);
            ms->Release();
            return hr;
        }

        STDMETHODIMP QBaseFilter::CheckCapabilities(DWORD *pCapabilities)
        {
            IMediaSeeking *ms = getUpstreamMediaSeeking();
            if (!ms) {
                return E_NOTIMPL;
            }

            HRESULT hr = ms->CheckCapabilities(pCapabilities);
            ms->Release();
            return hr;
        }

        STDMETHODIMP QBaseFilter::IsFormatSupported(const GUID *pFormat)
        {
            IMediaSeeking *ms = getUpstreamMediaSeeking();
            if (!ms) {
                return E_NOTIMPL;
            }

            HRESULT hr = ms->IsFormatSupported(pFormat);
            ms->Release();
            return hr;
        }

        STDMETHODIMP QBaseFilter::QueryPreferredFormat(GUID *pFormat)
        {
            IMediaSeeking *ms = getUpstreamMediaSeeking();
            if (!ms) {
                return E_NOTIMPL;
            }

            HRESULT hr = ms->QueryPreferredFormat(pFormat);
            ms->Release();
            return hr;
        }

        STDMETHODIMP QBaseFilter::GetTimeFormat(GUID *pFormat)
        {
            IMediaSeeking *ms = getUpstreamMediaSeeking();
            if (!ms) {
                return E_NOTIMPL;
            }

            HRESULT hr = ms->GetTimeFormat(pFormat);
            ms->Release();
            return hr;
        }

        STDMETHODIMP QBaseFilter::IsUsingTimeFormat(const GUID *pFormat)
        {
            IMediaSeeking *ms = getUpstreamMediaSeeking();
            if (!ms) {
                return E_NOTIMPL;
            }

            HRESULT hr = ms->IsUsingTimeFormat(pFormat);
            ms->Release();
            return hr;
        }

        STDMETHODIMP QBaseFilter::SetTimeFormat(const GUID *pFormat)
        {
            IMediaSeeking *ms = getUpstreamMediaSeeking();
            if (!ms) {
                return E_NOTIMPL;
            }

            HRESULT hr = ms->SetTimeFormat(pFormat);
            ms->Release();
            return hr;
        }

        STDMETHODIMP QBaseFilter::GetDuration(LONGLONG *pDuration)
        {
            IMediaSeeking *ms = getUpstreamMediaSeeking();
            if (!ms) {
                return E_NOTIMPL;
            }

            HRESULT hr = ms->GetDuration(pDuration);
            ms->Release();
            return hr;
        }

        STDMETHODIMP QBaseFilter::GetStopPosition(LONGLONG *pStop)
        {
            IMediaSeeking *ms = getUpstreamMediaSeeking();
            if (!ms) {
                return E_NOTIMPL;
            }

            HRESULT hr = ms->GetStopPosition(pStop);
            ms->Release();
            return hr;
        }

        STDMETHODIMP QBaseFilter::GetCurrentPosition(LONGLONG *pCurrent)
        {
            IMediaSeeking *ms = getUpstreamMediaSeeking();
            if (!ms) {
                return E_NOTIMPL;
            }

            HRESULT hr = ms->GetCurrentPosition(pCurrent);
            ms->Release();
            return hr;
        }

        STDMETHODIMP QBaseFilter::ConvertTimeFormat(LONGLONG *pTarget,
            const GUID *pTargetFormat, LONGLONG Source, const GUID *pSourceFormat)
        {
            IMediaSeeking *ms = getUpstreamMediaSeeking();
            if (!ms) {
                return E_NOTIMPL;
            }

            HRESULT hr = ms->ConvertTimeFormat(pTarget, pTargetFormat, Source, pSourceFormat);
            ms->Release();
            return hr;
        }

        STDMETHODIMP QBaseFilter::SetPositions(LONGLONG *pCurrent, DWORD dwCurrentFlags, LONGLONG *pStop, DWORD dwStopFlags)
        {
            IMediaSeeking *ms = getUpstreamMediaSeeking();
            if (!ms) {
                return E_NOTIMPL;
            }

            HRESULT hr = ms->SetPositions(pCurrent, dwCurrentFlags, pStop, dwStopFlags);
            ms->Release();
            return hr;
        }

        STDMETHODIMP QBaseFilter::GetPositions(LONGLONG *pCurrent, LONGLONG *pStop)
        {
            IMediaSeeking *ms = getUpstreamMediaSeeking();
            if (!ms) {
                return E_NOTIMPL;
            }

            HRESULT hr = ms->GetPositions(pCurrent, pStop);
            ms->Release();
            return hr;
        }

        STDMETHODIMP QBaseFilter::GetAvailable(LONGLONG *pEarliest, LONGLONG *pLatest)
        {
            IMediaSeeking *ms = getUpstreamMediaSeeking();
            if (!ms) {
                return E_NOTIMPL;
            }

            HRESULT hr = ms->GetAvailable(pEarliest, pLatest);
            ms->Release();
            return hr;
        }

        STDMETHODIMP QBaseFilter::SetRate(double dRate)
        {
            IMediaSeeking *ms = getUpstreamMediaSeeking();
            if (!ms) {
                return E_NOTIMPL;
            }

            HRESULT hr = ms->SetRate(dRate);
            ms->Release();
            return hr;
        }

        STDMETHODIMP QBaseFilter::GetRate(double *dRate)
        {
            IMediaSeeking *ms = getUpstreamMediaSeeking();
            if (!ms) {
                return E_NOTIMPL;
            }

            HRESULT hr = ms->GetRate(dRate);
            ms->Release();
            return hr;
        }

        STDMETHODIMP QBaseFilter::GetPreroll(LONGLONG *pllPreroll)
        {
            IMediaSeeking *ms = getUpstreamMediaSeeking();
            if (!ms) {
                return E_NOTIMPL;
            }

            HRESULT hr = ms->GetPreroll(pllPreroll);
            ms->Release();
            return hr;
        }

        //implementation from IMediaPosition
        STDMETHODIMP QBaseFilter::get_Duration(REFTIME *plength)
        {
            IMediaPosition *mp = getUpstreamMediaPosition();
            if (!mp) {
                return E_NOTIMPL;
            }

            HRESULT hr = mp->get_Duration(plength);
            mp->Release();
            return hr;
        }

        STDMETHODIMP QBaseFilter::put_CurrentPosition(REFTIME llTime)
        {
            IMediaPosition *mp = getUpstreamMediaPosition();
            if (!mp) {
                return E_NOTIMPL;
            }

            HRESULT hr = mp->put_CurrentPosition(llTime);
            mp->Release();
            return hr;
        }

        STDMETHODIMP QBaseFilter::get_CurrentPosition(REFTIME *pllTime)
        {
            IMediaPosition *mp = getUpstreamMediaPosition();
            if (!mp) {
                return E_NOTIMPL;
            }

            HRESULT hr = mp->get_CurrentPosition(pllTime);
            mp->Release();
            return hr;
        }

        STDMETHODIMP QBaseFilter::get_StopTime(REFTIME *pllTime)
        {
            IMediaPosition *mp = getUpstreamMediaPosition();
            if (!mp) {
                return E_NOTIMPL;
            }

            HRESULT hr = mp->get_StopTime(pllTime);
            mp->Release();
            return hr;
        }

        STDMETHODIMP QBaseFilter::put_StopTime(REFTIME llTime)
        {
            IMediaPosition *mp = getUpstreamMediaPosition();
            if (!mp) {
                return E_NOTIMPL;
            }

            HRESULT hr = mp->put_StopTime(llTime);
            mp->Release();
            return hr;
        }

        STDMETHODIMP QBaseFilter::get_PrerollTime(REFTIME *pllTime)
        {
            IMediaPosition *mp = getUpstreamMediaPosition();
            if (!mp) {
                return E_NOTIMPL;
            }

            HRESULT hr = mp->get_PrerollTime(pllTime);
            mp->Release();
            return hr;
        }

        STDMETHODIMP QBaseFilter::put_PrerollTime(REFTIME llTime)
        {
            IMediaPosition *mp = getUpstreamMediaPosition();
            if (!mp) {
                return E_NOTIMPL;
            }

            HRESULT hr = mp->put_PrerollTime(llTime);
            mp->Release();
            return hr;
        }

        STDMETHODIMP QBaseFilter::put_Rate(double dRate)
        {
            IMediaPosition *mp = getUpstreamMediaPosition();
            if (!mp) {
                return E_NOTIMPL;
            }

            HRESULT hr = mp->put_Rate(dRate);
            mp->Release();
            return hr;
        }

        STDMETHODIMP QBaseFilter::get_Rate(double *pdRate)
        {
            IMediaPosition *mp = getUpstreamMediaPosition();
            if (!mp) {
                return E_NOTIMPL;
            }

            HRESULT hr = mp->get_Rate(pdRate);
            mp->Release();
            return hr;
        }

        STDMETHODIMP QBaseFilter::CanSeekForward(LONG *pCanSeekForward)
        {
            IMediaPosition *mp = getUpstreamMediaPosition();
            if (!mp) {
                return E_NOTIMPL;
            }

            HRESULT hr = mp->CanSeekForward(pCanSeekForward);
            mp->Release();
            return hr;
        }

        STDMETHODIMP QBaseFilter::CanSeekBackward(LONG *pCanSeekBackward)
        {
            IMediaPosition *mp = getUpstreamMediaPosition();
            if (!mp) {
                return E_NOTIMPL;
            }

            HRESULT hr = mp->CanSeekBackward(pCanSeekBackward);
            mp->Release();
            return hr;
        }

        STDMETHODIMP QBaseFilter::GetTypeInfoCount(UINT *pctinfo)
        {
            IMediaPosition *mp = getUpstreamMediaPosition();
            if (!mp) {
                return E_NOTIMPL;
            }

            HRESULT hr = mp->GetTypeInfoCount(pctinfo);
            mp->Release();
            return hr;
        }

        STDMETHODIMP QBaseFilter::GetTypeInfo(UINT iTInfo, LCID lcid, ITypeInfo **ppTInfo)
        {
            IMediaPosition *mp = getUpstreamMediaPosition();
            if (!mp) {
                return E_NOTIMPL;
            }

            HRESULT hr = mp->GetTypeInfo(iTInfo, lcid, ppTInfo);
            mp->Release();
            return hr;
        }

        STDMETHODIMP QBaseFilter::GetIDsOfNames(REFIID riid, LPOLESTR *rgszNames, UINT cNames, LCID lcid, DISPID *rgDispId)
        {
            IMediaPosition *mp = getUpstreamMediaPosition();
            if (!mp) {
                return E_NOTIMPL;
            }

            HRESULT hr = mp->GetIDsOfNames(riid, rgszNames, cNames, lcid, rgDispId);
            mp->Release();
            return hr;
        }

        STDMETHODIMP QBaseFilter::Invoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS *pDispParams,
            VARIANT *pVarResult, EXCEPINFO *pExcepInfo, UINT *puArgErr)
        {
            IMediaPosition *mp = getUpstreamMediaPosition();
            if (!mp) {
                return E_NOTIMPL;
            }

            HRESULT hr = mp->Invoke(dispIdMember, riid, lcid, wFlags, pDispParams, pVarResult, pExcepInfo, puArgErr);
            mp->Release();
            return hr;
        }


        IMediaSeeking *QBaseFilter::getUpstreamMediaSeeking()
        {
            return static_cast<IMediaSeeking*>(getUpStreamInterface(IID_IMediaSeeking));
        }

        IMediaPosition *QBaseFilter::getUpstreamMediaPosition()
        {
            return static_cast<IMediaPosition*>(getUpStreamInterface(IID_IMediaPosition));
        }

        QList<QPin*> QBaseFilter::inputPins() const
        {
            QList<QPin*> ret;
            for(int i = 0; i < m_pins.count(); ++i) {
                QPin * pin = m_pins.at(i);
                if (pin->direction() == PINDIR_INPUT) {
                    ret += pin;
                }
            }
            return ret;
        }

        QList<QPin*> QBaseFilter::outputPins() const
        {
            QList<QPin*> ret;
            for(int i = 0; i < m_pins.count(); ++i) {
                QPin * pin = m_pins.at(i);
                if (pin->direction() == PINDIR_OUTPUT) {
                    ret += pin;
                }
            }
            return ret;
        }

        void *QBaseFilter::getUpStreamInterface(const IID &iid) const
        {
            const QList<QPin*> inputs = inputPins();
            for (int i = 0; i < inputs.count(); ++i) {
                IPin *out = inputs.at(i)->connected();
                if (out) {
                    void *ms = 0;
                    out->QueryInterface(iid, &ms);
                    if (ms) {
                        return ms;
                    }
                }
            }
            //none was found
            return 0;
        }


        //addition
        HRESULT QBaseFilter::processSample(IMediaSample *)
        {
            return S_OK;
        }

    }
}

QT_END_NAMESPACE
