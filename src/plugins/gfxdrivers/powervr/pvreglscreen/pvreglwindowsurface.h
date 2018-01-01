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

#ifndef PVREGLWINDOWSURFACE_H
#define PVREGLWINDOWSURFACE_H

#include <qglwindowsurface_qws_p.h>
#include <pvrqwsdrawable.h>

class PvrEglScreen;

class PvrEglWindowSurface : public QWSGLWindowSurface
{
public:
    PvrEglWindowSurface(QWidget *widget, PvrEglScreen *screen, int screenNum);
    PvrEglWindowSurface();
    ~PvrEglWindowSurface();

    QString key() const { return QLatin1String("PvrEgl"); }

    bool isValid() const;

    void setGeometry(const QRect &rect);
    bool move(const QPoint &offset);

    QByteArray permanentState() const;
    void setPermanentState(const QByteArray &state);

    void flush(QWidget *widget, const QRegion &region, const QPoint &offset);

    QImage image() const;
    QPaintDevice *paintDevice();

    void setDirectRegion(const QRegion &region, int id);

    long nativeDrawable() const { return (long)widget; }

private:
    QWidget *widget;
    PvrQwsDrawable *drawable;
    PvrEglScreen *screen;
    QPaintDevice *pdevice;

    void transformRects(PvrQwsRect *rects, int count) const;
};

#endif
