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

#ifndef QPLATFORMINTEGRATION_DIRECTFB_H
#define QPLATFORMINTEGRATION_DIRECTFB_H

#include "qdirectfbinput.h"
#include "qdirectfbscreen.h"

#include <QtGui/QPlatformIntegration>
#include <directfb.h>
#include <directfb_version.h>

QT_BEGIN_NAMESPACE

class QThread;

class QDirectFbIntegration : public QPlatformIntegration
{
public:
    QDirectFbIntegration();
    ~QDirectFbIntegration();

    QPixmapData *createPixmapData(QPixmapData::PixelType type) const;
    QPlatformWindow *createPlatformWindow(QWidget *widget, WId winId = 0) const;
    QWindowSurface *createWindowSurface(QWidget *widget, WId winId) const;

    QList<QPlatformScreen *> screens() const { return mScreens; }

    QPlatformFontDatabase *fontDatabase() const;

    void initialize();

protected:
    virtual void initializeDirectFB();
    virtual void initializeScreen();
    virtual void initializeInput();

protected:
    QList<QPlatformScreen *> mScreens;
    QDirectFBPointer<IDirectFB> m_dfb;
    QScopedPointer<QDirectFbScreen> m_primaryScreen;
    QScopedPointer<QDirectFbInput> m_input;
    QScopedPointer<QThread> m_inputRunner;
    QScopedPointer<QPlatformFontDatabase> m_fontDb;
};

QT_END_NAMESPACE

#endif
