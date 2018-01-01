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

/********************************************************
**  Copyright (C) 2005-2006 Matthias Kretz <kretz@kde.org
********************************************************/

#include "videowidget.h"
#include "videowidget_p.h"
#include "videowidgetinterface.h"
#include "factory_p.h"
#include "phonondefs_p.h"
#include "phononnamespace_p.h"

#include <QtGui/QAction>
#define IFACES4 VideoWidgetInterface44
#define IFACES0 VideoWidgetInterface, IFACES4
#define PHONON_INTERFACENAME IFACES0

QT_BEGIN_NAMESPACE

#ifndef QT_NO_PHONON_VIDEO

namespace Phonon
{

VideoWidget::VideoWidget(QWidget *parent)
    : QWidget(parent)
    , Phonon::AbstractVideoOutput(*new VideoWidgetPrivate(this))
{
    K_D(VideoWidget);
    d->init();
    d->createBackendObject();
    setMouseTracking(true);
}



VideoWidget::VideoWidget(VideoWidgetPrivate &dd, QWidget *parent)
    : QWidget(parent),
    Phonon::AbstractVideoOutput(dd)
{
    K_D(VideoWidget);
    d->init();
}

void VideoWidgetPrivate::init()
{
    Q_Q(VideoWidget);
    changeFlags = q->windowFlags() & (Qt::SubWindow | Qt::Window);
}

void VideoWidget::mouseMoveEvent(QMouseEvent *e)
{
    QWidget::mouseMoveEvent(e);
}

void VideoWidgetPrivate::createBackendObject()
{
    if (m_backendObject)
        return;
    Q_Q(VideoWidget);
    m_backendObject = Factory::createVideoWidget(q);
    if (m_backendObject) {
        setupBackendObject();
    }
}

#define PHONON_CLASSNAME VideoWidget

PHONON_INTERFACE_GETTER(Phonon::VideoWidget::AspectRatio, aspectRatio, d->aspectRatio)
PHONON_INTERFACE_SETTER(setAspectRatio, aspectRatio, Phonon::VideoWidget::AspectRatio)

PHONON_INTERFACE_GETTER(Phonon::VideoWidget::ScaleMode, scaleMode, d->scaleMode)
PHONON_INTERFACE_SETTER(setScaleMode, scaleMode, Phonon::VideoWidget::ScaleMode)

PHONON_INTERFACE_GETTER(qreal, brightness, d->brightness)
PHONON_INTERFACE_SETTER(setBrightness, brightness, qreal)

PHONON_INTERFACE_GETTER(qreal, contrast, d->contrast)
PHONON_INTERFACE_SETTER(setContrast, contrast, qreal)

PHONON_INTERFACE_GETTER(qreal, hue, d->hue)
PHONON_INTERFACE_SETTER(setHue, hue, qreal)

PHONON_INTERFACE_GETTER(qreal, saturation, d->saturation)
PHONON_INTERFACE_SETTER(setSaturation, saturation, qreal)


QImage VideoWidget::snapshot() const {
    K_D(const VideoWidget);
    ConstIface<IFACES4> iface(d);
    if(iface) return iface->snapshot();
    return QImage(); // TODO not implemented in VideoInterface
}


void VideoWidget::setFullScreen(bool newFullScreen)
{
    pDebug() << Q_FUNC_INFO << newFullScreen;
    K_D(VideoWidget);
    // TODO: disable screensaver? or should we leave that responsibility to the
    // application?
    Qt::WindowFlags flags = windowFlags();
    if (newFullScreen) {
        if (!isFullScreen()) {
            //we only update that value if it is not already fullscreen
            d->changeFlags = flags & (Qt::Window | Qt::SubWindow);
            flags |= Qt::Window;
            flags ^= Qt::SubWindow;
            setWindowFlags(flags);
#ifdef Q_WS_X11
            // This works around a bug with Compiz
            // as the window must be visible before we can set the state
            show();
            raise();
            setWindowState( windowState() | Qt::WindowFullScreen ); // set
#else
            setWindowState( windowState() | Qt::WindowFullScreen ); // set
            show();
#endif
        }
    } else if (isFullScreen()) {
        flags ^= (Qt::Window | Qt::SubWindow); //clear the flags...
        flags |= d->changeFlags; //then we reset the flags (window and subwindow)
        setWindowFlags(flags);
        setWindowState( windowState()  ^ Qt::WindowFullScreen ); // reset
        show();
    }
}

void VideoWidget::exitFullScreen()
{
    setFullScreen(false);
}

void VideoWidget::enterFullScreen()
{
    setFullScreen(true);
}

bool VideoWidgetPrivate::aboutToDeleteBackendObject()
{
    aspectRatio = pINTERFACE_CALL(aspectRatio());
    scaleMode = pINTERFACE_CALL(scaleMode());
    return AbstractVideoOutputPrivate::aboutToDeleteBackendObject();
}

void VideoWidgetPrivate::setupBackendObject()
{
    Q_Q(VideoWidget);
    Q_ASSERT(m_backendObject);
    //AbstractVideoOutputPrivate::setupBackendObject();
    pDebug() << "calling setAspectRatio on the backend " << aspectRatio;
    pINTERFACE_CALL(setAspectRatio(aspectRatio));
    pINTERFACE_CALL(setScaleMode(scaleMode));

    QWidget *w = pINTERFACE_CALL(widget());
    if (w) {
        layout.addWidget(w);
        q->setSizePolicy(w->sizePolicy());
        w->setMouseTracking(true);
    }
}

bool VideoWidget::event(QEvent *e)
{
    return QWidget::event(e);
}

} //namespace Phonon

#endif //QT_NO_PHONON_VIDEO

QT_END_NAMESPACE

#undef PHONON_CLASSNAME
