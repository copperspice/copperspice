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

#ifndef QWINDOWSURFACE_X11GL_P_H
#define QWINDOWSURFACE_X11GL_P_H

#include <qwindowsurface_p.h>

QT_BEGIN_NAMESPACE

class QX11GLWindowSurface : public QWindowSurface
{
 public:
   QX11GLWindowSurface(QWidget *window);
   virtual ~QX11GLWindowSurface();

   // Inherreted from QWindowSurface
   QPaintDevice *paintDevice() override;
   void flush(QWidget *widget, const QRegion &region, const QPoint &offset) override;
   void setGeometry(const QRect &rect) override;
   bool scroll(const QRegion &area, int dx, int dy) override;
   QPixmap grabWidget(const QWidget *widget, const QRect &rectangle = QRect()) const;

 private:
   GC      m_windowGC;
   GC      m_pixmapGC;
   QPixmap m_backBuffer;
   QWidget *m_window;
};


QT_END_NAMESPACE

#endif // QWINDOWSURFACE_X11GL_P_H
