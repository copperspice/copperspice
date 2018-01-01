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

#ifndef QHELPENGINECORE_H
#define QHELPENGINECORE_H

#include <QtHelp/qhelp_global.h>

#include <QtCore/QUrl>
#include <QtCore/QMap>
#include <QtCore/QObject>
#include <QtCore/QVariant>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Help)

class QHelpEngineCorePrivate;

class QHELP_EXPORT QHelpEngineCore : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool autoSaveFilter READ autoSaveFilter WRITE setAutoSaveFilter)
    Q_PROPERTY(QString collectionFile READ collectionFile WRITE setCollectionFile)
    Q_PROPERTY(QString currentFilter READ currentFilter WRITE setCurrentFilter)
    
public:
    explicit QHelpEngineCore(const QString &collectionFile, QObject *parent = 0);
    virtual ~QHelpEngineCore();

    bool setupData();

    QString collectionFile() const;
    void setCollectionFile(const QString &fileName);

    bool copyCollectionFile(const QString &fileName);

    static QString namespaceName(const QString &documentationFileName);
    bool registerDocumentation(const QString &documentationFileName);
    bool unregisterDocumentation(const QString &namespaceName);
    QString documentationFileName(const QString &namespaceName);

    QStringList customFilters() const;
    bool removeCustomFilter(const QString &filterName);
    bool addCustomFilter(const QString &filterName,
        const QStringList &attributes);

    QStringList filterAttributes() const;
    QStringList filterAttributes(const QString &filterName) const;

    QString currentFilter() const;
    void setCurrentFilter(const QString &filterName);

    QStringList registeredDocumentations() const;
    QList<QStringList> filterAttributeSets(const QString &namespaceName) const;
    QList<QUrl> files(const QString namespaceName,
        const QStringList &filterAttributes,
        const QString &extensionFilter = QString());
    QUrl findFile(const QUrl &url) const;
    QByteArray fileData(const QUrl &url) const;

    QMap<QString, QUrl> linksForIdentifier(const QString &id) const;

    bool removeCustomValue(const QString &key);
    QVariant customValue(const QString &key,
        const QVariant &defaultValue = QVariant()) const;
    bool setCustomValue(const QString &key, const QVariant &value);

    static QVariant metaData(const QString &documentationFileName,
        const QString &name);

    QString error() const;

    void setAutoSaveFilter(bool save);
    bool autoSaveFilter() const;

Q_SIGNALS:
    void setupStarted();
    void setupFinished();
    void currentFilterChanged(const QString &newFilter);
    void warning(const QString &msg);
    void readersAboutToBeInvalidated();

protected:
    QHelpEngineCore(QHelpEngineCorePrivate *helpEngineCorePrivate,
        QObject *parent);

private:
    QHelpEngineCorePrivate *d;
    friend class QHelpEngineCorePrivate;
};

QT_END_NAMESPACE

QT_END_HEADER

#endif // QHELPENGINECORE_H
