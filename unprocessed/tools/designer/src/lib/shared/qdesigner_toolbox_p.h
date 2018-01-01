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

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of Qt Designer.  This header
// file may change from version to version without notice, or even be removed.
//
// We mean it.
//

#ifndef QDESIGNER_TOOLBOX_H
#define QDESIGNER_TOOLBOX_H

#include "shared_global_p.h"
#include "qdesigner_propertysheet_p.h"
#include "qdesigner_utils_p.h"
#include <QtGui/QPalette>

QT_BEGIN_NAMESPACE

namespace qdesigner_internal {
    class PromotionTaskMenu;
}

class QToolBox;

class QAction;
class QMenu;

class QDESIGNER_SHARED_EXPORT QToolBoxHelper : public QObject
{
    Q_OBJECT

    explicit QToolBoxHelper(QToolBox *toolbox);
public:
    // Install helper on QToolBox
    static void install(QToolBox *toolbox);
    static QToolBoxHelper *helperOf(const QToolBox *toolbox);
    // Convenience to add a menu on a toolbox
    static QMenu *addToolBoxContextMenuActions(const QToolBox *toolbox, QMenu *popup);

    QPalette::ColorRole currentItemBackgroundRole() const;
    void setCurrentItemBackgroundRole(QPalette::ColorRole role);

    bool eventFilter(QObject *watched, QEvent *event);
    // Add context menu and return page submenu or 0.

    QMenu *addContextMenuActions(QMenu *popup) const;

private slots:
    void removeCurrentPage();
    void addPage();
    void addPageAfter();
    void changeOrder();

private:
    QToolBox *m_toolbox;
    QAction *m_actionDeletePage;
    QAction *m_actionInsertPage;
    QAction *m_actionInsertPageAfter;
    QAction *m_actionChangePageOrder;
    qdesigner_internal::PromotionTaskMenu* m_pagePromotionTaskMenu;
};

// PropertySheet to handle the page properties
class QDESIGNER_SHARED_EXPORT QToolBoxWidgetPropertySheet : public QDesignerPropertySheet {
public:
    explicit QToolBoxWidgetPropertySheet(QToolBox *object, QObject *parent = 0);

    virtual void setProperty(int index, const QVariant &value);
    virtual QVariant property(int index) const;
    virtual bool reset(int index);
    virtual bool isEnabled(int index) const;

    // Check whether the property is to be saved. Returns false for the page
    // properties (as the property sheet has no concept of 'stored')
    static bool checkProperty(const QString &propertyName);

private:
    enum ToolBoxProperty { PropertyCurrentItemText, PropertyCurrentItemName, PropertyCurrentItemIcon,
                           PropertyCurrentItemToolTip,  PropertyTabSpacing, PropertyToolBoxNone };

    static ToolBoxProperty toolBoxPropertyFromName(const QString &name);
    QToolBox *m_toolBox;
    struct PageData
        {
        qdesigner_internal::PropertySheetStringValue text;
        qdesigner_internal::PropertySheetStringValue tooltip;
        qdesigner_internal::PropertySheetIconValue icon;
        };
    QMap<QWidget *, PageData> m_pageToData;
};

typedef QDesignerPropertySheetFactory<QToolBox, QToolBoxWidgetPropertySheet> QToolBoxWidgetPropertySheetFactory;

QT_END_NAMESPACE

#endif // QDESIGNER_TOOLBOX_H
