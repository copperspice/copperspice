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

#include "qhelpprojectdata_p.h"

#include <QtCore/QCoreApplication>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QStack>
#include <QtCore/QMap>
#include <QtCore/QRegExp>
#include <QtCore/QUrl>
#include <QtCore/QVariant>
#include <QtXml/QXmlStreamReader>

QT_BEGIN_NAMESPACE

class QHelpProjectDataPrivate : public QXmlStreamReader
{
public:
    void readData(const QByteArray &contents);

    QString virtualFolder;
    QString namespaceName;
    QString rootPath;

    QStringList fileList;
    QList<QHelpDataCustomFilter> customFilterList;
    QList<QHelpDataFilterSection> filterSectionList;
    QMap<QString, QVariant> metaData;

    QString errorMsg;

private:
    void readProject();
    void readCustomFilter();
    void readFilterSection();
    void readTOC();
    void readKeywords();
    void readFiles();
    void raiseUnknownTokenError();
    void addMatchingFiles(const QString &pattern);
    bool hasValidSyntax(const QString &nameSpace, const QString &vFolder) const;

    QMap<QString, QStringList> dirEntriesCache;
};

void QHelpProjectDataPrivate::raiseUnknownTokenError()
{
    raiseError(QCoreApplication::translate("QHelpProject", "Unknown token."));
}

void QHelpProjectDataPrivate::readData(const QByteArray &contents)
{
    addData(contents);
    while (!atEnd()) {
        readNext();
        if (isStartElement()) {
            if (name() == QLatin1String("QtHelpProject")
                && attributes().value(QLatin1String("version")) == QLatin1String("1.0"))
                readProject();
            else
                raiseError(QCoreApplication::translate("QHelpProject",
                               "Unknown token. Expected \"QtHelpProject\"!"));
        }
    }

    if (hasError()) {
        raiseError(QCoreApplication::translate("QHelpProject",
                       "Error in line %1: %2").arg(lineNumber())
            .arg(errorString()));
    }
}

void QHelpProjectDataPrivate::readProject()
{
    while (!atEnd()) {
        readNext();
        if (isStartElement()) {
            if (name() == QLatin1String("virtualFolder")) {
                virtualFolder = readElementText();
                if (!hasValidSyntax(QLatin1String("test"), virtualFolder))
                    raiseError(QCoreApplication::translate("QHelpProject",
                                   "Virtual folder has invalid syntax."));
            } else if (name() == QLatin1String("namespace")) {
                namespaceName = readElementText();
                if (!hasValidSyntax(namespaceName, QLatin1String("test")))
                    raiseError(QCoreApplication::translate("QHelpProject",
                                   "Namespace has invalid syntax."));
            } else if (name() == QLatin1String("customFilter")) {
                readCustomFilter();
            } else if (name() == QLatin1String("filterSection")) {
                readFilterSection();
            } else if (name() == QLatin1String("metaData")) {
                QString n = attributes().value(QLatin1String("name")).toString();
                if (!metaData.contains(n))
                    metaData[n]
                        = attributes().value(QLatin1String("value")).toString();
                else
                    metaData.insert(n, attributes().
                                    value(QLatin1String("value")).toString());
            } else {
                raiseUnknownTokenError();
            }
        } else if (isEndElement() && name() == QLatin1String("QtHelpProject")) {
            if (namespaceName.isEmpty())
                raiseError(QCoreApplication::translate("QHelpProject",
                              "Missing namespace in QtHelpProject."));
            else if (virtualFolder.isEmpty())
                raiseError(QCoreApplication::translate("QHelpProject",
                               "Missing virtual folder in QtHelpProject"));
            break;
        }
    }
}

void QHelpProjectDataPrivate::readCustomFilter()
{
    QHelpDataCustomFilter filter;
    filter.name = attributes().value(QLatin1String("name")).toString();
    while (!atEnd()) {
        readNext();
        if (isStartElement()) {
            if (name() == QLatin1String("filterAttribute"))
                filter.filterAttributes.append(readElementText());
            else
                raiseUnknownTokenError();
        } else if (isEndElement() && name() == QLatin1String("customFilter")) {
            break;
        }
    }
    customFilterList.append(filter);
}

void QHelpProjectDataPrivate::readFilterSection()
{
    filterSectionList.append(QHelpDataFilterSection());
    while (!atEnd()) {
        readNext();
        if (isStartElement()) {
            if (name() == QLatin1String("filterAttribute"))
                filterSectionList.last().addFilterAttribute(readElementText());
            else if (name() == QLatin1String("toc"))
                readTOC();
            else if (name() == QLatin1String("keywords"))
                readKeywords();
            else if (name() == QLatin1String("files"))
                readFiles();
            else
                raiseUnknownTokenError();
        } else if (isEndElement() && name() == QLatin1String("filterSection")) {
            break;
        }
    }
}

void QHelpProjectDataPrivate::readTOC()
{
    QStack<QHelpDataContentItem*> contentStack;
    QHelpDataContentItem *itm = 0;
    while (!atEnd()) {
        readNext();
        if (isStartElement()) {
            if (name() == QLatin1String("section")) {
                QString title = attributes().value(QLatin1String("title")).toString();
                QString ref = attributes().value(QLatin1String("ref")).toString();
                if (contentStack.isEmpty()) {
                    itm = new QHelpDataContentItem(0, title, ref);
                    filterSectionList.last().addContent(itm);
                } else {
                    itm = new QHelpDataContentItem(contentStack.top(), title, ref);
                }
                contentStack.push(itm);
            } else {
                raiseUnknownTokenError();
            }
        } else if (isEndElement()) {
            if (name() == QLatin1String("section")) {
                contentStack.pop();
                continue;
            } else if (name() == QLatin1String("toc") && contentStack.isEmpty()) {
                break;
            } else {
                raiseUnknownTokenError();
            }
        }
    }
}

void QHelpProjectDataPrivate::readKeywords()
{
    while (!atEnd()) {
        readNext();
        if (isStartElement()) {
            if (name() == QLatin1String("keyword")) {
                if (attributes().value(QLatin1String("ref")).toString().isEmpty()
                    || (attributes().value(QLatin1String("name")).toString().isEmpty()
                    && attributes().value(QLatin1String("id")).toString().isEmpty()))
                    raiseError(QCoreApplication::translate("QHelpProject",
                                   "Missing attribute in keyword at line %1.")
                               .arg(lineNumber()));
                filterSectionList.last()
                    .addIndex(QHelpDataIndexItem(attributes().
                                  value(QLatin1String("name")).toString(),
                              attributes().value(QLatin1String("id")).toString(),
                              attributes().value(QLatin1String("ref")).toString()));
            } else {
                raiseUnknownTokenError();
            }
        } else if (isEndElement()) {
            if (name() == QLatin1String("keyword"))
                continue;
            else if (name() == QLatin1String("keywords"))
                break;
            else
                raiseUnknownTokenError();
        }
    }
}

void QHelpProjectDataPrivate::readFiles()
{
    while (!atEnd()) {
        readNext();
        if (isStartElement()) {
            if (name() == QLatin1String("file"))
                addMatchingFiles(readElementText());
            else
                raiseUnknownTokenError();
        } else if (isEndElement()) {
            if (name() == QLatin1String("file"))
                continue;
            else if (name() == QLatin1String("files"))
                break;
            else
                raiseUnknownTokenError();
        }
    }
}

// Expand file pattern and add matches into list. If the pattern does not match
// any files, insert the pattern itself so the QHelpGenerator will emit a
// meaningful warning later.
void QHelpProjectDataPrivate::addMatchingFiles(const QString &pattern)
{
    // The pattern matching is expensive, so we skip it if no
    // wildcard symbols occur in the string.
    if (!pattern.contains('?') && !pattern.contains('*')
        && !pattern.contains('[') && !pattern.contains(']')) {
        filterSectionList.last().addFile(pattern);
        return;
    }

    QFileInfo fileInfo(rootPath + '/' + pattern);
    const QDir &dir = fileInfo.dir();
    const QString &path = dir.canonicalPath();

    // QDir::entryList() is expensive, so we cache the results.
    QMap<QString, QStringList>::ConstIterator it = dirEntriesCache.find(path);
    const QStringList &entries = it != dirEntriesCache.constEnd() ?
                                 it.value() : dir.entryList(QDir::Files);
    if (it == dirEntriesCache.constEnd())
        dirEntriesCache.insert(path, entries);

    bool matchFound = false;
#ifdef Q_OS_WIN
    Qt::CaseSensitivity cs = Qt::CaseInsensitive;
#else
    Qt::CaseSensitivity cs = Qt::CaseSensitive;
#endif
    QRegExp regExp(fileInfo.fileName(), cs, QRegExp::Wildcard);
    foreach (const QString &file, entries) {
        if (regExp.exactMatch(file)) {
            matchFound = true;
            filterSectionList.last().
                addFile(QFileInfo(pattern).dir().path() + '/' + file);
        }
    }
    if (!matchFound)
        filterSectionList.last().addFile(pattern);
}

bool QHelpProjectDataPrivate::hasValidSyntax(const QString &nameSpace,
                                             const QString &vFolder) const
{
    const QLatin1Char slash('/');
    if (nameSpace.contains(slash) || vFolder.contains(slash))
        return false;
    QUrl url;
    const QLatin1String scheme("qthelp");
    url.setScheme(scheme);
    const QString canonicalNamespace = nameSpace.toLower();
    url.setHost(canonicalNamespace);
    url.setPath(vFolder);

    const QString expectedUrl(scheme + QLatin1String("://")
        + canonicalNamespace + slash + vFolder);
    return url.isValid() && url.toString() == expectedUrl;
}

/*!
    \internal
    \class QHelpProjectData
    \since 4.4
    \brief The QHelpProjectData class stores all information found
    in a Qt help project file.

    The structure is filled with data by calling readData(). The
    specified file has to have the Qt help project file format in
    order to be read successfully. Possible reading errors can be
    retrieved by calling errorMessage().
*/

/*!
    Constructs a Qt help project data structure.
*/
QHelpProjectData::QHelpProjectData()
{
    d = new QHelpProjectDataPrivate;
}

/*!
    Destroys the help project data.
*/
QHelpProjectData::~QHelpProjectData()
{
    delete d;
}

/*!
    Reads the file \a fileName and stores the help data. The file has to
    have the Qt help project file format. Returns true if the file
    was successfully read, otherwise false.

    \sa errorMessage()
*/
bool QHelpProjectData::readData(const QString &fileName)
{
    d->rootPath = QFileInfo(fileName).absolutePath();
    QFile file(fileName);
    if (!file.open(QIODevice::ReadOnly)) {
        d->errorMsg = QCoreApplication::translate("QHelpProject",
                          "The input file %1 could not be opened!").arg(fileName);
        return false;
    }

    d->readData(file.readAll());
    return !d->hasError();
}

/*!
    Returns an error message if the reading of the Qt help project
    file failed. Otherwise, an empty QString is returned.

    \sa readData()
*/
QString QHelpProjectData::errorMessage() const
{
    if (d->hasError())
        return d->errorString();
    return d->errorMsg;
}

/*!
    \internal
*/
QString QHelpProjectData::namespaceName() const
{
    return d->namespaceName;
}

/*!
    \internal
*/
QString QHelpProjectData::virtualFolder() const
{
    return d->virtualFolder;
}

/*!
    \internal
*/
QList<QHelpDataCustomFilter> QHelpProjectData::customFilters() const
{
    return d->customFilterList;
}

/*!
    \internal
*/
QList<QHelpDataFilterSection> QHelpProjectData::filterSections() const
{
    return d->filterSectionList;
}

/*!
    \internal
*/
QMap<QString, QVariant> QHelpProjectData::metaData() const
{
    return d->metaData;
}

/*!
    \internal
*/
QString QHelpProjectData::rootPath() const
{
    return d->rootPath;
}

QT_END_NAMESPACE
