/***********************************************************************
*
* Copyright (c) 2012-2016 Barbara Geller
* Copyright (c) 2012-2016 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or 
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see 
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QDBUSMETATYPE_P_H
#define QDBUSMETATYPE_P_H

#include <qdbusmetatype.h>

QT_BEGIN_NAMESPACE

struct QDBusMetaTypeId
{
    static int message;         // QDBusMessage
    static int argument;        // QDBusArgument
    static int variant;         // QDBusVariant
    static int objectpath;      // QDBusObjectPath
    static int signature;       // QDBusSignature
    static int error;           // QDBusError
    static int unixfd;          // QDBusUnixFileDescriptor

    static void init();
};

QT_END_NAMESPACE

#endif
