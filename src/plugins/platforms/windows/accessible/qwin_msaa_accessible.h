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

#ifndef QWINDOWS_MSAA_ACCESSIBLE_H
#define QWINDOWS_MSAA_ACCESSIBLE_H

#include <qglobal.h>

#ifndef QT_NO_ACCESSIBILITY

#include <qwin_additional.h>
#include <qsharedpointer.h>
#include <qaccessible.h>

#include <basetyps.h>
#include <oleacc.h>

#if defined(CS_SHOW_DEBUG_PLATFORM)
void accessibleDebugClientCalls_helper(const char* funcName, const QAccessibleInterface *iface);
# define accessibleDebugClientCalls(iface)   accessibleDebugClientCalls_helper(Q_FUNC_INFO, iface)

#else
# define accessibleDebugClientCalls(iface)   (void) iface

#endif

QWindow *window_helper(const QAccessibleInterface *iface);

class QWindowsMsaaAccessible : public IAccessible, public IOleWindow
{
public:
    QWindowsMsaaAccessible(QAccessibleInterface *a)
        : ref(0)
    {
        id = QAccessible::uniqueId(a);
    }

    virtual ~QWindowsMsaaAccessible()
    {
    }

    /* IUnknown */
    HRESULT STDMETHODCALLTYPE QueryInterface(REFIID, LPVOID *) override;
    ULONG STDMETHODCALLTYPE AddRef() override;
    ULONG STDMETHODCALLTYPE Release() override;

    /* IDispatch */
    HRESULT STDMETHODCALLTYPE GetTypeInfoCount(unsigned int *) override;
    HRESULT STDMETHODCALLTYPE GetTypeInfo(unsigned int, unsigned long, ITypeInfo **) override;
    HRESULT STDMETHODCALLTYPE GetIDsOfNames(const _GUID &, wchar_t **, unsigned int, unsigned long, long *) override;
    HRESULT STDMETHODCALLTYPE Invoke(long, const _GUID &, unsigned long, unsigned short, tagDISPPARAMS *,
                  tagVARIANT *, tagEXCEPINFO *, unsigned int *) override;

    /* IAccessible */
    HRESULT STDMETHODCALLTYPE accHitTest(long xLeft, long yTop, VARIANT *pvarID) override;
    HRESULT STDMETHODCALLTYPE accLocation(long *pxLeft, long *pyTop, long *pcxWidth, long *pcyHeight, VARIANT varID) override;
    HRESULT STDMETHODCALLTYPE accNavigate(long navDir, VARIANT varStart, VARIANT *pvarEnd) override;
    HRESULT STDMETHODCALLTYPE get_accChild(VARIANT varChildID, IDispatch** ppdispChild) override;
    HRESULT STDMETHODCALLTYPE get_accChildCount(long* pcountChildren) override;
    HRESULT STDMETHODCALLTYPE get_accParent(IDispatch** ppdispParent) override;

    HRESULT STDMETHODCALLTYPE accDoDefaultAction(VARIANT varID) override;
    HRESULT STDMETHODCALLTYPE get_accDefaultAction(VARIANT varID, BSTR* pszDefaultAction) override;
    HRESULT STDMETHODCALLTYPE get_accDescription(VARIANT varID, BSTR* pszDescription) override;
    HRESULT STDMETHODCALLTYPE get_accHelp(VARIANT varID, BSTR *pszHelp) override;
    HRESULT STDMETHODCALLTYPE get_accHelpTopic(BSTR *pszHelpFile, VARIANT varChild, long *pidTopic) override;
    HRESULT STDMETHODCALLTYPE get_accKeyboardShortcut(VARIANT varID, BSTR *pszKeyboardShortcut) override;
    HRESULT STDMETHODCALLTYPE get_accName(VARIANT varID, BSTR* pszName) override;
    HRESULT STDMETHODCALLTYPE put_accName(VARIANT varChild, BSTR szName) override;
    HRESULT STDMETHODCALLTYPE get_accRole(VARIANT varID, VARIANT *pvarRole) override;
    HRESULT STDMETHODCALLTYPE get_accState(VARIANT varID, VARIANT *pvarState) override;
    HRESULT STDMETHODCALLTYPE get_accValue(VARIANT varID, BSTR* pszValue) override;
    HRESULT STDMETHODCALLTYPE put_accValue(VARIANT varChild, BSTR szValue) override;

    HRESULT STDMETHODCALLTYPE accSelect(long flagsSelect, VARIANT varID) override;
    HRESULT STDMETHODCALLTYPE get_accFocus(VARIANT *pvarID) override;
    HRESULT STDMETHODCALLTYPE get_accSelection(VARIANT *pvarChildren) override;

    /* IOleWindow */
    HRESULT STDMETHODCALLTYPE GetWindow(HWND *phwnd) override;
    HRESULT STDMETHODCALLTYPE ContextSensitiveHelp(BOOL fEnterMode) override;

protected:
    virtual QByteArray IIDToString(REFIID id);

    QAccessible::Id id;

    QAccessibleInterface *accessibleInterface() const
    {
         QAccessibleInterface *iface = QAccessible::accessibleInterface(id);
         if (iface && iface->isValid())
             return iface;
         return nullptr;
    }

    static QAccessibleInterface *childPointer(QAccessibleInterface *parent, VARIANT varID)
    {
        // -1 since windows API always uses 1 for the first child
        Q_ASSERT(parent);

        QAccessibleInterface *acc = nullptr;
        int childIndex = varID.lVal;

        if (childIndex == 0) {
            // Yes, some AT clients (Active Accessibility Object Inspector)
            // actually ask for the same object. As a consequence, we need to clone ourselves:
            acc = parent;

        } else if (childIndex < 0) {
            acc = QAccessible::accessibleInterface((QAccessible::Id)childIndex);
        } else {
            acc = parent->child(childIndex - 1);
        }

        return acc;
    }

private:
    ULONG ref;

};

#endif

#endif
