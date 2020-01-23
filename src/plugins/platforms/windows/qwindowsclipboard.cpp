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

#include <qwindowsclipboard.h>
#include <qwindowscontext.h>
#include <qwindowsole.h>
#include <qwindowsmime.h>

#include <QApplication>
#include <QClipboard>
#include <QColor>
#include <QImage>
#include <QDebug>
#include <QMimeData>
#include <QStringList>
#include <QVariant>
#include <QUrl>

#include <qwindowsguieventdispatcher_p.h>

static QDebug operator<<(QDebug d, const QMimeData *mimeData)
{
   QDebugStateSaver saver(d);

   d.nospace();
   d << "QMimeData(";

   if (mimeData) {
      const QStringList formats = mimeData->formats();

      d << "formats =" << formats.join(QString(", "));
      if (mimeData->hasText()) {
         d << ", text =" << mimeData->text();
      }
      if (mimeData->hasHtml()) {
         d << ", html =" << mimeData->html();
      }
      if (mimeData->hasColor()) {
         d << ", colorData =" << qvariant_cast<QColor>(mimeData->colorData());
      }
      if (mimeData->hasImage()) {
         d << ", imageData =" << qvariant_cast<QImage>(mimeData->imageData());
      }
      if (mimeData->hasUrls()) {
         d << ", urls =" << mimeData->urls();
      }

   } else {
      d << '0';
   }

   d << ')';

   return d;
}

IDataObject *QWindowsClipboardRetrievalMimeData::retrieveDataObject() const
{
   IDataObject *pDataObj = 0;

   if (OleGetClipboard(&pDataObj) == S_OK) {

      if (QWindowsContext::verbose > 1) {
         qDebug() << "retrieveDataObject():" << pDataObj;
      }

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

QWindowsClipboard *QWindowsClipboard::m_instance = 0;

QWindowsClipboard::QWindowsClipboard()
   : m_data(0), m_clipboardViewer(0), m_nextClipboardViewer(0), m_formatListenerRegistered(false)
{
   QWindowsClipboard::m_instance = this;
   qAddPostRoutine(cleanClipboardPostRoutine);
}

QWindowsClipboard::~QWindowsClipboard()
{
   cleanup();
   QWindowsClipboard::m_instance = 0;
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
      m_data->releaseQt();
      m_data->Release();
      m_data = 0;
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

   if (!m_formatListenerRegistered) {
      m_nextClipboardViewer = SetClipboardViewer(m_clipboardViewer);
   }

#if defined(CS_SHOW_DEBUG)
   qDebug() << "QWindowsClipboard::registerViewer():"
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
         m_nextClipboardViewer = 0;
      }
      DestroyWindow(m_clipboardViewer);
      m_clipboardViewer = 0;
   }
}

// ### FIXME: Qt6: Remove the clipboard chain handling code and make the format listener the default

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

   // In rare cases, a clipboard viewer can hang (application crashed,
   // suspended by a shell prompt 'Select' or debugger).
   if (QWindowsContext::user32dll.isHungAppWindow
         && QWindowsContext::user32dll.isHungAppWindow(m_nextClipboardViewer)) {
      qWarning("Cowardly refusing to send clipboard message to hung application...");
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
   enum { wMClipboardUpdate = 0x031D };

   *result = 0;

   if (QWindowsContext::verbose) {
      qDebug() << "QWindowsClipboard::clipboardViewerWndProc:" << hwnd << message
               << QWindowsGuiEventDispatcher::windowsMessageName(message);
   }

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
#if defined(CS_SHOW_DEBUG)
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
#if defined(CS_SHOW_DEBUG)
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
      QString mimeDataFormats = mimeData ? mimeData->formats().join(QString(", ")) : QString("NULL");

      qErrnoWarning("OleSetClipboard: Failed to set mime data (%s) on clipboard: %s",
         qPrintable(mimeDataFormats), QWindowsContext::comErrorString(src).constData());

      releaseIData();
      return;
   }
}

void QWindowsClipboard::clear()
{
   const HRESULT src = OleSetClipboard(0);

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

#if defined(CS_SHOW_DEBUG)
   qDebug() << "QWindowsClipboard::ownsMode: mode =" <<  mode << result;
#endif

   return result;
}

