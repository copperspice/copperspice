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

#ifndef QCOCOACLIPBOARD_H
#define QCOCOACLIPBOARD_H

#include <qplatform_clipboard.h>

#include <qmacclipboard.h>
#include <qscopedpointer.h>

class QCocoaClipboard : public QObject, public QPlatformClipboard
{
   CS_OBJECT_MULTIPLE(QCocoaClipboard, QObject)

 public:
   QCocoaClipboard();

   QMimeData *mimeData(QClipboard::Mode mode = QClipboard::Clipboard) override;
   void setMimeData(QMimeData *data, QClipboard::Mode mode = QClipboard::Clipboard) override;
   bool supportsMode(QClipboard::Mode mode) const override;
   bool ownsMode(QClipboard::Mode mode) const override;

 protected:
   QMacPasteboard *pasteboardForMode(QClipboard::Mode mode) const;

 private:
   QScopedPointer<QMacPasteboard> m_clipboard;
   QScopedPointer<QMacPasteboard> m_find;

   CS_SLOT_1(Private, void handleApplicationStateChanged(Qt::ApplicationState state))
   CS_SLOT_2(handleApplicationStateChanged)
};

#endif
