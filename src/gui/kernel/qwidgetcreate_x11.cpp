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

#include "qwidget.h"
#include "qt_x11_p.h"

/*
  Internal Qt functions to create X windows.  We have put them in
  separate functions to allow the programmer to reimplement them by
  custom versions.
*/

QT_BEGIN_NAMESPACE

Window qt_XCreateWindow(const QWidget *, Display *display, Window parent,
                        int x, int y, uint w, uint h,
                        int borderwidth, int depth,
                        uint windowclass, Visual *visual,
                        ulong valuemask, XSetWindowAttributes *attributes)
{
   return XCreateWindow(display, parent, x, y, w, h, borderwidth, depth,
                        windowclass, visual, valuemask, attributes);
}


Window qt_XCreateSimpleWindow(const QWidget *, Display *display, Window parent,
                              int x, int y, uint w, uint h, int borderwidth,
                              ulong border, ulong background)
{
   return XCreateSimpleWindow(display, parent, x, y, w, h, borderwidth,
                              border, background);
}


void qt_XDestroyWindow(const QWidget *, Display *display, Window window)
{
   if (window) {
      XDestroyWindow(display, window);
   }
}

QT_END_NAMESPACE
