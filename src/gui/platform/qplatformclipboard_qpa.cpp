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

#include <qplatformclipboard_qpa.h>

#ifndef QT_NO_CLIPBOARD

#include <qapplication_p.h>

QT_BEGIN_NAMESPACE

class QClipboardData
{
 public:
   QClipboardData();
   ~QClipboardData();

   void setSource(QMimeData *s) {
      if (s == src) {
         return;
      }
      delete src;
      src = s;
   }
   QMimeData *source() {
      return src;
   }

 private:
   QMimeData *src;
};

QClipboardData::QClipboardData()
{
   src = 0;
}

QClipboardData::~QClipboardData()
{
   delete src;
}

Q_GLOBAL_STATIC(QClipboardData, q_clipboardData);

QPlatformClipboard::~QPlatformClipboard()
{

}

QMimeData *QPlatformClipboard::mimeData(QClipboard::Mode mode)
{
   //we know its clipboard
   Q_UNUSED(mode);
   return q_clipboardData()->source();
}

void QPlatformClipboard::setMimeData(QMimeData *data, QClipboard::Mode mode)
{
   //we know its clipboard
   Q_UNUSED(mode);
   q_clipboardData()->setSource(data);
}

bool QPlatformClipboard::supportsMode(QClipboard::Mode mode) const
{
   return mode == QClipboard::Clipboard;
}

void QPlatformClipboard::emitChanged(QClipboard::Mode mode)
{
   QApplication::clipboard()->emitChanged(mode);
}

QT_END_NAMESPACE

#endif //QT_NO_CLIPBOARD
