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

#include <qplatform_clipboard.h>

#ifndef QT_NO_CLIPBOARD

#include <qmimedata.h>
#include <qapplication_p.h>


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
   src = nullptr;
}

QClipboardData::~QClipboardData()
{
   delete src;
}

static QClipboardData *q_clipboardData()
{
   static QClipboardData retval;
   return &retval;
}

QPlatformClipboard::~QPlatformClipboard()
{
}

QMimeData *QPlatformClipboard::mimeData(QClipboard::Mode mode)
{
   //we know its clipboard
   (void) mode;
   return q_clipboardData()->source();
}

void QPlatformClipboard::setMimeData(QMimeData *data, QClipboard::Mode mode)
{
   //we know its clipboard
   (void) mode;

   q_clipboardData()->setSource(data);
   emitChanged(mode);
}

bool QPlatformClipboard::supportsMode(QClipboard::Mode mode) const
{
   return mode == QClipboard::Clipboard;
}

bool QPlatformClipboard::ownsMode(QClipboard::Mode mode) const
{
   (void) mode;
   return false;
}

void QPlatformClipboard::emitChanged(QClipboard::Mode mode)
{
   if (!QGuiApplicationPrivate::is_app_closing)  {
      //  prevent emission when closing down.
      QGuiApplication::clipboard()->emitChanged(mode);
   }
}

#endif //QT_NO_CLIPBOARD
