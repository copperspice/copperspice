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

#ifndef TREEWIDGETEDITOR_H
#define TREEWIDGETEDITOR_H

#include "ui_treewidgeteditor.h"

#include "listwidgeteditor.h"

#include <QtGui/QDialog>

QT_BEGIN_NAMESPACE

class QTreeWidget;
class QDesignerFormWindowInterface;

namespace qdesigner_internal {

class FormWindowBase;
class PropertySheetIconValue;

class TreeWidgetEditor: public AbstractItemEditor
{
    Q_OBJECT
public:
    explicit TreeWidgetEditor(QDesignerFormWindowInterface *form, QDialog *dialog);

    TreeWidgetContents fillContentsFromTreeWidget(QTreeWidget *treeWidget);
    TreeWidgetContents contents() const;

private slots:
    void on_newItemButton_clicked();
    void on_newSubItemButton_clicked();
    void on_deleteItemButton_clicked();
    void on_moveItemUpButton_clicked();
    void on_moveItemDownButton_clicked();
    void on_moveItemRightButton_clicked();
    void on_moveItemLeftButton_clicked();

    void on_treeWidget_currentItemChanged();
    void on_treeWidget_itemChanged(QTreeWidgetItem *item, int column);

    void on_columnEditor_indexChanged(int idx);
    void on_columnEditor_itemChanged(int idx, int role, const QVariant &v);

    void on_columnEditor_itemInserted(int idx);
    void on_columnEditor_itemDeleted(int idx);
    void on_columnEditor_itemMovedUp(int idx);
    void on_columnEditor_itemMovedDown(int idx);

    void togglePropertyBrowser();
    void cacheReloaded();

protected:
    virtual void setItemData(int role, const QVariant &v);
    virtual QVariant getItemData(int role) const;

private:
    void setPropertyBrowserVisible(bool v);
    QtVariantProperty *setupPropertyGroup(const QString &title, PropertyDefinition *propDefs);
    void updateEditor();
    void moveColumnItems(const PropertyDefinition *propList, QTreeWidgetItem *item, int fromColumn, int toColumn, int step);
    void moveColumns(int fromColumn, int toColumn, int step);
    void moveColumnsLeft(int fromColumn, int toColumn);
    void moveColumnsRight(int fromColumn, int toColumn);
    void closeEditors();

    Ui::TreeWidgetEditor ui;
    ItemListEditor *m_columnEditor;
    bool m_updatingBrowser;
};

class TreeWidgetEditorDialog : public QDialog
{
    Q_OBJECT
public:
    explicit TreeWidgetEditorDialog(QDesignerFormWindowInterface *form, QWidget *parent);

    TreeWidgetContents fillContentsFromTreeWidget(QTreeWidget *treeWidget);
    TreeWidgetContents contents() const;

private:
    TreeWidgetEditor m_editor;
};

}  // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // TREEWIDGETEDITOR_H
