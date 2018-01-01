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

#include "qclucenefieldnames_p.h"

QT_BEGIN_NAMESPACE

namespace fulltextsearch {
namespace clucene {
const QString AttributeField(QLatin1String("attribute"));
const QString ContentField(QLatin1String("content"));
const QString NamespaceField(QLatin1String("namespace"));
const QString PathField(QLatin1String("path"));
const QString TitleField(QLatin1String("title"));
const QString TitleTokenizedField(QLatin1String("titleTokenized"));
} // namespace clucene
} // namespace fulltextsearch

QT_END_NAMESPACE
