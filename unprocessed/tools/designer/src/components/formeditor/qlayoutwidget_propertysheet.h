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

#ifndef QLAYOUTWIDGET_PROPERTYSHEET_H
#define QLAYOUTWIDGET_PROPERTYSHEET_H

#include <qdesigner_propertysheet_p.h>
#include <extensionfactory_p.h>
#include <qlayout_widget_p.h>

QT_BEGIN_NAMESPACE

namespace qdesigner_internal {

class QLayoutWidgetPropertySheet: public QDesignerPropertySheet
{
    Q_OBJECT
    Q_INTERFACES(QDesignerPropertySheetExtension)
public:
    explicit QLayoutWidgetPropertySheet(QLayoutWidget *object, QObject *parent = 0);
    virtual ~QLayoutWidgetPropertySheet();

    virtual void setProperty(int index, const QVariant &value);
    virtual bool isVisible(int index) const;

    virtual bool dynamicPropertiesAllowed() const;
};

typedef QDesignerPropertySheetFactory<QLayoutWidget, QLayoutWidgetPropertySheet> QLayoutWidgetPropertySheetFactory;
}  // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // QLAYOUTWIDGET_PROPERTYSHEET_H
