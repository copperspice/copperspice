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

#include "qhelpdatainterface_p.h"

QT_BEGIN_NAMESPACE

/*!
    \internal
    \class QHelpDataContentItem
    \since 4.4
    \brief The QHelpDataContentItem class provides an item which represents
    a topic or section of the contents.

    Every item holds several pieces of information, most notably the title
    which can later be displayed in a contents overview. The reference is used
    to store a relative file link to the corresponding section in the
    documentation.
*/

/*!
    Constructs a new content item with \a parent as parent item.
    The constucted item has the title \a title and links to the
    location specified by \a reference.
*/
QHelpDataContentItem::QHelpDataContentItem(QHelpDataContentItem *parent,
                                           const QString &title, const QString &reference)
    : m_title(title), m_reference(reference)
{
    if (parent)
        parent->m_children.append(this);
}

/*!
    Destructs the item and its children.
*/
QHelpDataContentItem::~QHelpDataContentItem()
{
    qDeleteAll(m_children);
}

/*!
    Returns the title of the item.
*/
QString QHelpDataContentItem::title() const
{
    return m_title;
}

/*!
    Returns the file reference of the item.
*/
QString QHelpDataContentItem::reference() const
{
    return m_reference;
}

/*!
    Returns a list of all its child items.
*/
QList<QHelpDataContentItem*> QHelpDataContentItem::children() const
{
    return m_children;
}

bool QHelpDataIndexItem::operator==(const QHelpDataIndexItem & other) const
{
    return (other.name == name) 
            && (other.reference == reference);
}



/*!
    \internal
    \class QHelpDataFilterSection
    \since 4.4
*/

/*!
    Constructs a help data filter section.
*/
QHelpDataFilterSection::QHelpDataFilterSection()
{
    d = new QHelpDataFilterSectionData();
}

/*!
    Adds the filter attribute \a filter to the filter attributes of
    this section.
*/
void QHelpDataFilterSection::addFilterAttribute(const QString &filter)
{
    d->filterAttributes.append(filter);
}

/*!
    Returns a list of all filter attributes defined for this section.
*/
QStringList QHelpDataFilterSection::filterAttributes() const
{
    return d->filterAttributes;
}

/*!
    Adds the index item \a index to the list of indices.
*/
void QHelpDataFilterSection::addIndex(const QHelpDataIndexItem &index)
{
    d->indices.append(index);
}

/*!
    Sets the filter sections list of indices to \a indices.
*/
void QHelpDataFilterSection::setIndices(const QList<QHelpDataIndexItem> &indices)
{
    d->indices = indices;
}

/*!
    Returns the list of indices.
*/
QList<QHelpDataIndexItem> QHelpDataFilterSection::indices() const
{
    return d->indices;
}

/*!
    Adds the top level content item \a content to the filter section.
*/
void QHelpDataFilterSection::addContent(QHelpDataContentItem *content)
{
    d->contents.append(content);
}

/*!
    Sets the list of top level content items of the filter section to
    \a contents.
*/
void QHelpDataFilterSection::setContents(const QList<QHelpDataContentItem*> &contents)
{
    qDeleteAll(d->contents);
    d->contents = contents;
}

/*!
    Returns a list of top level content items.
*/
QList<QHelpDataContentItem*> QHelpDataFilterSection::contents() const
{
    return d->contents;
}

/*!
    Adds the file \a file to the filter section.
*/
void QHelpDataFilterSection::addFile(const QString &file)
{
    d->files.append(file);
}

/*!
    Set the list of files to \a files.
*/
void QHelpDataFilterSection::setFiles(const QStringList &files)
{
    d->files = files;
}

/*!
    Returns the list of files.
*/
QStringList QHelpDataFilterSection::files() const
{
    return d->files;
}

/*!
    \internal
    \class QHelpDataInterface
    \since 4.4
*/

/*!
    \fn QHelpDataInterface::QHelpDataInterface()

    Constructs a new help data interface.
*/

/*!
    \fn QHelpDataInterface::~QHelpDataInterface()

    Destroys the help data interface.
*/

/*!
    \fn QString QHelpDataInterface::namespaceName() const = 0

    Returns the namespace name of the help data set.
*/

/*!
    \fn QString QHelpDataInterface::virtualFolder() const = 0

    Returns the virtual folder of the help data set.
*/

/*!
    \fn QList<QHelpDataCustomFilter> QHelpDataInterface::customFilters () const = 0

    Returns a list of custom filters. Defining custom filters is optional.
*/

/*!
    \fn QList<QHelpDataFilterSection> QHelpDataInterface::filterSections() const = 0

    Returns a list of filter sections.
*/

/*!
    \fn QMap<QString, QVariant> QHelpDataInterface::metaData() const = 0

    Returns a map of meta data. A meta data item can hold almost any data
    and is identified by its name.
*/

/*!
    \fn QString QHelpDataInterface::rootPath() const = 0

    Returns the root file path of the documentation data. All referenced file
    path or links of content items are relative to this path.
*/

QT_END_NAMESPACE
