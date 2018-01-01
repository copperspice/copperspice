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

#ifndef DS9_QBASEFILTER_H
#define DS9_QBASEFILTER_H

#include "phononds9_namespace.h"
#include <QtCore/QString>
#include <QtCore/QList>
#include <QtCore/QMutex>
#include <dshow.h>

QT_BEGIN_NAMESPACE

namespace Phonon
{
    namespace DS9
    {
        class QPin;
        class QBaseFilter : public IBaseFilter, public IMediaSeeking, public IMediaPosition
        {
        public:
            QBaseFilter(const CLSID &clsid);
            virtual ~QBaseFilter();

            //implementation from IUnknown
            STDMETHODIMP QueryInterface(REFIID iid, void** out) override;
            STDMETHODIMP_(ULONG) AddRef() override;
            STDMETHODIMP_(ULONG) Release() override;

            //implementation from IPersist
            STDMETHODIMP GetClassID(CLSID *) override;

            //implementation from IMediaFilter
            STDMETHODIMP Stop() override;
            STDMETHODIMP Pause() override;
            STDMETHODIMP Run(REFERENCE_TIME) override;
            STDMETHODIMP GetState(DWORD, FILTER_STATE*) override;
            STDMETHODIMP SetSyncSource(IReferenceClock*) override;
            STDMETHODIMP GetSyncSource(IReferenceClock**) override;

            //implementation from IBaseFilter
            STDMETHODIMP EnumPins(IEnumPins**) override;
            STDMETHODIMP FindPin(LPCWSTR, IPin**) override;
            STDMETHODIMP QueryFilterInfo(FILTER_INFO*) override;
            STDMETHODIMP JoinFilterGraph(IFilterGraph*, LPCWSTR) override;
            STDMETHODIMP QueryVendorInfo(LPWSTR*) override;

            //implementation from IMediaSeeking
            STDMETHODIMP GetCapabilities(DWORD *pCapabilities) override;
            STDMETHODIMP CheckCapabilities(DWORD *pCapabilities) override;
            STDMETHODIMP IsFormatSupported(const GUID *pFormat) override;
            STDMETHODIMP QueryPreferredFormat(GUID *pFormat) override;
            STDMETHODIMP GetTimeFormat(GUID *pFormat);
            STDMETHODIMP IsUsingTimeFormat(const GUID *pFormat) override;
            STDMETHODIMP SetTimeFormat(const GUID *pFormat) override;
            STDMETHODIMP GetDuration(LONGLONG *pDuration) override;
            STDMETHODIMP GetStopPosition(LONGLONG *pStop) override;
            STDMETHODIMP GetCurrentPosition(LONGLONG *pCurrent) override;
            STDMETHODIMP ConvertTimeFormat(LONGLONG *pTarget, const GUID *pTargetFormat, LONGLONG Source, const GUID *pSourceFormat) override;
            STDMETHODIMP SetPositions(LONGLONG *pCurrent, DWORD dwCurrentFlags, LONGLONG *pStop, DWORD dwStopFlags) override;
            STDMETHODIMP GetPositions(LONGLONG *pCurrent, LONGLONG *pStop) override;
            STDMETHODIMP GetAvailable(LONGLONG *pEarliest, LONGLONG *pLatest) override;
            STDMETHODIMP SetRate(double dRate) override;
            STDMETHODIMP GetRate(double *dRate) override;
            STDMETHODIMP GetPreroll(LONGLONG *pllPreroll) override;

            //implementation from IMediaPosition
            STDMETHODIMP get_Duration(REFTIME *plength) override;
            STDMETHODIMP put_CurrentPosition(REFTIME llTime) override;
            STDMETHODIMP get_CurrentPosition(REFTIME *pllTime) override;
            STDMETHODIMP get_StopTime(REFTIME *pllTime) override;
            STDMETHODIMP put_StopTime(REFTIME llTime) override;
            STDMETHODIMP get_PrerollTime(REFTIME *pllTime) override;
            STDMETHODIMP put_PrerollTime(REFTIME llTime) override;
            STDMETHODIMP put_Rate(double dRate) override;
            STDMETHODIMP get_Rate(double *pdRate) override;
            STDMETHODIMP CanSeekForward(LONG *pCanSeekForward) override;
            STDMETHODIMP CanSeekBackward(LONG *pCanSeekBackward) override;

            //implementation from IDispatch (coming from IMediaPosition)
            STDMETHODIMP GetTypeInfoCount(UINT *pctinfo) override;
            STDMETHODIMP GetTypeInfo(UINT iTInfo, LCID lcid, ITypeInfo **ppTInfo) override;
            STDMETHODIMP GetIDsOfNames(REFIID riid, LPOLESTR *rgszNames, UINT cNames, LCID lcid, DISPID *rgDispId) override;
            STDMETHODIMP Invoke(DISPID dispIdMember, REFIID riid, LCID lcid, WORD wFlags, DISPPARAMS *pDispParams, 
                VARIANT *pVarResult, EXCEPINFO *pExcepInfo, UINT *puArgErr) override;

            //own methods
            const QList<QPin *> pins() const;
            void addPin(QPin *pin);
            void removePin(QPin *pin);
            IFilterGraph *graph() const;
            FILTER_STATE state() const;


            //reimplement this if you want specific processing of media sample
            virtual HRESULT processSample(IMediaSample *);

        private:
            QList<QPin*> outputPins() const;
            QList<QPin*> inputPins() const;

            void *getUpStreamInterface(const IID &iid) const;
            IMediaSeeking *getUpstreamMediaSeeking();
            IMediaPosition *getUpstreamMediaPosition();

            LONG m_refCount;
            CLSID m_clsid;
            QString m_name;
            IReferenceClock *m_clock;
            IFilterGraph *m_graph;
            FILTER_STATE m_state;
            QList<QPin *> m_pins;
            mutable QMutex m_mutex;
        };
    }
}
QT_END_NAMESPACE

#endif
