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

#include <qsimpledrag_p.h>

#include <qbitmap.h>
#include <qbuffer.h>
#include <qdebug.h>
#include <qdir.h>
#include <qdrag.h>
#include <qevent.h>
#include <qeventloop.h>
#include <qfile.h>
#include <qguiapplication.h>
#include <qimage.h>
#include <qimagereader.h>
#include <qimagewriter.h>
#include <qpixmap.h>
#include <qpoint.h>
#include <qregularexpression.h>
#include <qtextcodec.h>

#include <qdnd_p.h>
#include <qguiapplication_p.h>
#include <qhighdpiscaling_p.h>
#include <qshapedpixmapdndwindow_p.h>

#ifndef QT_NO_DRAGANDDROP

static QWindow *topLevelAt(const QPoint &pos)
{
   QWindowList list = QGuiApplication::topLevelWindows();
   for (int i = list.count() - 1; i >= 0; --i) {
      QWindow *w = list.at(i);
      if (w->isVisible() && w->geometry().contains(pos) && !qobject_cast<QShapedPixmapWindow *>(w)) {
         return w;
      }
   }
   return nullptr;
}

QBasicDrag::QBasicDrag()
   : m_restoreCursor(false), m_eventLoop(nullptr), m_executed_drop_action(Qt::IgnoreAction),
     m_can_drop(false), m_drag(nullptr), m_drag_icon_window(nullptr), m_useCompositing(true), m_screen(nullptr)
{
}

QBasicDrag::~QBasicDrag()
{
   delete m_drag_icon_window;
}

void QBasicDrag::enableEventFilter()
{
   qApp->installEventFilter(this);
}

void QBasicDrag::disableEventFilter()
{
   qApp->removeEventFilter(this);
}

static inline QPoint getNativeMousePos(QEvent *e, QObject *o)
{
   return QHighDpi::toNativePixels(static_cast<QMouseEvent *>(e)->globalPos(), qobject_cast<QWindow *>(o));
}

bool QBasicDrag::eventFilter(QObject *o, QEvent *e)
{
   (void) o;

   if (! m_drag) {
      if (e->type() == QEvent::KeyRelease && static_cast<QKeyEvent *>(e)->key() == Qt::Key_Escape) {
         disableEventFilter();
         exitDndEventLoop();
         return true; // block the key release
      }
      return false;
   }

   switch (e->type()) {
      case QEvent::ShortcutOverride:
         // prevent accelerators from firing while dragging
         e->accept();
         return true;

      case QEvent::KeyPress:
      case QEvent::KeyRelease: {
         QKeyEvent *ke = static_cast<QKeyEvent *>(e);
         if (ke->key() == Qt::Key_Escape && e->type() == QEvent::KeyPress) {
            cancel();
            disableEventFilter();
            exitDndEventLoop();

         }
         return true; // Eat all key events
      }

      case QEvent::MouseMove: {
         QPoint nativePosition = getNativeMousePos(e, o);
         move(nativePosition);
         return true; // Eat all mouse move events
      }
      case QEvent::MouseButtonRelease:
         disableEventFilter();
         if (canDrop()) {
            QPoint nativePosition = getNativeMousePos(e, o);
            drop(nativePosition);
         } else {
            cancel();
         }
         exitDndEventLoop();
         QCoreApplication::postEvent(o, new QMouseEvent(*static_cast<QMouseEvent *>(e)));
         return true; // defer mouse release events until drag event loop has returned
      case QEvent::MouseButtonDblClick:
      case QEvent::Wheel:
         return true;
      default:
         break;
   }
   return false;
}

Qt::DropAction QBasicDrag::drag(QDrag *o)
{
   m_drag = o;
   m_executed_drop_action = Qt::IgnoreAction;
   m_can_drop = false;
   m_restoreCursor = true;

#ifndef QT_NO_CURSOR
   qApp->setOverrideCursor(Qt::DragCopyCursor);
   updateCursor(m_executed_drop_action);
#endif

   startDrag();
   m_eventLoop = new QEventLoop;
   m_eventLoop->exec();
   delete m_eventLoop;
   m_eventLoop = nullptr;
   m_drag = nullptr;
   endDrag();

   return m_executed_drop_action;
}

void QBasicDrag::restoreCursor()
{
   if (m_restoreCursor) {
#ifndef QT_NO_CURSOR
      QGuiApplication::restoreOverrideCursor();
#endif
      m_restoreCursor = false;
   }
}

void QBasicDrag::startDrag()
{
   QPoint pos;

#ifndef QT_NO_CURSOR
   pos = QCursor::pos();

   if (pos.x() == int(std::numeric_limits<double>::infinity())) {
      // ### fixme: no mouse pos registered, get pos from touch
      pos = QPoint();
   }
#endif

   recreateShapedPixmapWindow(m_screen, pos);
   enableEventFilter();
}

void QBasicDrag::endDrag()
{
}

void QBasicDrag::recreateShapedPixmapWindow(QScreen *screen, const QPoint &pos)
{
   delete m_drag_icon_window;
   // ### TODO Check if its really necessary to have m_drag_icon_window
   // when QDrag is used without a pixmap - QDrag::setPixmap()
   m_drag_icon_window = new QShapedPixmapWindow(screen);

   m_drag_icon_window->setUseCompositing(m_useCompositing);
   m_drag_icon_window->setPixmap(m_drag->pixmap());
   m_drag_icon_window->setHotspot(m_drag->hotSpot());
   m_drag_icon_window->updateGeometry(pos);
   m_drag_icon_window->setVisible(true);
}

void QBasicDrag::cancel()
{
   disableEventFilter();
   restoreCursor();
   m_drag_icon_window->setVisible(false);
}

void QBasicDrag::moveShapedPixmapWindow(const QPoint &globalPos)
{
   if (m_drag) {
      m_drag_icon_window->updateGeometry(globalPos);
   }
}

void QBasicDrag::drop(const QPoint &)
{
   disableEventFilter();
   restoreCursor();
   m_drag_icon_window->setVisible(false);
}

void  QBasicDrag::exitDndEventLoop()
{
   if (m_eventLoop && m_eventLoop->isRunning()) {
      m_eventLoop->exit();
   }
}

void QBasicDrag::updateCursor(Qt::DropAction action)
{
#ifndef QT_NO_CURSOR
   Qt::CursorShape cursorShape = Qt::ForbiddenCursor;
   if (canDrop()) {
      switch (action) {
         case Qt::CopyAction:
            cursorShape = Qt::DragCopyCursor;
            break;
         case Qt::LinkAction:
            cursorShape = Qt::DragLinkCursor;
            break;
         default:
            cursorShape = Qt::DragMoveCursor;
            break;
      }
   }

   QCursor *cursor = QGuiApplication::overrideCursor();
   QPixmap pixmap = m_drag->dragCursor(action);
   if (!cursor) {
      QGuiApplication::changeOverrideCursor((pixmap.isNull()) ? QCursor(cursorShape) : QCursor(pixmap));
   } else {
      if (!pixmap.isNull()) {
         if ((cursor->pixmap().cacheKey() != pixmap.cacheKey())) {
            QGuiApplication::changeOverrideCursor(QCursor(pixmap));
         }
      } else {
         if (cursorShape != cursor->shape()) {
            QGuiApplication::changeOverrideCursor(QCursor(cursorShape));
         }
      }
   }
#endif
   updateAction(action);
}

QSimpleDrag::QSimpleDrag()
   : m_current_window(nullptr)
{
}

QMimeData *QSimpleDrag::platformDropData()
{
   if (drag()) {
      return drag()->mimeData();
   }
   return nullptr;
}

void QSimpleDrag::startDrag()
{
   QBasicDrag::startDrag();
   m_current_window = topLevelAt(QCursor::pos());
   if (m_current_window) {
      QPlatformDragQtResponse response = QWindowSystemInterface::handleDrag(m_current_window, drag()->mimeData(), QCursor::pos(),
            drag()->supportedActions());
      setCanDrop(response.isAccepted());
      updateCursor(response.acceptedAction());
   } else {
      setCanDrop(false);
      updateCursor(Qt::IgnoreAction);
   }
   setExecutedDropAction(Qt::IgnoreAction);
}

void QSimpleDrag::cancel()
{
   QBasicDrag::cancel();
   if (drag() && m_current_window) {
      QWindowSystemInterface::handleDrag(m_current_window, nullptr, QPoint(), Qt::IgnoreAction);
      m_current_window = nullptr;
   }
}

void QSimpleDrag::move(const QPoint &globalPos)
{
   //### not high-DPI aware
   moveShapedPixmapWindow(globalPos);
   QWindow *window = topLevelAt(globalPos);
   if (!window) {
      return;
   }

   const QPoint pos = globalPos - window->geometry().topLeft();
   const QPlatformDragQtResponse qt_response =
      QWindowSystemInterface::handleDrag(window, drag()->mimeData(), pos, drag()->supportedActions());

   updateCursor(qt_response.acceptedAction());
   setCanDrop(qt_response.isAccepted());
}

void QSimpleDrag::drop(const QPoint &globalPos)
{
   //### not high-DPI aware

   QBasicDrag::drop(globalPos);
   QWindow *window = topLevelAt(globalPos);
   if (!window) {
      return;
   }

   const QPoint pos = globalPos - window->geometry().topLeft();
   const QPlatformDropQtResponse response =
      QWindowSystemInterface::handleDrop(window, drag()->mimeData(), pos, drag()->supportedActions());
   if (response.isAccepted()) {
      setExecutedDropAction(response.acceptedAction());
   } else {
      setExecutedDropAction(Qt::IgnoreAction);
   }
}

#endif // QT_NO_DRAGANDDROP


