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

/***********************************************************************
* Copyright (c) 2007-2008, Apple, Inc.
* All rights reserved.
*
* Refer to APPLE_LICENSE.TXT (in this directory) for license terms
***********************************************************************/

#include "qcocoaintrospection.h"

void qt_cocoa_change_implementation(Class baseClass, SEL originalSel, Class proxyClass, SEL replacementSel, SEL backupSel)
{
   // The following code replaces the _implementation_ for the selector we want to hack
   // (originalSel) with the implementation found in proxyClass. Then it creates
   // a new 'backup' method inside baseClass containing the old, original,
   // implementation (fakeSel). You can let the proxy implementation of originalSel
   // call fakeSel if needed (similar approach to calling a super class implementation).
   // fakeSel must also be implemented in proxyClass, as the signature is used
   // as template for the method one we add into baseClass.
   // NB: You will typically never create any instances of proxyClass; we use it
   // only for stealing its contents and put it into baseClass.
   if (!replacementSel) {
      replacementSel = originalSel;
   }

   Method originalMethod = class_getInstanceMethod(baseClass, originalSel);
   Method replacementMethod = class_getInstanceMethod(proxyClass, replacementSel);
   IMP originalImp = method_setImplementation(originalMethod, method_getImplementation(replacementMethod));

   if (backupSel) {
      Method backupMethod = class_getInstanceMethod(proxyClass, backupSel);
      class_addMethod(baseClass, backupSel, originalImp, method_getTypeEncoding(backupMethod));
   }
}

void qt_cocoa_change_back_implementation(Class baseClass, SEL originalSel, SEL backupSel)
{
   Method originalMethod = class_getInstanceMethod(baseClass, originalSel);
   Method backupMethodInBaseClass = class_getInstanceMethod(baseClass, backupSel);
   method_setImplementation(originalMethod, method_getImplementation(backupMethodInBaseClass));
}

