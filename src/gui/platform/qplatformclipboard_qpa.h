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

#ifndef QPLATFORMCLIPBOARD_QPA_H
#define QPLATFORMCLIPBOARD_QPA_H

#include <qplatformdefs.h>

#ifndef QT_NO_CLIPBOARD

#include <QtGui/QClipboard>

QT_BEGIN_NAMESPACE

class Q_GUI_EXPORT QPlatformClipboard
{

 public:
   virtual ~QPlatformClipboard();

   virtual QMimeData *mimeData(QClipboard::Mode mode = QClipboard::Clipboard);
   virtual void setMimeData(QMimeData *data, QClipboard::Mode mode = QClipboard::Clipboard);
   virtual bool supportsMode(QClipboard::Mode mode) const;
   void emitChanged(QClipboard::Mode mode);
};

QT_END_NAMESPACE

#endif // QT_NO_CLIPBOARD

#endif //QPLATFORMCLIPBOARD_QPA_H
