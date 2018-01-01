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

#ifndef QMOUSETSLIB_QWS_H
#define QMOUSETSLIB_QWS_H

#include <QtGui/qmouse_qws.h>

QT_BEGIN_NAMESPACE

#if !defined(QT_NO_QWS_MOUSE_TSLIB) || defined(QT_PLUGIN)

class QWSTslibMouseHandlerPrivate;

class QWSTslibMouseHandler : public QWSCalibratedMouseHandler
{

 public:
   explicit QWSTslibMouseHandler(const QString &driver = QString(), const QString &device = QString());
   ~QWSTslibMouseHandler();

   void suspend();
   void resume();

   void calibrate(const QWSPointerCalibrationData *data);
   void clearCalibration();

 protected:
   friend class QWSTslibMouseHandlerPrivate;
   QWSTslibMouseHandlerPrivate *d;
};

#endif // QT_NO_QWS_MOUSE_TSLIB

QT_END_NAMESPACE

#endif // QMOUSETSLIB_QWS_H
