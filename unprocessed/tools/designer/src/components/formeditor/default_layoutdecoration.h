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

#ifndef DEFAULT_LAYOUTDECORATION_H
#define DEFAULT_LAYOUTDECORATION_H

#include "formeditor_global.h"
#include <QtDesigner/QDesignerLayoutDecorationExtension>
#include <QtDesigner/default_extensionfactory.h>

QT_BEGIN_NAMESPACE

class QDesignerFormWindowInterface;

namespace qdesigner_internal {
class QDesignerLayoutDecorationFactory: public QExtensionFactory
{
    Q_OBJECT
    Q_INTERFACES(QAbstractExtensionFactory)
public:
    explicit QDesignerLayoutDecorationFactory(QExtensionManager *parent = 0);

protected:
    virtual QObject *createExtension(QObject *object, const QString &iid, QObject *parent) const;
};

}  // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // DEFAULT_LAYOUTDECORATION_H
