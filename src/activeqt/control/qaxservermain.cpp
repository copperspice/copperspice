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

#include <qstringlist.h>
#include <qvector.h>
#include <qaxfactory.h>

#ifndef QT_NO_WIN_ACTIVEQT

#include <qt_windows.h>

QT_BEGIN_NAMESPACE

static DWORD *classRegistration = 0;
static DWORD dwThreadID;
static bool qAxActivity = false;
static HANDLE hEventShutdown;

#ifdef QT_DEBUG
static const DWORD dwTimeOut = 1000;
static const DWORD dwPause = 500;
#else
static const DWORD dwTimeOut = 5000; // time for EXE to be idle before shutting down
static const DWORD dwPause = 1000; // time to wait for threads to finish up
#endif

extern HANDLE hEventShutdown;
extern bool qAxActivity;
extern HANDLE qAxInstance;
extern bool qAxIsServer;
extern bool qAxOutProcServer;
extern wchar_t qAxModuleFilename[MAX_PATH];
extern QString qAxInit();
extern void qAxCleanup();
extern HRESULT UpdateRegistry(BOOL bRegister);
extern HRESULT GetClassObject(const GUID &clsid, const GUID &iid, void **ppUnk);
extern ulong qAxLockCount();
extern bool qax_winEventFilter(void *message);


STDAPI DumpIDL(const QString &outfile, const QString &ver);

// Monitors the shutdown event
static DWORD WINAPI MonitorProc(void* pv)
{
    while (1) {
        WaitForSingleObject(hEventShutdown, INFINITE);
        DWORD dwWait=0;
        do {
            qAxActivity = false;
            dwWait = WaitForSingleObject(hEventShutdown, dwTimeOut);
        } while (dwWait == WAIT_OBJECT_0);
        // timed out
        if (!qAxActivity && !qAxLockCount()) // if no activity let's really bail
            break;
    }
    CloseHandle(hEventShutdown);
    PostThreadMessage(dwThreadID, WM_QUIT, 0, 0);
    PostQuitMessage(0);
    
    return 0;
}

// Starts the monitoring thread
static bool StartMonitor()
{
    dwThreadID = GetCurrentThreadId();
    hEventShutdown = CreateEvent(0, false, false, 0);
    if (hEventShutdown == 0)
        return false;
    DWORD dwThreadID;
    HANDLE h = CreateThread(0, 0, MonitorProc, 0, 0, &dwThreadID);
    return (h != NULL);
}

void qax_shutDown()
{
    qAxActivity = true;
    if (hEventShutdown)
        SetEvent(hEventShutdown); // tell monitor that we transitioned to zero
}

/*
    Start the COM server (if necessary).
*/
bool qax_startServer(QAxFactory::ServerType type)
{
    if (qAxIsServer)
        return true;
    
    const QStringList keys = qAxFactory()->featureList();
    if (!keys.count())
        return false;
    
    if (!qAxFactory()->isService())
        StartMonitor();
    
    classRegistration = new DWORD[keys.count()];
    int object = 0;
    for (QStringList::ConstIterator key = keys.begin(); key != keys.end(); ++key, ++object) {
        IUnknown* p = 0;
        CLSID clsid = qAxFactory()->classID(*key);
        
        // Create a QClassFactory (implemented in qaxserverbase.cpp)
        HRESULT hRes = GetClassObject(clsid, IID_IClassFactory, (void**)&p);
        if (SUCCEEDED(hRes))
            hRes = CoRegisterClassObject(clsid, p, CLSCTX_LOCAL_SERVER,
                type == QAxFactory::MultipleInstances ? REGCLS_MULTIPLEUSE : REGCLS_SINGLEUSE,
                classRegistration+object);
        if (p)
            p->Release();
    }

    qAxIsServer = true;
    return true;
}

/*
    Stop the COM server (if necessary).
*/
bool qax_stopServer()
{
    if (!qAxIsServer || !classRegistration)
        return true;
    
    qAxIsServer = false;
    
    const QStringList keys = qAxFactory()->featureList();
    int object = 0;
    for (QStringList::ConstIterator key = keys.begin(); key != keys.end(); ++key, ++object)
        CoRevokeClassObject(classRegistration[object]);
    
    delete []classRegistration;
    classRegistration = 0;
    
    Sleep(dwPause); //wait for any threads to finish
    
    return true;
}


extern void qWinMain(HINSTANCE, HINSTANCE, LPSTR, int, int &, QVector<char *> &);

QT_END_NAMESPACE

#if defined(QT_NEEDS_QMAIN)
int qMain(int, char **);
#define main qMain
#else
extern "C" int main(int, char **);
#endif


EXTERN_C int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR, int nShowCmd)
{
    QT_USE_NAMESPACE 

    qAxOutProcServer = true;
    GetModuleFileName(0, qAxModuleFilename, MAX_PATH);
    qAxInstance = hInstance;

    QByteArray cmdParam = QString::fromWCharArray(GetCommandLine()).toLocal8Bit();
    QList<QByteArray> cmds = cmdParam.split(' ');
    QByteArray unprocessed;

    int nRet = 0;
    bool run = true;
    bool runServer = false;
    for (int i = 0; i < cmds.count(); ++i) {
        QByteArray cmd = cmds.at(i).toLower();
        if (cmd == "-activex" || cmd == "/activex" || cmd == "-embedding" || cmd == "/embedding") {
            runServer = true;
        } else if (cmd == "-unregserver" || cmd == "/unregserver") {
            nRet = UpdateRegistry(false);
            run = false;
            break;
        } else if (cmd == "-regserver" || cmd == "/regserver") {
            nRet = UpdateRegistry(true);
            run = false;
            break;
        } else if (cmd == "-dumpidl" || cmd == "/dumpidl") {
            ++i;
            if (i < cmds.count()) {
                QByteArray outfile = cmds.at(i);
                ++i;
                QByteArray version;
                if (i < cmds.count() && (cmds.at(i) == "-version" || cmds.at(i) == "/version")) {
                    ++i;
                    if (i < cmds.count())
                        version = cmds.at(i);
                    else
                        version = "1.0";
                }
                
                nRet = DumpIDL(QString::fromLatin1(outfile.constData()), QString::fromLatin1(version.constData()));
            } else {
                qWarning("Wrong commandline syntax: <app> -dumpidl <idl file> [-version <x.y.z>]");
            }
            run = false;
            break;
        } else {
            unprocessed += cmds.at(i) + ' ';
        }
    }
    
    if (run) {
        HRESULT hRes = CoInitialize(0);

        int argc;
        QVector<char*> argv(8);
        qWinMain(hInstance, hPrevInstance, unprocessed.data(), nShowCmd, argc, argv);
        qAxInit();
        if (runServer)
            QAxFactory::startServer();
        nRet = ::main(argc, argv.data());
        QAxFactory::stopServer();
        qAxCleanup();
        CoUninitialize();
        
    }
    
    return nRet;
}

#endif // QT_NO_WIN_ACTIVEQT
