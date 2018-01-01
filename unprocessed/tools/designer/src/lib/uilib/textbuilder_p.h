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

#ifndef TEXTBUILDER_H
#define TEXTBUILDER_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QtDesigner/uilib_global.h>
#include <QtCore/QList>
#include <QtCore/QString>

QT_BEGIN_NAMESPACE

class QDir;
class QVariant;

#ifdef QFORMINTERNAL_NAMESPACE
namespace QFormInternal
{
#endif

class DomProperty;
class DomString;

class QDESIGNER_UILIB_EXPORT QTextBuilder
{
public:
    QTextBuilder();
    virtual ~QTextBuilder();

    virtual QVariant loadText(const DomProperty *property) const;

    virtual QVariant toNativeValue(const QVariant &value) const;

    virtual DomProperty *saveText(const QVariant &value) const;
};


#ifdef QFORMINTERNAL_NAMESPACE
}
#endif

QT_END_NAMESPACE

#endif // TEXTBUILDER_H
