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

#ifndef QMOUSE_QWS_H
#define QMOUSE_QWS_H

#include <QtCore/qobject.h>
#include <QtGui/qpolygon.h>

QT_BEGIN_NAMESPACE

class QWSMouseHandlerPrivate;
class QScreen;

class Q_GUI_EXPORT QWSPointerCalibrationData
{
 public:
   enum Location { TopLeft = 0, BottomLeft = 1, BottomRight = 2, TopRight = 3,
                   Center = 4, LastLocation = Center
                 };
   QPoint devPoints[5];
   QPoint screenPoints[5];
};

class Q_GUI_EXPORT QWSMouseHandler
{
 public:
   explicit QWSMouseHandler(const QString &driver = QString(), const QString &device = QString());
   virtual ~QWSMouseHandler();

   virtual void clearCalibration() {}
   virtual void calibrate(const QWSPointerCalibrationData *) {}
   virtual void getCalibration(QWSPointerCalibrationData *) const {}

   virtual void resume() = 0;
   virtual void suspend() = 0;

   void limitToScreen(QPoint &pt);
   void mouseChanged(const QPoint &pos, int bstate, int wheel = 0);

   const QPoint &pos() const {
      return mousePos;
   }

   void setScreen(const QScreen *screen);

 protected:
   QPoint &mousePos;
   QWSMouseHandlerPrivate *d_ptr;
};


class Q_GUI_EXPORT QWSCalibratedMouseHandler : public QWSMouseHandler
{
 public:
   explicit QWSCalibratedMouseHandler(const QString &driver = QString(), const QString &device = QString());

   virtual void clearCalibration();
   virtual void calibrate(const QWSPointerCalibrationData *);
   virtual void getCalibration(QWSPointerCalibrationData *) const;

 protected:
   bool sendFiltered(const QPoint &, int button);
   QPoint transform(const QPoint &);

   void readCalibration();
   void writeCalibration();
   void setFilterSize(int);

 private:
   int a, b, c;
   int d, e, f;
   int s;
   QPolygon samples;
   int currSample;
   int numSamples;
};

QT_END_NAMESPACE

#endif // QMOUSE_QWS_H
