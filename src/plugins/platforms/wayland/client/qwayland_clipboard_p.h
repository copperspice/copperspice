/***********************************************************************
*
* Copyright (c) 2012-2026 Barbara Geller
* Copyright (c) 2012-2026 Ansel Sermersheim
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

#ifndef QWAYLAND_CLIPBOARD_H
#define QWAYLAND_CLIPBOARD_H

#include <qmimedata.h>
#include <qplatform_clipboard.h>
#include <qvariant.h>

#ifndef QT_NO_DRAGANDDROP

namespace QtWaylandClient {

class QWaylandDisplay;

class Q_WAYLAND_CLIENT_EXPORT QWaylandClipboard : public QPlatformClipboard
{
 public:
   QWaylandClipboard(QWaylandDisplay *display);

   ~QWaylandClipboard();

   QMimeData *mimeData(QClipboard::Mode mode = QClipboard::Clipboard) override;
   void setMimeData(QMimeData *data, QClipboard::Mode mode = QClipboard::Clipboard) override;
   bool supportsMode(QClipboard::Mode mode) const override;
   bool ownsMode(QClipboard::Mode mode) const override;

 private:
   QWaylandDisplay *m_display;
   QMimeData m_emptyData;
};

}

#endif // QT_NO_DRAGANDDROP

#endif
