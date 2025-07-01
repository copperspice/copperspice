/***********************************************************************
*
* Copyright (c) 2012-2025 Barbara Geller
* Copyright (c) 2012-2025 Ansel Sermersheim
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

#include <qwayland_touch_p.h>


namespace QtWaylandClient {

QWaylandTouchExtension::QWaylandTouchExtension(QWaylandDisplay *display, uint32_t id)
   : mDisplay(display), mTouchDevice(nullptr), mTargetWindow(nullptr), mInputDevice(nullptr),
     mTimestamp(0), mFlags(0), mPointsLeft(0), mMouseSourceId(-1)
{
}

void QWaylandTouchExtension::registerDevice(int caps)
{
   mTouchDevice = new QTouchDevice;
   mTouchDevice->setType(QTouchDevice::TouchScreen);
   mTouchDevice->setCapabilities(QTouchDevice::Capabilities(caps));

   QWindowSystemInterface::registerTouchDevice(mTouchDevice);
}

static inline qreal fromFixed(int f)
{
   return f / qreal(10000);
}

void QWaylandTouchExtension::touch_extension_touch(uint32_t time, uint32_t id, uint32_t state, int32_t x, int32_t y,
      int32_t normalized_x, int32_t normalized_y, int32_t width, int32_t height, uint32_t pressure,
      int32_t velocity_x, int32_t velocity_y, uint32_t flags, wl_array *rawdata)
{
   // pending implementation
}

void QWaylandTouchExtension::sendTouchEvent()
{
   // Copy all points which are in the previous list but not in the current list
   for (int i = 0; i < mPrevTouchPoints.count(); ++i) {
      const QWindowSystemInterface::TouchPoint &prevPoint(mPrevTouchPoints.at(i));

      if (prevPoint.state == Qt::TouchPointReleased) {
         continue;
      }

      bool found = false;

      for (int j = 0; j < mTouchPoints.count(); ++j)
         if (mTouchPoints.at(j).id == prevPoint.id) {
            found = true;
            break;
         }

      if (! found) {
         QWindowSystemInterface::TouchPoint p = prevPoint;
         p.state = Qt::TouchPointStationary;
         mTouchPoints.append(p);
      }
   }

   if (mTouchPoints.isEmpty()) {
      mPrevTouchPoints.clear();
      return;
   }

   QWindowSystemInterface::handleTouchEvent(mTargetWindow, mTimestamp, mTouchDevice, mTouchPoints);

   Qt::TouchPointStates states = nullptr;

   for (int i = 0; i < mTouchPoints.count(); ++i) {
      states |= mTouchPoints.at(i).state;
   }

   if (mFlags & QT_TOUCH_EXTENSION_FLAGS_MOUSE_FROM_TOUCH) {
      if (states == Qt::TouchPointPressed) {
         mMouseSourceId = mTouchPoints.first().id;
      }

      for (int i = 0; i < mTouchPoints.count(); ++i) {
         const QWindowSystemInterface::TouchPoint &tp(mTouchPoints.at(i));

         if (tp.id == mMouseSourceId) {
            Qt::MouseButtons buttons = tp.state == Qt::TouchPointReleased ? Qt::NoButton : Qt::LeftButton;
            mLastMouseGlobal = tp.area.center();

            QPoint globalPoint = mLastMouseGlobal.toPoint();
            QPointF delta = mLastMouseGlobal - globalPoint;

            mLastMouseLocal = mTargetWindow->mapFromGlobal(globalPoint) + delta;
            QWindowSystemInterface::handleMouseEvent(mTargetWindow, mTimestamp, mLastMouseLocal, mLastMouseGlobal, buttons);

            if (buttons == Qt::NoButton) {
               mMouseSourceId = -1;
            }

            break;
         }
      }
   }

   mPrevTouchPoints = mTouchPoints;
   mTouchPoints.clear();

   if (states == Qt::TouchPointReleased) {
      mPrevTouchPoints.clear();
   }
}

void QWaylandTouchExtension::touchCanceled()
{
   mTouchPoints.clear();
   mPrevTouchPoints.clear();

   if (mMouseSourceId != -1) {
      QWindowSystemInterface::handleMouseEvent(mTargetWindow, mTimestamp, mLastMouseLocal, mLastMouseGlobal, Qt::NoButton);
   }
}

void QWaylandTouchExtension::touch_extension_configure(uint32_t flags)
{
   mFlags = flags;
}

}
