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

#ifndef QDESIGNER_RESOURCE_H
#define QDESIGNER_RESOURCE_H

#include "formeditor_global.h"
#include "qsimpleresource_p.h"

#include <QtCore/QHash>
#include <QtCore/QStack>
#include <QtCore/QList>

QT_BEGIN_NAMESPACE

class DomCustomWidget;
class DomCustomWidgets;
class DomResource;

class QDesignerContainerExtension;
class QDesignerFormEditorInterface;
class QDesignerCustomWidgetInterface;
class QDesignerWidgetDataBaseItemInterface;

class QTabWidget;
class QStackedWidget;
class QToolBox;
class QToolBar;
class QDesignerDockWidget;
class QLayoutWidget;
class QWizardPage;

namespace qdesigner_internal {

class FormWindow;

class QT_FORMEDITOR_EXPORT QDesignerResource : public QEditorFormBuilder
{
public:
    explicit QDesignerResource(FormWindow *fw);
    virtual ~QDesignerResource();

    virtual void save(QIODevice *dev, QWidget *widget);

    virtual bool copy(QIODevice *dev, const FormBuilderClipboard &selection);
    virtual DomUI *copy(const FormBuilderClipboard &selection);

    virtual FormBuilderClipboard paste(DomUI *ui, QWidget *widgetParent, QObject *actionParent = 0);
    virtual FormBuilderClipboard paste(QIODevice *dev,  QWidget *widgetParent, QObject *actionParent = 0);

    bool saveRelative() const;
    void setSaveRelative(bool relative);

    virtual QWidget *load(QIODevice *dev, QWidget *parentWidget = 0);

protected:
    using QEditorFormBuilder::create;
    using QEditorFormBuilder::createDom;

    virtual void saveDom(DomUI *ui, QWidget *widget);
    virtual QWidget *create(DomUI *ui, QWidget *parentWidget);
    virtual QWidget *create(DomWidget *ui_widget, QWidget *parentWidget);
    virtual QLayout *create(DomLayout *ui_layout, QLayout *layout, QWidget *parentWidget);
    virtual QLayoutItem *create(DomLayoutItem *ui_layoutItem, QLayout *layout, QWidget *parentWidget);
    virtual void applyProperties(QObject *o, const QList<DomProperty*> &properties);
    virtual QList<DomProperty*> computeProperties(QObject *obj);
    virtual DomProperty *createProperty(QObject *object, const QString &propertyName, const QVariant &value);

    virtual QWidget *createWidget(const QString &widgetName, QWidget *parentWidget, const QString &name);
    virtual QLayout *createLayout(const QString &layoutName, QObject *parent, const QString &name);
    virtual void createCustomWidgets(DomCustomWidgets *);
    virtual void createResources(DomResources*);
    virtual void applyTabStops(QWidget *widget, DomTabStops *tabStops);

    virtual bool addItem(DomLayoutItem *ui_item, QLayoutItem *item, QLayout *layout);
    virtual bool addItem(DomWidget *ui_widget, QWidget *widget, QWidget *parentWidget);

    virtual DomWidget *createDom(QWidget *widget, DomWidget *ui_parentWidget, bool recursive = true);
    virtual DomLayout *createDom(QLayout *layout, DomLayout *ui_layout, DomWidget *ui_parentWidget);
    virtual DomLayoutItem *createDom(QLayoutItem *item, DomLayout *ui_layout, DomWidget *ui_parentWidget);

    virtual QAction *create(DomAction *ui_action, QObject *parent);
    virtual QActionGroup *create(DomActionGroup *ui_action_group, QObject *parent);
    virtual void addMenuAction(QAction *action);

    virtual DomAction *createDom(QAction *action);
    virtual DomActionGroup *createDom(QActionGroup *actionGroup);
    virtual DomActionRef *createActionRefDom(QAction *action);

    virtual QAction *createAction(QObject *parent, const QString &name);
    virtual QActionGroup *createActionGroup(QObject *parent, const QString &name);

    virtual bool checkProperty(QObject *obj, const QString &prop) const;

    DomWidget *saveWidget(QTabWidget *widget, DomWidget *ui_parentWidget);
    DomWidget *saveWidget(QStackedWidget *widget, DomWidget *ui_parentWidget);
    DomWidget *saveWidget(QToolBox *widget, DomWidget *ui_parentWidget);
    DomWidget *saveWidget(QWidget *widget, QDesignerContainerExtension *container, DomWidget *ui_parentWidget);
    DomWidget *saveWidget(QToolBar *toolBar, DomWidget *ui_parentWidget);
    DomWidget *saveWidget(QDesignerDockWidget *dockWidget, DomWidget *ui_parentWidget);
    DomWidget *saveWidget(QWizardPage *wizardPage, DomWidget *ui_parentWidget);

    virtual DomCustomWidgets *saveCustomWidgets();
    virtual DomTabStops *saveTabStops();
    virtual DomResources *saveResources();

    virtual void layoutInfo(DomLayout *layout, QObject *parent, int *margin, int *spacing);

    virtual void loadExtraInfo(DomWidget *ui_widget, QWidget *widget, QWidget *parentWidget);

    void changeObjectName(QObject *o, QString name);
    DomProperty *applyProperStdSetAttribute(QObject *object, const QString &propertyName, DomProperty *property);

private:
    void addUserDefinedScripts(QWidget *w, DomWidget *ui_widget);
    DomResources *saveResources(const QStringList &qrcPaths);
    bool canCompressMargins(QObject *object) const;
    bool canCompressSpacings(QObject *object) const;
    QStringList mergeWithLoadedPaths(const QStringList &paths) const;
    void applyAttributesToPropertySheet(const DomWidget *ui_widget, QWidget *widget);

    typedef QList<DomCustomWidget*> DomCustomWidgetList;
    void addCustomWidgetsToWidgetDatabase(DomCustomWidgetList& list);
    FormWindow *m_formWindow;
    bool m_isMainWidget;
    QHash<QString, QString> m_internal_to_qt;
    QHash<QString, QString> m_qt_to_internal;
    QStack<QLayout*> m_chain;
    QHash<QDesignerWidgetDataBaseItemInterface*, bool> m_usedCustomWidgets;
    int m_topLevelSpacerCount;
    bool m_copyWidget;
    QWidget *m_selected;
    class QDesignerResourceBuilder *m_resourceBuilder;
};

}  // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // QDESIGNER_RESOURCE_H
