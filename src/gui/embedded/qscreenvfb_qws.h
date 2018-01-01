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

#ifndef QSCREENVFB_QWS_H
#define QSCREENVFB_QWS_H

#include <QtGui/qscreen_qws.h>
#include <QtGui/qvfbhdr.h>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_QWS_QVFB

class QVFbScreenPrivate;

class Q_GUI_EXPORT QVFbScreen : public QScreen
{

 public:
   explicit QVFbScreen(int display_id);
   virtual ~QVFbScreen();
   virtual bool initDevice();
   virtual bool connect(const QString &displaySpec);
   virtual void disconnect();
   virtual void shutdownDevice();
   virtual void save();
   virtual void restore();
   virtual void setMode(int nw, int nh, int nd);
   virtual void setDirty(const QRect &r);
   virtual void blank(bool);

#ifdef QTOPIA_QVFB_BRIGHTNESS
   static void setBrightness(int b);
#endif

 private:
   QVFbScreenPrivate *d_ptr;
};

#endif // QT_NO_QWS_QVFB

QT_END_NAMESPACE

#endif // QSCREENVFB_QWS_H
