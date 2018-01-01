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

#ifndef ITEMVIEW_PROPERTYSHEET_H
#define ITEMVIEW_PROPERTYSHEET_H

#include <qdesigner_propertysheet_p.h>
#include <extensionfactory_p.h>

#include <QtGui/QTreeView>
#include <QtGui/QTableView>

QT_BEGIN_NAMESPACE

namespace qdesigner_internal {

struct ItemViewPropertySheetPrivate;

class ItemViewPropertySheet: public QDesignerPropertySheet
{
    Q_OBJECT
    Q_INTERFACES(QDesignerPropertySheetExtension)
public:
    explicit ItemViewPropertySheet(QTreeView *treeViewObject, QObject *parent = 0);
    explicit ItemViewPropertySheet(QTableView *tableViewObject, QObject *parent = 0);
    ~ItemViewPropertySheet();

    QHash<QString,QString> propertyNameMap() const;

    // QDesignerPropertySheet
    QVariant property(int index) const;
    void setProperty(int index, const QVariant &value);

    virtual void setChanged(int index, bool changed);
    virtual bool isChanged(int index) const;

    virtual bool hasReset(int index) const;
    virtual bool reset(int index);

private:
    void initHeaderProperties(QHeaderView *hv, const QString &prefix);

    ItemViewPropertySheetPrivate *d;
};

typedef QDesignerPropertySheetFactory<QTreeView, ItemViewPropertySheet>
                                      QTreeViewPropertySheetFactory;
typedef QDesignerPropertySheetFactory<QTableView, ItemViewPropertySheet>
                                      QTableViewPropertySheetFactory;
}  // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // ITEMVIEW_PROPERTYSHEET_H
