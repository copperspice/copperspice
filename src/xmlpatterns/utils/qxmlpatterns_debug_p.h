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

#ifndef QXmlPatterns_Debug_P_H
#define QXmlPatterns_Debug_P_H

#include <qdebug.h>

#ifdef QT_DEBUG
#   define Patternist_DEBUG_PARSER
#   define Patternist_DEBUG
#endif

namespace QPatternist {

#ifdef Patternist_DEBUG

inline QDebug pDebug()
{
   return qDebug();
}
#else

inline QNoDebug pDebug()
{
   return QNoDebug();
}
#endif

}

#endif
