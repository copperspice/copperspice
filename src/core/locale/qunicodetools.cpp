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

#include <qunicodetools_p.h>
#include <qunicodetables_p.h>

#include <qfile.h>

#define FLAG(x) (1 << (x))

namespace QUnicodeTools {

namespace GB {

enum State : uchar {
    Break,
    NoBr,
    GB10,
    GB11,
    GB12,
    GB13
};

static const State breakTable[QUnicodeTables::NumGraphemeBreakClasses][QUnicodeTables::NumGraphemeBreakClasses] = {
//    Any     CR      LF   Control Extend  ZWJ    RI  Prepend S-Mark   L       V      T     LV    LVT   E_B     E_M    GAZ    EBG
    { Break, Break, Break, Break, NoBr,  NoBr,  Break, Break, NoBr,  Break, Break, Break, Break, Break, Break, Break, Break, Break }, // Any
    { Break, Break, NoBr,  Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break }, // CR
    { Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break }, // LF
    { Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break }, // Control
    { Break, Break, Break, Break, GB11,  NoBr,  Break, Break, NoBr,  Break, Break, Break, Break, Break, Break, GB12,  Break, Break }, // Extend
    { Break, Break, Break, Break, NoBr,  NoBr,  Break, Break, NoBr,  Break, Break, Break, Break, Break, Break, Break, NoBr,  NoBr  }, // ZWJ
    { Break, Break, Break, Break, NoBr,  NoBr,  GB13,  Break, NoBr,  Break, Break, Break, Break, Break, Break, Break, Break, Break }, // RegionalIndicator
    { NoBr,  Break, Break, Break, NoBr,  NoBr,  NoBr,  NoBr,  NoBr,  NoBr,  NoBr,  NoBr,  NoBr,  NoBr,  NoBr,  NoBr,  NoBr,  NoBr  }, // Prepend
    { Break, Break, Break, Break, NoBr,  NoBr,  Break, Break, NoBr,  Break, Break, Break, Break, Break, Break, Break, Break, Break }, // SpacingMark
    { Break, Break, Break, Break, NoBr,  NoBr,  Break, Break, NoBr,  NoBr,  NoBr,  Break, NoBr,  NoBr,  Break, Break, Break, Break }, // L
    { Break, Break, Break, Break, NoBr,  NoBr,  Break, Break, NoBr,  Break, NoBr,  NoBr,  Break, Break, Break, Break, Break, Break }, // V
    { Break, Break, Break, Break, NoBr,  NoBr,  Break, Break, NoBr,  Break, Break, NoBr,  Break, Break, Break, Break, Break, Break }, // T
    { Break, Break, Break, Break, NoBr,  NoBr,  Break, Break, NoBr,  Break, NoBr,  NoBr,  Break, Break, Break, Break, Break, Break }, // LV
    { Break, Break, Break, Break, NoBr,  NoBr,  Break, Break, NoBr,  Break, Break, NoBr,  Break, Break, Break, Break, Break, Break }, // LVT
    { Break, Break, Break, Break, GB10,  NoBr,  Break, Break, NoBr,  Break, Break, Break, Break, Break, Break, NoBr,  Break, Break }, // E_B
    { Break, Break, Break, Break, NoBr,  NoBr,  Break, Break, NoBr,  Break, Break, Break, Break, Break, Break, Break, Break, Break }, // E_M
    { Break, Break, Break, Break, NoBr,  NoBr,  Break, Break, NoBr,  Break, Break, Break, Break, Break, Break, Break, Break, Break }, // GAZ
    { Break, Break, Break, Break, GB10,  NoBr,  Break, Break, NoBr,  Break, Break, Break, Break, Break, Break, NoBr,  Break, Break }, // EBG
};

} // namespace

static void getGraphemeBreaks(const QString &str, QCharAttributes *attributes)
{
   // satifies GB1
   QUnicodeTables::GraphemeBreakClass last_bClass = QUnicodeTables::GraphemeBreak_LF;
   GB::State state = GB::Break;

   int k = 0;

   for (QChar ch : str) {
      const QUnicodeTables::Properties *prop = QUnicodeTables::properties(ch.unicode());
      QUnicodeTables::GraphemeBreakClass cur_bClass = (QUnicodeTables::GraphemeBreakClass) prop->graphemeBreakClass;

      GB::State tmpState = GB::breakTable[last_bClass][cur_bClass];

      if (tmpState == GB::Break) {
         attributes[k].graphemeBoundary = true;
         state = GB::Break;

      } else if (tmpState == GB::NoBr) {
         state = GB::Break;

      } else if (tmpState == GB::GB10) {
         state = GB::GB10;

      } else if (tmpState == GB::GB11) {

         if (state == GB::GB10 || state == GB::GB11) {
            state = GB::GB11;
         } else {
            state = GB::Break;
         }

      } else if (tmpState == GB::GB12) {

         if (state != GB::GB10 && state != GB::GB11) {
            attributes[k].graphemeBoundary = true;
         }

         state = GB::Break;

      } else if (tmpState == GB::GB13) {

         if (state != GB::GB13) {
            state = GB::GB13;

         } else {
            attributes[k].graphemeBoundary = true;
            state = GB::Break;
         }
      }

      last_bClass =  cur_bClass;
      ++k;
   }

   // GB2
   attributes[k].graphemeBoundary = true;
}


namespace WB {

enum Action {
    NoBr,
    Break,
    Scan,
    Word
};

static const Action breakTable[QUnicodeTables::NumWordBreakClasses][QUnicodeTables::NumWordBreakClasses] = {
//    Any      CR     LF   Newline Extend ZWJ   Format  RI    K     Hebrew   AL   SQuote DQuote  MidN   MidL   MidN  Num     Ext   E_Base E_Mod  GAZ    EBG     WSeg
   { Break, Break, Break, Break, NoBr,  NoBr,  NoBr,  Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break }, // Any
   { Break, Break, NoBr,  Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break }, // CR
   { Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break }, // LF
   { Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break }, // Newline
   { Break, Break, Break, Break, NoBr,  NoBr,  NoBr,  Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break }, // Extend
   { Break, Break, Break, Break, NoBr,  NoBr,  NoBr,  Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, NoBr,  NoBr,  Break }, // ZWJ
   { Break, Break, Break, Break, NoBr,  NoBr,  NoBr,  Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break }, // Format
   { Break, Break, Break, Break, NoBr,  NoBr,  NoBr,  NoBr,  Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break }, // Reg Ind
   { Break, Break, Break, Break, NoBr,  NoBr,  NoBr,  Break, NoBr,  Break, Break, Break, Break, Break, Break, Break, Break, NoBr,  Break, Break, Break, Break, Break }, // Katakana
   { Break, Break, Break, Break, NoBr,  NoBr,  NoBr,  Break, Break, NoBr,  NoBr,  Word,  Scan , Word,  Word,  Break, NoBr,  NoBr,  Break, Break, Break, Break, Break }, // HebrewL
   { Break, Break, Break, Break, NoBr,  NoBr,  NoBr,  Break, Break, NoBr,  NoBr,  Word,  Break, Word,  Word,  Break, NoBr,  NoBr,  Break, Break, Break, Break, Break }, // ALetter
   { Break, Break, Break, Break, NoBr,  NoBr,  NoBr,  Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break }, // SQuote
   { Break, Break, Break, Break, NoBr,  NoBr,  NoBr,  Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break }, // DQuote
   { Break, Break, Break, Break, NoBr,  NoBr,  NoBr,  Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break }, // MidNumLet
   { Break, Break, Break, Break, NoBr,  NoBr,  NoBr,  Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break }, // MidLetter
   { Break, Break, Break, Break, NoBr,  NoBr,  NoBr,  Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break }, // MidNum
   { Break, Break, Break, Break, NoBr,  NoBr,  NoBr,  Break, Break, NoBr,  NoBr,  Scan , Break, Scan , Break, Scan , NoBr,  NoBr,  Break, Break, Break, Break, Break }, // Numeric
   { Break, Break, Break, Break, NoBr,  NoBr,  NoBr,  Break, NoBr,  NoBr,  NoBr,  Break, Break, Break, Break, Break, NoBr,  NoBr,  Break, Break, Break, Break, Break }, // Ext
   { Break, Break, Break, Break, NoBr,  NoBr,  NoBr,  Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, NoBr,  Break, Break, Break }, // E_Base
   { Break, Break, Break, Break, NoBr,  NoBr,  NoBr,  Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break }, // E_Mod
   { Break, Break, Break, Break, NoBr,  NoBr,  NoBr,  Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break }, // GAZ
   { Break, Break, Break, Break, NoBr,  NoBr,  NoBr,  Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, NoBr,  Break, Break, Break }, // EBG
   { Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, Break, NoBr  }, // WSeg
};

} // namespace

static void getWordBreaks(const QString &str, QCharAttributes *attributes)
{
   enum WordType {
     WordType_None,
     WordType_AlphaNumeric,
     WordType_HiraganaKatakana
   };

   WordType currentWordType = WordType::WordType_None;

   // satifies WB1
   QUnicodeTables::WordBreakClass last_bClass = QUnicodeTables::WordBreak_LF;

   int k = 0;

   for (auto iter = str.begin(); iter != str.end(); ++iter, ++k) {

      int   pos = k;
      QChar ch  = *iter;

      const QUnicodeTables::Properties *prop    = QUnicodeTables::properties(ch.unicode());
      QUnicodeTables::WordBreakClass cur_bClass = (QUnicodeTables::WordBreakClass) prop->wordBreakClass;

      WB::Action tmpState = WB::breakTable[last_bClass][cur_bClass];

      if (tmpState == WB::Break) {
         // nothing

      } else if (tmpState == WB::NoBr) {

         if (cur_bClass == QUnicodeTables::WordBreak_Extend || cur_bClass == QUnicodeTables::WordBreak_ZWJ || cur_bClass == QUnicodeTables::WordBreak_Format) {
            // WB4: X(Extend|Format)* -> X

            if (last_bClass != QUnicodeTables::WordBreak_ZWJ)  {
               // WB3c
               continue;
            }
         }

         if (last_bClass == QUnicodeTables::WordBreak_RegionalIndicator) {
            // WB15/WB16: break between pairs of Regional indicator
            cur_bClass = QUnicodeTables::WordBreak_Any;
         }

      } else if (tmpState == WB::Scan || tmpState == WB::Word) {

         int tmp_k = k + 1;

         for (auto tmp_iter = iter + 1; tmp_iter != str.end(); ++tmp_iter, ++tmp_k) {
            ch = *tmp_iter;

            prop = QUnicodeTables::properties(ch.unicode());
            QUnicodeTables::WordBreakClass t_bClass = (QUnicodeTables::WordBreakClass) prop->wordBreakClass;

            if (t_bClass == QUnicodeTables::WordBreak_Extend || t_bClass == QUnicodeTables::WordBreak_ZWJ || t_bClass == QUnicodeTables::WordBreak_Format) {
               // WB4: X(Extend|Format)* -> X
               continue;
            }

            if (t_bClass == last_bClass || (tmpState == WB::Word && (t_bClass == QUnicodeTables::WordBreak_HebrewLetter || t_bClass == QUnicodeTables::WordBreak_ALetter))) {
               k    = tmp_k;
               iter = tmp_iter;

               cur_bClass = t_bClass;
               tmpState   = WB::NoBr;
            }

            break;
         }

         if (tmpState != WB::NoBr) {
            tmpState = WB::Break;

            if (cur_bClass == QUnicodeTables::WordBreak_SingleQuote && last_bClass == QUnicodeTables::WordBreak_HebrewLetter) {
               tmpState = WB::NoBr;    // WB7a
            }
         }
      }

      last_bClass = cur_bClass;

      if (tmpState == WB::Break) {
         attributes[pos].wordBreak = true;

         if (currentWordType != WordType_None) {
            attributes[pos].wordEnd = true;
         }

         switch (last_bClass) {
            case QUnicodeTables::WordBreak_Katakana:
               currentWordType = WordType_HiraganaKatakana;
               attributes[pos].wordStart = true;
               break;

            case QUnicodeTables::WordBreak_HebrewLetter:
            case QUnicodeTables::WordBreak_ALetter:
            case QUnicodeTables::WordBreak_Numeric:
               currentWordType = WordType_AlphaNumeric;
               attributes[pos].wordStart = true;
               break;

            default:
               currentWordType = WordType_None;
               break;
         }
      }
   }

   if (currentWordType != WordType_None) {
      attributes[k].wordEnd = true;
   }

   attributes[k].wordBreak = true; // WB2
}


namespace SB {

enum Action {
    Initial,
    Lower,
    Upper,
    LUATerm,
    ATerm,
    ATermC,
    ACS,
    STerm,
    STermC,
    SCS,
    BAfterC,
    BAfter,
    Break,
    Lookup
};

static const Action breakTable[BAfter + 1][QUnicodeTables::NumSentenceBreakClasses] = {
//    Any      CR       LF       Extend    Sep   Format  Sp      Lower    Upper    OLetter  Numeric  ATerm    SContinue STerm    Close
    { Initial, BAfterC, BAfter, Initial, BAfter, Initial,  Initial,  Lower,   Upper  , Initial, Initial, ATerm  , Initial, STerm  , Initial }, // Initial
    { Initial, BAfterC, BAfter, Lower,   BAfter, Lower,    Initial,  Initial, Initial, Initial, Initial, LUATerm, Initial, STerm  , Initial }, // Lower
    { Initial, BAfterC, BAfter, Upper,   BAfter, Upper,    Initial,  Initial, Upper  , Initial, Initial, LUATerm, STerm  , STerm  , Initial }, // Upper

    { Lookup , BAfterC, BAfter, LUATerm, BAfter, LUATerm,  ACS,      Initial, Upper  , Break  , Initial, ATerm  , STerm  , STerm  , ATermC  }, // LUATerm
    { Lookup , BAfterC, BAfter, ATerm,   BAfter, ATerm,    ACS,      Initial, Break  , Break  , Initial, ATerm  , STerm  , STerm  , ATermC  }, // ATerm
    { Lookup , BAfterC, BAfter, ATermC,  BAfter, ATermC,   ACS,      Initial, Break  , Break  , Lookup , ATerm  , STerm  , STerm  , ATermC  }, // ATermC
    { Lookup , BAfterC, BAfter, ACS,     BAfter, ACS,      ACS,      Initial, Break  , Break  , Lookup , ATerm  , STerm  , STerm  , Lookup  }, // ACS

    { Break  , BAfterC, BAfter, STerm,   BAfter, STerm,    SCS,      Break  , Break  , Break  , Break  , ATerm  , STerm  , STerm  , STermC  }, // STerm
    { Break  , BAfterC, BAfter, STermC,  BAfter, STermC,   SCS,      Break  , Break  , Break  , Break  , ATerm  , STerm  , STerm  , STermC  }, // STermC
    { Break  , BAfterC, BAfter, SCS,     BAfter, SCS,      SCS,      Break  , Break  , Break  , Break  , ATerm  , STerm  , STerm  , Break   }, // SCS
    { Break  , Break  , BAfter, Break,   Break , Break,    Break,    Break  , Break  , Break  , Break  , Break  , Break  , Break  , Break   }, // BAfterC
    { Break  , Break  , Break , Break,   Break , Break,    Break,    Break  , Break  , Break  , Break  , Break  , Break  , Break  , Break   }, // BAfter
};


} // namespace

static void getSentenceBreaks(const QString &str, QCharAttributes *attributes)
{
   // satisfies SB1
   SB::Action state = SB::BAfter;

   int k = 0;

   for (auto iter = str.begin(); iter != str.end(); ++iter, ++k) {
      int pos  = k;
      QChar ch = *iter;

      const QUnicodeTables::Properties *prop = QUnicodeTables::properties(ch.unicode());
      QUnicodeTables::SentenceBreakClass cur_bClass = (QUnicodeTables::SentenceBreakClass) prop->sentenceBreakClass;

      Q_ASSERT(state <= SB::BAfter);
      state = SB::breakTable[state][cur_bClass];

      if (state == SB::Lookup) {
         // SB8
         state = SB::Break;

         int tmp_k = k + 1;

         for (auto tmp_iter = iter + 1; tmp_iter != str.end(); ++tmp_iter, ++tmp_k) {
            ch = *tmp_iter;

            prop = QUnicodeTables::properties(ch.unicode());
            QUnicodeTables::SentenceBreakClass t_bClass = (QUnicodeTables::SentenceBreakClass) prop->sentenceBreakClass;

            switch (t_bClass) {
               case QUnicodeTables::SentenceBreak_Any:
               case QUnicodeTables::SentenceBreak_Extend:
               case QUnicodeTables::SentenceBreak_Sp:
               case QUnicodeTables::SentenceBreak_Numeric:
               case QUnicodeTables::SentenceBreak_SContinue:
               case QUnicodeTables::SentenceBreak_Close:
                  continue;

               case QUnicodeTables::SentenceBreak_Lower:
                  k    = tmp_k;
                  iter = tmp_iter;

                  state = SB::Initial;
                  break;

               default:
                  break;
             }

            break;
         }
      }

      if (state == SB::Break) {
         attributes[pos].sentenceBoundary = true;
         state = SB::breakTable[SB::Initial][cur_bClass];
      }
   }

   attributes[k].sentenceBoundary = true; // SB2
}

namespace LB {

// LB25 recommends to not break lines inside numbers of the form
// described by the following regular expression:   (PR|PO)?(OP|HY)?NU(NU|SY|IS)*(CL|CP)?(PR|PO)?

enum NumAction {
    None,
    Start,
    Continue,
    Break
};

enum NumClass {
    XX,
    PRPO,
    OPHY,
    NU,
    SYIS,
    CLCP
};

static const NumAction actionTable[CLCP + 1][CLCP + 1] = {
//     XX      PRPO      OPHY       NU       SYIS      CLCP
    { None,  Start,    Start,    Start,    None,     None     }, // XX
    { None,  Start,    Continue, Continue, None,     None     }, // PRPO
    { None,  Start,    Start,    Continue, None,     None     }, // OPHY
    { Break, Break,    Break,    Continue, Continue, Continue }, // NU
    { Break, Break,    Break,    Continue, Continue, Continue }, // SYIS
    { Break, Continue, Break,    Break,    Break,    Break    }, // CLCP
};

inline NumClass toClass(QUnicodeTables::LineBreakClass lbc, QChar::Category category)
{
   switch (lbc) {
      case QUnicodeTables::LineBreak_AL:
         if (category == QChar::Symbol_Math) {
            return NumClass::SYIS;
         }
         break;

      case QUnicodeTables::LineBreak_PR:
      case QUnicodeTables::LineBreak_PO:
         return NumClass::PRPO;

      case QUnicodeTables::LineBreak_OP:
      case QUnicodeTables::LineBreak_HY:
         return NumClass::OPHY;

      case QUnicodeTables::LineBreak_NU:
         return NumClass::NU;

      case QUnicodeTables::LineBreak_SY:
      case QUnicodeTables::LineBreak_IS:
         return NumClass::SYIS;

      case QUnicodeTables::LineBreak_CL:
      case QUnicodeTables::LineBreak_CP:
         return NumClass::CLCP;

      default:
         break;
   }

   return NumClass::XX;
}

enum Action {
    ProhibitedBreak,                      PB = ProhibitedBreak,
    DirectBreak,                          DB = DirectBreak,
    IndirectBreak,                        IB = IndirectBreak,
    CombiningIndirectBreak,               CI = CombiningIndirectBreak,
    CombiningProhibitedBreak,             CP = CombiningProhibitedBreak,
    ProhibitedBreakAfterHebrewPlusHyphen, HH = ProhibitedBreakAfterHebrewPlusHyphen
};

static const Action breakTable[QUnicodeTables::LineBreak_CB + 1][QUnicodeTables::LineBreak_CB + 1] = {
/*         OP  CL  CP  QU  GL  NS  EX  SY  IS  PR  PO  NU  AL  HL  ID  IN  HY  BA  BB  B2  ZW  CM  WJ  H2  H3  JL  JV  JT  RI  CB */
/* OP */ { PB, PB, PB, PB, PB, PB, PB, PB, PB, PB, PB, PB, PB, PB, PB, PB, PB, PB, PB, PB, PB, CP, PB, PB, PB, PB, PB, PB, PB, PB },
/* CL */ { DB, PB, PB, IB, IB, PB, PB, PB, PB, DB, DB, DB, DB, DB, DB, DB, IB, IB, DB, DB, PB, CI, PB, DB, DB, DB, DB, DB, DB, DB },
/* CP */ { DB, PB, PB, IB, IB, PB, PB, PB, PB, DB, DB, IB, IB, IB, DB, DB, IB, IB, DB, DB, PB, CI, PB, DB, DB, DB, DB, DB, DB, DB },
/* QU */ { PB, PB, PB, IB, IB, IB, PB, PB, PB, IB, IB, IB, IB, IB, IB, IB, IB, IB, IB, IB, PB, CI, PB, IB, IB, IB, IB, IB, IB, IB },
/* GL */ { IB, PB, PB, IB, IB, IB, PB, PB, PB, IB, IB, IB, IB, IB, IB, IB, IB, IB, IB, IB, PB, CI, PB, IB, IB, IB, IB, IB, IB, IB },
/* NS */ { DB, PB, PB, IB, IB, IB, PB, PB, PB, DB, DB, DB, DB, DB, DB, DB, IB, IB, DB, DB, PB, CI, PB, DB, DB, DB, DB, DB, DB, DB },
/* EX */ { DB, PB, PB, IB, IB, IB, PB, PB, PB, DB, DB, DB, DB, DB, DB, IB, IB, IB, DB, DB, PB, CI, PB, DB, DB, DB, DB, DB, DB, DB },
/* SY */ { DB, PB, PB, IB, IB, IB, PB, PB, PB, DB, DB, DB, DB, IB, DB, DB, IB, IB, DB, DB, PB, CI, PB, DB, DB, DB, DB, DB, DB, DB },
/* IS */ { DB, PB, PB, IB, IB, IB, PB, PB, PB, DB, DB, DB, IB, IB, DB, DB, IB, IB, DB, DB, PB, CI, PB, DB, DB, DB, DB, DB, DB, DB },
/* PR */ { DB, PB, PB, IB, IB, IB, PB, PB, PB, DB, DB, IB, IB, IB, IB, DB, IB, IB, DB, DB, PB, CI, PB, IB, IB, IB, IB, IB, DB, DB },
/* PO */ { DB, PB, PB, IB, IB, IB, PB, PB, PB, DB, DB, IB, IB, IB, DB, DB, IB, IB, DB, DB, PB, CI, PB, DB, DB, DB, DB, DB, DB, DB },
/* NU */ { IB, PB, PB, IB, IB, IB, PB, PB, PB, IB, IB, IB, IB, IB, DB, IB, IB, IB, DB, DB, PB, CI, PB, DB, DB, DB, DB, DB, DB, DB },
/* AL */ { IB, PB, PB, IB, IB, IB, PB, PB, PB, DB, DB, IB, IB, IB, DB, IB, IB, IB, DB, DB, PB, CI, PB, DB, DB, DB, DB, DB, DB, DB },
/* HL */ { IB, PB, PB, IB, IB, IB, PB, PB, PB, DB, DB, IB, IB, IB, DB, IB, CI, CI, DB, DB, PB, CI, PB, DB, DB, DB, DB, DB, DB, DB },
/* ID */ { DB, PB, PB, IB, IB, IB, PB, PB, PB, DB, IB, DB, DB, DB, DB, IB, IB, IB, DB, DB, PB, CI, PB, DB, DB, DB, DB, DB, DB, DB },
/* IN */ { DB, PB, PB, IB, IB, IB, PB, PB, PB, DB, DB, DB, DB, DB, DB, IB, IB, IB, DB, DB, PB, CI, PB, DB, DB, DB, DB, DB, DB, DB },
/* HY */ { HH, PB, PB, IB, HH, IB, PB, PB, PB, HH, HH, IB, HH, HH, HH, HH, IB, IB, HH, HH, PB, CI, PB, HH, HH, HH, HH, HH, HH, DB },
/* BA */ { HH, PB, PB, IB, HH, IB, PB, PB, PB, HH, HH, HH, HH, HH, HH, HH, IB, IB, HH, HH, PB, CI, PB, HH, HH, HH, HH, HH, HH, DB },
/* BB */ { IB, PB, PB, IB, IB, IB, PB, PB, PB, IB, IB, IB, IB, IB, IB, IB, IB, IB, IB, IB, PB, CI, PB, IB, IB, IB, IB, IB, IB, DB },
/* B2 */ { DB, PB, PB, IB, IB, IB, PB, PB, PB, DB, DB, DB, DB, DB, DB, DB, IB, IB, DB, PB, PB, CI, PB, DB, DB, DB, DB, DB, DB, DB },
/* ZW */ { DB, DB, DB, DB, DB, DB, DB, DB, DB, DB, DB, DB, DB, DB, DB, DB, DB, DB, DB, DB, PB, DB, DB, DB, DB, DB, DB, DB, DB, DB },
/* CM */ { IB, PB, PB, IB, IB, IB, PB, PB, PB, DB, DB, IB, IB, IB, DB, IB, IB, IB, DB, DB, PB, CI, PB, DB, DB, DB, DB, DB, DB, DB },
/* WJ */ { IB, PB, PB, IB, IB, IB, PB, PB, PB, IB, IB, IB, IB, IB, IB, IB, IB, IB, IB, IB, PB, CI, PB, IB, IB, IB, IB, IB, IB, IB },
/* H2 */ { DB, PB, PB, IB, IB, IB, PB, PB, PB, DB, IB, DB, DB, DB, DB, IB, IB, IB, DB, DB, PB, CI, PB, DB, DB, DB, IB, IB, DB, DB },
/* H3 */ { DB, PB, PB, IB, IB, IB, PB, PB, PB, DB, IB, DB, DB, DB, DB, IB, IB, IB, DB, DB, PB, CI, PB, DB, DB, DB, DB, IB, DB, DB },
/* JL */ { DB, PB, PB, IB, IB, IB, PB, PB, PB, DB, IB, DB, DB, DB, DB, IB, IB, IB, DB, DB, PB, CI, PB, IB, IB, IB, IB, DB, DB, DB },
/* JV */ { DB, PB, PB, IB, IB, IB, PB, PB, PB, DB, IB, DB, DB, DB, DB, IB, IB, IB, DB, DB, PB, CI, PB, DB, DB, DB, IB, IB, DB, DB },
/* JT */ { DB, PB, PB, IB, IB, IB, PB, PB, PB, DB, IB, DB, DB, DB, DB, IB, IB, IB, DB, DB, PB, CI, PB, DB, DB, DB, DB, IB, DB, DB },
/* RI */ { DB, PB, PB, IB, IB, IB, PB, PB, PB, DB, DB, DB, DB, DB, DB, DB, IB, IB, DB, DB, PB, CI, PB, DB, DB, DB, DB, DB, IB, DB },
/* CB */ { DB, PB, PB, IB, IB, DB, PB, PB, PB, DB, DB, DB, DB, DB, DB, DB, DB, DB, DB, DB, PB, CI, PB, DB, DB, DB, DB, DB, DB, DB }
};

// The following line break classes are not treated by the pair table
// and must be resolved outside: AI, BK, CB, CJ, CR, LF, NL, SA, SG, SP, XX

} // namespace

static void getLineBreaks(const QString &str, QCharAttributes *attributes)
{
   int n_begin = 0;
   LB::NumClass n_row = LB::NumClass::XX;

   // to meet LB10
   QUnicodeTables::LineBreakClass last_bClass = QUnicodeTables::LineBreak_LF;
   QUnicodeTables::LineBreakClass x_bClass    = last_bClass;

   int k = 0;

   for (auto iter = str.begin(); iter != str.end(); ++iter, ++k) {
      QChar ch = *iter;

      const QUnicodeTables::Properties *prop    = QUnicodeTables::properties(ch.unicode());
      QUnicodeTables::LineBreakClass cur_bClass = (QUnicodeTables::LineBreakClass) prop->lineBreakClass;

      if (cur_bClass == QUnicodeTables::LineBreak_SA) {
         // LB1: resolve SA to AL, except of those that have Category Mn or Mc be resolved to CM
         static const int test = FLAG(QChar::Mark_NonSpacing) | FLAG(QChar::Mark_SpacingCombining);

         if (FLAG(prop->category) & test) {
            cur_bClass = QUnicodeTables::LineBreak_CM;
         }
      }

      if (cur_bClass == QUnicodeTables::LineBreak_CM) {
         // LB10: treat CM that follows SP, BK, CR, LF, NL, or ZW as AL
         if (last_bClass == QUnicodeTables::LineBreak_ZW || last_bClass >= QUnicodeTables::LineBreak_SP) {
            cur_bClass = QUnicodeTables::LineBreak_AL;
         }
      }

      if (cur_bClass != QUnicodeTables::LineBreak_CM) {
         // LB25: do not break lines inside numbers
         LB::NumClass n_col = LB::toClass(cur_bClass, (QChar::Category)prop->category);

         switch (LB::actionTable[n_row][n_col]) {

            case LB::NumAction::Break:
               // do not change breaks before and after the expression
               for (int j = n_begin + 1; j < k; ++j) {
                  attributes[j].lineBreak = false;
               }
               [[fallthrough]];

            case LB::NumAction::None:
               n_row = LB::NumClass::XX;    // reset state
               break;

            case LB::NumAction::Start:
               n_begin = k;
               [[fallthrough]];

            default:
               n_row = n_col;
               break;
        }
      }

      if (last_bClass >= QUnicodeTables::LineBreak_CR) {
         // LB4: BK!, LB5: (CRxLF|CR|LF|NL)!

         if (last_bClass > QUnicodeTables::LineBreak_CR || cur_bClass != QUnicodeTables::LineBreak_LF) {
            attributes[k].lineBreak      = true;
            attributes[k].mandatoryBreak = true;
         }

         x_bClass    = cur_bClass;
         last_bClass = cur_bClass;
         continue;
      }

      if (cur_bClass >= QUnicodeTables::LineBreak_SP) {
         if (cur_bClass > QUnicodeTables::LineBreak_SP) {
            // LB6: x(BK|CR|LF|NL)
            x_bClass = cur_bClass;
         }

         // LB7: xSP
         last_bClass = cur_bClass;
         continue;
      }

      // for South East Asian chars that require a complex analysis, the Unicode
      // standard recommends to treat them as AL. tailoring that do dictionary analysis can override

      if (x_bClass >= QUnicodeTables::LineBreak_SA) {
         x_bClass = QUnicodeTables::LineBreak_AL;
      }

      switch (LB::breakTable[x_bClass][cur_bClass < QUnicodeTables::LineBreak_SA ? cur_bClass : QUnicodeTables::LineBreak_AL]) {

         case LB::Action::DirectBreak:
            attributes[k].lineBreak = true;
            break;

        case LB::Action::IndirectBreak:
            if (last_bClass == QUnicodeTables::LineBreak_SP) {
               attributes[k].lineBreak = true;
            }
            break;

        case LB::Action::CombiningIndirectBreak:
            if (last_bClass != QUnicodeTables::LineBreak_SP) {
               last_bClass = cur_bClass;
               continue;
            }

            attributes[k].lineBreak = true;
            break;

        case LB::Action::CombiningProhibitedBreak:
            if (last_bClass != QUnicodeTables::LineBreak_SP) {
               last_bClass = cur_bClass;
               continue;
            }
            break;

        case LB::ProhibitedBreakAfterHebrewPlusHyphen:
            if (last_bClass != QUnicodeTables::LineBreak_HL) {
               attributes[k].lineBreak = true;
            }
            break;

        case LB::Action::ProhibitedBreak:
            // nothing to do

        default:
            break;
      }

      x_bClass    = cur_bClass;
      last_bClass = cur_bClass;
   }

   if (LB::actionTable[n_row][LB::NumClass::XX] == LB::NumAction::Break) {
      // LB25: do not break lines inside numbers
      for (int j = n_begin + 1; j < k; ++j) {
         attributes[j].lineBreak = false;
      }
   }

   attributes[0].lineBreak      = false;
   attributes[0].mandatoryBreak = false; // LB2

   attributes[k].lineBreak      = true;
   attributes[k].mandatoryBreak = true;  // LB3
}

static void getWhiteSpaces(const QString &str, QCharAttributes *attributes)
{
   int k = 0;

   for (QChar ch : str) {

      if (ch.isSpace()) {
         attributes[k].whiteSpace = true;
      }

      ++k;
   }
}

/*
static void resolveClusters(const QString &, QCharAttributes *)
{
   // emerald, passed parm names are "str, attributes"
}
*/

Q_CORE_EXPORT void initCharAttributes(const QString &str, QVector<QUnicodeTools::ScriptItem> &items,
                  QCharAttributes *attributes, CharAttributeOptions options)
{
   auto length = str.size();

   if (length <= 0) {
      return;
   }

   if (! (options & DontClearAttributes)) {
     ::memset(attributes, 0, (length + 1) * sizeof(QCharAttributes));
   }

   if (options & GraphemeBreaks) {
     getGraphemeBreaks(str, attributes);
   }

   if (options & WordBreaks) {
     getWordBreaks(str, attributes);
   }

   if (options & SentenceBreaks) {
     getSentenceBreaks(str, attributes);
   }

   if (options & LineBreaks) {
     getLineBreaks(str, attributes);
   }

   if (options & WhiteSpaces) {
     getWhiteSpaces(str, attributes);
   }

   if (items.size() <= 0) {
      return;
   }

   // emerald - implement another function to resolve unicode clusters for Indic languages
   // resolveClusters(Attributes(str, attributes));
}

Q_CORE_EXPORT void initScripts(const QString &str, QVector<QChar::Script> &items)
{
   int start = 0;
   int k     = 0;

   QChar::Script script = QChar::Script_Common;

   for (QChar ch : str) {
      const QUnicodeTables::Properties *prop = QUnicodeTables::properties(ch.unicode());

      if (prop->script == script || prop->script <= QChar::Script_Inherited) {
         ++k;
         continue;
      }

      // Never break between a combining mark (gc= Mc, Mn or Me) and its base character.
      // Thus, a combining mark — whatever its script property value is — should inherit
      // the script property value of its base character.

      static const int test = (FLAG(QChar::Mark_NonSpacing) | FLAG(QChar::Mark_SpacingCombining) | FLAG(QChar::Mark_Enclosing));

      if (FLAG(prop->category) & test) {
         ++k;
         continue;
      }

      while (start < k) {
        items.append(script);
        ++start;
      }

      script = QChar::Script(prop->script);
      ++k;
   }

   while (start < k) {
      items.append(script);
      ++start;
   }
}

} // namespace

