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

#ifndef QEVENT_H
#define QEVENT_H

#include <qwindowdefs.h>
#include <qobject.h>
#include <qregion.h>
#include <qnamespace.h>
#include <qstring.h>
#include <qkeysequence.h>
#include <qcoreevent.h>
#include <qmime.h>
#include <qdrag.h>
#include <qvariant.h>
#include <qmap.h>
#include <qset.h>
#include <qfile.h>

QT_BEGIN_NAMESPACE

class QAction;
#ifndef QT_NO_GESTURES
class QGesture;
#endif

class Q_GUI_EXPORT QInputEvent : public QEvent
{

 public:
   QInputEvent(Type type, Qt::KeyboardModifiers modifiers = Qt::NoModifier);
   ~QInputEvent();

   inline Qt::KeyboardModifiers modifiers() const {
      return modState;
   }
   inline void setModifiers(Qt::KeyboardModifiers amodifiers) {
      modState = amodifiers;
   }

 protected:
   Qt::KeyboardModifiers modState;
};

class Q_GUI_EXPORT QMouseEvent : public QInputEvent
{
 public:
   QMouseEvent(Type type, const QPoint &pos, Qt::MouseButton button,
               Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers);

   QMouseEvent(Type type, const QPoint &pos, const QPoint &globalPos,
               Qt::MouseButton button, Qt::MouseButtons buttons,
               Qt::KeyboardModifiers modifiers);

   ~QMouseEvent();

   inline const QPoint &pos() const {
      return p;
   }
   inline const QPoint &globalPos() const {
      return g;
   }
   inline int x() const {
      return p.x();
   }
   inline int y() const {
      return p.y();
   }
   inline int globalX() const {
      return g.x();
   }
   inline int globalY() const {
      return g.y();
   }
   inline Qt::MouseButton button() const {
      return b;
   }
   inline Qt::MouseButtons buttons() const {
      return mouseState;
   }

   static QMouseEvent *createExtendedMouseEvent(Type type, const QPointF &pos,
         const QPoint &globalPos, Qt::MouseButton button,
         Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers);

   inline bool hasExtendedInfo() const {
      return reinterpret_cast<const QMouseEvent *>(d) == this;
   }
   QPointF posF() const;

 protected:
   QPoint p, g;
   Qt::MouseButton b;
   Qt::MouseButtons mouseState;
};

class Q_GUI_EXPORT QHoverEvent : public QEvent
{
 public:
   QHoverEvent(Type type, const QPoint &pos, const QPoint &oldPos);
   ~QHoverEvent();

   inline const QPoint &pos() const {
      return p;
   }
   inline const QPoint &oldPos() const {
      return op;
   }

 protected:
   QPoint p, op;
};

#ifndef QT_NO_WHEELEVENT
class Q_GUI_EXPORT QWheelEvent : public QInputEvent
{
 public:
   QWheelEvent(const QPoint &pos, int delta,
               Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers,
               Qt::Orientation orient = Qt::Vertical);
   QWheelEvent(const QPoint &pos, const QPoint &globalPos, int delta,
               Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers,
               Qt::Orientation orient = Qt::Vertical);
   ~QWheelEvent();

   inline int delta() const {
      return d;
   }
   inline const QPoint &pos() const {
      return p;
   }
   inline const QPoint &globalPos()   const {
      return g;
   }
   inline int x() const {
      return p.x();
   }
   inline int y() const {
      return p.y();
   }
   inline int globalX() const {
      return g.x();
   }
   inline int globalY() const {
      return g.y();
   }

   inline Qt::MouseButtons buttons() const {
      return mouseState;
   }
   Qt::Orientation orientation() const {
      return o;
   }

 protected:
   QPoint p;
   QPoint g;
   int d;
   Qt::MouseButtons mouseState;
   Qt::Orientation o;
};
#endif

#ifndef QT_NO_TABLETEVENT
class Q_GUI_EXPORT QTabletEvent : public QInputEvent
{
 public:
   enum TabletDevice { NoDevice, Puck, Stylus, Airbrush, FourDMouse,
                       XFreeEraser /*internal*/, RotationStylus
                     };

   enum PointerType { UnknownPointer, Pen, Cursor, Eraser };
   QTabletEvent(Type t, const QPoint &pos, const QPoint &globalPos, const QPointF &hiResGlobalPos,
                int device, int pointerType, qreal pressure, int xTilt, int yTilt,
                qreal tangentialPressure, qreal rotation, int z,
                Qt::KeyboardModifiers keyState, qint64 uniqueID);

   ~QTabletEvent();

   inline const QPoint &pos() const {
      return mPos;
   }
   inline const QPoint &globalPos() const {
      return mGPos;
   }
   inline const QPointF &hiResGlobalPos() const {
      return mHiResGlobalPos;
   }
   inline int x() const {
      return mPos.x();
   }
   inline int y() const {
      return mPos.y();
   }
   inline int globalX() const {
      return mGPos.x();
   }
   inline int globalY() const {
      return mGPos.y();
   }
   inline qreal hiResGlobalX() const {
      return mHiResGlobalPos.x();
   }
   inline qreal hiResGlobalY() const {
      return mHiResGlobalPos.y();
   }
   inline TabletDevice device() const {
      return TabletDevice(mDev);
   }
   inline PointerType pointerType() const {
      return PointerType(mPointerType);
   }
   inline qint64 uniqueId() const {
      return mUnique;
   }
   inline qreal pressure() const {
      return mPress;
   }
   inline int z() const {
      return mZ;
   }
   inline qreal tangentialPressure() const {
      return mTangential;
   }
   inline qreal rotation() const {
      return mRot;
   }
   inline int xTilt() const {
      return mXT;
   }
   inline int yTilt() const {
      return mYT;
   }

 protected:
   QPoint mPos, mGPos;
   QPointF mHiResGlobalPos;
   int mDev, mPointerType, mXT, mYT, mZ;
   qreal mPress, mTangential, mRot;
   qint64 mUnique;

   // I don't know what the future holds for tablets but there could be some
   // new devices coming along, and there seem to be "holes" in the
   // OS-specific events for this.
   void *mExtra;
};
#endif // QT_NO_TABLETEVENT

class Q_GUI_EXPORT QKeyEvent : public QInputEvent
{
 public:
   QKeyEvent(Type type, int key, Qt::KeyboardModifiers modifiers, const QString &text = QString(),
             bool autorep = false, ushort count = 1);
   ~QKeyEvent();

   int key() const {
      return k;
   }

#ifndef QT_NO_SHORTCUT
   bool matches(QKeySequence::StandardKey key) const;
#endif

   Qt::KeyboardModifiers modifiers() const;
   inline QString text() const {
      return txt;
   }
   inline bool isAutoRepeat() const {
      return autor;
   }
   inline int count() const {
      return int(c);
   }

   // Functions for the extended key event information
   static QKeyEvent *createExtendedKeyEvent(Type type, int key, Qt::KeyboardModifiers modifiers,
         quint32 nativeScanCode, quint32 nativeVirtualKey, quint32 nativeModifiers,
         const QString &text = QString(), bool autorep = false, ushort count = 1);

   inline bool hasExtendedInfo() const {
      return reinterpret_cast<const QKeyEvent *>(d) == this;
   }

   quint32 nativeScanCode() const;
   quint32 nativeVirtualKey() const;
   quint32 nativeModifiers() const;

 protected:
   QString txt;
   int k;
   ushort c;
   uint autor: 1;
};


class Q_GUI_EXPORT QFocusEvent : public QEvent
{
 public:
   QFocusEvent(Type type, Qt::FocusReason reason = Qt::OtherFocusReason);
   ~QFocusEvent();

   inline bool gotFocus() const {
      return type() == FocusIn;
   }
   inline bool lostFocus() const {
      return type() == FocusOut;
   }

   Qt::FocusReason reason();
   Qt::FocusReason reason() const;

 private:
   Qt::FocusReason m_reason;
};


class Q_GUI_EXPORT QPaintEvent : public QEvent
{
 public:
   QPaintEvent(const QRegion &paintRegion);
   QPaintEvent(const QRect &paintRect);
   ~QPaintEvent();

   inline const QRect &rect() const {
      return m_rect;
   }
   inline const QRegion &region() const {
      return m_region;
   }

 protected:
   friend class QApplication;
   friend class QCoreApplication;
   QRect m_rect;
   QRegion m_region;
   bool m_erased;
};

class QUpdateLaterEvent : public QEvent
{
 public:
   QUpdateLaterEvent(const QRegion &paintRegion);
   ~QUpdateLaterEvent();

   inline const QRegion &region() const {
      return m_region;
   }

 protected:
   QRegion m_region;
};

class Q_GUI_EXPORT QMoveEvent : public QEvent
{
 public:
   QMoveEvent(const QPoint &pos, const QPoint &oldPos);
   ~QMoveEvent();

   inline const QPoint &pos() const {
      return p;
   }
   inline const QPoint &oldPos() const {
      return oldp;
   }
 protected:
   QPoint p, oldp;
   friend class QApplication;
   friend class QCoreApplication;
};


class Q_GUI_EXPORT QResizeEvent : public QEvent
{
 public:
   QResizeEvent(const QSize &size, const QSize &oldSize);
   ~QResizeEvent();

   inline const QSize &size() const {
      return s;
   }
   inline const QSize &oldSize()const {
      return olds;
   }
 protected:
   QSize s, olds;
   friend class QApplication;
   friend class QCoreApplication;
};


class Q_GUI_EXPORT QCloseEvent : public QEvent
{
 public:
   QCloseEvent();
   ~QCloseEvent();
};


class Q_GUI_EXPORT QIconDragEvent : public QEvent
{
 public:
   QIconDragEvent();
   ~QIconDragEvent();
};


class Q_GUI_EXPORT QShowEvent : public QEvent
{
 public:
   QShowEvent();
   ~QShowEvent();
};


class Q_GUI_EXPORT QHideEvent : public QEvent
{
 public:
   QHideEvent();
   ~QHideEvent();
};

#ifndef QT_NO_CONTEXTMENU
class Q_GUI_EXPORT QContextMenuEvent : public QInputEvent
{
 public:
   enum Reason { Mouse, Keyboard, Other };

   QContextMenuEvent(Reason reason, const QPoint &pos, const QPoint &globalPos,
                     Qt::KeyboardModifiers modifiers);
   QContextMenuEvent(Reason reason, const QPoint &pos, const QPoint &globalPos);
   QContextMenuEvent(Reason reason, const QPoint &pos);
   ~QContextMenuEvent();

   inline int x() const {
      return p.x();
   }
   inline int y() const {
      return p.y();
   }
   inline int globalX() const {
      return gp.x();
   }
   inline int globalY() const {
      return gp.y();
   }

   inline const QPoint &pos() const {
      return p;
   }
   inline const QPoint &globalPos() const {
      return gp;
   }

   inline Reason reason() const {
      return Reason(reas);
   }

 protected:
   QPoint p;
   QPoint gp;
   uint reas : 8;
};
#endif // QT_NO_CONTEXTMENU

#ifndef QT_NO_INPUTMETHOD
class Q_GUI_EXPORT QInputMethodEvent : public QEvent
{
 public:
   enum AttributeType {
      TextFormat,
      Cursor,
      Language,
      Ruby,
      Selection
   };
   class Attribute
   {
    public:
      Attribute(AttributeType t, int s, int l, QVariant val) : type(t), start(s), length(l), value(val) {}
      AttributeType type;

      int start;
      int length;
      QVariant value;
   };
   QInputMethodEvent();
   QInputMethodEvent(const QString &preeditText, const QList<Attribute> &attributes);
   void setCommitString(const QString &commitString, int replaceFrom = 0, int replaceLength = 0);

   inline const QList<Attribute> &attributes() const {
      return attrs;
   }
   inline const QString &preeditString() const {
      return preedit;
   }

   inline const QString &commitString() const {
      return commit;
   }
   inline int replacementStart() const {
      return replace_from;
   }
   inline int replacementLength() const {
      return replace_length;
   }

   QInputMethodEvent(const QInputMethodEvent &other);

 private:
   QString preedit;
   QList<Attribute> attrs;
   QString commit;
   int replace_from;
   int replace_length;
};
#endif // QT_NO_INPUTMETHOD

#ifndef QT_NO_DRAGANDDROP

class QMimeData;

class Q_GUI_EXPORT QDropEvent : public QEvent
{
 public:
   QDropEvent(const QPoint &pos, Qt::DropActions actions, const QMimeData *data,
              Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers, Type type = Drop);
   ~QDropEvent();

   inline const QPoint &pos() const {
      return p;
   }
   inline Qt::MouseButtons mouseButtons() const {
      return mouseState;
   }
   inline Qt::KeyboardModifiers keyboardModifiers() const {
      return modState;
   }

   inline Qt::DropActions possibleActions() const {
      return act;
   }
   inline Qt::DropAction proposedAction() const {
      return default_action;
   }
   inline void acceptProposedAction() {
      drop_action = default_action;
      accept();
   }

   inline Qt::DropAction dropAction() const {
      return drop_action;
   }
   void setDropAction(Qt::DropAction action);

   QWidget *source() const;
   inline const QMimeData *mimeData() const {
      return mdata;
   }

 protected:
   friend class QApplication;
   QPoint p;
   Qt::MouseButtons mouseState;
   Qt::KeyboardModifiers modState;
   Qt::DropActions act;
   Qt::DropAction drop_action;
   Qt::DropAction default_action;
   const QMimeData *mdata;
};


class Q_GUI_EXPORT QDragMoveEvent : public QDropEvent
{
 public:
   QDragMoveEvent(const QPoint &pos, Qt::DropActions actions, const QMimeData *data,
                  Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers, Type type = DragMove);
   ~QDragMoveEvent();

   inline QRect answerRect() const {
      return rect;
   }

   inline void accept() {
      QDropEvent::accept();
   }
   inline void ignore() {
      QDropEvent::ignore();
   }

   inline void accept(const QRect &r) {
      accept();
      rect = r;
   }
   inline void ignore(const QRect &r) {
      ignore();
      rect = r;
   }

 protected:
   friend class QApplication;
   QRect rect;
};


class Q_GUI_EXPORT QDragEnterEvent : public QDragMoveEvent
{
 public:
   QDragEnterEvent(const QPoint &pos, Qt::DropActions actions, const QMimeData *data,
                   Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers);
   ~QDragEnterEvent();
};


/* An internal class */
class Q_GUI_EXPORT QDragResponseEvent : public QEvent
{
 public:
   QDragResponseEvent(bool accepted);
   ~QDragResponseEvent();

   inline bool dragAccepted() const {
      return a;
   }
 protected:
   bool a;
};


class Q_GUI_EXPORT QDragLeaveEvent : public QEvent
{
 public:
   QDragLeaveEvent();
   ~QDragLeaveEvent();
};
#endif // QT_NO_DRAGANDDROP


class Q_GUI_EXPORT QHelpEvent : public QEvent
{
 public:
   QHelpEvent(Type type, const QPoint &pos, const QPoint &globalPos);
   ~QHelpEvent();

   inline int x() const {
      return p.x();
   }
   inline int y() const {
      return p.y();
   }
   inline int globalX() const {
      return gp.x();
   }
   inline int globalY() const {
      return gp.y();
   }

   inline const QPoint &pos()  const {
      return p;
   }
   inline const QPoint &globalPos() const {
      return gp;
   }

 private:
   QPoint p;
   QPoint gp;
};

#ifndef QT_NO_STATUSTIP
class Q_GUI_EXPORT QStatusTipEvent : public QEvent
{
 public:
   QStatusTipEvent(const QString &tip);
   ~QStatusTipEvent();

   inline QString tip() const {
      return s;
   }
 private:
   QString s;
};
#endif

#ifndef QT_NO_WHATSTHIS
class Q_GUI_EXPORT QWhatsThisClickedEvent : public QEvent
{
 public:
   QWhatsThisClickedEvent(const QString &href);
   ~QWhatsThisClickedEvent();

   inline QString href() const {
      return s;
   }
 private:
   QString s;
};
#endif

#ifndef QT_NO_ACTION
class Q_GUI_EXPORT QActionEvent : public QEvent
{
   QAction *act, *bef;
 public:
   QActionEvent(int type, QAction *action, QAction *before = 0);
   ~QActionEvent();

   inline QAction *action() const {
      return act;
   }
   inline QAction *before() const {
      return bef;
   }
};
#endif

class Q_GUI_EXPORT QFileOpenEvent : public QEvent
{
 public:
   QFileOpenEvent(const QString &file);
   QFileOpenEvent(const QUrl &url);
   ~QFileOpenEvent();

   inline QString file() const {
      return f;
   }
   QUrl url() const;
   bool openFile(QFile &file, QIODevice::OpenMode flags) const;
 private:
   QString f;
};

#ifndef QT_NO_TOOLBAR
class Q_GUI_EXPORT QToolBarChangeEvent : public QEvent
{
 public:
   QToolBarChangeEvent(bool t);
   ~QToolBarChangeEvent();

   inline bool toggle() const {
      return tog;
   }
 private:
   uint tog : 1;
};
#endif

#ifndef QT_NO_SHORTCUT
class Q_GUI_EXPORT QShortcutEvent : public QEvent
{
 public:
   QShortcutEvent(const QKeySequence &key, int id, bool ambiguous = false);
   ~QShortcutEvent();

   inline const QKeySequence &key() {
      return sequence;
   }
   inline const QKeySequence &key() const {
      return sequence;
   }
   inline int shortcutId() {
      return sid;
   }
   inline int shortcutId() const {
      return sid;
   }
   inline bool isAmbiguous() {
      return ambig;
   }
   inline bool isAmbiguous() const {
      return ambig;
   }
 protected:
   QKeySequence sequence;
   bool ambig;
   int  sid;
};
#endif

#ifndef QT_NO_CLIPBOARD
class Q_GUI_EXPORT QClipboardEvent : public QEvent
{
 public:
   QClipboardEvent(QEventPrivate *data);
   ~QClipboardEvent();

   QEventPrivate *data() {
      return d;
   }
};
#endif

class Q_GUI_EXPORT QWindowStateChangeEvent: public QEvent
{
 public:
   QWindowStateChangeEvent(Qt::WindowStates aOldState);
   QWindowStateChangeEvent(Qt::WindowStates aOldState, bool isOverride);
   ~QWindowStateChangeEvent();

   inline Qt::WindowStates oldState() const {
      return ostate;
   }
   bool isOverride() const;

 private:
   Qt::WindowStates ostate;
};

Q_GUI_EXPORT QDebug operator<<(QDebug, const QEvent *);

#ifndef QT_NO_SHORTCUT
inline bool operator==(QKeyEvent *e, QKeySequence::StandardKey key)
{
   return (e ? e->matches(key) : false);
}
inline bool operator==(QKeySequence::StandardKey key, QKeyEvent *e)
{
   return (e ? e->matches(key) : false);
}
#endif

class QTouchEventTouchPointPrivate;
class Q_GUI_EXPORT QTouchEvent : public QInputEvent
{
 public:
   class Q_GUI_EXPORT TouchPoint
   {
    public:
      TouchPoint(int id = -1);
      TouchPoint(const QTouchEvent::TouchPoint &other);
      ~TouchPoint();

      int id() const;

      Qt::TouchPointState state() const;
      bool isPrimary() const;

      QPointF pos() const;
      QPointF startPos() const;
      QPointF lastPos() const;

      QPointF scenePos() const;
      QPointF startScenePos() const;
      QPointF lastScenePos() const;

      QPointF screenPos() const;
      QPointF startScreenPos() const;
      QPointF lastScreenPos() const;

      QPointF normalizedPos() const;
      QPointF startNormalizedPos() const;
      QPointF lastNormalizedPos() const;

      QRectF rect() const;
      QRectF sceneRect() const;
      QRectF screenRect() const;

      qreal pressure() const;

      // internal
      void setId(int id);
      void setState(Qt::TouchPointStates state);
      void setPos(const QPointF &pos);
      void setScenePos(const QPointF &scenePos);
      void setScreenPos(const QPointF &screenPos);
      void setNormalizedPos(const QPointF &normalizedPos);
      void setStartPos(const QPointF &startPos);
      void setStartScenePos(const QPointF &startScenePos);
      void setStartScreenPos(const QPointF &startScreenPos);
      void setStartNormalizedPos(const QPointF &startNormalizedPos);
      void setLastPos(const QPointF &lastPos);
      void setLastScenePos(const QPointF &lastScenePos);
      void setLastScreenPos(const QPointF &lastScreenPos);
      void setLastNormalizedPos(const QPointF &lastNormalizedPos);
      void setRect(const QRectF &rect);
      void setSceneRect(const QRectF &sceneRect);
      void setScreenRect(const QRectF &screenRect);
      void setPressure(qreal pressure);
      QTouchEvent::TouchPoint &operator=(const QTouchEvent::TouchPoint &other);

    private:
      QTouchEventTouchPointPrivate *d;
      friend class QApplication;
      friend class QApplicationPrivate;
   };

   enum DeviceType {
      TouchScreen,
      TouchPad
   };

   QTouchEvent(QEvent::Type eventType,
               QTouchEvent::DeviceType deviceType = TouchScreen,
               Qt::KeyboardModifiers modifiers = Qt::NoModifier,
               Qt::TouchPointStates touchPointStates = 0,
               const QList<QTouchEvent::TouchPoint> &touchPoints = QList<QTouchEvent::TouchPoint>());
   ~QTouchEvent();

   inline QWidget *widget() const {
      return _widget;
   }
   inline QTouchEvent::DeviceType deviceType() const {
      return _deviceType;
   }
   inline Qt::TouchPointStates touchPointStates() const {
      return _touchPointStates;
   }
   inline const QList<QTouchEvent::TouchPoint> &touchPoints() const {
      return _touchPoints;
   }

   // internal
   inline void setWidget(QWidget *awidget) {
      _widget = awidget;
   }
   inline void setDeviceType(DeviceType adeviceType) {
      _deviceType = adeviceType;
   }
   inline void setTouchPointStates(Qt::TouchPointStates aTouchPointStates) {
      _touchPointStates = aTouchPointStates;
   }
   inline void setTouchPoints(const QList<QTouchEvent::TouchPoint> &atouchPoints) {
      _touchPoints = atouchPoints;
   }

 protected:
   QWidget *_widget;
   QTouchEvent::DeviceType _deviceType;
   Qt::TouchPointStates _touchPointStates;
   QList<QTouchEvent::TouchPoint> _touchPoints;

   friend class QApplication;
   friend class QApplicationPrivate;
};

#ifndef QT_NO_GESTURES
class QGesture;
class QGestureEventPrivate;
class Q_GUI_EXPORT QGestureEvent : public QEvent
{
 public:
   QGestureEvent(const QList<QGesture *> &gestures);
   ~QGestureEvent();

   QList<QGesture *> gestures() const;
   QGesture *gesture(Qt::GestureType type) const;

   QList<QGesture *> activeGestures() const;
   QList<QGesture *> canceledGestures() const;

   using QEvent::setAccepted;
   using QEvent::isAccepted;
   using QEvent::accept;
   using QEvent::ignore;

   void setAccepted(QGesture *, bool);
   void accept(QGesture *);
   void ignore(QGesture *);
   bool isAccepted(QGesture *) const;

   void setAccepted(Qt::GestureType, bool);
   void accept(Qt::GestureType);
   void ignore(Qt::GestureType);
   bool isAccepted(Qt::GestureType) const;

   void setWidget(QWidget *widget);
   QWidget *widget() const;

#ifndef QT_NO_GRAPHICSVIEW
   QPointF mapToGraphicsScene(const QPointF &gesturePoint) const;
#endif

 private:
   QGestureEventPrivate *d_func();
   const QGestureEventPrivate *d_func() const;

   friend class QApplication;
   friend class QGestureManager;
};
#endif // QT_NO_GESTURES

QT_END_NAMESPACE


#endif // QEVENT_H
