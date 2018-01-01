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

#include "line_propertysheet.h"
#include "formwindow.h"

// sdk
#include <QtDesigner/QExtensionManager>

#include <QtGui/QLayout>
#include <QtCore/QMetaObject>
#include <QtCore/QMetaProperty>
#include <QtCore/qdebug.h>

QT_BEGIN_NAMESPACE

using namespace qdesigner_internal;

LinePropertySheet::LinePropertySheet(Line *object, QObject *parent)
    : QDesignerPropertySheet(object, parent)
{
    clearFakeProperties();
}

LinePropertySheet::~LinePropertySheet()
{
}

bool LinePropertySheet::isVisible(int index) const
{
    const QString name = propertyName(index);

    if (name == QLatin1String("frameShape"))
        return false;
    return QDesignerPropertySheet::isVisible(index);
}

void LinePropertySheet::setProperty(int index, const QVariant &value)
{
    QDesignerPropertySheet::setProperty(index, value);
}

QString LinePropertySheet::propertyGroup(int index) const
{
    return QDesignerPropertySheet::propertyGroup(index);
}

QT_END_NAMESPACE
