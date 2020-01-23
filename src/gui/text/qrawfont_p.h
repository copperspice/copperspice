/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
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

#ifndef QRAWFONT_P_H
#define QRAWFONT_P_H

#include <qrawfont.h>
#include <qfontengine_p.h>
#include <qthread.h>
#include <qthreadstorage.h>

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
   {}

   QRawFontPrivate(const QRawFontPrivate &other)
      : fontEngine(other.fontEngine)
      , hintingPreference(other.hintingPreference)
      , thread(other.thread) {
      if (fontEngine != 0) {
         fontEngine->ref.ref();
      }
   }

   ~QRawFontPrivate() {
      Q_ASSERT(ref.load() == 0);
      cleanUp();
   }

   inline void cleanUp() {
      setFontEngine(0);
      hintingPreference = QFont::PreferDefaultHinting;
   }
   inline bool isValid() const {
      Q_ASSERT(fontEngine == 0 || thread == QThread::currentThread());
      return fontEngine != 0;
   }

   inline void setFontEngine(QFontEngine *engine) {
      Q_ASSERT(fontEngine == 0 || thread == QThread::currentThread());

      if (fontEngine == engine) {
         return;
      }

      if (fontEngine != 0) {
         if (!fontEngine->ref.deref()) {
            delete fontEngine;
         }

         thread = 0;

      }

      fontEngine = engine;

      if (fontEngine != 0) {
         fontEngine->ref.ref();

         thread = QThread::currentThread();
         Q_ASSERT(thread);

      }
   }
   void loadFromData(const QByteArray &fontData,
      qreal pixelSize,
      QFont::HintingPreference hintingPreference);

   static QRawFontPrivate *get(const QRawFont &font) {
      return font.d.data();
   }

   QFontEngine *fontEngine;
   QFont::HintingPreference hintingPreference;

   QAtomicInt ref;

 private:
   QThread *thread;

};

#endif
