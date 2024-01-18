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

#ifndef QUNDOVIEW_H
#define QUNDOVIEW_H

#include <qlistview.h>
#include <qstring.h>

#ifndef QT_NO_UNDOVIEW

class QUndoViewPrivate;
class QUndoStack;
class QUndoGroup;
class QIcon;

class Q_GUI_EXPORT QUndoView : public QListView
{
   GUI_CS_OBJECT(QUndoView)

   GUI_CS_PROPERTY_READ(emptyLabel, emptyLabel)
   GUI_CS_PROPERTY_WRITE(emptyLabel, setEmptyLabel)
   GUI_CS_PROPERTY_READ(cleanIcon, cleanIcon)
   GUI_CS_PROPERTY_WRITE(cleanIcon, setCleanIcon)

 public:
   explicit QUndoView(QWidget *parent = nullptr);
   explicit QUndoView(QUndoStack *stack, QWidget *parent = nullptr);

#ifndef QT_NO_UNDOGROUP
   explicit QUndoView(QUndoGroup *group, QWidget *parent = nullptr);
#endif

   QUndoView(const QUndoView &) = delete;
   QUndoView &operator=(const QUndoView &) = delete;

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
   Q_DECLARE_PRIVATE(QUndoView)
};

#endif // QT_NO_UNDOVIEW
#endif // QUNDOVIEW_H
