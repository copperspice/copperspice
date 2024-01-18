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

#ifndef QWINDOWSCLIPBOARD_H
#define QWINDOWSCLIPBOARD_H

#include <qwin_internal_mimedata.h>
#include <qplatform_clipboard.h>

class QWindowsOleDataObject;

class QWindowsClipboardRetrievalMimeData : public QWindowsInternalMimeData
{
 protected:
   IDataObject *retrieveDataObject() const override;
   void releaseDataObject(IDataObject *) const override;
};

class QWindowsClipboard : public QPlatformClipboard
{
 public:
   QWindowsClipboard();
   ~QWindowsClipboard();

   void registerViewer();    // Call in initialization, when context is up.
   void cleanup();

   QMimeData *mimeData(QClipboard::Mode mode = QClipboard::Clipboard) override;
   void setMimeData(QMimeData *data, QClipboard::Mode mode = QClipboard::Clipboard) override;
   bool supportsMode(QClipboard::Mode mode) const override;
   bool ownsMode(QClipboard::Mode mode) const override;

   inline bool clipboardViewerWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam, LRESULT *result);

   static QWindowsClipboard *instance() {
      return m_instance;
   }

 private:
   void clear();
   void releaseIData();

   inline void propagateClipboardMessage(UINT message, WPARAM wParam, LPARAM lParam) const;
   inline void unregisterViewer();
   inline bool ownsClipboard() const;

   static QWindowsClipboard *m_instance;

   QWindowsClipboardRetrievalMimeData m_retrievalData;
   QWindowsOleDataObject *m_data;
   HWND m_clipboardViewer;
   HWND m_nextClipboardViewer;
   bool m_formatListenerRegistered;
};

#endif
