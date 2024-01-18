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

#include <qglobal.h>

#ifndef QT_NO_ACCESSIBILITY

#include <qwin_accessibility.h>

#include <qaccessible.h>
#include <qapplication.h>
#include <qlocale.h>
#include <qmap.h>
#include <qpair.h>
#include <qpointer.h>
#include <qsettings.h>
#include <qplatform_nativeinterface.h>
#include <qplatform_integration.h>
#include <qwindow.h>
#include <qwin_additional.h>
#include <qwin_msaa_accessible.h>

#include <qapplication_p.h>
#include <qsystemlibrary_p.h>

#include <comutils.h>

#ifndef UiaRootObjectId
#define UiaRootObjectId        -25
#endif

#include <winuser.h>

#if ! defined(WINABLEAPI)
#include <winable.h>
#endif

#include <servprov.h>

#if ! defined (Q_CC_GNU)
#include <comdef.h>
#endif

QWindowsAccessibility::QWindowsAccessibility()
{
}

// Retrieve sound name by checking the icon property of a message box
static inline QString messageBoxAlertSound(const QObject *messageBox)
{
    enum MessageBoxIcon { // Keep in sync with QMessageBox::Icon
        Information = 1,
        Warning = 2,
        Critical = 3
    };

    switch (messageBox->property("icon").toInt()) {
    case Information:
        return QString("SystemAsterisk");
    case Warning:
        return QString("SystemExclamation");
    case Critical:
        return QString("SystemHand");
    }

    return QString();
}

void QWindowsAccessibility::notifyAccessibilityUpdate(QAccessibleEvent *event)
{
    QString soundName;

    switch (event->type()) {
       case QAccessible::PopupMenuStart:
           soundName = "MenuPopup";
           break;

       case QAccessible::MenuCommand:
           soundName = "MenuCommand";
           break;

       case QAccessible::Alert:
           soundName = event->object()->inherits("QMessageBox") ?
               messageBoxAlertSound(event->object()) : QString("SystemAsterisk");
           break;

       default:
           break;
    }

    if (! soundName.isEmpty()) {

#ifndef QT_NO_SETTINGS
        QSettings settings("HKEY_CURRENT_USER\\AppEvents\\Schemes\\Apps\\.Default\\" + soundName, QSettings::NativeFormat);
        QString file = settings.value(".Current/.").toString();
#else
        QString file;
#endif

        if (! file.isEmpty()) {
            std::wstring tmp = soundName.toStdWString();
            PlaySound(tmp.data(), nullptr, SND_ALIAS | SND_ASYNC | SND_NODEFAULT | SND_NOWAIT);
        }
    }

    // An event has to be associated with a window,
    // so find the first parent that is a widget and that has a WId
    QAccessibleInterface *iface = event->accessibleInterface();

    if (! isActive() || !iface || ! iface->isValid())
        return;

    QWindow *window = QWindowsAccessibility::windowHelper(iface);

    if (!window) {
        window = QApplication::focusWindow();
        if (!window)
            return;
    }

    QPlatformNativeInterface *platform = QApplication::platformNativeInterface();
    if (!window->handle()) // Called before show(), no native window yet.
        return;

    const HWND hWnd = reinterpret_cast<HWND>(platform->nativeResourceForWindow("handle", window));

    if (event->type() != QAccessible::MenuCommand && // MenuCommand is faked
        event->type() != QAccessible::ObjectDestroyed) {
        ::NotifyWinEvent(event->type(), hWnd, OBJID_CLIENT, QAccessible::uniqueId(iface));
    }

}

QWindow *QWindowsAccessibility::windowHelper(const QAccessibleInterface *iface)
{
    QWindow *window = iface->window();
    if (!window) {
        QAccessibleInterface *acc = iface->parent();
        while (acc && acc->isValid() && !window) {
            window = acc->window();
            QAccessibleInterface *par = acc->parent();
            acc = par;
        }
    }
    return window;
}

/*!
  \internal
  helper to wrap a QAccessibleInterface inside a IAccessible*
*/
IAccessible *QWindowsAccessibility::wrap(QAccessibleInterface *acc)
{
    if (! acc)
        return nullptr;

    // ### FIXME: maybe we should accept double insertions into the cache
    if (!QAccessible::uniqueId(acc))
        QAccessible::registerAccessibleInterface(acc);

    QWindowsMsaaAccessible *wacc = new QWindowsMsaaAccessible(acc);

    IAccessible *iacc = nullptr;
    wacc->QueryInterface(IID_IAccessible, reinterpret_cast<void **>(&iacc));

    return iacc;
}

bool QWindowsAccessibility::handleAccessibleObjectFromWindowRequest(HWND hwnd, WPARAM wParam, LPARAM lParam, LRESULT *lResult)
{
    if (static_cast<long>(lParam) == static_cast<long>(UiaRootObjectId)) {
        /* For UI Automation */

    } else if (DWORD(lParam) == DWORD(OBJID_CLIENT)) {
        // Start handling accessibility internally
        QApplicationPrivate::platformIntegration()->accessibility()->setActive(true);

        // Ignoring all requests while starting up
        if (QCoreApplication::startingUp() || QCoreApplication::closingDown())
            return false;

        typedef LRESULT (WINAPI *PtrLresultFromObject)(REFIID, WPARAM, LPUNKNOWN);
        static PtrLresultFromObject ptrLresultFromObject = nullptr;
        static bool oleaccChecked = false;

        if (! oleaccChecked) {
            oleaccChecked = true;
            ptrLresultFromObject = reinterpret_cast<PtrLresultFromObject>
                  (QSystemLibrary::resolve(QString("oleacc"), "LresultFromObject"));
        }

        if (ptrLresultFromObject) {
            QWindow *window = QWindowsContext::instance()->findWindow(hwnd);

            if (window) {
                QAccessibleInterface *acc = window->accessibleRoot();
                if (acc) {
                    if (IAccessible *iface = wrap(acc)) {
                        *lResult = ptrLresultFromObject(IID_IAccessible, wParam, iface);  // ref == 2
                        if (*lResult) {
                            iface->Release(); // the client will release the object again, and then it will destroy itself
                        }
                        return true;
                    }
                }
            }
        }
    }

    return false;
}

#endif //QT_NO_ACCESSIBILITY
