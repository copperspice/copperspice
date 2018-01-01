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

#include "qnswindowdelegate.h"

#include <QDebug>
#include <QWindowSystemInterface>

@implementation QNSWindowDelegate

- (id) initWithQCocoaWindow: (QCocoaWindow *) cocoaWindow
{
    self = [super init];

    if (self) {
        m_cocoaWindow = cocoaWindow;
    }
    return self;
}

- (void)windowDidResize:(NSNotification *)notification
{
    Q_UNUSED(notification);
    if (m_cocoaWindow) {
        m_cocoaWindow->windowDidResize();
    }
}

- (void)windowWillClose:(NSNotification *)notification
{
    Q_UNUSED(notification);
    QWindowSystemInterface::handleCloseEvent(m_cocoaWindow->widget());
}

@end
