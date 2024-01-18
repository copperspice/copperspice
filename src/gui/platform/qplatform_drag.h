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

#ifndef QPLATFORM_DRAG_H
#define QPLATFORM_DRAG_H

#include <qglobal.h>
#include <qpixmap.h>

#ifndef QT_NO_DRAGANDDROP

class QMimeData;
class QMouseEvent;
class QDrag;
class QObject;
class QEvent;
class QPlatformDragPrivate;

class Q_GUI_EXPORT QPlatformDropQtResponse
{
 public:
   QPlatformDropQtResponse(bool accepted, Qt::DropAction acceptedAction);
   bool isAccepted() const;
   Qt::DropAction acceptedAction() const;

 private:
   bool m_accepted;
   Qt::DropAction m_accepted_action;

};

class Q_GUI_EXPORT QPlatformDragQtResponse : public QPlatformDropQtResponse
{
 public:
   QPlatformDragQtResponse(bool accepted, Qt::DropAction acceptedAction, QRect answerRect);

   QRect answerRect() const;

 private:
   QRect m_answer_rect;
};

class Q_GUI_EXPORT QPlatformDrag
{
 public:
   QPlatformDrag();

   QPlatformDrag(const QPlatformDrag &) = delete;
   QPlatformDrag &operator=(const QPlatformDrag &) = delete;

   virtual ~QPlatformDrag();

   QDrag *currentDrag() const;
   virtual QMimeData *platformDropData() = 0;

   virtual Qt::DropAction drag(QDrag *m_drag) = 0;
   void updateAction(Qt::DropAction action);

   virtual Qt::DropAction defaultAction(Qt::DropActions possibleActions, Qt::KeyboardModifiers modifiers) const;

   static QPixmap defaultPixmap();

   virtual bool ownsDragObject() const;

 private:
   Q_DECLARE_PRIVATE(QPlatformDrag)
   QPlatformDragPrivate *d_ptr;
};

#endif // QT_NO_DRAGANDDROP

#endif
