/***********************************************************************
*
* Copyright (c) 2012-2019 Barbara Geller
* Copyright (c) 2012-2019 Ansel Sermersheim
*
* Copyright (C) 2015 The Qt Company Ltd.
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

#include <qtextengine_p.h>
#include <qfontengine_coretext_p.h>
#include <qfontengine_mac_p.h>

// set the glyph attributes heuristically. Assumes a 1 to 1 relationship between chars and glyphs
// and no reordering, also computes logClusters heuristically

static void heuristicSetGlyphAttributes(QStringView str, QGlyphLayout *glyphs, unsigned short *logClusters, int num_glyphs)
{
   // ### zeroWidth and justification are missing here
   Q_UNUSED(num_glyphs);

   const bool symbolFont = false;
   glyphs->attributes[0].mark = false;
   glyphs->attributes[0].clusterStart = true;
   glyphs->attributes[0].dontPrint = (! symbolFont && str[0] == 0x00ad) || qIsControlChar(str[0]);

   int pos = 0;
   int lastCat = str[0].category();

   int i = -1;

   for (QChar c : str) {
      ++i;

      if (logClusters[i] == pos) {
         // same glyph
         continue;
      }

      ++pos;

      while (pos < logClusters[i]) {
         ++pos;
      }

      // hide soft-hyphens by default
      if ((! symbolFont && str[i] == 0x00ad) || qIsControlChar(str[i])) {
         glyphs->attributes[pos].dontPrint = true;
      }

      const QUnicodeTables::Properties *prop = QUnicodeTables::properties(str[i].unicode());
      int cat = prop->category;

      // one gets an inter character justification point if the current char is not a non spacing mark.
      // as then the current char belongs to the last one and one gets a space justification point after the space char.

      if (lastCat == QChar::Separator_Space) {
         glyphs->attributes[pos - 1].justification = HB_Space;
      } else if (cat != QChar::Mark_NonSpacing) {
         glyphs->attributes[pos - 1].justification = HB_Character;
      } else {
         glyphs->attributes[pos - 1].justification = HB_NoJustification;
      }

      lastCat = cat;
   }

   pos = logClusters[str.size() - 1];

   if (lastCat == QChar::Separator_Space) {
      glyphs->attributes[pos].justification = HB_Space;
   } else {
      glyphs->attributes[pos].justification = HB_Character;
   }
}

struct QArabicProperties {
   unsigned char shape;
   unsigned char justification;
};
Q_DECLARE_TYPEINFO(QArabicProperties, Q_PRIMITIVE_TYPE);

enum QArabicShape {
   XIsolated,
   XFinal,
   XInitial,
   XMedial,
   // intermediate state
   XCausing
};


// these groups correspond to the groups defined in the Unicode standard, some of these groups are equal
// with regards to both joining and line breaking behaviour, and thus have the same enum value
//
// not sure the mapping of syriac to arabic enums is correct with regards to justification,
// could not find any better document

enum ArabicGroup {
   // NonJoining
   ArabicNone,
   ArabicSpace,
   // Transparent
   Transparent,
   // Causing
   Center,
   Kashida,

   // Arabic
   // Dual
   Beh,
   Noon,
   Meem = Noon,
   Heh = Noon,
   KnottedHeh = Noon,
   HehGoal = Noon,
   SwashKaf = Noon,
   Yeh,
   Hah,
   Seen,
   Sad = Seen,
   Tah,
   Kaf = Tah,
   Gaf = Tah,
   Lam = Tah,
   Ain,
   Feh = Ain,
   Qaf = Ain,
   // Right
   Alef,
   Waw,
   Dal,
   TehMarbuta = Dal,
   Reh,
   HamzaOnHehGoal,
   YehWithTail = HamzaOnHehGoal,
   YehBarre = HamzaOnHehGoal,

   // Syriac
   // Dual
   Beth = Beh,
   Gamal = Ain,
   Heth = Noon,
   Teth = Hah,
   Yudh = Noon,
   Kaph = Noon,
   Lamadh = Lam,
   Mim = Noon,
   Nun = Noon,
   Semakh = Noon,
   FinalSemakh = Noon,
   SyriacE = Ain,
   Pe = Ain,
   ReversedPe = Hah,
   Qaph = Noon,
   Shin = Noon,
   Fe = Ain,

   // Right
   Alaph = Alef,
   Dalath = Dal,
   He = Dal,
   SyriacWaw = Waw,
   Zain = Alef,
   YudhHe = Waw,
   Sadhe = HamzaOnHehGoal,
   Taw = Dal,

   // Compiler bug? Otherwise ArabicGroupsEnd would be equal to Dal + 1.
   Dummy = HamzaOnHehGoal,
   ArabicGroupsEnd
};

static const unsigned char arabic_group[0x150] = {
   ArabicNone, ArabicNone, ArabicNone, ArabicNone,
   ArabicNone, ArabicNone, ArabicNone, ArabicNone,
   ArabicNone, ArabicNone, ArabicNone, ArabicNone,
   ArabicNone, ArabicNone, ArabicNone, ArabicNone,

   Transparent, Transparent, Transparent, Transparent,
   Transparent, Transparent, ArabicNone, ArabicNone,
   ArabicNone, ArabicNone, ArabicNone, ArabicNone,
   ArabicNone, ArabicNone, ArabicNone, ArabicNone,

   ArabicNone, ArabicNone, Alef, Alef,
   Waw, Alef, Yeh, Alef,
   Beh, TehMarbuta, Beh, Beh,
   Hah, Hah, Hah, Dal,

   Dal, Reh, Reh, Seen,
   Seen, Sad, Sad, Tah,
   Tah, Ain, Ain, ArabicNone,
   ArabicNone, ArabicNone, ArabicNone, ArabicNone,

   // 0x640
   Kashida, Feh, Qaf, Kaf,
   Lam, Meem, Noon, Heh,
   Waw, Yeh, Yeh, Transparent,
   Transparent, Transparent, Transparent, Transparent,

   Transparent, Transparent, Transparent, Transparent,
   Transparent, Transparent, Transparent, Transparent,
   Transparent, ArabicNone, ArabicNone, ArabicNone,
   ArabicNone, ArabicNone, ArabicNone, ArabicNone,

   ArabicNone, ArabicNone, ArabicNone, ArabicNone,
   ArabicNone, ArabicNone, ArabicNone, ArabicNone,
   ArabicNone, ArabicNone, ArabicNone, ArabicNone,
   ArabicNone, ArabicNone, Beh, Qaf,

   Transparent, Alef, Alef, Alef,
   ArabicNone, Alef, Waw, Waw,
   Yeh, Beh, Beh, Beh,
   Beh, Beh, Beh, Beh,

   // 0x680
   Beh, Hah, Hah, Hah,
   Hah, Hah, Hah, Hah,
   Dal, Dal, Dal, Dal,
   Dal, Dal, Dal, Dal,

   Dal, Reh, Reh, Reh,
   Reh, Reh, Reh, Reh,
   Reh, Reh, Seen, Seen,
   Seen, Sad, Sad, Tah,

   Ain, Feh, Feh, Feh,
   Feh, Feh, Feh, Qaf,
   Qaf, Gaf, SwashKaf, Gaf,
   Kaf, Kaf, Kaf, Gaf,

   Gaf, Gaf, Gaf, Gaf,
   Gaf, Lam, Lam, Lam,
   Lam, Noon, Noon, Noon,
   Noon, Noon, KnottedHeh, Hah,

   // 0x6c0
   TehMarbuta, HehGoal, HamzaOnHehGoal, HamzaOnHehGoal,
   Waw, Waw, Waw, Waw,
   Waw, Waw, Waw, Waw,
   Yeh, YehWithTail, Yeh, Waw,

   Yeh, Yeh, YehBarre, YehBarre,
   ArabicNone, TehMarbuta, Transparent, Transparent,
   Transparent, Transparent, Transparent, Transparent,
   Transparent, ArabicNone, ArabicNone, Transparent,

   Transparent, Transparent, Transparent, Transparent,
   Transparent, ArabicNone, ArabicNone, Transparent,
   Transparent, ArabicNone, Transparent, Transparent,
   Transparent, Transparent, Dal, Reh,

   ArabicNone, ArabicNone, ArabicNone, ArabicNone,
   ArabicNone, ArabicNone, ArabicNone, ArabicNone,
   ArabicNone, ArabicNone, Seen, Sad,
   Ain, ArabicNone, ArabicNone, KnottedHeh,

   // 0x700
   ArabicNone, ArabicNone, ArabicNone, ArabicNone,
   ArabicNone, ArabicNone, ArabicNone, ArabicNone,
   ArabicNone, ArabicNone, ArabicNone, ArabicNone,
   ArabicNone, ArabicNone, ArabicNone, ArabicNone,

   Alaph, Transparent, Beth, Gamal,
   Gamal, Dalath, Dalath, He,
   SyriacWaw, Zain, Heth, Teth,
   Teth, Yudh, YudhHe, Kaph,

   Lamadh, Mim, Nun, Semakh,
   FinalSemakh, SyriacE, Pe, ReversedPe,
   Sadhe, Qaph, Dalath, Shin,
   Taw, Beth, Gamal, Dalath,

   Transparent, Transparent, Transparent, Transparent,
   Transparent, Transparent, Transparent, Transparent,
   Transparent, Transparent, Transparent, Transparent,
   Transparent, Transparent, Transparent, Transparent,

   Transparent, Transparent, Transparent, Transparent,
   Transparent, Transparent, Transparent, Transparent,
   Transparent, Transparent, Transparent, ArabicNone,
   ArabicNone, Zain, Kaph, Fe,
};

static inline ArabicGroup arabicGroup(QChar uc)
{
   if (uc >= 0x0600 && uc < 0x750) {
      return (ArabicGroup) arabic_group[uc.unicode() - 0x600];

   } else if (uc == 0x200d) {
      return Center;

   } else if (uc.category() == QChar::Separator_Space) {
      return ArabicSpace;

   } else {
      return ArabicNone;
   }
}


/*
   Arabic shaping obeys a number of rules according to the joining classes (see Unicode book, section on
   arabic).

   Each unicode char has a joining class (right, dual (left&right), center (joincausing) or transparent).
   transparent joining is not encoded in QChar::joining(), but applies to all combining marks and format marks.

   Right join-causing: dual + center
   Left join-causing: dual + right + center

   Rules are as follows (for a string already in visual order, as we have it here):

   R1 Transparent characters do not affect joining behaviour.
   R2 A right joining character, that has a right join-causing char on the right will get form XRight
   (R3 A left joining character, that has a left join-causing char on the left will get form XLeft)
   Note: the above rule is meaningless, as there are no pure left joining characters defined in Unicode
   R4 A dual joining character, that has a left join-causing char on the left and a right join-causing char on
             the right will get form XMedial
   R5  A dual joining character, that has a right join causing char on the right, and no left join causing char on the left
         will get form XRight
   R6 A dual joining character, that has a  left join causing char on the left, and no right join causing char on the right
         will get form XLeft
   R7 Otherwise the character will get form XIsolated

   Additionally we have to do the minimal ligature support for lam-alef ligatures:

   L1 Transparent characters do not affect ligature behaviour.
   L2 Any sequence of Alef(XRight) + Lam(XMedial) will form the ligature Alef.Lam(XLeft)
   L3 Any sequence of Alef(XRight) + Lam(XLeft) will form the ligature Alef.Lam(XIsolated)

   The state table below handles rules R1-R7.
*/

enum Joining {
   JNone,
   JCausing,
   JDual,
   JRight,
   JTransparent
};

static const Joining joining_for_group[ArabicGroupsEnd] = {
   // NonJoining
   JNone, // ArabicNone
   JNone, // ArabicSpace
   // Transparent
   JTransparent, // Transparent
   // Causing
   JCausing, // Center
   JCausing, // Kashida
   // Dual
   JDual, // Beh
   JDual, // Noon
   JDual, // Yeh
   JDual, // Hah
   JDual, // Seen
   JDual, // Tah
   JDual, // Ain
   // Right
   JRight, // Alef
   JRight, // Waw
   JRight, // Dal
   JRight, // Reh
   JRight  // HamzaOnHehGoal
};


struct JoiningPair {
   QArabicShape form1;
   QArabicShape form2;
};

static const JoiningPair joining_table[5][4] =
   // None, Causing, Dual, Right
{
   { { XIsolated, XIsolated }, { XIsolated, XCausing }, { XIsolated, XInitial }, { XIsolated, XIsolated } }, // XIsolated
   { { XFinal, XIsolated }, { XFinal, XCausing }, { XFinal, XInitial }, { XFinal, XIsolated } }, // XFinal
   { { XIsolated, XIsolated }, { XInitial, XCausing }, { XInitial, XMedial }, { XInitial, XFinal } }, // XInitial
   { { XFinal, XIsolated }, { XMedial, XCausing }, { XMedial, XMedial }, { XMedial, XFinal } }, // XMedial
   { { XIsolated, XIsolated }, { XIsolated, XCausing }, { XIsolated, XMedial }, { XIsolated, XFinal } }, // XCausing
};


/*
According to http://www.microsoft.com/middleeast/Arabicdev/IE6/KBase.asp

1. Find the priority of the connecting opportunities in each word
2. Add expansion at the highest priority connection opportunity
3. If more than one connection opportunity have the same highest value,
   use the opportunity closest to the end of the word.

Following is a chart that provides the priority for connection
opportunities and where expansion occurs. The character group names
are those in table 6.6 of the UNICODE 2.0 book.


PrioritY        Glyph                   Condition                                       Kashida Location

Arabic_Kashida    User inserted Kashida   The user entered a Kashida in a position.       After the user
                  (Shift+j or Shift+[E with hat])    Thus, it is the highest priority to insert an   inserted kashida
                                        automatic kashida.

Arabic_Seen       Seen, Sad               Connecting to the next character.            After the character.
                                        (Initial or medial form).

Arabic_HaaDal     Teh Marbutah, Haa, Dal  Connecting to previous character.          Before the final form
                                                                                     of these characters.

Arabic_Alef       Alef, Tah, Lam,         Connecting to previous character.          Before the final form
                  Kaf and Gaf                                                        of these characters.

Arabic_BaRa       Reh, Yeh                Connected to medial Beh                    Before preceding medial Baa

Arabic_Waw        Waw, Ain, Qaf, Feh      Connecting to previous character.          Before the final form of
                                                                                     these characters.

Arabic_Normal     Other connecting        Connecting to previous character.          Before the final form
                  characters                                                         of these characters.

This seems to imply that we have at most one kashida point per arabic word.

*/

void qt_getArabicProperties(QStringView str, int len, QArabicProperties *properties)
{
   int lastPos = 0;
   int lastGroup = ArabicNone;

   ArabicGroup group  = arabicGroup(str[0]);
   Joining j          = joining_for_group[group];
   QArabicShape shape = joining_table[XIsolated][j].form2;

   properties[0].justification = HB_NoJustification;

   for (int i = 1; i < len; ++i) {
      // #### fix handling for spaces and punctuation
      properties[i].justification = HB_NoJustification;

      group = arabicGroup(str[i]);
      j = joining_for_group[group];

      if (j == JTransparent) {
         properties[i].shape = XIsolated;
         continue;
      }

      properties[lastPos].shape = joining_table[shape][j].form1;
      shape = joining_table[shape][j].form2;

      switch (lastGroup) {
         case Seen:
            if (properties[lastPos].shape == XInitial || properties[lastPos].shape == XMedial) {
               properties[i - 1].justification = HB_Arabic_Seen;
            }
            break;
         case Hah:
            if (properties[lastPos].shape == XFinal) {
               properties[lastPos - 1].justification = HB_Arabic_HaaDal;
            }
            break;
         case Alef:
            if (properties[lastPos].shape == XFinal) {
               properties[lastPos - 1].justification = HB_Arabic_Alef;
            }
            break;
         case Ain:
            if (properties[lastPos].shape == XFinal) {
               properties[lastPos - 1].justification = HB_Arabic_Waw;
            }
            break;
         case Noon:
            if (properties[lastPos].shape == XFinal) {
               properties[lastPos - 1].justification = HB_Arabic_Normal;
            }
            break;
         case ArabicNone:
            break;

         default:
            Q_ASSERT(false);
      }

      lastGroup = ArabicNone;

      switch (group) {
         case ArabicNone:
         case Transparent:
         // ### Center should probably be treated as transparent when it comes to justification.
         case Center:
            break;
         case ArabicSpace:
            properties[i].justification = HB_Arabic_Space;
            break;
         case Kashida:
            properties[i].justification = HB_Arabic_Kashida;
            break;
         case Seen:
            lastGroup = Seen;
            break;

         case Hah:
         case Dal:
            lastGroup = Hah;
            break;

         case Alef:
         case Tah:
            lastGroup = Alef;
            break;

         case Yeh:
         case Reh:
            if (properties[lastPos].shape == XMedial && arabicGroup(str[lastPos]) == Beh) {
               properties[lastPos - 1].justification = HB_Arabic_BaRa;
            }
            break;

         case Ain:
         case Waw:
            lastGroup = Ain;
            break;

         case Noon:
         case Beh:
         case HamzaOnHehGoal:
            lastGroup = Noon;
            break;
         case ArabicGroupsEnd:
            Q_ASSERT(false);
      }

      lastPos = i;
   }

   properties[lastPos].shape = joining_table[shape][JNone].form1;
}

void QTextEngine::shapeTextMac(int item) const
{
   QScriptItem &si = layoutData->items[item];

   si.glyph_data_offset = layoutData->used;

   QFontEngine *font = fontEngine(si, &si.ascent, &si.descent, &si.leading);
   if (font->type() != QFontEngine::Multi) {
      shapeTextWithHarfbuzz(item);
      return;
   }

   QCoreTextFontEngineMulti *fe = static_cast<QCoreTextFontEngineMulti *>(font);

   QTextEngine::ShaperFlags flags;
   if (si.analysis.bidiLevel % 2) {
      flags |= RightToLeft;
   }
   if (option.useDesignMetrics()) {
      flags |= DesignMetrics;
   }

   attributes(); // pre-initialize char attributes

   const int len  = length(item);
   int num_glyphs = length(item);

   QStringView str = layoutData->string.midView(si.position, len);
   QString upperCased;

   if (si.analysis.flags == QScriptAnalysis::SmallCaps || si.analysis.flags == QScriptAnalysis::Uppercase
         || si.analysis.flags == QScriptAnalysis::Lowercase) {

      if (si.analysis.flags == QScriptAnalysis::Lowercase) {
         upperCased = str.toLower();

      } else {
         upperCased = str.toUpper();

      }

      str = upperCased;
   }

   ensureSpace(num_glyphs);
   num_glyphs = layoutData->glyphLayout.numGlyphs - layoutData->used;

   QGlyphLayout g = availableGlyphs(&si);
   g.numGlyphs    = num_glyphs;
   unsigned short *log_clusters = logClusters(&si);

   bool stringToCMapFailed = false;

   if (! fe->stringToCMap(str, &g, &num_glyphs, flags, log_clusters, attributes(), &si)) {
      ensureSpace(num_glyphs);
      g = availableGlyphs(&si);

      stringToCMapFailed = ! fe->stringToCMap(str, &g, &num_glyphs, flags, log_clusters, attributes(), &si);
   }

   if (! stringToCMapFailed) {
      heuristicSetGlyphAttributes(str, &g, log_clusters, num_glyphs);

      si.num_glyphs = num_glyphs;
      layoutData->used += si.num_glyphs;

      QGlyphLayout g = shapedGlyphs(&si);

      if (si.analysis.script == QChar::Script_Arabic) {
         QVarLengthArray<QArabicProperties> props(len + 2);
         QArabicProperties *properties = props.data();

         int f = si.position;
         int l = len;
         if (f > 0) {
            --f;
            ++l;
            ++properties;
         }

         if (f + l < layoutData->string.length()) {
            ++l;
         }

         qt_getArabicProperties(layoutData->string.midView(f), l, props.data());

         unsigned short *log_clusters = logClusters(&si);

         for (int i = 0; i < len; ++i) {
            int gpos = log_clusters[i];
            g.attributes[gpos].justification = properties[i].justification;
         }
      }
   }
}

