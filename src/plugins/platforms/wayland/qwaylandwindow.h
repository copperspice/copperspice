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

#ifndef QWAYLANDWINDOW_H
#define QWAYLANDWINDOW_H

#include <QtGui/QPlatformWindow>
#include <QtCore/QWaitCondition>

#include "qwaylanddisplay.h"

class QWaylandDisplay;
class QWaylandBuffer;
struct wl_egl_window;

class QWaylandWindow : public QPlatformWindow
{
public:
    enum WindowType {
        Shm,
        Egl
    };

    QWaylandWindow(QWidget *window);
    ~QWaylandWindow();

    virtual WindowType windowType() const = 0;
    WId winId() const;
    void setVisible(bool visible);
    void setParent(const QPlatformWindow *parent);

    void configure(uint32_t time, uint32_t edges,
                   int32_t x, int32_t y, int32_t width, int32_t height);

    void attach(QWaylandBuffer *buffer);
    void damage(const QRect &rect);

    void waitForFrameSync();

    struct wl_surface *wl_surface() const { return mSurface; }

protected:
    struct wl_surface *mSurface;
    virtual void newSurfaceCreated();
    QWaylandDisplay *mDisplay;
    QWaylandBuffer *mBuffer;
    WId mWindowId;
    bool mWaitingForFrameSync;
    QWaitCondition mFrameSyncWait;

private:
    static void frameCallback(struct wl_surface *surface, void *data, uint32_t time);


};


#endif // QWAYLANDWINDOW_H
