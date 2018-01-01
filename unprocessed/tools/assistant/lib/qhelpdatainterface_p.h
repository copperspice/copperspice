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

#ifndef QHELPDATAINTERFACE_H
#define QHELPDATAINTERFACE_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API. It exists for the convenience
// of the help generator tools. This header file may change from version
// to version without notice, or even be removed.
//
// We mean it.
//

#include "qhelp_global.h"

#include <QtCore/QStringList>
#include <QtCore/QSharedData>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

class QHELP_EXPORT QHelpDataContentItem
{
public:
    QHelpDataContentItem(QHelpDataContentItem *parent, const QString &title,
        const QString &reference);
    ~QHelpDataContentItem();

    QString title() const;
    QString reference() const;
    QList<QHelpDataContentItem*> children() const;

private:
    QString m_title;
    QString m_reference;
    QList<QHelpDataContentItem*> m_children;
};

struct QHELP_EXPORT QHelpDataIndexItem {
    QHelpDataIndexItem() {}
    QHelpDataIndexItem(const QString &n, const QString &id, const QString &r)
        : name(n), identifier(id), reference(r) {}

    QString name;
    QString identifier;
    QString reference;
    
    bool operator==(const QHelpDataIndexItem & other) const;
};

class QHelpDataFilterSectionData : public QSharedData
{
public:
    ~QHelpDataFilterSectionData()
    {
        qDeleteAll(contents);
    }

    QStringList filterAttributes;
    QList<QHelpDataIndexItem> indices;
    QList<QHelpDataContentItem*> contents;
    QStringList files;
};

class QHELP_EXPORT QHelpDataFilterSection
{
public:
    QHelpDataFilterSection();

    void addFilterAttribute(const QString &filter);
    QStringList filterAttributes() const;

    void addIndex(const QHelpDataIndexItem &index);
    void setIndices(const QList<QHelpDataIndexItem> &indices);
    QList<QHelpDataIndexItem> indices() const;

    void addContent(QHelpDataContentItem *content);
    void setContents(const QList<QHelpDataContentItem*> &contents);
    QList<QHelpDataContentItem*> contents() const;

    void addFile(const QString &file);
    void setFiles(const QStringList &files);
    QStringList files() const;

private:
    QSharedDataPointer<QHelpDataFilterSectionData> d;
};

struct QHELP_EXPORT QHelpDataCustomFilter {
    QStringList filterAttributes;
    QString name;
};

class QHELP_EXPORT QHelpDataInterface
{
public:
    QHelpDataInterface() {}
    virtual ~QHelpDataInterface() {}

    virtual QString namespaceName() const = 0;
    virtual QString virtualFolder() const = 0;
    virtual QList<QHelpDataCustomFilter> customFilters() const = 0;
    virtual QList<QHelpDataFilterSection> filterSections() const = 0;
    virtual QMap<QString, QVariant> metaData() const = 0;
    virtual QString rootPath() const = 0;
};

QT_END_NAMESPACE

QT_END_HEADER

#endif // QHELPDATAINTERFACE_H
