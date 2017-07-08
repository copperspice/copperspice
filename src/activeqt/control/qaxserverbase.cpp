/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the ActiveQt framework of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Nokia Corporation and its Subsidiary(-ies) nor
**     the names of its contributors may be used to endorse or promote
**     products derived from this software without specific prior written
**     permission.
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef QT_NO_WIN_ACTIVEQT

#include <qabstracteventdispatcher.h>
#include <qapplication.h>
#include <qbuffer.h>
#include <qdatastream.h>
#include <qdebug.h>
#include <qevent.h>
#include <qeventloop.h>
#include <qfile.h>
#include <qpointer.h>
#include <qhash.h>
#include <qmap.h>
#include <qmenubar.h>
#include <qmenu.h>
#include <qmetaobject.h>
#include <qpixmap.h>
#include <qstatusbar.h>
#include <qwhatsthis.h>
#include <ocidl.h>
#include <olectl.h>
#include <qcoreapplication_p.h>

#include "qaxfactory.h"
#include "qaxbindable.h"
#include "qaxaggregated.h"

#include "../shared/qaxtypes.h"

#if defined Q_CC_GNU
#   include <w32api.h>
#endif

#ifndef Q_OS_WIN64
#define ULONG_PTR DWORD
#endif

QT_BEGIN_NAMESPACE

extern HHOOK qax_hhook;

// in qaxserver.cpp
extern ITypeLib *qAxTypeLibrary;
extern QAxFactory *qAxFactory();
extern unsigned long qAxLock();
extern unsigned long qAxUnlock();
extern HANDLE qAxInstance;
extern bool qAxOutProcServer;

static int invokeCount = 0;

#ifdef QT_DEBUG
unsigned long qaxserverbase_instance_count = 0;
#endif

// in qaxserverdll.cpp
extern bool qax_ownQApp;

struct QAxExceptInfo
{
    QAxExceptInfo(int c, const QString &s, const QString &d, const QString &x)
	: code(c), src(s), desc(d), context(x)
    {
    }
    int code;
    QString src;
    QString desc;
    QString context;
};


bool qt_sendSpontaneousEvent(QObject*, QEvent*);

/*
    \class QAxServerBase
    \brief The QAxServerBase class is an ActiveX control hosting a QWidget.

    \internal
*/
class QAxServerBase :
    public QObject,
    public IAxServerBase,
    public IDispatch,
    public IOleObject,
    public IOleControl,
#if defined Q_CC_GNU
#   if (__W32API_MAJOR_VERSION < 2 || (__W32API_MAJOR_VERSION == 2 && __W32API_MINOR_VERSION < 5))
    public IViewObject, // this should not be needed as IViewObject2 is meant to inherit from this,
                        // untill the mingw headers are fixed this will need to stay.
#   endif
#endif
    public IViewObject2,
    public IOleInPlaceObject,
    public IOleInPlaceActiveObject,
    public IProvideClassInfo2,
    public IConnectionPointContainer,
    public IPersistStream,
    public IPersistStreamInit,
    public IPersistStorage,
    public IPersistPropertyBag,
    public IPersistFile,
    public IDataObject
{
public:
    typedef QMap<QUuid,IConnectionPoint*> ConnectionPoints;
    typedef QMap<QUuid,IConnectionPoint*>::Iterator ConnectionPointsIterator;

    QAxServerBase(const QString &classname, IUnknown *outerUnknown);
    QAxServerBase(QObject *o);

    void init();

    ~QAxServerBase();

// Window creation
    HWND create(HWND hWndParent, RECT& rcPos);
    HMENU createPopup(QMenu *popup, HMENU oldMenu = 0);
    void createMenu(QMenuBar *menuBar);
    void removeMenu();

    static LRESULT QT_WIN_CALLBACK ActiveXProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

// Object registration with OLE
    void registerActiveObject(IUnknown *object);
    void revokeActiveObject();

// IUnknown
    unsigned long WINAPI AddRef()
    {
	if (m_outerUnknown)
	    return m_outerUnknown->AddRef();

	EnterCriticalSection(&refCountSection);
	unsigned long r = ++ref;
	LeaveCriticalSection(&refCountSection);

	return r;
    }
    unsigned long WINAPI Release()
    {
    	if (m_outerUnknown)
	    return m_outerUnknown->Release();

	EnterCriticalSection(&refCountSection);
	unsigned long r = --ref;
	LeaveCriticalSection(&refCountSection);

	if (!r) {
	    delete this;
	    return 0;
	}
	return r;
    }
    HRESULT WINAPI QueryInterface(REFIID iid, void **iface);
    HRESULT InternalQueryInterface(REFIID iid, void **iface);

// IAxServerBase
    IUnknown *clientSite() const
    {
	return m_spClientSite;
    }

    void emitPropertyChanged(const char*);
    bool emitRequestPropertyChange(const char*);
    QObject *qObject() const
    {
	return theObject;
    }
    void ensureMetaData();
    bool isPropertyExposed(int index);

    void reportError(int code, const QString &src, const QString &desc, const QString &context)
    {
        if (exception)
            delete exception;
        exception = new QAxExceptInfo(code, src, desc, context);
    }

// IDispatch
    STDMETHOD(GetTypeInfoCount)(UINT* pctinfo);
    STDMETHOD(GetTypeInfo)(UINT itinfo, LCID lcid, ITypeInfo** pptinfo);
    STDMETHOD(GetIDsOfNames)(REFIID riid, LPOLESTR* rgszNames, UINT cNames, LCID lcid, DISPID* rgdispid);
    STDMETHOD(Invoke)(DISPID dispidMember, REFIID riid,
		LCID lcid, WORD wFlags, DISPPARAMS* pdispparams, VARIANT* pvarResult,
		EXCEPINFO* pexcepinfo, UINT* puArgErr);

// IProvideClassInfo
    STDMETHOD(GetClassInfo)(ITypeInfo** pptinfo);

// IProvideClassInfo2
    STDMETHOD(GetGUID)(DWORD dwGuidKind, GUID* pGUID);

// IOleObject
    STDMETHOD(Advise)(IAdviseSink* pAdvSink, DWORD* pdwConnection);
    STDMETHOD(Close)(DWORD dwSaveOption);
    STDMETHOD(DoVerb)(LONG iVerb, LPMSG lpmsg, IOleClientSite* pActiveSite, LONG lindex, HWND hwndParent, LPCRECT lprcPosRect);
    STDMETHOD(EnumAdvise)(IEnumSTATDATA** ppenumAdvise);
    STDMETHOD(EnumVerbs)(IEnumOLEVERB** ppEnumOleVerb);
    STDMETHOD(GetClientSite)(IOleClientSite** ppClientSite);
    STDMETHOD(GetClipboardData)(DWORD dwReserved, IDataObject** ppDataObject);
    STDMETHOD(GetExtent)(DWORD dwDrawAspect, SIZEL* psizel);
    STDMETHOD(GetMiscStatus)(DWORD dwAspect, DWORD *pdwStatus);
    STDMETHOD(GetMoniker)(DWORD dwAssign, DWORD dwWhichMoniker, IMoniker** ppmk);
    STDMETHOD(GetUserClassID)(CLSID* pClsid);
    STDMETHOD(GetUserType)(DWORD dwFormOfType, LPOLESTR *pszUserType);
    STDMETHOD(InitFromData)(IDataObject* pDataObject, BOOL fCreation, DWORD dwReserved);
    STDMETHOD(IsUpToDate)();
    STDMETHOD(SetClientSite)(IOleClientSite* pClientSite);
    STDMETHOD(SetColorScheme)(LOGPALETTE* pLogPal);
    STDMETHOD(SetExtent)(DWORD dwDrawAspect, SIZEL* psizel);
    STDMETHOD(SetHostNames)(LPCOLESTR szContainerApp, LPCOLESTR szContainerObj);
    STDMETHOD(SetMoniker)(DWORD dwWhichMoniker, IMoniker* ppmk);
    STDMETHOD(Unadvise)(DWORD dwConnection);
    STDMETHOD(Update)();

// IViewObject
    STDMETHOD(Draw)(DWORD dwAspect, LONG lIndex, void *pvAspect, DVTARGETDEVICE *ptd,
		    HDC hicTargetDevice, HDC hdcDraw, LPCRECTL lprcBounds, LPCRECTL lprcWBounds,
		    BOOL(__stdcall*pfnContinue)(ULONG_PTR), ULONG_PTR dwContinue);
    STDMETHOD(GetColorSet)(DWORD dwDrawAspect, LONG lindex, void *pvAspect, DVTARGETDEVICE *ptd,
		    HDC hicTargetDev, LOGPALETTE **ppColorSet);
    STDMETHOD(Freeze)(DWORD dwAspect, LONG lindex, void *pvAspect, DWORD *pdwFreeze);
    STDMETHOD(Unfreeze)(DWORD dwFreeze);
    STDMETHOD(SetAdvise)(DWORD aspects, DWORD advf, IAdviseSink *pAdvSink);
    STDMETHOD(GetAdvise)(DWORD *aspects, DWORD *advf, IAdviseSink **pAdvSink);

// IViewObject2
    STDMETHOD(GetExtent)(DWORD dwAspect, LONG lindex, DVTARGETDEVICE *ptd, LPSIZEL lpsizel);

// IOleControl
    STDMETHOD(FreezeEvents)(BOOL);
    STDMETHOD(GetControlInfo)(LPCONTROLINFO);
    STDMETHOD(OnAmbientPropertyChange)(DISPID);
    STDMETHOD(OnMnemonic)(LPMSG);

// IOleWindow
    STDMETHOD(GetWindow)(HWND *pHwnd);
    STDMETHOD(ContextSensitiveHelp)(BOOL fEnterMode);

// IOleInPlaceObject
    STDMETHOD(InPlaceDeactivate)();
    STDMETHOD(UIDeactivate)();
    STDMETHOD(SetObjectRects)(LPCRECT lprcPosRect, LPCRECT lprcClipRect);
    STDMETHOD(ReactivateAndUndo)();

// IOleInPlaceActiveObject
    STDMETHOD(TranslateAcceleratorW)(MSG *pMsg);
    STDMETHOD(TranslateAcceleratorA)(MSG *pMsg);
    STDMETHOD(OnFrameWindowActivate)(BOOL);
    STDMETHOD(OnDocWindowActivate)(BOOL fActivate);
    STDMETHOD(ResizeBorder)(LPCRECT prcBorder, IOleInPlaceUIWindow *pUIWindow, BOOL fFrameWindow);
    STDMETHOD(EnableModeless)(BOOL);

// IConnectionPointContainer
    STDMETHOD(EnumConnectionPoints)(IEnumConnectionPoints**);
    STDMETHOD(FindConnectionPoint)(REFIID, IConnectionPoint**);

// IPersist
    STDMETHOD(GetClassID)(GUID*clsid)
    {
	*clsid = qAxFactory()->classID(class_name);
	return S_OK;
    }

// IPersistStreamInit
    STDMETHOD(InitNew)(VOID);
    STDMETHOD(IsDirty)();
    STDMETHOD(Load)(IStream *pStm);
    STDMETHOD(Save)(IStream *pStm, BOOL fClearDirty);
    STDMETHOD(GetSizeMax)(ULARGE_INTEGER *pcbSize);

// IPersistPropertyBag
    STDMETHOD(Load)(IPropertyBag *, IErrorLog *);
    STDMETHOD(Save)(IPropertyBag *, BOOL, BOOL);

// IPersistStorage
    STDMETHOD(InitNew)(IStorage *pStg);
    STDMETHOD(Load)(IStorage *pStg);
    STDMETHOD(Save)(IStorage *pStg, BOOL fSameAsLoad);
    STDMETHOD(SaveCompleted)(IStorage *pStgNew);
    STDMETHOD(HandsOffStorage)();

// IPersistFile
    STDMETHOD(SaveCompleted)(LPCOLESTR fileName);
    STDMETHOD(GetCurFile)(LPOLESTR *currentFile);
    STDMETHOD(Load)(LPCOLESTR fileName, DWORD mode);
    STDMETHOD(Save)(LPCOLESTR fileName, BOOL fRemember);

// IDataObject
    STDMETHOD(GetData)(FORMATETC *pformatetcIn, STGMEDIUM *pmedium);
    STDMETHOD(GetDataHere)(FORMATETC* /* pformatetc */, STGMEDIUM* /* pmedium */);
    STDMETHOD(QueryGetData)(FORMATETC* /* pformatetc */);
    STDMETHOD(GetCanonicalFormatEtc)(FORMATETC* /* pformatectIn */,FORMATETC* /* pformatetcOut */);
    STDMETHOD(SetData)(FORMATETC* /* pformatetc */, STGMEDIUM* /* pmedium */, BOOL /* fRelease */);
    STDMETHOD(EnumFormatEtc)(DWORD /* dwDirection */, IEnumFORMATETC** /* ppenumFormatEtc */);
    STDMETHOD(DAdvise)(FORMATETC *pformatetc, DWORD advf, IAdviseSink *pAdvSink, DWORD *pdwConnection);
    STDMETHOD(DUnadvise)(DWORD dwConnection);
    STDMETHOD(EnumDAdvise)(IEnumSTATDATA **ppenumAdvise);

// QObject
    int qt_metacall(QMetaObject::Call, int index, void **argv);

    bool eventFilter(QObject *o, QEvent *e);
private:
    void update();
    void resize(const QSize &newSize);
    void updateGeometry();
    void updateMask();
    bool internalCreate();
    void internalBind();
    void internalConnect();
    HRESULT internalActivate();

    friend class QAxBindable;
    friend class QAxPropertyPage;

    QAxAggregated *aggregatedObject;
    ConnectionPoints points;

    union {
	QWidget *widget;
	QObject *object;
    } qt;
    QPointer<QObject> theObject;
    unsigned isWidget		:1;
    unsigned ownObject		:1;
    unsigned initNewCalled	:1;
    unsigned dirtyflag		:1;
    unsigned hasStockEvents	:1;
    unsigned stayTopLevel	:1;
    unsigned isInPlaceActive	:1;
    unsigned isUIActive		:1;
    unsigned wasUIActive	:1;
    unsigned inDesignMode	:1;
    unsigned canTakeFocus	:1;
    short freezeEvents;

    HWND m_hWnd;

    HMENU hmenuShared;
    HOLEMENU holemenu;
    HWND hwndMenuOwner;
    QMap<HMENU, QMenu*> menuMap;
    QMap<UINT, QAction*> actionMap;
    QPointer<QMenuBar> menuBar;
    QPointer<QStatusBar> statusBar;
    QPointer<QMenu> currentPopup;
    QAxExceptInfo *exception;

    CRITICAL_SECTION refCountSection;
    CRITICAL_SECTION createWindowSection;

    unsigned long ref;
    unsigned long ole_ref;

    QString class_name;
    QString currentFileName;

    QHash<long, int> indexCache;
    QHash<int,DISPID> signalCache;

    IUnknown *m_outerUnknown;
    IAdviseSink *m_spAdviseSink;
    QList<STATDATA> adviseSinks;
    IOleClientSite *m_spClientSite;
    IOleInPlaceSiteWindowless *m_spInPlaceSite;
    IOleInPlaceFrame *m_spInPlaceFrame;
    ITypeInfo *m_spTypeInfo;
    IStorage *m_spStorage;
    QSize m_currentExtent;
};

class QAxServerAggregate : public IUnknown
{
public:
    QAxServerAggregate(const QString &className, IUnknown *outerUnknown)
	: m_outerUnknown(outerUnknown), ref(0)
    {
	object = new QAxServerBase(className, outerUnknown);
	object->registerActiveObject(this);

	InitializeCriticalSection(&refCountSection);
	InitializeCriticalSection(&createWindowSection);
    }
    ~QAxServerAggregate()
    {
	DeleteCriticalSection(&refCountSection);
	DeleteCriticalSection(&createWindowSection);

	delete object;
    }

// IUnknown
    unsigned long WINAPI AddRef()
    {
	EnterCriticalSection(&refCountSection);
	unsigned long r = ++ref;
	LeaveCriticalSection(&refCountSection);

	return r;
    }
    unsigned long WINAPI Release()
    {
	EnterCriticalSection(&refCountSection);
	unsigned long r = --ref;
	LeaveCriticalSection(&refCountSection);

	if (!r) {
	    delete this;
	    return 0;
	}
	return r;
    }
    HRESULT WINAPI QueryInterface(REFIID iid, void **iface)
    {
	*iface = 0;

	HRESULT res = E_NOINTERFACE;
	if (iid == IID_IUnknown) {
	    *iface = (IUnknown*)this;
	    AddRef();
	    return S_OK;
	}
	return object->InternalQueryInterface(iid, iface);
    }

private:
    QAxServerBase *object;
    IUnknown *m_outerUnknown;
    unsigned long ref;

    CRITICAL_SECTION refCountSection;
    CRITICAL_SECTION createWindowSection;
};

bool QAxFactory::createObjectWrapper(QObject *object, IDispatch **wrapper)
{
    *wrapper = 0;
    QAxServerBase *obj = new QAxServerBase(object);
    obj->QueryInterface(IID_IDispatch, (void**)wrapper);
    if (*wrapper)
	return true;

    delete obj;
    return false;
}


/*
    Helper class to enumerate all supported event interfaces.
*/
class QAxSignalVec : public IEnumConnectionPoints
{
public:
    QAxSignalVec(const QAxServerBase::ConnectionPoints &points)
        : cpoints(points.values())
        , current(0)
    , ref(0)
    {
	InitializeCriticalSection(&refCountSection);
        const int count = cpoints.count();
        for (int i = 0; i < count; ++i)
            cpoints.at(i)->AddRef();
    }
    QAxSignalVec(const QAxSignalVec &old)
        : cpoints(old.cpoints)
        , current(old.current)
    {
	InitializeCriticalSection(&refCountSection);
	ref = 0;
        const int count = cpoints.count();
        for (int i = 0; i < count; ++i)
            cpoints.at(i)->AddRef();
    }
    ~QAxSignalVec()
    {
        const int count = cpoints.count();
        for (int i = 0; i < count; ++i)
            cpoints.at(i)->Release();

	DeleteCriticalSection(&refCountSection);
    }

    unsigned long __stdcall AddRef()
    {
	EnterCriticalSection(&refCountSection);
	unsigned long r = ++ref;
	LeaveCriticalSection(&refCountSection);
	return ++r;
    }
    unsigned long __stdcall Release()
    {
	EnterCriticalSection(&refCountSection);
	unsigned long r = --ref;
	LeaveCriticalSection(&refCountSection);

	if (!r) {
	    delete this;
	    return 0;
	}
	return r;
    }
    STDMETHOD(QueryInterface)(REFIID iid, void **iface)
    {
        if (!iface)
            return E_POINTER;
	*iface = 0;
	if (iid == IID_IUnknown)
	    *iface = this;
	else if (iid == IID_IEnumConnectionPoints)
	    *iface = this;
	else
	    return E_NOINTERFACE;

	AddRef();
	return S_OK;
    }
    STDMETHOD(Next)(ULONG cConnections, IConnectionPoint **cpoint, ULONG *pcFetched)
    {
        if (!cpoint)
            return E_POINTER;

        if (!pcFetched && cConnections > 1)
            return E_POINTER;

        const int count = cpoints.count();
	unsigned long i;
	for (i = 0; i < cConnections; i++) {
        if (current==count)
		break;
        IConnectionPoint *cp = cpoints.at(current);
	    cp->AddRef();
	    cpoint[i] = cp;
        ++current;
	}
        if (pcFetched)
	*pcFetched = i;
	return i == cConnections ? S_OK : S_FALSE;
    }
    STDMETHOD(Skip)(ULONG cConnections)
    {
        const int count = cpoints.count();
	while (cConnections) {
        if (current == count)
            return S_FALSE;
        ++current;
	    --cConnections;
	}
	return S_OK;
    }
    STDMETHOD(Reset)()
    {
        current = 0;
	return S_OK;
    }
    STDMETHOD(Clone)(IEnumConnectionPoints **ppEnum)
    {
        if (!ppEnum)
            return E_POINTER;
	*ppEnum = new QAxSignalVec(*this);
	(*ppEnum)->AddRef();

	return S_OK;
    }

    QList<IConnectionPoint*> cpoints;
    int current;

private:
    CRITICAL_SECTION refCountSection;

    unsigned long ref;
};

/*
    Helper class to store and enumerate all connected event listeners.
*/
class QAxConnection : public IConnectionPoint,
		      public IEnumConnections
{
public:
    typedef QList<CONNECTDATA> Connections;
    typedef QList<CONNECTDATA>::Iterator Iterator;

    QAxConnection(QAxServerBase *parent, const QUuid &uuid)
        : that(parent), iid(uuid), current(0), ref(1)
    {
	InitializeCriticalSection(&refCountSection);
    }
    QAxConnection(const QAxConnection &old)
        : current(old.current)
    {
	InitializeCriticalSection(&refCountSection);
	ref = 0;
	connections = old.connections;
	that = old.that;
	iid = old.iid;
	QList<CONNECTDATA>::Iterator it = connections.begin();
	while (it != connections.end()) {
	    CONNECTDATA connection = *it;
	    ++it;
	    connection.pUnk->AddRef();
	}
    }
    ~QAxConnection()
    {
	DeleteCriticalSection(&refCountSection);
    }

    unsigned long __stdcall AddRef()
    {
	EnterCriticalSection(&refCountSection);
	unsigned long r = ++ref;
	LeaveCriticalSection(&refCountSection);
	return r;
    }
    unsigned long __stdcall Release()
    {
	EnterCriticalSection(&refCountSection);
	unsigned long r = --ref;
	LeaveCriticalSection(&refCountSection);

	if (!r) {
	    delete this;
	    return 0;
	}
	return r;
    }
    STDMETHOD(QueryInterface)(REFIID iid, void **iface)
    {
        if (!iface)
            return E_POINTER;
	*iface = 0;
	if (iid == IID_IUnknown)
	    *iface = (IConnectionPoint*)this;
	else if (iid == IID_IConnectionPoint)
	    *iface = this;
	else if (iid == IID_IEnumConnections)
	    *iface = this;
	else
	    return E_NOINTERFACE;

	AddRef();
	return S_OK;
    }
    STDMETHOD(GetConnectionInterface)(IID *pIID)
    {
	*pIID = iid;
	return S_OK;
    }
    STDMETHOD(GetConnectionPointContainer)(IConnectionPointContainer **ppCPC)
    {
	return that->QueryInterface(IID_IConnectionPointContainer, (void**)ppCPC);
    }
    STDMETHOD(Advise)(IUnknown*pUnk, DWORD *pdwCookie)
    {
        if (!pUnk || !pdwCookie)
            return E_POINTER;

	{
	    IDispatch *checkImpl = 0;
	    pUnk->QueryInterface(iid, (void**)&checkImpl);
	    if (!checkImpl)
		return CONNECT_E_CANNOTCONNECT;
	    checkImpl->Release();
	}

	CONNECTDATA cd;
	cd.dwCookie = connections.count()+1;
	cd.pUnk = pUnk;
	cd.pUnk->AddRef();
	connections.append(cd);
	*pdwCookie = cd.dwCookie;
	return S_OK;
    }
    STDMETHOD(Unadvise)(DWORD dwCookie)
    {
        const int count = connections.count();
        for (int i = 0; i < count; ++i) {
            if (connections.at(i).dwCookie == dwCookie) {
                connections.removeAt(i);
                if (current >= i && current != 0)
                    --current;
                return S_OK;
            }
        }
	return CONNECT_E_NOCONNECTION;
    }
    STDMETHOD(EnumConnections)(IEnumConnections **ppEnum)
    {
        if (!ppEnum)
            return E_POINTER;
	*ppEnum = this;
	AddRef();

	return S_OK;
    }
    STDMETHOD(Next)(ULONG cConnections, CONNECTDATA *cd, ULONG *pcFetched)
    {
        if (!cd)
            return E_POINTER;

        if (!pcFetched && cConnections > 1)
            return E_POINTER;

        const int count = connections.count();

	unsigned long i;
	for (i = 0; i < cConnections; i++) {
        if (current == count)
		break;
        cd[i] = connections.at(current);
	    cd[i].pUnk->AddRef();
        ++current;
	}
	if (pcFetched)
	    *pcFetched = i;
	return i == cConnections ? S_OK : S_FALSE;
    }
    STDMETHOD(Skip)(ULONG cConnections)
    {
        const int count = connections.count();
	while (cConnections) {
        if (current == count)
            return S_FALSE;
        ++current;
	    --cConnections;
	}
	return S_OK;
    }
    STDMETHOD(Reset)()
    {
        current = 0;
	return S_OK;
    }
    STDMETHOD(Clone)(IEnumConnections **ppEnum)
    {
        if (!ppEnum)
            return E_POINTER;
	*ppEnum = new QAxConnection(*this);
	(*ppEnum)->AddRef();

	return S_OK;
    }

private:
    QAxServerBase *that;
    QUuid iid;
    Connections connections;
    int current;

    CRITICAL_SECTION refCountSection;
    unsigned long ref;
};

// callback for DLL server to hook into non-Qt eventloop
LRESULT QT_WIN_CALLBACK axs_FilterProc(int nCode, WPARAM wParam, LPARAM lParam)
{
    if (qApp && !invokeCount)
        qApp->sendPostedEvents();

    return CallNextHookEx(qax_hhook, nCode, wParam, lParam);
}

// filter for executable case to hook into Qt eventloop
// for DLLs the client calls TranslateAccelerator
bool qax_winEventFilter(void *message)
{
    MSG *pMsg = (MSG*)message;
    if (pMsg->message < WM_KEYFIRST || pMsg->message > WM_KEYLAST)
	return false;

    bool ret = false;
    QWidget *aqt = QWidget::find(pMsg->hwnd);
    if (!aqt)
	return ret;

    HWND baseHwnd = ::GetParent(aqt->winId());
    QAxServerBase *axbase = 0;
    while (!axbase && baseHwnd) {
#ifdef GWLP_USERDATA
        axbase = (QAxServerBase*)GetWindowLongPtr(baseHwnd, GWLP_USERDATA);
#else
        axbase = (QAxServerBase*)GetWindowLong(baseHwnd, GWL_USERDATA);
#endif

	baseHwnd = ::GetParent(baseHwnd);
    }
    if (!axbase)
	return ret;

    HRESULT hres = axbase->TranslateAcceleratorW(pMsg);
    return hres == S_OK;
}

extern void qWinMsgHandler(QtMsgType t, const char* str);

// COM Factory class, mapping COM requests to ActiveQt requests.
// One instance of this class for each ActiveX the server can provide.
class QClassFactory : public IClassFactory2
{
public:
    QClassFactory(CLSID clsid)
	: ref(0), licensed(false)
    {
	InitializeCriticalSection(&refCountSection);

	// COM only knows the CLSID, but QAxFactory is class name based...
	QStringList keys = qAxFactory()->featureList();
	for (QStringList::Iterator  key = keys.begin(); key != keys.end(); ++key) {
	    if (qAxFactory()->classID(*key) == clsid) {
		className = *key;
		break;
	    }
	}

	const QMetaObject *mo = qAxFactory()->metaObject(className);
	if (mo) {
	    classKey = QLatin1String(mo->classInfo(mo->indexOfClassInfo("LicenseKey")).value());
	    licensed = !classKey.isEmpty();
	}
    }

    ~QClassFactory()
    {
	DeleteCriticalSection(&refCountSection);
    }

    // IUnknown
    unsigned long WINAPI AddRef()
    {
	EnterCriticalSection(&refCountSection);
	unsigned long r = ++ref;
	LeaveCriticalSection(&refCountSection);
	return ++r;
    }
    unsigned long WINAPI Release()
    {
	EnterCriticalSection(&refCountSection);
	unsigned long r = --ref;
	LeaveCriticalSection(&refCountSection);

	if (!r) {
	    delete this;
	    return 0;
	}
	return r;
    }
    HRESULT WINAPI QueryInterface(REFIID iid, LPVOID *iface)
    {
	*iface = 0;
	if (iid == IID_IUnknown)
	    *iface = (IUnknown*)this;
	else if (iid == IID_IClassFactory)
	    *iface = (IClassFactory*)this;
	else if (iid == IID_IClassFactory2 && licensed)
	    *iface = (IClassFactory2*)this;
	else
	    return E_NOINTERFACE;

	AddRef();
	return S_OK;
    }

    HRESULT WINAPI CreateInstanceHelper(IUnknown *pUnkOuter, REFIID iid, void **ppObject)
    {
	if (pUnkOuter) {
	    if (iid != IID_IUnknown)
		return CLASS_E_NOAGGREGATION;
	    const QMetaObject *mo = qAxFactory()->metaObject(className);
	    if (mo && !qstricmp(mo->classInfo(mo->indexOfClassInfo("Aggregatable")).value(), "no"))
		return CLASS_E_NOAGGREGATION;
	}

    	// Make sure a QApplication instance is present (inprocess case)
        if (!qApp) {
            qInstallMsgHandler(qWinMsgHandler);
            qax_ownQApp = true;
            int argc = 0;
            QApplication *app = new QApplication(argc, 0);
        }
        qApp->setQuitOnLastWindowClosed(false);

        if (qAxOutProcServer)
            QAbstractEventDispatcher::instance()->setEventFilter(qax_winEventFilter);
        else
            QApplication::instance()->d_func()->in_exec = true;

        // hook into eventloop; this allows a server to create his own QApplication object
        if (!qax_hhook && qax_ownQApp) {
            qax_hhook = SetWindowsHookEx(WH_GETMESSAGE, axs_FilterProc, 0, GetCurrentThreadId());
        }

	HRESULT res;
	// Create the ActiveX wrapper - aggregate if requested
	if (pUnkOuter) {
	    QAxServerAggregate *aggregate = new QAxServerAggregate(className, pUnkOuter);
	    res = aggregate->QueryInterface(iid, ppObject);
	    if (FAILED(res))
		delete aggregate;
	} else {
	    QAxServerBase *activeqt = new QAxServerBase(className, pUnkOuter);
	    res = activeqt->QueryInterface(iid, ppObject);
	    if (FAILED(res))
		delete activeqt;
	    else
		activeqt->registerActiveObject((IUnknown*)(IDispatch*)activeqt);
	}
	return res;
    }

    // IClassFactory
    HRESULT WINAPI CreateInstance(IUnknown *pUnkOuter, REFIID iid, void **ppObject)
    {
	// class is licensed
	if (licensed && !qAxFactory()->validateLicenseKey(className, QString()))
	    return CLASS_E_NOTLICENSED;

	return CreateInstanceHelper(pUnkOuter, iid, ppObject);
    }
    HRESULT WINAPI LockServer(BOOL fLock)
    {
	if (fLock)
	    qAxLock();
	else
	    qAxUnlock();

	return S_OK;
    }

    // IClassFactory2
    HRESULT WINAPI RequestLicKey(DWORD, BSTR *pKey)
    {
	if (!pKey)
	    return E_POINTER;
	*pKey = 0;

	// This of course works only on fully licensed machines
	if (!qAxFactory()->validateLicenseKey(className, QString()))
	    return CLASS_E_NOTLICENSED;

	*pKey = QStringToBSTR(classKey);
	return S_OK;
    }

    HRESULT WINAPI GetLicInfo(LICINFO *pLicInfo)
    {
	if (!pLicInfo)
	    return E_POINTER;
	pLicInfo->cbLicInfo = sizeof(LICINFO);

	// class specific license key?
	const QMetaObject *mo = qAxFactory()->metaObject(className);
	const char *key = mo->classInfo(mo->indexOfClassInfo("LicenseKey")).value();
	pLicInfo->fRuntimeKeyAvail = key && key[0];

	// machine fully licensed?
	pLicInfo->fLicVerified = qAxFactory()->validateLicenseKey(className, QString());

	return S_OK;
    }

    HRESULT WINAPI CreateInstanceLic(IUnknown *pUnkOuter, IUnknown *pUnkReserved, REFIID iid, BSTR bKey, PVOID *ppObject)
    {
        QString licenseKey = QString::fromWCharArray(bKey);
	if (!qAxFactory()->validateLicenseKey(className, licenseKey))
	    return CLASS_E_NOTLICENSED;
	return CreateInstanceHelper(pUnkOuter, iid, ppObject);
    }

    QString className;

protected:
    CRITICAL_SECTION refCountSection;
    unsigned long ref;
    bool licensed;
    QString classKey;
};

// Create a QClassFactory object for class \a iid
HRESULT GetClassObject(REFIID clsid, REFIID iid, void **ppUnk)
{
    QClassFactory *factory = new QClassFactory(clsid);
    if (!factory)
	return E_OUTOFMEMORY;
    if (factory->className.isEmpty()) {
	delete factory;
	return E_NOINTERFACE;
    }
    HRESULT res = factory->QueryInterface(iid, ppUnk);
    if (res != S_OK)
	delete factory;
    return res;
}


/*!
    Constructs a QAxServerBase object wrapping the QWidget \a
    classname into an ActiveX control.

    The constructor is called by the QClassFactory object provided by
    the COM server for the respective CLSID.
*/
QAxServerBase::QAxServerBase(const QString &classname, IUnknown *outerUnknown)
: aggregatedObject(0), ref(0), ole_ref(0), class_name(classname),
  m_hWnd(0), hmenuShared(0), hwndMenuOwner(0),
  m_outerUnknown(outerUnknown)
{
    init();

    internalCreate();
}

/*!
    Constructs a QAxServerBase object wrapping \a o.
*/
QAxServerBase::QAxServerBase(QObject *o)
: aggregatedObject(0), ref(0), ole_ref(0),
  m_hWnd(0), hmenuShared(0), hwndMenuOwner(0),
  m_outerUnknown(0)
{
    init();

    qt.object = o;
    if (o) {
	theObject = o;
	isWidget = false;
	class_name = QLatin1String(o->metaObject()->className());
    }
    internalBind();
    internalConnect();
}

/*!
    Initializes data members.
*/
void QAxServerBase::init()
{
    qt.object = 0;
    isWidget		= false;
    ownObject		= false;
    initNewCalled	= false;
    dirtyflag		= false;
    hasStockEvents	= false;
    stayTopLevel	= false;
    isInPlaceActive	= false;
    isUIActive		= false;
    wasUIActive		= false;
    inDesignMode	= false;
    canTakeFocus	= false;
    freezeEvents = 0;
    exception = 0;

    m_spAdviseSink = 0;
    m_spClientSite = 0;
    m_spInPlaceSite = 0;
    m_spInPlaceFrame = 0;
    m_spTypeInfo = 0;
    m_spStorage = 0;

    InitializeCriticalSection(&refCountSection);
    InitializeCriticalSection(&createWindowSection);

#ifdef QT_DEBUG
    EnterCriticalSection(&refCountSection);
    ++qaxserverbase_instance_count;
    LeaveCriticalSection(&refCountSection);
#endif

    qAxLock();

    points[IID_IPropertyNotifySink] = new QAxConnection(this, IID_IPropertyNotifySink);
}

/*!
    Destroys the QAxServerBase object, releasing all allocated
    resources and interfaces.
*/
QAxServerBase::~QAxServerBase()
{
#ifdef QT_DEBUG
    EnterCriticalSection(&refCountSection);
    --qaxserverbase_instance_count;
    LeaveCriticalSection(&refCountSection);
#endif

    revokeActiveObject();

    for (QAxServerBase::ConnectionPointsIterator it = points.begin(); it != points.end(); ++it) {
	if (it.value())
	    (*it)->Release();
    }
    delete aggregatedObject;
    aggregatedObject = 0;
    if (theObject) {
	qt.object->disconnect(this);
	QObject *aqt = qt.object;
	qt.object = 0;
	if (ownObject)
	    delete aqt;
    }

    if (m_spAdviseSink) m_spAdviseSink->Release();
    m_spAdviseSink = 0;
    for (int i = 0; i < adviseSinks.count(); ++i) {
        adviseSinks.at(i).pAdvSink->Release();
    }
    if (m_spClientSite) m_spClientSite->Release();
    m_spClientSite = 0;
    if (m_spInPlaceFrame) m_spInPlaceFrame->Release();
    m_spInPlaceFrame = 0;
    if (m_spInPlaceSite) m_spInPlaceSite->Release();
    m_spInPlaceSite = 0;
    if (m_spTypeInfo) m_spTypeInfo->Release();
    m_spTypeInfo = 0;
    if (m_spStorage) m_spStorage->Release();
    m_spStorage = 0;

    DeleteCriticalSection(&refCountSection);
    DeleteCriticalSection(&createWindowSection);

    qAxUnlock();
}

/*
    Registering with OLE
*/
void QAxServerBase::registerActiveObject(IUnknown *object)
{
    if (ole_ref || !qt.object || !qAxOutProcServer)
	return;

    const QMetaObject *mo = qt.object->metaObject();
    if (!qstricmp(mo->classInfo(mo->indexOfClassInfo("RegisterObject")).value(), "yes"))
	RegisterActiveObject(object, qAxFactory()->classID(class_name), ACTIVEOBJECT_WEAK, &ole_ref);
}

void QAxServerBase::revokeActiveObject()
{
    if (!ole_ref)
	return;

    RevokeActiveObject(ole_ref, 0);
    ole_ref = 0;
}

/*
    QueryInterface implementation.
*/
HRESULT WINAPI QAxServerBase::QueryInterface(REFIID iid, void **iface)
{
    if (m_outerUnknown)
	return m_outerUnknown->QueryInterface(iid, iface);

    return InternalQueryInterface(iid, iface);
}

HRESULT QAxServerBase::InternalQueryInterface(REFIID iid, void **iface)
{
    *iface = 0;

    if (iid == IID_IUnknown) {
	*iface = (IUnknown*)(IDispatch*)this;
    } else {
	HRESULT res = S_OK;
	if (aggregatedObject)
	    res = aggregatedObject->queryInterface(iid, iface);
	if (*iface)
	    return res;
    }

    if (!(*iface)) {
	if (iid == qAxFactory()->interfaceID(class_name))
	    *iface = (IDispatch*)this;
	if (iid == IID_IDispatch)
	    *iface = (IDispatch*)this;
	else if (iid == IID_IAxServerBase)
	    *iface = (IAxServerBase*)this;
	else if (iid == IID_IOleObject)
	    *iface = (IOleObject*)this;
	else if (iid == IID_IConnectionPointContainer)
	    *iface = (IConnectionPointContainer*)this;
	else if (iid == IID_IProvideClassInfo)
	    *iface = (IProvideClassInfo*)this;
	else if (iid == IID_IProvideClassInfo2)
	    *iface = (IProvideClassInfo2*)this;
	else if (iid == IID_IPersist)
	    *iface = (IPersist*)(IPersistStream*)this;
	else if (iid == IID_IPersistStream)
	    *iface = (IPersistStream*)this;
	else if (iid == IID_IPersistStreamInit)
	    *iface = (IPersistStreamInit*)this;
	else if (iid == IID_IPersistStorage)
	    *iface = (IPersistStorage*)this;
	else if (iid == IID_IPersistPropertyBag)
	    *iface = (IPersistPropertyBag*)this;
        else if (iid == IID_IPersistFile &&
            qAxFactory()->metaObject(class_name)->indexOfClassInfo("MIME") != -1)
            *iface = (IPersistFile*)this;
	else if (iid == IID_IViewObject)
	    *iface = (IViewObject*)this;
	else if (iid == IID_IViewObject2)
	    *iface = (IViewObject2*)this;
	else if (isWidget) {
	    if (iid == IID_IOleControl)
		*iface = (IOleControl*)this;
	    else if (iid == IID_IOleWindow)
		*iface = (IOleWindow*)(IOleInPlaceObject*)this;
	    else if (iid == IID_IOleInPlaceObject)
		*iface = (IOleInPlaceObject*)this;
	    else if (iid == IID_IOleInPlaceActiveObject)
		*iface = (IOleInPlaceActiveObject*)this;
	    else if (iid == IID_IDataObject)
		*iface = (IDataObject*)this;
	}
    }
    if (!*iface)
	return E_NOINTERFACE;

    AddRef();
    return S_OK;
}

/*!
    Detects and initilaizes implementation of QAxBindable in objects.
*/
void QAxServerBase::internalBind()
{
    QAxBindable *axb = (QAxBindable*)qt.object->qt_metacast("QAxBindable");
    if (axb) {
	// no addref; this is aggregated
	axb->activex = this;
	if (!aggregatedObject)
	    aggregatedObject = axb->createAggregate();
	if (aggregatedObject) {
	    aggregatedObject->controlling_unknown = (IUnknown*)(IDispatch*)this;
	    aggregatedObject->the_object = qt.object;
	}
    }
}

/*!
    Connects object signals to event dispatcher.
*/
void QAxServerBase::internalConnect()
{
    QUuid eventsID = qAxFactory()->eventsID(class_name);
    if (!eventsID.isNull()) {
	if (!points[eventsID])
	    points[eventsID] = new QAxConnection(this, eventsID);

	// connect the generic slot to all signals of qt.object
	const QMetaObject *mo = qt.object->metaObject();
        for (int isignal = mo->methodCount()-1; isignal >= 0; --isignal) {
            if (mo->method(isignal).methodType() == QMetaMethod::Signal)
	        QMetaObject::connect(qt.object, isignal, this, isignal);
        }
    }
}

/*!
    Creates the QWidget for the classname passed to the c'tor.

    All signals of the widget class are connected to the internal event mapper.
    If the widget implements QAxBindable, stock events are also connected.
*/
bool QAxServerBase::internalCreate()
{
    if (qt.object)
	return true;

    qt.object = qAxFactory()->createObject(class_name);
    Q_ASSERT(qt.object);
    if (!qt.object)
	return false;

    theObject = qt.object;
    ownObject = true;
    isWidget = qt.object->isWidgetType();
    hasStockEvents = qAxFactory()->hasStockEvents(class_name);
    stayTopLevel = qAxFactory()->stayTopLevel(class_name);

    internalBind();
    if (isWidget) {
        if (!stayTopLevel) {
            QEvent e(QEvent::EmbeddingControl);
            QApplication::sendEvent(qt.widget, &e);
            ::SetWindowLong(qt.widget->winId(), GWL_STYLE, WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS);
        }
        qt.widget->setAttribute(Qt::WA_QuitOnClose, false);
        qt.widget->move(0, 0);

        // initialize to sizeHint, but don't set resized flag so that container has a chance to override
        bool wasResized = qt.widget->testAttribute(Qt::WA_Resized);
        updateGeometry();
        if (!wasResized && qt.widget->testAttribute(Qt::WA_Resized)
            && qt.widget->sizePolicy() != QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed)) {
            qt.widget->setAttribute(Qt::WA_Resized, false);
        }
    }

    internalConnect();
    // install an event filter for stock events
    if (isWidget) {
        qt.object->installEventFilter(this);
        const QList<QWidget*> children = qt.object->findChildren<QWidget*>();
        QList<QWidget*>::ConstIterator it = children.constBegin();
        while (it != children.constEnd()) {
            (*it)->installEventFilter(this);
            ++it;
        }
    }
    return true;
}

/*
class HackMenuData : public QMenuData
{
    friend class QAxServerBase;
};
*/

class HackWidget : public QWidget
{
    friend class QAxServerBase;
};
/*
    Message handler. \a hWnd is always the ActiveX widget hosting the Qt widget.
    \a uMsg is handled as follows
    \list
    \i WM_CREATE The ActiveX control is created
    \i WM_DESTROY The QWidget is destroyed
    \i WM_SHOWWINDOW The QWidget is parented into the ActiveX window
    \i WM_PAINT The QWidget is updated
    \i WM_SIZE The QWidget is resized to the new size
    \i WM_SETFOCUS and
    \i WM_KILLFOCUS The client site is notified about the focus transfer
    \i WM_MOUSEACTIVATE The ActiveX is activated
    \endlist

    The semantics of \a wParam and \a lParam depend on the value of \a uMsg.
*/
LRESULT QT_WIN_CALLBACK QAxServerBase::ActiveXProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (uMsg == WM_CREATE) {
        CREATESTRUCT *cs = (CREATESTRUCT*)lParam;
        QAxServerBase *that = (QAxServerBase*)cs->lpCreateParams;

#ifdef GWLP_USERDATA
        SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)that);
#else
        SetWindowLong(hWnd, GWL_USERDATA, (LONG)that);
#endif

        that->m_hWnd = hWnd;

        return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
    }

    QAxServerBase *that = 0;

#ifdef GWLP_USERDATA
    that = (QAxServerBase*)GetWindowLongPtr(hWnd, GWLP_USERDATA);
#else
    that = (QAxServerBase*)GetWindowLong(hWnd, GWL_USERDATA);
#endif

    if (that) {
        int width = that->qt.widget ? that->qt.widget->width() : 0;
        int height = that->qt.widget ? that->qt.widget->height() : 0;
        RECT rcPos = {0, 0, width + 1, height + 1};

        switch (uMsg) {
        case WM_NCDESTROY:
	    that->m_hWnd = 0;
	    break;

        case WM_QUERYENDSESSION:
        case WM_DESTROY:
            // save the window handle
            if (that->qt.widget) {
                that->qt.widget->hide();
                ::SetParent(that->qt.widget->winId(), 0);
            }
	    break;

        case WM_SHOWWINDOW:
	    if(wParam) {
	        that->internalCreate();
	        if (!that->stayTopLevel) {
		    ::SetParent(that->qt.widget->winId(), that->m_hWnd);
		    that->qt.widget->raise();
		    that->qt.widget->move(0, 0);
	        }
	        that->qt.widget->show();
	    } else if (that->qt.widget) {
	        that->qt.widget->hide();
	    }
	    break;

        case WM_ERASEBKGND:
	    that->updateMask();
	    break;

        case WM_SIZE:
            that->resize(QSize(LOWORD(lParam), HIWORD(lParam)));
	    break;

        case WM_SETFOCUS:
	    if (that->isInPlaceActive && that->m_spClientSite && !that->inDesignMode && that->canTakeFocus) {
	        that->DoVerb(OLEIVERB_UIACTIVATE, NULL, that->m_spClientSite, 0, that->m_hWnd, &rcPos);
	        if (that->isUIActive) {
		    IOleControlSite *spSite = 0;
		    that->m_spClientSite->QueryInterface(IID_IOleControlSite, (void**)&spSite);
		    if (spSite) {
		        spSite->OnFocus(true);
		        spSite->Release();
		    }
                    QWidget *candidate = that->qt.widget;
                    while (!(candidate->focusPolicy() & Qt::TabFocus)) {
                        candidate = candidate->nextInFocusChain();
                        if (candidate == that->qt.widget) {
                            candidate = 0;
                            break;
                        }
                    }
                    if (candidate) {
                        candidate->setFocus();
                        HackWidget *widget = (HackWidget*)that->qt.widget;
                        if (::GetKeyState(VK_SHIFT) < 0)
                            widget->focusNextPrevChild(false);
                    }
	        }
	    }
	    break;

        case WM_KILLFOCUS:
	    if (that->isInPlaceActive && that->isUIActive && that->m_spClientSite) {
	        IOleControlSite *spSite = 0;
	        that->m_spClientSite->QueryInterface(IID_IOleControlSite, (void**)&spSite);
	        if (spSite) {
		    if (!::IsChild(that->m_hWnd, ::GetFocus()))
		        spSite->OnFocus(false);
		    spSite->Release();
	        }
	    }
	    break;

        case WM_MOUSEACTIVATE:
	    that->DoVerb(OLEIVERB_UIACTIVATE, NULL, that->m_spClientSite, 0, that->m_hWnd, &rcPos);
	    break;

        case WM_INITMENUPOPUP:
	    if (that->qt.widget) {
	        that->currentPopup = that->menuMap[(HMENU)wParam];
	        if (!that->currentPopup)
		    break;
	        const QMetaObject *mo = that->currentPopup->metaObject();
	        int index = mo->indexOfSignal("aboutToShow()");
	        if (index < 0)
		    break;

	        that->currentPopup->qt_metacall(QMetaObject::InvokeMetaMethod, index, 0);
	        that->createPopup(that->currentPopup, (HMENU)wParam);
	        return 0;
	    }
	    break;

        case WM_MENUSELECT:
        case WM_COMMAND:
	    if (that->qt.widget) {
	        QMenuBar *menuBar = that->menuBar;
	        if (!menuBar)
		    break;

                QObject *menuObject = 0;
	        bool menuClosed = false;

                if (uMsg == WM_COMMAND) {
		    menuObject = that->actionMap.value(wParam);
                } else if (!lParam) {
		    menuClosed = true;
                    menuObject = that->currentPopup;
                } else {
                    menuObject = that->actionMap.value(LOWORD(wParam));
                }

	        if (menuObject) {
		    const QMetaObject *mo = menuObject->metaObject();
		    int index = -1;

		    if (uMsg == WM_COMMAND)
		        index = mo->indexOfSignal("activated()");
		    else if (menuClosed)
		        index = mo->indexOfSignal("aboutToHide()");
		    else
		        index = mo->indexOfSignal("hovered()");

		    if (index < 0)
		        break;

		    menuObject->qt_metacall(QMetaObject::InvokeMetaMethod, index, 0);
                    if (menuClosed || uMsg == WM_COMMAND)
                        that->currentPopup = 0;
		    return 0;
	        }
	    }
	    break;

        default:
	    break;
        }
    }

    return ::DefWindowProc(hWnd, uMsg, wParam, lParam);
}

/*!
    Creates the window hosting the QWidget.
*/
HWND QAxServerBase::create(HWND hWndParent, RECT& rcPos)
{
    Q_ASSERT(isWidget && qt.widget);

    static ATOM atom = 0;
    HINSTANCE hInst = (HINSTANCE)qAxInstance;
    EnterCriticalSection(&createWindowSection);
    QString cn(QLatin1String("QAxControl"));
    cn += QString::number((quintptr)ActiveXProc);
    if (!atom) {
        WNDCLASS wcTemp;
        wcTemp.style = CS_DBLCLKS;
        wcTemp.cbClsExtra = 0;
        wcTemp.cbWndExtra = 0;
        wcTemp.hbrBackground = 0;
        wcTemp.hCursor = 0;
        wcTemp.hIcon = 0;
        wcTemp.hInstance = hInst;
        wcTemp.lpszClassName = (wchar_t*)cn.utf16();
        wcTemp.lpszMenuName = 0;
        wcTemp.lpfnWndProc = ActiveXProc;

        atom = RegisterClass(&wcTemp);
    }
    LeaveCriticalSection(&createWindowSection);
    if (!atom  && GetLastError() != ERROR_CLASS_ALREADY_EXISTS)
	return 0;

    Q_ASSERT(!m_hWnd);
    HWND hWnd = ::CreateWindow((wchar_t*)cn.utf16(), 0,
                               WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
                               rcPos.left, rcPos.top, rcPos.right - rcPos.left,
                               rcPos.bottom - rcPos.top, hWndParent, 0, hInst, this);

    Q_ASSERT(m_hWnd == hWnd);

    updateMask();
    EnableWindow(m_hWnd, qt.widget->isEnabled());

    return hWnd;
}

/*
    Recoursively creates Win32 submenus.
*/
HMENU QAxServerBase::createPopup(QMenu *popup, HMENU oldMenu)
{
    HMENU popupMenu = oldMenu ? oldMenu : CreatePopupMenu();
    menuMap.insert(popupMenu, popup);

    if (oldMenu) while (GetMenuItemCount(oldMenu)) {
	DeleteMenu(oldMenu, 0, MF_BYPOSITION);
    }

    const QList<QAction*> actions = popup->actions();
    for (int i = 0; i < actions.count(); ++i) {
        QAction *action = actions.at(i);

        uint flags = action->isEnabled() ? MF_ENABLED : MF_GRAYED;
        if (action->isSeparator())
            flags |= MF_SEPARATOR;
        else if (action->menu())
            flags |= MF_POPUP;
        else
            flags |= MF_STRING;
        if (action->isChecked())
            flags |= MF_CHECKED;

	ushort itemId;
        if (flags & MF_POPUP) {
            itemId = static_cast<ushort>(
                reinterpret_cast<quintptr>(createPopup(action->menu()))
            );
        } else {
            itemId = static_cast<ushort>(reinterpret_cast<quintptr>(action));
            actionMap.remove(itemId);
            actionMap.insert(itemId, action);
        }
        AppendMenu(popupMenu, flags, itemId, (const wchar_t *)action->text().utf16());
    }
    if (oldMenu)
        DrawMenuBar(hwndMenuOwner);
    return popupMenu;
}

/*!
    Creates a Win32 menubar.
*/
void QAxServerBase::createMenu(QMenuBar *menuBar)
{
    hmenuShared = ::CreateMenu();

    int edit = 0;
    int object = 0;
    int help = 0;

    const QList<QAction*> actions = menuBar->actions();
    for (int i = 0; i < actions.count(); ++i) {
        QAction *action = actions.at(i);

        uint flags = action->isEnabled() ? MF_ENABLED : MF_GRAYED;
	if (action->isSeparator())
	    flags |= MF_SEPARATOR;
	else if (action->menu())
	    flags |= MF_POPUP;
	else
	    flags |= MF_STRING;

	if (action->text() == QCoreApplication::translate(qt.widget->metaObject()->className(), "&Edit"))
	    edit++;
	else if (action->text() == QCoreApplication::translate(qt.widget->metaObject()->className(), "&Help"))
	    help++;
	else
	    object++;

	ushort itemId;
        if (flags & MF_POPUP) {
            itemId = static_cast<ushort>(
                reinterpret_cast<quintptr>(createPopup(action->menu()))
            );
        } else {
            itemId = static_cast<ushort>(reinterpret_cast<quintptr>(action));
            actionMap.insert(itemId, action);
        }
        AppendMenu(hmenuShared, flags, itemId, (const wchar_t *)action->text().utf16());
    }

    OLEMENUGROUPWIDTHS menuWidths = {0,edit,0,object,0,help};
    HRESULT hres = m_spInPlaceFrame->InsertMenus(hmenuShared, &menuWidths);
    if (FAILED(hres)) {
	::DestroyMenu(hmenuShared);
	hmenuShared = 0;
	return;
    }

    m_spInPlaceFrame->GetWindow(&hwndMenuOwner);

    holemenu = OleCreateMenuDescriptor(hmenuShared, &menuWidths);
    hres = m_spInPlaceFrame->SetMenu(hmenuShared, holemenu, m_hWnd);
    if (FAILED(hres)) {
	::DestroyMenu(hmenuShared);
	hmenuShared = 0;
	OleDestroyMenuDescriptor(holemenu);
    }
}

/*!
    Remove the Win32 menubar.
*/
void QAxServerBase::removeMenu()
{
    if (hmenuShared)
	m_spInPlaceFrame->RemoveMenus(hmenuShared);
    holemenu = 0;
    m_spInPlaceFrame->SetMenu(0, 0, m_hWnd);
    if (hmenuShared) {
	DestroyMenu(hmenuShared);
	hmenuShared = 0;
	menuMap.clear();
    }
    hwndMenuOwner = 0;
}

extern bool ignoreSlots(const char *test);
extern bool ignoreProps(const char *test);

/*!
    Makes sure the type info is loaded
*/
void QAxServerBase::ensureMetaData()
{
    if (!m_spTypeInfo) {
	qAxTypeLibrary->GetTypeInfoOfGuid(qAxFactory()->interfaceID(class_name), &m_spTypeInfo);
	m_spTypeInfo->AddRef();
    }
}

/*!
    \internal
    Returns true if the property \a index is exposed to COM and should
    be saved/loaded.
*/
bool QAxServerBase::isPropertyExposed(int index)
{
    if (!theObject)
	return false;

    bool result = false;
    const QMetaObject *mo = theObject->metaObject();

    int qtProps = 0;
    if (theObject->isWidgetType())
	qtProps = QWidget::staticMetaObject.propertyCount();
    QMetaProperty property = mo->property(index);
    if (index <= qtProps && ignoreProps(property.name()))
	return result;

    BSTR bstrNames = QStringToBSTR(QLatin1String(property.name()));
    DISPID dispId;
    GetIDsOfNames(IID_NULL, (BSTR*)&bstrNames, 1, LOCALE_USER_DEFAULT, &dispId);
    result = dispId != DISPID_UNKNOWN;
    SysFreeString(bstrNames);

    return result;
}


/*!
    \internal
    Updates the view, or asks the client site to do so.
*/
void QAxServerBase::update()
{
    if (isInPlaceActive) {
	if (m_hWnd)
	    ::InvalidateRect(m_hWnd, 0, true);
	else if (m_spInPlaceSite)
	    m_spInPlaceSite->InvalidateRect(NULL, true);
    } else if (m_spAdviseSink) {
        m_spAdviseSink->OnViewChange(DVASPECT_CONTENT, -1);
        for (int i = 0; i < adviseSinks.count(); ++i) {
	    adviseSinks.at(i).pAdvSink->OnViewChange(DVASPECT_CONTENT, -1);
        }
    }
}

/*!
    Resizes the control, faking a QResizeEvent if required
*/
void QAxServerBase::resize(const QSize &size)
{
    if (!isWidget || !qt.widget || !size.isValid() || size == QSize(0, 0))
        return;

    QSize oldSize = qt.widget->size();
    qt.widget->resize(size);
    QSize newSize = qt.widget->size();

    // make sure we get a resize event even if not embedded as a control
    if (! m_hWnd && !qt.widget->isVisible() && newSize != oldSize) {
        QResizeEvent resizeEvent(newSize, oldSize);

#ifdef QT_STATIC      // import from static library
        extern bool qt_sendSpontaneousEvent(QObject*,QEvent*);
#endif
        qt_sendSpontaneousEvent(qt.widget, &resizeEvent);
    }
    m_currentExtent = qt.widget->size();
}

/*!
    \internal

    Updates the internal size values.
*/
void QAxServerBase::updateGeometry()
{
    if (!isWidget || !qt.widget)
	return;

    const QSize sizeHint = qt.widget->sizeHint();
    const QSize size = qt.widget->size();
    if (sizeHint.isValid()) { // if provided, adjust to sizeHint
        QSize newSize = size;
        if (!qt.widget->testAttribute(Qt::WA_Resized)) {
            newSize = sizeHint;
        } else { // according to sizePolicy rules if already resized
            QSizePolicy sizePolicy = qt.widget->sizePolicy();
            if (sizeHint.width() > size.width() && !(sizePolicy.horizontalPolicy() & QSizePolicy::ShrinkFlag))
	        newSize.setWidth(sizeHint.width());
            if (sizeHint.width() < size.width() && !(sizePolicy.horizontalPolicy() & QSizePolicy::GrowFlag))
                newSize.setWidth(sizeHint.width());
            if (sizeHint.height() > size.height() && !(sizePolicy.verticalPolicy() & QSizePolicy::ShrinkFlag))
	        newSize.setHeight(sizeHint.height());
            if (sizeHint.height() < size.height() && !(sizePolicy.verticalPolicy() & QSizePolicy::GrowFlag))
                newSize.setHeight(sizeHint.height());
        }
        resize(newSize);

    // set an initial size suitable for embedded controls
    } else if (!qt.widget->testAttribute(Qt::WA_Resized)) {
        resize(QSize(100, 100));
        qt.widget->setAttribute(Qt::WA_Resized, false);
    }
}

/*!
    \internal

    Updates the mask of the widget parent.
*/
void QAxServerBase::updateMask()
{
    if (!isWidget || !qt.widget || qt.widget->mask().isEmpty())
	return;

    QRegion rgn = qt.widget->mask();
    HRGN hrgn = rgn.handle();

    // Since SetWindowRegion takes ownership
    HRGN wr = CreateRectRgn(0,0,0,0);
    CombineRgn(wr, hrgn, 0, RGN_COPY);
    SetWindowRgn(m_hWnd, wr, true);
}

static bool checkHRESULT(HRESULT hres)
{
    const char *name = 0;
    switch(hres) {
    case S_OK:
	return true;
    case DISP_E_BADPARAMCOUNT:
#if defined(QT_CHECK_STATE)
	qWarning("QAxBase: Error calling IDispatch member %s: Bad parameter count", name);
#endif
	return false;
    case DISP_E_BADVARTYPE:
#if defined(QT_CHECK_STATE)
	qWarning("QAxBase: Error calling IDispatch member %s: Bad variant type", name);
#endif
	return false;
    case DISP_E_EXCEPTION:
#if defined(QT_CHECK_STATE)
	    qWarning("QAxBase: Error calling IDispatch member %s: Exception thrown by server", name);
#endif
	return false;
    case DISP_E_MEMBERNOTFOUND:
#if defined(QT_CHECK_STATE)
	qWarning("QAxBase: Error calling IDispatch member %s: Member not found", name);
#endif
	return false;
    case DISP_E_NONAMEDARGS:
#if defined(QT_CHECK_STATE)
	qWarning("QAxBase: Error calling IDispatch member %s: No named arguments", name);
#endif
	return false;
    case DISP_E_OVERFLOW:
#if defined(QT_CHECK_STATE)
	qWarning("QAxBase: Error calling IDispatch member %s: Overflow", name);
#endif
	return false;
    case DISP_E_PARAMNOTFOUND:
#if defined(QT_CHECK_STATE)
	qWarning("QAxBase: Error calling IDispatch member %s: Parameter not found", name);
#endif
	return false;
    case DISP_E_TYPEMISMATCH:
#if defined(QT_CHECK_STATE)
	qWarning("QAxBase: Error calling IDispatch member %s: Type mismatch", name);
#endif
	return false;
    case DISP_E_UNKNOWNINTERFACE:
#if defined(QT_CHECK_STATE)
	qWarning("QAxBase: Error calling IDispatch member %s: Unknown interface", name);
#endif
	return false;
    case DISP_E_UNKNOWNLCID:
#if defined(QT_CHECK_STATE)
	qWarning("QAxBase: Error calling IDispatch member %s: Unknown locale ID", name);
#endif
	return false;
    case DISP_E_PARAMNOTOPTIONAL:
#if defined(QT_CHECK_STATE)
	qWarning("QAxBase: Error calling IDispatch member %s: Non-optional parameter missing", name);
#endif
	return false;
    default:
#if defined(QT_CHECK_STATE)
	qWarning("QAxBase: Error calling IDispatch member %s: Unknown error", name);
#endif
	return false;
    }
}

static inline QByteArray paramType(const QByteArray &ptype, bool *out)
{
    *out = ptype.endsWith('&') || ptype.endsWith("**");
    if (*out) {
        QByteArray res(ptype);
	res.truncate(res.length() - 1);
        return res;
    }

    return ptype;
}

/*!
    Catches all signals emitted by the Qt widget and fires the respective COM event.

    \a isignal is the Qt Meta Object index of the received signal, and \a _o the
    signal parameters.
*/
int QAxServerBase::qt_metacall(QMetaObject::Call call, int index, void **argv)
{
    Q_ASSERT(call == QMetaObject::InvokeMetaMethod);

    if (index == -1) {
        if (sender() && m_spInPlaceFrame) {
            if (qobject_cast<QStatusBar*>(sender()) != statusBar)
                return true;

            if (statusBar->isHidden()) {
                QString message = *(QString*)argv[1];
                m_spInPlaceFrame->SetStatusText(QStringToBSTR(message));
            }
        }
        return true;
    }

    if (freezeEvents || inDesignMode)
        return true;

    ensureMetaData();

    // get the signal information.
    const QMetaObject *mo = qt.object->metaObject();
    QMetaMethod signal;
    DISPID eventId = index;
    int pcount = 0;
    QByteArray type;
    QList<QByteArray> ptypes;

    switch(index) {
    case DISPID_KEYDOWN:
    case DISPID_KEYUP:
        pcount = 2;
        ptypes << "int&" << "int";
        break;
    case DISPID_KEYPRESS:
        pcount = 1;
        ptypes << "int&";
        break;
    case DISPID_MOUSEDOWN:
    case DISPID_MOUSEMOVE:
    case DISPID_MOUSEUP:
        pcount = 4;
        ptypes << "int" << "int" << "int" << "int";
        break;
    case DISPID_CLICK:
        pcount = 0;
        break;
    case DISPID_DBLCLICK:
        pcount = 0;
        break;
    default:
        {
            signal = mo->method(index);
            Q_ASSERT(signal.methodType() == QMetaMethod::Signal);
            type = signal.typeName();
            QByteArray signature(signal.signature());
            QByteArray name(signature);
            name.truncate(name.indexOf('('));

            eventId = signalCache.value(index, -1);
            if (eventId == -1) {
                ITypeInfo *eventInfo = 0;
                qAxTypeLibrary->GetTypeInfoOfGuid(qAxFactory()->eventsID(class_name), &eventInfo);
                if (eventInfo) {
                    QString uni_name = QLatin1String(name);
                    const OLECHAR *olename = reinterpret_cast<const OLECHAR *>(uni_name.utf16());
                    eventInfo->GetIDsOfNames((OLECHAR**)&olename, 1, &eventId);
                    eventInfo->Release();
                }
            }

            signature = signature.mid(name.length() + 1);
            signature.truncate(signature.length() - 1);

            if (!signature.isEmpty())
                ptypes = signature.split(',');

            pcount = ptypes.count();
        }
        break;
    }
    if (pcount && !argv) {
        qWarning("QAxServerBase::qt_metacall: Missing %d arguments", pcount);
        return false;
    }
    if (eventId == -1)
        return false;

    // For all connected event sinks...
    IConnectionPoint *cpoint = 0;
    GUID IID_QAxEvents = qAxFactory()->eventsID(class_name);
    FindConnectionPoint(IID_QAxEvents, &cpoint);
    if (cpoint) {
        IEnumConnections *clist = 0;
        cpoint->EnumConnections(&clist);
        if (clist) {
            clist->Reset();
            ULONG cc = 1;
            CONNECTDATA c[1];
            clist->Next(cc, (CONNECTDATA*)&c, &cc);
            if (cc) {
                // setup parameters
                unsigned int argErr = 0;
                DISPPARAMS dispParams;
                dispParams.cArgs = pcount;
                dispParams.cNamedArgs = 0;
                dispParams.rgdispidNamedArgs = 0;
                dispParams.rgvarg = 0;

                if (pcount) // Use malloc/free for eval package compatibility
                    dispParams.rgvarg = (VARIANTARG*)malloc(pcount * sizeof(VARIANTARG));
                int p = 0;
                for (p = 0; p < pcount; ++p) {
                    VARIANT *arg = dispParams.rgvarg + (pcount - p - 1);
                    VariantInit(arg);

                    bool out;
                    QByteArray ptype = paramType(ptypes.at(p), &out);
                    QVariant variant;
                    if (mo->indexOfEnumerator(ptype) != -1) {
                        // convert enum values to int
                        variant = QVariant(*reinterpret_cast<int *>(argv[p+1]));
                    } else {
                        QVariant::Type vt = QVariant::nameToType(ptype);
                        if (vt == QVariant::UserType) {
                            if (ptype.endsWith('*')) {
                                variant = QVariant(QMetaType::type(ptype), (void**)argv[p+1]);
                                // variant.setValue(*(void**)(argv[p + 1]), ptype);
                            } else {
                                variant = QVariant(QMetaType::type(ptype), argv[p+1]);
                                // variant.setValue(argv[p + 1], ptype);
                            }
                        } else {
                            variant = QVariant(vt, argv[p + 1]);
                        }
                    }

                    QVariantToVARIANT(variant, *arg, type, out);
                }

                VARIANT retval;
                VariantInit(&retval);
                VARIANT *pretval = 0;
                if (!type.isEmpty())
                    pretval = &retval;

                // call listeners (through IDispatch)
                while (cc) {
                    if (c->pUnk) {
                        IDispatch *disp = 0;
                        c->pUnk->QueryInterface(IID_QAxEvents, (void**)&disp);
                        if (disp) {
                            disp->Invoke(eventId, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_METHOD, &dispParams, pretval, 0, &argErr);

                            // update out-parameters and return value
                            if (index > 0) {
                                for (p = 0; p < pcount; ++p) {
                                    bool out;
                                    QByteArray ptype = paramType(ptypes.at(p), &out);
                                    if (out)
                                        QVariantToVoidStar(VARIANTToQVariant(dispParams.rgvarg[pcount - p - 1], ptype), argv[p+1], ptype);
                                }
                                if (pretval)
                                    QVariantToVoidStar(VARIANTToQVariant(retval, type), argv[0], type);
                            }
                            disp->Release();
                        }
                        c->pUnk->Release(); // AddRef'ed by clist->Next implementation
                    }
                    clist->Next(cc, (CONNECTDATA*)&c, &cc);
                }

                // clean up
                for (p = 0; p < pcount; ++p)
                    clearVARIANT(dispParams.rgvarg+p);
                free(dispParams.rgvarg);
            }
            clist->Release();
        }
        cpoint->Release();
    }

    return true;
}

/*!
    Call IPropertyNotifySink of connected clients.
    \a dispId specifies the ID of the property that changed.
*/
bool QAxServerBase::emitRequestPropertyChange(const char *property)
{
    long dispId = -1;

    IConnectionPoint *cpoint = 0;
    FindConnectionPoint(IID_IPropertyNotifySink, &cpoint);
    if (cpoint) {
	IEnumConnections *clist = 0;
	cpoint->EnumConnections(&clist);
	if (clist) {
	    clist->Reset();
	    ULONG cc = 1;
	    CONNECTDATA c[1];
	    clist->Next(cc, (CONNECTDATA*)&c, &cc);
	    if (cc) {
		if (dispId == -1) {
		    BSTR bstr = QStringToBSTR(QLatin1String(property));
		    GetIDsOfNames(IID_NULL, &bstr, 1, LOCALE_USER_DEFAULT, &dispId);
		    SysFreeString(bstr);
		}
		if (dispId != -1) while (cc) {
		    if (c->pUnk) {
			IPropertyNotifySink *sink = 0;
			c->pUnk->QueryInterface(IID_IPropertyNotifySink, (void**)&sink);
			bool disallows = sink && sink->OnRequestEdit(dispId) == S_FALSE;
			sink->Release();
			c->pUnk->Release();
			if (disallows) { // a client disallows the property to change
			    clist->Release();
			    cpoint->Release();
			    return false;
			}
		    }
		    clist->Next(cc, (CONNECTDATA*)&c, &cc);
		}
	    }
	    clist->Release();
	}
	cpoint->Release();
    }
    dirtyflag = true;
    return true;
}

/*!
    Call IPropertyNotifySink of connected clients.
    \a dispId specifies the ID of the property that changed.
*/
void QAxServerBase::emitPropertyChanged(const char *property)
{
    long dispId = -1;

    IConnectionPoint *cpoint = 0;
    FindConnectionPoint(IID_IPropertyNotifySink, &cpoint);
    if (cpoint) {
	IEnumConnections *clist = 0;
	cpoint->EnumConnections(&clist);
	if (clist) {
	    clist->Reset();
	    ULONG cc = 1;
	    CONNECTDATA c[1];
	    clist->Next(cc, (CONNECTDATA*)&c, &cc);
	    if (cc) {
		if (dispId == -1) {
		    BSTR bstr = QStringToBSTR(QLatin1String(property));
		    GetIDsOfNames(IID_NULL, &bstr, 1, LOCALE_USER_DEFAULT, &dispId);
		    SysFreeString(bstr);
		}
		if (dispId != -1) while (cc) {
		    if (c->pUnk) {
			IPropertyNotifySink *sink = 0;
			c->pUnk->QueryInterface(IID_IPropertyNotifySink, (void**)&sink);
			if (sink) {
			    sink->OnChanged(dispId);
			    sink->Release();
			}
			c->pUnk->Release();
		    }
		    clist->Next(cc, (CONNECTDATA*)&c, &cc);
		}
	    }
	    clist->Release();
	}
	cpoint->Release();
    }
    dirtyflag = true;
}

//**** IProvideClassInfo
/*
    Provide the ITypeInfo implementation for the COM class.
*/
HRESULT WINAPI QAxServerBase::GetClassInfo(ITypeInfo** pptinfo)
{
    if (!pptinfo)
	return E_POINTER;

    *pptinfo = 0;
    if (!qAxTypeLibrary)
	return DISP_E_BADINDEX;

    return qAxTypeLibrary->GetTypeInfoOfGuid(qAxFactory()->classID(class_name), pptinfo);
}

//**** IProvideClassInfo2
/*
    Provide the ID of the event interface.
*/
HRESULT WINAPI QAxServerBase::GetGUID(DWORD dwGuidKind, GUID* pGUID)
{
    if (!pGUID)
	return E_POINTER;

    if (dwGuidKind == GUIDKIND_DEFAULT_SOURCE_DISP_IID) {
	*pGUID = qAxFactory()->eventsID(class_name);
	return S_OK;
    }
    *pGUID = GUID_NULL;
    return E_FAIL;
}

//**** IDispatch
/*
    Returns the number of class infos for this IDispatch.
*/
HRESULT WINAPI QAxServerBase::GetTypeInfoCount(UINT* pctinfo)
{
    if (!pctinfo)
	return E_POINTER;

    *pctinfo = qAxTypeLibrary ? 1 : 0;
    return S_OK;
}

/*
    Provides the ITypeInfo for this IDispatch implementation.
*/
HRESULT WINAPI QAxServerBase::GetTypeInfo(UINT itinfo, LCID /*lcid*/, ITypeInfo** pptinfo)
{
    if (!pptinfo)
	return E_POINTER;

    if (!qAxTypeLibrary)
	return DISP_E_BADINDEX;

    ensureMetaData();

    *pptinfo = m_spTypeInfo;
    (*pptinfo)->AddRef();

    return S_OK;
}

/*
    Provides the names of the methods implemented in this IDispatch implementation.
*/
HRESULT WINAPI QAxServerBase::GetIDsOfNames(REFIID riid, LPOLESTR* rgszNames, UINT cNames,
				     LCID /*lcid*/, DISPID* rgdispid)
{
    if (!rgszNames || !rgdispid)
	return E_POINTER;

    if (!qAxTypeLibrary)
	return DISP_E_UNKNOWNNAME;

    ensureMetaData();
    if (!m_spTypeInfo)
	return DISP_E_UNKNOWNNAME;

    return m_spTypeInfo->GetIDsOfNames(rgszNames, cNames, rgdispid);
}

/*
    Map the COM call to the Qt slot/property for \a dispidMember.
*/
HRESULT WINAPI QAxServerBase::Invoke(DISPID dispidMember, REFIID riid,
		  LCID /*lcid*/, WORD wFlags, DISPPARAMS* pDispParams, VARIANT* pvarResult,
		  EXCEPINFO* pexcepinfo, UINT* puArgErr)
{
    if (riid != IID_NULL)
	return DISP_E_UNKNOWNINTERFACE;
    if (!theObject)
	return E_UNEXPECTED;

    HRESULT res = DISP_E_MEMBERNOTFOUND;

    bool uniqueIndex = wFlags == DISPATCH_PROPERTYGET || wFlags == DISPATCH_PROPERTYPUT || wFlags == DISPATCH_METHOD;

    int index = uniqueIndex ? indexCache.value(dispidMember, -1) : -1;
    QByteArray name;
    if (index == -1) {
	ensureMetaData();

        // This property or method is invoked when an ActiveX client specifies
        // the object name without a property or method. We only support property.
        if (dispidMember == DISPID_VALUE && (wFlags == DISPATCH_PROPERTYGET || wFlags == DISPATCH_PROPERTYPUT)) {
            const QMetaObject *mo = qt.object->metaObject();
            index = mo->indexOfClassInfo("DefaultProperty");
            if (index != -1) {
                name  = mo->classInfo(index).value();
                index = mo->indexOfProperty(name);
            }
        } else {
	    BSTR bname;
	    UINT cname = 0;
	    if (m_spTypeInfo)
	        m_spTypeInfo->GetNames(dispidMember, &bname, 1, &cname);
	    if (!cname)
	        return res;

            name = QString::fromWCharArray(bname).toLatin1();
	    SysFreeString(bname);
        }
    }

    const QMetaObject *mo = qt.object->metaObject();
    QSize oldSizeHint;
    if (isWidget)
	oldSizeHint = qt.widget->sizeHint();

    switch (wFlags) {
    case DISPATCH_PROPERTYGET|DISPATCH_METHOD:
    case DISPATCH_PROPERTYGET:
	{
	    if (index == -1) {
		index = mo->indexOfProperty(name);
		if (index == -1 && wFlags == DISPATCH_PROPERTYGET)
		    return res;
	    }

	    QMetaProperty property;
            if (index < mo->propertyCount())
                property = mo->property(index);

	    if (property.isReadable()) {
		if (!pvarResult)
		    return DISP_E_PARAMNOTOPTIONAL;
		if (pDispParams->cArgs ||
		     pDispParams->cNamedArgs)
		    return DISP_E_BADPARAMCOUNT;

		QVariant var = qt.object->property(property.name());
		if (!var.isValid())
		    res =  DISP_E_MEMBERNOTFOUND;
		else if (!QVariantToVARIANT(var, *pvarResult))
		    res = DISP_E_TYPEMISMATCH;
		else
		    res = S_OK;
		break;
	    } else if (wFlags == DISPATCH_PROPERTYGET) {
		break;
	    }
	}
	// FALLTHROUGH if wFlags == DISPATCH_PROPERTYGET|DISPATCH_METHOD AND not a property.
    case DISPATCH_METHOD:
	{
            int nameLength = 0;
	    if (index == -1) {
	        nameLength = name.length();
	        name += '(';
		// no parameter - shortcut
		if (!pDispParams->cArgs)
		    index = mo->indexOfSlot((name + ')'));
		// search
		if (index == -1) {
		    for (int i = 0; i < mo->methodCount(); ++i) {
                        const QMetaMethod slot(mo->method(i));
                        if (slot.methodType() == QMetaMethod::Slot && QByteArray(slot.signature()).startsWith(name)) {
			    index = i;
			    break;
			}
		    }
                    // resolve overloads
                    if (index == -1) {
                        QRegExp regexp(QLatin1String("_([0-9])\\("));
                        if (regexp.lastIndexIn(QString::fromLatin1(name.constData())) != -1) {
                            name = name.left(name.length() - regexp.cap(0).length()) + '(';
                            int overload = regexp.cap(1).toInt() + 1;

                            for (int s = 0; s < qt.object->metaObject()->methodCount(); ++s) {
                                QMetaMethod slot = qt.object->metaObject()->method(s);
                                if (slot.methodType() == QMetaMethod::Slot && QByteArray(slot.signature()).startsWith(name)) {
                                    if (!--overload) {
                                        index = s;
                                        break;
                                    }
                                }
                            }
                        }
                    }
		    if (index == -1)
			return res;
		}
	    }

            int lookupIndex = index;

	    // get slot info
	    QMetaMethod slot(mo->method(index));
            Q_ASSERT(slot.methodType() == QMetaMethod::Slot);
	    QByteArray type = slot.typeName();
	    name = slot.signature();
            nameLength = name.indexOf('(');
	    QByteArray prototype = name.mid(nameLength + 1);
	    prototype.truncate(prototype.length() - 1);
	    QList<QByteArray> ptypes;
	    if (!prototype.isEmpty())
		ptypes = prototype.split(',');
	    int pcount = ptypes.count();

	    // verify parameter count
            if (pcount > pDispParams->cArgs) {
                // count cloned slots immediately following the real thing
                int defArgs = 0;
                while (index < mo->methodCount()) {
                    ++index;
                    slot = mo->method(index);
                    if (!(slot.attributes() & QMetaMethod::Cloned))
                        break;
                    --pcount;
                    // found a matching overload. ptypes still valid
                    if (pcount <= pDispParams->cArgs)
                        break;
                }
                // still wrong :(
                if (pcount > pDispParams->cArgs)
		    return DISP_E_PARAMNOTOPTIONAL;
            } else if (pcount < pDispParams->cArgs) {
		return DISP_E_BADPARAMCOUNT;
            }

	    // setup parameters (pcount + return)
	    bool ok = true;
            void *static_argv[QAX_NUM_PARAMS + 1];
            QVariant static_varp[QAX_NUM_PARAMS + 1];
            void *static_argv_pointer[QAX_NUM_PARAMS + 1];

            int totalParam = pcount;
            if (!type.isEmpty())
                ++totalParam;

	    void **argv = 0; // the actual array passed into qt_metacall
            void **argv_pointer = 0; // in case we need an additional level of indirection
	    QVariant *varp = 0; // QVariants to hold the temporary Qt data object for us

            if (totalParam) {
                if (totalParam <= QAX_NUM_PARAMS) {
                    argv = static_argv;
                    argv_pointer = static_argv_pointer;
                    varp = static_varp;
                } else {
                    argv = new void*[pcount + 1];
                    argv_pointer = new void*[pcount + 1];
                    varp = new QVariant[pcount + 1];
                }

                argv_pointer[0] = 0;
            }

	    for (int p = 0; p < pcount; ++p) {
		// map the VARIANT to the void*
		bool out;
		QByteArray ptype = paramType(ptypes.at(p), &out);
		varp[p + 1] = VARIANTToQVariant(pDispParams->rgvarg[pcount - p - 1], ptype);
                argv_pointer[p + 1] = 0;
		if (varp[p + 1].isValid()) {
                    if (varp[p + 1].type() == QVariant::UserType) {
                        argv[p + 1] = varp[p + 1].data();
                    } else if (ptype == "QVariant") {
                        argv[p + 1] = varp + p + 1;
                    } else {
                        argv[p + 1] = const_cast<void*>(varp[p + 1].constData());
                        if (ptype.endsWith('*')) {
                            argv_pointer[p + 1] = argv[p + 1];
                            argv[p + 1] = argv_pointer + p + 1;
                        }
                    }
                } else if (ptype == "QVariant") {
                    argv[p + 1] = varp + p + 1;
		} else {
		    if (puArgErr)
			*puArgErr = pcount-p-1;
		    ok = false;
		}
	    }

            // return value
	    if (!type.isEmpty()) {
                QVariant::Type vt = QVariant::nameToType(type);
                if (vt == QVariant::UserType)
                    vt = QVariant::Invalid;
                varp[0] = QVariant(vt);
                if (varp[0].type() == QVariant::Invalid && mo->indexOfEnumerator(slot.typeName()) != -1)
                    varp[0] = QVariant(QVariant::Int);

                if (varp[0].type() == QVariant::Invalid) {
                    if (type == "QVariant")
                        argv[0] = varp;
                    else
                        argv[0] = 0;
                } else {
                    argv[0] = const_cast<void*>(varp[0].constData());
                }
                if (type.endsWith('*')) {
                    argv_pointer[0] = argv[0];
                    argv[0] = argv_pointer;
                }
	    }

	    // call the slot if everthing went fine.
	    if (ok) {
            ++invokeCount;
            qt.object->qt_metacall(QMetaObject::InvokeMetaMethod, index, argv);
            if (--invokeCount < 0)
                invokeCount = 0;

		// update reference parameters and return value
		for (int p = 0; p < pcount; ++p) {
		    bool out;
		    QByteArray ptype = paramType(ptypes.at(p), &out);
		    if (out) {
			if (!QVariantToVARIANT(varp[p + 1], pDispParams->rgvarg[pcount - p - 1], ptype, out))
			    ok = false;
		    }
		}
                if (!type.isEmpty() && pvarResult) {
                    if (!varp[0].isValid() && type != "QVariant")
                        varp[0] = QVariant(QMetaType::type(type), argv_pointer);
//                        varp[0].setValue(argv_pointer[0], type);
		    ok = QVariantToVARIANT(varp[0], *pvarResult, type);
                }
	    }
            if (argv && argv != static_argv) {
                delete []argv;
                delete []argv_pointer;
                delete []varp;
            }

	    res = ok ? S_OK : DISP_E_TYPEMISMATCH;

            // reset in case index changed for default-arg handling
            index = lookupIndex;
	}
	break;
    case DISPATCH_PROPERTYPUT:
    case DISPATCH_PROPERTYPUT|DISPATCH_PROPERTYPUTREF:
	{
            if (index == -1) {
                index = mo->indexOfProperty(name);
                if (index == -1)
                    return res;
            }

            QMetaProperty property;
            if (index < mo->propertyCount())
                property = mo->property(index);
            if (!property.isWritable())
                return DISP_E_MEMBERNOTFOUND;
            if (!pDispParams->cArgs)
                return DISP_E_PARAMNOTOPTIONAL;
            if (pDispParams->cArgs != 1 ||
                pDispParams->cNamedArgs != 1 ||
                *pDispParams->rgdispidNamedArgs != DISPID_PROPERTYPUT)
                return DISP_E_BADPARAMCOUNT;

            QVariant var = VARIANTToQVariant(*pDispParams->rgvarg, property.typeName(), property.type());
            if (!var.isValid()) {
                if (puArgErr)
                    *puArgErr = 0;
                return DISP_E_BADVARTYPE;
            }
            if (!qt.object->setProperty(property.name(), var)) {
                if (puArgErr)
                    *puArgErr = 0;
                return DISP_E_TYPEMISMATCH;
            }

            res = S_OK;
	}
	break;

    default:
	break;
    }

    // maybe calling a setter? Notify client about changes
    switch(wFlags) {
    case DISPATCH_METHOD:
    case DISPATCH_PROPERTYPUT:
    case DISPATCH_PROPERTYPUT|DISPATCH_PROPERTYPUTREF:
        if (m_spAdviseSink || adviseSinks.count()) {
            FORMATETC fmt;
            fmt.cfFormat = 0;
            fmt.ptd = 0;
            fmt.dwAspect = DVASPECT_CONTENT;
            fmt.lindex = -1;
            fmt.tymed = TYMED_NULL;

            STGMEDIUM stg;
            stg.tymed = TYMED_NULL;
            stg.pUnkForRelease = 0;
            stg.hBitmap = 0; // initializes the whole union

            if (m_spAdviseSink) {
                m_spAdviseSink->OnViewChange(DVASPECT_CONTENT, -1);
                m_spAdviseSink->OnDataChange(&fmt, &stg);
            }
            for (int i = 0; i < adviseSinks.count(); ++i) {
                adviseSinks.at(i).pAdvSink->OnDataChange(&fmt, &stg);
            }
        }

        dirtyflag = true;
        break;
    default:
        break;
    }

    if (index != -1 && uniqueIndex)
	indexCache.insert(dispidMember, index);

    if (exception) {
	if (pexcepinfo) {
	    memset(pexcepinfo, 0, sizeof(EXCEPINFO));

	    pexcepinfo->wCode = exception->code;
	    if (!exception->src.isNull())
		pexcepinfo->bstrSource = QStringToBSTR(exception->src);
	    if (!exception->desc.isNull())
		pexcepinfo->bstrDescription = QStringToBSTR(exception->desc);
	    if (!exception->context.isNull()) {
		QString context = exception->context;
		int contextID = 0;
		int br = context.indexOf(QLatin1Char('['));
		if (br != -1) {
		    context = context.mid(br+1);
		    context = context.left(context.length() - 1);
		    contextID = context.toInt();

		    context = exception->context;
		    context = context.left(br-1);
		}
		pexcepinfo->bstrHelpFile = QStringToBSTR(context);
		pexcepinfo->dwHelpContext = contextID;
	    }
	}
	delete exception;
	exception = 0;
	return DISP_E_EXCEPTION;
    } else if (isWidget) {
	QSize sizeHint = qt.widget->sizeHint();
	if (oldSizeHint != sizeHint) {
	    updateGeometry();
	    if (m_spInPlaceSite) {
                RECT rect = {0, 0, sizeHint.width(), sizeHint.height()};
		m_spInPlaceSite->OnPosRectChange(&rect);
	    }
	}
	updateMask();
    }

    return res;
}

//**** IConnectionPointContainer
/*
    Provide the IEnumConnectionPoints implemented in the QAxSignalVec class.
*/
HRESULT WINAPI QAxServerBase::EnumConnectionPoints(IEnumConnectionPoints **epoints)
{
    if (!epoints)
	return E_POINTER;
    *epoints = new QAxSignalVec(points);
    (*epoints)->AddRef();
    return S_OK;
}

/*
    Provide the IConnectionPoint implemented in the QAxConnection for \a iid.
*/
HRESULT WINAPI QAxServerBase::FindConnectionPoint(REFIID iid, IConnectionPoint **cpoint)
{
    if (!cpoint)
	return E_POINTER;

    IConnectionPoint *cp = points[iid];
    *cpoint = cp;
    if (cp) {
	cp->AddRef();
	return S_OK;
    }
    return CONNECT_E_NOCONNECTION;
}

//**** IPersistStream
/*
    \reimp

    See documentation of IPersistStorage::IsDirty.
*/
HRESULT WINAPI QAxServerBase::IsDirty()
{
    return dirtyflag ? S_OK : S_FALSE;
}

HRESULT WINAPI QAxServerBase::Load(IStream *pStm)
{
    STATSTG stat;
    HRESULT hres = pStm->Stat(&stat, STATFLAG_DEFAULT);
    bool openAsText = false;
    QByteArray qtarray;
    if (hres == S_OK) {
        QString streamName = QString::fromWCharArray(stat.pwcsName);
        CoTaskMemFree(stat.pwcsName);
        openAsText = streamName == QLatin1String("SomeStreamName");
	if (stat.cbSize.HighPart) // more than 4GB - too large!
	    return S_FALSE;

	qtarray.resize(stat.cbSize.LowPart);
        ULONG read;
	pStm->Read(qtarray.data(), stat.cbSize.LowPart, &read);
    } else if (hres == E_NOTIMPL) {
        ULONG read = 0;
        while (hres != S_FALSE) {
            QByteArray arrayRead;
            arrayRead.resize(4098);
            hres = pStm->Read(arrayRead.data(), arrayRead.size(), &read);
            if (hres != S_OK && hres != S_FALSE) {
                qtarray.resize(0);
                break;
            } else if (read == 0)
                break;
            qtarray.append(arrayRead);
        }
    }
    const QMetaObject *mo = qt.object->metaObject();

    QBuffer qtbuffer(&qtarray);
    QByteArray mimeType = mo->classInfo(mo->indexOfClassInfo("MIME")).value();
    if (!mimeType.isEmpty()) {
        mimeType = mimeType.left(mimeType.indexOf(':')); // first type
        QAxBindable *axb = (QAxBindable*)qt.object->qt_metacast("QAxBindable");
        if (axb && axb->readData(&qtbuffer, QString::fromLatin1(mimeType)))
            return S_OK;
    }

    qtbuffer.close(); // resets
    qtbuffer.open(openAsText ? (QIODevice::ReadOnly | QIODevice::Text) : QIODevice::ReadOnly);

    QDataStream qtstream(&qtbuffer);
    int version;
    qtstream >> version;
    qtstream.setVersion(version);
    int more = 0;
    qtstream >> more;

    while (!qtbuffer.atEnd() && more) {
	QString propname;
	QVariant value;
	qtstream >> propname;
	if (propname.isEmpty())
	    break;
	qtstream >> value;
	qtstream >> more;

	int idx = mo->indexOfProperty(propname.toLatin1());
	QMetaProperty property = mo->property(idx);
	if (property.isWritable())
	    qt.object->setProperty(propname.toLatin1(), value);
    }
    return S_OK;
}

HRESULT WINAPI QAxServerBase::Save(IStream *pStm, BOOL clearDirty)
{
    const QMetaObject *mo = qt.object->metaObject();

    QBuffer qtbuffer;
    bool saved = false;
    QByteArray mimeType = mo->classInfo(mo->indexOfClassInfo("MIME")).value();
    if (!mimeType.isEmpty()) {
        QAxBindable *axb = (QAxBindable*)qt.object->qt_metacast("QAxBindable");
        saved = axb && axb->writeData(&qtbuffer);
        qtbuffer.close();
    }

    if (!saved) {
        qtbuffer.open(QIODevice::WriteOnly);
        QDataStream qtstream(&qtbuffer);
        qtstream << qtstream.version();

        for (int prop = 0; prop < mo->propertyCount(); ++prop) {
	    if (!isPropertyExposed(prop))
	        continue;
	    QMetaProperty metaprop = mo->property(prop);
            if (QByteArray(metaprop.typeName()).endsWith('*'))
                continue;
	    QString property = QLatin1String(metaprop.name());
	    QVariant qvar = qt.object->property(metaprop.name());
	    if (qvar.isValid()) {
	        qtstream << int(1);
	        qtstream << property;
	        qtstream << qvar;
	    }
        }

        qtstream << int(0);
        qtbuffer.close();
    }

    QByteArray qtarray = qtbuffer.buffer();
    ULONG written = 0;
    const char *data = qtarray.constData();
    ULARGE_INTEGER newsize;
    newsize.HighPart = 0;
    newsize.LowPart = qtarray.size();
    pStm->SetSize(newsize);
    pStm->Write(data, qtarray.size(), &written);
    pStm->Commit(STGC_ONLYIFCURRENT);

    if (clearDirty)
        dirtyflag = false;
    return S_OK;
}

HRESULT WINAPI QAxServerBase::GetSizeMax(ULARGE_INTEGER *pcbSize)
{
    const QMetaObject *mo = qt.object->metaObject();

    int np = mo->propertyCount();
    pcbSize->HighPart = 0;
    pcbSize->LowPart = np * 50;

    return S_OK;
}

//**** IPersistStorage

HRESULT WINAPI QAxServerBase::InitNew(IStorage *pStg)
{
    if (initNewCalled)
	return CO_E_ALREADYINITIALIZED;

    dirtyflag = false;
    initNewCalled = true;

    m_spStorage = pStg;
    if (m_spStorage)
	m_spStorage->AddRef();
    return S_OK;
}

HRESULT WINAPI QAxServerBase::Load(IStorage *pStg)
{
    if (InitNew(pStg) != S_OK)
	return CO_E_ALREADYINITIALIZED;

    IStream *spStream = 0;
    QString streamName = QLatin1String(qt.object->metaObject()->className());
    streamName.replace(QLatin1Char(':'), QLatin1Char('.'));
    /* Also invalid, but not relevant
    streamName.replace(QLatin1Char('/'), QLatin1Char('_'));
    streamName.replace(QLatin1Char('\\'), QLatin1Char('_'));
    */
    streamName += QLatin1String("_Stream4.2");

    pStg->OpenStream((const wchar_t *)streamName.utf16(), 0, STGM_READ | STGM_SHARE_EXCLUSIVE, 0, &spStream);
    if (!spStream) // support for streams saved with 4.1 and earlier
        pStg->OpenStream(L"SomeStreamName", 0, STGM_READ | STGM_SHARE_EXCLUSIVE, 0, &spStream);
    if (!spStream)
	return E_FAIL;

    Load(spStream);
    spStream->Release();

    return S_OK;
}

HRESULT WINAPI QAxServerBase::Save(IStorage *pStg, BOOL fSameAsLoad)
{
    IStream *spStream = 0;
    QString streamName = QLatin1String(qt.object->metaObject()->className());
    streamName.replace(QLatin1Char(':'), QLatin1Char('.'));
    /* Also invalid, but not relevant
    streamName.replace(QLatin1Char('/'), QLatin1Char('_'));
    streamName.replace(QLatin1Char('\\'), QLatin1Char('_'));
    */
    streamName += QLatin1String("_Stream4.2");

    pStg->CreateStream((const wchar_t *)streamName.utf16(), STGM_CREATE | STGM_WRITE | STGM_SHARE_EXCLUSIVE, 0, 0, &spStream);
    if (!spStream)
	return E_FAIL;

    Save(spStream, true);

    spStream->Release();
    return S_OK;
}

HRESULT WINAPI QAxServerBase::SaveCompleted(IStorage *pStgNew)
{
    if (pStgNew) {
	if (m_spStorage)
	    m_spStorage->Release();
	m_spStorage = pStgNew;
	m_spStorage->AddRef();
    }
    return S_OK;
}

HRESULT WINAPI QAxServerBase::HandsOffStorage()
{
    if (m_spStorage) m_spStorage->Release();
    m_spStorage = 0;

    return S_OK;
}

//**** IPersistPropertyBag
/*
    Initialize the properties of the Qt widget.
*/
HRESULT WINAPI QAxServerBase::InitNew()
{
    if (initNewCalled)
	return CO_E_ALREADYINITIALIZED;

    dirtyflag = false;
    initNewCalled = true;
    return S_OK;
}

/*
    Set the properties of the Qt widget to the values provided in the \a bag.
*/
HRESULT WINAPI QAxServerBase::Load(IPropertyBag *bag, IErrorLog * /*log*/)
{
    if (!bag)
	return E_POINTER;

    if (InitNew() != S_OK)
	return E_UNEXPECTED;

    bool error = false;
    const QMetaObject *mo = qt.object->metaObject();
    for (int prop = 0; prop < mo->propertyCount(); ++prop) {
	if (!isPropertyExposed(prop))
	    continue;
	QMetaProperty property = mo->property(prop);
	const char* pname = property.name();
	BSTR bstr = QStringToBSTR(QLatin1String(pname));
	VARIANT var;
	var.vt = VT_EMPTY;
	HRESULT res = bag->Read(bstr, &var, 0);
	if (property.isWritable() && var.vt != VT_EMPTY) {
	    if (res != S_OK || !qt.object->setProperty(pname, VARIANTToQVariant(var, property.typeName(), property.type())))
		error = true;
	}
	SysFreeString(bstr);
    }

    updateGeometry();

    return /*error ? E_FAIL :*/ S_OK;
}

/*
    Save the properties of the Qt widget into the \a bag.
*/
HRESULT WINAPI QAxServerBase::Save(IPropertyBag *bag, BOOL clearDirty, BOOL /*saveAll*/)
{
    if (!bag)
	return E_POINTER;

    if (clearDirty)
	dirtyflag = false;
    bool error = false;
    const QMetaObject *mo = qt.object->metaObject();
    for (int prop = 0; prop < mo->propertyCount(); ++prop) {
	if (!isPropertyExposed(prop))
	    continue;
	QMetaProperty property = mo->property(prop);
        if (QByteArray(property.typeName()).endsWith('*'))
            continue;

	BSTR bstr = QStringToBSTR(QLatin1String(property.name()));
	QVariant qvar = qt.object->property(property.name());
	if (!qvar.isValid())
	    error = true;
	VARIANT var;
	QVariantToVARIANT(qvar, var);
	bag->Write(bstr, &var);
	SysFreeString(bstr);
    }
    return /*error ? E_FAIL :*/ S_OK;
}

//**** IPersistFile
/*
*/
HRESULT WINAPI QAxServerBase::SaveCompleted(LPCOLESTR fileName)
{
    if (qt.object->metaObject()->indexOfClassInfo("MIME") == -1)
        return E_NOTIMPL;

    currentFileName = QString::fromWCharArray(fileName);
    return S_OK;
}

HRESULT WINAPI QAxServerBase::GetCurFile(LPOLESTR *currentFile)
{
    if (qt.object->metaObject()->indexOfClassInfo("MIME") == -1)
        return E_NOTIMPL;

    if (currentFileName.isEmpty()) {
        *currentFile = 0;
        return S_FALSE;
    }
    IMalloc *malloc = 0;
    CoGetMalloc(1, &malloc);
    if (!malloc)
        return E_OUTOFMEMORY;

    *currentFile = static_cast<wchar_t *>(malloc->Alloc(currentFileName.length() * 2));
    malloc->Release();
    memcpy(*currentFile, currentFileName.unicode(), currentFileName.length() * 2);

    return S_OK;
}

HRESULT WINAPI QAxServerBase::Load(LPCOLESTR fileName, DWORD mode)
{
    const QMetaObject *mo = qt.object->metaObject();
    int mimeIndex = mo->indexOfClassInfo("MIME");
    if (mimeIndex == -1)
        return E_NOTIMPL;

    QAxBindable *axb = (QAxBindable*)qt.object->qt_metacast("QAxBindable");
    if (!axb) {
        qWarning() << class_name << ": No QAxBindable implementation for mime-type handling";
        return E_NOTIMPL;
    }

    QString loadFileName = QString::fromWCharArray(fileName);
    QString fileExtension = loadFileName.mid(loadFileName.lastIndexOf(QLatin1Char('.')) + 1);
    QFile file(loadFileName);

    QString mimeType = QLatin1String(mo->classInfo(mimeIndex).value());
    QStringList mimeTypes = mimeType.split(QLatin1Char(';'));
    for (int m = 0; m < mimeTypes.count(); ++m) {
        QString mime = mimeTypes.at(m);
        if (mime.count(QLatin1Char(':')) != 2) {
            qWarning() << class_name << ": Invalid syntax in Q_CLASSINFO for MIME";
            continue;
        }

        mimeType = mime.left(mimeType.indexOf(QLatin1Char(':'))); // first type
        if (mimeType.isEmpty()) {
            qWarning() << class_name << ": Invalid syntax in Q_CLASSINFO for MIME";
            continue;
        }
        QString mimeExtension = mime.mid(mimeType.length() + 1);
        mimeExtension = mimeExtension.left(mimeExtension.indexOf(QLatin1Char(':')));
        if (mimeExtension != fileExtension)
            continue;

        if (axb->readData(&file, mimeType)) {
            currentFileName = loadFileName;
            return S_OK;
        }
    }

    return E_FAIL;
}

HRESULT WINAPI QAxServerBase::Save(LPCOLESTR fileName, BOOL fRemember)
{
    const QMetaObject *mo = qt.object->metaObject();
    int mimeIndex = mo->indexOfClassInfo("MIME");
    if (mimeIndex == -1)
        return E_NOTIMPL;

    QAxBindable *axb = (QAxBindable*)qt.object->qt_metacast("QAxBindable");
    if (!axb) {
        qWarning() << class_name << ": No QAxBindable implementation for mime-type handling";
        return E_NOTIMPL;
    }

    QString saveFileName = QString::fromWCharArray(fileName);
    QString fileExtension = saveFileName.mid(saveFileName.lastIndexOf(QLatin1Char('.')) + 1);
    QFile file(saveFileName);

    QString mimeType = QLatin1String(mo->classInfo(mimeIndex).value());
    QStringList mimeTypes = mimeType.split(QLatin1Char(';'));
    for (int m = 0; m < mimeTypes.count(); ++m) {
        QString mime = mimeTypes.at(m);
        if (mime.count(QLatin1Char(':')) != 2) {
            qWarning() << class_name << ": Invalid syntax in Q_CLASSINFO for MIME";
            continue;
        }
        mimeType = mime.left(mimeType.indexOf(QLatin1Char(':'))); // first type
        if (mimeType.isEmpty()) {
            qWarning() << class_name << ": Invalid syntax in Q_CLASSINFO for MIME";
            continue;
        }
        QString mimeExtension = mime.mid(mimeType.length() + 1);
        mimeExtension = mimeExtension.left(mimeExtension.indexOf(QLatin1Char(':')));
        if (mimeExtension != fileExtension)
            continue;
        if (axb->writeData(&file)) {
            if (fRemember)
                currentFileName = saveFileName;
            return S_OK;
        }
    }
    return E_FAIL;
}

//**** IViewObject
/*
    Draws the widget into the provided device context.
*/
HRESULT WINAPI QAxServerBase::Draw(DWORD dwAspect, LONG lindex, void *pvAspect, DVTARGETDEVICE *ptd,
		HDC hicTargetDev, HDC hdcDraw, LPCRECTL lprcBounds, LPCRECTL /*lprcWBounds*/,
		BOOL(__stdcall* /*pfnContinue*/)(ULONG_PTR), ULONG_PTR /*dwContinue*/)
{
    if (!lprcBounds)
	return E_INVALIDARG;

    internalCreate();
    if (!isWidget || !qt.widget)
	return OLE_E_BLANK;

    switch (dwAspect) {
    case DVASPECT_CONTENT:
    case DVASPECT_OPAQUE:
    case DVASPECT_TRANSPARENT:
	break;
    default:
	return DV_E_DVASPECT;
    }
    if (!ptd)
	hicTargetDev = 0;

    bool bDeleteDC = false;
    if (!hicTargetDev) {
	hicTargetDev = ::CreateDC(L"DISPLAY", NULL, NULL, NULL);
	bDeleteDC = (hicTargetDev != hdcDraw);
    }

    RECTL rc = *lprcBounds;
    bool bMetaFile = GetDeviceCaps(hdcDraw, TECHNOLOGY) == DT_METAFILE;
    if (!bMetaFile)
        ::LPtoDP(hicTargetDev, (LPPOINT)&rc, 2);

    QPixmap pm = QPixmap::grabWidget(qt.widget);
    HBITMAP hbm = pm.toWinHBITMAP();
    HDC hdc = CreateCompatibleDC(0);
    SelectObject(hdc, hbm);
    ::StretchBlt(hdcDraw, rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, hdc, 0, 0,pm.width(), pm.height(), SRCCOPY);
    DeleteDC(hdc);
    DeleteObject(hbm);

    if (bDeleteDC)
	DeleteDC(hicTargetDev);

    return S_OK;
}

/*
    Not implemented.
*/
HRESULT WINAPI QAxServerBase::GetColorSet(DWORD dwDrawAspect, LONG lindex, void *pvAspect, DVTARGETDEVICE *ptd,
		HDC hicTargetDev, LOGPALETTE **ppColorSet)
{
    return E_NOTIMPL;
}

/*
    Not implemented.
*/
HRESULT WINAPI QAxServerBase::Freeze(DWORD dwAspect, LONG lindex, void *pvAspect, DWORD *pdwFreeze)
{
    return E_NOTIMPL;
}

/*
    Not implemented.
*/
HRESULT WINAPI QAxServerBase::Unfreeze(DWORD dwFreeze)
{
    return E_NOTIMPL;
}

/*
    Stores the provided advise sink.
*/
HRESULT WINAPI QAxServerBase::SetAdvise(DWORD /*aspects*/, DWORD /*advf*/, IAdviseSink *pAdvSink)
{
    if (m_spAdviseSink) m_spAdviseSink->Release();

    m_spAdviseSink = pAdvSink;
    if (m_spAdviseSink) m_spAdviseSink->AddRef();
    return S_OK;
}

/*
    Returns the advise sink.
*/
HRESULT WINAPI QAxServerBase::GetAdvise(DWORD* /*aspects*/, DWORD* /*advf*/, IAdviseSink **ppAdvSink)
{
    if (!ppAdvSink)
	return E_POINTER;

    *ppAdvSink = m_spAdviseSink;
    if (*ppAdvSink)
	(*ppAdvSink)->AddRef();
    return S_OK;
}

//**** IViewObject2
/*
    Returns the current size ONLY if the widget has already been sized.
*/
HRESULT WINAPI QAxServerBase::GetExtent(DWORD dwAspect, LONG /*lindex*/, DVTARGETDEVICE* /*ptd*/, LPSIZEL lpsizel)
{
    if (!isWidget || !qt.widget || !qt.widget->testAttribute(Qt::WA_Resized))
        return OLE_E_BLANK;

    return GetExtent(dwAspect, lpsizel);
}

//**** IOleControl
/*
    Not implemented.
*/
HRESULT WINAPI QAxServerBase::GetControlInfo(LPCONTROLINFO)
{
    return E_NOTIMPL;
}

/*
    Turns event firing on and off.
*/
HRESULT WINAPI QAxServerBase::FreezeEvents(BOOL bFreeze)
{
    // member of CComControl
    if (bFreeze)
	freezeEvents++;
    else
	freezeEvents--;

    return S_OK;
}

/*
    Not implemented.
*/
HRESULT WINAPI QAxServerBase::OnMnemonic(LPMSG)
{
    return E_NOTIMPL;
}

/*
    Update the ambient properties of the Qt widget.
*/
HRESULT WINAPI QAxServerBase::OnAmbientPropertyChange(DISPID dispID)
{
    if (!m_spClientSite || !theObject)
	return S_OK;

    IDispatch *disp = 0;
    m_spClientSite->QueryInterface(IID_IDispatch, (void**)&disp);
    if (!disp)
	return S_OK;

    VARIANT var;
    VariantInit(&var);
    DISPPARAMS params = { 0, 0, 0, 0 };
    disp->Invoke(dispID, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET, &params, &var, 0, 0);
    disp->Release();
    disp = 0;

    switch(dispID) {
    case DISPID_AMBIENT_APPEARANCE:
	break;
    case DISPID_AMBIENT_AUTOCLIP:
	break;
    case DISPID_AMBIENT_BACKCOLOR:
    case DISPID_AMBIENT_FORECOLOR:
	if (isWidget) {
	    long rgb;
	    if (var.vt == VT_UI4)
		rgb = var.ulVal;
	    else if (var.vt == VT_I4)
		rgb = var.lVal;
	    else
		break;
	    QPalette pal = qt.widget->palette();
	    pal.setColor(dispID == DISPID_AMBIENT_BACKCOLOR ? QPalette::Window : QPalette::WindowText,
		OLEColorToQColor(rgb));
	    qt.widget->setPalette(pal);
	}
	break;
    case DISPID_AMBIENT_DISPLAYASDEFAULT:
	break;
    case DISPID_AMBIENT_DISPLAYNAME:
	if (var.vt != VT_BSTR || !isWidget)
	    break;
	qt.widget->setWindowTitle(QString::fromWCharArray(var.bstrVal));
	break;
    case DISPID_AMBIENT_FONT:
	if (var.vt != VT_DISPATCH || !isWidget)
	    break;
	{
            QVariant qvar = VARIANTToQVariant(var, "QFont", QVariant::Font);
            QFont qfont = qvariant_cast<QFont>(qvar);
            qt.widget->setFont(qfont);
	}
	break;
    case DISPID_AMBIENT_LOCALEID:
	break;
    case DISPID_AMBIENT_MESSAGEREFLECT:
	if (var.vt != VT_BOOL)
	    break;
	if (var.boolVal)
	    qt.widget->installEventFilter(this);
	else
	    qt.widget->removeEventFilter(this);
	break;
    case DISPID_AMBIENT_PALETTE:
	break;
    case DISPID_AMBIENT_SCALEUNITS:
	break;
    case DISPID_AMBIENT_SHOWGRABHANDLES:
	break;
    case DISPID_AMBIENT_SHOWHATCHING:
	break;
    case DISPID_AMBIENT_SUPPORTSMNEMONICS:
	break;
    case DISPID_AMBIENT_TEXTALIGN:
	break;
    case DISPID_AMBIENT_UIDEAD:
	if (var.vt != VT_BOOL || !isWidget)
	    break;
	qt.widget->setEnabled(!var.boolVal);
	break;
    case DISPID_AMBIENT_USERMODE:
	if (var.vt != VT_BOOL)
	    break;
	inDesignMode = !var.boolVal;
	break;
    case DISPID_AMBIENT_RIGHTTOLEFT:
	if (var.vt != VT_BOOL)
	    break;
	qApp->setLayoutDirection(var.boolVal?Qt::RightToLeft:Qt::LeftToRight);
	break;
    }

    return S_OK;
}

//**** IOleWindow
/*
    Returns the HWND of the control.
*/
HRESULT WINAPI QAxServerBase::GetWindow(HWND *pHwnd)
{
    if (!pHwnd)
	return E_POINTER;
    *pHwnd = m_hWnd;
    return S_OK;
}

/*
    Enters What's This mode.
*/
HRESULT WINAPI QAxServerBase::ContextSensitiveHelp(BOOL fEnterMode)
{
    if (fEnterMode)
	QWhatsThis::enterWhatsThisMode();
    else
	QWhatsThis::leaveWhatsThisMode();
    return S_OK;
}

//**** IOleInPlaceObject
/*
    Deactivates the control in place.
*/
HRESULT WINAPI QAxServerBase::InPlaceDeactivate()
{
    if (!isInPlaceActive)
	return S_OK;
    UIDeactivate();

    isInPlaceActive = false;

    // if we have a window, tell it to go away.
    if (m_hWnd) {
	if (::IsWindow(m_hWnd))
	    ::DestroyWindow(m_hWnd);
	m_hWnd = 0;
    }

    if (m_spInPlaceSite)
	m_spInPlaceSite->OnInPlaceDeactivate();

    return S_OK;
}

/*
    Deactivates the control's user interface.
*/
HRESULT WINAPI QAxServerBase::UIDeactivate()
{
    // if we're not UIActive, not much to do.
    if (!isUIActive || !m_spInPlaceSite)
	return S_OK;

    isUIActive = false;

    // notify frame windows, if appropriate, that we're no longer ui-active.
    HWND hwndParent;
    if (m_spInPlaceSite->GetWindow(&hwndParent) == S_OK) {
	if (m_spInPlaceFrame) m_spInPlaceFrame->Release();
	m_spInPlaceFrame = 0;
	IOleInPlaceUIWindow *spInPlaceUIWindow = 0;
        RECT rcPos, rcClip;
        OLEINPLACEFRAMEINFO frameInfo;
        frameInfo.cb = sizeof(OLEINPLACEFRAMEINFO);

	m_spInPlaceSite->GetWindowContext(&m_spInPlaceFrame, &spInPlaceUIWindow, &rcPos, &rcClip, &frameInfo);
	if (spInPlaceUIWindow) {
	    spInPlaceUIWindow->SetActiveObject(0, 0);
	    spInPlaceUIWindow->Release();
	}
	if (m_spInPlaceFrame) {
	    removeMenu();
            if (menuBar) {
                menuBar->removeEventFilter(this);
                menuBar = 0;
            }
            if (statusBar) {
                statusBar->removeEventFilter(this);
		const int index = statusBar->metaObject()->indexOfSignal("messageChanged(QString)");
		QMetaObject::disconnect(statusBar, index, this, -1);
	        statusBar = 0;
            }
	    m_spInPlaceFrame->SetActiveObject(0, 0);
	    m_spInPlaceFrame->Release();
	    m_spInPlaceFrame = 0;
	}
    }
    // we don't need to explicitly release the focus here since somebody
    // else grabbing the focus is usually why we are getting called at all
    m_spInPlaceSite->OnUIDeactivate(false);

    return S_OK;
}

/*
    Positions the control, and applies requested clipping.
*/
HRESULT WINAPI QAxServerBase::SetObjectRects(LPCRECT prcPos, LPCRECT prcClip)
{
    if (prcPos == 0 || prcClip == 0)
	return E_POINTER;

    if (m_hWnd) {
	// the container wants us to clip, so figure out if we really need to
	RECT rcIXect;
	BOOL b = IntersectRect(&rcIXect, prcPos, prcClip);
	HRGN tempRgn = 0;
	if (b && !EqualRect(&rcIXect, prcPos)) {
	    OffsetRect(&rcIXect, -(prcPos->left), -(prcPos->top));
	    tempRgn = CreateRectRgnIndirect(&rcIXect);
	}

	::SetWindowRgn(m_hWnd, tempRgn, true);
	::SetWindowPos(m_hWnd, 0, prcPos->left, prcPos->top,
            prcPos->right - prcPos->left, prcPos->bottom - prcPos->top,
	    SWP_NOZORDER | SWP_NOACTIVATE);
    }

    //Save the new extent.
    m_currentExtent.rwidth() = qBound(qt.widget->minimumWidth(), int(prcPos->right - prcPos->left), qt.widget->maximumWidth());
    m_currentExtent.rheight() = qBound(qt.widget->minimumHeight(), int(prcPos->bottom - prcPos->top), qt.widget->maximumHeight());

    return S_OK;
}

/*
    Not implemented.
*/
HRESULT WINAPI QAxServerBase::ReactivateAndUndo()
{
    return E_NOTIMPL;
}

//**** IOleInPlaceActiveObject

Q_GUI_EXPORT int qt_translateKeyCode(int);

HRESULT WINAPI QAxServerBase::TranslateAcceleratorW(MSG *pMsg)
{
    if (pMsg->message != WM_KEYDOWN || !isWidget)
        return S_FALSE;

    DWORD dwKeyMod = 0;
    if (::GetKeyState(VK_SHIFT) < 0)
        dwKeyMod |= 1;	// KEYMOD_SHIFT
    if (::GetKeyState(VK_CONTROL) < 0)
        dwKeyMod |= 2;	// KEYMOD_CONTROL
    if (::GetKeyState(VK_MENU) < 0)
        dwKeyMod |= 4;	// KEYMOD_ALT

    switch (LOWORD(pMsg->wParam)) {
    case VK_TAB:
        if (isUIActive) {
            bool shift = ::GetKeyState(VK_SHIFT) < 0;
            bool giveUp = true;
            QWidget *curFocus = qt.widget->focusWidget();
            if (curFocus) {
                if (shift) {
                    if (!curFocus->isWindow()) {
                        QWidget *nextFocus = curFocus->nextInFocusChain();
                        QWidget *prevFocus = 0;
                        QWidget *topLevel = 0;
                        while (nextFocus != curFocus) {
                            if (nextFocus->focusPolicy() & Qt::TabFocus) {
                                prevFocus = nextFocus;
                                topLevel = 0;
                            } else if (nextFocus->isWindow()) {
                                topLevel = nextFocus;
                            }
                            nextFocus = nextFocus->nextInFocusChain();
                        }

                        if (!topLevel) {
                            giveUp = false;
                            ((HackWidget*)curFocus)->focusNextPrevChild(false);
                            curFocus->window()->setAttribute(Qt::WA_KeyboardFocusChange);
                        }
                    }
                } else {
                    QWidget *nextFocus = curFocus;
                    while (1) {
                        nextFocus = nextFocus->nextInFocusChain();
                        if (nextFocus->isWindow())
                            break;
                        if (nextFocus->focusPolicy() & Qt::TabFocus) {
                            giveUp = false;
                            ((HackWidget*)curFocus)->focusNextPrevChild(true);
                            curFocus->window()->setAttribute(Qt::WA_KeyboardFocusChange);
                            break;
                        }
                    }
                }
            }
            if (giveUp) {
                HWND hwnd = ::GetParent(m_hWnd);
                ::SetFocus(hwnd);
            } else {
                return S_OK;
            }

        }
        break;

    case VK_LEFT:
    case VK_RIGHT:
    case VK_UP:
    case VK_DOWN:
        if (isUIActive)
            return S_FALSE;
        break;

    default:
        if (isUIActive && qt.widget->focusWidget()) {
            int state = Qt::NoButton;
            if (dwKeyMod & 1)
                state |= Qt::ShiftModifier;
            if (dwKeyMod & 2)
                state |= Qt::ControlModifier;
            if (dwKeyMod & 4)
                state |= Qt::AltModifier;

            int key = pMsg->wParam;
            if (!(key >= 'A' && key <= 'Z') && !(key >= '0' && key <= '9'))
                key = qt_translateKeyCode(pMsg->wParam);

            QKeyEvent override(QEvent::ShortcutOverride, key, (Qt::KeyboardModifiers)state);
            override.ignore();
            QApplication::sendEvent(qt.widget->focusWidget(), &override);
            if (override.isAccepted())
                return S_FALSE;
        }
        break;
    }

    if (!m_spClientSite)
        return S_FALSE;

    IOleControlSite *controlSite = 0;
    m_spClientSite->QueryInterface(IID_IOleControlSite, (void**)&controlSite);
    if (!controlSite)
        return S_FALSE;
    bool resetUserData = false;
    // set server type in the user-data of the window.
#ifdef GWLP_USERDATA
    LONG_PTR serverType = QAX_INPROC_SERVER;
#else
    LONG serverType = QAX_INPROC_SERVER;
#endif
    if (qAxOutProcServer)
        serverType = QAX_OUTPROC_SERVER;
#ifdef GWLP_USERDATA
    LONG_PTR oldData = SetWindowLongPtr(pMsg->hwnd, GWLP_USERDATA, serverType);
#else
    LONG oldData = SetWindowLong(pMsg->hwnd, GWL_USERDATA, serverType);
#endif
    HRESULT hres = controlSite->TranslateAcceleratorW(pMsg, dwKeyMod);
    controlSite->Release();
    // reset the user-data for the window.
#ifdef GWLP_USERDATA
    SetWindowLongPtr(pMsg->hwnd, GWLP_USERDATA, oldData);
#else
    SetWindowLong(pMsg->hwnd, GWL_USERDATA, oldData);
#endif
    return hres;
}

HRESULT WINAPI QAxServerBase::TranslateAcceleratorA(MSG *pMsg)
{
    return TranslateAcceleratorW(pMsg);
}

HRESULT WINAPI QAxServerBase::OnFrameWindowActivate(BOOL fActivate)
{
    if (fActivate) {
	if (wasUIActive)
	    ::SetFocus(m_hWnd);
    } else {
	wasUIActive = isUIActive;
    }
    return S_OK;
}

HRESULT WINAPI QAxServerBase::OnDocWindowActivate(BOOL fActivate)
{
    return S_OK;
}

HRESULT WINAPI QAxServerBase::ResizeBorder(LPCRECT prcBorder, IOleInPlaceUIWindow *pUIWindow, BOOL fFrameWindow)
{
    return S_OK;
}

HRESULT WINAPI QAxServerBase::EnableModeless(BOOL fEnable)
{
    if (!isWidget)
	return S_OK;

    EnableWindow(qt.widget->winId(), fEnable);
    return S_OK;
}

//**** IOleObject

static inline LPOLESTR QStringToOLESTR(const QString &qstring)
{
    LPOLESTR olestr = (wchar_t*)CoTaskMemAlloc(qstring.length()*2+2);
    memcpy(olestr, (ushort*)qstring.unicode(), qstring.length()*2);
    olestr[qstring.length()] = 0;
    return olestr;
}

/*
    \reimp

    See documentation of IOleObject::GetUserType.
*/
HRESULT WINAPI QAxServerBase::GetUserType(DWORD dwFormOfType, LPOLESTR *pszUserType)
{
    if (!pszUserType)
	return E_POINTER;

    switch (dwFormOfType) {
    case USERCLASSTYPE_FULL:
	*pszUserType = QStringToOLESTR(class_name);
	break;
    case USERCLASSTYPE_SHORT:
	if (!qt.widget || !isWidget || qt.widget->windowTitle().isEmpty())
	    *pszUserType = QStringToOLESTR(class_name);
	else
	    *pszUserType = QStringToOLESTR(qt.widget->windowTitle());
	break;
    case USERCLASSTYPE_APPNAME:
	*pszUserType = QStringToOLESTR(qApp->objectName());
	break;
    }

    return S_OK;
}

/*
    Returns the status flags registered for this control.
*/
HRESULT WINAPI QAxServerBase::GetMiscStatus(DWORD dwAspect, DWORD *pdwStatus)
{
    return OleRegGetMiscStatus(qAxFactory()->classID(class_name), dwAspect, pdwStatus);
}

/*
    Stores the provided advise sink.
*/
HRESULT WINAPI QAxServerBase::Advise(IAdviseSink* pAdvSink, DWORD* pdwConnection)
{
    *pdwConnection = adviseSinks.count() + 1;
    STATDATA data = { {0, 0, DVASPECT_CONTENT, -1, TYMED_NULL} , 0, pAdvSink, *pdwConnection };
    adviseSinks.append(data);
    pAdvSink->AddRef();
    return S_OK;
}

/*
    Closes the control.
*/
HRESULT WINAPI QAxServerBase::Close(DWORD dwSaveOption)
{
    if (dwSaveOption != OLECLOSE_NOSAVE && m_spClientSite)
	m_spClientSite->SaveObject();
    if (isInPlaceActive) {
	HRESULT hr = InPlaceDeactivate();
	if (FAILED(hr))
	    return hr;
    }
    if (m_hWnd) {
	if (IsWindow(m_hWnd))
	    DestroyWindow(m_hWnd);
	m_hWnd = 0;
	if (m_spClientSite)
	    m_spClientSite->OnShowWindow(false);
    }

    if (m_spInPlaceSite) m_spInPlaceSite->Release();
    m_spInPlaceSite = 0;

    if (m_spAdviseSink)
	m_spAdviseSink->OnClose();
    for (int i = 0; i < adviseSinks.count(); ++i) {
        adviseSinks.at(i).pAdvSink->OnClose();
    }

    return S_OK;
}

bool qax_disable_inplaceframe = true;

/*
    Executes the steps to activate the control.
*/
HRESULT QAxServerBase::internalActivate()
{
    if (!m_spClientSite)
	return S_OK;
    if (!m_spInPlaceSite)
        m_spClientSite->QueryInterface(IID_IOleInPlaceSite, (void**)&m_spInPlaceSite);
    if (!m_spInPlaceSite)
	return E_FAIL;

    HRESULT hr = E_FAIL;
    if (!isInPlaceActive) {
	BOOL bNoRedraw = false;
	hr = m_spInPlaceSite->CanInPlaceActivate();
	if (FAILED(hr))
	    return hr;
	if (hr != S_OK)
	    return E_FAIL;
	m_spInPlaceSite->OnInPlaceActivate();
    }

    isInPlaceActive = true;
    OnAmbientPropertyChange(DISPID_AMBIENT_USERMODE);

    if (isWidget) {
        IOleInPlaceUIWindow *spInPlaceUIWindow = 0;
        HWND hwndParent;
        if (m_spInPlaceSite->GetWindow(&hwndParent) == S_OK) {
            // get location in the parent window, as well as some information about the parent
            if (m_spInPlaceFrame) m_spInPlaceFrame->Release();
            m_spInPlaceFrame = 0;
            RECT rcPos, rcClip;
            OLEINPLACEFRAMEINFO frameInfo;
            frameInfo.cb = sizeof(OLEINPLACEFRAMEINFO);
            m_spInPlaceSite->GetWindowContext(&m_spInPlaceFrame, &spInPlaceUIWindow, &rcPos, &rcClip, &frameInfo);
            if (m_hWnd) {
                ::ShowWindow(m_hWnd, SW_SHOW);
                if (!::IsChild(m_hWnd, ::GetFocus()) && qt.widget->focusPolicy() != Qt::NoFocus)
                    ::SetFocus(m_hWnd);
            } else {
                create(hwndParent, rcPos);
            }
        }

	// Gone active by now, take care of UIACTIVATE
	canTakeFocus = qt.widget->focusPolicy() != Qt::NoFocus && !inDesignMode;
	if (!canTakeFocus && !inDesignMode) {
	    QList<QWidget*> widgets = qt.widget->findChildren<QWidget*>();
	    for (int w = 0; w < widgets.count(); ++w) {
		QWidget *widget = widgets[w];
		canTakeFocus = widget->focusPolicy() != Qt::NoFocus;
                if (canTakeFocus)
                    break;
	    }
	}
	if (!isUIActive && canTakeFocus) {
	    isUIActive = true;
	    hr = m_spInPlaceSite->OnUIActivate();
	    if (FAILED(hr)) {
		if (m_spInPlaceFrame) m_spInPlaceFrame->Release();
		m_spInPlaceFrame = 0;
		if (spInPlaceUIWindow) spInPlaceUIWindow->Release();
		return hr;
	    }

	    if (isInPlaceActive) {
		if (!::IsChild(m_hWnd, ::GetFocus()))
		    ::SetFocus(m_hWnd);
	    }

	    if (m_spInPlaceFrame) {
		hr = m_spInPlaceFrame->SetActiveObject(this, QStringToBSTR(class_name));
		if (!FAILED(hr)) {
		    menuBar = (qt.widget && !qax_disable_inplaceframe) ? qt.widget->findChild<QMenuBar*>() : 0;
		    if (menuBar && !menuBar->isVisible()) {
			createMenu(menuBar);
			menuBar->hide();
			menuBar->installEventFilter(this);
		    }
		    statusBar = qt.widget ? qt.widget->findChild<QStatusBar*>() : 0;
		    if (statusBar && !statusBar->isVisible()) {
			const int index = statusBar->metaObject()->indexOfSignal("messageChanged(QString)");
			QMetaObject::connect(statusBar, index, this, -1);
			statusBar->hide();
			statusBar->installEventFilter(this);
		    }
		}
	    }
	    if (spInPlaceUIWindow) {
		spInPlaceUIWindow->SetActiveObject(this, QStringToBSTR(class_name));
		spInPlaceUIWindow->SetBorderSpace(0);
	    }
	}
        if (spInPlaceUIWindow) spInPlaceUIWindow->Release();
	ShowWindow(m_hWnd, SW_NORMAL);
    }

    m_spClientSite->ShowObject();

    return S_OK;
}

/*
    Executes the "verb" \a iVerb.
*/
HRESULT WINAPI QAxServerBase::DoVerb(LONG iVerb, LPMSG /*lpmsg*/, IOleClientSite* /*pActiveSite*/, LONG /*lindex*/,
			       HWND /*hwndParent*/, LPCRECT /*prcPosRect*/)
{
    HRESULT hr = E_NOTIMPL;
    switch (iVerb)
    {
    case OLEIVERB_SHOW:
	hr = internalActivate();
	if (SUCCEEDED(hr))
	    hr = S_OK;
	break;

    case OLEIVERB_PRIMARY:
    case OLEIVERB_INPLACEACTIVATE:
	hr = internalActivate();
	if (SUCCEEDED(hr)) {
	    hr = S_OK;
	    update();
	}
	break;

    case OLEIVERB_UIACTIVATE:
	if (!isUIActive) {
	    hr = internalActivate();
	    if (SUCCEEDED(hr))
		hr = S_OK;
	}
	break;

    case OLEIVERB_HIDE:
	UIDeactivate();
	if (m_hWnd)
	    ::ShowWindow(m_hWnd, SW_HIDE);
	hr = S_OK;
	return hr;

    default:
	break;
    }
    return hr;
}

/*
    Not implemented.
*/
HRESULT WINAPI QAxServerBase::EnumAdvise(IEnumSTATDATA** /*ppenumAdvise*/)
{
    return E_NOTIMPL;
}

/*
    Returns an enumerator for the verbs registered for this class.
*/
HRESULT WINAPI QAxServerBase::EnumVerbs(IEnumOLEVERB** ppEnumOleVerb)
{
    if (!ppEnumOleVerb)
	return E_POINTER;
    return OleRegEnumVerbs(qAxFactory()->classID(class_name), ppEnumOleVerb);
}

/*
    Returns the current client site..
*/
HRESULT WINAPI QAxServerBase::GetClientSite(IOleClientSite** ppClientSite)
{
    if (!ppClientSite)
	return E_POINTER;
    *ppClientSite = m_spClientSite;
    if (*ppClientSite)
	(*ppClientSite)->AddRef();
    return S_OK;
}

/*
    Not implemented.
*/
HRESULT WINAPI QAxServerBase::GetClipboardData(DWORD, IDataObject**)
{
    return E_NOTIMPL;
}

/*
    Returns the current extent.
*/
HRESULT WINAPI QAxServerBase::GetExtent(DWORD dwDrawAspect, SIZEL* psizel)
{
    if (dwDrawAspect != DVASPECT_CONTENT || !isWidget || !qt.widget)
	return E_FAIL;
    if (!psizel)
	return E_POINTER;

    psizel->cx = MAP_PIX_TO_LOGHIM(m_currentExtent.width(), qt.widget->logicalDpiX());
    psizel->cy = MAP_PIX_TO_LOGHIM(m_currentExtent.height(), qt.widget->logicalDpiY());
    return S_OK;
}

/*
    Not implemented.
*/
HRESULT WINAPI QAxServerBase::GetMoniker(DWORD, DWORD, IMoniker** )
{
    return E_NOTIMPL;
}

/*
    Returns the CLSID of this class.
*/
HRESULT WINAPI QAxServerBase::GetUserClassID(CLSID* pClsid)
{
    if (!pClsid)
	return E_POINTER;
    *pClsid = qAxFactory()->classID(class_name);
    return S_OK;
}

/*
    Not implemented.
*/
HRESULT WINAPI QAxServerBase::InitFromData(IDataObject*, BOOL, DWORD)
{
    return E_NOTIMPL;
}

/*
    Not implemented.
*/
HRESULT WINAPI QAxServerBase::IsUpToDate()
{
    return S_OK;
}

/*
    Stores the client site.
*/
HRESULT WINAPI QAxServerBase::SetClientSite(IOleClientSite* pClientSite)
{
    // release all client site interfaces
    if (m_spClientSite) m_spClientSite->Release();
    if (m_spInPlaceSite) m_spInPlaceSite->Release();
    m_spInPlaceSite = 0;
    if (m_spInPlaceFrame) m_spInPlaceFrame->Release();
    m_spInPlaceFrame = 0;

    m_spClientSite = pClientSite;
    if (m_spClientSite) {
        m_spClientSite->AddRef();
	m_spClientSite->QueryInterface(IID_IOleInPlaceSite, (void **)&m_spInPlaceSite);
    }

    return S_OK;
}

/*
    Not implemented.
*/
HRESULT WINAPI QAxServerBase::SetColorScheme(LOGPALETTE*)
{
    return E_NOTIMPL;
}


#ifndef QT_STATIC    // avoid conflict with symbol in static lib
bool qt_sendSpontaneousEvent(QObject *o, QEvent *e)
{
    return QCoreApplication::sendSpontaneousEvent(o, e);
}
#endif

/*
    Tries to set the size of the control.
*/
HRESULT WINAPI QAxServerBase::SetExtent(DWORD dwDrawAspect, SIZEL* psizel)
{
    if (dwDrawAspect != DVASPECT_CONTENT)
	return DV_E_DVASPECT;
    if (!psizel)
	return E_POINTER;

    if (!isWidget || !qt.widget) // nothing to do
	return S_OK;

    QSize proposedSize(MAP_LOGHIM_TO_PIX(psizel->cx, qt.widget->logicalDpiX()),
        MAP_LOGHIM_TO_PIX(psizel->cy, qt.widget->logicalDpiY()));

    // can the widget be resized at all?
    if (qt.widget->minimumSize() == qt.widget->maximumSize() && qt.widget->minimumSize() != proposedSize)
        return E_FAIL;
    //Save the extent, bound to the widget restrictions.
    m_currentExtent.rwidth() = qBound(qt.widget->minimumWidth(), proposedSize.width(), qt.widget->maximumWidth());
    m_currentExtent.rheight() = qBound(qt.widget->minimumHeight(), proposedSize.height(), qt.widget->maximumHeight());

    resize(proposedSize);
    return S_OK;
}

/*
    Not implemented.
*/
HRESULT WINAPI QAxServerBase::SetHostNames(LPCOLESTR szContainerApp, LPCOLESTR szContainerObj)
{
    return S_OK;
}

/*
    Not implemented.
*/
HRESULT WINAPI QAxServerBase::SetMoniker(DWORD, IMoniker*)
{
    return E_NOTIMPL;
}

/*
    Disconnects an advise sink.
*/
HRESULT WINAPI QAxServerBase::Unadvise(DWORD dwConnection)
{
    for (int i = 0; i < adviseSinks.count(); ++i) {
        STATDATA entry = adviseSinks.at(i);
        if (entry.dwConnection == dwConnection) {
            entry.pAdvSink->Release();
            adviseSinks.removeAt(i);
            return S_OK;
        }
    }
    return OLE_E_NOCONNECTION;
}

/*
    Not implemented.
*/
HRESULT WINAPI QAxServerBase::Update()
{
    return S_OK;
}

//**** IDataObject
/*
    Calls IViewObject::Draw after setting up the parameters.
*/
HRESULT WINAPI QAxServerBase::GetData(FORMATETC *pformatetcIn, STGMEDIUM *pmedium)
{
    if (!pmedium)
	return E_POINTER;
    if ((pformatetcIn->tymed & TYMED_MFPICT) == 0)
	return DATA_E_FORMATETC;

    internalCreate();
    if (!isWidget || !qt.widget)
	return E_UNEXPECTED;

    // Container wants to draw, but the size is not defined yet - ask container
    if (m_spInPlaceSite && !qt.widget->testAttribute(Qt::WA_Resized)) {
	IOleInPlaceUIWindow *spInPlaceUIWindow = 0;
        RECT rcPos, rcClip;
        OLEINPLACEFRAMEINFO frameInfo;
        frameInfo.cb = sizeof(OLEINPLACEFRAMEINFO);

	HRESULT hres = m_spInPlaceSite->GetWindowContext(&m_spInPlaceFrame, &spInPlaceUIWindow, &rcPos, &rcClip, &frameInfo);
        if (hres == S_OK) {
            QSize size(rcPos.right - rcPos.left, rcPos.bottom - rcPos.top);
            resize(size);
        } else {
            qt.widget->adjustSize();
        }
        if (spInPlaceUIWindow) spInPlaceUIWindow->Release(); // no need for it
    }

    int width = qt.widget->width();
    int height = qt.widget->height();
    RECTL rectl = {0, 0, width, height};

    HDC hdc = CreateMetaFile(0);
    SaveDC(hdc);
    SetWindowOrgEx(hdc, 0, 0, 0);
    SetWindowExtEx(hdc, rectl.right, rectl.bottom, 0);

    Draw(pformatetcIn->dwAspect, pformatetcIn->lindex, 0, pformatetcIn->ptd, 0, hdc, &rectl, &rectl, 0, 0);

    RestoreDC(hdc, -1);
    HMETAFILE hMF = CloseMetaFile(hdc);
    if (!hMF)
	return E_UNEXPECTED;

    HGLOBAL hMem = GlobalAlloc(GMEM_SHARE | GMEM_MOVEABLE, sizeof(METAFILEPICT));
    if (!hMem) {
	DeleteMetaFile(hMF);
	return ResultFromScode(STG_E_MEDIUMFULL);
    }

    LPMETAFILEPICT pMF = (LPMETAFILEPICT)GlobalLock(hMem);
    pMF->hMF = hMF;
    pMF->mm = MM_ANISOTROPIC;
    pMF->xExt = MAP_PIX_TO_LOGHIM(width, qt.widget->logicalDpiX());
    pMF->yExt = MAP_PIX_TO_LOGHIM(height, qt.widget->logicalDpiY());
    GlobalUnlock(hMem);

    memset(pmedium, 0, sizeof(STGMEDIUM));
    pmedium->tymed = TYMED_MFPICT;
    pmedium->hGlobal = hMem;
    pmedium->pUnkForRelease = 0;

    return S_OK;
}

/*
    Not implemented.
*/
HRESULT WINAPI QAxServerBase::DAdvise(FORMATETC *pformatetc, DWORD advf,
				      IAdviseSink *pAdvSink, DWORD *pdwConnection)
{
    if (pformatetc->dwAspect != DVASPECT_CONTENT)
        return E_FAIL;

    *pdwConnection = adviseSinks.count() + 1;
    STATDATA data = {
        {pformatetc->cfFormat,pformatetc->ptd,pformatetc->dwAspect,pformatetc->lindex,pformatetc->tymed},
        advf, pAdvSink, *pdwConnection
    };
    adviseSinks.append(data);
    pAdvSink->AddRef();
    return S_OK;
}

/*
    Not implemented.
*/
HRESULT WINAPI QAxServerBase::DUnadvise(DWORD dwConnection)
{
    return Unadvise(dwConnection);
}

/*
    Not implemented.
*/
HRESULT WINAPI QAxServerBase::EnumDAdvise(IEnumSTATDATA ** /*ppenumAdvise*/)
{
    return E_NOTIMPL;
}

/*
    Not implemented.
*/
HRESULT WINAPI QAxServerBase::GetDataHere(FORMATETC* /* pformatetc */, STGMEDIUM* /* pmedium */)
{
    return E_NOTIMPL;
}

/*
    Not implemented.
*/
HRESULT WINAPI QAxServerBase::QueryGetData(FORMATETC* /* pformatetc */)
{
    return E_NOTIMPL;
}

/*
    Not implemented.
*/
HRESULT WINAPI QAxServerBase::GetCanonicalFormatEtc(FORMATETC* /* pformatectIn */,FORMATETC* /* pformatetcOut */)
{
    return E_NOTIMPL;
}

/*
    Not implemented.
*/
HRESULT WINAPI QAxServerBase::SetData(FORMATETC* /* pformatetc */, STGMEDIUM* /* pmedium */, BOOL /* fRelease */)
{
    return E_NOTIMPL;
}

/*
    Not implemented.
*/
HRESULT WINAPI QAxServerBase::EnumFormatEtc(DWORD /* dwDirection */, IEnumFORMATETC** /* ppenumFormatEtc */)
{
    return E_NOTIMPL;
}



static int mapModifiers(int state)
{
    int ole = 0;
    if (state & Qt::ShiftModifier)
	ole |= 1;
    if (state & Qt::ControlModifier)
	ole |= 2;
    if (state & Qt::AltModifier)
	ole |= 4;

    return ole;
}

/*
    \reimp
*/
bool QAxServerBase::eventFilter(QObject *o, QEvent *e)
{
    if (!theObject)
	return QObject::eventFilter(o, e);

    if ((e->type() == QEvent::Show || e->type() == QEvent::Hide) && (o == statusBar || o == menuBar)) {
	if (o == menuBar) {
	    if (e->type() == QEvent::Hide) {
		createMenu(menuBar);
	    } else if (e->type() == QEvent::Show) {
		removeMenu();
	    }
	} else if (statusBar) {
	    statusBar->setSizeGripEnabled(false);
	}
	updateGeometry();
	if (m_spInPlaceSite && qt.widget->sizeHint().isValid()) {
            RECT rect = {0, 0, qt.widget->sizeHint().width(), qt.widget->sizeHint().height()};
	    m_spInPlaceSite->OnPosRectChange(&rect);
	}
    }
    switch (e->type()) {
    case QEvent::ChildAdded:
	static_cast<QChildEvent*>(e)->child()->installEventFilter(this);
	break;
    case QEvent::ChildRemoved:
	static_cast<QChildEvent*>(e)->child()->removeEventFilter(this);
	break;
    case QEvent::KeyPress:
	if (o == qt.object && hasStockEvents) {
	    QKeyEvent *ke = (QKeyEvent*)e;
	    int key = ke->key();
	    int state = ke->modifiers();
	    void *argv[] = {
		0,
		&key,
		&state
	    };
	    qt_metacall(QMetaObject::InvokeMetaMethod, DISPID_KEYDOWN, argv);
	    if (!ke->text().isEmpty())
		qt_metacall(QMetaObject::InvokeMetaMethod, DISPID_KEYPRESS, argv);
	}
	break;
    case QEvent::KeyRelease:
	if (o == qt.object && hasStockEvents) {
	    QKeyEvent *ke = (QKeyEvent*)e;
	    int key = ke->key();
	    int state = ke->modifiers();
	    void *argv[] = {
		0,
		&key,
		&state
	    };
	    qt_metacall(QMetaObject::InvokeMetaMethod, DISPID_KEYUP, argv);
	}
	break;
    case QEvent::MouseMove:
	if (o == qt.object && hasStockEvents) {
	    QMouseEvent *me = (QMouseEvent*)e;
            int button = me->buttons() & Qt::MouseButtonMask;
	    int state = mapModifiers(me->modifiers());
	    int x = me->x();
	    int y = me->y();
	    void *argv[] = {
		0,
		&button,
		&state,
		&x,
		&y
	    };
	    qt_metacall(QMetaObject::InvokeMetaMethod, DISPID_MOUSEMOVE, argv);
	}
	break;
    case QEvent::MouseButtonRelease:
	if (o == qt.object && hasStockEvents) {
	    QMouseEvent *me = (QMouseEvent*)e;
	    int button = me->button();
	    int state = mapModifiers(me->modifiers());
	    int x = me->x();
	    int y = me->y();
	    void *argv[] = {
		0,
		&button,
		&state,
		&x,
		&y
	    };
	    qt_metacall(QMetaObject::InvokeMetaMethod, DISPID_MOUSEUP, argv);
	    qt_metacall(QMetaObject::InvokeMetaMethod, DISPID_CLICK, 0);
	}
	break;
    case QEvent::MouseButtonDblClick:
	if (o == qt.object && hasStockEvents) {
	    qt_metacall(QMetaObject::InvokeMetaMethod, DISPID_DBLCLICK, 0);
	}
	break;
    case QEvent::MouseButtonPress:
        if (m_spInPlaceSite && !isUIActive) {
            internalActivate();
        }
	if (o == qt.widget && hasStockEvents) {
	    QMouseEvent *me = (QMouseEvent*)e;
	    int button = me->button();
	    int state = mapModifiers(me->modifiers());
	    int x = me->x();
	    int y = me->y();
	    void *argv[] = {
		0,
		&button,
		&state,
		&x,
		&y
	    };
	    qt_metacall(QMetaObject::InvokeMetaMethod, DISPID_MOUSEDOWN, argv);
	}
	break;
    case QEvent::Show:
	if (m_hWnd && o == qt.widget)
	    ShowWindow(m_hWnd, SW_SHOW);
	updateMask();
	break;
    case QEvent::Hide:
	if (m_hWnd && o == qt.widget)
	    ShowWindow(m_hWnd, SW_HIDE);
	break;

    case QEvent::EnabledChange:
        if (m_hWnd && o == qt.widget)
            EnableWindow(m_hWnd, qt.widget->isEnabled());
        // Fall Through
    case QEvent::FontChange:
    case QEvent::ActivationChange:
    case QEvent::StyleChange:
    case QEvent::IconTextChange:
    case QEvent::ModifiedChange:
    case QEvent::Resize:
	updateMask();
	break;
    case QEvent::WindowBlocked: {
        if (!m_spInPlaceFrame)
            break;
        m_spInPlaceFrame->EnableModeless(FALSE);
        MSG msg;
        // Visual Basic 6.0 posts the message WM_USER+3078 from the EnableModeless().
        // While handling this message, VB will disable all current top-levels. After
        // this we have to re-enable the Qt modal widget to receive input events.
        if (PeekMessage(&msg, 0, WM_USER+3078, WM_USER+3078, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            QWidget *modalWidget = QApplication::activeModalWidget();
            if (modalWidget && modalWidget->isVisible() && modalWidget->isEnabled()
                && !IsWindowEnabled(modalWidget->effectiveWinId()))
                EnableWindow(modalWidget->effectiveWinId(), TRUE);
        }
        break;
        }
    case QEvent::WindowUnblocked:
        if (!m_spInPlaceFrame)
            break;
        m_spInPlaceFrame->EnableModeless(TRUE);
        break;
    default:
	break;
    }
    return QObject::eventFilter(o, e);
}

QT_END_NAMESPACE
#endif // QT_NO_WIN_ACTIVEQT
