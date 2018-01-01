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

#ifndef QDESIGNER_TABWIDGET_H
#define QDESIGNER_TABWIDGET_H

#include "shared_global_p.h"
#include "qdesigner_propertysheet_p.h"
#include "qdesigner_utils_p.h"

#include <QtCore/QPointer>
#include <QtGui/QIcon>

QT_BEGIN_NAMESPACE

class QDesignerFormWindowInterface;
class QTabWidget;
class QTabBar;
class QMenu;
class QAction;

namespace qdesigner_internal {
    class PromotionTaskMenu;
}

class QDESIGNER_SHARED_EXPORT QTabWidgetEventFilter : public QObject
{
    Q_OBJECT
public:
    explicit QTabWidgetEventFilter(QTabWidget *parent);
    ~QTabWidgetEventFilter();

    // Install helper on QTabWidget
    static void install(QTabWidget *tabWidget);
    static QTabWidgetEventFilter *eventFilterOf(const QTabWidget *tabWidget);
    // Convenience to add a menu on a tackedWidget
    static QMenu *addTabWidgetContextMenuActions(const QTabWidget *tabWidget, QMenu *popup);

    // Add context menu and return page submenu or 0.
    QMenu *addContextMenuActions(QMenu *popup);

    virtual bool eventFilter(QObject *o, QEvent *e);

    QDesignerFormWindowInterface *formWindow() const;

private slots:
    void removeCurrentPage();
    void addPage();
    void addPageAfter();

private:
    int pageFromPosition(const QPoint &pos, QRect &rect) const;
    QTabBar *tabBar() const;

    QTabWidget *m_tabWidget;
    mutable QPointer<QTabBar> m_cachedTabBar;
    QPoint m_pressPoint;
    QWidget *m_dropIndicator;
    int m_dragIndex;
    QWidget *m_dragPage;
    QString m_dragLabel;
    QIcon m_dragIcon;
    bool m_mousePressed;
    QAction *m_actionDeletePage;
    QAction *m_actionInsertPage;
    QAction *m_actionInsertPageAfter;
    qdesigner_internal::PromotionTaskMenu* m_pagePromotionTaskMenu;
};

// PropertySheet to handle the page properties
class QDESIGNER_SHARED_EXPORT QTabWidgetPropertySheet : public QDesignerPropertySheet {
public:
    explicit QTabWidgetPropertySheet(QTabWidget *object, QObject *parent = 0);

    virtual void setProperty(int index, const QVariant &value);
    virtual QVariant property(int index) const;
    virtual bool reset(int index);
    virtual bool isEnabled(int index) const;

    // Check whether the property is to be saved. Returns false for the page
    // properties (as the property sheet has no concept of 'stored')
    static bool checkProperty(const QString &propertyName);

private:
    enum TabWidgetProperty { PropertyCurrentTabText, PropertyCurrentTabName, PropertyCurrentTabIcon,
                             PropertyCurrentTabToolTip, PropertyCurrentTabWhatsThis, PropertyTabWidgetNone };

    static TabWidgetProperty tabWidgetPropertyFromName(const QString &name);
    QTabWidget *m_tabWidget;
    struct PageData
    {
    qdesigner_internal::PropertySheetStringValue text;
    qdesigner_internal::PropertySheetStringValue tooltip;
    qdesigner_internal::PropertySheetStringValue whatsthis;
    qdesigner_internal::PropertySheetIconValue icon;
    };
    QMap<QWidget *, PageData> m_pageToData;
};

typedef QDesignerPropertySheetFactory<QTabWidget, QTabWidgetPropertySheet> QTabWidgetPropertySheetFactory;

QT_END_NAMESPACE

#endif // QDESIGNER_TABWIDGET_H
