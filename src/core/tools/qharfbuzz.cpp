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

extern "C" {

   HB_GraphemeClass HB_GetGraphemeClass(HB_UChar32 ch)
   {
      const QUnicodeTables::Properties *prop = QUnicodeTables::properties(ch);

      // temporary fix to make the old HB enums work with new Unicode tables
      HB_GraphemeClass retval = HB_Grapheme_Other;

      switch (prop->graphemeBreakClass) {
         case QUnicodeTables::GraphemeBreak_Any:
            retval = HB_Grapheme_Other;
            break;

         case QUnicodeTables::GraphemeBreak_CR:
            retval = HB_Grapheme_CR;
            break;

         case QUnicodeTables::GraphemeBreak_LF:
            retval = HB_Grapheme_LF;
            break;

         case QUnicodeTables::GraphemeBreak_Control:
            retval = HB_Grapheme_Control;
            break;

         case QUnicodeTables::GraphemeBreak_Extend:
            retval = HB_Grapheme_Extend;
            break;

         case QUnicodeTables::GraphemeBreak_ZWJ:
            retval = HB_Grapheme_Other;
            break;

         case QUnicodeTables::GraphemeBreak_RegionalIndicator:
            retval = HB_GraphemeBreak_RegionalIndicator;
            break;

         case QUnicodeTables::GraphemeBreak_Prepend:
            retval = HB_GraphemeBreak_Prepend;
            break;

         case QUnicodeTables::GraphemeBreak_SpacingMark:
            retval = HB_GraphemeBreak_SpacingMark;
            break;

         case QUnicodeTables::GraphemeBreak_L:
            retval = HB_Grapheme_L;
            break;

         case QUnicodeTables::GraphemeBreak_V:
            retval = HB_Grapheme_V;
            break;

         case QUnicodeTables::GraphemeBreak_T:
            retval = HB_Grapheme_T;
            break;

         case QUnicodeTables::GraphemeBreak_LV:
            retval = HB_Grapheme_LV;
            break;

         case QUnicodeTables::GraphemeBreak_LVT:
            retval = HB_Grapheme_LVT;
            break;

         case QUnicodeTables::Graphemebreak_E_Base:
            retval = HB_Grapheme_Other;
            break;

         case QUnicodeTables::Graphemebreak_E_Modifier:
            retval = HB_Grapheme_Other;
            break;

         case QUnicodeTables::Graphemebreak_Glue_After_Zwj:
            retval = HB_Grapheme_Extend;
            break;

         case QUnicodeTables::Graphemebreak_E_Base_GAZ:
            retval = HB_Grapheme_Other;
            break;
      }

      return retval;
   }

   HB_WordClass HB_GetWordClass(HB_UChar32 ch)
   {
      const QUnicodeTables::Properties *prop = QUnicodeTables::properties(ch);

      // temporary fix to make the old HB enums work with new Unicode tables
      HB_WordClass retval = HB_Word_Other;

      switch (prop->wordBreakClass) {

         case QUnicodeTables::WordBreak_Any:
            retval = HB_Word_Other;
            break;

         case QUnicodeTables::WordBreak_CR:
            retval = HB_Word_CR;
            break;

         case QUnicodeTables::WordBreak_LF:
            retval = HB_Word_LF;
            break;

         case QUnicodeTables::WordBreak_Newline:
            retval = HB_Word_NewLine;
            break;

         case QUnicodeTables::WordBreak_Extend:
            retval = HB_Word_Extend;
            break;

         case QUnicodeTables::WordBreak_ZWJ:
            retval = HB_Word_Other;
            break;

         case QUnicodeTables::WordBreak_Format:
            retval = HB_Word_Extend;
            break;

         case QUnicodeTables::WordBreak_RegionalIndicator:
            retval = HB_Word_RegionalIndicator;
            break;

         case QUnicodeTables::WordBreak_Katakana:
            retval = HB_Word_Katakana;
            break;

         case QUnicodeTables::WordBreak_HebrewLetter:
            retval = HB_Word_HebrewLetter;
            break;

         case QUnicodeTables::WordBreak_ALetter:
            retval = HB_Word_ALetter;
            break;

         case QUnicodeTables::WordBreak_SingleQuote:
            retval = HB_Word_SingleQuote;
            break;

         case QUnicodeTables::WordBreak_DoubleQuote:
            retval = HB_Word_DoubleQuote;
            break;

         case QUnicodeTables::WordBreak_MidNumLet:
            retval = HB_Word_MidNumLet;
            break;

         case QUnicodeTables::WordBreak_MidLetter:
            retval = HB_Word_MidLetter;
            break;

         case QUnicodeTables::WordBreak_MidNum:
            retval = HB_Word_MidNum;
            break;

         case QUnicodeTables::WordBreak_Numeric:
            retval = HB_Word_Numeric;
            break;

         case QUnicodeTables::WordBreak_ExtendNumLet:
            retval = HB_Word_ExtendNumLet;
            break;

	 case QUnicodeTables::WordBreak_E_Base:
         case QUnicodeTables::WordBreak_E_Modifier:
         case QUnicodeTables::WordBreak_Glue_After_Zwj:
         case QUnicodeTables::WordBreak_E_Base_GAZ:
         case QUnicodeTables::WordBreak_WSegSpace:
            retval = HB_Word_Other;
            break;
      }

      return retval;
   }

   HB_SentenceClass HB_GetSentenceClass(HB_UChar32 ch)
   {
      const QUnicodeTables::Properties *prop = QUnicodeTables::properties(ch);

      // temporary fix to make the old HB enums work with new Unicode tables
      HB_SentenceClass retval = HB_Sentence_Other;

      switch (prop->sentenceBreakClass) {

         case QUnicodeTables::SentenceBreak_Any:
            retval = HB_Sentence_Other;
            break;

         case QUnicodeTables::SentenceBreak_CR:
            retval = HB_Sentence_CR;
            break;

         case QUnicodeTables::SentenceBreak_LF:
            retval = HB_Sentence_LF;
            break;

         case QUnicodeTables::SentenceBreak_Extend:
            retval = HB_Sentence_Extend;
            break;

         case QUnicodeTables::SentenceBreak_Sep:
            retval = HB_Sentence_Sep;
            break;

         case QUnicodeTables::SentenceBreak_Format:
            retval = HB_Sentence_Extend;
            break;

         case QUnicodeTables::SentenceBreak_Sp:
            retval = HB_Sentence_Sp;
            break;

         case QUnicodeTables::SentenceBreak_Lower:
            retval = HB_Sentence_Lower;
            break;

         case QUnicodeTables::SentenceBreak_Upper:
            retval = HB_Sentence_Upper;
            break;

         case QUnicodeTables::SentenceBreak_OLetter:
            retval = HB_Sentence_OLetter;
            break;

         case QUnicodeTables::SentenceBreak_Numeric:
            retval = HB_Sentence_Numeric;
            break;

         case QUnicodeTables::SentenceBreak_ATerm:
            retval = HB_Sentence_ATerm;
            break;

         case QUnicodeTables::SentenceBreak_SContinue:
            retval = HB_Sentence_SContinue;
            break;

         case QUnicodeTables::SentenceBreak_STerm:
            retval = HB_Sentence_STerm;
            break;

         case QUnicodeTables::SentenceBreak_Close:
            retval = HB_Sentence_Close;
            break;
      }

      return retval;
   }

   HB_LineBreakClass HB_GetLineBreakClass(HB_UChar32 ch)
   {
      // temporary fix to make the old HB enums work with new Unicode tables
      HB_LineBreakClass retval = HB_LineBreak_OP;

      switch (QUnicodeTables::lineBreakClass(ch)) {

         case QUnicodeTables::LineBreak_EB:
         case QUnicodeTables::LineBreak_EM:
         case QUnicodeTables::LineBreak_ZWJ:
            retval = HB_LineBreak_WJ;
            break;

         default:
            retval = static_cast<HB_LineBreakClass>(QUnicodeTables::lineBreakClass(ch));
      }

      return retval;
   }

   void HB_GetGraphemeAndLineBreakClass(HB_UChar32 ch, HB_GraphemeClass *grapheme, HB_LineBreakClass *lineBreak)
   {
      const QUnicodeTables::Properties *prop = QUnicodeTables::properties(ch);
      *grapheme  = static_cast<HB_GraphemeClass>(prop->graphemeBreakClass);
      *lineBreak = static_cast<HB_LineBreakClass>(prop->lineBreakClass);
   }

   void HB_GetUnicodeCharProperties(HB_UChar32 ch, HB_CharCategory *category, int *combiningClass)
   {
      const QUnicodeTables::Properties *prop = QUnicodeTables::properties(ch);
      *category       = static_cast<HB_CharCategory>(prop->category);
      *combiningClass = prop->combiningClass;
   }

   HB_CharCategory HB_GetUnicodeCharCategory(HB_UChar32 ch)
   {
      return static_cast<HB_CharCategory>(QChar(char32_t(ch)).category());
   }

   int HB_GetUnicodeCharCombiningClass(HB_UChar32 ch)
   {
      return QChar(char32_t(ch)).combiningClass();
   }

   HB_UChar16 HB_GetMirroredChar(HB_UChar16 ch)
   {
      return QChar(char32_t(ch)).mirroredChar().unicode();
   }

   void *HB_Library_Resolve(const char *library, int version, const char *symbol)
   {
      return QLibrary::resolve(QString::fromUtf8(library), version, QString::fromUtf8(symbol));
   }

} // extern "C"

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

void qGetCharAttributes(const HB_UChar16 *string, hb_uint32 stringLength, const HB_ScriptItem *items, hb_uint32 numItems, HB_CharAttributes *attributes)
{
   HB_GetCharAttributes(string, stringLength, items, numItems, attributes);
}

