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

#ifndef WIDGETBOXTREEWIDGET_H
#define WIDGETBOXTREEWIDGET_H

#include <qdesigner_widgetbox_p.h>

#include <QtGui/QTreeWidget>
#include <QtGui/QIcon>
#include <QtCore/QList>
#include <QtCore/QHash>
#include <QtCore/QXmlStreamReader> // Cannot forward declare them on Mac
#include <QtCore/QXmlStreamWriter>

QT_BEGIN_NAMESPACE

class QDesignerFormEditorInterface;
class QDesignerDnDItemInterface;

class QTimer;

namespace qdesigner_internal {

class WidgetBoxCategoryListView;

// WidgetBoxTreeWidget: A tree of categories

class WidgetBoxTreeWidget : public QTreeWidget
{
    Q_OBJECT

public:
    typedef QDesignerWidgetBoxInterface::Widget Widget;
    typedef QDesignerWidgetBoxInterface::Category Category;
    typedef QDesignerWidgetBoxInterface::CategoryList CategoryList;

    explicit WidgetBoxTreeWidget(QDesignerFormEditorInterface *core, QWidget *parent = 0);
    ~WidgetBoxTreeWidget();

    int categoryCount() const;
    Category category(int cat_idx) const;
    void addCategory(const Category &cat);
    void removeCategory(int cat_idx);

    int widgetCount(int cat_idx) const;
    Widget widget(int cat_idx, int wgt_idx) const;
    void addWidget(int cat_idx, const Widget &wgt);
    void removeWidget(int cat_idx, int wgt_idx);

    void dropWidgets(const QList<QDesignerDnDItemInterface*> &item_list);

    void setFileName(const QString &file_name);
    QString fileName() const;
    bool load(QDesignerWidgetBox::LoadMode loadMode);
    bool loadContents(const QString &contents);
    bool save();
    QIcon iconForWidget(QString iconName) const;

signals:
    void pressed(const QString name, const QString dom_xml, const QPoint &global_mouse_pos);

public slots:
    void filter(const QString &);

protected:
    void contextMenuEvent(QContextMenuEvent *e);
    void resizeEvent(QResizeEvent *e);

private slots:
    void slotSave();
    void slotScratchPadItemDeleted();
    void slotLastScratchPadItemDeleted();

    void handleMousePress(QTreeWidgetItem *item);
    void deleteScratchpad();
    void slotListMode();
    void slotIconMode();

private:
    WidgetBoxCategoryListView *addCategoryView(QTreeWidgetItem *parent, bool iconMode);
    WidgetBoxCategoryListView *categoryViewAt(int idx) const;
    void adjustSubListSize(QTreeWidgetItem *cat_item);

    static bool readCategories(const QString &fileName, const QString &xml, CategoryList *cats, QString *errorMessage);
    static bool readWidget(Widget *w, const QString &xml, QXmlStreamReader &r);

    CategoryList loadCustomCategoryList() const;
    void writeCategories(QXmlStreamWriter &writer, const CategoryList &cat_list) const;

    int indexOfCategory(const QString &name) const;
    int indexOfScratchpad() const;
    int ensureScratchpad();
    void addCustomCategories(bool replace);

    void saveExpandedState() const;
    void restoreExpandedState();
    void updateViewMode();

    QDesignerFormEditorInterface *m_core;
    QString m_file_name;
    typedef QHash<QString, QIcon> IconCache;
    mutable IconCache m_pluginIcons;
    bool m_iconMode;
    QTimer *m_scratchPadDeleteTimer;
};

}  // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // WIDGETBOXTREEWIDGET_H
