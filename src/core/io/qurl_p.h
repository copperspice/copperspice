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

#ifndef QURL_P_H
#define QURL_P_H

#include <qurl.h>

// in qurlrecode.cpp
extern int qt_urlRecode(QString &appendTo, const QChar *begin, const QChar *end,
                        QUrl::FormattingOptions encoding, const ushort *tableModifications = 0);

// in qurlidna.cpp
enum AceLeadingDot { AllowLeadingDot, ForbidLeadingDot };
enum AceOperation { ToAceOnly, NormalizeAce };

extern QString qt_ACE_do(const QString &domain, AceOperation op, AceLeadingDot dot);
extern  void qt_nameprep(QString *source, int from);
extern bool qt_check_std3rules(const QChar *uc, int len);
extern void qt_punycodeEncoder(const QChar *s, int ucLength, QString *output);
extern QString qt_punycodeDecoder(const QString &pc);
extern QString qt_urlRecodeByteArray(const QByteArray &ba);

#endif
