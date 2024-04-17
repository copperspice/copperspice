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

#ifndef QUNDOSTACK_P_H
#define QUNDOSTACK_P_H

#include <qaction.h>
#include <qlist.h>
#include <qstring.h>
#include <qundostack.h>

class QUndoCommand;
class QUndoGroup;

class QUndoCommandPrivate
{
 public:
   QUndoCommandPrivate() : id(-1) {}
   QList<QUndoCommand *> child_list;
   QString text;
   QString actionText;
   int id;
};

#ifndef QT_NO_UNDOSTACK

class QUndoStackPrivate
{
   Q_DECLARE_PUBLIC(QUndoStack)

 public:
   QUndoStackPrivate()
      : index(0), clean_index(0), group(nullptr), undo_limit(0)
   {
   }

   virtual ~QUndoStackPrivate()
   {
   }

   QList<QUndoCommand *> command_list;
   QList<QUndoCommand *> macro_stack;
   int index;
   int clean_index;
   QUndoGroup *group;
   int undo_limit;

   void setIndex(int idx, bool clean);
   bool checkUndoLimit();

 protected:
   QUndoStack *q_ptr;
};

#ifndef QT_NO_ACTION
class QUndoAction : public QAction
{
   GUI_CS_OBJECT(QUndoAction)

 public:
   QUndoAction(const QString &prefix, QObject *parent = nullptr);
   void setTextFormat(const QString &textFormat, const QString &defaultText);

   GUI_CS_SLOT_1(Public, void setPrefixedText(const QString &text))
   GUI_CS_SLOT_2(setPrefixedText)

 private:
   QString m_prefix;
   QString m_defaultText;

};
#endif // QT_NO_ACTION

#endif // QT_NO_UNDOSTACK
#endif // QUNDOSTACK_P_H
