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

#ifndef TABLEWIDGETEDITOR_H
#define TABLEWIDGETEDITOR_H

#include "ui_tablewidgeteditor.h"

#include "listwidgeteditor.h"

#include <QtGui/QDialog>

QT_BEGIN_NAMESPACE

class QTableWidget;
class QDesignerFormWindowInterface;

namespace qdesigner_internal {

class FormWindowBase;
class PropertySheetIconValue;

class TableWidgetEditor: public AbstractItemEditor
{
    Q_OBJECT
public:
    explicit TableWidgetEditor(QDesignerFormWindowInterface *form, QDialog *dialog);

    TableWidgetContents fillContentsFromTableWidget(QTableWidget *tableWidget);
    TableWidgetContents contents() const;

private slots:

    void on_tableWidget_currentCellChanged(int currentRow, int currnetCol, int, int);
    void on_tableWidget_itemChanged(QTableWidgetItem *item);

    void on_columnEditor_indexChanged(int idx);
    void on_columnEditor_itemChanged(int idx, int role, const QVariant &v);

    void on_columnEditor_itemInserted(int idx);
    void on_columnEditor_itemDeleted(int idx);
    void on_columnEditor_itemMovedUp(int idx);
    void on_columnEditor_itemMovedDown(int idx);

    void on_rowEditor_indexChanged(int idx);
    void on_rowEditor_itemChanged(int idx, int role, const QVariant &v);

    void on_rowEditor_itemInserted(int idx);
    void on_rowEditor_itemDeleted(int idx);
    void on_rowEditor_itemMovedUp(int idx);
    void on_rowEditor_itemMovedDown(int idx);

    void togglePropertyBrowser();

    void cacheReloaded();

protected:
    virtual void setItemData(int role, const QVariant &v);
    virtual QVariant getItemData(int role) const;

private:
    void setPropertyBrowserVisible(bool v);
    void updateEditor();
    void moveColumnsLeft(int fromColumn, int toColumn);
    void moveColumnsRight(int fromColumn, int toColumn);
    void moveRowsUp(int fromRow, int toRow);
    void moveRowsDown(int fromRow, int toRow);

    Ui::TableWidgetEditor ui;
    ItemListEditor *m_rowEditor;
    ItemListEditor *m_columnEditor;
    bool m_updatingBrowser;
};

class TableWidgetEditorDialog : public QDialog
{
    Q_OBJECT
public:
    explicit TableWidgetEditorDialog(QDesignerFormWindowInterface *form, QWidget *parent);

    TableWidgetContents fillContentsFromTableWidget(QTableWidget *tableWidget);
    TableWidgetContents contents() const;

private:
    TableWidgetEditor m_editor;
};

}  // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // TABLEWIDGETEDITOR_H
