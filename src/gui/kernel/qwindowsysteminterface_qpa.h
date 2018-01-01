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

#ifndef QWINDOWSYSTEMINTERFACE_QPA_H
#define QWINDOWSYSTEMINTERFACE_QPA_H

#include <QtCore/QTime>
#include <QtGui/qwindowdefs.h>
#include <qcoreevent.h>
#include <QtGui/QWidget>
#include <QtCore/QWeakPointer>
#include <QtCore/QMutex>
#include <QtGui/QTouchEvent>

QT_BEGIN_NAMESPACE

class Q_GUI_EXPORT QWindowSystemInterface
{

 public:
   static void handleMouseEvent(QWidget *w, const QPoint &local, const QPoint &global, Qt::MouseButtons b);
   static void handleMouseEvent(QWidget *w, ulong timestamp, const QPoint &local, const QPoint &global,
                                Qt::MouseButtons b);

   static void handleKeyEvent(QWidget *w, QEvent::Type t, int k, Qt::KeyboardModifiers mods,
                              const QString &text = QString(), bool autorep = false, ushort count = 1);

   static void handleKeyEvent(QWidget *w, ulong timestamp, QEvent::Type t, int k, Qt::KeyboardModifiers mods,
                              const QString &text = QString(), bool autorep = false, ushort count = 1);

   static void handleExtendedKeyEvent(QWidget *w, QEvent::Type type, int key, Qt::KeyboardModifiers modifiers,
                                      quint32 nativeScanCode, quint32 nativeVirtualKey,
                                      quint32 nativeModifiers,
                                      const QString &text = QString(), bool autorep = false,
                                      ushort count = 1);
   static void handleExtendedKeyEvent(QWidget *w, ulong timestamp, QEvent::Type type, int key,
                                      Qt::KeyboardModifiers modifiers,
                                      quint32 nativeScanCode, quint32 nativeVirtualKey,
                                      quint32 nativeModifiers,
                                      const QString &text = QString(), bool autorep = false,
                                      ushort count = 1);

   static void handleWheelEvent(QWidget *w, const QPoint &local, const QPoint &global, int d, Qt::Orientation o);
   static void handleWheelEvent(QWidget *w, ulong timestamp, const QPoint &local, const QPoint &global, int d,
                                Qt::Orientation o);

   struct TouchPoint {
      int id;                 // for application use
      bool isPrimary;         // for application use
      QPointF normalPosition; // touch device coordinates, (0 to 1, 0 to 1)
      QRectF area;            // the touched area, centered at position in screen coordinates
      qreal pressure;         // 0 to 1
      Qt::TouchPointState state; //Qt::TouchPoint{Pressed|Moved|Stationary|Released}
   };

   static void handleTouchEvent(QWidget *w, QEvent::Type type, QTouchEvent::DeviceType devType,
                                const QList<struct TouchPoint> &points);

   static void handleTouchEvent(QWidget *w, ulong timestamp, QEvent::Type type,
                                QTouchEvent::DeviceType devType, const QList<struct TouchPoint> &points);

   static void handleGeometryChange(QWidget *w, const QRect &newRect);
   static void handleCloseEvent(QWidget *w);
   static void handleEnterEvent(QWidget *w);
   static void handleLeaveEvent(QWidget *w);
   static void handleWindowActivated(QWidget *w);
   static void handleWindowStateChanged(QWidget *w, Qt::WindowState newState);

   // Changes to the screen
   static void handleScreenGeometryChange(int screenIndex);
   static void handleScreenAvailableGeometryChange(int screenIndex);
   static void handleScreenCountChange(int count);

   static void handleLocaleChange();
   static void handlePlatformPanelEvent(QWidget *w);
};

QT_END_NAMESPACE

#endif // QWINDOWSYSTEMINTERFACE_H
