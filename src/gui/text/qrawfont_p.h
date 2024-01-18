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
      : fontEngine(nullptr), hintingPreference(QFont::PreferDefaultHinting), thread(nullptr)
   {
   }

   QRawFontPrivate(const QRawFontPrivate &other)
      : fontEngine(other.fontEngine), hintingPreference(other.hintingPreference), thread(other.thread)
   {
      if (fontEngine != nullptr) {
         fontEngine->m_refCount.ref();
      }
   }

   ~QRawFontPrivate() {
      cleanUp();
   }

   inline void cleanUp() {
      setFontEngine(nullptr);
      hintingPreference = QFont::PreferDefaultHinting;
   }

   inline bool isValid() const {
      Q_ASSERT(fontEngine == nullptr || thread == QThread::currentThread());
      return fontEngine != nullptr;
   }

   inline void setFontEngine(QFontEngine *engine) {
      Q_ASSERT(fontEngine == nullptr || thread == QThread::currentThread());

      if (fontEngine == engine) {
         return;
      }

      if (fontEngine != nullptr) {
         if (! fontEngine->m_refCount.deref()) {
            delete fontEngine;
         }

         thread = nullptr;
      }

      fontEngine = engine;

      if (fontEngine != nullptr) {
         fontEngine->m_refCount.ref();

         thread = QThread::currentThread();
         Q_ASSERT(thread);
      }
   }

   void loadFromData(const QByteArray &fontData, qreal pixelSize, QFont::HintingPreference hintingPreference);

   static std::shared_ptr<QRawFontPrivate> get(const QRawFont &font) {
      return font.m_fontPrivate;
   }

   QFontEngine *fontEngine;
   QFont::HintingPreference hintingPreference;

 private:
   QThread *thread;
};

#endif
