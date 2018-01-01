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

#include <qdesktopwidget.h>
#include <qt_windows.h>
#include <qapplication_p.h>
#include <qwidget_p.h>
#include <qdebug.h>
#include <qsystemlibrary_p.h>
#include <qvector.h>
#include <limits.h>

QT_BEGIN_NAMESPACE

class QDesktopWidgetPrivate : public QWidgetPrivate
{
 public:
   QDesktopWidgetPrivate();
   ~QDesktopWidgetPrivate();

   static void init(QDesktopWidget *that);
   static void cleanup();
   static int screenCount;
   static int primaryScreen;

   static QVector<QRect> *rects;
   static QVector<QRect> *workrects;

   struct MONITORINFO {
      DWORD   cbSize;
      RECT    rcMonitor;
      RECT    rcWork;
      DWORD   dwFlags;
   };

   typedef BOOL (WINAPI *InfoFunc)(HMONITOR, MONITORINFO *);
   typedef BOOL (QT_WIN_CALLBACK *EnumProc)(HMONITOR, HDC, LPRECT, LPARAM);
   typedef BOOL (WINAPI *EnumFunc)(HDC, LPCRECT, EnumProc, LPARAM);

   static EnumFunc enumDisplayMonitors;
   static InfoFunc getMonitorInfo;
   static int refcount;
};

int QDesktopWidgetPrivate::screenCount = 1;
int QDesktopWidgetPrivate::primaryScreen = 0;
QDesktopWidgetPrivate::EnumFunc QDesktopWidgetPrivate::enumDisplayMonitors = 0;
QDesktopWidgetPrivate::InfoFunc QDesktopWidgetPrivate::getMonitorInfo = 0;
QVector<QRect> *QDesktopWidgetPrivate::rects = 0;
QVector<QRect> *QDesktopWidgetPrivate::workrects = 0;
static int screen_number = 0;
int QDesktopWidgetPrivate::refcount = 0;

BOOL QT_WIN_CALLBACK enumCallback(HMONITOR hMonitor, HDC, LPRECT, LPARAM)
{
   QDesktopWidgetPrivate::screenCount++;
   QDesktopWidgetPrivate::rects->resize(QDesktopWidgetPrivate::screenCount);
   QDesktopWidgetPrivate::workrects->resize(QDesktopWidgetPrivate::screenCount);
   // Get the MONITORINFO block
   QDesktopWidgetPrivate::MONITORINFO info;
   memset(&info, 0, sizeof(QDesktopWidgetPrivate::MONITORINFO));
   info.cbSize = sizeof(QDesktopWidgetPrivate::MONITORINFO);
   BOOL res = QDesktopWidgetPrivate::getMonitorInfo(hMonitor, &info);
   if (!res) {
      (*QDesktopWidgetPrivate::rects)[screen_number] = QRect();
      (*QDesktopWidgetPrivate::workrects)[screen_number] = QRect();
      return true;
   }

   // Fill list of rects
   RECT r = info.rcMonitor;
   QRect qr(QPoint(r.left, r.top), QPoint(r.right - 1, r.bottom - 1));
   (*QDesktopWidgetPrivate::rects)[screen_number] = qr;

   r = info.rcWork;
   qr = QRect(QPoint(r.left, r.top), QPoint(r.right - 1, r.bottom - 1));
   (*QDesktopWidgetPrivate::workrects)[screen_number] = qr;

   if (info.dwFlags & 0x00000001) { //MONITORINFOF_PRIMARY
      QDesktopWidgetPrivate::primaryScreen = screen_number;
   }

   ++screen_number;
   // Stop the enumeration if we have them all
   return true;
}

QDesktopWidgetPrivate::QDesktopWidgetPrivate()
{
   ++refcount;
}

void QDesktopWidgetPrivate::init(QDesktopWidget *that)
{
   if (rects) {
      return;
   }

   rects = new QVector<QRect>();
   workrects = new QVector<QRect>();
   screenCount = 0;

   QSystemLibrary user32Lib(QLatin1String("user32"));
   enumDisplayMonitors = (EnumFunc)user32Lib.resolve("EnumDisplayMonitors");
   getMonitorInfo = (InfoFunc)user32Lib.resolve("GetMonitorInfoW");

   if (!enumDisplayMonitors || !getMonitorInfo) {
      screenCount = GetSystemMetrics(80);  // SM_CMONITORS
      rects->resize(screenCount);
      for (int i = 0; i < screenCount; ++i) {
         rects->replace(i, that->rect());
      }
      return;
   }
   // Calls enumCallback
   enumDisplayMonitors(0, 0, enumCallback, 0);
   enumDisplayMonitors = 0;
   getMonitorInfo = 0;
}

QDesktopWidgetPrivate::~QDesktopWidgetPrivate()
{
   if (!--refcount) {
      cleanup();
   }
}

void QDesktopWidgetPrivate::cleanup()
{
   screen_number = 0;
   screenCount = 1;
   primaryScreen = 0;
   enumDisplayMonitors = 0;
   getMonitorInfo = 0;
   delete rects;
   rects = 0;
   delete workrects;
   workrects = 0;
}

/*
    \omit
    Function is commented out in header
    \fn void *QDesktopWidget::handle(int screen) const

    Returns the window system handle of the display device with the
    index \a screen, for low-level access.  Using this function is not
    portable.

    The return type varies with platform; see qwindowdefs.h for details.

    \sa x11Display(), QPaintDevice::handle()
    \endomit
*/

QDesktopWidget::QDesktopWidget()
   : QWidget(*new QDesktopWidgetPrivate, 0, Qt::Desktop)
{
   setObjectName(QLatin1String("desktop"));
   QDesktopWidgetPrivate::init(this);
}

QDesktopWidget::~QDesktopWidget()
{
}

bool QDesktopWidget::isVirtualDesktop() const
{
   return true;
}

int QDesktopWidget::primaryScreen() const
{
   return d_func()->primaryScreen;
}

int QDesktopWidget::numScreens() const
{
   return d_func()->screenCount;
}

QWidget *QDesktopWidget::screen(int /* screen */)
{
   // It seems that a Qt::WType_Desktop cannot be moved?
   return this;
}

const QRect QDesktopWidget::availableGeometry(int screen) const
{
   Q_D(const QDesktopWidget);

   if (screen < 0 || screen >= d->screenCount) {
      screen = d->primaryScreen;
   }

   return d->workrects->at(screen);
}

const QRect QDesktopWidget::screenGeometry(int screen) const
{
   const QDesktopWidgetPrivate *d = d_func();
   if (screen < 0 || screen >= d->screenCount) {
      screen = d->primaryScreen;
   }

   return d->rects->at(screen);
}

int QDesktopWidget::screenNumber(const QWidget *widget) const
{
   Q_D(const QDesktopWidget);
   if (!widget) {
      return d->primaryScreen;
   }

   QRect frame = widget->frameGeometry();
   if (!widget->isWindow()) {
      frame.moveTopLeft(widget->mapToGlobal(QPoint(0, 0)));
   }

   int maxSize = -1;
   int maxScreen = -1;

   for (int i = 0; i < d->screenCount; ++i) {
      QRect sect = d->rects->at(i).intersected(frame);
      int size = sect.width() * sect.height();
      if (size > maxSize && sect.width() > 0 && sect.height() > 0) {
         maxSize = size;
         maxScreen = i;
      }
   }

   return maxScreen;
}

int QDesktopWidget::screenNumber(const QPoint &point) const
{
   Q_D(const QDesktopWidget);

   int closestScreen = -1;
   int shortestDistance = INT_MAX;

   for (int i = 0; i < d->screenCount; ++i) {
      int thisDistance = d->pointToRect(point, d->rects->at(i));
      if (thisDistance < shortestDistance) {
         shortestDistance = thisDistance;
         closestScreen = i;
      }
   }

   return closestScreen;
}

void QDesktopWidget::resizeEvent(QResizeEvent *)
{
   Q_D(QDesktopWidget);
   const QVector<QRect> oldrects(*d->rects);
   const QVector<QRect> oldworkrects(*d->workrects);
   int oldscreencount = d->screenCount;

   QDesktopWidgetPrivate::cleanup();
   QDesktopWidgetPrivate::init(this);

   for (int i = 0; i < qMin(oldscreencount, d->screenCount); ++i) {
      const QRect oldrect = oldrects[i];
      const QRect newrect = d->rects->at(i);
      if (oldrect != newrect) {
         emit resized(i);
      }
   }

   for (int j = 0; j < qMin(oldscreencount, d->screenCount); ++j) {
      const QRect oldrect = oldworkrects[j];
      const QRect newrect = d->workrects->at(j);
      if (oldrect != newrect) {
         emit workAreaResized(j);
      }
   }

   if (oldscreencount != d->screenCount) {
      emit screenCountChanged(d->screenCount);
   }
}

QT_END_NAMESPACE
