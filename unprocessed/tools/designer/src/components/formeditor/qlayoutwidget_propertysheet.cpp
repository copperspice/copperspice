/***********************************************************************
*
* Copyright (c) 2012-2013 Barbara Geller
* Copyright (c) 2012-2013 Ansel Sermersheim
* Copyright (c) 2011 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or 
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see 
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#include "qlayoutwidget_propertysheet.h"
#include "qlayout_widget_p.h"
#include "formwindow.h"
#include "formeditor.h"

#include <QtDesigner/QExtensionManager>

#include <QtGui/QLayout>

QT_BEGIN_NAMESPACE

using namespace qdesigner_internal;

QLayoutWidgetPropertySheet::QLayoutWidgetPropertySheet(QLayoutWidget *object, QObject *parent)
    : QDesignerPropertySheet(object, parent)
{
    clearFakeProperties();
}

QLayoutWidgetPropertySheet::~QLayoutWidgetPropertySheet()
{
}

bool QLayoutWidgetPropertySheet::isVisible(int index) const
{
    static const QString layoutPropertyGroup = QLatin1String("Layout");
    if (propertyGroup(index) == layoutPropertyGroup)
        return QDesignerPropertySheet::isVisible(index);
    return false;
}

void QLayoutWidgetPropertySheet::setProperty(int index, const QVariant &value)
{
    QDesignerPropertySheet::setProperty(index, value);
}

bool QLayoutWidgetPropertySheet::dynamicPropertiesAllowed() const
{
    return false;
}

QT_END_NAMESPACE
