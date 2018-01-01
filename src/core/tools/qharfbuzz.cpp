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

#include <qunicodetables_p.h>
#include <qlibrary.h>
#include <qtextcodec.h>
#include <qharfbuzz_p.h>

QT_USE_NAMESPACE

extern "C" {

   HB_GraphemeClass HB_GetGraphemeClass(HB_UChar32 ch)
   {
      const QUnicodeTables::Properties *prop = QUnicodeTables::properties(ch);
      return (HB_GraphemeClass) prop->graphemeBreakClass;
   }

   HB_WordClass HB_GetWordClass(HB_UChar32 ch)
   {
      const QUnicodeTables::Properties *prop = QUnicodeTables::properties(ch);
      return (HB_WordClass) prop->wordBreakClass;
   }

   HB_SentenceClass HB_GetSentenceClass(HB_UChar32 ch)
   {
      const QUnicodeTables::Properties *prop = QUnicodeTables::properties(ch);
      return (HB_SentenceClass) prop->sentenceBreakClass;
   }

   HB_LineBreakClass HB_GetLineBreakClass(HB_UChar32 ch)
   {
      return (HB_LineBreakClass)QUnicodeTables::lineBreakClass(ch);
   }

   void HB_GetGraphemeAndLineBreakClass(HB_UChar32 ch, HB_GraphemeClass *grapheme, HB_LineBreakClass *lineBreak)
   {
      const QUnicodeTables::Properties *prop = QUnicodeTables::properties(ch);
      *grapheme  = (HB_GraphemeClass)  prop->graphemeBreakClass;
      *lineBreak = (HB_LineBreakClass) prop->lineBreakClass;
   }

   void HB_GetUnicodeCharProperties(HB_UChar32 ch, HB_CharCategory *category, int *combiningClass)
   {
      const QUnicodeTables::Properties *prop = QUnicodeTables::properties(ch);
      *category = (HB_CharCategory)prop->category;
      *combiningClass = prop->combiningClass;
   }

   HB_CharCategory HB_GetUnicodeCharCategory(HB_UChar32 ch)
   {
      return (HB_CharCategory)QChar::category(ch);
   }

   int HB_GetUnicodeCharCombiningClass(HB_UChar32 ch)
   {
      return QChar::combiningClass(ch);
   }

   HB_UChar16 HB_GetMirroredChar(HB_UChar16 ch)
   {
      return QChar::mirroredChar(ch);
   }

   void *HB_Library_Resolve(const char *library, int version, const char *symbol)
   {
      return QLibrary::resolve(QLatin1String(library), version, symbol);
   }

} // extern "C"

QT_BEGIN_NAMESPACE

HB_Bool qShapeItem(HB_ShaperItem *item)
{
   return HB_ShapeItem(item);
}

HB_Face qHBNewFace(void *font, HB_GetFontTableFunc tableFunc)
{
   return HB_NewFace(font, tableFunc);
}

void qHBFreeFace(HB_Face face)
{
   HB_FreeFace(face);
}

void qGetCharAttributes(const HB_UChar16 *string, hb_uint32 stringLength,
                        const HB_ScriptItem *items, hb_uint32 numItems,
                        HB_CharAttributes *attributes)
{
   HB_GetCharAttributes(string, stringLength, items, numItems, attributes);
}

QT_END_NAMESPACE
