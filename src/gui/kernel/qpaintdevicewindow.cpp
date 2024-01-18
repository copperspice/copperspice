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

#include <qpaintdevicewindow_p.h>

#include <qguiapplication.h>
#include <qscreen.h>

void QPaintDeviceWindow::update()
{
   update(QRect(QPoint(0, 0), size()));
}

void QPaintDeviceWindow::update(const QRect &rect)
{
   Q_D(QPaintDeviceWindow);

   d->dirtyRegion += rect;
   if (isExposed()) {
      requestUpdate();
   }
}

void QPaintDeviceWindow::update(const QRegion &region)
{
   Q_D(QPaintDeviceWindow);

   d->dirtyRegion += region;
   if (isExposed()) {
      requestUpdate();
   }
}

void QPaintDeviceWindow::paintEvent(QPaintEvent *event)
{
   (void) event;
}

/*!
  \internal
 */
int QPaintDeviceWindow::metric(PaintDeviceMetric metric) const
{
   QScreen *screen = this->screen();
   if (!screen && QGuiApplication::primaryScreen()) {
      screen = QGuiApplication::primaryScreen();
   }

   switch (metric) {
      case PdmWidth:
         return width();

      case PdmWidthMM:
         if (screen) {
            return width() * screen->physicalSize().width() / screen->geometry().width();
         }
         break;

      case PdmHeight:
         return height();

      case PdmHeightMM:
         if (screen) {
            return height() * screen->physicalSize().height() / screen->geometry().height();
         }
         break;

      case PdmDpiX:
         if (screen) {
            return qRound(screen->logicalDotsPerInchX());
         }
         break;

      case PdmDpiY:
         if (screen) {
            return qRound(screen->logicalDotsPerInchY());
         }
         break;

      case PdmPhysicalDpiX:
         if (screen) {
            return qRound(screen->physicalDotsPerInchX());
         }
         break;

      case PdmPhysicalDpiY:
         if (screen) {
            return qRound(screen->physicalDotsPerInchY());
         }
         break;

      case PdmDevicePixelRatio:
         return int(QWindow::devicePixelRatio());
         break;

      case PdmDevicePixelRatioScaled:
        return int(QWindow::devicePixelRatio() * devicePixelRatioFScale());
        break;

      default:
         break;
   }

   return QPaintDevice::metric(metric);
}

// internal
void QPaintDeviceWindow::exposeEvent(QExposeEvent *exposeEvent)
{
   (void) exposeEvent;

   Q_D(QPaintDeviceWindow);

   if (isExposed()) {
      d->markWindowAsDirty();
      // Do not rely on exposeEvent->region() as it has some issues for the
      // time being, namely that it is sometimes in local coordinates,
      // sometimes relative to the parent, depending on the platform plugin.
      // We require local coords here.
      d->doFlush(QRect(QPoint(0, 0), size()));

   } else if (!d->dirtyRegion.isEmpty()) {
      // Updates while non-exposed were ignored. Schedule an update now.
      requestUpdate();
   }
}

// internal
bool QPaintDeviceWindow::event(QEvent *event)
{
   Q_D(QPaintDeviceWindow);

   if (event->type() == QEvent::UpdateRequest) {
      if (handle()) {
         // platform window may be gone when the window is closed during app exit
         d->handleUpdateEvent();
      }

      return true;
   }

   return QWindow::event(event);
}

// internal
QPaintDeviceWindow::QPaintDeviceWindow(QPaintDeviceWindowPrivate &dd, QWindow *parent)
   : QWindow(dd, parent)
{
}

// internal
QPaintEngine *QPaintDeviceWindow::paintEngine() const
{
   return nullptr;
}

