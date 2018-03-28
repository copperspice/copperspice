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

#ifndef QJSONWRITER_P_H
#define QJSONWRITER_P_H

#include <qjsondocument.h>
#include <qjsonarray.h>
#include <qjsonobject.h>
#include <qstring.h>

class QJsonWriter
{
 public:
   static QString arrayToString(const QJsonArray &data, int indent, QJsonDocument::JsonFormat format);
   static QString objectToString(const QJsonObject &data, int indent, QJsonDocument::JsonFormat format);
};

#endif
