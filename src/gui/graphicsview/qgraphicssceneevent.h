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

#ifndef QGRAPHICSSCENEEVENT_H
#define QGRAPHICSSCENEEVENT_H

#include <qcoreevent.h>
#include <qpoint.h>
#include <qpolygon.h>
#include <qrect.h>
#include <qscopedpointer.h>
#include <qset.h>

#if ! defined(QT_NO_GRAPHICSVIEW)

class QGraphicsSceneContextMenuEventPrivate;
class QGraphicsSceneDragDropEventPrivate;
class QGraphicsSceneEventPrivate;
class QGraphicsSceneHelpEventPrivate;
class QGraphicsSceneHoverEventPrivate;
class QGraphicsSceneMouseEventPrivate;
class QGraphicsSceneMoveEventPrivate;
class QGraphicsSceneResizeEventPrivate;
class QGraphicsSceneWheelEventPrivate;
class QMimeData;
class QPointF;
class QSizeF;
class QWidget;

class Q_GUI_EXPORT QGraphicsSceneEvent : public QEvent
{
 public:
   explicit QGraphicsSceneEvent(Type type);

   QGraphicsSceneEvent(const QGraphicsSceneEvent &) = delete;
   QGraphicsSceneEvent &operator=(const QGraphicsSceneEvent &) = delete;

   ~QGraphicsSceneEvent();

   QWidget *widget() const;
   void setWidget(QWidget *widget);

 protected:
   QGraphicsSceneEvent(QGraphicsSceneEventPrivate &dd, Type type = None);
   QScopedPointer<QGraphicsSceneEventPrivate> d_ptr;

 private:
   Q_DECLARE_PRIVATE(QGraphicsSceneEvent)
};

class Q_GUI_EXPORT QGraphicsSceneMouseEvent : public QGraphicsSceneEvent
{
 public:
   explicit QGraphicsSceneMouseEvent(Type type = None);

   QGraphicsSceneMouseEvent(const QGraphicsSceneMouseEvent &) = delete;
   QGraphicsSceneMouseEvent &operator=(const QGraphicsSceneMouseEvent &) = delete;

   ~QGraphicsSceneMouseEvent();

   QPointF pos() const;
   void setPos(const QPointF &pos);

   QPointF scenePos() const;
   void setScenePos(const QPointF &pos);

   QPoint screenPos() const;
   void setScreenPos(const QPoint &pos);

   QPointF buttonDownPos(Qt::MouseButton button) const;
   void setButtonDownPos(Qt::MouseButton button, const QPointF &pos);

   QPointF buttonDownScenePos(Qt::MouseButton button) const;
   void setButtonDownScenePos(Qt::MouseButton button, const QPointF &pos);

   QPoint buttonDownScreenPos(Qt::MouseButton button) const;
   void setButtonDownScreenPos(Qt::MouseButton button, const QPoint &pos);

   QPointF lastPos() const;
   void setLastPos(const QPointF &pos);

   QPointF lastScenePos() const;
   void setLastScenePos(const QPointF &pos);

   QPoint lastScreenPos() const;
   void setLastScreenPos(const QPoint &pos);

   Qt::MouseButtons buttons() const;
   void setButtons(Qt::MouseButtons buttons);

   Qt::MouseButton button() const;
   void setButton(Qt::MouseButton button);

   Qt::KeyboardModifiers modifiers() const;
   void setModifiers(Qt::KeyboardModifiers modifiers);

   Qt::MouseEventSource source() const;
   void setSource(Qt::MouseEventSource source);
   Qt::MouseEventFlags flags() const;
   void setFlags(Qt::MouseEventFlags);

 private:
   Q_DECLARE_PRIVATE(QGraphicsSceneMouseEvent)
};

class Q_GUI_EXPORT QGraphicsSceneWheelEvent : public QGraphicsSceneEvent
{
 public:
   explicit QGraphicsSceneWheelEvent(Type type = None);

   QGraphicsSceneWheelEvent(const QGraphicsSceneWheelEvent &) = delete;
   QGraphicsSceneWheelEvent &operator=(const QGraphicsSceneWheelEvent &) = delete;

   ~QGraphicsSceneWheelEvent();

   QPointF pos() const;
   void setPos(const QPointF &pos);

   QPointF scenePos() const;
   void setScenePos(const QPointF &pos);

   QPoint screenPos() const;
   void setScreenPos(const QPoint &pos);

   Qt::MouseButtons buttons() const;
   void setButtons(Qt::MouseButtons buttons);

   Qt::KeyboardModifiers modifiers() const;
   void setModifiers(Qt::KeyboardModifiers modifiers);

   int delta() const;
   void setDelta(int delta);

   Qt::Orientation orientation() const;
   void setOrientation(Qt::Orientation orientation);

 private:
   Q_DECLARE_PRIVATE(QGraphicsSceneWheelEvent)
};

class Q_GUI_EXPORT QGraphicsSceneContextMenuEvent : public QGraphicsSceneEvent
{
 public:
   enum Reason {
      Mouse,
      Keyboard,
      Other
   };

   explicit QGraphicsSceneContextMenuEvent(Type type = None);

   QGraphicsSceneContextMenuEvent(const QGraphicsSceneContextMenuEvent &) = delete;
   QGraphicsSceneContextMenuEvent &operator=(const QGraphicsSceneContextMenuEvent &) = delete;

   ~QGraphicsSceneContextMenuEvent();

   QPointF pos() const;
   void setPos(const QPointF &pos);

   QPointF scenePos() const;
   void setScenePos(const QPointF &pos);

   QPoint screenPos() const;
   void setScreenPos(const QPoint &pos);

   Qt::KeyboardModifiers modifiers() const;
   void setModifiers(Qt::KeyboardModifiers modifiers);

   Reason reason() const;
   void setReason(Reason reason);

 private:
   Q_DECLARE_PRIVATE(QGraphicsSceneContextMenuEvent)
};

class Q_GUI_EXPORT QGraphicsSceneHoverEvent : public QGraphicsSceneEvent
{
 public:
   explicit QGraphicsSceneHoverEvent(Type type = None);

   QGraphicsSceneHoverEvent(const QGraphicsSceneHoverEvent &) = delete;
   QGraphicsSceneHoverEvent &operator=(const QGraphicsSceneHoverEvent &) = delete;

   ~QGraphicsSceneHoverEvent();

   QPointF pos() const;
   void setPos(const QPointF &pos);

   QPointF scenePos() const;
   void setScenePos(const QPointF &pos);

   QPoint screenPos() const;
   void setScreenPos(const QPoint &pos);

   QPointF lastPos() const;
   void setLastPos(const QPointF &pos);

   QPointF lastScenePos() const;
   void setLastScenePos(const QPointF &pos);

   QPoint lastScreenPos() const;
   void setLastScreenPos(const QPoint &pos);

   Qt::KeyboardModifiers modifiers() const;
   void setModifiers(Qt::KeyboardModifiers modifiers);

 private:
   Q_DECLARE_PRIVATE(QGraphicsSceneHoverEvent)
};

class Q_GUI_EXPORT QGraphicsSceneHelpEvent : public QGraphicsSceneEvent
{
 public:
   explicit QGraphicsSceneHelpEvent(Type type = None);

   QGraphicsSceneHelpEvent(const QGraphicsSceneHelpEvent &) = delete;
   QGraphicsSceneHelpEvent &operator=(const QGraphicsSceneHelpEvent &) = delete;

   ~QGraphicsSceneHelpEvent();

   QPointF scenePos() const;
   void setScenePos(const QPointF &pos);

   QPoint screenPos() const;
   void setScreenPos(const QPoint &pos);

 private:
   Q_DECLARE_PRIVATE(QGraphicsSceneHelpEvent)
};

class Q_GUI_EXPORT QGraphicsSceneDragDropEvent : public QGraphicsSceneEvent
{
 public:
   explicit QGraphicsSceneDragDropEvent(Type type = None);

   QGraphicsSceneDragDropEvent(const QGraphicsSceneDragDropEvent &) = delete;
   QGraphicsSceneDragDropEvent &operator=(const QGraphicsSceneDragDropEvent &) = delete;

   ~QGraphicsSceneDragDropEvent();

   QPointF pos() const;
   void setPos(const QPointF &pos);

   QPointF scenePos() const;
   void setScenePos(const QPointF &pos);

   QPoint screenPos() const;
   void setScreenPos(const QPoint &pos);

   Qt::MouseButtons buttons() const;
   void setButtons(Qt::MouseButtons buttons);

   Qt::KeyboardModifiers modifiers() const;
   void setModifiers(Qt::KeyboardModifiers modifiers);

   Qt::DropActions possibleActions() const;
   void setPossibleActions(Qt::DropActions actions);

   Qt::DropAction proposedAction() const;
   void setProposedAction(Qt::DropAction action);
   void acceptProposedAction();

   Qt::DropAction dropAction() const;
   void setDropAction(Qt::DropAction action);

   QWidget *source() const;
   void setSource(QWidget *source);

   const QMimeData *mimeData() const;
   void setMimeData(const QMimeData *data);

 private:
   Q_DECLARE_PRIVATE(QGraphicsSceneDragDropEvent)
};

class Q_GUI_EXPORT QGraphicsSceneResizeEvent : public QGraphicsSceneEvent
{
 public:
   QGraphicsSceneResizeEvent();

   QGraphicsSceneResizeEvent(const QGraphicsSceneResizeEvent &) = delete;
   QGraphicsSceneResizeEvent &operator=(const QGraphicsSceneResizeEvent &) = delete;

   ~QGraphicsSceneResizeEvent();

   QSizeF oldSize() const;
   void setOldSize(const QSizeF &size);

   QSizeF newSize() const;
   void setNewSize(const QSizeF &size);

 private:
   Q_DECLARE_PRIVATE(QGraphicsSceneResizeEvent)
};

class Q_GUI_EXPORT QGraphicsSceneMoveEvent : public QGraphicsSceneEvent
{
 public:
   QGraphicsSceneMoveEvent();

   QGraphicsSceneMoveEvent(const QGraphicsSceneMoveEvent &) = delete;
   QGraphicsSceneMoveEvent &operator=(const QGraphicsSceneMoveEvent &) = delete;

   ~QGraphicsSceneMoveEvent();

   QPointF oldPos() const;
   void setOldPos(const QPointF &pos);

   QPointF newPos() const;
   void setNewPos(const QPointF &pos);

 private:
   Q_DECLARE_PRIVATE(QGraphicsSceneMoveEvent)
};

#endif // QT_NO_GRAPHICSVIEW

#endif
