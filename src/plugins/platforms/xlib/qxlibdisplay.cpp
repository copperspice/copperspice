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

#include "qxlibdisplay.h"

QXlibDisplay::QXlibDisplay(Display *display)
    : mDisplay(display)
{
    if (!mDisplay) {
        qFatal("Cannot connect to X server");
    }
    mDisplayName = QString::fromLocal8Bit(DisplayString(mDisplay));
}

QXlibDisplay::~QXlibDisplay()
{
    XCloseDisplay(mDisplay);
}

QString QXlibDisplay::displayName() const
{
     { return mDisplayName; }
}



Display * QXlibDisplay::nativeDisplay() const
{
    return mDisplay;
}

void QXlibDisplay::sync() const
{
    XSync(mDisplay, False);
}

void QXlibDisplay::flush() const
{
    XFlush(mDisplay);
}
