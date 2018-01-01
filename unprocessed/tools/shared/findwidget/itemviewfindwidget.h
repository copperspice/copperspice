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

#ifndef ITEMVIEWFINDWIDGET_H
#define ITEMVIEWFINDWIDGET_H

#include "abstractfindwidget.h"

#include <QModelIndex>

QT_BEGIN_NAMESPACE

class QAbstractItemView;

class ItemViewFindWidget : public AbstractFindWidget
{
    Q_OBJECT

public:
    explicit ItemViewFindWidget(FindFlags flags = FindFlags(), QWidget *parent = 0);

    QAbstractItemView *itemView() const
    { return m_itemView; }

    void setItemView(QAbstractItemView *itemView);

protected:
    virtual void deactivate();
    virtual void find(const QString &textToFind, bool skipCurrent, bool backward, bool *found, bool *wrapped);

private:
    QModelIndex findHelper(const QString &textToFind, bool skipCurrent, bool backward,
        QModelIndex parent, int row, int column);

    QAbstractItemView *m_itemView;
};

QT_END_NAMESPACE

#endif  // ITEMVIEWFINDWIDGET_H
