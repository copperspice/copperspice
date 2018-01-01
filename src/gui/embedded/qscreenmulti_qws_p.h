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

#ifndef QSCREENMULTI_QWS_P_H
#define QSCREENMULTI_QWS_P_H

#include <qscreen_qws.h>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_QWS_MULTISCREEN

class QMultiScreenPrivate;

class QMultiScreen : public QScreen
{
 public:
   QMultiScreen(int displayId);
   ~QMultiScreen();
   bool initDevice();
   bool connect(const QString &displaySpec);
   void disconnect();
   void shutdownDevice();
   void setMode(int, int, int);
   bool supportsDepth(int) const;

   void save();
   void restore();
   void blank(bool on);

   bool onCard(const unsigned char *) const;
   bool onCard(const unsigned char *, ulong &out_offset) const;

   bool isInterlaced() const;

   int memoryNeeded(const QString &);
   int sharedRamSize(void *);

   void haltUpdates();
   void resumeUpdates();

   void exposeRegion(QRegion r, int changing);

   void blit(const QImage &img, const QPoint &topLeft, const QRegion &region);
   void solidFill(const QColor &color, const QRegion &region);
   void blit(QWSWindow *bs, const QRegion &clip);
   void setDirty(const QRect &);

   QWSWindowSurface *createSurface(QWidget *widget) const;
   QWSWindowSurface *createSurface(const QString &key) const;

   QList<QScreen *> subScreens() const;
   QRegion region() const;

 private:
   void addSubScreen(QScreen *screen);
   void removeSubScreen(QScreen *screen);

   QMultiScreenPrivate *d_ptr;
};


QT_END_NAMESPACE
#endif // QT_NO_QWS_MULTISCREEN
#endif // QMULTISCREEN_QWS_P_H
