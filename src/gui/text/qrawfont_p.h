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

#ifndef QRAWFONT_P_H
#define QRAWFONT_P_H

#include <qrawfont.h>
#include <qfontengine_p.h>
#include <QtCore/qthread.h>
#include <QtCore/qthreadstorage.h>

#if !defined(QT_NO_RAWFONT)

QT_BEGIN_NAMESPACE

namespace {
   class CustomFontFileLoader;
}

class QRawFontPrivate
{
 public:
   QRawFontPrivate()
      : fontEngine(0)
      , hintingPreference(QFont::PreferDefaultHinting)
      , thread(0)
#if defined(Q_OS_WIN)
      , fontHandle(NULL)
#endif
   {}

   QRawFontPrivate(const QRawFontPrivate &other)
      : fontEngine(other.fontEngine)
      , hintingPreference(other.hintingPreference)
      , thread(other.thread)
#if defined(Q_OS_WIN)
      , fontHandle(NULL)
#endif
   {
      if (fontEngine != 0) {
         fontEngine->ref.ref();
      }
   }

   ~QRawFontPrivate() {
      Q_ASSERT(ref.load() == 0);
      cleanUp();
   }

   inline bool isValid() const {
      Q_ASSERT(thread == 0 || thread == QThread::currentThread());
      return fontEngine != 0;
   }

   void cleanUp();
   void platformCleanUp();
   void platformLoadFromData(const QByteArray &fontData, qreal pixelSize, QFont::HintingPreference hintingPreference);

   static QRawFontPrivate *get(const QRawFont &font) {
      return font.d.data();
   }

   QFontEngine *fontEngine;
   QFont::HintingPreference hintingPreference;
   QThread *thread;
   QAtomicInt ref;

#if defined(Q_OS_WIN)
   HANDLE fontHandle;
#endif
};

QT_END_NAMESPACE

#endif // QT_NO_RAWFONT

#endif // QRAWFONTPRIVATE_P_H
