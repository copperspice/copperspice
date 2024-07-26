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

#include <qwin_screen.h>

#include <qapplication.h>
#include <qdebug.h>
#include <qpixmap.h>
#include <qscreen.h>
#include <qsettings.h>
#include <qwin_additional.h>
#include <qwin_context.h>
#include <qwin_cursor.h>
#include <qwin_integration.h>
#include <qwin_window.h>
#include <qwindowsysteminterface.h>

#include <qhighdpiscaling_p.h>

QWindowsScreenData::QWindowsScreenData()
   : dpi(96, 96), depth(32), format(QImage::Format_ARGB32_Premultiplied),
     flags(VirtualDesktop), orientation(Qt::LandscapeOrientation), refreshRateHz(60)
{
}

static inline QDpi deviceDPI(HDC hdc)
{
   return QDpi(GetDeviceCaps(hdc, LOGPIXELSX), GetDeviceCaps(hdc, LOGPIXELSY));
}

static inline QDpi monitorDPI(HMONITOR hMonitor)
{
   if (QWindowsContext::shcoredll.isValid()) {
      UINT dpiX;
      UINT dpiY;
      if (SUCCEEDED(QWindowsContext::shcoredll.getDpiForMonitor(hMonitor, 0, &dpiX, &dpiY))) {
         return QDpi(dpiX, dpiY);
      }
   }
   return QDpi(0, 0);
}


typedef QList<QWindowsScreenData> WindowsScreenDataList;

static bool monitorData(HMONITOR hMonitor, QWindowsScreenData *data)
{
   MONITORINFOEX info;
   memset(&info, 0, sizeof(MONITORINFOEX));
   info.cbSize = sizeof(MONITORINFOEX);
   if (GetMonitorInfo(hMonitor, &info) == FALSE) {
      return false;
   }

   data->geometry = QRect(QPoint(info.rcMonitor.left, info.rcMonitor.top), QPoint(info.rcMonitor.right - 1, info.rcMonitor.bottom - 1));
   data->availableGeometry = QRect(QPoint(info.rcWork.left, info.rcWork.top), QPoint(info.rcWork.right - 1, info.rcWork.bottom - 1));
   data->name = QString::fromStdWString(std::wstring(info.szDevice));

   if (data->name == "WinDisc") {
      data->flags |= QWindowsScreenData::LockScreen;

   } else {
      // Windows CE just supports one Display and expects to get only DISPLAY,
      // instead of DISPLAY0 and so on, which are passed by info.szDevice
      HDC hdc = CreateDC(TEXT("DISPLAY"), nullptr, nullptr, nullptr);

      if (hdc) {
         const QDpi dpi = monitorDPI(hMonitor);
         data->dpi = dpi.first ? dpi : deviceDPI(hdc);

         data->depth = GetDeviceCaps(hdc, BITSPIXEL);
         data->format = data->depth == 16 ? QImage::Format_RGB16 : QImage::Format_RGB32;
         data->physicalSizeMM = QSizeF(GetDeviceCaps(hdc, HORZSIZE), GetDeviceCaps(hdc, VERTSIZE));
         const int refreshRate = GetDeviceCaps(hdc, VREFRESH);

         if (refreshRate > 1) {
            // 0,1 means hardware default
            data->refreshRateHz = refreshRate;
         }
         DeleteDC(hdc);

      } else {
         qWarning("monitorData() Unable to obtain handle for monitor %s, defaulting to %g DPI.",
            csPrintable(QString::fromStdWString(std::wstring(info.szDevice))), data->dpi.first);

      } // CreateDC() failed

   } // not lock screen

   data->orientation = data->geometry.height() > data->geometry.width() ?
      Qt::PortraitOrientation : Qt::LandscapeOrientation;

   // EnumDisplayMonitors (as opposed to EnumDisplayDevices) enumerates only
   // virtual desktop screens.

   data->flags |= QWindowsScreenData::VirtualDesktop;
   if (info.dwFlags & MONITORINFOF_PRIMARY) {
      data->flags |= QWindowsScreenData::PrimaryScreen;
   }

   return true;
}

// from QDesktopWidget, taking WindowsScreenDataList as LPARAM
BOOL QT_WIN_CALLBACK monitorEnumCallback(HMONITOR hMonitor, HDC, LPRECT, LPARAM p)
{
   QWindowsScreenData data;

   if (monitorData(hMonitor, &data)) {
      WindowsScreenDataList *result = reinterpret_cast<WindowsScreenDataList *>(p);
      // QPlatformIntegration::screenAdded() documentation specifies that first
      // added screen will be the primary screen, so order accordingly.
      // Note that the side effect of this policy is that there is no way to change primary
      // screen reported by Qt, unless we want to delete all existing screens and add them
      // again whenever primary screen changes.

      if (data.flags & QWindowsScreenData::PrimaryScreen) {
         result->prepend(data);
      } else {
         result->append(data);
      }
   }
   return TRUE;
}

static inline WindowsScreenDataList monitorData()
{
   WindowsScreenDataList result;
   EnumDisplayMonitors(nullptr, nullptr, monitorEnumCallback, reinterpret_cast<LPARAM>(&result));

   return result;
}

#if defined(CS_SHOW_DEBUG_PLATFORM)
static QDebug operator<<(QDebug debug, const QWindowsScreenData &d)
{
   QDebugStateSaver saver(debug);
   debug.nospace();
   debug.noquote();

   debug << "  Screen Id = \"" << d.name << "\" "
      << " size = " << d.geometry.width() << "x" << d.geometry.height()
      << " location = " << d.geometry.x() << '+' << d.geometry.y()

      << "\n   Availiable = " << d.availableGeometry.width() << "x" << d.availableGeometry.height()
      << " location = " << d.availableGeometry.x() << '+' << d.availableGeometry.y()

      << "  Physical = " << d.physicalSizeMM.width() << " x " << d.physicalSizeMM.height()
      << "\n   DPI = " << d.dpi.first << "x" << d.dpi.second << "  Depth = " << d.depth
      << "  Format = " << d.format << "  Flags = ";

   if (d.flags & QWindowsScreenData::PrimaryScreen) {
      debug << "primary ";
   }

   if (d.flags & QWindowsScreenData::VirtualDesktop) {
      debug << "virtual desktop ";
   }

   if (d.flags & QWindowsScreenData::LockScreen) {
      debug << "lock screen";
   }

   return debug;
}
#endif

QWindowsScreen::QWindowsScreen(const QWindowsScreenData &data)
   : m_data(data)

#ifndef QT_NO_CURSOR
   , m_cursor(new QWindowsCursor(this))
#endif
{
}

Q_GUI_EXPORT QPixmap qt_pixmapFromWinHBITMAP(HBITMAP bitmap, int hbitmapFormat = 0);

QPixmap QWindowsScreen::grabWindow(WId window, int x, int y, int width, int height) const
{
   RECT r;
   HWND hwnd = window ? reinterpret_cast<HWND>(window) : GetDesktopWindow();
   GetClientRect(hwnd, &r);

   if (width < 0) {
      width = r.right - r.left;
   }
   if (height < 0) {
      height = r.bottom - r.top;
   }

   // Create and setup bitmap
   HDC display_dc = GetDC(nullptr);
   HDC bitmap_dc = CreateCompatibleDC(display_dc);
   HBITMAP bitmap = CreateCompatibleBitmap(display_dc, width, height);
   HGDIOBJ null_bitmap = SelectObject(bitmap_dc, bitmap);

   // copy data
   HDC window_dc = GetDC(hwnd);
   BitBlt(bitmap_dc, 0, 0, width, height, window_dc, x, y, SRCCOPY | CAPTUREBLT);

   // clean up all but bitmap
   ReleaseDC(hwnd, window_dc);
   SelectObject(bitmap_dc, null_bitmap);
   DeleteDC(bitmap_dc);

   const QPixmap pixmap = qt_pixmapFromWinHBITMAP(bitmap);

   DeleteObject(bitmap);
   ReleaseDC(nullptr, display_dc);

   return pixmap;
}

QWindow *QWindowsScreen::topLevelWindowAt(const QPoint &point) const
{
   QWindow *result = nullptr;

   if (QWindow *child = QWindowsScreen::windowAt(point, CWP_SKIPINVISIBLE)) {
      result = QWindowsWindow::topLevelOf(child);
   }

#if defined(CS_SHOW_DEBUG_PLATFORM)
   qDebug() << "QWindowsScreen::topLevelWindowAt() " << point << result;
#endif

   return result;
}

QWindow *QWindowsScreen::windowAt(const QPoint &screenPoint, unsigned flags)
{
   QWindow *result = nullptr;

   if (QPlatformWindow *bw = QWindowsContext::instance()->
         findPlatformWindowAt(GetDesktopWindow(), screenPoint, flags)) {

      result = bw->window();
   }

#if defined(CS_SHOW_DEBUG_PLATFORM)
   qDebug() << "QWindowsScreen::windowAt() " << screenPoint << " Return =" << result;
#endif

   return result;
}

qreal QWindowsScreen::pixelDensity() const
{
   // QTBUG-49195: Use logical DPI instead of physical DPI to calculate
   // the pixel density since it is reflects the Windows UI scaling.
   // High DPI auto scaling should be disabled when the user chooses
   // small fonts on a High DPI monitor, resulting in lower logical DPI.
   return qRound(logicalDpi().first / 96);
}

QList<QPlatformScreen *> QWindowsScreen::virtualSiblings() const
{
   QList<QPlatformScreen *> result;
   if (m_data.flags & QWindowsScreenData::VirtualDesktop) {
      for (QWindowsScreen *screen : QWindowsContext::instance()->screenManager().screens())
         if (screen->data().flags & QWindowsScreenData::VirtualDesktop) {
            result.push_back(screen);
         }
   } else {
      result.push_back(const_cast<QWindowsScreen *>(this));
   }

   return result;
}

void QWindowsScreen::handleChanges(const QWindowsScreenData &newData)
{
   m_data.physicalSizeMM = newData.physicalSizeMM;

   if (m_data.geometry != newData.geometry || m_data.availableGeometry != newData.availableGeometry) {
      m_data.geometry = newData.geometry;
      m_data.availableGeometry = newData.availableGeometry;
      QWindowSystemInterface::handleScreenGeometryChange(screen(),
         newData.geometry, newData.availableGeometry);
   }

   if (!qFuzzyCompare(m_data.dpi.first, newData.dpi.first)
      || !qFuzzyCompare(m_data.dpi.second, newData.dpi.second)) {
      m_data.dpi = newData.dpi;

      QWindowSystemInterface::handleScreenLogicalDotsPerInchChange(screen(),
         newData.dpi.first, newData.dpi.second);
   }
   if (m_data.orientation != newData.orientation) {
      m_data.orientation = newData.orientation;

      QWindowSystemInterface::handleScreenOrientationChange(screen(), newData.orientation);
   }
}

// matching Win32 API ORIENTATION_PREFERENCE
enum OrientationPreference : DWORD
{
   orientationPreferenceNone = 0,
   orientationPreferenceLandscape = 0x1,
   orientationPreferencePortrait = 0x2,
   orientationPreferenceLandscapeFlipped = 0x4,
   orientationPreferencePortraitFlipped = 0x8
};

bool QWindowsScreen::setOrientationPreference(Qt::ScreenOrientation o)
{
   bool result = false;

   if (QWindowsContext::user32dll.setDisplayAutoRotationPreferences) {
      DWORD orientationPreference = 0;
      switch (o) {
         case Qt::PrimaryOrientation:
            orientationPreference = orientationPreferenceNone;
            break;
         case Qt::PortraitOrientation:
            orientationPreference = orientationPreferencePortrait;
            break;
         case Qt::LandscapeOrientation:
            orientationPreference = orientationPreferenceLandscape;
            break;
         case Qt::InvertedPortraitOrientation:
            orientationPreference = orientationPreferencePortraitFlipped;
            break;
         case Qt::InvertedLandscapeOrientation:
            orientationPreference = orientationPreferenceLandscapeFlipped;
            break;
      }
      result = QWindowsContext::user32dll.setDisplayAutoRotationPreferences(orientationPreference);
   }

   return result;
}

Qt::ScreenOrientation QWindowsScreen::orientationPreference()
{
   Qt::ScreenOrientation result = Qt::PrimaryOrientation;

   if (QWindowsContext::user32dll.getDisplayAutoRotationPreferences) {
      DWORD orientationPreference = 0;
      if (QWindowsContext::user32dll.getDisplayAutoRotationPreferences(&orientationPreference)) {
         switch (orientationPreference) {
            case orientationPreferenceLandscape:
               result = Qt::LandscapeOrientation;
               break;
            case orientationPreferencePortrait:
               result = Qt::PortraitOrientation;
               break;
            case orientationPreferenceLandscapeFlipped:
               result = Qt::InvertedLandscapeOrientation;
               break;
            case orientationPreferencePortraitFlipped:
               result = Qt::InvertedPortraitOrientation;
               break;
         }
      }
   }

   return result;
}

QPlatformScreen::SubpixelAntialiasingType QWindowsScreen::subpixelAntialiasingTypeHint() const
{
#if ! defined(FT_LCD_FILTER_H) || ! defined(FT_CONFIG_OPTION_SUBPIXEL_RENDERING)
   return QPlatformScreen::Subpixel_None;
#else
   QPlatformScreen::SubpixelAntialiasingType type = QPlatformScreen::subpixelAntialiasingTypeHint();

   if (type == QPlatformScreen::Subpixel_None) {
      QSettings settings(QLatin1String("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Avalon.Graphics\\DISPLAY1"), QSettings::NativeFormat);
      int registryValue = settings.value(QLatin1String("PixelStructure"), -1).toInt();

      switch (registryValue) {
         case 0:
            type = QPlatformScreen::Subpixel_None;
            break;
         case 1:
            type = QPlatformScreen::Subpixel_RGB;
            break;
         case 2:
            type = QPlatformScreen::Subpixel_BGR;
            break;
         default:
            type = QPlatformScreen::Subpixel_None;
            break;
      }
   }
   return type;
#endif
}

QWindowsScreenManager::QWindowsScreenManager() :
   m_lastDepth(-1), m_lastHorizontalResolution(0), m_lastVerticalResolution(0)
{
}

bool QWindowsScreenManager::handleDisplayChange(WPARAM wParam, LPARAM lParam)
{
   const int newDepth = int(wParam);
   const WORD newHorizontalResolution = LOWORD(lParam);
   const WORD newVerticalResolution = HIWORD(lParam);

   if (newDepth != m_lastDepth || newHorizontalResolution != m_lastHorizontalResolution
      || newVerticalResolution != m_lastVerticalResolution) {
      m_lastDepth = newDepth;
      m_lastHorizontalResolution = newHorizontalResolution;
      m_lastVerticalResolution = newVerticalResolution;

#if defined(CS_SHOW_DEBUG_PLATFORM)
      qDebug() << "QWindowsScreenManager::handleDisplayChange() Depth =" << newDepth
               << ", Resolution =" << newHorizontalResolution << "x =" << newVerticalResolution;
#endif

      handleScreenChanges();
   }

   return false;
}

static inline int indexOfMonitor(const QList<QWindowsScreen *> &screens,
   const QString &monitorName)
{
   for (int i = 0; i < screens.size(); ++i) {
      if (screens.at(i)->data().name == monitorName) {
         return i;
      }
   }

   return -1;
}

static inline int indexOfMonitor(const QList<QWindowsScreenData> &screenData,
   const QString &monitorName)
{
   for (int i = 0; i < screenData.size(); ++i) {
      if (screenData.at(i).name == monitorName) {
         return i;
      }
   }

   return -1;
}

// Move a window to a new virtual screen, accounting for varying sizes.
static void moveToVirtualScreen(QWindow *w, const QScreen *newScreen)
{
   QRect geometry = w->geometry();
   const QRect oldScreenGeometry = w->screen()->geometry();
   const QRect newScreenGeometry = newScreen->geometry();
   QPoint relativePosition = geometry.topLeft() - oldScreenGeometry.topLeft();

   if (oldScreenGeometry.size() != newScreenGeometry.size()) {
      const qreal factor =
         qreal(QPoint(newScreenGeometry.width(), newScreenGeometry.height()).manhattanLength()) /
         qreal(QPoint(oldScreenGeometry.width(), oldScreenGeometry.height()).manhattanLength());
      relativePosition = (QPointF(relativePosition) * factor).toPoint();
   }

   geometry.moveTopLeft(relativePosition);
   w->setGeometry(geometry);
}

void QWindowsScreenManager::removeScreen(int index)
{
#if defined(CS_SHOW_DEBUG_PLATFORM)
   qDebug() << "QWindowsScreenManager::removeScreen() Removing Monitor = " << m_screens.at(index)->data();
#endif

   QScreen *screen = m_screens.at(index)->screen();
   QScreen *primaryScreen = QApplication::primaryScreen();

   // QTBUG-38650: When a screen is disconnected, Windows will automatically
   // move the Window to another screen. This will trigger a geometry change
   // event, but unfortunately after the screen destruction signal. To prevent
   // QtGui from automatically hiding the QWindow, pretend all Windows move to
   // the primary screen first (which is likely the correct, final screen).
   // QTBUG-39320: Windows does not automatically move WS_EX_TOOLWINDOW (dock) windows;
   // move those manually.

   if (screen != primaryScreen) {
      unsigned movedWindowCount = 0;

      for (QWindow *w : QApplication::topLevelWindows()) {
         if (w->screen() == screen && w->handle() && w->type() != Qt::Desktop) {
            if (w->isVisible() && w->windowState() != Qt::WindowMinimized
               && (QWindowsWindow::baseWindowOf(w)->exStyle() & WS_EX_TOOLWINDOW)) {
               moveToVirtualScreen(w, primaryScreen);
            } else {
               QWindowSystemInterface::handleWindowScreenChanged(w, primaryScreen);
            }
            ++movedWindowCount;
         }
      }
      if (movedWindowCount) {
         QWindowSystemInterface::flushWindowSystemEvents();
      }
   }

   QWindowsIntegration::instance()->emitDestroyScreen(m_screens.takeAt(index));
}

bool QWindowsScreenManager::handleScreenChanges()
{
   // Look for changed monitors, add new ones
   WindowsScreenDataList newDataList = monitorData();
   const bool lockScreen = (newDataList.size() == 1) && (newDataList.front().flags & QWindowsScreenData::LockScreen);

   for (const QWindowsScreenData &newData : newDataList) {
      const int existingIndex = indexOfMonitor(m_screens, newData.name);

      if (existingIndex != -1) {
         m_screens.at(existingIndex)->handleChanges(newData);

      } else {
         QWindowsScreen *newScreen = new QWindowsScreen(newData);
         m_screens.push_back(newScreen);

         QWindowsIntegration::instance()->emitScreenAdded(newScreen, newData.flags & QWindowsScreenData::PrimaryScreen);

#if defined(CS_SHOW_DEBUG_PLATFORM)
         qDebug() << "QWindowsScreenManager::handleScreenChanges() " << newData;
#endif
      }
   }

   // Remove deleted ones but keep main monitors if we get only the
   // temporary lock screen to avoid window recreation
   if (! lockScreen) {
      for (int i = m_screens.size() - 1; i >= 0; --i) {
         if (indexOfMonitor(newDataList, m_screens.at(i)->data().name) == -1) {
            removeScreen(i);
         }
      }
   }

   return true;
}

void QWindowsScreenManager::clearScreens()
{
   // Delete screens in reverse order to avoid crash in case of multiple screens
   while (! m_screens.isEmpty()) {
      QWindowsIntegration::instance()->emitDestroyScreen(m_screens.takeLast());
   }
}

const QWindowsScreen *QWindowsScreenManager::screenAtDp(const QPoint &p) const
{
   for (QWindowsScreen *scr : m_screens) {
      if (scr->geometry().contains(p)) {
         return scr;
      }
   }

   return nullptr;
}
