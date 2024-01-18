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

#ifndef QPAINTDEVICEWINDOW_P_H
#define QPAINTDEVICEWINDOW_P_H

#include <QPaintDeviceWindow>
#include <QCoreApplication>
#include <qwindow_p.h>
#include <QPaintEvent>

class Q_GUI_EXPORT QPaintDeviceWindowPrivate : public QWindowPrivate
{
   Q_DECLARE_PUBLIC(QPaintDeviceWindow)

 public:
   virtual void beginPaint(const QRegion &region) {
      (void) region;
   }

   virtual void endPaint() {
   }

   virtual void flush(const QRegion &region) {
      (void) region;
   }

   bool paint(const QRegion &region) {
      Q_Q(QPaintDeviceWindow);
      QRegion toPaint = region & dirtyRegion;

      if (toPaint.isEmpty()) {
         return false;
      }

      // Clear the region now. The overridden functions may call update().
      dirtyRegion -= toPaint;

      beginPaint(toPaint);

      QPaintEvent paintEvent(toPaint);
      q->paintEvent(&paintEvent);

      endPaint();

      return true;
   }

   void doFlush(const QRegion &region) {
      QRegion toFlush = region;

      if (paint(toFlush)) {
         flush(toFlush);
      }
   }

   void handleUpdateEvent() {
      if (dirtyRegion.isEmpty()) {
         return;
      }
      doFlush(dirtyRegion);
   }

   void markWindowAsDirty() {
      Q_Q(QPaintDeviceWindow);
      dirtyRegion += QRect(QPoint(0, 0), q->size());
   }

 private:
   QRegion dirtyRegion;
};

#endif
