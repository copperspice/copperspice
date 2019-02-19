/***********************************************************************
*
* Copyright (c) 2012-2018 Barbara Geller
* Copyright (c) 2012-2018 Ansel Sermersheim
* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).
* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).
* All rights reserved.
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
* <http://www.gnu.org/licenses/>.
*
***********************************************************************/

#include "qcocoaclipboard.h"

QT_BEGIN_NAMESPACE

QCocoaClipboard::QCocoaClipboard()
    :m_clipboard(new QMacPasteboard(kPasteboardClipboard, QMacInternalPasteboardMime::MIME_CLIP))
    ,m_find(new QMacPasteboard(kPasteboardFind, QMacInternalPasteboardMime::MIME_CLIP))
{
    connect(qGuiApp, &QGuiApplication::applicationStateChanged, this, &QCocoaClipboard::handleApplicationStateChanged);
}

QMimeData *QCocoaClipboard::mimeData(QClipboard::Mode mode)
{
    if (QMacPasteboard *pasteBoard = pasteboardForMode(mode)) {
        pasteBoard->sync();
        return pasteBoard->mimeData();
    }
    return 0;
}

void QCocoaClipboard::setMimeData(QMimeData *data, QClipboard::Mode mode)
{
    if (QMacPasteboard *pasteBoard = pasteboardForMode(mode)) {
        if (data == 0) {
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
    Q_UNUSED(mode);
    return false;
}

QMacPasteboard *QCocoaClipboard::pasteboardForMode(QClipboard::Mode mode) const
{
    if (mode == QClipboard::Clipboard)
        return m_clipboard.data();
    else if (mode == QClipboard::FindBuffer)
        return m_find.data();
    else
        return 0;
}

void QCocoaClipboard::handleApplicationStateChanged(Qt::ApplicationState state)
{
    if (state != Qt::ApplicationActive)
        return;

    if (m_clipboard->sync())
        emitChanged(QClipboard::Clipboard);
    if (m_find->sync())
        emitChanged(QClipboard::FindBuffer);
}

#include "moc_qcocoaclipboard.cpp"

QT_END_NAMESPACE
