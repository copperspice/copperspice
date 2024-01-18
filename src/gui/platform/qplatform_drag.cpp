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

#include <qplatform_drag.h>

#include <qdnd_p.h>
#include <qkeyevent.h>
#include <qguiapplication.h>
#include <qeventloop.h>

#ifndef QT_NO_DRAGANDDROP

QPlatformDropQtResponse::QPlatformDropQtResponse(bool accepted, Qt::DropAction acceptedAction)
   : m_accepted(accepted), m_accepted_action(acceptedAction)
{
}

bool QPlatformDropQtResponse::isAccepted() const
{
   return m_accepted;
}

Qt::DropAction QPlatformDropQtResponse::acceptedAction() const
{
   return m_accepted_action;
}

QPlatformDragQtResponse::QPlatformDragQtResponse(bool accepted, Qt::DropAction acceptedAction, QRect answerRect)
   : QPlatformDropQtResponse(accepted, acceptedAction), m_answer_rect(answerRect)
{
}

QRect QPlatformDragQtResponse::answerRect() const
{
   return m_answer_rect;
}

class QPlatformDragPrivate
{
 public:
   QPlatformDragPrivate() : cursor_drop_action(Qt::IgnoreAction) {}

   Qt::DropAction cursor_drop_action;
};

QPlatformDrag::QPlatformDrag() : d_ptr(new QPlatformDragPrivate)
{
}

QPlatformDrag::~QPlatformDrag()
{
   delete d_ptr;
}

QDrag *QPlatformDrag::currentDrag() const
{
   return QDragManager::self()->object();
}

Qt::DropAction QPlatformDrag::defaultAction(Qt::DropActions possibleActions, Qt::KeyboardModifiers modifiers) const
{
   Qt::DropAction default_action = Qt::IgnoreAction;

   if (currentDrag()) {
      default_action = currentDrag()->defaultAction();
   }

   if (default_action == Qt::IgnoreAction) {
      //This means that the drag was initiated by QDrag::start and we need to
      //preserve the old behavior
      default_action = Qt::CopyAction;
   }

   if (modifiers & Qt::ControlModifier && modifiers & Qt::ShiftModifier) {
      default_action = Qt::LinkAction;
   } else if (modifiers & Qt::ControlModifier) {
      default_action = Qt::CopyAction;
   } else if (modifiers & Qt::ShiftModifier) {
      default_action = Qt::MoveAction;
   } else if (modifiers & Qt::AltModifier) {
      default_action = Qt::LinkAction;
   }

   // Check if the action determined is allowed
   if (!(possibleActions & default_action)) {
      if (possibleActions & Qt::CopyAction) {
         default_action = Qt::CopyAction;
      } else if (possibleActions & Qt::MoveAction) {
         default_action = Qt::MoveAction;
      } else if (possibleActions & Qt::LinkAction) {
         default_action = Qt::LinkAction;
      } else {
         default_action = Qt::IgnoreAction;
      }
   }

   return default_action;
}

void QPlatformDrag::updateAction(Qt::DropAction action)
{
   Q_D(QPlatformDrag);

   if (d->cursor_drop_action != action) {
      d->cursor_drop_action = action;
      emit currentDrag()->actionChanged(action);
   }
}

static const char *const default_pm[] = {
   "13 9 3 1",
   ".      c None",
   "       c #000000",
   "X      c #FFFFFF",
   "X X X X X X X",
   " X X X X X X ",
   "X ......... X",
   " X.........X ",
   "X ......... X",
   " X.........X ",
   "X ......... X",
   " X X X X X X ",
   "X X X X X X X",
};

static QPixmap *qt_drag_default_pixmap()
{
   static QPixmap retval(default_pm);
   return &retval;
}

QPixmap QPlatformDrag::defaultPixmap()
{
   return *qt_drag_default_pixmap();
}

bool QPlatformDrag::ownsDragObject() const
{
   return false;
}

#endif


