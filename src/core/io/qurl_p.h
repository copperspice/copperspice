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

#ifndef QURL_P_H
#define QURL_P_H

#include <qurl.h>

// in qurlrecode.cpp
extern int qt_urlRecode(QString &appendTo, QString::const_iterator begin, QString::const_iterator end,
      QUrl::FormattingOptions encoding, const ushort *tableModifications = nullptr);

// in qurlidna.cpp
enum AceLeadingDot {
   AllowLeadingDot,
   ForbidLeadingDot
};

enum AceOperation {
   ToAceOnly,
   NormalizeAce
};

QString qt_ACE_do(QStringView domain, AceOperation op, AceLeadingDot dot);
QString qt_urlRecodeByteArray(const QByteArray &ba);

#endif
