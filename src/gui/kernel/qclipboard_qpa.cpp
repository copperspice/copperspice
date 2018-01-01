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

#include <qclipboard.h>

#ifndef QT_NO_CLIPBOARD

#include <qmimedata.h>
#include <qapplication_p.h>
#include <qplatformclipboard_qpa.h>

QT_BEGIN_NAMESPACE

QT_USE_NAMESPACE

void QClipboard::clear(Mode mode)
{
   setMimeData(0, mode);
}


bool QClipboard::event(QEvent *e)
{
   return QObject::event(e);
}

const QMimeData *QClipboard::mimeData(Mode mode) const
{
   QPlatformClipboard *clipboard = QApplicationPrivate::platformIntegration()->clipboard();
   if (!clipboard->supportsMode(mode)) {
      return 0;
   }
   return clipboard->mimeData(mode);
}

void QClipboard::setMimeData(QMimeData *src, Mode mode)
{
   QPlatformClipboard *clipboard = QApplicationPrivate::platformIntegration()->clipboard();
   if (!clipboard->supportsMode(mode)) {
      return;
   }

   clipboard->setMimeData(src, mode);

   emitChanged(mode);
}

bool QClipboard::supportsMode(Mode mode) const
{
   QPlatformClipboard *clipboard = QApplicationPrivate::platformIntegration()->clipboard();
   return clipboard->supportsMode(mode);
}

bool QClipboard::ownsMode(Mode mode) const
{
   if (mode == Clipboard) {
      qWarning("QClipboard::ownsClipboard: UNIMPLEMENTED!");
   }
   return false;
}

void QClipboard::connectNotify( const char *)
{
}

void QClipboard::ownerDestroyed()
{
}

#endif // QT_NO_CLIPBOARD

QT_END_NAMESPACE
