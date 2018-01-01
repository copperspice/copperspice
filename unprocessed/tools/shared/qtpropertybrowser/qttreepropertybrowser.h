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

#ifndef QTTREEPROPERTYBROWSER_H
#define QTTREEPROPERTYBROWSER_H

#include "qtpropertybrowser.h"

QT_BEGIN_NAMESPACE

class QTreeWidgetItem;
class QtTreePropertyBrowserPrivate;

class QtTreePropertyBrowser : public QtAbstractPropertyBrowser
{
    Q_OBJECT
    Q_ENUMS(ResizeMode)
    Q_PROPERTY(int indentation READ indentation WRITE setIndentation)
    Q_PROPERTY(bool rootIsDecorated READ rootIsDecorated WRITE setRootIsDecorated)
    Q_PROPERTY(bool alternatingRowColors READ alternatingRowColors WRITE setAlternatingRowColors)
    Q_PROPERTY(bool headerVisible READ isHeaderVisible WRITE setHeaderVisible)
    Q_PROPERTY(ResizeMode resizeMode READ resizeMode WRITE setResizeMode)
    Q_PROPERTY(int splitterPosition READ splitterPosition WRITE setSplitterPosition)
    Q_PROPERTY(bool propertiesWithoutValueMarked READ propertiesWithoutValueMarked WRITE setPropertiesWithoutValueMarked)
public:

    enum ResizeMode
    {
        Interactive,
        Stretch,
        Fixed,
        ResizeToContents
    };

    QtTreePropertyBrowser(QWidget *parent = 0);
    ~QtTreePropertyBrowser();

    int indentation() const;
    void setIndentation(int i);

    bool rootIsDecorated() const;
    void setRootIsDecorated(bool show);

    bool alternatingRowColors() const;
    void setAlternatingRowColors(bool enable);

    bool isHeaderVisible() const;
    void setHeaderVisible(bool visible);

    ResizeMode resizeMode() const;
    void setResizeMode(ResizeMode mode);

    int splitterPosition() const;
    void setSplitterPosition(int position);

    void setExpanded(QtBrowserItem *item, bool expanded);
    bool isExpanded(QtBrowserItem *item) const;

    bool isItemVisible(QtBrowserItem *item) const;
    void setItemVisible(QtBrowserItem *item, bool visible);

    void setBackgroundColor(QtBrowserItem *item, const QColor &color);
    QColor backgroundColor(QtBrowserItem *item) const;
    QColor calculatedBackgroundColor(QtBrowserItem *item) const;

    void setPropertiesWithoutValueMarked(bool mark);
    bool propertiesWithoutValueMarked() const;

    void editItem(QtBrowserItem *item);

Q_SIGNALS:

    void collapsed(QtBrowserItem *item);
    void expanded(QtBrowserItem *item);

protected:
    virtual void itemInserted(QtBrowserItem *item, QtBrowserItem *afterItem);
    virtual void itemRemoved(QtBrowserItem *item);
    virtual void itemChanged(QtBrowserItem *item);

private:

    QScopedPointer<QtTreePropertyBrowserPrivate> d_ptr;
    Q_DECLARE_PRIVATE(QtTreePropertyBrowser)
    Q_DISABLE_COPY(QtTreePropertyBrowser)

    Q_PRIVATE_SLOT(d_func(), void slotCollapsed(const QModelIndex &))
    Q_PRIVATE_SLOT(d_func(), void slotExpanded(const QModelIndex &))
    Q_PRIVATE_SLOT(d_func(), void slotCurrentBrowserItemChanged(QtBrowserItem *))
    Q_PRIVATE_SLOT(d_func(), void slotCurrentTreeItemChanged(QTreeWidgetItem *, QTreeWidgetItem *))

};

QT_END_NAMESPACE

#endif
