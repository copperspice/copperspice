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

#ifndef ITEMLISTEDITOR_H
#define ITEMLISTEDITOR_H

#include "ui_itemlisteditor.h"

#include <QtGui/QDialog>

QT_BEGIN_NAMESPACE

class QDesignerFormWindowInterface;
class QtProperty;
class QtVariantProperty;
class QtTreePropertyBrowser;
class QSplitter;
class QVBoxLayout;

namespace qdesigner_internal {

class DesignerIconCache;
class DesignerPropertyManager;
class DesignerEditorFactory;

// Utility class that ensures a bool is true while in scope.
// Courtesy of QBoolBlocker in qobject_p.h
class BoolBlocker
{
public:
    inline BoolBlocker(bool &b):block(b), reset(b){block = true;}
    inline ~BoolBlocker(){block = reset; }
private:
    bool &block;
    bool reset;
};

class AbstractItemEditor: public QWidget
{
    Q_OBJECT

public:
    explicit AbstractItemEditor(QDesignerFormWindowInterface *form, QWidget *parent);
    ~AbstractItemEditor();

    DesignerIconCache *iconCache() const { return m_iconCache; }

    struct PropertyDefinition {
        int role;
        int type;
        int (*typeFunc)();
        const char *name;
    };

private slots:
    void propertyChanged(QtProperty *property);
    void resetProperty(QtProperty *property);
    void cacheReloaded();

protected:
    void setupProperties(PropertyDefinition *propDefs);
    void setupObject(QWidget *object);
    void setupEditor(QWidget *object, PropertyDefinition *propDefs);
    void injectPropertyBrowser(QWidget *parent, QWidget *widget);
    void updateBrowser();
    virtual void setItemData(int role, const QVariant &v) = 0;
    virtual QVariant getItemData(int role) const = 0;

    DesignerIconCache *m_iconCache;
    DesignerPropertyManager *m_propertyManager;
    DesignerEditorFactory *m_editorFactory;
    QSplitter *m_propertySplitter;
    QtTreePropertyBrowser *m_propertyBrowser;
    QList<QtVariantProperty*> m_properties;
    QList<QtVariantProperty*> m_rootProperties;
    QHash<QtVariantProperty*, int> m_propertyToRole;
    bool m_updatingBrowser;
};

class ItemListEditor: public AbstractItemEditor
{
    Q_OBJECT

public:
    explicit ItemListEditor(QDesignerFormWindowInterface *form, QWidget *parent);

    void setupEditor(QWidget *object, PropertyDefinition *propDefs);
    QListWidget *listWidget() const { return ui.listWidget; }
    void setNewItemText(const QString &tpl) { m_newItemText = tpl; }
    QString newItemText() const { return m_newItemText; }
    void setCurrentIndex(int idx);

signals:
    void indexChanged(int idx);
    void itemChanged(int idx, int role, const QVariant &v);
    void itemInserted(int idx);
    void itemDeleted(int idx);
    void itemMovedUp(int idx);
    void itemMovedDown(int idx);

private slots:
    void on_newListItemButton_clicked();
    void on_deleteListItemButton_clicked();
    void on_moveListItemUpButton_clicked();
    void on_moveListItemDownButton_clicked();
    void on_listWidget_currentRowChanged();
    void on_listWidget_itemChanged(QListWidgetItem * item);
    void togglePropertyBrowser();
    void cacheReloaded();

protected:
    virtual void setItemData(int role, const QVariant &v);
    virtual QVariant getItemData(int role) const;

private:
    void setPropertyBrowserVisible(bool v);
    void updateEditor();
    Ui::ItemListEditor ui;
    bool m_updating;
    QString m_newItemText;
};

}  // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // ITEMLISTEDITOR_H
