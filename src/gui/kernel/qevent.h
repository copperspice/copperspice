/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
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

#ifndef QEVENT_H
#define QEVENT_H

#include <qwindowdefs.h>
#include <qobject.h>
#include <qregion.h>
#include <qnamespace.h>
#include <qstring.h>
#include <qkeysequence.h>
#include <qcoreevent.h>

#include <qvariant.h>
#include <qmap.h>
#include <qset.h>
#include <qvector.h>
#include <qurl.h>
#include <qfile.h>
#include <qvector2d.h>
#include <qtouchdevice.h>

class QAction;
class QScreen;

#ifndef QT_NO_GESTURES
class QGesture;
#endif

class Q_GUI_EXPORT QInputEvent : public QEvent
{

 public:
   explicit QInputEvent(Type type, Qt::KeyboardModifiers modifiers = Qt::NoModifier);
   ~QInputEvent();

   inline Qt::KeyboardModifiers modifiers() const {
      return modState;
   }

   inline void setModifiers(Qt::KeyboardModifiers amodifiers) {
      modState = amodifiers;
   }

   inline ulong timestamp() const {
      return ts;
   }

   inline void setTimestamp(ulong atimestamp) {
      ts = atimestamp;
   }

 protected:
   Qt::KeyboardModifiers modState;
   ulong ts;
};

class Q_GUI_EXPORT QEnterEvent : public QEvent
{
 public:
   QEnterEvent(const QPointF &localPos, const QPointF &windowPos, const QPointF &screenPos);
   ~QEnterEvent();

#ifndef QT_NO_INTEGER_EVENT_COORDINATES
   inline QPoint pos() const {
      return l.toPoint();
   }
   inline QPoint globalPos() const {
      return s.toPoint();
   }
   inline int x() const {
      return qRound(l.x());
   }
   inline int y() const {
      return qRound(l.y());
   }
   inline int globalX() const {
      return qRound(s.x());
   }
   inline int globalY() const {
      return qRound(s.y());
   }
#endif

   const QPointF &localPos() const {
      return l;
   }
   const QPointF &windowPos() const {
      return w;
   }
   const QPointF &screenPos() const {
      return s;
   }

 protected:
   QPointF l, w, s;
};

class Q_GUI_EXPORT QMouseEvent : public QInputEvent
{
 public:
   QMouseEvent(Type type, const QPointF &localPos, Qt::MouseButton button,
      Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers);
   QMouseEvent(Type type, const QPointF &localPos, const QPointF &screenPos,
      Qt::MouseButton button, Qt::MouseButtons buttons,
      Qt::KeyboardModifiers modifiers);
   QMouseEvent(Type type, const QPointF &localPos, const QPointF &windowPos, const QPointF &screenPos,
      Qt::MouseButton button, Qt::MouseButtons buttons,
      Qt::KeyboardModifiers modifiers);
   QMouseEvent(Type type, const QPointF &localPos, const QPointF &windowPos, const QPointF &screenPos,
      Qt::MouseButton button, Qt::MouseButtons buttons,
      Qt::KeyboardModifiers modifiers, Qt::MouseEventSource source);
   ~QMouseEvent();

#ifndef QT_NO_INTEGER_EVENT_COORDINATES
   inline QPoint pos() const {
      return l.toPoint();
   }
   inline QPoint globalPos() const {
      return s.toPoint();
   }
   inline int x() const {
      return qRound(l.x());
   }
   inline int y() const {
      return qRound(l.y());
   }
   inline int globalX() const {
      return qRound(s.x());
   }
   inline int globalY() const {
      return qRound(s.y());
   }
#endif

   const QPointF &localPos() const {
      return l;
   }
   const QPointF &windowPos() const {
      return w;
   }
   const QPointF &screenPos() const {
      return s;
   }

   inline Qt::MouseButton button() const {
      return b;
   }
   inline Qt::MouseButtons buttons() const {
      return mouseState;
   }


   Qt::MouseEventSource source() const;
   Qt::MouseEventFlags flags() const;
 protected:
   QPointF l, w, s;
   Qt::MouseButton b;
   Qt::MouseButtons mouseState;
   int caps;
   QVector2D velocity;
   friend class QApplicationPrivate;
};

class Q_GUI_EXPORT QHoverEvent : public QInputEvent
{
 public:
   QHoverEvent(Type type, const QPointF &pos, const QPointF &oldPos, Qt::KeyboardModifiers modifiers = Qt::NoModifier);
   ~QHoverEvent();

#ifndef QT_NO_INTEGER_EVENT_COORDINATES
   inline QPoint pos() const {
      return p.toPoint();
   }
   inline QPoint oldPos() const {
      return op.toPoint();
   }
#endif

   inline const QPointF &posF() const {
      return p;
   }
   inline const QPointF &oldPosF() const {
      return op;
   }

 protected:
   QPointF p, op;
};

#ifndef QT_NO_WHEELEVENT
class Q_GUI_EXPORT QWheelEvent : public QInputEvent
{
 public:
   enum { DefaultDeltasPerStep = 120 };
   QWheelEvent(const QPointF &pos, int delta,
      Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers,
      Qt::Orientation orient = Qt::Vertical);
   QWheelEvent(const QPointF &pos, const QPointF &globalPos, int delta,
      Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers,
      Qt::Orientation orient = Qt::Vertical);
   QWheelEvent(const QPointF &pos, const QPointF &globalPos,
      QPoint pixelDelta, QPoint angleDelta, int qt4Delta, Qt::Orientation qt4Orientation,
      Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers);
   QWheelEvent(const QPointF &pos, const QPointF &globalPos,
      QPoint pixelDelta, QPoint angleDelta, int qt4Delta, Qt::Orientation qt4Orientation,
      Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers, Qt::ScrollPhase phase);
   QWheelEvent(const QPointF &pos, const QPointF &globalPos, QPoint pixelDelta, QPoint angleDelta,
      int qt4Delta, Qt::Orientation qt4Orientation, Qt::MouseButtons buttons,
      Qt::KeyboardModifiers modifiers, Qt::ScrollPhase phase, Qt::MouseEventSource source);
   ~QWheelEvent();
   inline QPoint pixelDelta() const {
      return pixelD;
   }
   inline QPoint angleDelta() const {
      return angleD;
   }

   inline int delta() const  {
      return qt4D;
   }
   inline Qt::Orientation orientation() const {
      return qt4O;
   }

#ifndef QT_NO_INTEGER_EVENT_COORDINATES
   inline QPoint pos() const {
      return p.toPoint();
   }
   inline QPoint globalPos()   const {
      return g.toPoint();
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
#endif
   inline const QPointF &posF() const {
      return p;
   }
   inline const QPointF &globalPosF()   const {
      return g;
   }

   inline Qt::MouseButtons buttons() const {
      return mouseState;
   }

   inline Qt::ScrollPhase phase() const {
      return Qt::ScrollPhase(ph);
   }

   Qt::MouseEventSource source() const {
      return Qt::MouseEventSource(src);
   }

 protected:
   QPointF p;
   QPointF g;
   QPoint pixelD;
   QPoint angleD;
   int qt4D;
   Qt::Orientation qt4O;
   Qt::MouseButtons mouseState;
   uint ph : 2;
   uint src: 2;
   int reserved : 28;

   friend class QApplication;
};
#endif

#ifndef QT_NO_TABLETEVENT
class Q_GUI_EXPORT QTabletEvent : public QInputEvent
{
   GUI_CS_GADGET(QTabletEvent)

   GUI_CS_ENUM(TabletDevice)
   GUI_CS_ENUM(PointerType)

 public:
   enum TabletDevice { NoDevice, Puck, Stylus, Airbrush, FourDMouse,
      XFreeEraser /*internal*/, RotationStylus
   };

   enum PointerType { UnknownPointer, Pen, Cursor, Eraser };
   QTabletEvent(Type t, const QPointF &pos, const QPointF &globalPos,
      int device, int pointerType, qreal pressure, int xTilt, int yTilt,
      qreal tangentialPressure, qreal rotation, int z,
      Qt::KeyboardModifiers keyState, qint64 uniqueID);

   QTabletEvent(Type t, const QPointF &pos, const QPointF &globalPos,
      int device, int pointerType, qreal pressure, int xTilt, int yTilt,
      qreal tangentialPressure, qreal rotation, int z,
      Qt::KeyboardModifiers keyState, qint64 uniqueID,
      Qt::MouseButton button, Qt::MouseButtons buttons);
   ~QTabletEvent();

   inline QPoint pos() const {
      return mPos.toPoint();
   }

   inline QPoint globalPos() const {
      return mGPos.toPoint();
   }

   inline const QPointF &posF() const {
      return mPos;
   }
   inline const QPointF &globalPosF() const {
      return mGPos;
   }

   inline int x() const {
      return qRound(mPos.x());
   }

   inline int y() const {
      return qRound(mPos.y());
   }

   inline int globalX() const {
      return qRound(mGPos.x());
   }

   inline int globalY() const {
      return qRound(mGPos.y());
   }

   inline qreal hiResGlobalX() const {
      return mGPos.x();
   }

   inline qreal hiResGlobalY() const {
      return mGPos.y();
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

   Qt::MouseButton button() const;
   Qt::MouseButtons buttons() const;

 protected:
   QPointF mPos, mGPos;

   int mDev, mPointerType, mXT, mYT, mZ;
   qreal mPress, mTangential, mRot;
   qint64 mUnique;


   void *mExtra;
};
#endif // QT_NO_TABLETEVENT

#ifndef QT_NO_GESTURES
class Q_GUI_EXPORT QNativeGestureEvent : public QInputEvent
{
 public:
   QNativeGestureEvent(Qt::NativeGestureType type, const QPointF &localPos, const QPointF &windowPos,
      const QPointF &screenPos, qreal value, ulong sequenceId, quint64 intArgument);
   Qt::NativeGestureType gestureType() const {
      return mGestureType;
   }
   qreal value() const {
      return mRealValue;
   }

#ifndef QT_NO_INTEGER_EVENT_COORDINATES
   inline const QPoint pos() const {
      return mLocalPos.toPoint();
   }
   inline const QPoint globalPos() const {
      return mScreenPos.toPoint();
   }
#endif
   const QPointF &localPos() const {
      return mLocalPos;
   }
   const QPointF &windowPos() const {
      return mWindowPos;
   }
   const QPointF &screenPos() const {
      return mScreenPos;
   }

 protected:
   Qt::NativeGestureType mGestureType;
   QPointF mLocalPos;
   QPointF mWindowPos;
   QPointF mScreenPos;
   qreal mRealValue;
   ulong mSequenceId;
   quint64 mIntValue;
};
#endif
class Q_GUI_EXPORT QKeyEvent : public QInputEvent
{
 public:
   QKeyEvent(Type type, int key, Qt::KeyboardModifiers modifiers, const QString &text = QString(),
      bool autorep = false, ushort count = 1);

   QKeyEvent(Type type, int key, Qt::KeyboardModifiers modifiers,
      quint32 nativeScanCode, quint32 nativeVirtualKey, quint32 nativeModifiers,
      const QString &text = QString(), bool autorep = false, ushort count = 1);
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

   inline quint32 nativeScanCode() const {
      return nScanCode;
   }
   inline quint32 nativeVirtualKey() const {
      return nVirtualKey;
   }
   inline quint32 nativeModifiers() const {
      return nModifiers;
   }




 protected:
   QString txt;
   int k;
   quint32 nScanCode;
   quint32 nVirtualKey;
   quint32 nModifiers;
   ushort c;
   ushort autor: 1;
};


class Q_GUI_EXPORT QFocusEvent : public QEvent
{
 public:
   explicit QFocusEvent(Type type, Qt::FocusReason reason = Qt::OtherFocusReason);
   ~QFocusEvent();

   inline bool gotFocus() const {
      return type() == FocusIn;
   }
   inline bool lostFocus() const {
      return type() == FocusOut;
   }

   Qt::FocusReason reason() const;

 private:
   Qt::FocusReason m_reason;
};

class Q_GUI_EXPORT QPaintEvent : public QEvent
{
 public:
   explicit QPaintEvent(const QRegion &paintRegion);
   explicit QPaintEvent(const QRect &paintRect);
   ~QPaintEvent();

   inline const QRect &rect() const {
      return m_rect;
   }

   inline const QRegion &region() const {
      return m_region;
   }

 protected:
   QRect m_rect;
   QRegion m_region;
   bool m_erased;
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

class Q_GUI_EXPORT QExposeEvent : public QEvent
{
 public:
   explicit QExposeEvent(const QRegion &exposeRegion);
   ~QExposeEvent();

   inline const QRegion &region() const {
      return rgn;
   }

 protected:
   QRegion rgn;
};


class Q_GUI_EXPORT QPlatformSurfaceEvent : public QEvent
{
 public:
   enum SurfaceEventType {
      SurfaceCreated,
      SurfaceAboutToBeDestroyed
   };

   explicit QPlatformSurfaceEvent(SurfaceEventType surfaceEventType);
   ~QPlatformSurfaceEvent();

   inline SurfaceEventType surfaceEventType() const {
      return m_surfaceEventType;
   }

 protected:
   SurfaceEventType m_surfaceEventType;
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
      Attribute(AttributeType typ, int s, int l, QVariant val) : type(typ), start(s), length(l), value(std::move(val)) {}
      AttributeType type;

      int start;
      int length;
      QVariant value;
   };

   QInputMethodEvent();
   QInputMethodEvent(const QString &preeditText, const QList<Attribute> &attributes);

   ~QInputMethodEvent();
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
class Q_GUI_EXPORT QInputMethodQueryEvent : public QEvent
{
 public:
   explicit QInputMethodQueryEvent(Qt::InputMethodQueries queries);
   ~QInputMethodQueryEvent();

   Qt::InputMethodQueries queries() const {
      return m_queries;
   }

   void setValue(Qt::InputMethodQuery query, const QVariant &value);
   QVariant value(Qt::InputMethodQuery query) const;
 private:
   Qt::InputMethodQueries m_queries;
   struct QueryPair {
      Qt::InputMethodQuery query;
      QVariant value;
   };
   QVector<QueryPair> m_values;
};
#endif // QT_NO_INPUTMETHOD

#ifndef QT_NO_DRAGANDDROP

class QMimeData;

class Q_GUI_EXPORT QDropEvent : public QEvent
{
 public:
   QDropEvent(const QPointF &pos, Qt::DropActions actions, const QMimeData *data,
      Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers, Type type = Drop);
   ~QDropEvent();

   inline QPoint pos() const {
      return p.toPoint();
   }

   inline const QPointF &posF() const {
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

   QObject *source() const;
   inline const QMimeData *mimeData() const {
      return mdata;
   }

 protected:
   friend class QApplication;
   QPointF p;
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
   QRect rect;
};


class Q_GUI_EXPORT QDragEnterEvent : public QDragMoveEvent
{
 public:
   QDragEnterEvent(const QPoint &pos, Qt::DropActions actions, const QMimeData *data,
      Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers);
   ~QDragEnterEvent();
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
   explicit QStatusTipEvent(const QString &tip);
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
   explicit QWhatsThisClickedEvent(const QString &href);
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
   QActionEvent(int type, QAction *action, QAction *before = nullptr);
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
   explicit QFileOpenEvent(const QString &file);
   explicit QFileOpenEvent(const QUrl &url);
   ~QFileOpenEvent();

   inline QString file() const {
      return f;
   }

   QUrl url() const {
      return m_url;
   }

   bool openFile(QFile &file, QIODevice::OpenMode flags) const;

 private:
   QString f;
   QUrl m_url;
};

#ifndef QT_NO_TOOLBAR
class Q_GUI_EXPORT QToolBarChangeEvent : public QEvent
{
 public:
   explicit QToolBarChangeEvent(bool t);
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

   inline const QKeySequence &key() const {
      return sequence;
   }

   inline int shortcutId() const {
      return sid;
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

class Q_GUI_EXPORT QWindowStateChangeEvent: public QEvent
{
 public:
   explicit QWindowStateChangeEvent(Qt::WindowStates aOldState, bool isOverride = false);
   ~QWindowStateChangeEvent();

   inline Qt::WindowStates oldState() const {
      return ostate;
   }
   bool isOverride() const;

 private:
   Qt::WindowStates ostate;
   bool m_override;
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
      enum InfoFlag {
         Pen  = 0x0001
      };
      using InfoFlags = QFlags<InfoFlag>;

      explicit TouchPoint(int id = -1);
      TouchPoint(const QTouchEvent::TouchPoint &other);
      TouchPoint(TouchPoint &&other) : d(nullptr) {
         qSwap(d, other.d);
      }
      ~TouchPoint();

      QTouchEvent::TouchPoint &operator=(const QTouchEvent::TouchPoint &other) {
         if ( d != other.d ) {
            TouchPoint copy(other);
            swap(copy);
         }

         return *this;
      }





      TouchPoint &operator=(TouchPoint &&other) {
         qSwap(d, other.d);
         return *this;
      }

      int id() const;

      Qt::TouchPointState state() const;

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

      void swap(TouchPoint &other) {
         qSwap(d, other.d);
      }

      qreal pressure() const;
      QVector2D velocity() const;
      InfoFlags flags() const;
      QVector<QPointF> rawScreenPositions() const;

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
      void setVelocity(const QVector2D &v);
      void setFlags(InfoFlags flags);
      void setRawScreenPositions(const QVector<QPointF> &positions);

    private:
      QTouchEventTouchPointPrivate *d;
      friend class QApplication;
      friend class QApplicationPrivate;
   };


   explicit QTouchEvent(QEvent::Type eventType,
      QTouchDevice *device = nullptr,
      Qt::KeyboardModifiers modifiers = Qt::NoModifier,
      Qt::TouchPointStates touchPointStates = Qt::TouchPointStates(),
      const QList<QTouchEvent::TouchPoint> &touchPoints = QList<QTouchEvent::TouchPoint>());
   ~QTouchEvent();

   inline QWindow *window() const {
      return _window;
   }

   inline QObject *target() const {
      return _target;
   }

   inline Qt::TouchPointStates touchPointStates() const {
      return _touchPointStates;
   }

   inline const QList<QTouchEvent::TouchPoint> &touchPoints() const {
      return _touchPoints;
   }

   inline QTouchDevice *device() const {
      return _device;
   }

   // internal
   inline void setWindow(QWindow *awindow) {
      _window = awindow;
   }
   inline void setTarget(QObject *atarget) {
      _target = atarget;
   }

   inline void setTouchPointStates(Qt::TouchPointStates aTouchPointStates) {
      _touchPointStates = aTouchPointStates;
   }

   inline void setTouchPoints(const QList<QTouchEvent::TouchPoint> &atouchPoints) {
      _touchPoints = atouchPoints;
   }

   inline void setDevice(QTouchDevice *adevice) {
      _device = adevice;
   }


 protected:
   QWindow *_window;
   QObject *_target;

   QTouchDevice *_device;
   Qt::TouchPointStates _touchPointStates;
   QList<QTouchEvent::TouchPoint> _touchPoints;

   friend class QApplication;
   friend class QApplicationPrivate;
};

Q_DECLARE_TYPEINFO(QTouchEvent::TouchPoint, Q_MOVABLE_TYPE);
Q_DECLARE_OPERATORS_FOR_FLAGS(QTouchEvent::TouchPoint::InfoFlags)



Q_GUI_EXPORT QDebug operator<<(QDebug, const QTouchEvent::TouchPoint &);
class Q_GUI_EXPORT QScrollPrepareEvent : public QEvent
{
 public:
   explicit QScrollPrepareEvent(const QPointF &startPos);
   ~QScrollPrepareEvent();

   QPointF startPos() const;

   QSizeF viewportSize() const;
   QRectF contentPosRange() const;
   QPointF contentPos() const;

   void setViewportSize(const QSizeF &size);
   void setContentPosRange(const QRectF &rect);
   void setContentPos(const QPointF &pos);

 private:
   QObject *m_target; // Qt 6 remove.
   QPointF m_startPos;
   QSizeF m_viewportSize;
   QRectF m_contentPosRange;
   QPointF m_contentPos;
};
class Q_GUI_EXPORT QScrollEvent : public QEvent
{
 public:
   enum ScrollState {
      ScrollStarted,
      ScrollUpdated,
      ScrollFinished
   };

   QScrollEvent(const QPointF &contentPos, const QPointF &overshoot, ScrollState scrollState);
   ~QScrollEvent();

   QPointF contentPos() const;
   QPointF overshootDistance() const;
   ScrollState scrollState() const;

 private:
   QPointF m_contentPos;
   QPointF m_overshoot;
   QScrollEvent::ScrollState m_state;
};

class Q_GUI_EXPORT QScreenOrientationChangeEvent : public QEvent
{
 public:
   QScreenOrientationChangeEvent(QScreen *screen, Qt::ScreenOrientation orientation);
   ~QScreenOrientationChangeEvent();

   QScreen *screen() const;
   Qt::ScreenOrientation orientation() const;

 private:
   QScreen *m_screen;
   Qt::ScreenOrientation m_orientation;
};

class Q_GUI_EXPORT QApplicationStateChangeEvent : public QEvent
{
 public:
   explicit QApplicationStateChangeEvent(Qt::ApplicationState state);
   Qt::ApplicationState applicationState() const;

 private:
   Qt::ApplicationState m_applicationState;
};





#endif