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

#include <qwin_drag.h>
#include <qwin_context.h>
#include <qwin_screen.h>

#ifndef QT_NO_CLIPBOARD
#  include <qwin_clipboard.h>
#endif

#include <qwin_integration.h>
#include <qwin_ole.h>
#include <qwin_additional.h>
#include <qwin_window.h>
#include <qwin_mousehandler.h>
#include <qwin_cursor.h>
#include <qdebug.h>
#include <qbuffer.h>
#include <qpoint.h>
#include <QMouseEvent>
#include <QPixmap>
#include <QPainter>
#include <QRasterWindow>
#include <QApplication>

#include <qwindowsysteminterface_p.h>
#include <qapplication_p.h>
#include <qhighdpiscaling_p.h>

#include <shlobj.h>

class QWindowsDragCursorWindow : public QRasterWindow
{
 public:
   explicit QWindowsDragCursorWindow(QWindow *parent = nullptr);

   void setPixmap(const QPixmap &p);

 protected:
   void paintEvent(QPaintEvent *) override {
      QPainter painter(this);
      painter.drawPixmap(0, 0, m_pixmap);
   }

 private:
   QPixmap m_pixmap;
};

QWindowsDragCursorWindow::QWindowsDragCursorWindow(QWindow *parent)
   : QRasterWindow(parent)
{
   QSurfaceFormat windowFormat = format();

   windowFormat.setAlphaBufferSize(8);
   setFormat(windowFormat);
   setObjectName(QString("QWindowsDragCursorWindow"));

   setFlags(Qt::Popup | Qt::NoDropShadowWindowHint
      | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint
      | Qt::WindowDoesNotAcceptFocus | Qt::WindowTransparentForInput);
}

void QWindowsDragCursorWindow::setPixmap(const QPixmap &p)
{
   if (p.cacheKey() == m_pixmap.cacheKey()) {
      return;
   }

   const QSize oldSize = m_pixmap.size();
   QSize newSize = p.size();

   m_pixmap = p;
   if (oldSize != newSize) {
      const qreal pixDevicePixelRatio = p.devicePixelRatio();
      if (pixDevicePixelRatio > 1.0 && qFuzzyCompare(pixDevicePixelRatio, devicePixelRatio())) {
         newSize /= qRound(pixDevicePixelRatio);
      }
      resize(newSize);
   }
   if (isVisible()) {
      update();
   }
}

/*!
    \class QWindowsDropMimeData
    \brief Special mime data class for data retrieval from Drag operations.

    Implementation of QWindowsInternalMimeDataBase which retrieves the
    current drop data object from QWindowsDrag.

    \sa QWindowsDrag
    \internal
    \ingroup qt-lighthouse-win
*/

IDataObject *QWindowsDropMimeData::retrieveDataObject() const
{
   return QWindowsDrag::instance()->dropDataObject();
}

static inline Qt::DropActions translateToQDragDropActions(DWORD pdwEffects)
{
   Qt::DropActions actions = Qt::IgnoreAction;
   if (pdwEffects & DROPEFFECT_LINK) {
      actions |= Qt::LinkAction;
   }
   if (pdwEffects & DROPEFFECT_COPY) {
      actions |= Qt::CopyAction;
   }
   if (pdwEffects & DROPEFFECT_MOVE) {
      actions |= Qt::MoveAction;
   }
   return actions;
}

static inline Qt::DropAction translateToQDragDropAction(DWORD pdwEffect)
{
   if (pdwEffect & DROPEFFECT_LINK) {
      return Qt::LinkAction;
   }
   if (pdwEffect & DROPEFFECT_COPY) {
      return Qt::CopyAction;
   }
   if (pdwEffect & DROPEFFECT_MOVE) {
      return Qt::MoveAction;
   }
   return Qt::IgnoreAction;
}

static inline DWORD translateToWinDragEffects(Qt::DropActions action)
{
   DWORD effect = DROPEFFECT_NONE;
   if (action & Qt::LinkAction) {
      effect |= DROPEFFECT_LINK;
   }
   if (action & Qt::CopyAction) {
      effect |= DROPEFFECT_COPY;
   }
   if (action & Qt::MoveAction) {
      effect |= DROPEFFECT_MOVE;
   }
   return effect;
}

static inline Qt::KeyboardModifiers toQtKeyboardModifiers(DWORD keyState)
{
   Qt::KeyboardModifiers modifiers = Qt::NoModifier;

   if (keyState & MK_SHIFT) {
      modifiers |= Qt::ShiftModifier;
   }
   if (keyState & MK_CONTROL) {
      modifiers |= Qt::ControlModifier;
   }
   if (keyState & MK_ALT) {
      modifiers |= Qt::AltModifier;
   }

   return modifiers;
}

/*!
    \class QWindowsOleDropSource
    \brief Implementation of IDropSource

    Used for drag operations.

    \sa QWindowsDrag
    \internal
    \ingroup qt-lighthouse-win
*/

class QWindowsOleDropSource : public IDropSource
{
 public:
   enum Mode {
      MouseDrag,
      TouchDrag // Mouse cursor suppressed, use window as cursor.
   };

   explicit QWindowsOleDropSource(QWindowsDrag *drag);
   virtual ~QWindowsOleDropSource();

   void createCursors();

   // IUnknown methods
   STDMETHOD(QueryInterface)(REFIID riid, void **ppvObj) override;
   STDMETHOD_(ULONG, AddRef)(void) override;
   STDMETHOD_(ULONG, Release)(void) override;

   // IDropSource methods
   STDMETHOD(QueryContinueDrag)(BOOL fEscapePressed, DWORD grfKeyState) override;
   STDMETHOD(GiveFeedback)(DWORD dwEffect) override;

 private:
   struct CursorEntry {
      CursorEntry()
         : cacheKey(0)
      { }

      CursorEntry(const QPixmap &p, qint64 cK, const CursorHandlePtr &c, const QPoint &h)
         : pixmap(p), cacheKey(cK), cursor(c), hotSpot(h)
      { }

      QPixmap pixmap;
      qint64 cacheKey; // Cache key of cursor
      CursorHandlePtr cursor;
      QPoint hotSpot;
   };

   const Mode m_mode;
   QWindowsDrag *m_drag;
   Qt::MouseButtons m_currentButtons;

   QMap<Qt::DropAction, CursorEntry> m_cursors;
   QWindowsDragCursorWindow *m_touchDragWindow;

   ULONG m_refs;
};

QWindowsOleDropSource::QWindowsOleDropSource(QWindowsDrag *drag)
   : m_mode(QWindowsCursor::cursorState() != QWindowsCursor::CursorSuppressed ? MouseDrag : TouchDrag),
     m_drag(drag), m_currentButtons(Qt::NoButton), m_touchDragWindow(nullptr), m_refs(1)
{
}

QWindowsOleDropSource::~QWindowsOleDropSource()
{
   m_cursors.clear();
   delete m_touchDragWindow;
}

void QWindowsOleDropSource::createCursors()
{
   const QDrag *drag = m_drag->currentDrag();
   const QPixmap pixmap = drag->pixmap();
   const bool hasPixmap = !pixmap.isNull();

   // Find screen for drag. Could be obtained from QDrag::source(), but that might be a QWidget.
   const QPlatformScreen *platformScreen = QWindowsContext::instance()->screenManager().screenAtDp(QWindowsCursor::mousePosition());
   if (!platformScreen) {
      if (const QScreen *primaryScreen = QApplication::primaryScreen()) {
         platformScreen = primaryScreen->handle();
      }
   }
   Q_ASSERT(platformScreen);
   QPlatformCursor *platformCursor = platformScreen->cursor();

   qreal pixmapScaleFactor = 1;
   qreal hotSpotScaleFactor = 1;
   if (m_mode != TouchDrag) { // Touch drag: pixmap is shown in a separate QWindow, which will be scaled.)
      hotSpotScaleFactor = QHighDpiScaling::factor(platformScreen);
      pixmapScaleFactor = hotSpotScaleFactor / pixmap.devicePixelRatio();
   }
   QPixmap scaledPixmap = qFuzzyCompare(pixmapScaleFactor, 1.0)
      ? pixmap
      :  pixmap.scaled((QSizeF(pixmap.size()) * pixmapScaleFactor).toSize(),
         Qt::KeepAspectRatio, Qt::SmoothTransformation);

   scaledPixmap.setDevicePixelRatio(1);

   Qt::DropAction actions[] = { Qt::MoveAction, Qt::CopyAction, Qt::LinkAction, Qt::IgnoreAction };
   int actionCount = int(sizeof(actions) / sizeof(actions[0]));

   if (! hasPixmap) {
      --actionCount;   // No Qt::IgnoreAction unless pixmap
   }

   const QPoint hotSpot = qFuzzyCompare(hotSpotScaleFactor, 1.0)
      ?  drag->hotSpot()
      : (QPointF(drag->hotSpot()) * hotSpotScaleFactor).toPoint();

   for (int cnum = 0; cnum < actionCount; ++cnum) {
      const Qt::DropAction action = actions[cnum];
      QPixmap cursorPixmap = drag->dragCursor(action);

      if (cursorPixmap.isNull() && platformCursor) {
         cursorPixmap = static_cast<QWindowsCursor *>(platformCursor)->dragDefaultCursor(action);
      }

      const qint64 cacheKey = cursorPixmap.cacheKey();
      const auto iter = m_cursors.find(action);

      if (iter != m_cursors.end() && iter.value().cacheKey == cacheKey) {
         continue;
      }

      if (cursorPixmap.isNull()) {
         qWarning("QWindowsOleDropSource::createCursors() Unable to obtain drag cursor for %d", action);
         continue;
      }

      QPoint newHotSpot(0, 0);
      QPixmap newPixmap = cursorPixmap;

      if (hasPixmap) {
         const int x1 = qMin(-hotSpot.x(), 0);
         const int x2 = qMax(scaledPixmap.width() - hotSpot.x(), cursorPixmap.width());
         const int y1 = qMin(-hotSpot.y(), 0);
         const int y2 = qMax(scaledPixmap.height() - hotSpot.y(), cursorPixmap.height());

         QPixmap newCursor(x2 - x1 + 1, y2 - y1 + 1);
         newCursor.fill(Qt::transparent);

         QPainter p(&newCursor);
         const QPoint pmDest = QPoint(qMax(0, -hotSpot.x()), qMax(0, -hotSpot.y()));

         p.drawPixmap(pmDest, scaledPixmap);
         p.drawPixmap(qMax(0, hotSpot.x()), qMax(0, hotSpot.y()), cursorPixmap);
         newPixmap = newCursor;
         newHotSpot = QPoint(qMax(0, hotSpot.x()), qMax(0, hotSpot.y()));
      }

      if (const HCURSOR sysCursor = QWindowsCursor::createPixmapCursor(newPixmap, newHotSpot)) {
         const CursorEntry entry(newPixmap, cacheKey, CursorHandlePtr(new CursorHandle(sysCursor)), newHotSpot);

         if (iter == m_cursors.end()) {
            m_cursors.insert(action, entry);
         } else {
            iter.value() = entry;
         }
      }
   }
}

//---------------------------------------------------------------------
//                    IUnknown Methods
//---------------------------------------------------------------------

STDMETHODIMP QWindowsOleDropSource::QueryInterface(REFIID iid, void FAR *FAR *ppv)
{
   if (iid == IID_IUnknown || iid == IID_IDropSource) {
      *ppv = this;
      ++m_refs;
      return NOERROR;
   }

   *ppv = nullptr;

   return ResultFromScode(E_NOINTERFACE);
}

STDMETHODIMP_(ULONG)
QWindowsOleDropSource::AddRef(void)
{
   return ++m_refs;
}

STDMETHODIMP_(ULONG)
QWindowsOleDropSource::Release(void)
{
   if (--m_refs == 0) {
      delete this;
      return 0;
   }
   return m_refs;
}

/*!
    \brief Check for cancel.
*/

QT_ENSURE_STACK_ALIGNED_FOR_SSE STDMETHODIMP QWindowsOleDropSource::QueryContinueDrag(BOOL fEscapePressed, DWORD grfKeyState)
{
   HRESULT hr = S_OK;
   do {
      if (fEscapePressed) {
         hr = ResultFromScode(DRAGDROP_S_CANCEL);
         break;
      }

      // grfKeyState is broken on CE & some Windows XP versions,
      // therefore we need to check the state manually
      if ((GetAsyncKeyState(VK_LBUTTON) == 0)
         && (GetAsyncKeyState(VK_MBUTTON) == 0)
         && (GetAsyncKeyState(VK_RBUTTON) == 0)) {
         hr = ResultFromScode(DRAGDROP_S_DROP);
         break;
      }

      const Qt::MouseButtons buttons =  QWindowsMouseHandler::keyStateToMouseButtons(grfKeyState);
      if (m_currentButtons == Qt::NoButton) {
         m_currentButtons = buttons;
      } else {
         // Button changed: Complete Drop operation.
         if (!(m_currentButtons & buttons)) {
            hr = ResultFromScode(DRAGDROP_S_DROP);
            break;
         }
      }

      QApplication::processEvents();

   } while (false);

   return hr;
}

QT_ENSURE_STACK_ALIGNED_FOR_SSE STDMETHODIMP QWindowsOleDropSource::GiveFeedback(DWORD dwEffect)
{
   const Qt::DropAction action = translateToQDragDropAction(dwEffect);
   m_drag->updateAction(action);

   const qint64 currentCacheKey = m_drag->currentDrag()->dragCursor(action).cacheKey();
   auto iter = m_cursors.constFind(action);

   // If a custom drag cursor is set, check its cache key to detect changes.

   if (iter == m_cursors.constEnd() || (currentCacheKey && currentCacheKey != iter.value().cacheKey)) {
      createCursors();
      iter = m_cursors.constFind(action);
   }

   if (iter != m_cursors.constEnd()) {
      const CursorEntry &e = iter.value();

      switch (m_mode) {
         case MouseDrag:
            SetCursor(e.cursor->handle());
            break;

         case TouchDrag:
            if (!m_touchDragWindow) {
               m_touchDragWindow = new QWindowsDragCursorWindow;
            }

            m_touchDragWindow->setPixmap(e.pixmap);
            m_touchDragWindow->setFramePosition(QCursor::pos() - e.hotSpot);

            if (!m_touchDragWindow->isVisible()) {
               m_touchDragWindow->show();
            }

            break;
      }

      return ResultFromScode(S_OK);
   }

   return ResultFromScode(DRAGDROP_S_USEDEFAULTCURSORS);
}

QWindowsOleDropTarget::QWindowsOleDropTarget(QWindow *w) :
   m_refs(1), m_window(w), m_chosenEffect(0), m_lastKeyState(0)
{
}

QWindowsOleDropTarget::~QWindowsOleDropTarget()
{
}

STDMETHODIMP QWindowsOleDropTarget::QueryInterface(REFIID iid, void FAR *FAR *ppv)
{
   if (iid == IID_IUnknown || iid == IID_IDropTarget) {
      *ppv = this;
      AddRef();
      return NOERROR;
   }
   *ppv = nullptr;
   return ResultFromScode(E_NOINTERFACE);
}

STDMETHODIMP_(ULONG)
QWindowsOleDropTarget::AddRef(void)
{
   return ++m_refs;
}

STDMETHODIMP_(ULONG)
QWindowsOleDropTarget::Release(void)
{
   if (--m_refs == 0) {
      delete this;
      return 0;
   }
   return m_refs;
}

void QWindowsOleDropTarget::handleDrag(QWindow *window, DWORD grfKeyState,
   const QPoint &point, LPDWORD pdwEffect)
{
   Q_ASSERT(window);
   m_lastPoint = point;
   m_lastKeyState = grfKeyState;

   QWindowsDrag *windowsDrag = QWindowsDrag::instance();
   const Qt::DropActions actions = translateToQDragDropActions(*pdwEffect);
   QApplicationPrivate::modifier_buttons = toQtKeyboardModifiers(grfKeyState);
   QApplicationPrivate::mouse_buttons = QWindowsMouseHandler::keyStateToMouseButtons(grfKeyState);

   const QPlatformDragQtResponse response =
      QWindowSystemInterface::handleDrag(window, windowsDrag->dropData(), m_lastPoint, actions);

   m_answerRect = response.answerRect();
   const Qt::DropAction action = response.acceptedAction();

   if (response.isAccepted()) {
      m_chosenEffect = translateToWinDragEffects(action);
   } else {
      m_chosenEffect = DROPEFFECT_NONE;
   }

   *pdwEffect = m_chosenEffect;
}

QT_ENSURE_STACK_ALIGNED_FOR_SSE STDMETHODIMP QWindowsOleDropTarget::DragEnter(LPDATAOBJECT pDataObj, DWORD grfKeyState,
   POINTL pt, LPDWORD pdwEffect)
{
   if (IDropTargetHelper *dh = QWindowsDrag::instance()->dropHelper()) {
      dh->DragEnter(reinterpret_cast<HWND>(m_window->winId()), pDataObj, reinterpret_cast<POINT *>(&pt), *pdwEffect);
   }

   QWindowsDrag::instance()->setDropDataObject(pDataObj);
   pDataObj->AddRef();
   const QPoint point = QWindowsGeometryHint::mapFromGlobal(m_window, QPoint(pt.x, pt.y));
   handleDrag(m_window, grfKeyState, point, pdwEffect);
   return NOERROR;
}

QT_ENSURE_STACK_ALIGNED_FOR_SSE STDMETHODIMP QWindowsOleDropTarget::DragOver(DWORD grfKeyState, POINTL pt, LPDWORD pdwEffect)
{
   if (IDropTargetHelper *dh = QWindowsDrag::instance()->dropHelper()) {
      dh->DragOver(reinterpret_cast<POINT *>(&pt), *pdwEffect);
   }

   const QPoint tmpPoint = QWindowsGeometryHint::mapFromGlobal(m_window, QPoint(pt.x, pt.y));

   // see if we should compress this event
   if ((tmpPoint == m_lastPoint || m_answerRect.contains(tmpPoint)) && m_lastKeyState == grfKeyState) {
      *pdwEffect = m_chosenEffect;
      return NOERROR;
   }

   handleDrag(m_window, grfKeyState, tmpPoint, pdwEffect);
   return NOERROR;
}

QT_ENSURE_STACK_ALIGNED_FOR_SSE STDMETHODIMP QWindowsOleDropTarget::DragLeave()
{
   if (IDropTargetHelper *dh = QWindowsDrag::instance()->dropHelper()) {
      dh->DragLeave();
   }

   QWindowSystemInterface::handleDrag(m_window, nullptr, QPoint(), Qt::IgnoreAction);
   QWindowsDrag::instance()->releaseDropDataObject();

   return NOERROR;
}

#define KEY_STATE_BUTTON_MASK (MK_LBUTTON | MK_MBUTTON | MK_RBUTTON)

QT_ENSURE_STACK_ALIGNED_FOR_SSE STDMETHODIMP QWindowsOleDropTarget::Drop(LPDATAOBJECT pDataObj, DWORD grfKeyState,
   POINTL pt, LPDWORD pdwEffect)
{
   if (IDropTargetHelper *dh = QWindowsDrag::instance()->dropHelper()) {
      dh->Drop(pDataObj, reinterpret_cast<POINT *>(&pt), *pdwEffect);
   }

   m_lastPoint = QWindowsGeometryHint::mapFromGlobal(m_window, QPoint(pt.x, pt.y));

   // grfKeyState does not all ways contain button state in the drop so if
   // it doesn't then use the last known button state;
   if ((grfKeyState & KEY_STATE_BUTTON_MASK) == 0) {
      grfKeyState |= m_lastKeyState & KEY_STATE_BUTTON_MASK;
   }
   m_lastKeyState = grfKeyState;

   QWindowsDrag *windowsDrag = QWindowsDrag::instance();

   const QPlatformDropQtResponse response = QWindowSystemInterface::handleDrop(m_window, windowsDrag->dropData(),
                  m_lastPoint, translateToQDragDropActions(*pdwEffect));

   if (response.isAccepted()) {
      const Qt::DropAction action = response.acceptedAction();
      if (action == Qt::MoveAction || action == Qt::TargetMoveAction) {
         if (action == Qt::MoveAction) {
            m_chosenEffect = DROPEFFECT_MOVE;
         } else {
            m_chosenEffect = DROPEFFECT_COPY;
         }
         HGLOBAL hData = GlobalAlloc(0, sizeof(DWORD));
         if (hData) {
            DWORD *moveEffect = reinterpret_cast<DWORD *>(GlobalLock(hData));
            *moveEffect = DROPEFFECT_MOVE;
            GlobalUnlock(hData);
            STGMEDIUM medium;
            memset(&medium, 0, sizeof(STGMEDIUM));
            medium.tymed = TYMED_HGLOBAL;
            medium.hGlobal = hData;

            FORMATETC format;
            format.cfFormat = CLIPFORMAT(RegisterClipboardFormat(CFSTR_PERFORMEDDROPEFFECT));
            format.tymed    = TYMED_HGLOBAL;
            format.ptd      = nullptr;
            format.dwAspect = 1;
            format.lindex   = -1;
            windowsDrag->dropDataObject()->SetData(&format, &medium, true);
         }
      } else {
         m_chosenEffect = translateToWinDragEffects(action);
      }
   } else {
      m_chosenEffect = DROPEFFECT_NONE;
   }

   *pdwEffect = m_chosenEffect;

   windowsDrag->releaseDropDataObject();
   return NOERROR;
}

QWindowsDrag::QWindowsDrag()
   : m_dropDataObject(nullptr), m_cachedDropTargetHelper(nullptr)
{
}

QWindowsDrag::~QWindowsDrag()
{
   if (m_cachedDropTargetHelper) {
      m_cachedDropTargetHelper->Release();
   }
}

QMimeData *QWindowsDrag::dropData()
{
   if (const QDrag *drag = currentDrag()) {
      return drag->mimeData();
   }
   return &m_dropData;
}

IDropTargetHelper *QWindowsDrag::dropHelper()
{
   if (!m_cachedDropTargetHelper) {
      CoCreateInstance(CLSID_DragDropHelper, nullptr, CLSCTX_INPROC_SERVER,
         IID_IDropTargetHelper, reinterpret_cast<void **>(&m_cachedDropTargetHelper));
   }
   return m_cachedDropTargetHelper;
}

Qt::DropAction QWindowsDrag::drag(QDrag *drag)
{
   // TODO: Accessibility handling?
   QMimeData *dropData = drag->mimeData();
   Qt::DropAction dragResult = Qt::IgnoreAction;

   DWORD resultEffect;

   QWindowsOleDropSource *windowDropSource = new QWindowsOleDropSource(this);
   windowDropSource->createCursors();

   QWindowsOleDataObject *dropDataObject = new QWindowsOleDataObject(dropData);
   const Qt::DropActions possibleActions = drag->supportedActions();
   const DWORD allowedEffects = translateToWinDragEffects(possibleActions);

   const HRESULT r = DoDragDrop(dropDataObject, windowDropSource, allowedEffects, &resultEffect);
   const DWORD  reportedPerformedEffect = dropDataObject->reportedPerformedEffect();

   if (r == DRAGDROP_S_DROP) {
      if (reportedPerformedEffect == DROPEFFECT_MOVE && resultEffect != DROPEFFECT_MOVE) {
         dragResult = Qt::TargetMoveAction;
         resultEffect = DROPEFFECT_MOVE;
      } else {
         dragResult = translateToQDragDropAction(resultEffect);
      }

      // Force it to be a copy if an unsupported operation occurred.
      // This indicates a bug in the drop target.
      if (resultEffect != DROPEFFECT_NONE && !(resultEffect & allowedEffects)) {
         qWarning("QWindowsDrag::drag() Forcing Qt::CopyAction");
         dragResult = Qt::CopyAction;
      }
   }

   // clean up
   dropDataObject->releaseData();
   dropDataObject->Release();           // Will delete obj if refcount becomes 0
   windowDropSource->Release();         // Will delete src if refcount becomes 0

   return dragResult;
}

QWindowsDrag *QWindowsDrag::instance()
{
   return static_cast<QWindowsDrag *>(QWindowsIntegration::instance()->drag());
}

void QWindowsDrag::releaseDropDataObject()
{
   if (m_dropDataObject) {
      m_dropDataObject->Release();
      m_dropDataObject = nullptr;
   }
}
