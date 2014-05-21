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

#include "qaxscript.h"

#ifndef QT_NO_WIN_ACTIVEQT

#if defined(Q_CC_GNU)
# define QT_NO_QAXSCRIPT
#endif

#include <qapplication.h>
#include <qfile.h>
#include <qhash.h>
#include <qmetaobject.h>
#include <quuid.h>
#include <qwidget.h>

#include <qt_windows.h>
#ifndef QT_NO_QAXSCRIPT
#include <initguid.h>
#include <activscp.h>
#endif

#include "../shared/qaxtypes.h"

QT_BEGIN_NAMESPACE

struct QAxEngineDescriptor { QString name, extension, code; };
static QList<QAxEngineDescriptor> engines;

class QAxScriptManagerPrivate
{
public:
    QHash<QString, QAxScript*> scriptDict;
    QHash<QString, QAxBase*> objectDict;
};

/*
    \class QAxScriptSite
    \brief The QAxScriptSite class implements a Windows Scripting Host
    \internal

    The QAxScriptSite is used internally to communicate callbacks from the script
    engine to the script manager.
*/

#ifndef QT_NO_QAXSCRIPT

class QAxScriptSite : public IActiveScriptSite, public IActiveScriptSiteWindow
{
public:
    QAxScriptSite(QAxScript *script);
    
    ULONG WINAPI AddRef();
    ULONG WINAPI Release();
    HRESULT WINAPI QueryInterface(REFIID iid, void **ppvObject);
    
    HRESULT WINAPI GetLCID(LCID *plcid);
    HRESULT WINAPI GetItemInfo(LPCOLESTR pstrName, DWORD dwReturnMask, IUnknown **ppiunkItem, ITypeInfo **ppti);
    HRESULT WINAPI GetDocVersionString(BSTR *pbstrVersion);
    
    HRESULT WINAPI OnScriptTerminate(const VARIANT *pvarResult, const EXCEPINFO *pexcepinfo);
    HRESULT WINAPI OnStateChange(SCRIPTSTATE ssScriptState);
    HRESULT WINAPI OnScriptError(IActiveScriptError *pscripterror);
    HRESULT WINAPI OnEnterScript();
    HRESULT WINAPI OnLeaveScript();
    
    HRESULT WINAPI GetWindow(HWND *phwnd);
    HRESULT WINAPI EnableModeless(BOOL fEnable);
    
protected:
    QWidget *window() const;
    
private:
    QAxScript *script;
    LONG ref;
};

/*
    Constructs the site for the \a s.
*/
QAxScriptSite::QAxScriptSite(QAxScript *s)
: script(s), ref(1)
{
}

/*
    Implements IUnknown::AddRef
*/
ULONG WINAPI QAxScriptSite::AddRef()
{
    return InterlockedIncrement(&ref);
}

/*
    Implements IUnknown::Release
*/
ULONG WINAPI QAxScriptSite::Release()
{
    LONG refCount = InterlockedDecrement(&ref);
    if (!refCount)
        delete this;

    return refCount;
}

/*
    Implements IUnknown::QueryInterface
*/
HRESULT WINAPI QAxScriptSite::QueryInterface(REFIID iid, void **ppvObject)
{
    *ppvObject = 0;
    if (iid == IID_IUnknown)
        *ppvObject = (IUnknown*)(IActiveScriptSite*)this;
    else if (iid == IID_IActiveScriptSite)
        *ppvObject = (IActiveScriptSite*)this;
    else if (iid == IID_IActiveScriptSiteWindow)
        *ppvObject = (IActiveScriptSiteWindow*)this;
    else
        return E_NOINTERFACE;
    
    AddRef();
    return S_OK;
}

/*
    Implements IActiveScriptSite::GetLCID

    This method is not implemented. Use the system-defined locale.
*/
HRESULT WINAPI QAxScriptSite::GetLCID(LCID * /*plcid*/)
{
    return E_NOTIMPL;
}

/*
    Implements IActiveScriptSite::GetItemInfo

    Tries to find the QAxBase for \a pstrName and returns the 
    relevant interfaces in \a item and \a type as requested through \a mask.
*/
HRESULT WINAPI QAxScriptSite::GetItemInfo(LPCOLESTR pstrName, DWORD mask, IUnknown **item, ITypeInfo **type)
{
    if (item)
        *item = 0;
    else if (mask & SCRIPTINFO_IUNKNOWN)
        return E_POINTER;
    
    if (type)
        *type = 0;
    else if (mask & SCRIPTINFO_ITYPEINFO)
        return E_POINTER;
    
    QAxBase *object = script->findObject(QString::fromWCharArray(pstrName));
    if (!object)
        return TYPE_E_ELEMENTNOTFOUND;
    
    if (mask & SCRIPTINFO_IUNKNOWN)
        object->queryInterface(IID_IUnknown, (void**)item);
    if (mask & SCRIPTINFO_ITYPEINFO) {
        IProvideClassInfo *classInfo = 0;
        object->queryInterface(IID_IProvideClassInfo, (void**)&classInfo);
        if (classInfo) {
            classInfo->GetClassInfo(type);
            classInfo->Release();
        }
    }
    return S_OK;
}

/*
    Implements IActiveScriptSite::GetDocVersionString

    This method is not implemented. The scripting engine should assume 
    that the script is in sync with the document.
*/
HRESULT WINAPI QAxScriptSite::GetDocVersionString(BSTR * /*version*/)
{
    return E_NOTIMPL;
}

/*
    Implements IActiveScriptSite::OnScriptTerminate

    This method is usually not called, but if it is it fires
    QAxScript::finished().
*/
HRESULT WINAPI QAxScriptSite::OnScriptTerminate(const VARIANT *result, const EXCEPINFO *exception)
{
    emit script->finished();
    
    if (result && result->vt != VT_EMPTY)
        emit script->finished(VARIANTToQVariant(*result, 0));
    if (exception)
        emit script->finished(exception->wCode, 
        QString::fromWCharArray(exception->bstrSource),
        QString::fromWCharArray(exception->bstrDescription),
        QString::fromWCharArray(exception->bstrHelpFile)
			    );
    return S_OK;
}

/*
    Implements IActiveScriptSite::OnEnterScript

    Fires QAxScript::entered() to inform the host that the 
    scripting engine has begun executing the script code.
*/
HRESULT WINAPI QAxScriptSite::OnEnterScript()
{
    emit script->entered();
    return S_OK;
}

/*
    Implements IActiveScriptSite::OnLeaveScript

    Fires QAxScript::finished() to inform the host that the 
    scripting engine has returned from executing the script code.
*/
HRESULT WINAPI QAxScriptSite::OnLeaveScript()
{
    emit script->finished();
    return S_OK;
}

/*
    Implements IActiveScriptSite::OnScriptError

    Fires QAxScript::error() to inform the host that an 
    that an execution error occurred while the engine was running the script.
*/
HRESULT WINAPI QAxScriptSite::OnScriptError(IActiveScriptError *error)
{
    EXCEPINFO exception;
    memset(&exception, 0, sizeof(exception));
    DWORD context;
    ULONG lineNumber;
    LONG charPos;
    BSTR bstrLineText;
    QString lineText;
    
    error->GetExceptionInfo(&exception);
    error->GetSourcePosition(&context, &lineNumber, &charPos);
    HRESULT hres = error->GetSourceLineText(&bstrLineText);
    if (hres == S_OK) {
        lineText = QString::fromWCharArray(bstrLineText);
        SysFreeString(bstrLineText);
    }
    SysFreeString(exception.bstrSource);
    SysFreeString(exception.bstrDescription);
    SysFreeString(exception.bstrHelpFile);

    emit script->error(exception.wCode, QString::fromWCharArray(exception.bstrDescription), lineNumber, lineText);
    
    return S_OK;
}

/*
    Implements IActiveScriptSite::OnStateChange

    Fires QAxScript::stateChanged() to inform the
    the host that the scripting engine has changed states.
*/
HRESULT WINAPI QAxScriptSite::OnStateChange(SCRIPTSTATE ssScriptState)
{
    emit script->stateChanged(ssScriptState);
    return S_OK;
}

/*
    \internal
    Returns the toplevel widget parent of this script, or
    the application' active window if there is no widget parent.
*/
QWidget *QAxScriptSite::window() const
{
    QWidget *w = 0;
    QObject *p = script->parent();
    while (!w && p) {
        w = qobject_cast<QWidget*>(p);
        p = p->parent();
    }
    
    if (w)
        w = w->window();
    if (!w && qApp)
        w = qApp->activeWindow();
    
    return w;
}

/*
    Implements IActiveScriptSiteWindow::GetWindow

    Retrieves the handle to a window that can act as the owner of a 
    pop-up window that the scripting engine must display.
*/
HRESULT WINAPI QAxScriptSite::GetWindow(HWND *phwnd)
{
    if (!phwnd)
        return E_POINTER;
    
    *phwnd = 0;
    QWidget *w = window();
    if (!w)
        return E_FAIL;
    
    *phwnd = w->winId();
    return S_OK;
}

/*
    Implements IActiveScriptSiteWindow::EnableModeless

    Causes the host to enable or disable its main window 
    as well as any modeless dialog boxes.
*/
HRESULT WINAPI QAxScriptSite::EnableModeless(BOOL fEnable)
{
    QWidget *w = window();
    if (!w)
        return E_FAIL;
    
    EnableWindow(w->winId(), fEnable);
    return S_OK;
}

#endif //QT_NO_QAXSCRIPT


/*!
    \class QAxScriptEngine
    \brief The QAxScriptEngine class provides a wrapper around a script engine.
    \inmodule QAxContainer

    Every instance of the QAxScriptEngine class represents an interpreter
    for script code in a particular scripting language. The class is usually
    not used directly. The QAxScript and QAxScriptManager classes provide
    convenient functions to handle and call script code.

    Direct access to the script engine is provided through
    queryInterface().

    \warning This class is not available with the bcc5.5 and MingW
    compilers.

    \sa QAxScript, QAxScriptManager, QAxBase, {ActiveQt Framework}
*/

/*!
    \enum QAxScriptEngine::State

    The State enumeration defines the different states a script
    engine can be in.

    \value Uninitialized The script has been created, but not yet initialized
    \value Initialized The script has been initialized, but is not running
    \value Started The script can execute code, but does not yet handle events
    \value Connected The script can execute code and is connected so
    that it can handle events
    \value Disconnected The script is loaded, but is not connected to
    event sources
    \value Closed The script has been closed.
*/

/*!
    Constructs a QAxScriptEngine object interpreting script code in \a language
    provided by the code in \a script. This is usually done by the QAxScript 
    class when \link QAxScript::load() loading a script\endlink.

    Instances of QAxScriptEngine should always have both a language and a
    script.
*/
QAxScriptEngine::QAxScriptEngine(const QString &language, QAxScript *script)
: QAxObject(script), script_code(script), engine(0), script_language(language)
{
#ifdef QT_CHECK_STATE
    if (language.isEmpty())
        qWarning("QAxScriptEngine: created without language");
    
    if (!script_code)
        qWarning("QAxScriptEngine: created without script");
#endif
    setObjectName(QLatin1String("QAxScriptEngine_") + language);
    disableClassInfo();
    disableEventSink();
}

/*!
    Destroys the QAxScriptEngine object, releasing all allocated
    resources.
*/
QAxScriptEngine::~QAxScriptEngine()
{
#ifndef QT_NO_QAXSCRIPT
    if (engine) {
        engine->SetScriptState(SCRIPTSTATE_DISCONNECTED);
        engine->Close();
        engine->Release();
    }
#endif
}

/*! 
    \fn QString QAxScriptEngine::scriptLanguage() const
    Returns the scripting language, for example "VBScript",
    or "JScript".
*/

/*!
    \reimp
*/
bool QAxScriptEngine::initialize(IUnknown **ptr)
{
    *ptr = 0;
    
#ifndef QT_NO_QAXSCRIPT
    if (!script_code || script_language.isEmpty())
        return false;
    
    CLSID clsid;
    HRESULT hres = CLSIDFromProgID((wchar_t*)script_language.utf16(), &clsid);
    if(FAILED(hres))
        return false;
    
    CoCreateInstance(clsid, 0, CLSCTX_INPROC_SERVER, IID_IActiveScript, (void**)&engine);
    if (!engine)
        return false;
    
    IActiveScriptParse *parser = 0;
    engine->QueryInterface(IID_IActiveScriptParse, (void**)&parser);
    if (!parser) {
        engine->Release();
        engine = 0;
        return false;
    }
    
    if (engine->SetScriptSite(script_code->script_site) != S_OK) {
        engine->Release();
        engine = 0;
        return false;
    }
    if (parser->InitNew() != S_OK) {
        parser->Release();
        engine->Release();
        engine = 0;
        return false;
    }
    
    BSTR bstrCode = QStringToBSTR(script_code->scriptCode());
#ifdef Q_OS_WIN64
    hres = parser->ParseScriptText(bstrCode, 0, 0, 0, DWORDLONG(this), 0, SCRIPTTEXT_ISVISIBLE, 0, 0);
#else
    hres = parser->ParseScriptText(bstrCode, 0, 0, 0, DWORD(this), 0, SCRIPTTEXT_ISVISIBLE, 0, 0);
#endif
    SysFreeString(bstrCode);
    
    parser->Release();
    parser = 0;
    
    script_code->updateObjects();
    
    if (engine->SetScriptState(SCRIPTSTATE_CONNECTED) != S_OK) {
        engine = 0;
        return false;
    }
    
    IDispatch *scriptDispatch = 0;
    engine->GetScriptDispatch(0, &scriptDispatch);
    if (scriptDispatch) {
        scriptDispatch->QueryInterface(IID_IUnknown, (void**)ptr);
        scriptDispatch->Release();
    }
#endif
    
    return *ptr != 0;
}

/*!
    \fn bool QAxScriptEngine::isValid() const

    Returns true if the script engine has been initialized
    correctly; otherwise returns false.
*/

/*!
    Returns true if the script engine supports introspection;
    otherwise returns false.
*/
bool QAxScriptEngine::hasIntrospection() const
{
    if (!isValid())
        return false;
    
    IDispatch *scriptDispatch = 0;
    QAxBase::queryInterface(IID_IDispatch, (void**)&scriptDispatch);
    if (!scriptDispatch)
        return false;
    
    UINT tic = 0;
    HRESULT hres = scriptDispatch->GetTypeInfoCount(&tic);
    scriptDispatch->Release();
    return hres == S_OK && tic > 0;
}

/*!
    Requests the interface \a uuid from the script engine object and
    sets the value of \a iface to the provided interface, or to 0 if
    the requested interface could not be provided.

    Returns the result of the QueryInterface implementation of the COM
    object.
*/
long QAxScriptEngine::queryInterface(const QUuid &uuid, void **iface) const
{
    *iface = 0;
    if (!engine)
        return E_NOTIMPL;
    
#ifndef QT_NO_QAXSCRIPT
    return engine->QueryInterface(uuid, iface);
#else
    Q_UNUSED(uuid)
    return E_NOTIMPL;
#endif
}

/*!
    Returns the state of the script engine.
*/
QAxScriptEngine::State QAxScriptEngine::state() const
{
    if (!engine)
        return Uninitialized;
    
#ifndef QT_NO_QAXSCRIPT
    SCRIPTSTATE state;
    engine->GetScriptState(&state);
    return (State)state;
#else
    return Uninitialized;
#endif
}

/*!
    Sets the state of the script engine to \a st.
    Calling this function is usually not necessary.
*/
void QAxScriptEngine::setState(State st)
{
#ifndef QT_NO_QAXSCRIPT
    if (!engine)
        return;
    
    engine->SetScriptState((SCRIPTSTATE)st);
#else
    Q_UNUSED(st)
#endif
}

/*!
    Registers an item with the script engine. Script code can
    refer to this item using \a name.
*/
void QAxScriptEngine::addItem(const QString &name)
{
#ifndef QT_NO_QAXSCRIPT
    if (!engine)
        return;
    
    engine->AddNamedItem((wchar_t*)name.utf16(), SCRIPTITEM_ISSOURCE|SCRIPTITEM_ISVISIBLE);
#else
    Q_UNUSED(name)
#endif
}

/*!
    \class QAxScript
    \brief The QAxScript class provides a wrapper around script code.
    \inmodule QAxContainer

    Every instance of the QAxScript class represents a piece of
    scripting code in a particular scripting language. The code is
    loaded into the script engine using load(). Functions declared
    in the code can be called using call(). 

    The script provides scriptEngine() provides feedback to the 
    application through signals. The most important signal is the 
    error() signal. Direct access to the QAxScriptEngine is provided
    through the scriptEngine() function.

    \warning This class is not available with the bcc5.5 and MingW
    compilers.

    \sa QAxScriptEngine, QAxScriptManager, QAxBase, {ActiveQt Framework}
*/

/*!
    \enum QAxScript::FunctionFlags

    This FunctionFlags enum describes formatting for function introspection.

    \value FunctionNames Only function names are returned.
    \value FunctionSignatures Returns the functions with signatures.
*/

/*!
    Constructs a QAxScript object called \a name and registers
    it with the QAxScriptManager \a manager. This is usually done by the
    QAxScriptManager class when \link QAxScriptManager::load() loading a 
    script\endlink.

    A script should always have a name. A manager is necessary to allow
    the script code to reference objects in the application. The \a manager
    takes ownership of the object.
*/
QAxScript::QAxScript(const QString &name, QAxScriptManager *manager)
: QObject(manager), script_name(name), script_manager(manager),
script_engine(0)
{
    if (manager) {
        manager->d->scriptDict.insert(name, this);
        connect(this, SIGNAL(error(int,QString,int,QString)), 
            manager, SLOT(scriptError(int,QString,int,QString)));
    }
    
#ifndef QT_NO_QAXSCRIPT
    script_site = new QAxScriptSite(this);
#else
    script_site = 0;
#endif
}

/*!
    Destroys the object, releasing all allocated resources.
*/
QAxScript::~QAxScript()
{
    delete script_engine;
    script_engine = 0;
    
#ifndef QT_NO_QAXSCRIPT
    script_site->Release();
#endif
}

/*!
    Loads the script source \a code written in language \a language
    into the script engine. Returns true if \a code was successfully
    entered into the script engine; otherwise returns false.

    If \a language is empty (the default) it will be determined 
    heuristically. If \a code contains the string \c {End Sub} it will 
    be interpreted as VBScript, otherwise as JScript. Additional 
    scripting languages can be registered using 
    QAxScript::registerEngine().

    This function can only be called once for each QAxScript object,
    which is done automatically when using QAxScriptManager::load().
*/
bool QAxScript::load(const QString &code, const QString &language)
{
    if (script_engine || code.isEmpty())
        return false;
    
    script_code = code;
    QString lang = language;
    if (lang.isEmpty()) {
        if (code.contains(QLatin1String("End Sub"), Qt::CaseInsensitive))
            lang = QLatin1String("VBScript");
        
        QList<QAxEngineDescriptor>::ConstIterator it;
        for (it = engines.begin(); it != engines.end(); ++it) {
            QAxEngineDescriptor engine = *it;
            if (engine.code.isEmpty())
                continue;
            
            if (code.contains(engine.code)) {
                lang = engine.name;
                break;
            }
        }
    }
    if (lang.isEmpty())
        lang = QLatin1String("JScript");
    
    script_engine = new QAxScriptEngine(lang, this);
    // trigger call to initialize
    script_engine->metaObject();
    
    return script_engine->isValid();
}

/*!
    Returns a list of all the functions in this script if the respective
    script engine supports introspection; otherwise returns an empty list.
    The functions are either provided with full prototypes or only as 
    names, depending on the value of \a flags.

    \sa QAxScriptEngine::hasIntrospection()
*/
QStringList QAxScript::functions(FunctionFlags flags) const
{
    QStringList functions;
    
    const QMetaObject *mo = script_engine->metaObject();
    for (int i = mo->methodOffset(); i < mo->methodCount(); ++i) {
       const QMetaMethod slot(mo->method(i));
       if (slot.methodType() != QMetaMethod::Slot || slot.access() != QMetaMethod::Public)
            continue;
        QString slotname = QString::fromLatin1(slot.signature());
        if (slotname.contains(QLatin1Char('_')))
            continue;
        
        if (flags == FunctionSignatures)
            functions << slotname;
        else
            functions << slotname.left(slotname.indexOf(QLatin1Char('(')));
    }
    
    return functions;
}

/*!
    Calls \a function, passing the parameters \a var1, \a var1, 
    \a var2, \a var3, \a var4, \a var5, \a var6, \a var7 and \a var8
    as arguments and returns the value returned by the function, or an 
    invalid QVariant if the function does not return a value or when 
    the function call failed.

    See QAxScriptManager::call() for more information about how to call
    script functions.
*/
QVariant QAxScript::call(const QString &function, const QVariant &var1,
                         const QVariant &var2,
                         const QVariant &var3,
                         const QVariant &var4,
                         const QVariant &var5,
                         const QVariant &var6,
                         const QVariant &var7,
                         const QVariant &var8)
{
    if (!script_engine)
        return QVariant();
    
    return script_engine->dynamicCall(function.toLatin1(), var1, var2, var3, var4, var5, var6, var7, var8);
}

/*!
    \overload

    Calls \a function passing \a arguments as parameters, and returns
    the result. Returns when the script's execution has finished.

    See QAxScriptManager::call() for more information about how to call
    script functions.
*/
QVariant QAxScript::call(const QString &function, QList<QVariant> &arguments)
{
    if (!script_engine)
        return QVariant();
    
    return script_engine->dynamicCall(function.toLatin1(), arguments);
}

/*! \internal
    Registers all objects in the manager with the script engine.
*/
void QAxScript::updateObjects()
{
    if (!script_manager)
        return;
    
    script_manager->updateScript(this);
}

/*! \internal
    Returns the object \a name registered with the manager.
*/
QAxBase *QAxScript::findObject(const QString &name)
{
    if (!script_manager)
        return 0;
    
    return script_manager->d->objectDict.value(name);
}

/*! \fn QString QAxScript::scriptName() const
    Returns the name of the script.
*/

/*! \fn QString QAxScript::scriptCode() const
    Returns the script's code, or the null-string if no
    code has been loaded yet.

    \sa load()
*/

/*! \fn QAxScriptEngine* QAxScript::scriptEngine() const
    Returns a pointer to the script engine.

    You can use the object returned to connect signals to the 
    script functions, or to access the script engine directly.
*/

/*! \fn void QAxScript::entered()

    This signal is emitted when a script engine has started executing code.
*/

/*! \fn void QAxScript::finished()

    This signal is emitted when a script engine has finished executing code.
*/

/*!
    \fn void QAxScript::finished(const QVariant &result)
    \overload

    \a result contains the script's result. This will be an invalid
    QVariant if the script has no return value.
*/

/*! \fn void QAxScript::finished(int code, const QString &source,
                                 const QString &description, const QString &help)
    \overload

    \a code, \a source, \a description and \a help contain exception information
    when the script terminated.
*/

/*! \fn void QAxScript::stateChanged(int state);

    This signal is emitted when a script engine changes state.
    \a state can be any value in the QAxScriptEngineState enumeration.
*/

/*!
    \fn void QAxScript::error(int code, const QString &description,
    int sourcePosition, const QString &sourceText)

    This signal is emitted when an execution error occurred while
    running a script.

    \a code, \a description, \a sourcePosition and \a sourceText
    contain information about the execution error.
*/



/*!
    \class QAxScriptManager
    \brief The QAxScriptManager class provides a bridge between application objects
    and script code.
    \inmodule QAxContainer

    The QAxScriptManager acts as a bridge between the COM objects embedded 
    in the Qt application through QAxObject or QAxWidget, and the scripting 
    languages available through the Windows Script technologies, usually JScript 
    and VBScript.

    Create one QAxScriptManager for each separate document in your
    application, and add the COM objects the scripts need to access
    using addObject(). Then load() the script sources and invoke the
    functions using call().

    \warning This class is not available with the bcc5.5 and MingW
    compilers.

    \sa QAxScript, QAxScriptEngine, QAxBase, {ActiveQt Framework}
*/

/*!
    Creates a QAxScriptManager object. \a parent is passed on to the
    QObject constructor.

    It is usual to create one QAxScriptManager for each document in an
    application.
*/
QAxScriptManager::QAxScriptManager(QObject *parent)
: QObject(parent)
{
    d = new QAxScriptManagerPrivate;
}

/*!
    Destroys the objects, releasing all allocated resources.
*/
QAxScriptManager::~QAxScriptManager()
{
    delete d;
}

/*!
    Returns a list with all the functions that are available.
    Functions provided by script engines that don't support
    introspection are not included in the list.
    The functions are either provided with full prototypes or 
    only as names, depending on the value of \a flags.
*/
QStringList QAxScriptManager::functions(QAxScript::FunctionFlags flags) const
{
    QStringList functions;
    
    QHash<QString, QAxScript*>::ConstIterator scriptIt;
    for (scriptIt = d->scriptDict.begin(); scriptIt != d->scriptDict.end(); ++scriptIt) {
        QAxScript *script = scriptIt.value();
        functions += script->functions(flags);
    }
    
    return functions;
}

/*!
    Returns a list with the names of all the scripts.
*/
QStringList QAxScriptManager::scriptNames() const
{
    QStringList scripts;
    
    QHash<QString, QAxScript*>::ConstIterator scriptIt;
    for (scriptIt = d->scriptDict.begin(); scriptIt != d->scriptDict.end(); ++scriptIt) {
        scripts << scriptIt.key();
    }
    
    return scripts;
}

/*!
    Returns the script called \a name.

    You can use the returned pointer to call functions directly 
    through QAxScript::call(), to access the script engine directly, or
    to delete and thus unload the script.
*/
QAxScript *QAxScriptManager::script(const QString &name) const
{
    return d->scriptDict.value(name);
}

/*!
    Adds \a object to the manager. Scripts handled by this manager
    can access the object in the code using the object's
    \l{QObject::objectName}{objectName} property.

    You must add all the necessary objects before loading any scripts.
*/
void QAxScriptManager::addObject(QAxBase *object)
{
    QObject *obj = object->qObject();
    QString name = obj->objectName();
    if (d->objectDict.contains(name))
        return;
    
    d->objectDict.insert(name, object);
    connect(obj, SIGNAL(destroyed(QObject*)), this, SLOT(objectDestroyed(QObject*)));
}

/*! \fn void QAxScriptManager::addObject(QObject *object)
    \overload

    Adds a generic COM wrapper for \a object to the manager. \a object
    must be exposed as a COM object using the functionality provided
    by the QAxServer module. Applications
    using this function you must link against the qaxserver library.
*/

/*!
    Loads the script source \a code using the script engine for \a
    language. The script can later be referred to using its \a name
    which should not be empty.

    The function returns a pointer to the script for the given
    \a code if the \a code was loaded successfully; otherwise it
    returns 0.

    If \a language is empty it will be determined heuristically. If \a
    code contains the string "End Sub" it will be interpreted as
    VBScript, otherwise as JScript. Additional script engines can be
    registered using registerEngine().

    You must add all the objects necessary (using addObject()) \e
    before loading any scripts. If \a code declares a function that is
    already available (no matter in which language) the first function
    is overloaded and can no longer be called via call(); but it will
    still be available by calling its \link script() script \endlink 
    directly.

    \sa addObject(), scriptNames(), functions()
*/
QAxScript *QAxScriptManager::load(const QString &code, const QString &name, const QString &language)
{
    QAxScript *script = new QAxScript(name, this);
    if (script->load(code, language))
        return script;
    
    delete script;
    return 0;
}

/*!
    \overload

    Loads the source code from the \a file. The script can later be
    referred to using its \a name which should not be empty.

    The function returns a pointer to the script engine for the code
    in \a file if \a file was loaded successfully; otherwise it
    returns 0.

    The script engine used is determined from the file's extension. By
    default ".js" files are interpreted as JScript files, and ".vbs"
    and ".dsm" files are interpreted as VBScript. Additional script
    engines can be registered using registerEngine().
*/
QAxScript *QAxScriptManager::load(const QString &file, const QString &name)
{
    QFile f(file);
    if (!f.open(QIODevice::ReadOnly))
        return 0;
    QByteArray data = f.readAll();
    QString contents = QString::fromLocal8Bit(data, data.size());
    f.close();
    
    if (contents.isEmpty())
        return 0;
    
    QString language;
    if (file.endsWith(QLatin1String(".js"))) {
        language = QLatin1String("JScript");
    } else {
        QList<QAxEngineDescriptor>::ConstIterator it;
        for (it = engines.begin(); it != engines.end(); ++it) {
            QAxEngineDescriptor engine = *it;
            if (engine.extension.isEmpty())
                continue;
            
            if (file.endsWith(engine.extension)) {
                language = engine.name;
                break;
            }
        }
    }
    
    if (language.isEmpty())
        language = QLatin1String("VBScript");
    
    QAxScript *script = new QAxScript(name, this);
    if (script->load(contents, language))
        return script;
    
    delete script;
    return 0;
}

/*!
    Calls \a function, passing the parameters \a var1, \a var1, 
    \a var2, \a var3, \a var4, \a var5, \a var6, \a var7 and \a var8
    as arguments and returns the value returned by the function, or an 
    invalid QVariant if the function does not return a value or when 
    the function call failed. The call returns when the script's
    execution has finished.

    In most script engines the only supported parameter type is "const
    QVariant&", for example, to call a JavaScript function
    \snippet doc/src/snippets/code/src_activeqt_container_qaxscript.cpp 0
    use
    \snippet doc/src/snippets/code/src_activeqt_container_qaxscript.cpp 1
    As with \link QAxBase::dynamicCall() dynamicCall \endlink the 
    parameters can directly be embedded in the function string.
    \snippet doc/src/snippets/code/src_activeqt_container_qaxscript.cpp 2
    However, this is slower.

    Functions provided by script engines that don't support
    introspection are not available and must be called directly
    using QAxScript::call() on the respective \link script() 
    script \endlink object.

    Note that calling this function can be significantely slower than
    using call() on the respective QAxScript directly.
*/
QVariant QAxScriptManager::call(const QString &function, const QVariant &var1,
                                const QVariant &var2,
                                const QVariant &var3,
                                const QVariant &var4,
                                const QVariant &var5,
                                const QVariant &var6,
                                const QVariant &var7,
                                const QVariant &var8)
{
    QAxScript *s = script(function);
    if (!s) {
#ifdef QT_CHECK_STATE
        qWarning("QAxScriptManager::call: No script provides function %s, or this function\n"
            "\tis provided through an engine that does not support introspection", function.latin1());
#endif
        return QVariant();
    }
    
    return s->call(function, var1, var2, var3, var4, var5, var6, var7, var8);
}

/*! \overload

    Calls \a function passing \a arguments as parameters, and returns
    the result. Returns when the script's execution has finished.
*/
QVariant QAxScriptManager::call(const QString &function, QList<QVariant> &arguments)
{
    QAxScript *s = script(function);
    if (!s) {
#ifdef QT_CHECK_STATE
        qWarning("QAxScriptManager::call: No script provides function %s, or this function\n"
            "\tis provided through an engine that does not support introspection", function.latin1());
#endif
        return QVariant();
    }
    
    QList<QVariant> args(arguments);
    return s->call(function, args);
}

/*!
    Registers the script engine called \a name and returns true if the
    engine was found; otherwise does nothing and returns false.

    The script engine will be used when loading files with the given
    \a extension, or when loading source code that contains the string
    \a code.
*/
bool QAxScriptManager::registerEngine(const QString &name, const QString &extension, const QString &code)
{
    if (name.isEmpty())
        return false;
    
    CLSID clsid;
    HRESULT hres = CLSIDFromProgID((wchar_t*)name.utf16(), &clsid);
    if (hres != S_OK)
        return false;
    
    QAxEngineDescriptor engine;
    engine.name = name;
    engine.extension = extension;
    engine.code = code;
    
    engines.prepend(engine);
    return true;
}

/*!
    Returns a file filter listing all the supported script languages.
    This filter string is convenient for use with QFileDialog.
*/
QString QAxScriptManager::scriptFileFilter()
{
    QString allFiles = QLatin1String("Script Files (*.js *.vbs *.dsm");
    QString specialFiles = QLatin1String(";;VBScript Files (*.vbs *.dsm)"
        ";;JavaScript Files (*.js)");
    
    QList<QAxEngineDescriptor>::ConstIterator it;
    for (it = engines.begin(); it != engines.end(); ++it) {
        QAxEngineDescriptor engine = *it;
        if (engine.extension.isEmpty())
            continue;
        
        allFiles += QLatin1String(" *") + engine.extension;
        specialFiles += QLatin1String(";;") + engine.name + QLatin1String(" Files (*") + engine.extension + QLatin1Char(')');
    }
    allFiles += QLatin1Char(')');
    
    return allFiles + specialFiles + QLatin1String(";;All Files (*.*)");
}

/*!
    \fn void QAxScriptManager::error(QAxScript *script, int code, const QString &description,
    int sourcePosition, const QString &sourceText)

    This signal is emitted when an execution error occurred while
    running \a script.

    \a code, \a description, \a sourcePosition and \a sourceText
    contain information about the execution error.

    \warning Do not delete \a script in a slot connected to this signal. Use deleteLater()
    instead.
*/

/*!
    \internal

    Returns a pointer to the first QAxScript that knows 
    about \a function, or 0 if this function is unknown.
*/
QAxScript *QAxScriptManager::scriptForFunction(const QString &function) const
{
    // check full prototypes if included
    if (function.contains(QLatin1Char('('))) {
        QHash<QString, QAxScript*>::ConstIterator scriptIt;
        for (scriptIt = d->scriptDict.begin(); scriptIt != d->scriptDict.end(); ++scriptIt) {
            QAxScript *script = scriptIt.value();
            
            if (script->functions(QAxScript::FunctionSignatures).contains(function))
                return script;
        }
    }
    
    QString funcName = function;
    funcName = funcName.left(funcName.indexOf(QLatin1Char('(')));
    // second try, checking only names, not prototypes
    QHash<QString, QAxScript*>::ConstIterator scriptIt;
    for (scriptIt = d->scriptDict.begin(); scriptIt != d->scriptDict.end(); ++scriptIt) {
        QAxScript *script = scriptIt.value();
        
        if (script->functions(QAxScript::FunctionNames).contains(funcName))
            return script;
    }
    
    return 0;
}

/*!
    \internal
*/
void QAxScriptManager::updateScript(QAxScript *script)
{
    QHash<QString, QAxBase*>::ConstIterator objectIt;
    for (objectIt = d->objectDict.constBegin(); objectIt != d->objectDict.constEnd(); ++objectIt) {
        QString name = objectIt.key();
        
        QAxScriptEngine *engine = script->scriptEngine();
        if (engine)
            engine->addItem(name);
    }
}

/*!
    \internal
*/
void QAxScriptManager::objectDestroyed(QObject *o)
{
    d->objectDict.take(o->objectName());
}

/*!
    \internal
*/
void QAxScriptManager::scriptError(int code, const QString &desc, int spos, const QString &stext)
{
    QAxScript *source = qobject_cast<QAxScript*>(sender());
    emit error(source, code, desc, spos, stext);
}

QT_END_NAMESPACE
#endif // QT_NO_WIN_ACTIVEQT
