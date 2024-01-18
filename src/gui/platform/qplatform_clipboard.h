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

#ifndef QPLATFORM_CLIPBOARD_H
#define QPLATFORM_CLIPBOARD_H

#include <qglobal.h>

#ifndef QT_NO_CLIPBOARD

#include <qclipboard.h>

class Q_GUI_EXPORT QPlatformClipboard
{
 public:
   virtual ~QPlatformClipboard();

   virtual QMimeData *mimeData(QClipboard::Mode mode = QClipboard::Clipboard);
   virtual void setMimeData(QMimeData *data, QClipboard::Mode mode = QClipboard::Clipboard);
   virtual bool supportsMode(QClipboard::Mode mode) const;
   virtual bool ownsMode(QClipboard::Mode mode) const;
   void emitChanged(QClipboard::Mode mode);
};

#endif // QT_NO_CLIPBOARD

#endif
