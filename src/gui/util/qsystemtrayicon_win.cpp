/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
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

#include <qsystemtrayicon_p.h>

#ifndef QT_NO_SYSTEMTRAYICON

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600         // Windows Vista
#endif

#if defined(_WIN32_IE) && _WIN32_IE < 0x0600
#  undef _WIN32_IE
#endif

#ifndef _WIN32_IE
#define _WIN32_IE 0x0600             //required for NOTIFYICONDATA_V2_SIZE
#endif

#include <qplatform_nativeinterface.h>
#include <QSettings>
#include <QDebug>
#include <QHash>

#include <qt_windows.h>

#include <qsystemlibrary_p.h>
#include <qguiapplication_p.h>

#include <commctrl.h>
#include <windowsx.h>

static const UINT q_uNOTIFYICONID = 0;

static uint MYWM_TASKBARCREATED = 0;
#define MYWM_NOTIFYICON (WM_APP+101)

struct Q_NOTIFYICONIDENTIFIER {
   DWORD cbSize;
   HWND hWnd;
   UINT uID;
   GUID guidItem;
};

#ifndef NOTIFYICON_VERSION_4
#define NOTIFYICON_VERSION_4 4
#endif

#ifndef NIN_SELECT
#define NIN_SELECT (WM_USER + 0)
#endif

#ifndef NIN_KEYSELECT
#define NIN_KEYSELECT (WM_USER + 1)
#endif

#ifndef NIN_BALLOONTIMEOUT
#define NIN_BALLOONTIMEOUT (WM_USER + 4)
#endif

#ifndef NIN_BALLOONUSERCLICK
#define NIN_BALLOONUSERCLICK (WM_USER + 5)
#endif

#ifndef NIF_SHOWTIP
#define NIF_SHOWTIP 0x00000080
#endif

#define Q_MSGFLT_ALLOW 1

typedef HRESULT (WINAPI *PtrShell_NotifyIconGetRect)(const Q_NOTIFYICONIDENTIFIER *identifier, RECT *iconLocation);
typedef BOOL (WINAPI *PtrChangeWindowMessageFilter)(UINT message, DWORD dwFlag);
typedef BOOL (WINAPI *PtrChangeWindowMessageFilterEx)(HWND hWnd, UINT message, DWORD action, void *pChangeFilterStruct);

static void cs_internal_StringToArray(const QString &str, wchar_t *output, int length)
{
   std::wstring tmp = str.toStdWString();
   std::wcsncpy(output, &tmp[0], length);
   output[length - 1] = 0;
}

class QSystemTrayIconSys
{
 public:
   QSystemTrayIconSys(HWND hwnd, QSystemTrayIcon *object);
   ~QSystemTrayIconSys();

   bool winEvent(MSG *m, long *result);
   bool trayMessage(DWORD msg);
   void setIconContents(NOTIFYICONDATA &data);
   bool showMessage(const QString &title, const QString &message, QSystemTrayIcon::MessageIcon type, uint uSecs);
   QRect findIconGeometry(UINT iconId);
   HICON createIcon();

 private:
   const HWND m_hwnd;
   HICON hIcon;
   QPoint globalPos;
   QSystemTrayIcon *q;

   uint notifyIconSize;

   int version;
   bool ignoreNextMouseRelease;
};

static bool allowsMessages()
{
#ifndef QT_NO_SETTINGS
   const QString key = "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced";
   const QSettings settings(key, QSettings::NativeFormat);

   return settings.value("EnableBalloonTips", true).toBool();

#else
   return false;

#endif
}

typedef QHash<HWND, QSystemTrayIconSys *> HandleTrayIconHash;

Q_GLOBAL_STATIC(HandleTrayIconHash, handleTrayIconHash)

extern "C" LRESULT QT_WIN_CALLBACK qWindowsTrayconWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
   if (message == MYWM_TASKBARCREATED || message == MYWM_NOTIFYICON) {
      if (QSystemTrayIconSys *trayIcon = handleTrayIconHash()->value(hwnd)) {
         MSG msg;
         msg.hwnd = hwnd;         // re-create MSG structure
         msg.message = message;   // time and pt fields ignored
         msg.wParam = wParam;
         msg.lParam = lParam;
         msg.pt.x = GET_X_LPARAM(lParam);
         msg.pt.y = GET_Y_LPARAM(lParam);
         long result = 0;
         if (trayIcon->winEvent(&msg, &result)) {
            return result;
         }
      }
   }
   return DefWindowProc(hwnd, message, wParam, lParam);
}

// Invoke a service of the native Windows interface to create
// a non-visible toplevel window to receive tray messages.
// Note: Message windows (HWND_MESSAGE) are not sufficient, they
// will not receive the "TaskbarCreated" message.
static inline HWND createTrayIconMessageWindow()
{
   QPlatformNativeInterface *ni = QGuiApplication::platformNativeInterface();
   if (! ni) {
      return 0;
   }

   // Register window class in the platform plugin.
   QString className;
   void *wndProc = reinterpret_cast<void *>(qWindowsTrayconWndProc);

   if (! QMetaObject::invokeMethod(ni, "registerWindowClass", Qt::DirectConnection, Q_RETURN_ARG(QString, className),
                  Q_ARG(const QString &, "QTrayIconMessageWindowClass"), Q_ARG(void *, wndProc))) {

      return 0;
   }

   const wchar_t windowName[] = L"QTrayIconMessageWindow";
   std::wstring tmp = className.toStdWString();

   return CreateWindowEx(0, &tmp[0], windowName, WS_OVERLAPPED, CW_USEDEFAULT, CW_USEDEFAULT,
         CW_USEDEFAULT, CW_USEDEFAULT, NULL, NULL, (HINSTANCE)GetModuleHandle(0), NULL);
}

QSystemTrayIconSys::QSystemTrayIconSys(HWND hwnd, QSystemTrayIcon *object)
   : m_hwnd(hwnd), hIcon(0), q(object)
   , notifyIconSize(NOTIFYICONDATA_V2_SIZE), version(NOTIFYICON_VERSION)
   , ignoreNextMouseRelease(false)

{
   handleTrayIconHash()->insert(m_hwnd, this);

   if (QSysInfo::windowsVersion() < QSysInfo::WV_VISTA) {
      notifyIconSize = NOTIFYICONDATA_V2_SIZE;
      version = NOTIFYICON_VERSION;
   }

   // For restoring the tray icon after explorer crashes
   if (!MYWM_TASKBARCREATED) {
      MYWM_TASKBARCREATED = RegisterWindowMessage(L"TaskbarCreated");
   }

   // Allow the WM_TASKBARCREATED message through the UIPI filter on Windows Vista and higher
   static PtrChangeWindowMessageFilterEx pChangeWindowMessageFilterEx =
      (PtrChangeWindowMessageFilterEx)QSystemLibrary::resolve(QLatin1String("user32"), "ChangeWindowMessageFilterEx");

   if (pChangeWindowMessageFilterEx) {
      // Call the safer ChangeWindowMessageFilterEx API if available
      pChangeWindowMessageFilterEx(m_hwnd, MYWM_TASKBARCREATED, Q_MSGFLT_ALLOW, 0);
   } else {
      static PtrChangeWindowMessageFilter pChangeWindowMessageFilter =
         (PtrChangeWindowMessageFilter)QSystemLibrary::resolve(QLatin1String("user32"), "ChangeWindowMessageFilter");

      if (pChangeWindowMessageFilter) {
         // Call the deprecated ChangeWindowMessageFilter API otherwise
         pChangeWindowMessageFilter(MYWM_TASKBARCREATED, Q_MSGFLT_ALLOW);
      }
   }
}

QSystemTrayIconSys::~QSystemTrayIconSys()
{
   handleTrayIconHash()->remove(m_hwnd);
   if (hIcon) {
      DestroyIcon(hIcon);
   }
   DestroyWindow(m_hwnd);
}

void QSystemTrayIconSys::setIconContents(NOTIFYICONDATA &tnd)
{
   tnd.uFlags |= NIF_MESSAGE | NIF_ICON | NIF_TIP;
   tnd.uCallbackMessage = MYWM_NOTIFYICON;
   tnd.hIcon = hIcon;

   const QString tip = q->toolTip();

   if (! tip.isEmpty()) {
      cs_internal_StringToArray(tip, tnd.szTip, sizeof(tnd.szTip) / sizeof(wchar_t));
   }
}

#ifndef NIIF_LARGE_ICON
#  define NIIF_LARGE_ICON 0x00000020
#endif

bool QSystemTrayIconSys::showMessage(const QString &title, const QString &message, QSystemTrayIcon::MessageIcon type, uint uSecs)
{
   NOTIFYICONDATA tnd;
   memset(&tnd, 0, notifyIconSize);

   cs_internal_StringToArray(message, tnd.szInfo, 256);
   cs_internal_StringToArray(title, tnd.szInfoTitle, 64);

   tnd.uID = q_uNOTIFYICONID;
   switch (type) {
      case QSystemTrayIcon::Information:
         tnd.dwInfoFlags = NIIF_INFO;
         break;
      case QSystemTrayIcon::Warning:
         tnd.dwInfoFlags = NIIF_WARNING;
         break;
      case QSystemTrayIcon::Critical:
         tnd.dwInfoFlags = NIIF_ERROR;
         break;
      case QSystemTrayIcon::NoIcon:
         tnd.dwInfoFlags = hIcon ? NIIF_USER : NIIF_NONE;
         break;
   }
   if (QSysInfo::windowsVersion() >= QSysInfo::WV_VISTA) {
      tnd.dwInfoFlags |= NIIF_LARGE_ICON;
   }
   tnd.cbSize = notifyIconSize;
   tnd.hWnd = m_hwnd;
   tnd.uTimeout = uSecs;
   tnd.uFlags = NIF_INFO | NIF_SHOWTIP;

   return Shell_NotifyIcon(NIM_MODIFY, &tnd);
}

bool QSystemTrayIconSys::trayMessage(DWORD msg)
{
   NOTIFYICONDATA tnd;
   memset(&tnd, 0, notifyIconSize);

   tnd.uID = q_uNOTIFYICONID;
   tnd.cbSize = notifyIconSize;
   tnd.hWnd = m_hwnd;
   tnd.uFlags = NIF_SHOWTIP;
   tnd.uVersion = version;

   if (msg == NIM_ADD || msg == NIM_MODIFY) {
      setIconContents(tnd);
   }

   bool success = Shell_NotifyIcon(msg, &tnd);

   if (msg == NIM_ADD) {
      return success && Shell_NotifyIcon(NIM_SETVERSION, &tnd);
   } else {
      return success;
   }
}
Q_GUI_EXPORT HICON qt_pixmapToWinHICON(const QPixmap &);

HICON QSystemTrayIconSys::createIcon()
{
   const HICON oldIcon = hIcon;
   hIcon = 0;
   const QIcon icon = q->icon();
   if (icon.isNull()) {
      return oldIcon;
   }
   const QSize requestedSize = QSysInfo::windowsVersion() >= QSysInfo::WV_VISTA
      ? QSize(GetSystemMetrics(SM_CXICON), GetSystemMetrics(SM_CYICON))
      : QSize(GetSystemMetrics(SM_CXSMICON), GetSystemMetrics(SM_CYSMICON));
   const QSize size = icon.actualSize(requestedSize);
   const QPixmap pm = icon.pixmap(size);
   if (pm.isNull()) {
      return oldIcon;
   }
   hIcon = qt_pixmapToWinHICON(pm);
   return oldIcon;
}

bool QSystemTrayIconSys::winEvent( MSG *m, long *result )
{
   *result = 0;
   switch (m->message) {
      case MYWM_NOTIFYICON: {
         int message = 0;
         QPoint gpos;

         if (version == NOTIFYICON_VERSION_4) {
            Q_ASSERT(q_uNOTIFYICONID == HIWORD(m->lParam));
            message = LOWORD(m->lParam);
            gpos = QPoint(GET_X_LPARAM(m->wParam), GET_Y_LPARAM(m->wParam));
         } else {
            Q_ASSERT(q_uNOTIFYICONID == m->wParam);
            message = m->lParam;
            gpos = QCursor::pos();
         }

         switch (message) {
            case NIN_SELECT:
            case NIN_KEYSELECT:
               if (ignoreNextMouseRelease) {
                  ignoreNextMouseRelease = false;
               } else {
                  emit q->activated(QSystemTrayIcon::Trigger);
               }
               break;

            case WM_LBUTTONDBLCLK:
               ignoreNextMouseRelease = true; // Since DBLCLICK Generates a second mouse
               // release we must ignore it
               emit q->activated(QSystemTrayIcon::DoubleClick);
               break;

            case WM_CONTEXTMENU:
               if (q->contextMenu()) {
                  q->contextMenu()->popup(gpos);
                  q->contextMenu()->activateWindow();
               }
               emit q->activated(QSystemTrayIcon::Context);
               break;

            case NIN_BALLOONUSERCLICK:
               emit q->messageClicked();
               break;

            case WM_MBUTTONUP:
               emit q->activated(QSystemTrayIcon::MiddleClick);
               break;

            default:
               break;
         }
         break;
      }

      default:
         if (m->message == MYWM_TASKBARCREATED) {
            trayMessage(NIM_ADD);
         }

         break;
   }

   return false;
}
QSystemTrayIconPrivate::QSystemTrayIconPrivate()
   : sys(0),
     visible(false)
{
}
QSystemTrayIconPrivate::~QSystemTrayIconPrivate()
{
}

void QSystemTrayIconPrivate::install_sys()
{
   Q_Q(QSystemTrayIcon);
   if (!sys) {
      if (const HWND hwnd = createTrayIconMessageWindow()) {
         sys = new QSystemTrayIconSys(hwnd, q);
         sys->createIcon();
         sys->trayMessage(NIM_ADD);
      } else {
         qWarning("The platform plugin failed to create a message window.");
      }
   }
}

/*
* This function tries to determine the icon geometry from the tray
*
* If it fails an invalid rect is returned.
*/
QRect QSystemTrayIconSys::findIconGeometry(UINT iconId)
{
   struct AppData {
      HWND hwnd;
      UINT uID;
   };

   static PtrShell_NotifyIconGetRect Shell_NotifyIconGetRect =
      (PtrShell_NotifyIconGetRect)QSystemLibrary::resolve(QLatin1String("shell32"), "Shell_NotifyIconGetRect");

   if (Shell_NotifyIconGetRect) {
      Q_NOTIFYICONIDENTIFIER nid;
      memset(&nid, 0, sizeof(nid));
      nid.cbSize = sizeof(nid);
      nid.hWnd = m_hwnd;
      nid.uID = iconId;

      RECT rect;
      HRESULT hr = Shell_NotifyIconGetRect(&nid, &rect);
      if (SUCCEEDED(hr)) {
         return QRect(rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top);
      }
   }

   QRect ret;

   TBBUTTON buttonData;
   DWORD processID = 0;
   HWND trayHandle = FindWindow(L"Shell_TrayWnd", NULL);

   //find the toolbar used in the notification area
   if (trayHandle) {
      trayHandle = FindWindowEx(trayHandle, NULL, L"TrayNotifyWnd", NULL);
      if (trayHandle) {
         HWND hwnd = FindWindowEx(trayHandle, NULL, L"SysPager", NULL);
         if (hwnd) {
            hwnd = FindWindowEx(hwnd, NULL, L"ToolbarWindow32", NULL);
            if (hwnd) {
               trayHandle = hwnd;
            }
         }
      }
   }

   if (!trayHandle) {
      return ret;
   }

   GetWindowThreadProcessId(trayHandle, &processID);
   if (processID <= 0) {
      return ret;
   }

   HANDLE trayProcess = OpenProcess(PROCESS_VM_OPERATION | PROCESS_VM_READ, 0, processID);
   if (!trayProcess) {
      return ret;
   }

   int buttonCount = SendMessage(trayHandle, TB_BUTTONCOUNT, 0, 0);
   LPVOID data = VirtualAllocEx(trayProcess, NULL, sizeof(TBBUTTON), MEM_COMMIT, PAGE_READWRITE);

   if ( buttonCount < 1 || !data ) {
      CloseHandle(trayProcess);
      return ret;
   }

   //search for our icon among all toolbar buttons
   for (int toolbarButton = 0; toolbarButton  < buttonCount; ++toolbarButton ) {
      SIZE_T numBytes = 0;
      AppData appData = { 0, 0 };
      SendMessage(trayHandle, TB_GETBUTTON, toolbarButton, (LPARAM)data);

      if (!ReadProcessMemory(trayProcess, data, &buttonData, sizeof(TBBUTTON), &numBytes)) {
         continue;
      }

      if (!ReadProcessMemory(trayProcess, (LPVOID) buttonData.dwData, &appData, sizeof(AppData), &numBytes)) {
         continue;
      }

      bool isHidden = buttonData.fsState & TBSTATE_HIDDEN;

      if (m_hwnd == appData.hwnd && appData.uID == iconId && !isHidden) {
         SendMessage(trayHandle, TB_GETITEMRECT, toolbarButton, (LPARAM)data);
         RECT iconRect = {0, 0, 0, 0};
         if (ReadProcessMemory(trayProcess, data, &iconRect, sizeof(RECT), &numBytes)) {
            MapWindowPoints(trayHandle, NULL, (LPPOINT)&iconRect, 2);
            QRect geometry(iconRect.left + 1, iconRect.top + 1,
               iconRect.right - iconRect.left - 2,
               iconRect.bottom - iconRect.top - 2);
            if (geometry.isValid()) {
               ret = geometry;
            }
            break;
         }
      }
   }
   VirtualFreeEx(trayProcess, data, 0, MEM_RELEASE);
   CloseHandle(trayProcess);
   return ret;
}

void QSystemTrayIconPrivate::showMessage_sys(const QString &title, const QString &message,
   QSystemTrayIcon::MessageIcon type, int timeOut)
{
   if (! sys || ! allowsMessages()) {
      return;
   }

   uint uSecs = 0;
   if ( timeOut < 0) {
      uSecs = 10000;   //10 sec default
   } else {
      uSecs = (int)timeOut;
   }

   QString msg = message;

   if (msg.isEmpty() && ! title.isEmpty()) {
      msg.append(' ');
   }

   sys->showMessage(title, message, type, uSecs);
}

QRect QSystemTrayIconPrivate::geometry_sys() const
{
   if (!sys) {
      return QRect();
   }

   return sys->findIconGeometry(q_uNOTIFYICONID);
}

void QSystemTrayIconPrivate::remove_sys()
{
   if (!sys) {
      return;
   }

   sys->trayMessage(NIM_DELETE);
   delete sys;
   sys = 0;
}

void QSystemTrayIconPrivate::updateIcon_sys()
{
   if (!sys) {
      return;
   }

   const HICON hIconToDestroy = sys->createIcon();
   sys->trayMessage(NIM_MODIFY);

   if (hIconToDestroy) {
      DestroyIcon(hIconToDestroy);
   }
}

void QSystemTrayIconPrivate::updateMenu_sys()
{

}

void QSystemTrayIconPrivate::updateToolTip_sys()
{
   if (!sys) {
      return;
   }

   sys->trayMessage(NIM_MODIFY);
}

bool QSystemTrayIconPrivate::isSystemTrayAvailable_sys()
{
   return true;
}

bool QSystemTrayIconPrivate::supportsMessages_sys()
{
   return allowsMessages();
}


#endif
