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

#include <qwin_context.h>

#include <qapplication.h>
#include <qdebug.h>
#include <qinputmethodevent.h>
#include <qobject.h>
#include <qpalette.h>
#include <qrect.h>
#include <qrectf.h>
#include <qtextboundaryfinder.h>
#include <qtextcharformat.h>
#include <qwin_inputcontext.h>
#include <qwin_integration.h>
#include <qwin_mousehandler.h>
#include <qwin_window.h>

#include <qhighdpiscaling_p.h>

#include <algorithm>

// Cancel current IME composition.
static inline void imeNotifyCancelComposition(HWND hwnd)
{
   if (! hwnd) {
      qWarning() << "imeNotifyCancelComposition() Called with" << hwnd;
      return;
   }

   const HIMC himc = ImmGetContext(hwnd);
   ImmNotifyIME(himc, NI_COMPOSITIONSTR, CPS_CANCEL, 0);
   ImmReleaseContext(hwnd, himc);
}

static inline LCID languageIdFromLocaleId(LCID localeId)
{
   return localeId & 0xFFFF;
}

static inline LCID currentInputLanguageId()
{
   return languageIdFromLocaleId(reinterpret_cast<quintptr>(GetKeyboardLayout(0)));
}

Q_CORE_EXPORT QLocale qt_localeFromLCID(LCID id); // from qlocale_win.cpp

HIMC QWindowsInputContext::m_defaultContext = nullptr;

QWindowsInputContext::CompositionContext::CompositionContext()
   : hwnd(nullptr), haveCaret(false), position(0), isComposing(false), factor(1)
{
}

QWindowsInputContext::QWindowsInputContext()
   : m_WM_MSIME_MOUSE(RegisterWindowMessage(L"MSIMEMouseOperation")), m_endCompositionRecursionGuard(false),
     m_languageId(currentInputLanguageId()), m_locale(qt_localeFromLCID(m_languageId))
{
   connect(QApplication::inputMethod(), &QInputMethod::cursorRectangleChanged,
                  this, &QWindowsInputContext::cursorRectChanged);
}

QWindowsInputContext::~QWindowsInputContext()
{
}

bool QWindowsInputContext::hasCapability(Capability capability) const
{
   switch (capability) {
      case QPlatformInputContext::HiddenTextCapability:
         return false; // QTBUG-40691, do not show IME on desktop for password entry fields.

      default:
         break;
   }

   return true;
}

void QWindowsInputContext::reset()
{
   QPlatformInputContext::reset();

   if (!m_compositionContext.hwnd) {
      return;
   }

   if (m_compositionContext.isComposing && !m_compositionContext.focusObject.isNull()) {
      QInputMethodEvent event;
      if (!m_compositionContext.composition.isEmpty()) {
         event.setCommitString(m_compositionContext.composition);
      }
      QCoreApplication::sendEvent(m_compositionContext.focusObject, &event);
      endContextComposition();
   }
   imeNotifyCancelComposition(m_compositionContext.hwnd);
   doneContext();
}

void QWindowsInputContext::setFocusObject(QObject *)
{
   // ### fixme: On Windows 8.1, it has been observed that the Input context
   // remains active when this happens resulting in a lock-up. Consecutive
   // key events still have VK_PROCESSKEY set and are thus ignored.
   if (m_compositionContext.isComposing) {
      reset();
   }
   updateEnabled();
}

void QWindowsInputContext::updateEnabled()
{
   if (! QApplication::focusObject()) {
      return;
   }

   const QWindow *window = QApplication::focusWindow();

   if (window && window->handle()) {
      QWindowsWindow *platformWindow = QWindowsWindow::baseWindowOf(window);
      const bool accepted = inputMethodAccepted();

      QWindowsInputContext::setWindowsImeEnabled(platformWindow, accepted);
   }
}

void QWindowsInputContext::setWindowsImeEnabled(QWindowsWindow *platformWindow, bool enabled)
{
   if (! platformWindow || platformWindow->testFlag(QWindowsWindow::InputMethodDisabled) == !enabled) {
      return;
   }

   if (enabled) {
      // Re-enable Windows IME by associating default context saved on first disabling.
      ImmAssociateContext(platformWindow->handle(), QWindowsInputContext::m_defaultContext);
      platformWindow->clearFlag(QWindowsWindow::InputMethodDisabled);

   } else {
      // Disable Windows IME by associating 0 context. Store context first time.
      const HIMC oldImC = ImmAssociateContext(platformWindow->handle(), nullptr);
      platformWindow->setFlag(QWindowsWindow::InputMethodDisabled);

      if (! QWindowsInputContext::m_defaultContext && oldImC) {
         QWindowsInputContext::m_defaultContext = oldImC;
      }
   }
}

void QWindowsInputContext::update(Qt::InputMethodQueries queries)
{
   if (queries & Qt::ImEnabled) {
      updateEnabled();
   }

   QPlatformInputContext::update(queries);
}

void QWindowsInputContext::cursorRectChanged()
{
   if (!m_compositionContext.hwnd) {
      return;
   }
   const QInputMethod *inputMethod = QApplication::inputMethod();
   const QRectF cursorRectangleF = inputMethod->cursorRectangle();

   if (!cursorRectangleF.isValid()) {
      return;
   }

   const QRect cursorRectangle =
      QRectF(cursorRectangleF.topLeft() * m_compositionContext.factor,
         cursorRectangleF.size() * m_compositionContext.factor).toRect();

   const HIMC himc = ImmGetContext(m_compositionContext.hwnd);
   if (!himc) {
      return;
   }

   // Move candidate list window to the microfocus position.
   COMPOSITIONFORM cf;

   // ### need X-like inputStyle config settings
   cf.dwStyle = CFS_FORCE_POSITION;
   cf.ptCurrentPos.x = cursorRectangle.x();
   cf.ptCurrentPos.y = cursorRectangle.y();

   CANDIDATEFORM candf;
   candf.dwIndex = 0;
   candf.dwStyle = CFS_EXCLUDE;
   candf.ptCurrentPos.x = cursorRectangle.x();
   candf.ptCurrentPos.y = cursorRectangle.y() + cursorRectangle.height();
   candf.rcArea.left = cursorRectangle.x();
   candf.rcArea.top = cursorRectangle.y();
   candf.rcArea.right = cursorRectangle.x() + cursorRectangle.width();
   candf.rcArea.bottom = cursorRectangle.y() + cursorRectangle.height();

   if (m_compositionContext.haveCaret) {
      SetCaretPos(cursorRectangle.x(), cursorRectangle.y());
   }

   ImmSetCompositionWindow(himc, &cf);
   ImmSetCandidateWindow(himc, &candf);
   ImmReleaseContext(m_compositionContext.hwnd, himc);
}

void QWindowsInputContext::invokeAction(QInputMethod::Action action, int cursorPosition)
{
   if (action != QInputMethod::Click || ! m_compositionContext.hwnd) {
      QPlatformInputContext::invokeAction(action, cursorPosition);
      return;
   }

   if (cursorPosition < 0 || cursorPosition > m_compositionContext.composition.size()) {
      reset();
   }

   // Magic code that notifies Japanese IME about the cursor position.
   const HIMC himc = ImmGetContext(m_compositionContext.hwnd);
   const HWND imeWindow = ImmGetDefaultIMEWnd(m_compositionContext.hwnd);
   const WPARAM mouseOperationCode = MAKELONG(MAKEWORD(MK_LBUTTON, cursorPosition == 0 ? 2 : 1), cursorPosition);

   SendMessage(imeWindow, m_WM_MSIME_MOUSE, mouseOperationCode, LPARAM(himc));
   ImmReleaseContext(m_compositionContext.hwnd, himc);
}

static inline QString getCompositionString(HIMC himc, DWORD dwIndex)
{
   static constexpr const int bufferSize = 256;

   wchar_t buffer[bufferSize];

   const int length = ImmGetCompositionString(himc, dwIndex, buffer, bufferSize * sizeof(wchar_t));

   return QString::fromStdWString(std::wstring(buffer, size_t(length) / sizeof(wchar_t)));
}

// Determine the converted string range as pair of start/length to be selected.
static inline void getCompositionStringConvertedRange(HIMC himc, int *selStart, int *selLength)
{
   static constexpr const int bufferSize = 256;

   // Find the range of bytes with ATTR_TARGET_CONVERTED set.
   char attrBuffer[bufferSize];
   *selStart = *selLength = 0;

   if (const int attrLength = ImmGetCompositionString(himc, GCS_COMPATTR, attrBuffer, bufferSize)) {
      int start = 0;

      while (start < attrLength && !(attrBuffer[start] & ATTR_TARGET_CONVERTED)) {
         start++;
      }

      if (start < attrLength) {
         int end = start + 1;
         while (end < attrLength && (attrBuffer[end] & ATTR_TARGET_CONVERTED)) {
            end++;
         }
         *selStart = start;
         *selLength = end - start;
      }
   }
}

enum StandardFormat {
   PreeditFormat,
   SelectionFormat
};

static inline QTextFormat standardFormat(StandardFormat format)
{
   QTextCharFormat result;
   switch (format) {
      case PreeditFormat:
         result.setUnderlineStyle(QTextCharFormat::DashUnderline);
         break;

      case SelectionFormat: {
         // TODO: Should be that of the widget?
         const QPalette palette = QApplication::palette();
         const QColor background = palette.text().color();
         result.setBackground(QBrush(background));
         result.setForeground(palette.background());
         break;
      }
   }
   return result;
}

bool QWindowsInputContext::startComposition(HWND hwnd)
{
   QObject *fo = QApplication::focusObject();
   if (!fo) {
      return false;
   }

   // This should always match the object.
   QWindow *window = QApplication::focusWindow();

   if (!window) {
      return false;
   }

   if (! fo || QWindowsWindow::handleOf(window) != hwnd) {
      return false;
   }

   initContext(hwnd, QHighDpiScaling::factor(window), fo);
   startContextComposition();

   return true;
}

void QWindowsInputContext::startContextComposition()
{
   if (m_compositionContext.isComposing) {
      qWarning("QWindowsInputContext::startContextComposition() Called out of sequence");
      return;
   }
   m_compositionContext.isComposing = true;
   m_compositionContext.composition.clear();
   m_compositionContext.position = 0;
   cursorRectChanged(); // position cursor initially.
   update(Qt::ImQueryAll);
}

void QWindowsInputContext::endContextComposition()
{
   if (!m_compositionContext.isComposing) {
      qWarning("WindowsInputContext::endContextComposition() Called out of sequence");
      return;
   }

   m_compositionContext.composition.clear();
   m_compositionContext.position = 0;
   m_compositionContext.isComposing = false;
}

// Create a list of markup attributes for QInputMethodEvent
// to display the selected part of the intermediate composition
// result differently.
static inline QList<QInputMethodEvent::Attribute> intermediateMarkup(int position, int compositionLength,
   int selStart, int selLength)
{
   QList<QInputMethodEvent::Attribute> attributes;
   if (selStart > 0)
      attributes << QInputMethodEvent::Attribute(QInputMethodEvent::TextFormat, 0, selStart,
            standardFormat(PreeditFormat));
   if (selLength)
      attributes << QInputMethodEvent::Attribute(QInputMethodEvent::TextFormat, selStart, selLength,
            standardFormat(SelectionFormat));
   if (selStart + selLength < compositionLength)
      attributes << QInputMethodEvent::Attribute(QInputMethodEvent::TextFormat, selStart + selLength,
            compositionLength - selStart - selLength,
            standardFormat(PreeditFormat));
   if (position >= 0) {
      attributes << QInputMethodEvent::Attribute(QInputMethodEvent::Cursor, position, selLength ? 0 : 1, QVariant());
   }
   return attributes;
}

bool QWindowsInputContext::composition(HWND hwnd, LPARAM lParamIn)
{
   const int lParam = int(lParamIn);

   if (m_compositionContext.focusObject.isNull() || m_compositionContext.hwnd != hwnd || !lParam) {
      return false;
   }

   const HIMC himc = ImmGetContext(m_compositionContext.hwnd);

   if (! himc) {
      return false;
   }

   QScopedPointer<QInputMethodEvent> event;
   if (lParam & (GCS_COMPSTR | GCS_COMPATTR | GCS_CURSORPOS)) {
      if (! m_compositionContext.isComposing) {
         startContextComposition();
      }

      // Some intermediate composition result. Parametrize event with
      // attribute sequence specifying the formatting of the converted part.

      int selStart, selLength;
      m_compositionContext.composition = getCompositionString(himc, GCS_COMPSTR);
      m_compositionContext.position = ImmGetCompositionString(himc, GCS_CURSORPOS, nullptr, 0);
      getCompositionStringConvertedRange(himc, &selStart, &selLength);

      if ((lParam & CS_INSERTCHAR) && (lParam & CS_NOMOVECARET)) {
         // make Korean work correctly. Hope this is correct for all IMEs
         selStart = 0;
         selLength = m_compositionContext.composition.size();
      }

      if (!selLength) {
         selStart = 0;
      }

      event.reset(new QInputMethodEvent(m_compositionContext.composition,
            intermediateMarkup(m_compositionContext.position,
               m_compositionContext.composition.size(),
               selStart, selLength)));
   }
   if (event.isNull()) {
      event.reset(new QInputMethodEvent);
   }

   if (lParam & GCS_RESULTSTR) {
      // A fixed result, return the converted string
      event->setCommitString(getCompositionString(himc, GCS_RESULTSTR));
      if (!(lParam & GCS_DELTASTART)) {
         endContextComposition();
      }
   }

   const bool result = QCoreApplication::sendEvent(m_compositionContext.focusObject, event.data());

   update(Qt::ImQueryAll);
   ImmReleaseContext(m_compositionContext.hwnd, himc);

   return result;
}

bool QWindowsInputContext::endComposition(HWND hwnd)
{
   // Googles Pinyin Input Method likes to call endComposition again
   // when we call notifyIME with CPS_CANCEL, so protect ourselves against that.
   if (m_endCompositionRecursionGuard || m_compositionContext.hwnd != hwnd) {
      return false;
   }

   if (m_compositionContext.focusObject.isNull()) {
      return false;
   }

   m_endCompositionRecursionGuard = true;

   imeNotifyCancelComposition(m_compositionContext.hwnd);
   if (m_compositionContext.isComposing) {
      QInputMethodEvent event;
      QCoreApplication::sendEvent(m_compositionContext.focusObject, &event);
   }
   doneContext();

   m_endCompositionRecursionGuard = false;
   return true;
}

void QWindowsInputContext::initContext(HWND hwnd, qreal factor, QObject *focusObject)
{
   if (m_compositionContext.hwnd) {
      doneContext();
   }

   m_compositionContext.hwnd = hwnd;
   m_compositionContext.focusObject = focusObject;
   m_compositionContext.factor = factor;

   // Create a hidden caret which is kept at the microfocus
   // position in update(). This is important for some
   // Chinese input methods.

   m_compositionContext.haveCaret = CreateCaret(hwnd, nullptr, 1, 1);
   HideCaret(hwnd);
   update(Qt::ImQueryAll);
   m_compositionContext.isComposing = false;
   m_compositionContext.position = 0;
}

void QWindowsInputContext::doneContext()
{
   if (!m_compositionContext.hwnd) {
      return;
   }

   if (m_compositionContext.haveCaret) {
      DestroyCaret();
   }

   m_compositionContext.hwnd = nullptr;
   m_compositionContext.composition.clear();
   m_compositionContext.position    = 0;
   m_compositionContext.isComposing = m_compositionContext.haveCaret = false;
   m_compositionContext.focusObject = nullptr;
}

bool QWindowsInputContext::handleIME_Request(WPARAM wParam,
   LPARAM lParam,
   LRESULT *result)
{
   switch (int(wParam)) {
      case IMR_RECONVERTSTRING: {
         const int size = reconvertString(reinterpret_cast<RECONVERTSTRING *>(lParam));
         if (size < 0) {
            return false;
         }
         *result = size;
      }
      return true;

      case IMR_CONFIRMRECONVERTSTRING:
         return true;

      default:
         break;
   }
   return false;
}

void QWindowsInputContext::handleInputLanguageChanged(WPARAM wparam, LPARAM lparam)
{
   const LCID newLanguageId = languageIdFromLocaleId(WORD(lparam));

   if (newLanguageId == m_languageId) {
      return;
   }

   const LCID oldLanguageId = m_languageId;
   m_languageId = newLanguageId;
   m_locale = qt_localeFromLCID(m_languageId);
   emitLocaleChanged();

#if defined(CS_SHOW_DEBUG_PLATFORM)
   qDebug() << "QWindowsInputContext::handleInputLanguageChanged() Old set = " << hex << showbase
      << oldLanguageId << "->" << newLanguageId
         << "Character set =" << DWORD(wparam) << dec << noshowbase << m_locale.name() ;
#else
   (void) wparam;
   (void) oldLanguageId;
#endif

}

int QWindowsInputContext::reconvertString(RECONVERTSTRING *reconv)
{
   QObject *fo = QApplication::focusObject();
   if (!fo) {
      return false;
   }

   const QVariant surroundingTextV = QInputMethod::queryFocusObject(Qt::ImSurroundingText, QVariant());
   if (!surroundingTextV.isValid()) {
      return -1;
   }

   const QString surroundingText = surroundingTextV.toString();
   const int memSize = int(sizeof(RECONVERTSTRING))
      + (surroundingText.length() + 1) * int(sizeof(ushort));

   // If memory is not allocated, return the required size.
   if (!reconv) {
      return surroundingText.isEmpty() ? -1 : memSize;
   }

   const QVariant posV = QInputMethod::queryFocusObject(Qt::ImCursorPosition, QVariant());
   const int pos = posV.isValid() ? posV.toInt() : 0;

   // Find the word in the surrounding text.
   QTextBoundaryFinder bounds(QTextBoundaryFinder::Word, surroundingText);
   bounds.setPosition(pos);

   if (bounds.position() > 0 && !(bounds.boundaryReasons() & QTextBoundaryFinder::StartOfItem)) {
      bounds.toPreviousBoundary();
   }

   const int startPos = bounds.position();
   bounds.toNextBoundary();

   const int endPos = bounds.position();

   // Select the text, this will be overwritten by following IME events.
   QList<QInputMethodEvent::Attribute> attributes;
   attributes << QInputMethodEvent::Attribute(QInputMethodEvent::Selection, startPos, endPos - startPos, QVariant());

   QInputMethodEvent selectEvent(QString(), attributes);
   QCoreApplication::sendEvent(fo, &selectEvent);

   reconv->dwSize            = DWORD(memSize);
   reconv->dwVersion         = 0;
   reconv->dwStrLen          = DWORD(surroundingText.size());
   reconv->dwStrOffset       = sizeof(RECONVERTSTRING);
   reconv->dwCompStrLen      = DWORD(endPos - startPos);             // TCHAR count.
   reconv->dwCompStrOffset   = DWORD(startPos) * sizeof(ushort);     // byte count.
   reconv->dwTargetStrLen    = reconv->dwCompStrLen;
   reconv->dwTargetStrOffset = reconv->dwCompStrOffset;

   ushort *pastReconv = reinterpret_cast<ushort *>(reconv + 1);

   std::wstring tmp = surroundingText.toStdWString();
   std::copy(tmp.begin(), tmp.end(), pastReconv);

   return memSize;
}

