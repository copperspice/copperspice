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

#include <qxcb_glx_window.h>

#include <qxcb_screen.h>
#include <qglx_convenience_p.h>

QXcbGlxWindow::QXcbGlxWindow(QWindow *window)
   : QXcbWindow(window)
{
}

QXcbGlxWindow::~QXcbGlxWindow()
{
}

void QXcbGlxWindow::resolveFormat()
{
   m_format = window()->requestedFormat(); // qglx_findVisualInfo sets the resovled format
}

void *QXcbGlxWindow::createVisual()
{
   QXcbScreen *scr = xcbScreen();

   if (! scr) {
      return nullptr;
   }

   return qglx_findVisualInfo(DISPLAY_FROM_XCB(scr), scr->screenNumber(), &m_format);
}

