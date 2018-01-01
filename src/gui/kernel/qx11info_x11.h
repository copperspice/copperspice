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

#ifndef QX11INFO_X11_H
#define QX11INFO_X11_H

#include <QtCore/qnamespace.h>

typedef struct _XDisplay Display;

QT_BEGIN_NAMESPACE

struct QX11InfoData;
class QX11Info;
class QPaintDevice;
class QApplicationPrivate;
class QX11InfoPrivate;
struct QX11WindowAttributes;

void qt_x11_getX11InfoForWindow(QX11Info *xinfo, const QX11WindowAttributes &a);

class Q_GUI_EXPORT QX11Info
{

 public:
   QX11Info();
   ~QX11Info();
   QX11Info(const QX11Info &other);
   QX11Info &operator=(const QX11Info &other);

   static Display *display();
   static const char *appClass();
   int screen() const;
   int depth() const;
   int cells() const;
   Qt::HANDLE colormap() const;
   bool defaultColormap() const;
   void *visual() const;
   bool defaultVisual() const;

   static int appScreen();
   static int appDepth(int screen = -1);
   static int appCells(int screen = -1);
   static Qt::HANDLE appColormap(int screen = -1);
   static void *appVisual(int screen = -1);
   static Qt::HANDLE appRootWindow(int screen = -1);
   static bool appDefaultColormap(int screen = -1);
   static bool appDefaultVisual(int screen = -1);
   static int appDpiX(int screen = -1);
   static int appDpiY(int screen = -1);
   static void setAppDpiX(int screen, int dpi);
   static void setAppDpiY(int screen, int dpi);
   static unsigned long appTime();
   static unsigned long appUserTime();
   static void setAppTime(unsigned long time);
   static void setAppUserTime(unsigned long time);
   static bool isCompositingManagerRunning();

 protected:
   void copyX11Data(const QPaintDevice *);
   void cloneX11Data(const QPaintDevice *);
   void setX11Data(const QX11InfoData *);
   QX11InfoData *getX11Data(bool def = false) const;

   QX11InfoData *x11data;

   friend class QX11PaintEngine;
   friend class QPixmap;
   friend class QX11PixmapData;
   friend class QWidget;
   friend class QWidgetPrivate;
   friend class QGLWidget;
   friend void qt_init(QApplicationPrivate *priv, int, Display *display, Qt::HANDLE visual,
                       Qt::HANDLE colormap);
   friend void qt_cleanup();
   friend void qt_x11_getX11InfoForWindow(QX11Info *xinfo, const QX11WindowAttributes &a);
};

QT_END_NAMESPACE

#endif // QX11INFO_X11_H
