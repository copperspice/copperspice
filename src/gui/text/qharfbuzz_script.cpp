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

#include <qharfbuzz_p.h>

#include <qstring.h>
#include <qvector.h>

static const hb_script_t cs_internal_scriptTable[] = {
   HB_SCRIPT_UNKNOWN,
   HB_SCRIPT_INHERITED,
   HB_SCRIPT_COMMON,

   HB_SCRIPT_LATIN,
   HB_SCRIPT_GREEK,
   HB_SCRIPT_CYRILLIC,
   HB_SCRIPT_ARMENIAN,
   HB_SCRIPT_HEBREW,
   HB_SCRIPT_ARABIC,
   HB_SCRIPT_SYRIAC,
   HB_SCRIPT_THAANA,
   HB_SCRIPT_DEVANAGARI,
   HB_SCRIPT_BENGALI,
   HB_SCRIPT_GURMUKHI,
   HB_SCRIPT_GUJARATI,
   HB_SCRIPT_ORIYA,
   HB_SCRIPT_TAMIL,
   HB_SCRIPT_TELUGU,
   HB_SCRIPT_KANNADA,
   HB_SCRIPT_MALAYALAM,
   HB_SCRIPT_SINHALA,
   HB_SCRIPT_THAI,
   HB_SCRIPT_LAO,
   HB_SCRIPT_TIBETAN,
   HB_SCRIPT_MYANMAR,
   HB_SCRIPT_GEORGIAN,
   HB_SCRIPT_HANGUL,
   HB_SCRIPT_ETHIOPIC,
   HB_SCRIPT_CHEROKEE,
   HB_SCRIPT_CANADIAN_ABORIGINAL,
   HB_SCRIPT_OGHAM,
   HB_SCRIPT_RUNIC,
   HB_SCRIPT_KHMER,
   HB_SCRIPT_MONGOLIAN,
   HB_SCRIPT_HIRAGANA,
   HB_SCRIPT_KATAKANA,
   HB_SCRIPT_BOPOMOFO,
   HB_SCRIPT_HAN,
   HB_SCRIPT_YI,
   HB_SCRIPT_OLD_ITALIC,
   HB_SCRIPT_GOTHIC,
   HB_SCRIPT_DESERET,
   HB_SCRIPT_TAGALOG,
   HB_SCRIPT_HANUNOO,
   HB_SCRIPT_BUHID,
   HB_SCRIPT_TAGBANWA,
   HB_SCRIPT_COPTIC,

   // Unicode 4.0
   HB_SCRIPT_LIMBU,
   HB_SCRIPT_TAI_LE,
   HB_SCRIPT_LINEAR_B,
   HB_SCRIPT_UGARITIC,
   HB_SCRIPT_SHAVIAN,
   HB_SCRIPT_OSMANYA,
   HB_SCRIPT_CYPRIOT,
   HB_SCRIPT_BRAILLE,

   // Unicode 4.1
   HB_SCRIPT_BUGINESE,
   HB_SCRIPT_NEW_TAI_LUE,
   HB_SCRIPT_GLAGOLITIC,
   HB_SCRIPT_TIFINAGH,
   HB_SCRIPT_SYLOTI_NAGRI,
   HB_SCRIPT_OLD_PERSIAN,
   HB_SCRIPT_KHAROSHTHI,

   // Unicode 5.0
   HB_SCRIPT_BALINESE,
   HB_SCRIPT_CUNEIFORM,
   HB_SCRIPT_PHOENICIAN,
   HB_SCRIPT_PHAGS_PA,
   HB_SCRIPT_NKO,

   // Unicode 5.1
   HB_SCRIPT_SUNDANESE,
   HB_SCRIPT_LEPCHA,
   HB_SCRIPT_OL_CHIKI,
   HB_SCRIPT_VAI,
   HB_SCRIPT_SAURASHTRA,
   HB_SCRIPT_KAYAH_LI,
   HB_SCRIPT_REJANG,
   HB_SCRIPT_LYCIAN,
   HB_SCRIPT_CARIAN,
   HB_SCRIPT_LYDIAN,
   HB_SCRIPT_CHAM,

   // Unicode 5.2
   HB_SCRIPT_TAI_THAM,
   HB_SCRIPT_TAI_VIET,
   HB_SCRIPT_AVESTAN,
   HB_SCRIPT_EGYPTIAN_HIEROGLYPHS,
   HB_SCRIPT_SAMARITAN,
   HB_SCRIPT_LISU,
   HB_SCRIPT_BAMUM,
   HB_SCRIPT_JAVANESE,
   HB_SCRIPT_MEETEI_MAYEK,
   HB_SCRIPT_IMPERIAL_ARAMAIC,
   HB_SCRIPT_OLD_SOUTH_ARABIAN,
   HB_SCRIPT_INSCRIPTIONAL_PARTHIAN,
   HB_SCRIPT_INSCRIPTIONAL_PAHLAVI,
   HB_SCRIPT_OLD_TURKIC,
   HB_SCRIPT_KAITHI,

   // Unicode 6.0
   HB_SCRIPT_BATAK,
   HB_SCRIPT_BRAHMI,
   HB_SCRIPT_MANDAIC,

   // Unicode 6.1
   HB_SCRIPT_CHAKMA,
   HB_SCRIPT_MEROITIC_CURSIVE,
   HB_SCRIPT_MEROITIC_HIEROGLYPHS,
   HB_SCRIPT_MIAO,
   HB_SCRIPT_SHARADA,
   HB_SCRIPT_SORA_SOMPENG,
   HB_SCRIPT_TAKRI,

   // Unicode 7.0
   HB_SCRIPT_CAUCASIAN_ALBANIAN,
   HB_SCRIPT_BASSA_VAH,
   HB_SCRIPT_DUPLOYAN,
   HB_SCRIPT_ELBASAN,
   HB_SCRIPT_GRANTHA,
   HB_SCRIPT_PAHAWH_HMONG,
   HB_SCRIPT_KHOJKI,
   HB_SCRIPT_LINEAR_A,
   HB_SCRIPT_MAHAJANI,
   HB_SCRIPT_MANICHAEAN,
   HB_SCRIPT_MENDE_KIKAKUI,
   HB_SCRIPT_MODI,
   HB_SCRIPT_MRO,
   HB_SCRIPT_OLD_NORTH_ARABIAN,
   HB_SCRIPT_NABATAEAN,
   HB_SCRIPT_PALMYRENE,
   HB_SCRIPT_PAU_CIN_HAU,
   HB_SCRIPT_OLD_PERMIC,
   HB_SCRIPT_PSALTER_PAHLAVI,
   HB_SCRIPT_SIDDHAM,
   HB_SCRIPT_KHUDAWADI,
   HB_SCRIPT_TIRHUTA,
   HB_SCRIPT_WARANG_CITI,

   // Unicode 8.0
   HB_SCRIPT_AHOM,
   HB_SCRIPT_ANATOLIAN_HIEROGLYPHS,
   HB_SCRIPT_HATRAN,
   HB_SCRIPT_MULTANI,
   HB_SCRIPT_OLD_HUNGARIAN,
   HB_SCRIPT_SIGNWRITING,

   // Unicode 9.0
   HB_SCRIPT_ADLAM,
   HB_SCRIPT_BHAIKSUKI,
   HB_SCRIPT_MARCHEN,
   HB_SCRIPT_NEWA,
   HB_SCRIPT_OSAGE,
   HB_SCRIPT_TANGUT,

   // Unicode 10.0
   HB_SCRIPT_MASARAM_GONDI,
   HB_SCRIPT_NUSHU,
   HB_SCRIPT_SOYOMBO,
   HB_SCRIPT_ZANABAZAR_SQUARE,

   // Unicode 11.0
   HB_SCRIPT_DOGRA,
   HB_SCRIPT_GUNJALA_GONDI,
   HB_SCRIPT_HANIFI_ROHINGYA,
   HB_SCRIPT_MAKASAR,
   HB_SCRIPT_MEDEFAIDRIN,
   HB_SCRIPT_OLD_SOGDIAN,
   HB_SCRIPT_SOGDIAN,

   // Unicode 12.0
   HB_SCRIPT_ELYMAIC,
   HB_SCRIPT_NANDINAGARI,
   HB_SCRIPT_NYIAKENG_PUACHUE_HMONG,
   HB_SCRIPT_WANCHO,

   // Unicode 13.0
   HB_SCRIPT_CHORASMIAN,
   HB_SCRIPT_DIVES_AKURU,
   HB_SCRIPT_KHITAN_SMALL_SCRIPT,
   HB_SCRIPT_YEZIDI,

   // Unicode 14.0
   HB_SCRIPT_CYPRO_MINOAN,
   HB_SCRIPT_OLD_UYGHUR,
   HB_SCRIPT_TANGSA,
   HB_SCRIPT_TOTO,
   HB_SCRIPT_VITHKUQI,

   // Unicode 15.0
   HB_SCRIPT_KAWI,
   HB_SCRIPT_NAG_MUNDARI,

   // Unicode 15.1
   // nothing was added
};
static_assert(QChar::ScriptCount == sizeof(cs_internal_scriptTable) / sizeof(cs_internal_scriptTable[0]), "QChar script count mismatch");

static const hb_unicode_general_category_t cs_internal_categoryTable[] = {
   HB_UNICODE_GENERAL_CATEGORY_NON_SPACING_MARK,    //   Mn
   HB_UNICODE_GENERAL_CATEGORY_SPACING_MARK,        //   Mc
   HB_UNICODE_GENERAL_CATEGORY_ENCLOSING_MARK,      //   Me

   HB_UNICODE_GENERAL_CATEGORY_DECIMAL_NUMBER,      //   Nd
   HB_UNICODE_GENERAL_CATEGORY_LETTER_NUMBER,       //   Nl
   HB_UNICODE_GENERAL_CATEGORY_OTHER_NUMBER,        //   No

   HB_UNICODE_GENERAL_CATEGORY_SPACE_SEPARATOR,     //   Zs
   HB_UNICODE_GENERAL_CATEGORY_LINE_SEPARATOR,      //   Zl
   HB_UNICODE_GENERAL_CATEGORY_PARAGRAPH_SEPARATOR, //   Zp

   HB_UNICODE_GENERAL_CATEGORY_CONTROL,             //   Cc
   HB_UNICODE_GENERAL_CATEGORY_FORMAT,              //   Cf
   HB_UNICODE_GENERAL_CATEGORY_SURROGATE,           //   Cs
   HB_UNICODE_GENERAL_CATEGORY_PRIVATE_USE,         //   Co
   HB_UNICODE_GENERAL_CATEGORY_UNASSIGNED,          //   Cn

   HB_UNICODE_GENERAL_CATEGORY_UPPERCASE_LETTER,    //   Lu
   HB_UNICODE_GENERAL_CATEGORY_LOWERCASE_LETTER,    //   Ll
   HB_UNICODE_GENERAL_CATEGORY_TITLECASE_LETTER,    //   Lt
   HB_UNICODE_GENERAL_CATEGORY_MODIFIER_LETTER,     //   Lm
   HB_UNICODE_GENERAL_CATEGORY_OTHER_LETTER,        //   Lo

   HB_UNICODE_GENERAL_CATEGORY_CONNECT_PUNCTUATION, //   Pc
   HB_UNICODE_GENERAL_CATEGORY_DASH_PUNCTUATION,    //   Pd
   HB_UNICODE_GENERAL_CATEGORY_OPEN_PUNCTUATION,    //   Ps
   HB_UNICODE_GENERAL_CATEGORY_CLOSE_PUNCTUATION,   //   Pe
   HB_UNICODE_GENERAL_CATEGORY_INITIAL_PUNCTUATION, //   Pi
   HB_UNICODE_GENERAL_CATEGORY_FINAL_PUNCTUATION,   //   Pf
   HB_UNICODE_GENERAL_CATEGORY_OTHER_PUNCTUATION,   //   Po

   HB_UNICODE_GENERAL_CATEGORY_MATH_SYMBOL,         //   Sm
   HB_UNICODE_GENERAL_CATEGORY_CURRENCY_SYMBOL,     //   Sc
   HB_UNICODE_GENERAL_CATEGORY_MODIFIER_SYMBOL,     //   Sk
   HB_UNICODE_GENERAL_CATEGORY_OTHER_SYMBOL         //   So
};
static_assert(QChar::CategoryCount == sizeof(cs_internal_categoryTable) / sizeof(cs_internal_categoryTable[0]), "QChar category count mismatch");


hb_script_t cs_script_to_hb_script(QChar::Script script)
{
   return cs_internal_scriptTable[script];
}

static hb_unicode_combining_class_t cs_combining_class(hb_unicode_funcs_t *, hb_codepoint_t unicode, void *)
{
   return hb_unicode_combining_class_t(QChar(char32_t(unicode)).combiningClass());
}

static hb_codepoint_t cs_mirroring(hb_unicode_funcs_t *, hb_codepoint_t unicode, void *)
{
   return QChar(char32_t(unicode)).mirroredChar().unicode();
}

static hb_unicode_general_category_t cs_category(hb_unicode_funcs_t *, hb_codepoint_t unicode, void *)
{
   return cs_internal_categoryTable[QChar(char32_t(unicode)).category()];
}

static hb_script_t cs_script(hb_unicode_funcs_t *, hb_codepoint_t unicode, void *)
{
   return cs_internal_scriptTable[QChar(char32_t(unicode)).script()];
}

static hb_bool_t cs_compose(hb_unicode_funcs_t *, hb_codepoint_t a, hb_codepoint_t b, hb_codepoint_t *ab, void *)
{
   QString s = QString(char32_t(a)) + QString(char32_t(b));

   QString normalized = s.normalized(QString::NormalizationForm_C);
   Q_ASSERT(! normalized.empty());

   *ab = normalized[0].unicode();

   return normalized.size() == 1;
}

static hb_bool_t cs_decompose(hb_unicode_funcs_t *, hb_codepoint_t ab, hb_codepoint_t *a, hb_codepoint_t *b, void *)
{
   QChar ch_ab = QChar(char32_t(ab));

   if (ch_ab.decompositionTag() != QChar::Canonical) {
      return false;
   }

   QString normalized = ch_ab.decomposition();
   if (normalized.isEmpty()) {
      return false;
   }

   *a = normalized[0].unicode();

   if (normalized.size() == 1) {
      *b = 0;
      return *a != ab;
   }

   *b = normalized[1].unicode();

   if (normalized.size() == 2) {
      // if ab decomposes to a single character and that character decomposes again,
      // we have to detect and undo the second part

      const QString recomposed = normalized.normalized(QString::NormalizationForm_C);
      Q_ASSERT(! recomposed.isEmpty());

      const hb_codepoint_t c = recomposed[0].unicode();

      if (c != *a && c != ab) {
         *a = c;
         *b = 0;
      }

      return true;
   }

   // If decomposed to more than two characters, take the last one and recompose the rest to get the first component

   *b = normalized.last().unicode();
   normalized.chop(1);

   const QString recomposed = normalized.normalized(QString::NormalizationForm_C);
   Q_ASSERT(recomposed.size() == 1);

   // expect that recomposed has exactly one character now
   *a = recomposed[0].unicode();

   return true;
}

/* not used right now

static unsigned int cs_decompose_compatibility(hb_unicode_funcs_t *, hb_codepoint_t u,
   hb_codepoint_t *decomposed, void *)
{
   const QString normalized = QChar(char32_t(u)).decomposition();

   uint outlen = 0;

   for (QChar ch : normalized) {
      Q_ASSERT(outlen < HB_UNICODE_MAX_DECOMPOSITION_LEN);
      decomposed[outlen++] = ch.unicode();
   }

   return outlen;
}

*/

struct cs_hb_unicode_funcs_t {

   cs_hb_unicode_funcs_t() {
      funcs = hb_unicode_funcs_create(nullptr);

      hb_unicode_funcs_set_combining_class_func(funcs,   cs_combining_class, nullptr, nullptr);
      hb_unicode_funcs_set_general_category_func(funcs,  cs_category, nullptr, nullptr);
      hb_unicode_funcs_set_mirroring_func(funcs,         cs_mirroring, nullptr, nullptr);
      hb_unicode_funcs_set_script_func(funcs,            cs_script, nullptr, nullptr);
      hb_unicode_funcs_set_compose_func(funcs,           cs_compose, nullptr, nullptr);
      hb_unicode_funcs_set_decompose_func(funcs,         cs_decompose, nullptr, nullptr);
   }

   ~cs_hb_unicode_funcs_t() {
      hb_unicode_funcs_destroy(funcs);
   }

   hb_unicode_funcs_t *funcs;
};

hb_unicode_funcs_t *cs_get_unicode_funcs()
{
   static cs_hb_unicode_funcs_t retval;
   return retval.funcs;
}
