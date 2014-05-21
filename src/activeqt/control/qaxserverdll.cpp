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

#include <qapplication.h>
#include <qwidget.h>

#include <qt_windows.h>

QT_BEGIN_NAMESPACE

bool qax_ownQApp = false;
HHOOK qax_hhook = 0;

// in qaxserver.cpp
extern wchar_t qAxModuleFilename[MAX_PATH];
extern bool qAxIsServer;
extern ITypeLib *qAxTypeLibrary;
extern unsigned long qAxLockCount();
extern QString qAxInit();
extern void qAxCleanup();
extern HANDLE qAxInstance;
static uint qAxThreadId = 0;

extern HRESULT UpdateRegistry(int bRegister);
extern HRESULT GetClassObject(const GUID &clsid, const GUID &iid, void **ppUnk);

STDAPI DllRegisterServer()
{
    return UpdateRegistry(true);
}

STDAPI DllUnregisterServer()
{
    return UpdateRegistry(false);
}

STDAPI DllGetClassObject(const GUID &clsid, const GUID &iid, void** ppv)
{
    if (!qAxThreadId)
        qAxThreadId = GetCurrentThreadId();
    else if (GetCurrentThreadId() != qAxThreadId)
        return E_FAIL; 

    GetClassObject(clsid, iid, ppv);
    if (!*ppv)
        return CLASS_E_CLASSNOTAVAILABLE;
    return S_OK;
}

STDAPI DllCanUnloadNow()
{
    if (GetCurrentThreadId() != qAxThreadId)
        return S_FALSE;
    if (qAxLockCount())
        return S_FALSE;
    if (!qax_ownQApp)
        return S_OK;
    
    // check if qApp still runs widgets (in other DLLs)
    QWidgetList widgets = qApp->allWidgets();
    int count = widgets.count();
    for (int w = 0; w < widgets.count(); ++w) {
        // remove all Qt generated widgets
        QWidget *widget = widgets.at(w);
        if (widget->windowType() == Qt::Desktop || widget->objectName() == QLatin1String("Qt internal tablet widget"))
            count--;
    }
    if (count)
        return S_FALSE;
    
    // no widgets left - destroy qApp
    if (qax_hhook)
        UnhookWindowsHookEx(qax_hhook);
    
    delete qApp;
    qax_ownQApp = false;
    
    // never allow unloading - safety net for Internet Explorer
    return S_FALSE;
}


EXTERN_C BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID lpvReserved)
{
    GetModuleFileName(hInstance, qAxModuleFilename, MAX_PATH);
    qAxInstance = hInstance;
    qAxIsServer = true;
    
    if (dwReason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hInstance);
        qAxInit();
    } else if (dwReason == DLL_PROCESS_DETACH) {
        qAxCleanup();
    }
    
    return true;
}

QT_END_NAMESPACE
#endif // QT_NO_WIN_ACTIVEQT
