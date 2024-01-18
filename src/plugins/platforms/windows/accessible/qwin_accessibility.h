/***********************************************************************
*
* Copyright (c) 2012-2024 Barbara Geller
* Copyright (c) 2012-2024 Ansel Sermersheim
*
* Copyright (c) 2015 The Qt Company Ltd.
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
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
* https://www.gnu.org/licenses/
*
***********************************************************************/

#ifndef QWINDOWSACCESSIBILITY_H
#define QWINDOWSACCESSIBILITY_H

#ifndef QT_NO_ACCESSIBILITY

#include <qwin_global.h>
#include <qwin_context.h>
#include <qplatform_accessibility.h>

#include <oleacc.h>

class QWindowsAccessibility : public QPlatformAccessibility
{
public:
    QWindowsAccessibility();
    static bool handleAccessibleObjectFromWindowRequest(HWND hwnd, WPARAM wParam, LPARAM lParam, LRESULT *lResult);
    void notifyAccessibilityUpdate(QAccessibleEvent *event) override;
    static IAccessible *wrap(QAccessibleInterface *acc);
    static QWindow *windowHelper(const QAccessibleInterface *iface);
};

#endif

#endif
