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

#include <QtCore/qglobal.h>

#if !defined(QT_NO_RAWFONT)

#include <qrawfont_p.h>
#include <qfontengine_coretext_p.h>

QT_BEGIN_NAMESPACE

void QRawFontPrivate::platformCleanUp()
{
}

static void releaseFontData(void *info, const void *data, size_t size)
{
   Q_UNUSED(data);
   Q_UNUSED(size);
   delete (QByteArray *)info;
}

extern int qt_defaultDpi();

void QRawFontPrivate::platformLoadFromData(const QByteArray &fontData,
      qreal pixelSize,
      QFont::HintingPreference hintingPreference)
{
   // Mac OS X ignores it
   Q_UNUSED(hintingPreference);

   QByteArray *fontDataCopy = new QByteArray(fontData);
   QCFType<CGDataProviderRef> dataProvider = CGDataProviderCreateWithData(fontDataCopy,
         fontDataCopy->constData(), fontDataCopy->size(), releaseFontData);

   CGFontRef cgFont = CGFontCreateWithDataProvider(dataProvider);

   if (cgFont == NULL) {
      qWarning("QRawFont::platformLoadFromData: CGFontCreateWithDataProvider failed");
   } else {
      QFontDef def;
      def.pixelSize = pixelSize;
      def.pointSize = pixelSize * 72.0 / qt_defaultDpi();
      fontEngine = new QCoreTextFontEngine(cgFont, def);
      CFRelease(cgFont);
      fontEngine->ref.ref();
   }
}

QT_END_NAMESPACE

#endif // QT_NO_RAWFONT
