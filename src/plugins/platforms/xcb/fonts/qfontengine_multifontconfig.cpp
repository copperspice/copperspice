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

#include <qfontengine_multifontconfig_p.h>

#include <qfontengine_ft_p.h>

QFontEngineMultiFontConfig::QFontEngineMultiFontConfig(QFontEngine *fe, int script)
   : QFontEngineMulti(fe, script)
{
}

QFontEngineMultiFontConfig::~QFontEngineMultiFontConfig()
{
   for (FcPattern *pattern : cachedMatchPatterns) {
      if (pattern) {
         FcPatternDestroy(pattern);
      }
   }
}

bool QFontEngineMultiFontConfig::shouldLoadFontEngineForCharacter(int at, char32_t ch) const
{
   bool charSetHasChar = true;
   FcPattern *matchPattern = getMatchPatternForFallback(at - 1);

   if (matchPattern != nullptr) {
      FcCharSet *charSet;
      FcPatternGetCharSet(matchPattern, FC_CHARSET, 0, &charSet);
      charSetHasChar = FcCharSetHasChar(charSet, ch);
   }

   return charSetHasChar;
}

FcPattern *QFontEngineMultiFontConfig::getMatchPatternForFallback(int fallBackIndex) const
{
   Q_ASSERT(fallBackIndex < fallbackFamilyCount());

   if (fallbackFamilyCount() > cachedMatchPatterns.size()) {
      cachedMatchPatterns.resize(fallbackFamilyCount());
   }

   FcPattern *retval = cachedMatchPatterns.at(fallBackIndex);

   if (retval) {
      return retval;
   }

   FcPattern *requestPattern = FcPatternCreate();
   FcValue value;
   value.type = FcTypeString;

   QByteArray cs = fallbackFamilyAt(fallBackIndex).toUtf8();
   value.u.s = reinterpret_cast<const FcChar8 *>(cs.data());
   FcPatternAdd(requestPattern, FC_FAMILY, value, true);

   FcResult result;
   retval = FcFontMatch(nullptr, requestPattern, &result);
   cachedMatchPatterns.insert(fallBackIndex, retval);
   FcPatternDestroy(requestPattern);

   return retval;
}


