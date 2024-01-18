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

#ifndef QXCB_CLIPBOARD_H
#define QXCB_CLIPBOARD_H

#include <qplatform_clipboard.h>
#include <qxcb_object.h>

#include <xcb/xcb.h>
#include <xcb/xfixes.h>

#ifndef QT_NO_CLIPBOARD

class QXcbConnection;
class QXcbScreen;
class QXcbClipboardMime;

class QXcbClipboard : public QXcbObject, public QPlatformClipboard
{
 public:
   QXcbClipboard(QXcbConnection *connection);
   ~QXcbClipboard();

   QMimeData *mimeData(QClipboard::Mode mode) override;
   void setMimeData(QMimeData *data, QClipboard::Mode mode) override;

   bool supportsMode(QClipboard::Mode mode) const override;
   bool ownsMode(QClipboard::Mode mode) const override;

   QXcbScreen *screen() const;

   xcb_window_t requestor() const;
   void setRequestor(xcb_window_t window);

   xcb_window_t owner() const;

   void handleSelectionRequest(xcb_selection_request_event_t *event);
   void handleSelectionClearRequest(xcb_selection_clear_event_t *event);
   void handleXFixesSelectionRequest(xcb_xfixes_selection_notify_event_t *event);

   bool clipboardReadProperty(xcb_window_t win, xcb_atom_t property, bool deleteProperty, QByteArray *buffer, int *size, xcb_atom_t *type,
      int *format);
   QByteArray clipboardReadIncrementalProperty(xcb_window_t win, xcb_atom_t property, int nbytes, bool nullterm);

   QByteArray getDataInFormat(xcb_atom_t modeAtom, xcb_atom_t fmtatom);

   void setProcessIncr(bool process) {
      m_incr_active = process;
   }
   bool processIncr() {
      return m_incr_active;
   }
   void incrTransactionPeeker(xcb_generic_event_t *ge, bool &accepted);

   xcb_window_t getSelectionOwner(xcb_atom_t atom) const;
   QByteArray getSelection(xcb_atom_t selection, xcb_atom_t target, xcb_atom_t property, xcb_timestamp_t t = 0);

 private:
   xcb_generic_event_t *waitForClipboardEvent(xcb_window_t win, int type, int timeout, bool checkManager = false);

   xcb_atom_t sendTargetsSelection(QMimeData *d, xcb_window_t window, xcb_atom_t property);
   xcb_atom_t sendSelection(QMimeData *d, xcb_atom_t target, xcb_window_t window, xcb_atom_t property);

   xcb_atom_t atomForMode(QClipboard::Mode mode) const;
   QClipboard::Mode modeForAtom(xcb_atom_t atom) const;

   // Selection and Clipboard
   QScopedPointer<QXcbClipboardMime> m_xClipboard[2];
   QMimeData *m_clientClipboard[2];
   xcb_timestamp_t m_timestamp[2];

   xcb_window_t m_requestor;
   xcb_window_t m_owner;

   static const int clipboard_timeout;

   bool m_incr_active;
   bool m_clipboard_closing;
   xcb_timestamp_t m_incr_receive_time;
};

#endif // QT_NO_CLIPBOARD

#endif // QXCBCLIPBOARD_H
