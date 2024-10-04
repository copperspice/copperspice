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

#ifndef QCommonNamespaces_P_H
#define QCommonNamespaces_P_H

#include <qstring.h>

namespace QPatternist {

namespace CommonNamespaces {
const QString XML("http://www.w3.org/XML/1998/namespace");

const QString XMLNS("http://www.w3.org/2000/xmlns/");

const QString WXS("http://www.w3.org/2001/XMLSchema");

const QString XSI("http://www.w3.org/2001/XMLSchema-instance");

const QString XFN("http://www.w3.org/2005/xpath-functions");

const QString XSLT("http://www.w3.org/1999/XSL/Transform");

const QString XPERR("http://www.w3.org/2005/xqt-errors");

const char *const UNICODE_COLLATION = "http://www.w3.org/2005/xpath-functions/collation/codepoint";

const QString XDT_LOCAL("http://www.w3.org/2005/xquery-local-functions");
}
}

#endif
