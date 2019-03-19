/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

#include <QtCore/qcoreevent.h>
#include <QtCore/qpoint.h>
#include <QtCore/qscopedpointer.h>
#include <QtCore/qrect.h>
#include <QtGui/qpolygon.h>
#include <QtCore/qset.h>
#include <QtCore/qhash.h>

QT_BEGIN_NAMESPACE

#if !defined(QT_NO_GRAPHICSVIEW)

class QMimeData;
class QPointF;
class QSizeF;
class QWidget;
class QGraphicsSceneEventPrivate;
class QGraphicsSceneMouseEventPrivate;
class QGraphicsSceneWheelEventPrivate;
class QGraphicsSceneContextMenuEventPrivate;
class QGraphicsSceneHoverEventPrivate;
class QGraphicsSceneHelpEventPrivate;
class QGraphicsSceneDragDropEventPrivate;
class QGraphicsSceneResizeEventPrivate;
class QGraphicsSceneMoveEventPrivate;

class Q_GUI_EXPORT QGraphicsSceneEvent : public QEvent
{

 public:
   QGraphicsSceneEvent(Type type);
   ~QGraphicsSceneEvent();

   QWidget *widget() const;
   void setWidget(QWidget *widget);

 protected:
   QGraphicsSceneEvent(QGraphicsSceneEventPrivate &dd, Type type = None);
   QScopedPointer<QGraphicsSceneEventPrivate> d_ptr;
   Q_DECLARE_PRIVATE(QGraphicsSceneEvent)

 private:
   Q_DISABLE_COPY(QGraphicsSceneEvent)
};


class Q_GUI_EXPORT QGraphicsSceneMouseEvent : public QGraphicsSceneEvent
{

 public:
   QGraphicsSceneMouseEvent(Type type = None);
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

 private:
   Q_DECLARE_PRIVATE(QGraphicsSceneMouseEvent)
   Q_DISABLE_COPY(QGraphicsSceneMouseEvent)
};

class Q_GUI_EXPORT QGraphicsSceneWheelEvent : public QGraphicsSceneEvent
{

 public:
   QGraphicsSceneWheelEvent(Type type = None);
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
   Q_DISABLE_COPY(QGraphicsSceneWheelEvent)
};


class Q_GUI_EXPORT QGraphicsSceneContextMenuEvent : public QGraphicsSceneEvent
{

 public:
   enum Reason { Mouse, Keyboard, Other };

   QGraphicsSceneContextMenuEvent(Type type = None);
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
   Q_DISABLE_COPY(QGraphicsSceneContextMenuEvent)
};

class Q_GUI_EXPORT QGraphicsSceneHoverEvent : public QGraphicsSceneEvent
{

 public:
   QGraphicsSceneHoverEvent(Type type = None);
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
   Q_DISABLE_COPY(QGraphicsSceneHoverEvent)
};

class Q_GUI_EXPORT QGraphicsSceneHelpEvent : public QGraphicsSceneEvent
{

 public:
   QGraphicsSceneHelpEvent(Type type = None);
   ~QGraphicsSceneHelpEvent();

   QPointF scenePos() const;
   void setScenePos(const QPointF &pos);

   QPoint screenPos() const;
   void setScreenPos(const QPoint &pos);

 private:
   Q_DECLARE_PRIVATE(QGraphicsSceneHelpEvent)
   Q_DISABLE_COPY(QGraphicsSceneHelpEvent)
};

class Q_GUI_EXPORT QGraphicsSceneDragDropEvent : public QGraphicsSceneEvent
{

 public:
   QGraphicsSceneDragDropEvent(Type type = None);
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
   Q_DISABLE_COPY(QGraphicsSceneDragDropEvent)
};

class Q_GUI_EXPORT QGraphicsSceneResizeEvent : public QGraphicsSceneEvent
{
   Q_DECLARE_PRIVATE(QGraphicsSceneResizeEvent)
   Q_DISABLE_COPY(QGraphicsSceneResizeEvent)

 public:
   QGraphicsSceneResizeEvent();
   ~QGraphicsSceneResizeEvent();

   QSizeF oldSize() const;
   void setOldSize(const QSizeF &size);

   QSizeF newSize() const;
   void setNewSize(const QSizeF &size);
};


class Q_GUI_EXPORT QGraphicsSceneMoveEvent : public QGraphicsSceneEvent
{
   Q_DECLARE_PRIVATE(QGraphicsSceneMoveEvent)
   Q_DISABLE_COPY(QGraphicsSceneMoveEvent)

 public:
   QGraphicsSceneMoveEvent();
   ~QGraphicsSceneMoveEvent();

   QPointF oldPos() const;
   void setOldPos(const QPointF &pos);

   QPointF newPos() const;
   void setNewPos(const QPointF &pos);
};

#endif // QT_NO_GRAPHICSVIEW

QT_END_NAMESPACE

#endif
