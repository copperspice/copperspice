/***********************************************************************
*
* Copyright (c) 2012-2024 Barbara Geller
* Copyright (c) 2012-2024 Ansel Sermersheim
*
* Copyright (c) 2015 The Qt Company Ltd.
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
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
* https://www.gnu.org/licenses/
*
***********************************************************************/

#ifndef QABSTRACTITEMDELEGATE_P_H
#define QABSTRACTITEMDELEGATE_P_H

#include <qabstractitemdelegate.h>

#ifndef QT_NO_ITEMVIEWS

class QAbstractItemDelegatePrivate
{
   Q_DECLARE_PUBLIC(QAbstractItemDelegate)

 public:
   QAbstractItemDelegatePrivate();
   virtual ~QAbstractItemDelegatePrivate() = default;

   bool editorEventFilter(QObject *object, QEvent *event);
   bool tryFixup(QWidget *editor);
   QString textForRole(Qt::ItemDataRole role, const QVariant &value, const QLocale &locale, int precision = 6) const;
   void _q_commitDataAndCloseEditor(QWidget *editor);

 protected:
   QAbstractItemDelegate *q_ptr;
};

#endif // QT_NO_ITEMVIEWS

#endif
