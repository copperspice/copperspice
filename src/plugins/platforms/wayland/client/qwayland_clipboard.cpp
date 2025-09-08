/***********************************************************************
*
* Copyright (c) 2012-2025 Barbara Geller
* Copyright (c) 2012-2025 Ansel Sermersheim
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

#include <qwayland_clipboard_p.h>

#include <qwayland_data_device_p.h>
#include <qwayland_data_source_p.h>
#include <qwayland_dataoffer_p.h>
#include <qwayland_display_p.h>
#include <qwayland_inputdevice_p.h>

#ifndef QT_NO_DRAGANDDROP

namespace QtWaylandClient {

QWaylandClipboard::QWaylandClipboard(QWaylandDisplay *display)
   : m_display(display)
{
}

QWaylandClipboard::~QWaylandClipboard()
{
}

QMimeData *QWaylandClipboard::mimeData(QClipboard::Mode mode)
{
   if (mode != QClipboard::Clipboard) {
      return &m_emptyData;
   }

   QWaylandInputDevice *inputDevice = m_display->currentInputDevice();

   if (inputDevice == nullptr || inputDevice->dataDevice() == nullptr) {
      return &m_emptyData;
   }

   QWaylandDataSource *source = inputDevice->dataDevice()->selectionSource();

   if (source != nullptr) {
      return source->mimeData();
   }

   if (inputDevice->dataDevice()->selectionOffer()) {
      return inputDevice->dataDevice()->selectionOffer()->mimeData();
   }

   return &m_emptyData;
}

void QWaylandClipboard::setMimeData(QMimeData *data, QClipboard::Mode mode)
{
   if (mode != QClipboard::Clipboard) {
      return;
   }

   QWaylandInputDevice *inputDevice = m_display->currentInputDevice();

   if (inputDevice == nullptr|| inputDevice->dataDevice() == nullptr) {
      return;
   }

   static const QString plain = "text/plain";
   static const QString utf8  = "text/plain;charset=utf-8";

   if (data != nullptr && data->hasFormat(plain) && ! data->hasFormat(utf8)) {
      data->setData(utf8, data->data(plain));
   }

   inputDevice->dataDevice()->setSelectionSource(data ? new QWaylandDataSource(m_display->dndSelectionHandler(), data) : nullptr);

   emitChanged(mode);
}

bool QWaylandClipboard::supportsMode(QClipboard::Mode mode) const
{
   return mode == QClipboard::Clipboard;
}

bool QWaylandClipboard::ownsMode(QClipboard::Mode mode) const
{
   if (mode != QClipboard::Clipboard) {
      return false;
   }

   QWaylandInputDevice *inputDevice = m_display->currentInputDevice();

   if (inputDevice == nullptr || inputDevice->dataDevice() == nullptr) {
      return false;
   }

   return inputDevice->dataDevice()->selectionSource() != nullptr;
}

}

#endif // QT_NO_DRAGANDDROP
