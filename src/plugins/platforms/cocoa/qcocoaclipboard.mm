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

#include <qcocoaclipboard.h>
#include <qapplication.h>

QCocoaClipboard::QCocoaClipboard()
   : m_clipboard(new QMacPasteboard(kPasteboardClipboard, QMacInternalPasteboardMime::MIME_CLIP)),
     m_find(new QMacPasteboard(kPasteboardFind, QMacInternalPasteboardMime::MIME_CLIP))
{
   connect(qApp, &QApplication::applicationStateChanged, this, &QCocoaClipboard::handleApplicationStateChanged);
}

QMimeData *QCocoaClipboard::mimeData(QClipboard::Mode mode)
{
   if (QMacPasteboard *pasteBoard = pasteboardForMode(mode)) {
      pasteBoard->sync();
      return pasteBoard->mimeData();
   }

   return nullptr;
}

void QCocoaClipboard::setMimeData(QMimeData *data, QClipboard::Mode mode)
{
   if (QMacPasteboard *pasteBoard = pasteboardForMode(mode)) {

      if (data == nullptr) {
         pasteBoard->clear();
      }

      pasteBoard->sync();
      pasteBoard->setMimeData(data);
      emitChanged(mode);
   }
}

bool QCocoaClipboard::supportsMode(QClipboard::Mode mode) const
{
   return (mode == QClipboard::Clipboard || mode == QClipboard::FindBuffer);
}

bool QCocoaClipboard::ownsMode(QClipboard::Mode mode) const
{
   return false;
}

QMacPasteboard *QCocoaClipboard::pasteboardForMode(QClipboard::Mode mode) const
{
   if (mode == QClipboard::Clipboard) {
      return m_clipboard.data();
   } else if (mode == QClipboard::FindBuffer) {
      return m_find.data();
   } else {
      return nullptr;
   }
}

void QCocoaClipboard::handleApplicationStateChanged(Qt::ApplicationState state)
{
   if (state != Qt::ApplicationActive) {
      return;
   }

   if (m_clipboard->sync()) {
      emitChanged(QClipboard::Clipboard);
   }

   if (m_find->sync()) {
      emitChanged(QClipboard::FindBuffer);
   }
}


