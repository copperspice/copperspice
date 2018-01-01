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

#ifndef QSCREENQNX_QWS_H
#define QSCREENQNX_QWS_H

#include <QtGui/qscreen_qws.h>

#ifndef QT_NO_QWS_QNX

QT_BEGIN_NAMESPACE

struct QQnxScreenContext;

class QQnxScreen : public QScreen
{

 public:
   explicit QQnxScreen(int display_id);
   ~QQnxScreen();

   bool initDevice();
   bool connect(const QString &displaySpec);
   void disconnect();
   void shutdownDevice();
   void setMode(int, int, int);
   bool supportsDepth(int) const;
   void blank(bool on);

   void exposeRegion(QRegion r, int changing);

 private:
   QQnxScreenContext *const d;
};

QT_END_NAMESPACE

#endif // QT_NO_QWS_QNX

#endif
