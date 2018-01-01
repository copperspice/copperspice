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

#ifndef PLUGINMANAGER_H
#define PLUGINMANAGER_H

#include "shared_global_p.h"
#include "shared_enums_p.h"

#include <QtCore/QSharedDataPointer>
#include <QtCore/QMap>
#include <QtCore/QPair>
#include <QtCore/QStringList>

QT_BEGIN_NAMESPACE

class QDesignerFormEditorInterface;
class QDesignerCustomWidgetInterface;
class QDesignerPluginManagerPrivate;

class QDesignerCustomWidgetSharedData;

/* Information contained in the Dom XML of a custom widget. */
class QDESIGNER_SHARED_EXPORT QDesignerCustomWidgetData {
public:
    // StringPropertyType: validation mode and translatable flag.
    typedef QPair<qdesigner_internal::TextPropertyValidationMode, bool> StringPropertyType;

    explicit QDesignerCustomWidgetData(const QString &pluginPath = QString());

    enum ParseResult { ParseOk, ParseWarning, ParseError };
    ParseResult parseXml(const QString &xml, const QString &name, QString *errorMessage);

    QDesignerCustomWidgetData(const QDesignerCustomWidgetData&);
    QDesignerCustomWidgetData& operator=(const QDesignerCustomWidgetData&);
    ~QDesignerCustomWidgetData();

    bool isNull() const;

    QString pluginPath() const;

    // Data as parsed from the widget's domXML().
    QString xmlClassName() const;
    // Optional. The language the plugin is supposed to be used with.
    QString xmlLanguage() const;
    // Optional. method used to add pages to a container with a container extension
    QString xmlAddPageMethod() const;
    // Optional. Base class
    QString xmlExtends() const;
    // Optional. The name to be used in the widget box.
    QString xmlDisplayName() const;
    // Type of a string property
    bool xmlStringPropertyType(const QString &name, StringPropertyType *type) const;

private:
    QSharedDataPointer<QDesignerCustomWidgetSharedData> m_d;
};

class QDESIGNER_SHARED_EXPORT QDesignerPluginManager: public QObject
{
    Q_OBJECT
public:
    typedef QList<QDesignerCustomWidgetInterface*> CustomWidgetList;

    explicit QDesignerPluginManager(QDesignerFormEditorInterface *core);
    virtual ~QDesignerPluginManager();

    QDesignerFormEditorInterface *core() const;

    QObject *instance(const QString &plugin) const;

    QStringList registeredPlugins() const;

    QStringList findPlugins(const QString &path);

    QStringList pluginPaths() const;
    void setPluginPaths(const QStringList &plugin_paths);

    QStringList disabledPlugins() const;
    void setDisabledPlugins(const QStringList &disabled_plugins);

    QStringList failedPlugins() const;
    QString failureReason(const QString &pluginName) const;

    QObjectList instances() const;

    CustomWidgetList registeredCustomWidgets() const;
    QDesignerCustomWidgetData customWidgetData(QDesignerCustomWidgetInterface *w) const;
    QDesignerCustomWidgetData customWidgetData(const QString &className) const;

    bool registerNewPlugins();

public slots:
    bool syncSettings();
    void ensureInitialized();

private:
    void updateRegisteredPlugins();
    void registerPath(const QString &path);
    void registerPlugin(const QString &plugin);

private:
    static QStringList defaultPluginPaths();

    QDesignerPluginManagerPrivate *m_d;
};

QT_END_NAMESPACE

#endif // PLUGINMANAGER_H
