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

#include "sheet_delegate_p.h"

#include <QtCore/QAbstractItemModel>
#include <QtGui/QTreeView>
#include <QtGui/QStyle>
#include <QtGui/QPainter>

QT_BEGIN_NAMESPACE

namespace qdesigner_internal {

SheetDelegate::SheetDelegate(QTreeView *view, QWidget *parent)
    : QItemDelegate(parent),
      m_view(view)
{
}

void SheetDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    const QAbstractItemModel *model = index.model();
    Q_ASSERT(model);

    if (!model->parent(index).isValid()) {
        // this is a top-level item.
        QStyleOptionButton buttonOption;

        buttonOption.state = option.state;
#ifdef Q_WS_MAC
        buttonOption.state |= QStyle::State_Raised;
#endif
        buttonOption.state &= ~QStyle::State_HasFocus;

        buttonOption.rect = option.rect;
        buttonOption.palette = option.palette;
        buttonOption.features = QStyleOptionButton::None;
        m_view->style()->drawControl(QStyle::CE_PushButton, &buttonOption, painter, m_view);

        QStyleOption branchOption;
        static const int i = 9; // ### hardcoded in qcommonstyle.cpp
        QRect r = option.rect;
        branchOption.rect = QRect(r.left() + i/2, r.top() + (r.height() - i)/2, i, i);
        branchOption.palette = option.palette;
        branchOption.state = QStyle::State_Children;

        if (m_view->isExpanded(index))
            branchOption.state |= QStyle::State_Open;

        m_view->style()->drawPrimitive(QStyle::PE_IndicatorBranch, &branchOption, painter, m_view);

        // draw text
        QRect textrect = QRect(r.left() + i*2, r.top(), r.width() - ((5*i)/2), r.height());
        QString text = elidedText(option.fontMetrics, textrect.width(), Qt::ElideMiddle, 
            model->data(index, Qt::DisplayRole).toString());
        m_view->style()->drawItemText(painter, textrect, Qt::AlignCenter,
            option.palette, m_view->isEnabled(), text);

    } else {
        QItemDelegate::paint(painter, option, index);
    }
}

QSize SheetDelegate::sizeHint(const QStyleOptionViewItem &opt, const QModelIndex &index) const
{
    QStyleOptionViewItem option = opt;
    QSize sz = QItemDelegate::sizeHint(opt, index) + QSize(2, 2);
    return sz;
}

} // namespace qdesigner_internal

QT_END_NAMESPACE
