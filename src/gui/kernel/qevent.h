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

#ifndef QEVENT_H
#define QEVENT_H

#include <qcoreevent.h>
#include <qfile.h>
#include <qkeysequence.h>
#include <qmap.h>
#include <qnamespace.h>
#include <qobject.h>
#include <qregion.h>
#include <qset.h>
#include <qstring.h>
#include <qtouchdevice.h>
#include <qurl.h>
#include <qvariant.h>
#include <qvector.h>
#include <qvector2d.h>
#include <qwindowdefs.h>

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

   Qt::KeyboardModifiers modifiers() const {
      return modState;
   }

   void setModifiers(Qt::KeyboardModifiers modifiers) {
      modState = modifiers;
   }

   ulong timestamp() const {
      return ts;
   }

   void setTimestamp(ulong timestamp) {
      ts = timestamp;
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
   QPoint pos() const {
      return l.toPoint();
   }

   QPoint globalPos() const {
      return s.toPoint();
   }

   int x() const {
      return qRound(l.x());
   }

   int y() const {
      return qRound(l.y());
   }

   int globalX() const {
      return qRound(s.x());
   }

   int globalY() const {
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
   QPointF l;
   QPointF w;
   QPointF s;
};

class Q_GUI_EXPORT QMouseEvent : public QInputEvent
{
 public:
   QMouseEvent(Type type, const QPointF &localPos, Qt::MouseButton button,
      Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers);

   QMouseEvent(Type type, const QPointF &localPos, const QPointF &screenPos,
      Qt::MouseButton button, Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers);

   QMouseEvent(Type type, const QPointF &localPos, const QPointF &windowPos, const QPointF &screenPos,
      Qt::MouseButton button, Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers);

   QMouseEvent(Type type, const QPointF &localPos, const QPointF &windowPos, const QPointF &screenPos,
      Qt::MouseButton button, Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers, Qt::MouseEventSource source);

   ~QMouseEvent();

#ifndef QT_NO_INTEGER_EVENT_COORDINATES
   QPoint pos() const {
      return l.toPoint();
   }

   QPoint globalPos() const {
      return s.toPoint();
   }

   int x() const {
      return qRound(l.x());
   }

   int y() const {
      return qRound(l.y());
   }

   int globalX() const {
      return qRound(s.x());
   }

   int globalY() const {
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

   Qt::MouseButton button() const {
      return b;
   }

   Qt::MouseButtons buttons() const {
      return mouseState;
   }

   Qt::MouseEventSource source() const;
   Qt::MouseEventFlags flags() const;

 protected:
   QPointF l;
   QPointF w;
   QPointF s;

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
   QPoint pos() const {
      return p.toPoint();
   }

   QPoint oldPos() const {
      return op.toPoint();
   }
#endif

   const QPointF &posF() const {
      return p;
   }

   const QPointF &oldPosF() const {
      return op;
   }

 protected:
   QPointF p;
   QPointF op;
};

#ifndef QT_NO_WHEELEVENT
class Q_GUI_EXPORT QWheelEvent : public QInputEvent
{
 public:
   static constexpr const int DefaultDeltasPerStep = 120;

   QWheelEvent(const QPointF &pos, int delta,
      Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers, Qt::Orientation orientation = Qt::Vertical);

   QWheelEvent(const QPointF &pos, const QPointF &globalPosition, int delta,
      Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers, Qt::Orientation orientation = Qt::Vertical);

   QWheelEvent(const QPointF &pos, const QPointF &globalPosition, QPoint pixelDelta, QPoint angleDelta,
      Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers, Qt::ScrollPhase phase,
      Qt::MouseEventSource source = Qt::MouseEventNotSynthesized);

   ~QWheelEvent();

   QPoint pixelDelta() const {
      return pixelD;
   }

   QPoint angleDelta() const {
      return angleD;
   }

   int delta() const  {
      if (orientation() == Qt::Vertical)  {
         return angleD.y();
      }

      return angleD.x();
   }

   Qt::Orientation orientation() const {
      if (std::abs(angleD.y()) > std::abs(angleD.x()) ) {
         return Qt::Vertical;
      }

      return Qt::Horizontal;
   }

#ifndef QT_NO_INTEGER_EVENT_COORDINATES
   QPoint pos() const {
      return p.toPoint();
   }

   QPoint globalPos() const {
      return g.toPoint();
   }

   int x() const {
      return p.x();
   }

   int y() const {
      return p.y();
   }

   int globalX() const {
      return g.x();
   }

   int globalY() const {
      return g.y();
   }
#endif

   const QPointF &posF() const {
      return p;
   }

   const QPointF &globalPosF() const {
      return g;
   }

   Qt::MouseButtons buttons() const {
      return mouseState;
   }

   Qt::ScrollPhase phase() const {
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
   enum TabletDevice {
      NoDevice,
      Puck,
      Stylus,
      Airbrush,
      FourDMouse,
      RotationStylus
   };

   enum PointerType {
      UnknownPointer,
      Pen,
      Cursor,
      Eraser
   };

   QTabletEvent(Type type, const QPointF &pos, const QPointF &globalPos, int device, int pointerType,
         qreal pressure, int xTilt, int yTilt, qreal tangentialPressure, qreal rotation, int z,
         Qt::KeyboardModifiers keyState, qint64 uniqueID,
         Qt::MouseButton button = Qt::NoButton, Qt::MouseButtons buttons = Qt::NoButton);

   ~QTabletEvent();

   QPoint pos() const {
      return mPos.toPoint();
   }

   QPoint globalPos() const {
      return mGPos.toPoint();
   }

   const QPointF &posF() const {
      return mPos;
   }

   const QPointF &globalPosF() const {
      return mGPos;
   }

   int x() const {
      return qRound(mPos.x());
   }

   int y() const {
      return qRound(mPos.y());
   }

   int globalX() const {
      return qRound(mGPos.x());
   }

   int globalY() const {
      return qRound(mGPos.y());
   }

   qreal hiResGlobalX() const {
      return mGPos.x();
   }

   qreal hiResGlobalY() const {
      return mGPos.y();
   }

   TabletDevice device() const {
      return TabletDevice(mDev);
   }

   PointerType pointerType() const {
      return PointerType(mPointerType);
   }

   qint64 uniqueId() const {
      return mUnique;
   }

   qreal pressure() const {
      return mPress;
   }

   int z() const {
      return mZ;
   }

   qreal tangentialPressure() const {
      return mTangential;
   }

   qreal rotation() const {
      return mRot;
   }

   int xTilt() const {
      return mXT;
   }

   int yTilt() const {
      return mYT;
   }

   Qt::MouseButton button() const;
   Qt::MouseButtons buttons() const;

 protected:
   QPointF mPos;
   QPointF mGPos;

   int mDev, mPointerType, mXT, mYT, mZ;
   qreal mPress, mTangential, mRot;
   qint64 mUnique;

   void *mExtra;
};
#endif

#ifndef QT_NO_GESTURES
class Q_GUI_EXPORT QNativeGestureEvent : public QInputEvent
{
 public:
   QNativeGestureEvent(Qt::NativeGestureType type, const QPointF &localPos, const QPointF &windowPos,
      const QPointF &screenPos, qreal realValue, ulong sequenceId, quint64 intValue);

   Qt::NativeGestureType gestureType() const {
      return mGestureType;
   }

   qreal value() const {
      return mRealValue;
   }

#ifndef QT_NO_INTEGER_EVENT_COORDINATES
   const QPoint pos() const {
      return mLocalPos.toPoint();
   }

   const QPoint globalPos() const {
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
   QString text() const {
      return txt;
   }

   bool isAutoRepeat() const {
      return autor;
   }

   int count() const {
      return int(c);
   }

   quint32 nativeScanCode() const {
      return nScanCode;
   }

   quint32 nativeVirtualKey() const {
      return nVirtualKey;
   }

   quint32 nativeModifiers() const {
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

   bool gotFocus() const {
      return type() == FocusIn;
   }

   bool lostFocus() const {
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

   const QRect &rect() const {
      return m_rect;
   }

   const QRegion &region() const {
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

   const QPoint &pos() const {
      return p;
   }

   const QPoint &oldPos() const {
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

   const QRegion &region() const {
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

   SurfaceEventType surfaceEventType() const {
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

   const QSize &size() const {
      return s;
   }

   const QSize &oldSize()const {
      return olds;
   }

 protected:
   QSize s;
   QSize olds;

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
   enum Reason {
      Mouse,
      Keyboard,
      Other
   };

   QContextMenuEvent(Reason reason, const QPoint &pos, const QPoint &globalPos,
      Qt::KeyboardModifiers modifiers);

   QContextMenuEvent(Reason reason, const QPoint &pos, const QPoint &globalPos);
   QContextMenuEvent(Reason reason, const QPoint &pos);

   ~QContextMenuEvent();

   int x() const {
      return p.x();
   }

   int y() const {
      return p.y();
   }

   int globalX() const {
      return gp.x();
   }

   int globalY() const {
      return gp.y();
   }

   const QPoint &pos() const {
      return p;
   }

   const QPoint &globalPos() const {
      return gp;
   }

   Reason reason() const {
      return Reason(reas);
   }

 protected:
   QPoint p;
   QPoint gp;
   uint reas : 8;
};
#endif

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
      Attribute(AttributeType xType, int xStart, int xLength, QVariant xValue)
         : type(xType), start(xStart), length(xLength), value(std::move(xValue))
      {
      }

      AttributeType type;
      int start;
      int length;
      QVariant value;
   };

   QInputMethodEvent();
   QInputMethodEvent(const QString &preeditText, const QList<Attribute> &attributes);

   ~QInputMethodEvent();
   void setCommitString(const QString &commitString, int replaceFrom = 0, int replaceLength = 0);

   const QList<Attribute> &attributes() const {
      return attrs;
   }

   const QString &preeditString() const {
      return preedit;
   }

   const QString &commitString() const {
      return commit;
   }

   int replacementStart() const {
      return replace_from;
   }

   int replacementLength() const {
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

   QPoint pos() const {
      return p.toPoint();
   }

   const QPointF &posF() const {
      return p;
   }

   Qt::MouseButtons mouseButtons() const {
      return mouseState;
   }

   Qt::KeyboardModifiers keyboardModifiers() const {
      return modState;
   }

   Qt::DropActions possibleActions() const {
      return act;
   }

   Qt::DropAction proposedAction() const {
      return default_action;
   }

   void acceptProposedAction() {
      drop_action = default_action;
      accept();
   }

   Qt::DropAction dropAction() const {
      return drop_action;
   }
   void setDropAction(Qt::DropAction action);

   QObject *source() const;
   const QMimeData *mimeData() const {
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

   QRect answerRect() const {
      return rect;
   }

   void accept() {
      QDropEvent::accept();
   }

   void ignore() {
      QDropEvent::ignore();
   }

   void accept(const QRect &xRect) {
      accept();
      rect = xRect;
   }

   void ignore(const QRect &xRect) {
      ignore();
      rect = xRect;
   }

 protected:
   QRect rect;
};

class Q_GUI_EXPORT QDragEnterEvent : public QDragMoveEvent
{
 public:
   QDragEnterEvent(const QPoint &point, Qt::DropActions actions, const QMimeData *data,
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

   int x() const {
      return p.x();
   }

   int y() const {
      return p.y();
   }

   int globalX() const {
      return gp.x();
   }

   int globalY() const {
      return gp.y();
   }

   const QPoint &pos()  const {
      return p;
   }

   const QPoint &globalPos() const {
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

   QString tip() const {
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

   QString href() const {
      return s;
   }

 private:
   QString s;
};
#endif

#ifndef QT_NO_ACTION
class Q_GUI_EXPORT QActionEvent : public QEvent
{
 public:
   QActionEvent(int type, QAction *action, QAction *before = nullptr);
   ~QActionEvent();

   QAction *action() const {
      return act;
   }

   QAction *before() const {
      return bef;
   }

 private:
    QAction *act;
    QAction *bef;
};
#endif

class Q_GUI_EXPORT QFileOpenEvent : public QEvent
{
 public:
   explicit QFileOpenEvent(const QString &file);
   explicit QFileOpenEvent(const QUrl &url);
   ~QFileOpenEvent();

   QString file() const {
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

   bool toggle() const {
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

   const QKeySequence &key() const {
      return sequence;
   }

   int shortcutId() const {
      return sid;
   }

   bool isAmbiguous() const {
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
   explicit QWindowStateChangeEvent(Qt::WindowStates oldState, bool isOverride = false);
   ~QWindowStateChangeEvent();

   Qt::WindowStates oldState() const {
      return ostate;
   }

   bool isOverride() const;

 private:
   Qt::WindowStates ostate;
   bool m_override;
};

Q_GUI_EXPORT QDebug operator<<(QDebug, const QEvent *event);

#ifndef QT_NO_SHORTCUT
inline bool operator==(QKeyEvent *e, QKeySequence::StandardKey key)
{
   return (e ? e->matches(key) : false);
}

inline bool operator==(QKeySequence::StandardKey key, QKeyEvent *event)
{
   return (event != nullptr ? event->matches(key) : false);
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
         Pen = 0x0001
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

   explicit QTouchEvent(QEvent::Type eventType, QTouchDevice *device = nullptr,
      Qt::KeyboardModifiers modifiers = Qt::NoModifier,
      Qt::TouchPointStates touchPointStates = Qt::TouchPointStates(),
      const QList<QTouchEvent::TouchPoint> &touchPoints = QList<QTouchEvent::TouchPoint>());

   ~QTouchEvent();

   QWindow *window() const {
      return _window;
   }

   QObject *target() const {
      return _target;
   }

   Qt::TouchPointStates touchPointStates() const {
      return _touchPointStates;
   }

   const QList<QTouchEvent::TouchPoint> &touchPoints() const {
      return _touchPoints;
   }

   QTouchDevice *device() const {
      return _device;
   }

   // internal
   void setWindow(QWindow *xWindow) {
      _window = xWindow;
   }

   void setTarget(QObject *xTarget) {
      _target = xTarget;
   }

   void setTouchPointStates(Qt::TouchPointStates touchPoint) {
      _touchPointStates = touchPoint;
   }

   void setTouchPoints(const QList<QTouchEvent::TouchPoint> &touchPointList) {
      _touchPoints = touchPointList;
   }

   void setDevice(QTouchDevice *xDevice) {
      _device = xDevice;
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
   QPointF m_startPos;
   QSizeF  m_viewportSize;
   QRectF  m_contentPosRange;
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

   QScrollEvent(const QPointF &contentPos, const QPointF &overshootDistance, ScrollState scrollState);
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
