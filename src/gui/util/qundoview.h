/***********************************************************************
*
* Copyright (c) 2012-2014 Barbara Geller
* Copyright (c) 2012-2014 Ansel Sermersheim
* Copyright (c) 2012-2014 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
*
* This file is part of CopperSpice.
*
* CopperSpice is free software: you can redistribute it and/or
* modify it under the terms of the GNU Lesser General Public License
* version 2.1 as published by the Free Software Foundation.
*
* CopperSpice is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
* Lesser General Public License for more details.
*
* You should have received a copy of the GNU Lesser General Public
* License along with CopperSpice.  If not, see
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#ifndef QUNDOVIEW_H
#define QUNDOVIEW_H

#include <QtGui/qlistview.h>
#include <QtCore/qstring.h>

#ifndef QT_NO_UNDOVIEW

QT_BEGIN_NAMESPACE

class QUndoViewPrivate;
class QUndoStack;
class QUndoGroup;
class QIcon;

class Q_GUI_EXPORT QUndoView : public QListView
{
   CS_OBJECT(QUndoView)
   Q_DECLARE_PRIVATE(QUndoView)

   GUI_CS_PROPERTY_READ(emptyLabel, emptyLabel)
   GUI_CS_PROPERTY_WRITE(emptyLabel, setEmptyLabel)
   GUI_CS_PROPERTY_READ(cleanIcon, cleanIcon)
   GUI_CS_PROPERTY_WRITE(cleanIcon, setCleanIcon)

 public:
   explicit QUndoView(QWidget *parent = 0);
   explicit QUndoView(QUndoStack *stack, QWidget *parent = 0);

#ifndef QT_NO_UNDOGROUP
   explicit QUndoView(QUndoGroup *group, QWidget *parent = 0);
#endif

   ~QUndoView();
   QUndoStack *stack() const;

#ifndef QT_NO_UNDOGROUP
   QUndoGroup *group() const;
#endif

   void setEmptyLabel(const QString &label);
   QString emptyLabel() const;

   void setCleanIcon(const QIcon &icon);
   QIcon cleanIcon() const;

   GUI_CS_SLOT_1(Public, void setStack(QUndoStack *stack))
   GUI_CS_SLOT_2(setStack)

#ifndef QT_NO_UNDOGROUP
   GUI_CS_SLOT_1(Public, void setGroup(QUndoGroup *group))
   GUI_CS_SLOT_2(setGroup)
#endif

 private:
   Q_DISABLE_COPY(QUndoView)
};

QT_END_NAMESPACE

#endif // QT_NO_UNDOVIEW
#endif // QUNDOVIEW_H
