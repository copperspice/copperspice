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

#include <qglobal.h>

#if !defined(QT_NO_RAWFONT)

#include <qrawfont_p.h>
#include <qfontengine_ft_p.h>
#include <quuid.h>

#if defined(Q_WS_X11) && !defined(QT_NO_FONTCONFIG)
#  include <qfontengine_x11_p.h>
#endif

QT_BEGIN_NAMESPACE

class QFontEngineFTRawFont

#if defined(Q_WS_X11) && !defined(QT_NO_FONTCONFIG)
   : public QFontEngineX11FT
#else
   : public QFontEngineFT
#endif

{
 public:
   QFontEngineFTRawFont(const QFontDef &fontDef)
#if defined(Q_WS_X11) && !defined(QT_NO_FONTCONFIG)
      : QFontEngineX11FT(fontDef)
#else
      : QFontEngineFT(fontDef)
#endif
   {
   }

   void updateFamilyNameAndStyle() {
      fontDef.family = QString::fromLatin1(freetype->face->family_name);

      if (freetype->face->style_flags & FT_STYLE_FLAG_ITALIC) {
         fontDef.style = QFont::StyleItalic;
      }

      if (freetype->face->style_flags & FT_STYLE_FLAG_BOLD) {
         fontDef.weight = QFont::Bold;
      }
   }

   bool initFromData(const QByteArray &fontData) {
      FaceId faceId;
      faceId.filename = "";
      faceId.index = 0;
      faceId.uuid = QUuid::createUuid().toByteArray();

      return init(faceId, true, Format_None, fontData);
   }
};


void QRawFontPrivate::platformCleanUp()
{
   // Font engine handles all resources
}

void QRawFontPrivate::platformLoadFromData(const QByteArray &fontData, qreal pixelSize,
      QFont::HintingPreference hintingPreference)
{
   Q_ASSERT(fontEngine == 0);

   QFontDef fontDef;
   fontDef.pixelSize = pixelSize;

   QFontEngineFTRawFont *fe = new QFontEngineFTRawFont(fontDef);
   if (!fe->initFromData(fontData)) {
      delete fe;
      return;
   }

   fe->updateFamilyNameAndStyle();

   switch (hintingPreference) {
      case QFont::PreferNoHinting:
         fe->setDefaultHintStyle(QFontEngineFT::HintNone);
         break;
      case QFont::PreferFullHinting:
         fe->setDefaultHintStyle(QFontEngineFT::HintFull);
         break;
      case QFont::PreferVerticalHinting:
         fe->setDefaultHintStyle(QFontEngineFT::HintLight);
         break;
      default:
         // Leave it as it is
         break;
   }

   fontEngine = fe;
   fontEngine->ref.ref();
}

QT_END_NAMESPACE

#endif // QT_NO_RAWFONT
