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
#include <QtGui/qplatformfontdatabase_qpa.h>
#include <qapplication_p.h>

QT_BEGIN_NAMESPACE

void QRawFontPrivate::platformCleanUp()
{
}

void QRawFontPrivate::platformLoadFromData(const QByteArray &fontData, qreal pixelSize,
      QFont::HintingPreference hintingPreference)
{
   Q_ASSERT(fontEngine == 0);

   QPlatformFontDatabase *pfdb = QApplicationPrivate::platformIntegration()->fontDatabase();
   fontEngine = pfdb->fontEngine(fontData, pixelSize, hintingPreference);
   if (fontEngine != 0) {
      fontEngine->ref.ref();
   }
}

QT_END_NAMESPACE

#endif // QT_NO_RAWFONT
