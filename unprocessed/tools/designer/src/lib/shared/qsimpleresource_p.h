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

#ifndef QSIMPLERESOURCE_H
#define QSIMPLERESOURCE_H

#include "shared_global_p.h"
#include "abstractformbuilder.h"
#include <QtCore/QStringList>

QT_BEGIN_NAMESPACE

class DomScript;
class DomCustomWidgets;
class DomCustomWidget;
class DomSlots;

class QDesignerFormEditorInterface;

namespace qdesigner_internal {

class WidgetDataBaseItem;

class QDESIGNER_SHARED_EXPORT QSimpleResource : public QAbstractFormBuilder
{
public:
    explicit QSimpleResource(QDesignerFormEditorInterface *core);
    virtual ~QSimpleResource();

    QBrush setupBrush(DomBrush *brush);
    DomBrush *saveBrush(const QBrush &brush);

    inline QDesignerFormEditorInterface *core() const
    { return m_core; }

    // Query extensions for additional data
    static void addExtensionDataToDOM(QAbstractFormBuilder *afb,
                                      QDesignerFormEditorInterface *core,
                                      DomWidget *ui_widget, QWidget *widget);
    static void applyExtensionDataFromDOM(QAbstractFormBuilder *afb,
                                          QDesignerFormEditorInterface *core,
                                          DomWidget *ui_widget, QWidget *widget,
                                          bool applyState);
    // Enable warnings while saving. Turn off for backups.
    static bool setWarningsEnabled(bool warningsEnabled);
    static bool warningsEnabled();
    // Return the script returned by the CustomWidget codeTemplate API
    static QString customWidgetScript(QDesignerFormEditorInterface *core, QObject *object);
    static QString customWidgetScript(QDesignerFormEditorInterface *core, const QString &className);
    static bool hasCustomWidgetScript(QDesignerFormEditorInterface *core, QObject *object);

    // Implementation for FormBuilder::createDomCustomWidgets() that adds
    // the custom widgets to the widget database
    static void handleDomCustomWidgets(const QDesignerFormEditorInterface *core,
                                       const DomCustomWidgets *dom_custom_widgets);

protected:
    virtual QIcon nameToIcon(const QString &filePath, const QString &qrcPath);
    virtual QString iconToFilePath(const QIcon &pm) const;
    virtual QString iconToQrcPath(const QIcon &pm) const;
    virtual QPixmap nameToPixmap(const QString &filePath, const QString &qrcPath);
    virtual QString pixmapToFilePath(const QPixmap &pm) const;
    virtual QString pixmapToQrcPath(const QPixmap &pm) const;

    enum ScriptSource { ScriptDesigner, ScriptExtension, ScriptCustomWidgetPlugin };
    static DomScript*createScript(const QString &script, ScriptSource source);
    typedef QList<DomScript*> DomScripts;
    static void addScript(const QString &script, ScriptSource source, DomScripts &domScripts);

    static bool addFakeMethods(const DomSlots *domSlots, QStringList &fakeSlots, QStringList &fakeSignals);

private:
    static void addCustomWidgetsToWidgetDatabase(const QDesignerFormEditorInterface *core,
                                                 QList<DomCustomWidget*>& custom_widget_list);
    static void addFakeMethodsToWidgetDataBase(const DomCustomWidget *domCustomWidget, WidgetDataBaseItem *item);

    static bool m_warningsEnabled;
    QDesignerFormEditorInterface *m_core;
};

// Contents of clipboard for formbuilder copy and paste operations
// (Actions and widgets)
struct QDESIGNER_SHARED_EXPORT FormBuilderClipboard {
    typedef QList<QAction*> ActionList;

    FormBuilderClipboard() {}
    FormBuilderClipboard(QWidget *w);

    bool empty() const;

    QWidgetList m_widgets;
    ActionList m_actions;
};

// Base class for a form builder used in the editor that
// provides copy and paste.(move into base interface)
class QDESIGNER_SHARED_EXPORT QEditorFormBuilder : public QSimpleResource
{
public:
    explicit QEditorFormBuilder(QDesignerFormEditorInterface *core) : QSimpleResource(core) {}

    virtual bool copy(QIODevice *dev, const FormBuilderClipboard &selection) = 0;
    virtual DomUI *copy(const FormBuilderClipboard &selection) = 0;

    // A widget parent needs to be specified, otherwise, the widget factory cannot locate the form window via parent
    // and thus is not able to construct special widgets (QLayoutWidget).
    virtual FormBuilderClipboard paste(DomUI *ui, QWidget *widgetParent, QObject *actionParent = 0) = 0;
    virtual FormBuilderClipboard paste(QIODevice *dev, QWidget *widgetParent, QObject *actionParent = 0) = 0;
};

} // namespace qdesigner_internal

QT_END_NAMESPACE

#endif
