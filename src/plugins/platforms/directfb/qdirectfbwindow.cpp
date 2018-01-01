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

#include "qdirectfbwindow.h"
#include "qdirectfbwindowsurface.h"
#include "qdirectfbinput.h"
#include "qdirectfbscreen.h"


#include <directfb.h>

QT_BEGIN_NAMESPACE

QDirectFbWindow::QDirectFbWindow(QWidget *tlw, QDirectFbInput *inputhandler)
    : QPlatformWindow(tlw), m_inputHandler(inputhandler)
{
    DFBDisplayLayerConfig layerConfig;
    IDirectFBDisplayLayer *layer;

    layer = toDfbScreen(tlw)->dfbLayer();
    toDfbScreen(tlw)->dfbLayer()->GetConfiguration(layer, &layerConfig);

    DFBWindowDescription description;
    memset(&description,0,sizeof(DFBWindowDescription));
    description.flags = DFBWindowDescriptionFlags(DWDESC_WIDTH|DWDESC_HEIGHT|DWDESC_POSX|DWDESC_POSY|DWDESC_SURFACE_CAPS
#if DIRECTFB_MINOR_VERSION >= 1
                                                  |DWDESC_OPTIONS
#endif
                                                  |DWDESC_CAPS);
    description.width = tlw->width();
    description.height = tlw->height();
    description.posx = tlw->x();
    description.posy = tlw->y();

    if (layerConfig.surface_caps & DSCAPS_PREMULTIPLIED)
        description.surface_caps = DSCAPS_PREMULTIPLIED;
    description.pixelformat = layerConfig.pixelformat;

#if DIRECTFB_MINOR_VERSION >= 1
    description.options = DFBWindowOptions(DWOP_ALPHACHANNEL);
#endif
    description.caps = DFBWindowCapabilities(DWCAPS_DOUBLEBUFFER|DWCAPS_ALPHACHANNEL);
    description.surface_caps = DSCAPS_PREMULTIPLIED;

    DFBResult result = layer->CreateWindow(layer, &description, m_dfbWindow.outPtr());
    if (result != DFB_OK) {
        DirectFBError("QDirectFbGraphicsSystemScreen: failed to create window",result);
    }

    m_dfbWindow->SetOpacity(m_dfbWindow.data(), 0xff);

    setVisible(widget()->isVisible());

    m_inputHandler->addWindow(m_dfbWindow.data(), tlw);
}

QDirectFbWindow::~QDirectFbWindow()
{
    m_inputHandler->removeWindow(m_dfbWindow.data());
    m_dfbWindow->Destroy(m_dfbWindow.data());
}

void QDirectFbWindow::setGeometry(const QRect &rect)
{
    bool isMoveOnly = (rect.topLeft() != geometry().topLeft()) && (rect.size() == geometry().size());

    QPlatformWindow::setGeometry(rect);
    if (widget()->isVisible() && !(widget()->testAttribute(Qt::WA_DontShowOnScreen))) {
        m_dfbWindow->SetBounds(m_dfbWindow.data(), rect.x(),rect.y(),
                               rect.width(), rect.height());
        //Hack. When moving since the WindowSurface of a window becomes invalid when moved
        if (isMoveOnly) { //if resize then windowsurface is updated.
            widget()->windowSurface()->resize(rect.size());
            widget()->update();
        }
    }
}

void QDirectFbWindow::setOpacity(qreal level)
{
    const quint8 windowOpacity = quint8(level * 0xff);
    m_dfbWindow->SetOpacity(m_dfbWindow.data(), windowOpacity);
}

void QDirectFbWindow::setVisible(bool visible)
{
    if (visible) {
        int x = geometry().x();
        int y = geometry().y();
        m_dfbWindow->MoveTo(m_dfbWindow.data(), x, y);
    } else {
        QDirectFBPointer<IDirectFBDisplayLayer> displayLayer;
        QDirectFbConvenience::dfbInterface()->GetDisplayLayer(QDirectFbConvenience::dfbInterface(), DLID_PRIMARY, displayLayer.outPtr());

        DFBDisplayLayerConfig config;
        displayLayer->GetConfiguration(displayLayer.data(), &config);
        m_dfbWindow->MoveTo(m_dfbWindow.data(), config. width + 1, config.height + 1);
    }
}

Qt::WindowFlags QDirectFbWindow::setWindowFlags(Qt::WindowFlags flags)
{
    switch (flags & Qt::WindowType_Mask) {
    case Qt::ToolTip: {
        DFBWindowOptions options;
        m_dfbWindow->GetOptions(m_dfbWindow.data(), &options);
        options = DFBWindowOptions(options | DWOP_GHOST);
        m_dfbWindow->SetOptions(m_dfbWindow.data(), options);
        break; }
    default:
        break;
    }

    m_dfbWindow->SetStackingClass(m_dfbWindow.data(), flags & Qt::WindowStaysOnTopHint ? DWSC_UPPER : DWSC_MIDDLE);
    return flags;
}

void QDirectFbWindow::raise()
{
    m_dfbWindow->RaiseToTop(m_dfbWindow.data());
}

void QDirectFbWindow::lower()
{
    m_dfbWindow->LowerToBottom(m_dfbWindow.data());
}

WId QDirectFbWindow::winId() const
{
    DFBWindowID id;
    m_dfbWindow->GetID(m_dfbWindow.data(), &id);
    return WId(id);
}

bool QDirectFbWindow::setKeyboardGrabEnabled(bool grab)
{
    DFBResult res;

    if (grab)
        res = m_dfbWindow->GrabKeyboard(m_dfbWindow.data());
    else
        res = m_dfbWindow->UngrabKeyboard(m_dfbWindow.data());

    return res == DFB_OK;
}

bool QDirectFbWindow::setMouseGrabEnabled(bool grab)
{
    DFBResult res;

    if (grab)
        res = m_dfbWindow->GrabPointer(m_dfbWindow.data());
    else
        res = m_dfbWindow->UngrabPointer(m_dfbWindow.data());

    return res == DFB_OK;
}

IDirectFBWindow *QDirectFbWindow::dfbWindow() const
{
    return m_dfbWindow.data();
}

IDirectFBSurface *QDirectFbWindow::dfbSurface()
{
    if (!m_dfbSurface) {
        DFBResult res = m_dfbWindow->GetSurface(m_dfbWindow.data(), m_dfbSurface.outPtr());
        if (res != DFB_OK)
            DirectFBError(QDFB_PRETTY, res);
    }

    return m_dfbSurface.data();
}

QT_END_NAMESPACE
