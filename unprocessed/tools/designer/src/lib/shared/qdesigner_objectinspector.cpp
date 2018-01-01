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

#include "qdesigner_objectinspector_p.h"

QT_BEGIN_NAMESPACE

namespace qdesigner_internal {

QDesignerObjectInspector::QDesignerObjectInspector(QWidget *parent, Qt::WindowFlags flags) :
    QDesignerObjectInspectorInterface(parent, flags)
{
}

void QDesignerObjectInspector::mainContainerChanged()
{
}

void Selection::clear()
{
    managed.clear();
    unmanaged.clear();
    objects.clear();
}

bool Selection::empty() const
{
    return managed.empty() && unmanaged.empty() && objects.empty();
}

QObjectList Selection::selection() const
{
    QObjectList rc(objects);
    foreach (QObject* o, managed)
        rc.push_back(o);
    foreach (QObject* o, unmanaged)
        rc.push_back(o);
    return rc;
}
}

QT_END_NAMESPACE
