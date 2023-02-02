/***********************************************************************
*
* Copyright (c) 2012-2023 Barbara Geller
* Copyright (c) 2012-2023 Ansel Sermersheim
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

#include <qevent.h>

#include <qcursor.h>
#include <qdebug.h>
#include <qgesture.h>
#include <qmath.h>
#include <qmimedata.h>
#include <qplatform_integration.h>
#include <qplatform_drag.h>
#include <qwidget.h>

#include <qapplication_p.h>
#include <qdebug_p.h>
#include <qevent_p.h>
#include <qdnd_p.h>
#include <qevent_p.h>

QEnterEvent::QEnterEvent(const QPointF &localPos, const QPointF &windowPos, const QPointF &screenPos)
   : QEvent(QEvent::Enter), l(localPos), w(windowPos), s(screenPos)
{
}

QEnterEvent::~QEnterEvent()
{
}

QInputEvent::QInputEvent(Type type, Qt::KeyboardModifiers modifiers)
   : QEvent(type), modState(modifiers), ts(0)
{ }

QInputEvent::~QInputEvent()
{
}

QMouseEvent::QMouseEvent(Type type, const QPointF &localPos, Qt::MouseButton button,
   Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers)
   : QInputEvent(type, modifiers), l(localPos), w(localPos), b(button), mouseState(buttons), caps(0)
{
#ifndef QT_NO_CURSOR
   s = QCursor::pos();
#endif
}

QMouseEvent::QMouseEvent(Type type, const QPointF &localPos, const QPointF &screenPos,
   Qt::MouseButton button, Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers)
   : QInputEvent(type, modifiers), l(localPos), w(localPos), s(screenPos), b(button), mouseState(buttons), caps(0)
{
}

QMouseEvent::QMouseEvent(Type type, const QPointF &localPos, const QPointF &windowPos, const QPointF &screenPos,
   Qt::MouseButton button, Qt::MouseButtons buttons,
   Qt::KeyboardModifiers modifiers)
   : QInputEvent(type, modifiers), l(localPos), w(windowPos), s(screenPos), b(button), mouseState(buttons), caps(0)
{
}

QMouseEvent::QMouseEvent(QEvent::Type type, const QPointF &localPos, const QPointF &windowPos, const QPointF &screenPos,
   Qt::MouseButton button, Qt::MouseButtons buttons,
   Qt::KeyboardModifiers modifiers, Qt::MouseEventSource source)
   : QInputEvent(type, modifiers), l(localPos), w(windowPos), s(screenPos), b(button), mouseState(buttons), caps(0)
{
   QGuiApplicationPrivate::setMouseEventSource(this, source);
}

QMouseEvent::~QMouseEvent()
{
}

Qt::MouseEventSource QMouseEvent::source() const
{
   return QGuiApplicationPrivate::mouseEventSource(this);
}

Qt::MouseEventFlags QMouseEvent::flags() const
{
   return QGuiApplicationPrivate::mouseEventFlags(this);
}

QHoverEvent::QHoverEvent(Type type, const QPointF &pos, const QPointF &oldPos, Qt::KeyboardModifiers modifiers)
   : QInputEvent(type, modifiers), p(pos), op(oldPos)
{
}

QHoverEvent::~QHoverEvent()
{
}

#ifndef QT_NO_WHEELEVENT

QWheelEvent::QWheelEvent(const QPointF &pos, int delta,
      Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers, Qt::Orientation orientation)
   : QInputEvent(Wheel, modifiers), p(pos), mouseState(buttons), ph(Qt::NoScrollPhase), src(Qt::MouseEventNotSynthesized)
{
   if (! QGuiApplicationPrivate::scrollNoPhaseAllowed) {
      ph = Qt::ScrollUpdate;
   }

   g = QCursor::pos();

   if (orientation == Qt::Vertical) {
      angleD = QPoint(0, delta);
   } else {
      angleD = QPoint(delta, 0);
   }
}

QWheelEvent::QWheelEvent(const QPointF &pos, const QPointF &globalPosition, int delta,
      Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers, Qt::Orientation orientation)
   : QInputEvent(Wheel, modifiers), p(pos), g(globalPosition), mouseState(buttons),
     ph(Qt::NoScrollPhase), src(Qt::MouseEventNotSynthesized)
{
   if (! QGuiApplicationPrivate::scrollNoPhaseAllowed) {
      ph = Qt::ScrollUpdate;
   }

   if (orientation == Qt::Vertical) {
      angleD = QPoint(0, delta);
   } else {
      angleD = QPoint(delta, 0);
   }
}

QWheelEvent::QWheelEvent(const QPointF &pos, const QPointF &globalPosition, QPoint pixelDelta, QPoint angleDelta,
      Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers, Qt::ScrollPhase phase, Qt::MouseEventSource source )
   : QInputEvent(Wheel, modifiers), p(pos), g(globalPosition), pixelD(pixelDelta), angleD(angleDelta),
     mouseState(buttons), ph(phase), src(source)
{
}

QWheelEvent::~QWheelEvent()
{
}

#endif // QT_NO_WHEELEVENT

QKeyEvent::QKeyEvent(Type type, int key, Qt::KeyboardModifiers modifiers, const QString &text, bool autorep, ushort count)
   : QInputEvent(type, modifiers), txt(text), k(key), nScanCode(0), nVirtualKey(0), nModifiers(0), c(count), autor(autorep)
{
   if (type == QEvent::ShortcutOverride) {
      ignore();
   }
}

QKeyEvent::QKeyEvent(Type type, int key, Qt::KeyboardModifiers modifiers, quint32 nativeScanCode,
      quint32 nativeVirtualKey, quint32 nativeModifiers, const QString &text, bool autorep, ushort count)
   : QInputEvent(type, modifiers), txt(text), k(key),
     nScanCode(nativeScanCode), nVirtualKey(nativeVirtualKey), nModifiers(nativeModifiers),
     c(count), autor(autorep)
{
   if (type == QEvent::ShortcutOverride) {
      ignore();
   }
}

QKeyEvent::~QKeyEvent()
{
}

Qt::KeyboardModifiers QKeyEvent::modifiers() const
{
   if (key() == Qt::Key_Shift) {
      return Qt::KeyboardModifiers(QInputEvent::modifiers()^Qt::ShiftModifier);
   }

   if (key() == Qt::Key_Control) {
      return Qt::KeyboardModifiers(QInputEvent::modifiers()^Qt::ControlModifier);
   }

   if (key() == Qt::Key_Alt) {
      return Qt::KeyboardModifiers(QInputEvent::modifiers()^Qt::AltModifier);
   }

   if (key() == Qt::Key_Meta) {
      return Qt::KeyboardModifiers(QInputEvent::modifiers()^Qt::MetaModifier);
   }

   if (key() == Qt::Key_AltGr) {
      return Qt::KeyboardModifiers(QInputEvent::modifiers()^Qt::GroupSwitchModifier);
   }

   return QInputEvent::modifiers();
}

#ifndef QT_NO_SHORTCUT

bool QKeyEvent::matches(QKeySequence::StandardKey matchKey) const
{
   // The keypad and group switch modifier should not make a difference
   uint searchkey = (modifiers() | key()) & ~(Qt::KeypadModifier | Qt::GroupSwitchModifier);

   const QList<QKeySequence> bindings = QKeySequence::keyBindings(matchKey);
   return bindings.contains(QKeySequence(searchkey));

}
#endif

QFocusEvent::QFocusEvent(Type type, Qt::FocusReason reason)
   : QEvent(type), m_reason(reason)
{
}

// internal
QFocusEvent::~QFocusEvent()
{
}

Qt::FocusReason QFocusEvent::reason() const
{
   return m_reason;
}

QPaintEvent::QPaintEvent(const QRegion &paintRegion)
   : QEvent(Paint), m_rect(paintRegion.boundingRect()), m_region(paintRegion), m_erased(false)
{
}

QPaintEvent::QPaintEvent(const QRect &paintRect)
   : QEvent(Paint), m_rect(paintRect), m_region(paintRect), m_erased(false)
{
}

// internal
QPaintEvent::~QPaintEvent()
{ }

QMoveEvent::QMoveEvent(const QPoint &pos, const QPoint &oldPos)
   : QEvent(Move), p(pos), oldp(oldPos)
{}

// internal
QMoveEvent::~QMoveEvent()
{ }

QExposeEvent::QExposeEvent(const QRegion &exposeRegion)
   : QEvent(Expose), rgn(exposeRegion)
{ }

QExposeEvent::~QExposeEvent()
{
}

QPlatformSurfaceEvent::QPlatformSurfaceEvent(SurfaceEventType surfaceEventType)
   : QEvent(PlatformSurface), m_surfaceEventType(surfaceEventType)
{
}

QPlatformSurfaceEvent::~QPlatformSurfaceEvent()
{
}

QResizeEvent::QResizeEvent(const QSize &size, const QSize &oldSize)
   : QEvent(Resize), s(size), olds(oldSize)
{
}

QResizeEvent::~QResizeEvent()
{
}

QCloseEvent::QCloseEvent()
   : QEvent(Close)
{
}

QCloseEvent::~QCloseEvent()
{
}

QIconDragEvent::QIconDragEvent()
   : QEvent(IconDrag)
{
   ignore();
}

QIconDragEvent::~QIconDragEvent()
{
}

#ifndef QT_NO_CONTEXTMENU
QContextMenuEvent::QContextMenuEvent(Reason reason, const QPoint &pos, const QPoint &globalPos)
   : QInputEvent(ContextMenu), p(pos), gp(globalPos), reas(reason)
{
}

QContextMenuEvent::QContextMenuEvent(Reason reason, const QPoint &pos, const QPoint &globalPos,
   Qt::KeyboardModifiers modifiers)
   : QInputEvent(ContextMenu, modifiers), p(pos), gp(globalPos), reas(reason)
{
}

QContextMenuEvent::~QContextMenuEvent()
{
}

QContextMenuEvent::QContextMenuEvent(Reason reason, const QPoint &pos)
   : QInputEvent(ContextMenu), p(pos), reas(reason)
{
#ifndef QT_NO_CURSOR
   gp = QCursor::pos();
#endif
}

#endif // QT_NO_CONTEXTMENU

QInputMethodEvent::QInputMethodEvent()
   : QEvent(QEvent::InputMethod), replace_from(0), replace_length(0)
{ }

QInputMethodEvent::QInputMethodEvent(const QString &preeditText, const QList<Attribute> &attributes)
   : QEvent(QEvent::InputMethod), preedit(preeditText), attrs(attributes),
     replace_from(0), replace_length(0)
{
}

QInputMethodEvent::QInputMethodEvent(const QInputMethodEvent &other)
   : QEvent(QEvent::InputMethod), preedit(other.preedit), attrs(other.attrs),
     commit(other.commit), replace_from(other.replace_from), replace_length(other.replace_length)
{
}

QInputMethodEvent::~QInputMethodEvent()
{
}

void QInputMethodEvent::setCommitString(const QString &commitString, int replaceFrom, int replaceLength)
{
   commit = commitString;
   replace_from = replaceFrom;
   replace_length = replaceLength;
}

QInputMethodQueryEvent::QInputMethodQueryEvent(Qt::InputMethodQueries queries)
   : QEvent(InputMethodQuery),m_queries(queries)
{
}

QInputMethodQueryEvent::~QInputMethodQueryEvent()
{
}

void QInputMethodQueryEvent::setValue(Qt::InputMethodQuery query, const QVariant &value)
{
   for (int i = 0; i < m_values.size(); ++i) {
      if (m_values.at(i).query == query) {
         m_values[i].value = value;
         return;
      }
   }

   QueryPair pair = { query, value };
   m_values.append(pair);
}

QVariant QInputMethodQueryEvent::value(Qt::InputMethodQuery query) const
{
   for (int i = 0; i < m_values.size(); ++i) {
      if (m_values.at(i).query == query) {
         return m_values.at(i).value;
      }
   }

   return QVariant();
}

#ifndef QT_NO_TABLETEVENT

QTabletEvent::QTabletEvent(Type type, const QPointF &pos, const QPointF &globalPos, int device, int pointerType,
      qreal pressure, int xTilt, int yTilt, qreal tangentialPressure, qreal rotation, int z,
      Qt::KeyboardModifiers keyState, qint64 uniqueID, Qt::MouseButton button, Qt::MouseButtons buttons)

   : QInputEvent(type, keyState), mPos(pos), mGPos(globalPos), mDev(device), mPointerType(pointerType),
     mXT(xTilt), mYT(yTilt), mZ(z), mPress(pressure), mTangential(tangentialPressure), mRot(rotation),
     mUnique(uniqueID), mExtra(new QTabletEventPrivate(button, buttons))
{
}

QTabletEvent::~QTabletEvent()
{
}

Qt::MouseButton QTabletEvent::button() const
{
   return static_cast<QTabletEventPrivate *>(mExtra)->b;
}

Qt::MouseButtons QTabletEvent::buttons() const
{
   return static_cast<QTabletEventPrivate *>(mExtra)->buttonState;
}

#endif // QT_NO_TABLETEVENT

#ifndef QT_NO_GESTURES
QNativeGestureEvent::QNativeGestureEvent(Qt::NativeGestureType type, const QPointF &localPos, const QPointF &windowPos,
   const QPointF &screenPos, qreal realValue, ulong sequenceId, quint64 intValue)
   : QInputEvent(QEvent::NativeGesture), mGestureType(type),
     mLocalPos(localPos), mWindowPos(windowPos), mScreenPos(screenPos), mRealValue(realValue),
     mSequenceId(sequenceId), mIntValue(intValue)
{
}
#endif

#ifndef QT_NO_DRAGANDDROP
QDragMoveEvent::QDragMoveEvent(const QPoint &pos, Qt::DropActions actions, const QMimeData *data,
   Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers, Type type)
   : QDropEvent(pos, actions, data, buttons, modifiers, type), rect(pos, QSize(1, 1))
{
}

QDragMoveEvent::~QDragMoveEvent()
{
}

QDropEvent::QDropEvent(const QPointF &pos, Qt::DropActions actions, const QMimeData *data,
   Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers, Type type)
   : QEvent(type), p(pos), mouseState(buttons), modState(modifiers), act(actions), mdata(data)
{
   default_action = QGuiApplicationPrivate::platformIntegration()->drag()->defaultAction(act, modifiers);
   drop_action = default_action;
   ignore();
}

// internal
QDropEvent::~QDropEvent()
{}

QObject *QDropEvent::source() const
{
   if (const QDragManager *manager = QDragManager::self()) {
      return manager->source();
   }

   return nullptr;
}

void QDropEvent::setDropAction(Qt::DropAction action)
{
   if (!(action & act) && action != Qt::IgnoreAction) {
      action = default_action;
   }

   drop_action = action;
}

QDragEnterEvent::QDragEnterEvent(const QPoint &point, Qt::DropActions actions, const QMimeData *data,
   Qt::MouseButtons buttons, Qt::KeyboardModifiers modifiers)
   : QDragMoveEvent(point, actions, data, buttons, modifiers, DragEnter)
{}

// internal
QDragEnterEvent::~QDragEnterEvent()
{}

QDragLeaveEvent::QDragLeaveEvent()
   : QEvent(DragLeave)
{}

// internal
QDragLeaveEvent::~QDragLeaveEvent()
{}
#endif // QT_NO_DRAGANDDROP

QHelpEvent::QHelpEvent(Type type, const QPoint &pos, const QPoint &globalPos)
   : QEvent(type), p(pos), gp(globalPos)
{}

QHelpEvent::~QHelpEvent()
{}

#ifndef QT_NO_STATUSTIP

QStatusTipEvent::QStatusTipEvent(const QString &tip)
   : QEvent(StatusTip), s(tip)
{}

// internal
QStatusTipEvent::~QStatusTipEvent()
{}

#endif

#ifndef QT_NO_WHATSTHIS

QWhatsThisClickedEvent::QWhatsThisClickedEvent(const QString &href)
   : QEvent(WhatsThisClicked), s(href)
{}

// internal
QWhatsThisClickedEvent::~QWhatsThisClickedEvent()
{
}

#endif

#ifndef QT_NO_ACTION

QActionEvent::QActionEvent(int type, QAction *action, QAction *before)
   : QEvent(static_cast<QEvent::Type>(type)), act(action), bef(before)
{}

// internal
QActionEvent::~QActionEvent()
{}

#endif

QHideEvent::QHideEvent()
   : QEvent(Hide)
{}

// internal
QHideEvent::~QHideEvent()
{}

QShowEvent::QShowEvent()
   : QEvent(Show)
{}

// internal
QShowEvent::~QShowEvent()
{}

// internal
QFileOpenEvent::QFileOpenEvent(const QString &file)
   : QEvent(FileOpen), f(file), m_url(QUrl::fromLocalFile(file))
{}

// internal
QFileOpenEvent::QFileOpenEvent(const QUrl &url)
   : QEvent(FileOpen), f(url.toLocalFile()), m_url(url)
{}

// internal
QFileOpenEvent::~QFileOpenEvent()
{}

bool QFileOpenEvent::openFile(QFile &file, QIODevice::OpenMode flags) const
{
   file.setFileName(f);
   return file.open(flags);
}

#ifndef QT_NO_TOOLBAR

// internal
QToolBarChangeEvent::QToolBarChangeEvent(bool t)
   : QEvent(ToolBarChange), tog(t)
{}

// internal
QToolBarChangeEvent::~QToolBarChangeEvent()
{}

#endif

#ifndef QT_NO_SHORTCUT

QShortcutEvent::QShortcutEvent(const QKeySequence &key, int id, bool ambiguous)
   : QEvent(Shortcut), sequence(key), ambig(ambiguous), sid(id)
{}

QShortcutEvent::~QShortcutEvent()
{}

#endif

// start of debbug stream
static inline void formatTouchEvent(QDebug d, const QTouchEvent &t)
{
   d << "QTouchEvent(";
   QtDebugUtils::formatQEnum(d, t.type());
   d << " device: " << t.device()->name();
   d << " states: ";
   QtDebugUtils::formatQFlags(d, t.touchPointStates());
   d << ", " << t.touchPoints().size() << " points: " << t.touchPoints() << ')';
}

static void formatUnicodeString(QDebug d, const QString &s)
{
   d << '"' << hex;

   for (int i = 0; i < s.size(); ++i) {
      if (i) {
         d << ',';
      }
      d << "U+" << s.at(i).unicode();
   }

   d << dec << '"';
}

static inline void formatInputMethodEvent(QDebug d, const QInputMethodEvent *e)
{
   d << "QInputMethodEvent(";

   if (! e->preeditString().isEmpty()) {
      d << "preedit=";
      formatUnicodeString(d, e->preeditString());
   }

   if (! e->commitString().isEmpty()) {
      d << ", commit=";
      formatUnicodeString(d, e->commitString());
   }

   if (e->replacementLength()) {
      d << ", replacementStart=" << e->replacementStart() << ", replacementLength="
         << e->replacementLength();
   }

   if (const int attributeCount = e->attributes().size()) {
      d << ", attributes= {";

      for (int a = 0; a < attributeCount; ++a) {
         const QInputMethodEvent::Attribute &at = e->attributes().at(a);

         if (a) {
            d << ',';
         }

         d << "[type= " << at.type << ", start=" << at.start << ", length=" << at.length
           << ", value=" << at.value.toString() << ']';
      }

      d << '}';
   }

   d << ')';
}
static inline void formatInputMethodQueryEvent(QDebug d, const QInputMethodQueryEvent *e)
{
   const Qt::InputMethodQueries queries = e->queries();
   d << "QInputMethodQueryEvent(queries=" << showbase << hex << int(queries)
     << noshowbase << dec << ", {";

   for (unsigned mask = 1; mask <= Qt::ImTextAfterCursor; mask <<= 1) {
      if (queries & mask) {
         const QVariant value = e->value(static_cast<Qt::InputMethodQuery>(mask));

         if (value.isValid()) {
            d << '[' << showbase << hex << mask <<  noshowbase << dec << '=' << value.toString() << "],";
         }
      }
   }

   d << "})";
}

static const char *eventClassName(QEvent::Type t)
{
   switch (t) {
      case QEvent::ActionAdded:
      case QEvent::ActionRemoved:
      case QEvent::ActionChanged:
         return "QActionEvent";
      case QEvent::MouseButtonPress:
      case QEvent::MouseButtonRelease:
      case QEvent::MouseButtonDblClick:
      case QEvent::MouseMove:
      case QEvent::NonClientAreaMouseMove:
      case QEvent::NonClientAreaMouseButtonPress:
      case QEvent::NonClientAreaMouseButtonRelease:
      case QEvent::NonClientAreaMouseButtonDblClick:
         return "QMouseEvent";
      case QEvent::DragEnter:
         return "QDragEnterEvent";
      case QEvent::DragMove:
         return "QDragMoveEvent";
      case QEvent::Drop:
         return "QDropEvent";
      case QEvent::KeyPress:
      case QEvent::KeyRelease:
      case QEvent::ShortcutOverride:
         return "QKeyEvent";
      case QEvent::FocusIn:
      case QEvent::FocusOut:
      case QEvent::FocusAboutToChange:
         return "QFocusEvent";
      case QEvent::ChildAdded:
      case QEvent::ChildPolished:
      case QEvent::ChildRemoved:
         return "QChildEvent";
      case QEvent::Paint:
         return "QPaintEvent";
      case QEvent::Move:
         return "QMoveEvent";
      case QEvent::Resize:
         return "QResizeEvent";
      case QEvent::Show:
         return "QShowEvent";
      case QEvent::Hide:
         return "QHideEvent";
      case QEvent::Enter:
         return "QEnterEvent";
      case QEvent::Close:
         return "QCloseEvent";
      case QEvent::FileOpen:
         return "QFileOpenEvent";

#ifndef QT_NO_GESTURES
      case QEvent::NativeGesture:
         return "QNativeGestureEvent";
      case QEvent::Gesture:
      case QEvent::GestureOverride:
         return "QGestureEvent";
#endif

      case QEvent::HoverEnter:
      case QEvent::HoverLeave:
      case QEvent::HoverMove:
         return "QHoverEvent";
      case QEvent::TabletEnterProximity:
      case QEvent::TabletLeaveProximity:
      case QEvent::TabletPress:
      case QEvent::TabletMove:
      case QEvent::TabletRelease:
         return "QTabletEvent";
      case QEvent::StatusTip:
         return "QStatusTipEvent";
      case QEvent::ToolTip:
         return "QHelpEvent";
      case QEvent::WindowStateChange:
         return "QWindowStateChangeEvent";
      case QEvent::Wheel:
         return "QWheelEvent";
      case QEvent::TouchBegin:
      case QEvent::TouchUpdate:
      case QEvent::TouchEnd:
         return "QTouchEvent";
      case QEvent::Shortcut:
         return "QShortcutEvent";
      case QEvent::InputMethod:
         return "QInputMethodEvent";
      case QEvent::InputMethodQuery:
         return "QInputMethodQueryEvent";
      case QEvent::OrientationChange:
         return "QScreenOrientationChangeEvent";
      case QEvent::ScrollPrepare:
         return "QScrollPrepareEvent";
      case QEvent::Scroll:
         return "QScrollEvent";
      case QEvent::GraphicsSceneMouseMove:
      case QEvent::GraphicsSceneMousePress:
      case QEvent::GraphicsSceneMouseRelease:
      case QEvent::GraphicsSceneMouseDoubleClick:
         return "QGraphicsSceneMouseEvent";
      case QEvent::GraphicsSceneContextMenu:
      case QEvent::GraphicsSceneHoverEnter:
      case QEvent::GraphicsSceneHoverMove:
      case QEvent::GraphicsSceneHoverLeave:
      case QEvent::GraphicsSceneHelp:
      case QEvent::GraphicsSceneDragEnter:
      case QEvent::GraphicsSceneDragMove:
      case QEvent::GraphicsSceneDragLeave:
      case QEvent::GraphicsSceneDrop:
      case QEvent::GraphicsSceneWheel:
         return "QGraphicsSceneEvent";

      case QEvent::Timer:
         return "QTimerEvent";

      case QEvent::PlatformSurface:
         return "QPlatformSurfaceEvent";

      default:
         break;
   }

   return "QEvent";
}

#ifndef QT_NO_DRAGANDDROP
static void formatDropEvent(QDebug d, const QDropEvent *e)
{
   const QEvent::Type type = e->type();
   d << eventClassName(type) << "(dropAction=";
   QtDebugUtils::formatQEnum(d, e->dropAction());
   d << ", proposedAction=";
   QtDebugUtils::formatQEnum(d, e->proposedAction());
   d << ", possibleActions=";
   QtDebugUtils::formatQFlags(d, e->possibleActions());
   d << ", posF=";
   QtDebugUtils::formatQPoint(d,  e->posF());
   if (type == QEvent::DragMove || type == QEvent::DragEnter) {
      d << ", answerRect=" << static_cast<const QDragMoveEvent *>(e)->answerRect();
   }
   d << ", formats=" << e->mimeData()->formats();
   QtDebugUtils::formatNonNullQFlags(d, ", keyboardModifiers=", e->keyboardModifiers());
   d << ", ";
   QtDebugUtils::formatQFlags(d, e->mouseButtons());
}
#endif

#ifndef QT_NO_TABLETEVENT

static void formatTabletEvent(QDebug d, const QTabletEvent *e)
{
   const QEvent::Type type = e->type();

   d << eventClassName(type)  << '(';
   QtDebugUtils::formatQEnum(d, type);
   d << ", device=";
   QtDebugUtils::formatQEnum(d, e->device());
   d << ", pointerType=";
   QtDebugUtils::formatQEnum(d, e->pointerType());
   d << ", uniqueId=" << e->uniqueId()
      << ", pos=" << e->posF()
      << ", z=" << e->z()
      << ", xTilt=" << e->xTilt()
      << ", yTilt=" << e->yTilt()
      << ", ";
   QtDebugUtils::formatQFlags(d, e->buttons());
   if (type == QEvent::TabletPress || type == QEvent::TabletMove) {
      d << ", pressure=" << e->pressure();
   }
   if (e->device() == QTabletEvent::RotationStylus || e->device() == QTabletEvent::FourDMouse) {
      d << ", rotation=" << e->rotation();
   }
   if (e->device() == QTabletEvent::Airbrush) {
      d << ", tangentialPressure=" << e->tangentialPressure();
   }
}

#endif // !QT_NO_TABLETEVENT

QDebug operator<<(QDebug dbg, const QTouchEvent::TouchPoint &tp)
{
   QDebugStateSaver saver(dbg);
   dbg.nospace();
   dbg << "TouchPoint(" << tp.id() << " (";
   QtDebugUtils::formatQRect(dbg, tp.rect());
   dbg << ") ";
   QtDebugUtils::formatQEnum(dbg, tp.state());
   dbg << " press " << tp.pressure() << " vel " << tp.velocity()
      << " start (";
   QtDebugUtils::formatQPoint(dbg, tp.startPos());
   dbg << ") last (";
   QtDebugUtils::formatQPoint(dbg, tp.lastPos());
   dbg << ") delta (";
   QtDebugUtils::formatQPoint(dbg, tp.pos() - tp.lastPos());
   dbg << ')';
   return dbg;
}

QDebug operator<<(QDebug dbg, const QEvent *e)
{
   QDebugStateSaver saver(dbg);
   dbg.nospace();

   if (!e) {
      dbg << "QEvent(this = 0x0)";
      return dbg;
   }

   // More useful event output could be added here
   const QEvent::Type type = e->type();
   switch (type) {
      case QEvent::Expose:
         dbg << "QExposeEvent(" << static_cast<const QExposeEvent *>(e)->region() << ')';
         break;

      case QEvent::Paint:
         dbg << "QPaintEvent(" << static_cast<const QPaintEvent *>(e)->region() << ')';
         break;

      case QEvent::MouseButtonPress:
      case QEvent::MouseMove:
      case QEvent::MouseButtonRelease:
      case QEvent::MouseButtonDblClick:
      case QEvent::NonClientAreaMouseButtonPress:
      case QEvent::NonClientAreaMouseMove:
      case QEvent::NonClientAreaMouseButtonRelease:
      case QEvent::NonClientAreaMouseButtonDblClick: {
         const QMouseEvent *me = static_cast<const QMouseEvent *>(e);
         const Qt::MouseButton button   = me->button();
         const Qt::MouseButtons buttons = me->buttons();
         dbg << "QMouseEvent(";

         QtDebugUtils::formatQEnum(dbg, type);
         if (type != QEvent::MouseMove && type != QEvent::NonClientAreaMouseMove) {
            dbg << ", ";
            QtDebugUtils::formatQEnum(dbg, button);
         }

         if (buttons && (Qt::MouseButtons(button) != buttons)) {
            dbg << ", buttons=";
            QtDebugUtils::formatQFlags(dbg, buttons);
         }

         QtDebugUtils::formatNonNullQFlags(dbg, ", ", me->modifiers());
         dbg << ", localPos=";

         QtDebugUtils::formatQPoint(dbg, me->localPos());
         dbg << ", screenPos=";

         QtDebugUtils::formatQPoint(dbg, me->screenPos());
         QtDebugUtils::formatNonNullQEnum(dbg, ", ", me->source());
         QtDebugUtils::formatNonNullQFlags(dbg, ", flags=", me->flags());
         dbg << ')';
      }

      break;

#ifndef QT_NO_WHEELEVENT
      case QEvent::Wheel: {
         const QWheelEvent *we = static_cast<const QWheelEvent *>(e);
         dbg << "QWheelEvent(" << "pixelDelta=" << we->pixelDelta() << ", angleDelta=" << we->angleDelta() << ')';
      }
      break;
#endif

      case QEvent::KeyPress:
      case QEvent::KeyRelease:
      case QEvent::ShortcutOverride: {
         const QKeyEvent *ke = static_cast<const QKeyEvent *>(e);
         dbg << "QKeyEvent(";
         QtDebugUtils::formatQEnum(dbg, type);
         dbg << ", ";
         QtDebugUtils::formatQEnum(dbg, static_cast<Qt::Key>(ke->key()));
         QtDebugUtils::formatNonNullQFlags(dbg, ", ", ke->modifiers());
         if (!ke->text().isEmpty()) {
            dbg << ", text=" << ke->text();
         }
         if (ke->isAutoRepeat()) {
            dbg << ", autorepeat, count=" << ke->count();
         }
         dbg << ')';
      }
      break;

      case QEvent::Shortcut: {
         const QShortcutEvent *se = static_cast<const QShortcutEvent *>(e);
         dbg << "QShortcutEvent(" << se->key().toString() << ", id=" << se->shortcutId();
         if (se->isAmbiguous()) {
            dbg << ", ambiguous";
         }
         dbg << ')';
      }
      break;

      case QEvent::FocusAboutToChange:
      case QEvent::FocusIn:
      case QEvent::FocusOut:
         dbg << "QFocusEvent(";
         QtDebugUtils::formatQEnum(dbg, type);
         dbg << ", ";
         QtDebugUtils::formatQEnum(dbg, static_cast<const QFocusEvent *>(e)->reason());
         dbg << ')';
         break;
      case QEvent::Move: {
         const QMoveEvent *me = static_cast<const QMoveEvent *>(e);
         dbg << "QMoveEvent(";
         QtDebugUtils::formatQPoint(dbg, me->pos());
         if (!me->spontaneous()) {
            dbg << ", non-spontaneous";
         }
         dbg << ')';
      }
      break;

      case QEvent::Resize: {
         const QResizeEvent *re = static_cast<const QResizeEvent *>(e);
         dbg << "QResizeEvent(";
         QtDebugUtils::formatQSize(dbg, re->size());
         if (!re->spontaneous()) {
            dbg << ", non-spontaneous";
         }
         dbg << ')';
      }
      break;

#ifndef QT_NO_DRAGANDDROP
      case QEvent::DragEnter:
      case QEvent::DragMove:
      case QEvent::Drop:
         formatDropEvent(dbg, static_cast<const QDropEvent *>(e));
         break;
#endif

      case QEvent::InputMethod:
         formatInputMethodEvent(dbg, static_cast<const QInputMethodEvent *>(e));
         break;

      case QEvent::InputMethodQuery:
         formatInputMethodQueryEvent(dbg, static_cast<const QInputMethodQueryEvent *>(e));
         break;

      case QEvent::TouchBegin:
      case QEvent::TouchUpdate:
      case QEvent::TouchEnd:
         formatTouchEvent(dbg, *static_cast<const QTouchEvent *>(e));
         break;

      case QEvent::ChildAdded:
      case QEvent::ChildPolished:
      case QEvent::ChildRemoved:
         dbg << "QChildEvent(";
         QtDebugUtils::formatQEnum(dbg, type);
         dbg << ", " << (static_cast<const QChildEvent *>(e))->child() << ')';
         break;

#ifndef QT_NO_GESTURES
      case QEvent::NativeGesture: {
         const QNativeGestureEvent *ne = static_cast<const QNativeGestureEvent *>(e);
         dbg << "QNativeGestureEvent(";
         QtDebugUtils::formatQEnum(dbg, ne->gestureType());
         dbg << "localPos=";
         QtDebugUtils::formatQPoint(dbg, ne->localPos());
         dbg << ", value=" << ne->value() << ')';
      }
      break;
#endif

      case QEvent::ApplicationStateChange:
         dbg << "QApplicationStateChangeEvent(";
         QtDebugUtils::formatQEnum(dbg, static_cast<const QApplicationStateChangeEvent *>(e)->applicationState());
         dbg << ')';
         break;

      case QEvent::ContextMenu:
         dbg << "QContextMenuEvent(" << static_cast<const QContextMenuEvent *>(e)->pos() << ')';
         break;

#ifndef QT_NO_TABLETEVENT
      case QEvent::TabletEnterProximity:
      case QEvent::TabletLeaveProximity:
      case QEvent::TabletPress:
      case QEvent::TabletMove:
      case QEvent::TabletRelease:
         formatTabletEvent(dbg, static_cast<const QTabletEvent *>(e));
         break;
#endif

      case QEvent::Enter:
         dbg << "QEnterEvent(" << static_cast<const QEnterEvent *>(e)->pos() << ')';
         break;

      case QEvent::Timer:
         dbg << "QTimerEvent(id=" << static_cast<const QTimerEvent *>(e)->timerId() << ')';
         break;

      case QEvent::PlatformSurface:
         dbg << "QPlatformSurfaceEvent(surfaceEventType=";
         switch (static_cast<const QPlatformSurfaceEvent *>(e)->surfaceEventType()) {
            case QPlatformSurfaceEvent::SurfaceCreated:
               dbg << "SurfaceCreated";
               break;
            case QPlatformSurfaceEvent::SurfaceAboutToBeDestroyed:
               dbg << "SurfaceAboutToBeDestroyed";
               break;
         }
         dbg << ')';
         break;

      default:
         dbg << eventClassName(type) << '(';
         QtDebugUtils::formatQEnum(dbg, type);
         dbg << ", " << (const void *)e << ')';
         break;
   }
   return dbg;
}
// end debug stream

QWindowStateChangeEvent::QWindowStateChangeEvent(Qt::WindowStates s, bool isOverride)
   : QEvent(WindowStateChange), ostate(s), m_override(isOverride)
{
}

// internal
bool QWindowStateChangeEvent::isOverride() const
{
   return m_override;
}

// internal
QWindowStateChangeEvent::~QWindowStateChangeEvent()
{}

QTouchEvent::QTouchEvent(QEvent::Type eventType, QTouchDevice *device, Qt::KeyboardModifiers modifiers,
            Qt::TouchPointStates touchPointStates, const QList<QTouchEvent::TouchPoint> &touchPoints)
   : QInputEvent(eventType, modifiers), _window(nullptr), _target(nullptr), _device(device),
     _touchPointStates(touchPointStates), _touchPoints(touchPoints)
{
}

QTouchEvent::~QTouchEvent()
{
}

// internal
QTouchEvent::TouchPoint::TouchPoint(int id)
   : d(new QTouchEventTouchPointPrivate(id))
{
}

// internal
QTouchEvent::TouchPoint::TouchPoint(const QTouchEvent::TouchPoint &other)
   : d(other.d)
{
   d->ref.ref();
}

// internal
QTouchEvent::TouchPoint::~TouchPoint()
{
   if (d && ! d->ref.deref()) {
      delete d;
   }
}

int QTouchEvent::TouchPoint::id() const
{
   return d->id;
}

Qt::TouchPointState QTouchEvent::TouchPoint::state() const
{
   return Qt::TouchPointState(int(d->state));
}

QPointF QTouchEvent::TouchPoint::pos() const
{
   return d->rect.center();
}

QPointF QTouchEvent::TouchPoint::scenePos() const
{
   return d->sceneRect.center();
}

QPointF QTouchEvent::TouchPoint::screenPos() const
{
   return d->screenRect.center();
}

QPointF QTouchEvent::TouchPoint::normalizedPos() const
{
   return d->normalizedPos;
}

QPointF QTouchEvent::TouchPoint::startPos() const
{
   return d->startPos;
}

QPointF QTouchEvent::TouchPoint::startScenePos() const
{
   return d->startScenePos;
}

QPointF QTouchEvent::TouchPoint::startScreenPos() const
{
   return d->startScreenPos;
}

QPointF QTouchEvent::TouchPoint::startNormalizedPos() const
{
   return d->startNormalizedPos;
}

QPointF QTouchEvent::TouchPoint::lastPos() const
{
   return d->lastPos;
}

QPointF QTouchEvent::TouchPoint::lastScenePos() const
{
   return d->lastScenePos;
}

QPointF QTouchEvent::TouchPoint::lastScreenPos() const
{
   return d->lastScreenPos;
}

QPointF QTouchEvent::TouchPoint::lastNormalizedPos() const
{
   return d->lastNormalizedPos;
}

QRectF QTouchEvent::TouchPoint::rect() const
{
   return d->rect;
}

QRectF QTouchEvent::TouchPoint::sceneRect() const
{
   return d->sceneRect;
}

QRectF QTouchEvent::TouchPoint::screenRect() const
{
   return d->screenRect;
}

qreal QTouchEvent::TouchPoint::pressure() const
{
   return d->pressure;
}

QVector2D QTouchEvent::TouchPoint::velocity() const
{
   return d->velocity;
}

QTouchEvent::TouchPoint::InfoFlags QTouchEvent::TouchPoint::flags() const
{
   return d->flags;
}

QVector<QPointF> QTouchEvent::TouchPoint::rawScreenPositions() const
{
   return d->rawScreenPositions;
}

// internal
void QTouchEvent::TouchPoint::setId(int id)
{
   if (d->ref.load() != 1) {
      d = d->detach();
   }
   d->id = id;
}

// internal
void QTouchEvent::TouchPoint::setState(Qt::TouchPointStates state)
{
   if (d->ref.load() != 1) {
      d = d->detach();
   }
   d->state = state;
}

// internal
void QTouchEvent::TouchPoint::setPos(const QPointF &pos)
{
   if (d->ref.load() != 1) {
      d = d->detach();
   }
   d->rect.moveCenter(pos);
}

// internal
void QTouchEvent::TouchPoint::setScenePos(const QPointF &scenePos)
{
   if (d->ref.load() != 1) {
      d = d->detach();
   }
   d->sceneRect.moveCenter(scenePos);
}

// internal
void QTouchEvent::TouchPoint::setScreenPos(const QPointF &screenPos)
{
   if (d->ref.load() != 1) {
      d = d->detach();
   }
   d->screenRect.moveCenter(screenPos);
}

// internal
void QTouchEvent::TouchPoint::setNormalizedPos(const QPointF &normalizedPos)
{
   if (d->ref.load() != 1) {
      d = d->detach();
   }
   d->normalizedPos = normalizedPos;
}

// internal
void QTouchEvent::TouchPoint::setStartPos(const QPointF &startPos)
{
   if (d->ref.load() != 1) {
      d = d->detach();
   }
   d->startPos = startPos;
}

// internal
void QTouchEvent::TouchPoint::setStartScenePos(const QPointF &startScenePos)
{
   if (d->ref.load() != 1) {
      d = d->detach();
   }

   d->startScenePos = startScenePos;
}

// internal
void QTouchEvent::TouchPoint::setStartScreenPos(const QPointF &startScreenPos)
{
   if (d->ref.load() != 1) {
      d = d->detach();
   }
   d->startScreenPos = startScreenPos;
}

// internal
void QTouchEvent::TouchPoint::setStartNormalizedPos(const QPointF &startNormalizedPos)
{
   if (d->ref.load() != 1) {
      d = d->detach();
   }
   d->startNormalizedPos = startNormalizedPos;
}

// internal
void QTouchEvent::TouchPoint::setLastPos(const QPointF &lastPos)
{
   if (d->ref.load() != 1) {
      d = d->detach();
   }
   d->lastPos = lastPos;
}

// internal
void QTouchEvent::TouchPoint::setLastScenePos(const QPointF &lastScenePos)
{
   if (d->ref.load() != 1) {
      d = d->detach();
   }
   d->lastScenePos = lastScenePos;
}

// internal
void QTouchEvent::TouchPoint::setLastScreenPos(const QPointF &lastScreenPos)
{
   if (d->ref.load() != 1) {
      d = d->detach();
   }
   d->lastScreenPos = lastScreenPos;
}

// internal
void QTouchEvent::TouchPoint::setLastNormalizedPos(const QPointF &lastNormalizedPos)
{
   if (d->ref.load() != 1) {
      d = d->detach();
   }
   d->lastNormalizedPos = lastNormalizedPos;
}

// internal
void QTouchEvent::TouchPoint::setRect(const QRectF &rect)
{
   if (d->ref.load() != 1) {
      d = d->detach();
   }
   d->rect = rect;
}

// internal
void QTouchEvent::TouchPoint::setSceneRect(const QRectF &sceneRect)
{
   if (d->ref.load() != 1) {
      d = d->detach();
   }
   d->sceneRect = sceneRect;
}

/*! \internal */
void QTouchEvent::TouchPoint::setScreenRect(const QRectF &screenRect)
{
   if (d->ref.load() != 1) {
      d = d->detach();
   }
   d->screenRect = screenRect;
}

// internal
void QTouchEvent::TouchPoint::setPressure(qreal pressure)
{
   if (d->ref.load() != 1) {
      d = d->detach();
   }
   d->pressure = pressure;
}

void QTouchEvent::TouchPoint::setVelocity(const QVector2D &v)
{
   if (d->ref.load() != 1) {
      d = d->detach();
   }

   d->velocity = v;
}

void QTouchEvent::TouchPoint::setRawScreenPositions(const QVector<QPointF> &positions)
{
   if (d->ref.load() != 1) {
      d = d->detach();
   }

   d->rawScreenPositions = positions;
}

void QTouchEvent::TouchPoint::setFlags(InfoFlags flags)
{
   if (d->ref.load() != 1) {
      d = d->detach();
   }

   d->flags = flags;
}

QScrollPrepareEvent::QScrollPrepareEvent(const QPointF &startPos)
   : QEvent(QEvent::ScrollPrepare), m_startPos(startPos)
{
}

QScrollPrepareEvent::~QScrollPrepareEvent()
{
}

QPointF QScrollPrepareEvent::startPos() const
{
   return m_startPos;
}

QSizeF QScrollPrepareEvent::viewportSize() const
{
   return m_viewportSize;
}
QRectF QScrollPrepareEvent::contentPosRange() const
{
   return m_contentPosRange;
}

QPointF QScrollPrepareEvent::contentPos() const
{
   return m_contentPos;
}

void QScrollPrepareEvent::setViewportSize(const QSizeF &size)
{
   m_viewportSize = size;
}

void QScrollPrepareEvent::setContentPosRange(const QRectF &rect)
{
   m_contentPosRange = rect;
}

void QScrollPrepareEvent::setContentPos(const QPointF &pos)
{
   m_contentPos = pos;
}

QScrollEvent::QScrollEvent(const QPointF &contentPos, const QPointF &overshootDistance, ScrollState scrollState)
   : QEvent(QEvent::Scroll), m_contentPos(contentPos), m_overshoot(overshootDistance), m_state(scrollState)
{
}

QScrollEvent::~QScrollEvent()
{
}

QPointF QScrollEvent::contentPos() const
{
   return m_contentPos;
}

QPointF QScrollEvent::overshootDistance() const
{
   return m_overshoot;
}

QScrollEvent::ScrollState QScrollEvent::scrollState() const
{
   return m_state;
}

QScreenOrientationChangeEvent::QScreenOrientationChangeEvent(QScreen *screen, Qt::ScreenOrientation screenOrientation)
   : QEvent(QEvent::OrientationChange), m_screen(screen), m_orientation(screenOrientation)
{
}

QScreenOrientationChangeEvent::~QScreenOrientationChangeEvent()
{
}

QScreen *QScreenOrientationChangeEvent::screen() const
{
   return m_screen;
}

Qt::ScreenOrientation QScreenOrientationChangeEvent::orientation() const
{
   return m_orientation;
}

QApplicationStateChangeEvent::QApplicationStateChangeEvent(Qt::ApplicationState applicationState)
   : QEvent(QEvent::ApplicationStateChange), m_applicationState(applicationState)
{
}

Qt::ApplicationState QApplicationStateChangeEvent::applicationState() const
{
   return m_applicationState;
}
