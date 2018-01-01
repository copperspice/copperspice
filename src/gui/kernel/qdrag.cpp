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

#include <qwidget.h>
#include <qdrag.h>
#include <qpixmap.h>
#include <qpoint.h>
#include <qdnd_p.h>

#ifndef QT_NO_DRAGANDDROP

QT_BEGIN_NAMESPACE

QDrag::QDrag(QWidget *dragSource)
   : QObject(dragSource), d_ptr(new QDragPrivate)
{
   Q_D(QDrag);
   d->source = dragSource;
   d->target = 0;
   d->data = 0;
   d->hotspot = QPoint(-10, -10);
   d->possible_actions = Qt::CopyAction;
   d->executed_action = Qt::IgnoreAction;
   d->defaultDropAction = Qt::IgnoreAction;
}

/*!
    Destroys the drag object.
*/
QDrag::~QDrag()
{
   Q_D(QDrag);
   delete d->data;
   QDragManager *manager = QDragManager::self();
   if (manager && manager->object == this) {
      manager->cancel(false);
   }
}

/*!
    Sets the data to be sent to the given MIME \a data. Ownership of the data is
    transferred to the QDrag object.
*/
void QDrag::setMimeData(QMimeData *data)
{
   Q_D(QDrag);
   if (d->data == data) {
      return;
   }
   if (d->data != 0) {
      delete d->data;
   }
   d->data = data;
}

/*!
    Returns the MIME data that is encapsulated by the drag object.
*/
QMimeData *QDrag::mimeData() const
{
   Q_D(const QDrag);
   return d->data;
}

/*!
    Sets \a pixmap as the pixmap used to represent the data in a drag
    and drop operation. You can only set a pixmap before the drag is
    started.
*/
void QDrag::setPixmap(const QPixmap &pixmap)
{
   Q_D(QDrag);
   d->pixmap = pixmap;
}

/*!
    Returns the pixmap used to represent the data in a drag and drop operation.
*/
QPixmap QDrag::pixmap() const
{
   Q_D(const QDrag);
   return d->pixmap;
}

/*!
    Sets the position of the hot spot relative to the top-left corner of the
    pixmap used to the point specified by \a hotspot.

    \bold{Note:} on X11, the pixmap may not be able to keep up with the mouse
    movements if the hot spot causes the pixmap to be displayed
    directly under the cursor.
*/
void QDrag::setHotSpot(const QPoint &hotspot)
{
   Q_D(QDrag);
   d->hotspot = hotspot;
}

/*!
    Returns the position of the hot spot relative to the top-left corner of the
    cursor.
*/
QPoint QDrag::hotSpot() const
{
   Q_D(const QDrag);
   return d->hotspot;
}

/*!
    Returns the source of the drag object. This is the widget where the drag
    and drop operation originated.
*/
QWidget *QDrag::source() const
{
   Q_D(const QDrag);
   return d->source;
}

/*!
    Returns the target of the drag and drop operation. This is the widget where
    the drag object was dropped.
*/
QWidget *QDrag::target() const
{
   Q_D(const QDrag);
   return d->target;
}

/*!
    \since 4.3

    Starts the drag and drop operation and returns a value indicating the requested
    drop action when it is completed. The drop actions that the user can choose
    from are specified in \a supportedActions. The default proposed action will be selected
    among the allowed actions in the following order: Move, Copy and Link.

    \bold{Note:} On Linux and Mac OS X, the drag and drop operation
    can take some time, but this function does not block the event
    loop. Other events are still delivered to the application while
    the operation is performed. On Windows, the Qt event loop is
    blocked while during the operation.
*/

Qt::DropAction QDrag::exec(Qt::DropActions supportedActions)
{
   return exec(supportedActions, Qt::IgnoreAction);
}

/*!
    \since 4.3

    Starts the drag and drop operation and returns a value indicating the requested
    drop action when it is completed. The drop actions that the user can choose
    from are specified in \a supportedActions.

    The \a defaultDropAction determines which action will be proposed when the user performs a
    drag without using modifier keys.

    \bold{Note:} On Linux and Mac OS X, the drag and drop operation
    can take some time, but this function does not block the event
    loop. Other events are still delivered to the application while
    the operation is performed. On Windows, the Qt event loop is
    blocked during the operation. However, QDrag::exec() on
	Windows causes processEvents() to be called frequently to keep the GUI responsive.
	If any loops or operations are called while a drag operation is active, it will block the drag operation.
*/

Qt::DropAction QDrag::exec(Qt::DropActions supportedActions, Qt::DropAction defaultDropAction)
{
   Q_D(QDrag);
   if (!d->data) {
      qWarning("QDrag: No mimedata set before starting the drag");
      return d->executed_action;
   }
   QDragManager *manager = QDragManager::self();
   d->defaultDropAction = Qt::IgnoreAction;
   d->possible_actions = supportedActions;

   if (manager) {
      if (defaultDropAction == Qt::IgnoreAction) {
         if (supportedActions & Qt::MoveAction) {
            d->defaultDropAction = Qt::MoveAction;
         } else if (supportedActions & Qt::CopyAction) {
            d->defaultDropAction = Qt::CopyAction;
         } else if (supportedActions & Qt::LinkAction) {
            d->defaultDropAction = Qt::LinkAction;
         }
      } else {
         d->defaultDropAction = defaultDropAction;
      }
      d->executed_action = manager->drag(this);
   }

   return d->executed_action;
}

/*!
    \obsolete

    \bold{Note:} It is recommended to use exec() instead of this function.

    Starts the drag and drop operation and returns a value indicating the requested
    drop action when it is completed. The drop actions that the user can choose
    from are specified in \a request. Qt::CopyAction is always allowed.

    \bold{Note:} Although the drag and drop operation can take some time, this function
    does not block the event loop. Other events are still delivered to the application
    while the operation is performed.

    \sa exec()
*/
Qt::DropAction QDrag::start(Qt::DropActions request)
{
   Q_D(QDrag);
   if (!d->data) {
      qWarning("QDrag: No mimedata set before starting the drag");
      return d->executed_action;
   }
   QDragManager *manager = QDragManager::self();
   d->defaultDropAction = Qt::IgnoreAction;
   d->possible_actions = request | Qt::CopyAction;
   if (manager) {
      d->executed_action = manager->drag(this);
   }
   return d->executed_action;
}

/*!
    Sets the drag \a cursor for the \a action. This allows you
    to override the default native cursors. To revert to using the
    native cursor for \a action pass in a null QPixmap as \a cursor.

    The \a action can only be CopyAction, MoveAction or LinkAction.
    All other values of DropAction are ignored.
*/
void QDrag::setDragCursor(const QPixmap &cursor, Qt::DropAction action)
{
   Q_D(QDrag);
   if (action != Qt::CopyAction && action != Qt::MoveAction && action != Qt::LinkAction) {
      return;
   }
   if (cursor.isNull()) {
      d->customCursors.remove(action);
   } else {
      d->customCursors[action] = cursor;
   }
}

/*!
    \fn void QDrag::actionChanged(Qt::DropAction action)

    This signal is emitted when the \a action associated with the
    drag changes.

    \sa targetChanged()
*/

/*!
    \fn void QDrag::targetChanged(QWidget *newTarget)

    This signal is emitted when the target of the drag and drop
    operation changes, with \a newTarget the new target.

    \sa target(), actionChanged()
*/

QT_END_NAMESPACE

#endif // QT_NO_DRAGANDDROP
