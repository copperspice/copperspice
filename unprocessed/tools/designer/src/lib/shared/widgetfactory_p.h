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


#ifndef WIDGETFACTORY_H
#define WIDGETFACTORY_H

#include "shared_global_p.h"
#include "pluginmanager_p.h"

#include <QtDesigner/QDesignerWidgetFactoryInterface>

#include <QtCore/QMap>
#include <QtCore/QHash>
#include <QtCore/QVariant>
#include <QtCore/QPointer>

QT_BEGIN_NAMESPACE

class QObject;
class QWidget;
class QLayout;
class QDesignerFormEditorInterface;
class QDesignerCustomWidgetInterface;
class QDesignerFormWindowInterface;
class QStyle;

namespace qdesigner_internal {

class QDESIGNER_SHARED_EXPORT WidgetFactory: public QDesignerWidgetFactoryInterface
{
    Q_OBJECT
public:
    explicit WidgetFactory(QDesignerFormEditorInterface *core, QObject *parent = 0);
    ~WidgetFactory();

    virtual QWidget* containerOfWidget(QWidget *widget) const;
    virtual QWidget* widgetOfContainer(QWidget *widget) const;

    QObject* createObject(const QString &className, QObject* parent) const;

    virtual QWidget *createWidget(const QString &className, QWidget *parentWidget) const;
    virtual QLayout *createLayout(QWidget *widget, QLayout *layout, int type) const;

    virtual bool isPassiveInteractor(QWidget *widget);
    virtual void initialize(QObject *object) const;
    void initializeCommon(QWidget *object) const;
    void initializePreview(QWidget *object) const;


    virtual QDesignerFormEditorInterface *core() const;

    static QString classNameOf(QDesignerFormEditorInterface *core, const QObject* o);

    QDesignerFormWindowInterface *currentFormWindow(QDesignerFormWindowInterface *fw);

    static QLayout *createUnmanagedLayout(QWidget *parentWidget, int type);

    // The widget factory maintains a cache of styles which it owns.
    QString styleName() const;
    void setStyleName(const QString &styleName);

    /* Return a cached style matching the name or QApplication's style if
     * it is the default. */
    QStyle *getStyle(const QString &styleName);
    // Return the current style used by the factory. This either a cached one
    // or QApplication's style */
    QStyle *style() const;

    // Apply one of the cached styles or QApplication's style to a toplevel widget.
    void applyStyleTopLevel(const QString &styleName, QWidget *w);
    static void applyStyleToTopLevel(QStyle *style, QWidget *widget);

    // Return whether object was created by the factory for the form editor.
    static bool isFormEditorObject(const QObject *o);

    // Boolean dynamic property to set on widgets to prevent custom
    // styles from interfering
    static const char *disableStyleCustomPaintingPropertyC;

public slots:
    void loadPlugins();

private slots:
    void activeFormWindowChanged(QDesignerFormWindowInterface *formWindow);
    void formWindowAdded(QDesignerFormWindowInterface *formWindow);

private:
    struct Strings { // Reduce string allocations by storing predefined strings
        Strings();
        const QString m_alignment;
        const QString m_bottomMargin;
        const QString m_geometry;
        const QString m_leftMargin;
        const QString m_line;
        const QString m_objectName;
        const QString m_spacerName;
        const QString m_orientation;
        const QString m_q3WidgetStack;
        const QString m_qAction;
        const QString m_qButtonGroup;
        const QString m_qAxWidget;
        const QString m_qDialog;
        const QString m_qDockWidget;
        const QString m_qLayoutWidget;
        const QString m_qMenu;
        const QString m_qMenuBar;
        const QString m_qWidget;
        const QString m_rightMargin;
        const QString m_sizeHint;
        const QString m_spacer;
        const QString m_text;
        const QString m_title;
        const QString m_topMargin;
        const QString m_windowIcon;
        const QString m_windowTitle;
    };

    QWidget* createCustomWidget(const QString &className, QWidget *parentWidget, bool *creationError) const;
    QDesignerFormWindowInterface *findFormWindow(QWidget *parentWidget) const;
    void setFormWindowStyle(QDesignerFormWindowInterface *formWindow);

    const Strings m_strings;
    QDesignerFormEditorInterface *m_core;
    typedef QMap<QString, QDesignerCustomWidgetInterface*> CustomWidgetFactoryMap;
    CustomWidgetFactoryMap m_customFactory;
    QDesignerFormWindowInterface *m_formWindow;

    // Points to the cached style or 0 if the default (qApp) is active
    QStyle *m_currentStyle;
    typedef QHash<QString, QStyle *> StyleCache;
    StyleCache m_styleCache;

    static QPointer<QWidget> *m_lastPassiveInteractor;
    static bool m_lastWasAPassiveInteractor;
};

} // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // WIDGETFACTORY_H
