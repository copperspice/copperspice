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

#ifndef QScreenProxy_QWS_H
#define QScreenProxy_QWS_H

#include <QtGui/qscreen_qws.h>

#ifndef QT_NO_QWS_PROXYSCREEN

QT_BEGIN_NAMESPACE

class QProxyScreenPrivate;

#ifndef QT_NO_QWS_CURSOR

class QProxyScreenCursorPrivate;

class Q_GUI_EXPORT QProxyScreenCursor : public QScreenCursor
{

 public:
   QProxyScreenCursor();
   ~QProxyScreenCursor();

   void setScreenCursor(QScreenCursor *cursor);
   QScreenCursor *screenCursor() const;

   void set(const QImage &image, int hotx, int hoty);
   void move(int x, int y);
   void show();
   void hide();

 private:
   void configure();

   QScreenCursor *realCursor;
   QProxyScreenCursorPrivate *d_ptr;
};

#endif // QT_NO_QWS_CURSOR

class Q_GUI_EXPORT QProxyScreen : public QScreen
{
 public:
   QProxyScreen(int display_id, ClassId = ProxyClass);
   ~QProxyScreen();

   void setScreen(QScreen *screen);
   QScreen *screen() const;

   QSize mapToDevice(const QSize &s) const;
   QSize mapFromDevice(const QSize &s) const;

   QPoint mapToDevice(const QPoint &, const QSize &) const;
   QPoint mapFromDevice(const QPoint &, const QSize &) const;

   QRect mapToDevice(const QRect &, const QSize &) const;
   QRect mapFromDevice(const QRect &, const QSize &) const;

   QRegion mapToDevice(const QRegion &, const QSize &) const;
   QRegion mapFromDevice(const QRegion &, const QSize &) const;

   bool connect(const QString &displaySpec);
   bool initDevice();
   void shutdownDevice();
   void disconnect();

   void setMode(int width, int height, int depth);
   bool supportsDepth(int) const;

   void save();
   void restore();
   void blank(bool on);

   bool onCard(const unsigned char *) const;
   bool onCard(const unsigned char *, ulong &out_offset) const;

   bool isInterlaced() const;
   bool isTransformed() const;
   int transformOrientation() const;

   int memoryNeeded(const QString &);
   int sharedRamSize(void *);

   void haltUpdates();
   void resumeUpdates();

   void exposeRegion(QRegion r, int changing);
   void blit(const QImage &img, const QPoint &topLeft, const QRegion &region);
   void solidFill(const QColor &color, const QRegion &region);
   void setDirty(const QRect &);

   QWSWindowSurface *createSurface(QWidget *widget) const;
   QWSWindowSurface *createSurface(const QString &key) const;

   QList<QScreen *> subScreens() const;
   QRegion region() const;

 private:
   void configure();

   QScreen *realScreen;
   QProxyScreenPrivate *d_ptr;
};

QT_END_NAMESPACE

#endif // QT_NO_QWS_PROXYSCREEN
#endif // QPROXYSCREEN_QWS_H
