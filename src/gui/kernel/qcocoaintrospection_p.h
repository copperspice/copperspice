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

/***********************************************************************
** Copyright (c) 2007-2008, Apple, Inc.
***********************************************************************/

#ifndef QCocoaIntrospection_P_H
#define QCocoaIntrospection_P_H

#include <qglobal.h>
#import <objc/objc-class.h>

QT_BEGIN_NAMESPACE

void qt_cocoa_change_implementation(Class baseClass, SEL originalSel, Class proxyClass, SEL replacementSel = 0,
                                    SEL backupSel = 0);
void qt_cocoa_change_back_implementation(Class baseClass, SEL originalSel, SEL backupSel);

QT_END_NAMESPACE

#endif
