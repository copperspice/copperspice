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


#ifndef WIDGETDATABASE_H
#define WIDGETDATABASE_H

#include "shared_global_p.h"

#include <QtDesigner/QDesignerWidgetDataBaseInterface>

#include <QtGui/QIcon>
#include <QtCore/QString>
#include <QtCore/QVariant>
#include <QtCore/QPair>
#include <QtCore/QStringList>

QT_BEGIN_NAMESPACE

class QObject;
class QDesignerCustomWidgetInterface;

namespace qdesigner_internal {

class QDESIGNER_SHARED_EXPORT WidgetDataBaseItem: public QDesignerWidgetDataBaseItemInterface
{
public:
    explicit WidgetDataBaseItem(const QString &name = QString(),
                                const QString &group = QString());

    QString name() const;
    void setName(const QString &name);

    QString group() const;
    void setGroup(const QString &group);

    QString toolTip() const;
    void setToolTip(const QString &toolTip);

    QString whatsThis() const;
    void setWhatsThis(const QString &whatsThis);

    QString includeFile() const;
    void setIncludeFile(const QString &includeFile);


    QIcon icon() const;
    void setIcon(const QIcon &icon);

    bool isCompat() const;
    void setCompat(bool compat);

    bool isContainer() const;
    void setContainer(bool b);

    bool isCustom() const;
    void setCustom(bool b);

    QString pluginPath() const;
    void setPluginPath(const QString &path);

    bool isPromoted() const;
    void setPromoted(bool b);

    QString extends() const;
    void setExtends(const QString &s);

    void setDefaultPropertyValues(const QList<QVariant> &list);
    QList<QVariant> defaultPropertyValues() const;

    static WidgetDataBaseItem *clone(const QDesignerWidgetDataBaseItemInterface *item);

    QStringList fakeSlots() const;
    void setFakeSlots(const QStringList &);

    QStringList fakeSignals() const;
    void setFakeSignals(const QStringList &);

    QString addPageMethod() const;
    void setAddPageMethod(const QString &m);

private:
    QString m_name;
    QString m_group;
    QString m_toolTip;
    QString m_whatsThis;
    QString m_includeFile;
    QString m_pluginPath;
    QString m_extends;
    QString m_addPageMethod;
    QIcon m_icon;
    uint m_compat: 1;
    uint m_container: 1;
    uint m_form: 1;
    uint m_custom: 1;
    uint m_promoted: 1;
    QList<QVariant> m_defaultPropertyValues;
    QStringList m_fakeSlots;
    QStringList m_fakeSignals;
};

enum IncludeType { IncludeLocal, IncludeGlobal  };

typedef  QPair<QString, IncludeType> IncludeSpecification;

QDESIGNER_SHARED_EXPORT IncludeSpecification  includeSpecification(QString includeFile);
QDESIGNER_SHARED_EXPORT QString buildIncludeFile(QString includeFile, IncludeType includeType);

class QDESIGNER_SHARED_EXPORT WidgetDataBase: public QDesignerWidgetDataBaseInterface
{
    Q_OBJECT
public:
    explicit WidgetDataBase(QDesignerFormEditorInterface *core, QObject *parent = 0);
    virtual ~WidgetDataBase();

    virtual QDesignerFormEditorInterface *core() const;

    virtual int indexOfObject(QObject *o, bool resolveName = true) const;

    void remove(int index);


    void grabDefaultPropertyValues();
    void grabStandardWidgetBoxIcons();

    // Helpers for 'New Form' wizards in integrations. Obtain a list of suitable classes and generate XML for them.
    static QStringList formWidgetClasses(const QDesignerFormEditorInterface *core);
    static QStringList customFormWidgetClasses(const QDesignerFormEditorInterface *core);
    static QString formTemplate(const QDesignerFormEditorInterface *core, const QString &className, const QString &objectName);

    // Helpers for 'New Form' wizards: Set a fixed size on a XML form template
    static QString scaleFormTemplate(const QString &xml, const QSize &size, bool fixed);

public slots:
    void loadPlugins();

private:
    QList<QVariant> defaultPropertyValues(const QString &name);

    QDesignerFormEditorInterface *m_core;
};

QDESIGNER_SHARED_EXPORT QDesignerWidgetDataBaseItemInterface
        *appendDerived(QDesignerWidgetDataBaseInterface *db,
                       const QString &className,
                       const QString &group,
                       const QString &baseClassName,
                       const QString &includeFile,
                       bool promoted,
                       bool custom);

typedef  QList<QDesignerWidgetDataBaseItemInterface*> WidgetDataBaseItemList;

QDESIGNER_SHARED_EXPORT WidgetDataBaseItemList
        promotionCandidates(const QDesignerWidgetDataBaseInterface *db,
                            const QString &baseClassName);
} // namespace qdesigner_internal

QT_END_NAMESPACE

#endif // WIDGETDATABASE_H
