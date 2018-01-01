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

#ifndef QLIGHTHOUSEGRAPHICSSCREEN_H
#define QLIGHTHOUSEGRAPHICSSCREEN_H

#include <qrect.h>
#include <qimage.h>
#include <qtimer.h>
#include <qpainter.h>
#include <QPlatformCursor>
#include <QPlatformScreen>
#include <QPlatformWindow>
#include <qwindowsurface_p.h>

class QMouseEvent;
class QSize;
class QPainter;

class QFbScreen;

class QPlatformSoftwareCursor : public QPlatformCursor
{
public:
    QPlatformSoftwareCursor(QPlatformScreen * scr);

    // output methods
    QRect dirtyRect();
    virtual QRect drawCursor(QPainter & painter);

    // input methods
    virtual void pointerEvent(const QMouseEvent & event);
    virtual void changeCursor(QCursor * widgetCursor, QWidget * widget);

    virtual void setDirty() { dirty = true; screen->setDirty(QRect()); }
    virtual bool isDirty() { return dirty; }
    virtual bool isOnScreen() { return onScreen; }
    virtual QRect lastPainted() { return prevRect; }

protected:
    QPlatformCursorImage * graphic;

private:
    void setCursor(const uchar *data, const uchar *mask, int width, int height, int hotX, int hotY);
    void setCursor(Qt::CursorShape shape);
    void setCursor(const QImage &image, int hotx, int hoty);
    QRect currentRect;      // next place to draw the cursor
    QRect prevRect;         // last place the cursor was drawn
    QRect getCurrentRect();
    bool dirty;
    bool onScreen;
};

class QFbWindow;

class QFbWindowSurface : public QWindowSurface
{
public:
    QFbWindowSurface(QFbScreen *screen, QWidget *window);
    ~QFbWindowSurface();

    virtual QPaintDevice *paintDevice() { return &mImage; }
    virtual void flush(QWidget *widget, const QRegion &region, const QPoint &offset);
    virtual bool scroll(const QRegion &area, int dx, int dy);

    virtual void beginPaint(const QRegion &region);
    virtual void endPaint(const QRegion &region);


    const QImage image() { return mImage; }
    void resize(const QSize &size);

protected:
    friend class QFbWindow;
    QFbWindow *platformWindow;

    QFbScreen *mScreen;
    QImage mImage;
};


class QFbWindow : public QPlatformWindow
{
public:

    QFbWindow(QWidget *window);
    ~QFbWindow();


    virtual void setVisible(bool visible);
    virtual bool visible() { return visibleFlag; }

    virtual void raise();
    virtual void lower();

    void setGeometry(const QRect &rect);

    virtual Qt::WindowFlags setWindowFlags(Qt::WindowFlags type);
    virtual Qt::WindowFlags windowFlags() const;

    WId winId() const { return windowId; }

    virtual void repaint(const QRegion&);

protected:
    friend class QFbWindowSurface;
    friend class QFbScreen;
    QFbWindowSurface *surface;
    QList<QFbScreen *> mScreens;
    QRect oldGeometry;
    bool visibleFlag;
    Qt::WindowFlags flags;

    WId windowId;
};

class QFbScreen : public QPlatformScreen
{
    Q_OBJECT
public:
    QFbScreen();
    ~QFbScreen();

    virtual QRect geometry() const { return mGeometry; }
    virtual int depth() const { return mDepth; }
    virtual QImage::Format format() const { return mFormat; }
    virtual QSize physicalSize() const { return mPhysicalSize; }

    virtual void setGeometry(QRect rect);
    virtual void setDepth(int depth);
    virtual void setFormat(QImage::Format format);
    virtual void setPhysicalSize(QSize size);

    virtual void setDirty(const QRect &rect);

    virtual void removeWindow(QFbWindow * surface);
    virtual void addWindow(QFbWindow * surface);
    virtual void raise(QPlatformWindow * surface);
    virtual void lower(QPlatformWindow * surface);
    virtual QWidget * topLevelAt(const QPoint & p) const;

    QImage * image() const { return mScreenImage; }
    QPaintDevice * paintDevice() const { return mScreenImage; }

protected:
    QList<QFbWindow *> windowStack;
    QRegion repaintRegion;
    QPlatformSoftwareCursor * cursor;
    QTimer redrawTimer;

protected slots:
    virtual QRegion doRedraw();

protected:
    QRect mGeometry;
    int mDepth;
    QImage::Format mFormat;
    QSize mPhysicalSize;
    QImage *mScreenImage;

private:
    QPainter * compositePainter;
    void generateRects();
    QList<QPair<QRect, int> > cachedRects;

    void invalidateRectCache() { isUpToDate = false; }
    friend class QFbWindowSurface;
    friend class QFbWindow;
    bool isUpToDate;
};

#endif // QLIGHTHOUSEGRAPHICSSCREEN_H
