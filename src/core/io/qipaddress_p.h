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

/****************************************************************************
**
** Copyright (C) 2012 Intel Corporation
** Contact: http://www.qt-project.org/
**
****************************************************************************/

#ifndef QIPADDRESS_P_H
#define QIPADDRESS_P_H

#include <qstring.h>

namespace QIPAddressUtils {

typedef quint32 IPv4Address;
typedef quint8 IPv6Address[16];

Q_CORE_EXPORT bool parseIp4(IPv4Address &address, const QChar *begin, const QChar *end);
Q_CORE_EXPORT const QChar *parseIp6(IPv6Address &address, const QChar *begin, const QChar *end);

Q_CORE_EXPORT void toString(QString &appendTo, IPv4Address address);
Q_CORE_EXPORT void toString(QString &appendTo, IPv6Address address);

} // namespace


#endif
