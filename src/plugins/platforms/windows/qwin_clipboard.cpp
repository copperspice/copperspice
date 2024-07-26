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

#include <qwin_clipboard.h>
#include <qwin_context.h>
#include <qwin_ole.h>
#include <qwin_mime.h>

#include <qapplication.h>
#include <qclipboard.h>
#include <qcolor.h>
#include <qimage.h>
#include <qmimedata.h>
#include <qstringlist.h>
#include <qurl.h>
#include <qvariant.h>

#include <qwin_gui_eventdispatcher_p.h>

#if defined(CS_SHOW_DEBUG_PLATFORM)
static QDebug operator<<(QDebug debug, const QMimeData *mimeData)
{
   QDebugStateSaver saver(debug);

   debug.nospace();
   debug << "QMimeData(";

   if (mimeData) {
      const QStringList formats = mimeData->formats();

      debug << "formats =" << formats.join(QString(", "));
      if (mimeData->hasText()) {
         debug << ", text =" << mimeData->text();
      }
      if (mimeData->hasHtml()) {
         debug << ", html =" << mimeData->html();
      }
      if (mimeData->hasColor()) {
         debug << ", colorData =" << (mimeData->colorData()).value<QColor>();
      }
      if (mimeData->hasImage()) {
         debug << ", imageData =" << (mimeData->imageData()).value<QImage>();
      }
      if (mimeData->hasUrls()) {
         debug << ", urls =" << mimeData->urls();
      }

   } else {
      debug << '0';
   }

   debug << ')';

   return debug;
}
#endif

IDataObject *QWindowsClipboardRetrievalMimeData::retrieveDataObject() const
{
   IDataObject *pDataObj = nullptr;

   if (OleGetClipboard(&pDataObj) == S_OK) {
      return pDataObj;
   }

   return nullptr;
}

void QWindowsClipboardRetrievalMimeData::releaseDataObject(IDataObject *dataObject) const
{
   dataObject->Release();
}

extern "C" LRESULT QT_WIN_CALLBACK qClipboardViewerWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
   LRESULT result = 0;
   if (QWindowsClipboard::instance()
         && QWindowsClipboard::instance()->clipboardViewerWndProc(hwnd, message, wParam, lParam, &result)) {
      return result;
   }

   return DefWindowProc(hwnd, message, wParam, lParam);
}

// ensure the clipboard is flushed before QApplication is destroyed since OleFlushClipboard()
// might query the data again which causes problems for QMimeData-derived classes using QPixmap/QImage
static void cleanClipboardPostRoutine()
{
   if (QWindowsClipboard *cl = QWindowsClipboard::instance()) {
      cl->cleanup();
   }
}

QWindowsClipboard *QWindowsClipboard::m_instance = nullptr;

QWindowsClipboard::QWindowsClipboard()
   : m_data(nullptr), m_clipboardViewer(nullptr), m_nextClipboardViewer(nullptr), m_formatListenerRegistered(false)
{
   QWindowsClipboard::m_instance = this;
   qAddPostRoutine(cleanClipboardPostRoutine);
}

QWindowsClipboard::~QWindowsClipboard()
{
   cleanup();
   QWindowsClipboard::m_instance = nullptr;
}

void QWindowsClipboard::cleanup()
{
   unregisterViewer(); // Should release data if owner
   releaseIData();
}

void QWindowsClipboard::releaseIData()
{
   if (m_data) {
      delete m_data->mimeData();

      m_data->releaseData();
      m_data->Release();
      m_data = nullptr;
   }
}

void QWindowsClipboard::registerViewer()
{
   m_clipboardViewer = QWindowsContext::instance()->
      createDummyWindow(QString("CsClipboardView"), L"CsClipboardView", qClipboardViewerWndProc, WS_OVERLAPPED);

   // Try format listener API (Vista onwards) first.
   if (QWindowsContext::user32dll.addClipboardFormatListener && QWindowsContext::user32dll.removeClipboardFormatListener) {
      m_formatListenerRegistered = QWindowsContext::user32dll.addClipboardFormatListener(m_clipboardViewer);

      if (! m_formatListenerRegistered) {
         qErrnoWarning("AddClipboardFormatListener() failed.");
      }
   }

   if (! m_formatListenerRegistered) {
      m_nextClipboardViewer = SetClipboardViewer(m_clipboardViewer);
   }

#if defined(CS_SHOW_DEBUG_PLATFORM)
   qDebug() << "QWindowsClipboard::registerViewer() "
            << "Format listener =" << m_formatListenerRegistered << " Next =" << m_nextClipboardViewer;
#endif

}

void QWindowsClipboard::unregisterViewer()
{
   if (m_clipboardViewer) {
      if (m_formatListenerRegistered) {
         QWindowsContext::user32dll.removeClipboardFormatListener(m_clipboardViewer);
         m_formatListenerRegistered = false;
      } else {
         ChangeClipboardChain(m_clipboardViewer, m_nextClipboardViewer);
         m_nextClipboardViewer = nullptr;
      }
      DestroyWindow(m_clipboardViewer);
      m_clipboardViewer = nullptr;
   }
}

// TODO: Remove the clipboard chain handling code and make the format listener the default

static bool isProcessBeingDebugged(HWND hwnd)
{
   DWORD pid = 0;
   if (!GetWindowThreadProcessId(hwnd, &pid) || !pid) {
      return false;
   }

   const HANDLE processHandle = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pid);
   if (!processHandle) {
      return false;
   }

   BOOL debugged = FALSE;
   CheckRemoteDebuggerPresent(processHandle, &debugged);
   CloseHandle(processHandle);

   return debugged != FALSE;
}

void QWindowsClipboard::propagateClipboardMessage(UINT message, WPARAM wParam, LPARAM lParam) const
{
   if (!m_nextClipboardViewer) {
      return;
   }

   if (QWindowsContext::user32dll.isHungAppWindow
         && QWindowsContext::user32dll.isHungAppWindow(m_nextClipboardViewer)) {
      qWarning("QWindowsClipboard::propagateClipboardMessage() Unable to send clipboard message to application");
      return;
   }

   // Do not block if the process is being debugged, specifically, if it is
   // displaying a runtime assert, which is not caught by isHungAppWindow().
   if (isProcessBeingDebugged(m_nextClipboardViewer)) {
      PostMessage(m_nextClipboardViewer, message, wParam, lParam);
   } else {
      SendMessage(m_nextClipboardViewer, message, wParam, lParam);
   }
}

bool QWindowsClipboard::clipboardViewerWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam, LRESULT *result)
{
   static constexpr const int wMClipboardUpdate = 0x031D;

   *result = 0;

#if defined(CS_SHOW_DEBUG_PLATFORM)
   qDebug() << "QWindowsClipboard::clipboardViewerWndProc() handle =" << hwnd
         << " message =" << message << QWindowsGuiEventDispatcher::windowsMessageName(message);
#endif

   switch (message) {
      case WM_CHANGECBCHAIN: {
         const HWND toBeRemoved = reinterpret_cast<HWND>(wParam);

         if (toBeRemoved == m_nextClipboardViewer) {
            m_nextClipboardViewer = reinterpret_cast<HWND>(lParam);
         } else {
            propagateClipboardMessage(message, wParam, lParam);
         }
      }
      return true;

      case wMClipboardUpdate:     // Clipboard Format listener (Vista onwards)
      case WM_DRAWCLIPBOARD: {
         // Clipboard Viewer Chain handling (up to XP)
         const bool owned = ownsClipboard();
         emitChanged(QClipboard::Clipboard);

         // clean up the clipboard object if we no longer own the clipboard
         if (!owned && m_data) {
            releaseIData();
         }
         if (!m_formatListenerRegistered) {
            propagateClipboardMessage(message, wParam, lParam);
         }
      }
      return true;
      case WM_DESTROY:
         // Recommended shutdown
         if (ownsClipboard()) {
            OleFlushClipboard();
            releaseIData();
         }
         return true;

   }

   return false;
}

QMimeData *QWindowsClipboard::mimeData(QClipboard::Mode mode)
{
#if defined(CS_SHOW_DEBUG_PLATFORM)
   qDebug() << "QWindowsClipboard::mimeData: mode =" <<  mode;
#endif

   if (mode != QClipboard::Clipboard) {
      return nullptr;
   }

   if (ownsClipboard()) {
      return m_data->mimeData();
   }

   return &m_retrievalData;
}

void QWindowsClipboard::setMimeData(QMimeData *mimeData, QClipboard::Mode mode)
{
#if defined(CS_SHOW_DEBUG_PLATFORM)
   qDebug() << "QWindowsClipboard::setMimeData: mode =" <<  mode << mimeData;
#endif

   if (mode != QClipboard::Clipboard) {
      return;
   }

   const bool newData = !m_data || m_data->mimeData() != mimeData;
   if (newData) {
      releaseIData();
      if (mimeData) {
         m_data = new QWindowsOleDataObject(mimeData);
      }
   }

   const HRESULT src = OleSetClipboard(m_data);

   if (src != S_OK) {
      QString mimeDataFormats = mimeData ? mimeData->formats().join(", ") : "NULL";

      qErrnoWarning("OleSetClipboard: Failed to set mime data (%s) on clipboard: %s",
         csPrintable(mimeDataFormats), QWindowsContext::comErrorString(src).constData());

      releaseIData();
      return;
   }
}

void QWindowsClipboard::clear()
{
   const HRESULT src = OleSetClipboard(nullptr);

   if (src != S_OK) {
      qErrnoWarning("OleSetClipboard: Failed to clear the clipboard: 0x%lx", src);
   }
}

bool QWindowsClipboard::supportsMode(QClipboard::Mode mode) const
{
   return mode == QClipboard::Clipboard;
}

// Need a non-virtual in destructor.
bool QWindowsClipboard::ownsClipboard() const
{
   return m_data && OleIsCurrentClipboard(m_data) == S_OK;
}

bool QWindowsClipboard::ownsMode(QClipboard::Mode mode) const
{
   const bool result = mode == QClipboard::Clipboard ? ownsClipboard() : false;

#if defined(CS_SHOW_DEBUG_PLATFORM)
   qDebug() << "QWindowsClipboard::ownsMode: mode =" <<  mode << result;
#endif

   return result;
}

