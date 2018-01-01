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

#include "qdirectfbintegration.h"
#include "qdirectfbwindowsurface.h"
#include "qdirectfbblitter.h"
#include "qdirectfbconvenience.h"
#include "qdirectfbcursor.h"
#include "qdirectfbwindow.h"

#include <QtGui/private/qpixmap_blitter_p.h>
#include <QtGui/private/qpixmap_raster_p.h>
#include <QtCore/QCoreApplication>
#include <QtCore/QThread>

#include "qgenericunixfontdatabase.h"

QT_BEGIN_NAMESPACE

QDirectFbIntegration::QDirectFbIntegration()
    : m_fontDb(new QGenericUnixFontDatabase())
{}

void QDirectFbIntegration::initialize()
{
    initializeDirectFB();
    initializeScreen();
    initializeInput();
}

void QDirectFbIntegration::initializeDirectFB()
{
    const QStringList args = QCoreApplication::arguments();
    int argc = args.size();
    char **argv = new char*[argc];

    for (int i = 0; i < argc; ++i)
        argv[i] = qstrdup(args.at(i).toLocal8Bit().constData());

    DFBResult result = DirectFBInit(&argc, &argv);
    if (result != DFB_OK) {
        DirectFBError("QDirectFBScreen: error initializing DirectFB",
                      result);
    }

    for (int i = 0; i < argc; ++i)
        delete[] argv[i];
    delete[] argv;

    // This must happen after DirectFBInit.
    m_dfb.reset(QDirectFbConvenience::dfbInterface());
}

void QDirectFbIntegration::initializeScreen()
{
    m_primaryScreen.reset(new QDirectFbScreen(0));
    mScreens.append(m_primaryScreen.data());
}

void QDirectFbIntegration::initializeInput()
{
    m_input.reset(new QDirectFbInput(m_dfb.data(), m_primaryScreen->dfbLayer()));
    m_input->start();
}

QDirectFbIntegration::~QDirectFbIntegration()
{
    m_input->stopInputEventLoop();
    m_input->wait();
}

QPixmapData *QDirectFbIntegration::createPixmapData(QPixmapData::PixelType type) const
{
    if (type == QPixmapData::BitmapType)
        return new QRasterPixmapData(type);
    else
        return new QDirectFbBlitterPlatformPixmap;
}

QPlatformWindow *QDirectFbIntegration::createPlatformWindow(QWidget *window, WId) const
{
    return new QDirectFbWindow(window,m_input.data());
}

QWindowSurface *QDirectFbIntegration::createWindowSurface(QWidget *window, WId) const
{
    return new QDirectFbWindowSurface(window);
}

QPlatformFontDatabase *QDirectFbIntegration::fontDatabase() const
{
    return m_fontDb.data();
}

QT_END_NAMESPACE
