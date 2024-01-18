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

#include <qbytearray.h>
#include <qchar.h>
#include <qdebug.h>
#include <qfile.h>
#include <qhash.h>
#include <qlist.h>
#include <qstring.h>
#include <qvector.h>

#include <utility>

#define DATA_VERSION_S "15.1"
#define DATA_VERSION_STR "QChar::Unicode_15_1"

const QString UnicodeDataPrefix = "../data/";

enum Direction          : int;
enum JoiningType        : int;
enum GraphemeBreakClass : int;
enum WordBreakClass     : int;
enum SentenceBreakClass : int;
enum LineBreakClass     : int;

static QHash<QByteArray, QChar::UnicodeVersion> age_map;
static QHash<QByteArray, QChar::Category>       categoryMap;
static QHash<QByteArray, QChar::Decomposition>  decompositionMap;
static QHash<QByteArray, Direction>             directionMap;
static QHash<QByteArray, JoiningType>           joining_map;

static QHash<QByteArray, GraphemeBreakClass>   grapheme_break_map;
static QHash<QByteArray, WordBreakClass>       word_break_map;
static QHash<QByteArray, SentenceBreakClass>   sentence_break_map;
static QHash<QByteArray, LineBreakClass>       line_break_map;
static QHash<QByteArray, QChar::Script>        scriptMap;

static void initAgeMap()
{
   struct AgeMap {
      const QChar::UnicodeVersion version;
      const char *age;
   } ageMap[] = {
      { QChar::Unicode_1_1,   "1.1"  },
      { QChar::Unicode_2_0,   "2.0"  },
      { QChar::Unicode_2_1,   "2.1"  },
      { QChar::Unicode_3_0,   "3.0"  },
      { QChar::Unicode_3_1,   "3.1"  },
      { QChar::Unicode_3_2,   "3.2"  },
      { QChar::Unicode_4_0,   "4.0"  },
      { QChar::Unicode_4_1,   "4.1"  },
      { QChar::Unicode_5_0,   "5.0"  },
      { QChar::Unicode_5_1,   "5.1"  },
      { QChar::Unicode_5_2,   "5.2"  },
      { QChar::Unicode_6_0,   "6.0"  },
      { QChar::Unicode_6_1,   "6.1"  },
      { QChar::Unicode_6_2,   "6.2"  },
      { QChar::Unicode_6_3,   "6.3"  },
      { QChar::Unicode_7_0,   "7.0"  },
      { QChar::Unicode_8_0,   "8.0"  },
      { QChar::Unicode_9_0,   "9.0"  },
      { QChar::Unicode_10_0,  "10.0" },
      { QChar::Unicode_11_0,  "11.0" },
      { QChar::Unicode_12_0,  "12.0" },
      { QChar::Unicode_12_1,  "12.1" },
      { QChar::Unicode_13_0,  "13.0" },
      { QChar::Unicode_14_0,  "14.0" },
      { QChar::Unicode_15_0,  "15.0" },
      { QChar::Unicode_15_1,  "15.1" },
      { QChar::Unicode_Unassigned, 0 }
   };

   AgeMap *d = ageMap;
   while (d->age) {
      age_map.insert(d->age, d->version);
      ++d;
   }
}

static void initCategoryMap()
{
   struct Cat {
      QChar::Category cat;
      const char *name;
   } categories[] = {
      { QChar::Mark_NonSpacing,          "Mn" },
      { QChar::Mark_SpacingCombining,    "Mc" },
      { QChar::Mark_Enclosing,           "Me" },

      { QChar::Number_DecimalDigit,      "Nd" },
      { QChar::Number_Letter,            "Nl" },
      { QChar::Number_Other,             "No" },

      { QChar::Separator_Space,          "Zs" },
      { QChar::Separator_Line,           "Zl" },
      { QChar::Separator_Paragraph,      "Zp" },

      { QChar::Other_Control,            "Cc" },
      { QChar::Other_Format,             "Cf" },
      { QChar::Other_Surrogate,          "Cs" },
      { QChar::Other_PrivateUse,         "Co" },
      { QChar::Other_NotAssigned,        "Cn" },

      { QChar::Letter_Uppercase,         "Lu" },
      { QChar::Letter_Lowercase,         "Ll" },
      { QChar::Letter_Titlecase,         "Lt" },
      { QChar::Letter_Modifier,          "Lm" },
      { QChar::Letter_Other,             "Lo" },

      { QChar::Punctuation_Connector,    "Pc" },
      { QChar::Punctuation_Dash,         "Pd" },
      { QChar::Punctuation_Open,         "Ps" },
      { QChar::Punctuation_Close,        "Pe" },
      { QChar::Punctuation_InitialQuote, "Pi" },
      { QChar::Punctuation_FinalQuote,   "Pf" },
      { QChar::Punctuation_Other,        "Po" },

      { QChar::Symbol_Math,              "Sm" },
      { QChar::Symbol_Currency,          "Sc" },
      { QChar::Symbol_Modifier,          "Sk" },
      { QChar::Symbol_Other,             "So" },
      { QChar::Other_NotAssigned,        0    }
   };

   Cat *c = categories;
   while (c->name) {
      categoryMap.insert(c->name, c->cat);
      ++c;
   }
}

static void initDecompositionMap()
{
   struct Dec {
      QChar::Decomposition dec;
      const char *name;
   } decompositionX[] = {
      { QChar::Canonical, "<canonical>" },
      { QChar::Font,      "<font>"      },
      { QChar::NoBreak,   "<noBreak>"   },
      { QChar::Initial,   "<initial>"   },
      { QChar::Medial,    "<medial>"    },
      { QChar::Final,     "<final>"     },
      { QChar::Isolated,  "<isolated>"  },
      { QChar::Circle,    "<circle>"    },
      { QChar::Super,     "<super>"     },
      { QChar::Sub,       "<sub>"       },
      { QChar::Vertical,  "<vertical>"  },
      { QChar::Wide,      "<wide>"      },
      { QChar::Narrow,    "<narrow>"    },
      { QChar::Small,     "<small>"     },
      { QChar::Square,    "<square>"    },
      { QChar::Compat,    "<compat>"    },
      { QChar::Fraction,  "<fraction>"  },
      { QChar::NoDecomposition, 0       }
   };

   Dec *d = decompositionX;
   while (d->name) {
      decompositionMap.insert(d->name, d->dec);
      ++d;
   }
}

enum Direction : int {
   DirL   = QChar::DirL,
   DirR   = QChar::DirR,
   DirEN  = QChar::DirEN,
   DirES  = QChar::DirES,
   DirET  = QChar::DirET,
   DirAN  = QChar::DirAN,
   DirCS  = QChar::DirCS,
   DirB   = QChar::DirB,
   DirS   = QChar::DirS,
   DirWS  = QChar::DirWS,
   DirON  = QChar::DirON,
   DirLRE = QChar::DirLRE,
   DirLRO = QChar::DirLRO,
   DirAL  = QChar::DirAL,
   DirRLE = QChar::DirRLE,
   DirRLO = QChar::DirRLO,
   DirPDF = QChar::DirPDF,
   DirNSM = QChar::DirNSM,
   DirBN  = QChar::DirBN,
   DirLRI = QChar::DirLRI,
   DirRLI = QChar::DirRLI,
   DirFSI = QChar::DirFSI,
   DirPDI = QChar::DirPDI,
   Dir_Unassigned
};

static void initDirectionMap()
{
   struct Dir {
      Direction dir;
      const char *name;
   } directions[] = {
      { DirL,   "L"   },      // left to right
      { DirR,   "R"   },      // right to left
      { DirEN,  "EN"  },      // european number
      { DirES,  "ES"  },      // european number separator
      { DirET,  "ET"  },      // european number terminator
      { DirAN,  "AN"  },      // arabic number
      { DirCS,  "CS"  },      // common number separator

      { DirB,   "B"   },      // paragraph separator
      { DirS,   "S"   },      // segment separator
      { DirWS,  "WS"  },      // white space
      { DirON,  "ON"  },      // other neutrals

      { DirLRE, "LRE" },      // left to right embedding
      { DirLRO, "LRO" },      // left to right override
      { DirAL,  "AL"  },      // right to left arabic
      { DirRLE, "RLE" },      // right to left embedding
      { DirRLO, "RLO" },      // right to left override
      { DirPDF, "PDF" },      // pop directional format
      { DirNSM, "NSM" },      // nonspacing mark
      { DirBN,  "BN"  },      // boundary neutral

      { DirLRI, "LRI" },      // left to right isolate
      { DirRLI, "RLI" },      // right to left isolate
      { DirFSI, "FSI" },      // first strong isolate
      { DirPDI, "PDI" },      // pop directional isolate
      { Dir_Unassigned, 0 }
   };

   Dir *d = directions;
   while (d->name) {
      directionMap.insert(d->name, d->dir);
      ++d;
   }
}

enum JoiningType : int {
   Joining_None,
   Joining_Causing,
   Joining_Dual,
   Joining_Right,
   Joining_Left,
   Joining_Transparent,
   Joining_Unassigned
};

static void initJoiningMap()
{
   struct JoiningList {
      JoiningType joining;
      const char *name;
   } joinings[] = {
      { Joining_None,        "U" },
      { Joining_Causing,     "C" },
      { Joining_Dual,        "D" },
      { Joining_Right,       "R" },
      { Joining_Left,        "L" },
      { Joining_Transparent, "T" },
      { Joining_Unassigned,  0   }
   };

   JoiningList *d = joinings;
   while (d->name) {
      joining_map.insert(d->name, d->joining);
      ++d;
   }
}

static const char *grapheme_break_class_string =
   "enum GraphemeBreakClass {\n"
   "    GraphemeBreak_Any,\n"
   "    GraphemeBreak_CR,\n"
   "    GraphemeBreak_LF,\n"
   "    GraphemeBreak_Control,\n"
   "    GraphemeBreak_Extend,\n"
   "    GraphemeBreak_ZWJ,\n"
   "    GraphemeBreak_RegionalIndicator,\n"
   "    GraphemeBreak_Prepend,\n"
   "    GraphemeBreak_SpacingMark,\n"
   "    GraphemeBreak_L,\n"
   "    GraphemeBreak_V,\n"
   "    GraphemeBreak_T,\n"
   "    GraphemeBreak_LV,\n"
   "    GraphemeBreak_LVT,\n"
   "    Graphemebreak_E_Base,\n"
   "    Graphemebreak_E_Modifier,\n"
   "    Graphemebreak_Glue_After_Zwj,\n"
   "    Graphemebreak_E_Base_GAZ,\n"
   "    NumGraphemeBreakClasses,\n"
   "};\n\n";

enum GraphemeBreakClass : int {
   GraphemeBreak_Any,
   GraphemeBreak_CR,
   GraphemeBreak_LF,
   GraphemeBreak_Control,
   GraphemeBreak_Extend,
   GraphemeBreak_ZWJ,
   GraphemeBreak_RegionalIndicator,
   GraphemeBreak_Prepend,
   GraphemeBreak_SpacingMark,
   GraphemeBreak_L,
   GraphemeBreak_V,
   GraphemeBreak_T,
   GraphemeBreak_LV,
   GraphemeBreak_LVT,
   Graphemebreak_E_Base,
   Graphemebreak_E_Modifier,
   Graphemebreak_Glue_After_Zwj,
   Graphemebreak_E_Base_GAZ,
   GraphemeBreak_Unassigned
};

static void initGraphemeBreak()
{
   struct GraphemeBreakList {
      GraphemeBreakClass brk;
      const char *name;
   } breaks[] = {
      { GraphemeBreak_Any,               "Any"         },
      { GraphemeBreak_CR,                "CR"          },
      { GraphemeBreak_LF,                "LF"          },
      { GraphemeBreak_Control,           "Control"     },
      { GraphemeBreak_Extend,            "Extend"      },
      { GraphemeBreak_ZWJ,               "ZWJ"         },
      { GraphemeBreak_RegionalIndicator, "Regional_Indicator" },
      { GraphemeBreak_Prepend,           "Prepend"     },
      { GraphemeBreak_SpacingMark,       "SpacingMark" },
      { GraphemeBreak_L,                 "L"           },
      { GraphemeBreak_V,                 "V"           },
      { GraphemeBreak_T,                 "T"           },
      { GraphemeBreak_LV,                "LV"          },
      { GraphemeBreak_LVT,               "LVT"         },
      { Graphemebreak_E_Base,            "E_Base"      },
      { Graphemebreak_E_Modifier,        "E_Modifier"  },
      { Graphemebreak_Glue_After_Zwj,    "Glue_After_Zwj"     },
      { Graphemebreak_E_Base_GAZ,        "E_Base_GAZ"  },
      { GraphemeBreak_Unassigned,        0 }
   };

   GraphemeBreakList *d = breaks;
   while (d->name) {
      grapheme_break_map.insert(d->name, d->brk);
      ++d;
   }
}

static const char *word_break_class_string =
   "enum WordBreakClass {\n"
   "    WordBreak_Any,\n"
   "    WordBreak_CR,\n"
   "    WordBreak_LF,\n"
   "    WordBreak_Newline,\n"
   "    WordBreak_Extend,\n"
   "    WordBreak_ZWJ,\n"
   "    WordBreak_Format,\n"
   "    WordBreak_RegionalIndicator,\n"
   "    WordBreak_Katakana,\n"
   "    WordBreak_HebrewLetter,\n"
   "    WordBreak_ALetter,\n"
   "    WordBreak_SingleQuote,\n"
   "    WordBreak_DoubleQuote,\n"
   "    WordBreak_MidNumLet,\n"
   "    WordBreak_MidLetter,\n"
   "    WordBreak_MidNum,\n"
   "    WordBreak_Numeric,\n"
   "    WordBreak_ExtendNumLet,\n"
   "    WordBreak_E_Base,\n"
   "    WordBreak_E_Modifier,\n"
   "    WordBreak_Glue_After_Zwj,\n"
   "    WordBreak_E_Base_GAZ,\n"
   "    WordBreak_WSegSpace,\n"
   "    NumWordBreakClasses,\n"
   "};\n\n";

enum WordBreakClass : int {
   WordBreak_Any,
   WordBreak_CR,
   WordBreak_LF,
   WordBreak_Newline,
   WordBreak_Extend,
   WordBreak_ZWJ,
   WordBreak_Format,
   WordBreak_RegionalIndicator,
   WordBreak_Katakana,
   WordBreak_HebrewLetter,
   WordBreak_ALetter,
   WordBreak_SingleQuote,
   WordBreak_DoubleQuote,
   WordBreak_MidNumLet,
   WordBreak_MidLetter,
   WordBreak_MidNum,
   WordBreak_Numeric,
   WordBreak_ExtendNumLet,
   WordBreak_E_Base,
   WordBreak_E_Modifier,
   WordBreak_Glue_After_Zwj,
   WordBreak_E_Base_GAZ,
   WordBreak_WSegSpace,
   WordBreak_Unassigned
};

static void initWordBreak()
{
   struct WordBreakList {
      WordBreakClass brk;
      const char *name;
   } breaks[] = {
      { WordBreak_Any,               "Any"                },
      { WordBreak_CR,                "CR"                 },
      { WordBreak_LF,                "LF"                 },
      { WordBreak_Newline,           "Newline"            },
      { WordBreak_Extend,            "Extend"             },
      { WordBreak_ZWJ,               "ZWJ"                },
      { WordBreak_Format,            "Format"             },
      { WordBreak_RegionalIndicator, "Regional_Indicator" },
      { WordBreak_Katakana,          "Katakana"           },
      { WordBreak_HebrewLetter,      "Hebrew_Letter"      },
      { WordBreak_ALetter,           "ALetter"            },
      { WordBreak_SingleQuote,       "Single_Quote"       },
      { WordBreak_DoubleQuote,       "Double_Quote"       },
      { WordBreak_MidNumLet,         "MidNumLet"          },
      { WordBreak_MidLetter,         "MidLetter"          },
      { WordBreak_MidNum,            "MidNum"             },
      { WordBreak_Numeric,           "Numeric"            },
      { WordBreak_ExtendNumLet,      "ExtendNumLet"       },
      { WordBreak_E_Base,            "E_Base"             },
      { WordBreak_E_Modifier,        "E_Modifier"         },
      { WordBreak_Glue_After_Zwj,    "Glue_After_Zwj"     },
      { WordBreak_E_Base_GAZ,        "E_Base_GAZ"         },
      { WordBreak_WSegSpace,         "WSegSpace"          },
      { WordBreak_Unassigned,        0                    }
   };

   WordBreakList *d = breaks;
   while (d->name) {
      word_break_map.insert(d->name, d->brk);
      ++d;
   }
}


static const char *sentence_break_class_string =
   "enum SentenceBreakClass {\n"
   "    SentenceBreak_Any,\n"
   "    SentenceBreak_CR,\n"
   "    SentenceBreak_LF,\n"
   "    SentenceBreak_Extend,\n"
   "    SentenceBreak_Sep,\n"
   "    SentenceBreak_Format,\n"
   "    SentenceBreak_Sp,\n"
   "    SentenceBreak_Lower,\n"
   "    SentenceBreak_Upper,\n"
   "    SentenceBreak_OLetter,\n"
   "    SentenceBreak_Numeric,\n"
   "    SentenceBreak_ATerm,\n"
   "    SentenceBreak_SContinue,\n"
   "    SentenceBreak_STerm,\n"
   "    SentenceBreak_Close,\n"
   "    NumSentenceBreakClasses\n"
   "};\n\n";

enum SentenceBreakClass : int {
   SentenceBreak_Any,
   SentenceBreak_CR,
   SentenceBreak_LF,
   SentenceBreak_Extend,
   SentenceBreak_Sep,
   SentenceBreak_Format,
   SentenceBreak_Sp,
   SentenceBreak_Lower,
   SentenceBreak_Upper,
   SentenceBreak_OLetter,
   SentenceBreak_Numeric,
   SentenceBreak_ATerm,
   SentenceBreak_SContinue,
   SentenceBreak_STerm,
   SentenceBreak_Close,
   SentenceBreak_Unassigned
};

static void initSentenceBreak()
{
   struct SentenceBreakList {
      SentenceBreakClass brk;
      const char *name;
   } breaks[] = {
      { SentenceBreak_Any,        "Any"       },
      { SentenceBreak_CR,         "CR"        },
      { SentenceBreak_LF,         "LF"        },
      { SentenceBreak_Extend,     "Extend"    },
      { SentenceBreak_Sep,        "Sep"       },
      { SentenceBreak_Format,     "Format"    },
      { SentenceBreak_Sp,         "Sp"        },
      { SentenceBreak_Lower,      "Lower"     },
      { SentenceBreak_Upper,      "Upper"     },
      { SentenceBreak_OLetter,    "OLetter"   },
      { SentenceBreak_Numeric,    "Numeric"   },
      { SentenceBreak_ATerm,      "ATerm"     },
      { SentenceBreak_SContinue,  "SContinue" },
      { SentenceBreak_STerm,      "STerm"     },
      { SentenceBreak_Close,      "Close"     },
      { SentenceBreak_Unassigned, 0 }
   };

   SentenceBreakList *d = breaks;
   while (d->name) {
      sentence_break_map.insert(d->name, d->brk);
      ++d;
   }
}


static const char *line_break_class_string =
   "// see http://www.unicode.org/reports/tr14/tr14-30.html\n"
   "// XX, AI, and AK classes are  mapped to AL\n"
   "enum LineBreakClass {\n"
   "    LineBreak_OP, LineBreak_CL, LineBreak_CP, LineBreak_QU, LineBreak_GL,\n"
   "    LineBreak_NS, LineBreak_EX, LineBreak_SY, LineBreak_IS, LineBreak_PR,\n"
   "    LineBreak_PO, LineBreak_NU, LineBreak_AL, LineBreak_HL, LineBreak_ID,\n"
   "    LineBreak_IN, LineBreak_HY, LineBreak_BA, LineBreak_BB, LineBreak_B2,\n"
   "    LineBreak_ZW, LineBreak_CM, LineBreak_WJ, LineBreak_H2, LineBreak_H3,\n"
   "    LineBreak_JL, LineBreak_JV, LineBreak_JT, LineBreak_RI, LineBreak_CB,\n"
   "    LineBreak_EB, LineBreak_EM, LineBreak_ZWJ,\n"
   "    LineBreak_SA, LineBreak_SG, LineBreak_SP,\n"
   "    LineBreak_CR, LineBreak_LF, LineBreak_BK,\n"
   "    NumLineBreakClasses\n"
   "};\n\n";

enum LineBreakClass : int {
   LineBreak_OP, LineBreak_CL, LineBreak_CP, LineBreak_QU, LineBreak_GL,
   LineBreak_NS, LineBreak_EX, LineBreak_SY, LineBreak_IS, LineBreak_PR,
   LineBreak_PO, LineBreak_NU, LineBreak_AL, LineBreak_HL, LineBreak_ID,
   LineBreak_IN, LineBreak_HY, LineBreak_BA, LineBreak_BB, LineBreak_B2,
   LineBreak_ZW, LineBreak_CM, LineBreak_WJ, LineBreak_H2, LineBreak_H3,
   LineBreak_JL, LineBreak_JV, LineBreak_JT, LineBreak_RI, LineBreak_CB,
   LineBreak_EB, LineBreak_EM, LineBreak_ZWJ,
   LineBreak_SA, LineBreak_SG, LineBreak_SP,
   LineBreak_CR, LineBreak_LF, LineBreak_BK,
   LineBreak_Unassigned
};

static void initLineBreak()
{
   struct LineBreakList {
      LineBreakClass brk;
      const char *name;
   } breaks[] = {
      { LineBreak_BK, "BK" },
      { LineBreak_CR, "CR" },
      { LineBreak_LF, "LF" },
      { LineBreak_CM, "CM" },
      { LineBreak_BK, "NL" },      // Class NL is mapped to BK
      { LineBreak_SG, "SG" },
      { LineBreak_WJ, "WJ" },
      { LineBreak_ZW, "ZW" },
      { LineBreak_GL, "GL" },
      { LineBreak_SP, "SP" },
      { LineBreak_B2, "B2" },
      { LineBreak_BA, "BA" },
      { LineBreak_BB, "BB" },
      { LineBreak_HY, "HY" },
      { LineBreak_CB, "CB" },
      { LineBreak_NS, "CJ" },      // CJ is mapped as NS to yield CSS strict line breaking
      { LineBreak_CL, "CL" },
      { LineBreak_CP, "CP" },
      { LineBreak_EX, "EX" },
      { LineBreak_IN, "IN" },
      { LineBreak_NS, "NS" },
      { LineBreak_OP, "OP" },
      { LineBreak_QU, "QU" },
      { LineBreak_IS, "IS" },
      { LineBreak_NU, "NU" },
      { LineBreak_PO, "PO" },
      { LineBreak_PR, "PR" },
      { LineBreak_SY, "SY" },
      { LineBreak_AL, "AL" },
      { LineBreak_AL, "AI" },      // AI is mapped to AL
      { LineBreak_AL, "AK" },      // AK is mapped to AL
      { LineBreak_AL, "AS" },      // AS is mapped to AL
      { LineBreak_AL, "AP" },      // AP is mapped to AL
      { LineBreak_AL, "VF" },      // VF is mapped to AL
      { LineBreak_AL, "VI" },      // VI is mapped to AL
      { LineBreak_AL, "XX" },      // XX is mapped to AL
      { LineBreak_HL, "HL" },
      { LineBreak_H2, "H2" },
      { LineBreak_H3, "H3" },
      { LineBreak_ID, "ID" },
      { LineBreak_JL, "JL" },
      { LineBreak_JV, "JV" },
      { LineBreak_JT, "JT" },
      { LineBreak_RI, "RI" },
      { LineBreak_SA, "SA" },
      { LineBreak_EB, "EB" },
      { LineBreak_EM, "EM" },
      { LineBreak_ZWJ, "ZWJ" },
      { LineBreak_Unassigned, 0 }
   };

   LineBreakList *d = breaks;
   while (d->name) {
      line_break_map.insert(d->name, d->brk);
      ++d;
   }
}

static void initScriptMap()
{
   struct Scrpt {
      QChar::Script script;
      const char *name;
   } scripts[] = {
      // general
      { QChar::Script_Unknown,                "Unknown" },
      { QChar::Script_Inherited,              "Inherited" },
      { QChar::Script_Common,                 "Common" },

      // pre-4.0
      { QChar::Script_Latin,                  "Latin" },
      { QChar::Script_Greek,                  "Greek" },
      { QChar::Script_Cyrillic,               "Cyrillic" },
      { QChar::Script_Armenian,               "Armenian" },
      { QChar::Script_Hebrew,                 "Hebrew" },
      { QChar::Script_Arabic,                 "Arabic" },
      { QChar::Script_Syriac,                 "Syriac" },
      { QChar::Script_Thaana,                 "Thaana" },
      { QChar::Script_Devanagari,             "Devanagari" },
      { QChar::Script_Bengali,                "Bengali" },
      { QChar::Script_Gurmukhi,               "Gurmukhi" },
      { QChar::Script_Gujarati,               "Gujarati" },
      { QChar::Script_Oriya,                  "Oriya" },
      { QChar::Script_Tamil,                  "Tamil" },
      { QChar::Script_Telugu,                 "Telugu" },
      { QChar::Script_Kannada,                "Kannada" },
      { QChar::Script_Malayalam,              "Malayalam" },
      { QChar::Script_Sinhala,                "Sinhala" },
      { QChar::Script_Thai,                   "Thai" },
      { QChar::Script_Lao,                    "Lao" },
      { QChar::Script_Tibetan,                "Tibetan" },
      { QChar::Script_Myanmar,                "Myanmar" },
      { QChar::Script_Georgian,               "Georgian" },
      { QChar::Script_Hangul,                 "Hangul" },
      { QChar::Script_Ethiopic,               "Ethiopic" },
      { QChar::Script_Cherokee,               "Cherokee" },
      { QChar::Script_CanadianAboriginal,     "CanadianAboriginal" },
      { QChar::Script_Ogham,                  "Ogham" },
      { QChar::Script_Runic,                  "Runic" },
      { QChar::Script_Khmer,                  "Khmer" },
      { QChar::Script_Mongolian,              "Mongolian" },
      { QChar::Script_Hiragana,               "Hiragana" },
      { QChar::Script_Katakana,               "Katakana" },
      { QChar::Script_Bopomofo,               "Bopomofo" },
      { QChar::Script_Han,                    "Han" },
      { QChar::Script_Yi,                     "Yi" },
      { QChar::Script_OldItalic,              "OldItalic" },
      { QChar::Script_Gothic,                 "Gothic" },
      { QChar::Script_Deseret,                "Deseret" },
      { QChar::Script_Tagalog,                "Tagalog" },
      { QChar::Script_Hanunoo,                "Hanunoo" },
      { QChar::Script_Buhid,                  "Buhid" },
      { QChar::Script_Tagbanwa,               "Tagbanwa" },
      { QChar::Script_Coptic,                 "Coptic" },

      // 4.0
      { QChar::Script_Limbu,                  "Limbu" },
      { QChar::Script_TaiLe,                  "TaiLe" },
      { QChar::Script_LinearB,                "LinearB" },
      { QChar::Script_Ugaritic,               "Ugaritic" },
      { QChar::Script_Shavian,                "Shavian" },
      { QChar::Script_Osmanya,                "Osmanya" },
      { QChar::Script_Cypriot,                "Cypriot" },
      { QChar::Script_Braille,                "Braille" },

      // 4.1
      { QChar::Script_Buginese,               "Buginese" },
      { QChar::Script_NewTaiLue,              "NewTaiLue" },
      { QChar::Script_Glagolitic,             "Glagolitic" },
      { QChar::Script_Tifinagh,               "Tifinagh" },
      { QChar::Script_SylotiNagri,            "SylotiNagri" },
      { QChar::Script_OldPersian,             "OldPersian" },
      { QChar::Script_Kharoshthi,             "Kharoshthi" },

      // 5.0
      { QChar::Script_Balinese,               "Balinese" },
      { QChar::Script_Cuneiform,              "Cuneiform" },
      { QChar::Script_Phoenician,             "Phoenician" },
      { QChar::Script_PhagsPa,                "PhagsPa" },
      { QChar::Script_Nko,                    "Nko" },

      // 5.1
      { QChar::Script_Sundanese,              "Sundanese" },
      { QChar::Script_Lepcha,                 "Lepcha" },
      { QChar::Script_OlChiki,                "OlChiki" },
      { QChar::Script_Vai,                    "Vai" },
      { QChar::Script_Saurashtra,             "Saurashtra" },
      { QChar::Script_KayahLi,                "KayahLi" },
      { QChar::Script_Rejang,                 "Rejang" },
      { QChar::Script_Lycian,                 "Lycian" },
      { QChar::Script_Carian,                 "Carian" },
      { QChar::Script_Lydian,                 "Lydian" },
      { QChar::Script_Cham,                   "Cham"   },

      // 5.2
      { QChar::Script_TaiTham,                "TaiTham" },
      { QChar::Script_TaiViet,                "TaiViet" },
      { QChar::Script_Avestan,                "Avestan" },
      { QChar::Script_EgyptianHieroglyphs,    "EgyptianHieroglyphs" },
      { QChar::Script_Samaritan,              "Samaritan" },
      { QChar::Script_Lisu,                   "Lisu" },
      { QChar::Script_Bamum,                  "Bamum" },
      { QChar::Script_Javanese,               "Javanese" },
      { QChar::Script_MeeteiMayek,            "MeeteiMayek" },
      { QChar::Script_ImperialAramaic,        "ImperialAramaic" },
      { QChar::Script_OldSouthArabian,        "OldSouthArabian" },
      { QChar::Script_InscriptionalParthian,  "InscriptionalParthian" },
      { QChar::Script_InscriptionalPahlavi,   "InscriptionalPahlavi" },
      { QChar::Script_OldTurkic,              "OldTurkic" },
      { QChar::Script_Kaithi,                 "Kaithi" },

      // 6.0
      { QChar::Script_Batak,                  "Batak" },
      { QChar::Script_Brahmi,                 "Brahmi" },
      { QChar::Script_Mandaic,                "Mandaic" },

      // 6.1
      { QChar::Script_Chakma,                 "Chakma" },
      { QChar::Script_MeroiticCursive,        "MeroiticCursive" },
      { QChar::Script_MeroiticHieroglyphs,    "MeroiticHieroglyphs" },
      { QChar::Script_Miao,                   "Miao" },
      { QChar::Script_Sharada,                "Sharada" },
      { QChar::Script_SoraSompeng,            "SoraSompeng" },
      { QChar::Script_Takri,                  "Takri" },

      // 7.0
      { QChar::Script_Caucasian_Albanian,     "CaucasianAlbanian" },
      { QChar::Script_Bassa_Vah,              "BassaVah" },
      { QChar::Script_Duployan,               "Duployan" },
      { QChar::Script_Elbasan,                "Elbasan" },
      { QChar::Script_Grantha,                "Grantha" },
      { QChar::Script_Pahawh_Hmong,           "PahawhHmong" },
      { QChar::Script_Khojki,                 "Khojki" },
      { QChar::Script_Linear_A,               "LinearA" },
      { QChar::Script_Mahajani,               "Mahajani" },
      { QChar::Script_Manichaean,             "Manichaean" },
      { QChar::Script_Mende_Kikakui,          "MendeKikakui" },
      { QChar::Script_Modi,                   "Modi" },
      { QChar::Script_Mro,                    "Mro" },
      { QChar::Script_Old_North_Arabian,      "OldNorthArabian" },
      { QChar::Script_Nabataean,              "Nabataean" },
      { QChar::Script_Palmyrene,              "Palmyrene" },
      { QChar::Script_PauCinHau,              "PauCinHau" },
      { QChar::Script_OldPermic,              "OldPermic" },
      { QChar::Script_Psalter_Pahlavi,        "PsalterPahlavi" },
      { QChar::Script_Siddham,                "Siddham" },
      { QChar::Script_Khudawadi,              "Khudawadi" },
      { QChar::Script_Tirhuta,                "Tirhuta" },
      { QChar::Script_Warang_Citi,            "WarangCiti" },

      // 8.0
      { QChar::Script_Ahom,                   "Ahom" },
      { QChar::Script_Anatolian_Hieroglyphs,  "AnatolianHieroglyphs" },
      { QChar::Script_Hatran,                 "Hatran" },
      { QChar::Script_Multani,                "Multani" },
      { QChar::Script_Old_Hungarian,          "OldHungarian" },
      { QChar::Script_SignWriting,            "SignWriting" },

      // 9.0
      { QChar::Script_Adlam,                  "Adlam" },
      { QChar::Script_Bhaiksuki,              "Bhaiksuki" },
      { QChar::Script_Marchen,                "Marchen" },
      { QChar::Script_Newa,                   "Newa" },
      { QChar::Script_Osage,                  "Osage" },
      { QChar::Script_Tangut,                 "Tangut" },

      // 10.0
      { QChar::Script_Masaram_Gondi,          "MasaramGondi" },
      { QChar::Script_Nushu,                  "Nushu" },
      { QChar::Script_Soyombo,                "Soyombo" },
      { QChar::Script_Zanabazar_Square,       "ZanabazarSquare" },

      // 11.0
      { QChar::Script_Hanifi_Rohingya,        "HanifiRohingya" },
      { QChar::Script_Old_Sogdian,            "OldSogdian" },
      { QChar::Script_Sogdian,                "Sogdian" },
      { QChar::Script_Dogra,                  "Dogra" },
      { QChar::Script_Gunjala_Gondi,          "GunjalaGondi" },
      { QChar::Script_Makasar,                "Makasar" },
      { QChar::Script_Medefaidrin,            "Medefaidrin" },

      // 12.0
      { QChar::Script_Elymaic,                "Elymaic" },
      { QChar::Script_Nandinagari,            "Nandinagari" },
      { QChar::Script_Nyiakeng_Puachue_Hmong, "NyiakengPuachueHmong" },
      { QChar::Script_Wancho,                 "Wancho" },

      // 13.0
      { QChar::Script_Chorasmian,             "Chorasmian" },
      { QChar::Script_Dives_Akuru,            "DivesAkuru" },
      { QChar::Script_Khitan_Small_Script,    "KhitanSmallScript" },
      { QChar::Script_Yezidi,                 "Yezidi" },

      // 14.0
      { QChar::Script_Cypro_Minoan,           "CyproMinoan"},
      { QChar::Script_Old_Uyghur,             "OldUyghur"},
      { QChar::Script_Tangsa,                 "Tangsa"},
      { QChar::Script_Toto,                   "Toto"},
      { QChar::Script_Vithkuqi,               "Vithkuqi"},

      // 15.0
      { QChar::Script_Kawi,                   "Kawi"},
      { QChar::Script_Nag_Mundari,            "NagMundari"},

      // 15.1
      // nothing was added

      // unhandled
      { QChar::Script_Unknown,                0 }
   };
   Scrpt *p = scripts;

   while (p->name) {
      scriptMap.insert(p->name, p->script);
      ++p;
   }
}


// keep in sync with the code in create_PropertyTables()
static const char *property_string =
   "struct Properties {\n"
   "    ushort category            : 8;       /* 5 used */\n"
   "    ushort direction           : 8;       /* 5 used */\n"
   "    ushort combiningClass      : 8;\n"
   "    ushort joining             : 3;\n"
   "    signed short digitValue    : 5;\n"
   "    signed short mirrorDiff    : 16;\n"
   "    ushort unicodeVersion      : 8;       /* 5 used */\n"
   "    ushort nfQuickCheck        : 8;\n"
   "    ushort graphemeBreakClass  : 5;       /* 5 used */\n"
   "    ushort wordBreakClass      : 5;       /* 5 used */\n"
   "    ushort sentenceBreakClass  : 8;       /* 4 used */\n"
   "    ushort lineBreakClass      : 6;       /* 6 used */\n"
   "    ushort script              : 8;\n"
   "};\n\n"

   "Q_CORE_EXPORT const Properties * properties(uint ucs4);\n"
   "Q_CORE_EXPORT const Properties * properties(ushort ucs2);\n\n"

   "extern const unsigned short uc_decomposition_trie[];\n"
   "extern const char32_t       uc_decomposition_map[];\n"
   "extern const unsigned short uc_ligature_trie[];\n"
   "extern const char32_t       uc_ligature_map[];\n\n"

   "char32_t uc_caseFold(char32_t value);\n"
   "char32_t uc_lowerCase(char32_t value);\n"
   "char32_t uc_titleCase(char32_t value);\n"
   "char32_t uc_upperCase(char32_t value);\n\n"

   "const char32_t *uc_caseFoldSpecial(char32_t value);\n"
   "const char32_t *uc_lowerCaseSpecial(char32_t value);\n"
   "const char32_t *uc_titleCaseSpecial(char32_t value);\n"
   "const char32_t *uc_upperCaseSpecial(char32_t value);\n\n"

   R"(struct CaseFoldTraits
   {
      static inline char32_t caseValue(char32_t ch)
      {
         if (ch >= QChar::SpecialCharacter::LastValidCodePoint) {
            return ch;

         } else {
            return QUnicodeTables::uc_caseFold(ch);
         }
      }

      static inline const char32_t *caseSpecial(char32_t ch)
      {
         return QUnicodeTables::uc_caseFoldSpecial(ch);
      }
   };

   )"

   R"(struct LowerCaseTraits
   {
      static inline char32_t caseValue(char32_t ch)
      {
         if (ch >= QChar::SpecialCharacter::LastValidCodePoint) {
            return ch;

         } else {
            return QUnicodeTables::uc_lowerCase(ch);
         }
      }

      static inline const char32_t *caseSpecial(char32_t ch)
      {
         return QUnicodeTables::uc_lowerCaseSpecial(ch);
      }
   };

   )"

   R"(struct TitleCaseTraits
   {
      static inline char32_t caseValue(char32_t ch)
      {
         if (ch >= QChar::SpecialCharacter::LastValidCodePoint) {
            return ch;

         } else {
            return QUnicodeTables::uc_titleCase(ch);
         }
      }

      static inline const char32_t *caseSpecial(char32_t ch)
      {
         return QUnicodeTables::uc_titleCaseSpecial(ch);
      }
   };

   )"

   R"(struct UpperCaseTraits
   {
      static inline char32_t caseValue(char32_t ch)
      {
         if (ch >= QChar::SpecialCharacter::LastValidCodePoint) {
            return ch;

         } else {
            return QUnicodeTables::uc_upperCase(ch);
         }
      }

      static inline const char32_t *caseSpecial(char32_t ch)
      {
         return QUnicodeTables::uc_upperCaseSpecial(ch);
      }
   };

   )";

   // end of function


static const char *breakMethods =
   "Q_CORE_EXPORT QUnicodeTables::GraphemeBreakClass graphemeBreakClass(uint ucs4);\n"
   "Q_CORE_EXPORT QUnicodeTables::WordBreakClass wordBreakClass(uint ucs4);\n"
   "Q_CORE_EXPORT QUnicodeTables::SentenceBreakClass sentenceBreakClass(uint ucs4);\n"
   "Q_CORE_EXPORT QUnicodeTables::LineBreakClass lineBreakClass(uint ucs4);\n"
   "\n"

   "inline GraphemeBreakClass graphemeBreakClass(QChar ch)\n"
   "{\n"
   "   return graphemeBreakClass(ch.unicode());\n"
   "}\n"
   "\n"

   "inline WordBreakClass wordBreakClass(QChar ch)\n"
   "{\n"
   "   return wordBreakClass(ch.unicode());\n"
   "}\n"
   "\n"

   "inline SentenceBreakClass sentenceBreakClass(QChar ch)\n"
   "{\n"
   "   return sentenceBreakClass(ch.unicode());\n"
   "}\n"
   "\n"

   "inline LineBreakClass lineBreakClass(QChar ch)\n"
   "{\n"
   "   return lineBreakClass(ch.unicode());\n"
   "}\n"
   "\n";

static const char *hangulConstants =
   "// constants for Hangul (de)composition, see UAX #15\n"
   "enum Hangul_Constants {\n"
   "   Hangul_SBase  = 0xac00,\n"
   "   Hangul_LBase  = 0x1100,\n"
   "   Hangul_VBase  = 0x1161,\n"
   "   Hangul_TBase  = 0x11a7,\n"
   "   Hangul_LCount = 19,\n"
   "   Hangul_VCount = 21,\n"
   "   Hangul_TCount = 28,\n"
   "   Hangul_NCount = Hangul_VCount * Hangul_TCount,\n"
   "   Hangul_SCount = Hangul_LCount * Hangul_NCount\n"
   "};\n"
   "\n";

struct PropertyFlags_Special
{
   QList<char32_t> lowerCaseList;
   QList<char32_t> upperCaseList;
   QList<char32_t> titleCaseList;
   QList<char32_t> caseFoldList;
};

struct PropertyFlags {
   bool operator==(const PropertyFlags &o) const {

      return (   combiningClass     == o.combiningClass
              && category           == o.category
              && direction          == o.direction
              && joining            == o.joining
              && age                == o.age
              && digitValue         == o.digitValue
              && mirrorDiff         == o.mirrorDiff
              && lowerCaseDiff      == o.lowerCaseDiff
              && upperCaseDiff      == o.upperCaseDiff
              && titleCaseDiff      == o.titleCaseDiff
              && caseFoldDiff       == o.caseFoldDiff
              && lowerCaseSpecial   == o.lowerCaseSpecial
              && upperCaseSpecial   == o.upperCaseSpecial
              && titleCaseSpecial   == o.titleCaseSpecial
              && caseFoldSpecial    == o.caseFoldSpecial
              && graphemeBreakClass == o.graphemeBreakClass
              && wordBreakClass     == o.wordBreakClass
              && sentenceBreakClass == o.sentenceBreakClass
              && lineBreakClass     == o.lineBreakClass
              && script             == o.script
              && nfQuickCheck       == o.nfQuickCheck);
   }

   // from UnicodeData.txt
   uchar combiningClass       : 8;
   QChar::Category category   : 5;
   QChar::Direction direction : 5;

   // from ArabicShaping.txt
   QChar::JoiningType joining : 3;

   // from DerivedAge.txt
   QChar::UnicodeVersion age  : 5;
   int digitValue;

   int mirrorDiff : 16;

   int lowerCaseDiff;
   int upperCaseDiff;
   int titleCaseDiff;
   int caseFoldDiff;

   bool lowerCaseSpecial;
   bool upperCaseSpecial;
   bool titleCaseSpecial;
   bool caseFoldSpecial;

   std::shared_ptr<PropertyFlags_Special> specialFlags;

   GraphemeBreakClass graphemeBreakClass;
   WordBreakClass     wordBreakClass;
   SentenceBreakClass sentenceBreakClass;
   LineBreakClass     lineBreakClass;

   int script;

   // from DerivedNormalizationProps.txt
   uchar nfQuickCheck;
};

// DerivedCoreProperties.txt
static inline bool isDefaultIgnorable(uint ucs4)
{
   // Default_Ignorable_Code_Point:
   //  Generated from
   //    Other_Default_Ignorable_Code_Point + Cf + Variation_Selector
   //    - White_Space - FFF9..FFFB (Annotation Characters)
   //    - 0600..0604, 06DD, 070F, 110BD (exceptional Cf characters that should be visible)
   if (ucs4 <= 0xff) {
      return ucs4 == 0xad;
   }

   return ucs4 == 0x034f
          || ucs4 == 0x061c
          || (ucs4 >= 0x115f  && ucs4 <= 0x1160)
          || (ucs4 >= 0x17b4  && ucs4 <= 0x17b5)
          || (ucs4 >= 0x180b  && ucs4 <= 0x180d)
          || ucs4 == 0x180e
          || (ucs4 >= 0x200b  && ucs4 <= 0x200f)
          || (ucs4 >= 0x202a  && ucs4 <= 0x202e)
          || (ucs4 >= 0x2060  && ucs4 <= 0x206f)
          || ucs4 == 0x3164
          || (ucs4 >= 0xfe00  && ucs4 <= 0xfe0f)
          || ucs4 == 0xfeff
          || ucs4 == 0xffa0
          || (ucs4 >= 0xfff0  && ucs4 <= 0xfff8)
          || (ucs4 >= 0x1bca0 && ucs4 <= 0x1bca3)
          || (ucs4 >= 0x1d173 && ucs4 <= 0x1d17a)
          || (ucs4 >= 0xe0000 && ucs4 <= 0xe0fff);
}

struct UnicodeData {
   UnicodeData(int codepoint = 0) {
      pFlags.category       = QChar::Other_NotAssigned;       // Cn
      pFlags.combiningClass = 0;
      pFlags.direction      = QChar::DirL;

      // DerivedBidiClass.txt

      if ((codepoint >= 0x0600 && codepoint <= 0x07BF)
            || (codepoint >= 0x08A0  && codepoint <= 0x08FF)
            || (codepoint >= 0xFB50  && codepoint <= 0xFDCF)
            || (codepoint >= 0xFDF0  && codepoint <= 0xFDFF)
            || (codepoint >= 0xFE70  && codepoint <= 0xFEFF)
            || (codepoint >= 0x1EE00 && codepoint <= 0x1EEFF)) {

         // The unassigned code points that default to AL are in the ranges:
         //     [U+0600..U+07BF, U+08A0..U+08FF, U+FB50..U+FDCF, U+FDF0..U+FDFF, U+FE70..U+FEFF, U+1EE00..U+1EEFF]

         pFlags.direction = QChar::DirAL;

      } else if ((codepoint >= 0x0590 && codepoint <= 0x05FF)
                 || (codepoint >= 0x07C0  && codepoint <= 0x089F)
                 || (codepoint >= 0xFB1D  && codepoint <= 0xFB4F)
                 || (codepoint >= 0x10800 && codepoint <= 0x10FFF)
                 || (codepoint >= 0x1E800 && codepoint <= 0x1EDFF)
                 || (codepoint >= 0x1EF00 && codepoint <= 0x1EFFF)) {

         // The unassigned code points that default to R are in the ranges:
         //     [U+0590..U+05FF, U+07C0..U+089F, U+FB1D..U+FB4F, U+10800..U+10FFF, U+1E800..U+1EDFF, U+1EF00..U+1EFFF]

         pFlags.direction = QChar::DirR;

      } else if (codepoint >= 0x20A0 && codepoint <= 0x20CF) {
         // The unassigned code points that default to ET are in the range:
         //     [U+20A0..U+20CF]

         pFlags.direction = QChar::DirET;

      } else if (QChar(codepoint).isNonCharacter() || isDefaultIgnorable(codepoint)) {

         // The unassigned code points that default to BN have one of the following properties:
         //     Default_Ignorable_Code_Point
         //     Noncharacter_Code_Point

         pFlags.direction = QChar::DirBN;
      }

      pFlags.lineBreakClass = LineBreak_AL; // XX -> AL

      // LineBreak.txt
      // The unassigned code points that default to "ID" include ranges in the following blocks:
      //     [U+3400..U+4DBF, U+4E00..U+9FFF, U+F900..U+FAFF, U+20000..U+2A6DF,
      //     U+2A700..U+2B73F, U+2B740..U+2B81F, U+2B820..U+2CEAF, U+2F800..U+2FA1F]
      // and any other reserved code points on
      //     [U+20000..U+2FFFD, U+30000..U+3FFFD]

      if ((     codepoint >= 0x3400  && codepoint <= 0x4DBF)
            || (codepoint >= 0x4E00  && codepoint <= 0x9FFF)
            || (codepoint >= 0xF900  && codepoint <= 0xFAFF)
            || (codepoint >= 0x20000 && codepoint <= 0x2A6DF)
            || (codepoint >= 0x2A700 && codepoint <= 0x2B73F)
            || (codepoint >= 0x2B740 && codepoint <= 0x2B81F)
            || (codepoint >= 0x2B820 && codepoint <= 0x2CEAF)
            || (codepoint >= 0x2F800 && codepoint <= 0x2FA1F)
            || (codepoint >= 0x20000 && codepoint <= 0x2FFFD)
            || (codepoint >= 0x30000 && codepoint <= 0x3FFFD)) {
         pFlags.lineBreakClass = LineBreak_ID;

      } else if (codepoint >= 0x20A0 && codepoint <= 0x20CF) {
         // The unassigned code points that default to "PR" comprise a range in the following block: [U+20A0..U+20CF]

         pFlags.lineBreakClass = LineBreak_PR;
      }

      mirroredChar            = 0;
      decompositionType       = QChar::NoDecomposition;

      pFlags.joining          = QChar::Joining_None;
      pFlags.age              = QChar::Unicode_Unassigned;
      pFlags.mirrorDiff       = 0;
      pFlags.digitValue       = -1;

      pFlags.lowerCaseDiff    = 0;
      pFlags.upperCaseDiff    = 0;
      pFlags.titleCaseDiff    = 0;
      pFlags.caseFoldDiff     = 0;

      pFlags.lowerCaseSpecial = 0;
      pFlags.upperCaseSpecial = 0;
      pFlags.titleCaseSpecial = 0;
      pFlags.caseFoldSpecial  = 0;

      pFlags.graphemeBreakClass = GraphemeBreak_Any;
      pFlags.wordBreakClass     = WordBreak_Any;
      pFlags.sentenceBreakClass = SentenceBreak_Any;

      pFlags.script             = QChar::Script_Unknown;
      pFlags.nfQuickCheck       = 0;

      propertyIndex             = -1;
      excludedComposition       = false;
   }

   static UnicodeData &get_codePoint_data(int codepoint);

   PropertyFlags pFlags;

   // from UnicodeData.txt
   static QList<UnicodeData> m_data;

   QChar::Decomposition decompositionType;
   QList<char32_t> decomposition;
   QList<char32_t> specialFolding;

   // from BidiMirroring.txt
   int mirroredChar;

   // DerivedNormalizationProps.txt
   bool excludedComposition;

   // computed position of unicode property set
   int propertyIndex;
};

QList<UnicodeData> UnicodeData::m_data;

UnicodeData &UnicodeData::get_codePoint_data(int codepoint)
{
   static bool initialized = false;

   if (! initialized) {
      for (int uc = 0; uc <= QChar::LastValidCodePoint; ++uc) {
         m_data.append(UnicodeData(uc));
      }

      initialized = true;
   }

   Q_ASSERT(codepoint <= 0x10ffff);

   return m_data[codepoint];
}


static QHash<int, int> decompositionLength;
static int highestComposedCharacter = 0;
static int numLigatures             = 0;
static int highestLigature          = 0;

struct Ligature {
   int u1;
   int u2;
   int ligature;
};

// sorted after the first component for fast lookup
bool operator < (const Ligature &l1, const Ligature &l2)
{ return l1.u1 < l2.u1; }

static QHash<int, QList<Ligature> > ligatureHashes;
static QHash<int, int> combiningClassUsage;

static void readUnicodeData()
{
   enum UniDataFields {
      UD_Value,                // 0
      UD_Name,
      UD_Category,
      UD_CombiningClass,
      UD_BidiCategory,
      UD_Decomposition,        // 5
      UD_DecimalDigitValue,
      UD_DigitValue,
      UD_NumericValue,
      UD_Mirrored,
      UD_OldName,              // 10
      UD_Comment,
      UD_UpperCase,
      UD_LowerCase,
      UD_TitleCase             // 13
   };

   qDebug("\nReading UnicodeData.txt");

   QFile f(UnicodeDataPrefix + "UnicodeData.txt");
   if (! f.exists()) {
      qFatal("Could not find UnicodeData.txt");
   }

   f.open(QFile::ReadOnly);

   while (! f.atEnd()) {
      QByteArray line = f.readLine();
      line = line.trimmed();

      int comment = line.indexOf('#');

      if (comment >= 0) {
         line = line.left(comment);
      }

      if (line.isEmpty()) {
         continue;
      }

      QList<QByteArray> properties = line.split(';');

      bool ok;
      int codepoint = properties[UD_Value].toInt(&ok, 16);

      Q_ASSERT(ok);
      Q_ASSERT(codepoint <= QChar::LastValidCodePoint);

      int lastCodepoint = codepoint;

      QByteArray name = properties[UD_Name];
      if (name.startsWith('<') && name.contains("First")) {
         QByteArray nextLine = f.readLine();
         nextLine = nextLine.trimmed();

         QList<QByteArray> properties = nextLine.split(';');

         Q_ASSERT(properties[UD_Name].startsWith('<') && properties[UD_Name].contains("Last"));
         lastCodepoint = properties[UD_Value].toInt(&ok, 16);

         Q_ASSERT(ok);
         Q_ASSERT(lastCodepoint <= QChar::LastValidCodePoint);
      }

      UnicodeData &rowData          = UnicodeData::get_codePoint_data(codepoint);
      rowData.pFlags.category       = categoryMap.value(properties[UD_Category], QChar::Other_NotAssigned);
      rowData.pFlags.combiningClass = properties[UD_CombiningClass].toInt();

      if (! combiningClassUsage.contains(rowData.pFlags.combiningClass)) {
         combiningClassUsage[rowData.pFlags.combiningClass] = 1;
      } else {
         ++combiningClassUsage[rowData.pFlags.combiningClass];
      }

      Direction dir = directionMap.value(properties[UD_BidiCategory], Dir_Unassigned);
      if (dir == Dir_Unassigned) {
         qFatal("Unhandled direction value: %s", properties[UD_BidiCategory].constData());
      }

      rowData.pFlags.direction = QChar::Direction(dir);

      if (! properties[UD_UpperCase].isEmpty()) {
         int upperCase = properties[UD_UpperCase].toInt(&ok, 16);
         Q_ASSERT(ok);

         int diff = upperCase - codepoint;
         rowData.pFlags.upperCaseDiff = diff;
      }

      if (! properties[UD_LowerCase].isEmpty()) {
         int lowerCase = properties[UD_LowerCase].toInt(&ok, 16);
         Q_ASSERT(ok);

         int diff = lowerCase - codepoint;
         rowData.pFlags.lowerCaseDiff = diff;
      }

      // if titleCase is empty use the upperCase value
      if (properties[UD_TitleCase].isEmpty()) {
         properties[UD_TitleCase] = properties[UD_UpperCase];
      }

      if (! properties[UD_TitleCase].isEmpty()) {
         int titleCase = properties[UD_TitleCase].toInt(&ok, 16);
         Q_ASSERT_X(ok, "Tables:: ", "Conversion to integer failed");

         int diff = titleCase - codepoint;
         rowData.pFlags.titleCaseDiff = diff;
      }

      if (! properties[UD_DigitValue].isEmpty()) {
         rowData.pFlags.digitValue = properties[UD_DigitValue].toInt();
      }

      // decompositition
      QByteArray decomposition = properties[UD_Decomposition];

      if (! decomposition.isEmpty()) {
         highestComposedCharacter = qMax(highestComposedCharacter, codepoint);
         QList<QByteArray> d = decomposition.split(' ');

         if (d[0].contains('<')) {
            rowData.decompositionType = decompositionMap.value(d[0], QChar::NoDecomposition);
            if (rowData.decompositionType == QChar::NoDecomposition) {
               qFatal("unhandled decomposition type: %s", d[0].constData());
            }

            d.takeFirst();

         } else {
            rowData.decompositionType = QChar::Canonical;
         }

         for (int i = 0; i < d.size(); ++i) {
            rowData.decomposition.append(d[i].toInt(&ok, 16));
            Q_ASSERT(ok);
         }
         ++decompositionLength[rowData.decomposition.size()];
      }

      for (int k = codepoint; k <= lastCodepoint; ++k) {
         UnicodeData::m_data[k] = rowData;
      }
   }
}

static int maxMirroredDiff = 0;

static void readBidiMirroring()
{
   qDebug("\nReading BidiMirroring.txt");

   QFile f(UnicodeDataPrefix + "BidiMirroring.txt");
   if (!f.exists()) {
      qFatal("Could not find BidiMirroring.txt");
   }

   f.open(QFile::ReadOnly);

   while (!f.atEnd()) {
      QByteArray line = f.readLine();
      line = line.trimmed();

      int comment = line.indexOf('#');
      if (comment >= 0) {
         line = line.left(comment);
      }

      if (line.isEmpty()) {
         continue;
      }
      line = line.replace(" ", "");

      QList<QByteArray> pair = line.split(';');
      Q_ASSERT(pair.size() == 2);

      bool ok;
      int codepoint = pair[0].toInt(&ok, 16);
      Q_ASSERT(ok);

      int mirror = pair[1].toInt(&ok, 16);
      Q_ASSERT(ok);

      UnicodeData &rowData = UnicodeData::get_codePoint_data(codepoint);

      rowData.mirroredChar      = mirror;
      rowData.pFlags.mirrorDiff = rowData.mirroredChar - codepoint;

      maxMirroredDiff = qMax(maxMirroredDiff, qAbs(rowData.pFlags.mirrorDiff));
   }
}

static void readArabicShaping()
{
   qDebug("\nReading ArabicShaping.txt");

   // Initialize defaults:
   // Code points that are not explicitly listed in ArabicShaping.txt are either of joining type T or U:
   // - Those that not explicitly listed that are of General Category Mn, Me, or Cf have joining type T.
   // - All others not explicitly listed have joining type U.

   for (int codepoint = 0; codepoint <= QChar::LastValidCodePoint; ++codepoint) {
      UnicodeData &rowData = UnicodeData::get_codePoint_data(codepoint);

      if (rowData.pFlags.joining == QChar::Joining_None) {

         if (rowData.pFlags.category == QChar::Mark_NonSpacing || rowData.pFlags.category == QChar::Mark_Enclosing ||
                  rowData.pFlags.category == QChar::Other_Format) {

            rowData.pFlags.joining = QChar::Joining_Transparent;
         }
      }
   }
   QFile f(UnicodeDataPrefix + "ArabicShaping.txt");
   if (! f.exists()) {
      qFatal("Could not find ArabicShaping.txt");
   }

   f.open(QFile::ReadOnly);

   while (! f.atEnd()) {
      QByteArray line = f.readLine();
      line = line.trimmed();

      int comment = line.indexOf('#');
      if (comment >= 0) {
         line = line.left(comment);
      }

      line = line.trimmed();

      if (line.isEmpty()) {
         continue;
      }

      QList<QByteArray> l = line.split(';');
      Q_ASSERT(l.size() == 4);

      bool ok;
      int codepoint = l[0].toInt(&ok, 16);
      Q_ASSERT(ok);

      UnicodeData &rowData = UnicodeData::get_codePoint_data(codepoint);
      JoiningType joining  = joining_map.value(l[2].trimmed(), Joining_Unassigned);

      switch (joining) {
         case Joining_Unassigned:
            qFatal("%x: unassigned or unhandled joining type: %s", codepoint, l[2].constData());
            break;

         case Joining_Transparent:
            if (rowData.pFlags.category != QChar::Mark_NonSpacing &&
                rowData.pFlags.category != QChar::Mark_Enclosing &&
                rowData.pFlags.category != QChar::Other_Format &&
                rowData.pFlags.category != QChar::Letter_Modifier)   {


               qFatal("%x: joining type '%s' was met, the current implementation needs to be revised",
                      codepoint, l[2].constData());
            }
            [[fallthrough]];

         default:
            rowData.pFlags.joining = QChar::JoiningType(joining);
            break;
      }
   }
}
static void readDerivedAge()
{
   qDebug("\nReading DerivedAge.txt");

   QFile f(UnicodeDataPrefix + "DerivedAge.txt");
   if (!f.exists()) {
      qFatal("Could not find DerivedAge.txt");
   }

   f.open(QFile::ReadOnly);

   while (! f.atEnd()) {
      QByteArray line = f.readLine();
      line = line.trimmed();

      int comment = line.indexOf('#');

      if (comment >= 0) {
         line = line.left(comment);
      }

      line.replace(" ", "");

      if (line.isEmpty()) {
         continue;
      }

      QList<QByteArray> l = line.split(';');
      Q_ASSERT(l.size() == 2);

      QByteArray codes = l[0];
      codes.replace("..", ".");
      QList<QByteArray> cl = codes.split('.');

      bool ok;
      int from = cl[0].toInt(&ok, 16);
      Q_ASSERT(ok);

      int to = from;
      if (cl.size() == 2) {
         to = cl[1].toInt(&ok, 16);
         Q_ASSERT(ok);
      }

      QChar::UnicodeVersion age = age_map.value(l[1].trimmed(), QChar::Unicode_Unassigned);

      if (age == QChar::Unicode_Unassigned) {
         qFatal("unassigned or unhandled age value: %s", l[1].constData());
      }

      for (int codepoint = from; codepoint <= to; ++codepoint) {
         UnicodeData &d = UnicodeData::get_codePoint_data(codepoint);
         d.pFlags.age = age;
      }
   }
}

static void readDerivedNormalizationProps()
{
   qDebug("\nReading DerivedNormalizationProps.txt");

   QFile f(UnicodeDataPrefix + "DerivedNormalizationProps.txt");

   if (! f.exists()) {
      qFatal("Could not find DerivedNormalizationProps.txt");
   }

   f.open(QFile::ReadOnly);

   while (!f.atEnd()) {
      QByteArray line = f.readLine();
      line = line.trimmed();

      int comment = line.indexOf('#');
      if (comment >= 0) {
         line = line.left(comment);
      }

      if (line.trimmed().isEmpty()) {
         continue;
      }

      QList<QByteArray> l = line.split(';');
      Q_ASSERT(l.size() >= 2);

      QByteArray propName = l[1].trimmed();
      if (propName != "Full_Composition_Exclusion" &&
            propName != "NFD_QC" && propName != "NFC_QC" &&
            propName != "NFKD_QC" && propName != "NFKC_QC") {
         // ###
         continue;
      }

      QByteArray codes = l[0].trimmed();
      codes.replace("..", ".");

      QList<QByteArray> cl = codes.split('.');

      bool ok;
      int from = cl[0].toInt(&ok, 16);
      Q_ASSERT(ok);

      int to = from;
      if (cl.size() == 2) {
         to = cl[1].toInt(&ok, 16);
         Q_ASSERT(ok);
      }

      for (int codepoint = from; codepoint <= to; ++codepoint) {
         UnicodeData &d = UnicodeData::get_codePoint_data(codepoint);

         if (propName == "Full_Composition_Exclusion") {
            d.excludedComposition = true;

         } else {
            static_assert(QString::NormalizationForm_D  == 0);
            static_assert(QString::NormalizationForm_C  == 1);
            static_assert(QString::NormalizationForm_KD == 2);
            static_assert(QString::NormalizationForm_KC == 3);

            QString::NormalizationForm form;
            if (propName == "NFD_QC") {
               form = QString::NormalizationForm_D;
            } else if (propName == "NFC_QC") {
               form = QString::NormalizationForm_C;
            } else if (propName == "NFKD_QC") {
               form = QString::NormalizationForm_KD;
            } else { // if (propName == "NFKC_QC")
               form = QString::NormalizationForm_KC;
            }

            Q_ASSERT(l.size() == 3);
            l[2] = l[2].trimmed();

            enum { NFQC_YES = 0, NFQC_NO = 1, NFQC_MAYBE = 3 };
            uchar ynm = (l[2] == "N" ? NFQC_NO : l[2] == "M" ? NFQC_MAYBE : NFQC_YES);

            if (ynm == NFQC_MAYBE) {
               // if this changes, we need to revise the normalizationQuickCheckHelper() implementation
               Q_ASSERT(form == QString::NormalizationForm_C || form == QString::NormalizationForm_KC);
            }
            d.pFlags.nfQuickCheck |= (ynm << (form << 1)); // 2 bits per NF
         }
      }
   }

   for (int codepoint = 0; codepoint <= QChar::LastValidCodePoint; ++codepoint) {
      UnicodeData &rowData = UnicodeData::get_codePoint_data(codepoint);

      if (! rowData.excludedComposition && rowData.decompositionType == QChar::Canonical && rowData.decomposition.size() > 1) {

         Q_ASSERT(rowData.decomposition.size() == 2);

         int part1 = rowData.decomposition.at(0);
         int part2 = rowData.decomposition.at(1);

         // all non-starters are listed in DerivedNormalizationProps.txt
         // and already excluded from composition
         Q_ASSERT(UnicodeData::get_codePoint_data(part1).pFlags.combiningClass == 0);

         ++numLigatures;
         highestLigature = qMax(highestLigature, part1);
         Ligature item   = { part1, part2, codepoint };

         ligatureHashes[part2].append(item);
      }
   }
}


struct NormalizationCorrection {
   char32_t codepoint;
   char32_t mapped;
   int      version;
};

static std::pair<QByteArray, QByteArray> createNormalizationCorrections()
{
   qDebug("\nReading NormalizationCorrections.txt");

   QFile f(UnicodeDataPrefix + "NormalizationCorrections.txt");
   if (! f.exists()) {
      qFatal("Could not find NormalizationCorrections.txt");
   }

   f.open(QFile::ReadOnly);

   QByteArray out_cpp;
   QByteArray out_h;

   out_h += "struct NormalizationCorrection {\n"
          "    char32_t ucs4;\n"
          "    char32_t old_mapping;\n"
          "    int      version;\n"
          "};\n\n";

   out_cpp += "const NormalizationCorrection uc_normalization_corrections[] = {\n";

   int maxVersion     = 0;
   int numCorrections = 0;

   while (! f.atEnd()) {
      QByteArray line = f.readLine();
      line = line.trimmed();

      int comment = line.indexOf('#');

      if (comment >= 0) {
         line = line.left(comment);
      }

      line.replace(" ", "");

      if (line.isEmpty()) {
         continue;
      }

      Q_ASSERT(! line.contains(".."));

      QList<QByteArray> fields = line.split(';');
      Q_ASSERT(fields.size() == 4);

      NormalizationCorrection c = { 0, 0, 0 };
      bool ok;
      c.codepoint = fields.at(0).toInt(&ok, 16);
      Q_ASSERT(ok);

      c.mapped = fields.at(1).toInt(&ok, 16);
      Q_ASSERT(ok);

      if (fields.at(3) == "3.2.0") {
         c.version = QChar::Unicode_3_2;

      } else if (fields.at(3) == "4.0.0") {
         c.version = QChar::Unicode_4_0;

      } else {
         qFatal("Unknown unicode version in NormalizationCorrection.txt");
      }

      out_cpp += "    { 0x" + QByteArray::number(c.codepoint, 16) + ", ";

      if (c.codepoint <= 0xffff) {
         out_cpp += " ";
      }

      out_cpp += "0x" + QByteArray::number(c.mapped, 16) + ", ";

      if (c.mapped <= 0xffff) {
         out_cpp += " ";
      }

      out_cpp += QByteArray::number(c.version) + " },\n";

      ++numCorrections;
      maxVersion = qMax(c.version, maxVersion);
   }

   if (out_cpp.endsWith(",\n")) {
      out_cpp.chop(2);
   }

   out_cpp += "\n};\n\n";


   out_h += "extern const NormalizationCorrection uc_normalization_corrections[];\n\n";

   out_h += "static constexpr const int NumNormalizationCorrections        = " + QByteArray::number(numCorrections) + ";\n";
   out_h += "static constexpr const int NormalizationCorrectionsVersionMax = " + QByteArray::number(maxVersion)     + ";\n\n";

   return { out_cpp, out_h };
}

static void readLineBreak()
{
   qDebug("\nReading LineBreak.txt");

   QFile f(UnicodeDataPrefix + "LineBreak.txt");
   if (! f.exists()) {
      qFatal("Could not find LineBreak.txt");
   }

   f.open(QFile::ReadOnly);

   while (! f.atEnd()) {
      QByteArray line = f.readLine();
      line = line.trimmed();

      int comment = line.indexOf('#');
      if (comment >= 0) {
         line = line.left(comment);
      }
      line.replace(" ", "");

      if (line.isEmpty()) {
         continue;
      }

      QList<QByteArray> l = line.split(';');
      Q_ASSERT(l.size() == 2);

      QByteArray codes = l[0];
      codes.replace("..", ".");
      QList<QByteArray> cl = codes.split('.');

      bool ok;
      int from = cl[0].toInt(&ok, 16);
      Q_ASSERT(ok);

      int to = from;

      if (cl.size() == 2) {
         to = cl[1].toInt(&ok, 16);
         Q_ASSERT(ok);
      }

      LineBreakClass lb = line_break_map.value(l[1], LineBreak_Unassigned);
      if (lb == LineBreak_Unassigned) {
         qFatal("unassigned line break class: %s", l[1].constData());
      }

      for (int codepoint = from; codepoint <= to; ++codepoint) {
         UnicodeData &rowData = UnicodeData::get_codePoint_data(codepoint);
         rowData.pFlags.lineBreakClass = lb;
      }
   }
}

static void readSpecialCasing()
{
   qDebug("\nReading SpecialCasing.txt");

   QFile f(UnicodeDataPrefix + "SpecialCasing.txt");
   if (! f.exists())  {
      qFatal("Could not find SpecialCasing.txt");
   }

   f.open(QFile::ReadOnly);

   while (! f.atEnd()) {
      QByteArray line = f.readLine();
      line = line.trimmed();

      int comment = line.indexOf('#');
      if (comment >= 0) {
         line = line.left(comment);
      }

      if (line.isEmpty()) {
         continue;
      }

      QList<QByteArray> l = line.split(';');

      QByteArray condition = l.size() < 5 ? QByteArray() : l[4].trimmed();
      if (! condition.isEmpty()) {
         continue;
      }

      bool ok;
      int codepoint = l[0].trimmed().toInt(&ok, 16);
      Q_ASSERT(ok);

      QList<QByteArray> lower = l[1].trimmed().split(' ');
      QList<char32_t> lowerMap;

      for (int i = 0; i < lower.size(); ++i) {
         bool ok;
         lowerMap.append(lower.at(i).toInt(&ok, 16));
         Q_ASSERT(ok);
      }

      QList<QByteArray> title = l[2].trimmed().split(' ');
      QList<char32_t> titleMap;

      for (int i = 0; i < title.size(); ++i) {
         bool ok;
         titleMap.append(title.at(i).toInt(&ok, 16));
         Q_ASSERT(ok);
      }

      QList<QByteArray> upper = l[3].trimmed().split(' ');
      QList<char32_t> upperMap;

      for (int i = 0; i < upper.size(); ++i) {
         bool ok;
         upperMap.append(upper.at(i).toInt(&ok, 16));
         Q_ASSERT(ok);
      }

      UnicodeData &rowData = UnicodeData::get_codePoint_data(codepoint);
      Q_ASSERT(lowerMap.size() > 1 || lowerMap.at(0) == codepoint + rowData.pFlags.lowerCaseDiff);
      Q_ASSERT(titleMap.size() > 1 || titleMap.at(0) == codepoint + rowData.pFlags.titleCaseDiff);
      Q_ASSERT(upperMap.size() > 1 || upperMap.at(0) == codepoint + rowData.pFlags.upperCaseDiff);

      if (lowerMap.size() > 1) {
         rowData.pFlags.lowerCaseSpecial = true;

         if (rowData.pFlags.specialFlags == nullptr) {
            rowData.pFlags.specialFlags = std::make_shared<PropertyFlags_Special>();
         }

         rowData.pFlags.specialFlags->lowerCaseList = lowerMap;
      }

      if (titleMap.size() > 1) {
         rowData.pFlags.titleCaseSpecial = true;

         if (rowData.pFlags.specialFlags == nullptr) {
            rowData.pFlags.specialFlags = std::make_shared<PropertyFlags_Special>();
         }

         rowData.pFlags.specialFlags->titleCaseList = titleMap;
      }

      if (upperMap.size() > 1) {
         rowData.pFlags.upperCaseSpecial = true;

         if (rowData.pFlags.specialFlags == nullptr) {
            rowData.pFlags.specialFlags = std::make_shared<PropertyFlags_Special>();
         }

         rowData.pFlags.specialFlags->upperCaseList = upperMap;
      }
   }
}

static int maxCaseFoldDiff = 0;

static void readCaseFolding()
{
   qDebug("\nReading CaseFolding.txt");

   QFile f(UnicodeDataPrefix + "CaseFolding.txt");
   if (! f.exists()) {
      qFatal("Could not find CaseFolding.txt");
   }

   f.open(QFile::ReadOnly);

   while (! f.atEnd()) {
      QByteArray line = f.readLine();
      line = line.trimmed();

      int comment = line.indexOf('#');
      if (comment >= 0) {
         line = line.left(comment);
      }

      if (line.isEmpty()) {
         continue;
      }

      QList<QByteArray> l = line.split(';');

      bool ok;
      int codepoint = l[0].trimmed().toInt(&ok, 16);
      Q_ASSERT(ok);

      l[1] = l[1].trimmed();


      QList<QByteArray> fold = l[2].trimmed().split(' ');
      QList<char32_t> foldMap;

      for (int i = 0; i < fold.size(); ++i) {
         bool ok;
         foldMap.append(fold.at(i).toInt(&ok, 16));
         Q_ASSERT(ok);
      }

      UnicodeData &rowData = UnicodeData::get_codePoint_data(codepoint);
      if (foldMap.size() == 1) {
         int caseFolded = foldMap.at(0);

         int diff = caseFolded - codepoint;
         rowData.pFlags.caseFoldDiff = diff;

      } else {
         rowData.pFlags.caseFoldSpecial = true;

         if (rowData.pFlags.specialFlags == nullptr) {
            rowData.pFlags.specialFlags = std::make_shared<PropertyFlags_Special>();
         }

         rowData.pFlags.specialFlags->caseFoldList = foldMap;
      }
   }
}

static void readGraphemeBreak()
{
   qDebug("\nReading GraphemeBreakProperty.txt");

   QFile f(UnicodeDataPrefix + "GraphemeBreakProperty.txt");
   if (! f.exists()) {
      qFatal("Could not find GraphemeBreakProperty.txt");
   }

   f.open(QFile::ReadOnly);

   while (!f.atEnd()) {
      QByteArray line = f.readLine();
      line = line.trimmed();

      int comment = line.indexOf('#');
      if (comment >= 0) {
         line = line.left(comment);
      }

      line.replace(" ", "");

      if (line.isEmpty()) {
         continue;
      }

      QList<QByteArray> l = line.split(';');
      Q_ASSERT(l.size() == 2);

      QByteArray codes = l[0];
      codes.replace("..", ".");
      QList<QByteArray> cl = codes.split('.');

      bool ok;
      int from = cl[0].toInt(&ok, 16);
      Q_ASSERT(ok);

      int to = from;
      if (cl.size() == 2) {
         to = cl[1].toInt(&ok, 16);
         Q_ASSERT(ok);
      }

      GraphemeBreakClass brk = grapheme_break_map.value(l[1], GraphemeBreak_Unassigned);
      if (brk == GraphemeBreak_Unassigned) {
         qFatal("unassigned grapheme break class: %s", l[1].constData());
      }

      for (int codepoint = from; codepoint <= to; ++codepoint) {
         UnicodeData &rowData = UnicodeData::get_codePoint_data(codepoint);
         rowData.pFlags.graphemeBreakClass = brk;
      }
   }
}

static void readWordBreak()
{
   qDebug("\nReading WordBreakProperty.txt");

   QFile f(UnicodeDataPrefix + "WordBreakProperty.txt");
   if (!f.exists()) {
      qFatal("Could not find WordBreakProperty.txt");
   }

   f.open(QFile::ReadOnly);

   while (! f.atEnd()) {
      QByteArray line = f.readLine();
      line = line.trimmed();

      int comment = line.indexOf('#');

      if (comment >= 0) {
         line = line.left(comment);
      }

      line.replace(" ", "");

      if (line.isEmpty()) {
         continue;
      }

      QList<QByteArray> l = line.split(';');
      Q_ASSERT(l.size() == 2);

      QByteArray codes = l[0];
      codes.replace("..", ".");
      QList<QByteArray> cl = codes.split('.');

      bool ok;
      int from = cl[0].toInt(&ok, 16);
      Q_ASSERT(ok);

      int to = from;

      if (cl.size() == 2) {
         to = cl[1].toInt(&ok, 16);
         Q_ASSERT(ok);
      }

      WordBreakClass brk = word_break_map.value(l[1], WordBreak_Unassigned);
      if (brk == WordBreak_Unassigned) {
         qFatal("unassigned word break class: %s", l[1].constData());
      }

      for (int codepoint = from; codepoint <= to; ++codepoint) {
         // as of Unicode 5.1, some punctuation marks were mapped to MidLetter and MidNumLet
         // which caused "hi.there" to be treated like if it were just a single word
         // until we have a tailoring mechanism, retain the old behavior by remapping those characters here.

         if (codepoint == 0x002E) {          // FULL STOP
            brk = WordBreak_MidNum;

         } else if (codepoint == 0x003A) {   // COLON
            brk = WordBreak_Any;
         }

         UnicodeData &rowData = UnicodeData::get_codePoint_data(codepoint);
         rowData.pFlags.wordBreakClass = brk;
      }
   }
}

static void readSentenceBreak()
{
   qDebug("\nReading SentenceBreakProperty.txt");

   QFile f(UnicodeDataPrefix + "SentenceBreakProperty.txt");
   if (! f.exists()) {
      qFatal("Could not find SentenceBreakProperty.txt");
   }

   f.open(QFile::ReadOnly);

   while (!f.atEnd()) {
      QByteArray line = f.readLine();
      line = line.trimmed();

      int comment = line.indexOf('#');
      if (comment >= 0) {
         line = line.left(comment);
      }

      line.replace(" ", "");

      if (line.isEmpty()) {
         continue;
      }

      QList<QByteArray> l = line.split(';');
      Q_ASSERT(l.size() == 2);

      QByteArray codes = l[0];
      codes.replace("..", ".");
      QList<QByteArray> cl = codes.split('.');

      bool ok;
      int from = cl[0].toInt(&ok, 16);
      Q_ASSERT(ok);

      int to = from;
      if (cl.size() == 2) {
         to = cl[1].toInt(&ok, 16);
         Q_ASSERT(ok);
      }

      SentenceBreakClass brk = sentence_break_map.value(l[1], SentenceBreak_Unassigned);
      if (brk == SentenceBreak_Unassigned) {
         qFatal("unassigned sentence break class: %s", l[1].constData());
      }

      for (int codepoint = from; codepoint <= to; ++codepoint) {
         UnicodeData &rowData = UnicodeData::get_codePoint_data(codepoint);
         rowData.pFlags.sentenceBreakClass = brk;
      }
   }
}

#if 0
static QList<QByteArray> blockNames;

struct BlockInfo {
   int blockIndex;
   int firstCodePoint;
   int lastCodePoint;
};
static QList<BlockInfo> blockInfoList;

static void readBlocks()
{
   qDebug("\nReading Blocks.txt");

   QFile f(UnicodeDataPrefix + "Blocks.txt");
   if (! f.exists()) {
      qFatal("Could not find Blocks.txt");
   }

   f.open(QFile::ReadOnly);

   while (!f.atEnd()) {
      QByteArray line = f.readLine();
      line = line.trimmed();

      int comment = line.indexOf("#");
      if (comment >= 0) {
         line = line.left(comment);
      }

      line.replace(" ", "");

      if (line.isEmpty()) {
         continue;
      }

      int semicolon = line.indexOf(';');
      Q_ASSERT(semicolon >= 0);

      QByteArray codePoints = line.left(semicolon);
      QByteArray blockName = line.mid(semicolon + 1);

      int blockIndex = blockNames.indexOf(blockName);
      if (blockIndex == -1) {
         blockIndex = blockNames.size();
         blockNames.append(blockName);
      }

      codePoints.replace("..", ".");
      QList<QByteArray> cl = codePoints.split('.');

      bool ok;
      int first = cl[0].toInt(&ok, 16);
      Q_ASSERT(ok);

      int last = first;
      if (cl.size() == 2) {
         last = cl[1].toInt(&ok, 16);
         Q_ASSERT(ok);
      }

      BlockInfo blockInfo = { blockIndex, first, last };
      blockInfoList.append(blockInfo);
   }
}
#endif

static void readScripts()
{
   qDebug("\nReading Scripts.txt");

   QFile f(UnicodeDataPrefix + "Scripts.txt");
   if (! f.exists()) {
      qFatal("Could not find Scripts.txt");
   }

   f.open(QFile::ReadOnly);

   while (! f.atEnd()) {
      QByteArray line = f.readLine();
      line = line.trimmed();

      int comment = line.indexOf("#");
      if (comment >= 0) {
         line = line.left(comment);
      }

      line.replace(" ", "");
      line.replace("_", "");

      if (line.isEmpty()) {
         continue;
      }

      int semicolon = line.indexOf(';');
      Q_ASSERT(semicolon >= 0);

      QByteArray codePoints = line.left(semicolon);
      QByteArray scriptName = line.mid(semicolon + 1);

      codePoints.replace("..", ".");
      QList<QByteArray> cl = codePoints.split('.');

      bool ok;
      int first = cl[0].toInt(&ok, 16);
      Q_ASSERT(ok);

      int last = first;
      if (cl.size() == 2) {
         last = cl[1].toInt(&ok, 16);
         Q_ASSERT(ok);
      }

      if (! scriptMap.contains(scriptName)) {
         qFatal("Unhandled script property value: %s", scriptName.constData());
      }
      QChar::Script script = scriptMap.value(scriptName, QChar::Script_Unknown);

      for (int codepoint = first; codepoint <= last; ++codepoint) {
         UnicodeData &rowData  = UnicodeData::get_codePoint_data(codepoint);
         rowData.pFlags.script = script;
      }
   }
}

static QList<PropertyFlags> uniqueProperties;

static void computeUniqueProperties()
{
   qDebug("computeUniqueProperties:");

   for (int codepoint = 0; codepoint <= QChar::LastValidCodePoint; ++codepoint) {
      UnicodeData &rowData = UnicodeData::get_codePoint_data(codepoint);
      int index = uniqueProperties.indexOf(rowData.pFlags);

      if (index == -1) {
         index = uniqueProperties.size();
         uniqueProperties.append(rowData.pFlags);
      }

      rowData.propertyIndex = index;
   }

   qDebug("    %d unique unicode properties found", uniqueProperties.size());
}

struct UniqueBlock {
   inline UniqueBlock() : index(-1) {}

   inline bool operator==(const UniqueBlock &other) const
   { return values == other.values; }

   int index;
   QVector<int> values;
};

static QByteArray create_PropertyTables()
{
   // reserve one bit more for the sign
   Q_ASSERT(maxMirroredDiff  < (1 << 12));

   const int BMP_BLOCKSIZE = 32;
   const int BMP_SHIFT     = 5;
   const int BMP_END       = 0x11000;
   const int SMP_END       = 0x110000;
   const int SMP_BLOCKSIZE = 256;
   const int SMP_SHIFT     = 8;

   QList<UniqueBlock> uniqueBlocks;
   QVector<int> blockMap;
   int used = 0;

   for (int block = 0; block < BMP_END / BMP_BLOCKSIZE; ++block) {
      UniqueBlock b;
      b.values.reserve(BMP_BLOCKSIZE);

      for (int i = 0; i < BMP_BLOCKSIZE; ++i) {
         int uc         = block * BMP_BLOCKSIZE + i;
         UnicodeData &d = UnicodeData::get_codePoint_data(uc);
         b.values.append(d.propertyIndex);
      }

      int index = uniqueBlocks.indexOf(b);
      if (index == -1) {
         index = uniqueBlocks.size();
         b.index = used;
         used    += BMP_BLOCKSIZE;
         uniqueBlocks.append(b);
      }

      blockMap.append(uniqueBlocks.at(index).index);
   }
   int bmp_blocks = uniqueBlocks.size();

   for (int block = BMP_END / SMP_BLOCKSIZE; block < SMP_END / SMP_BLOCKSIZE; ++block) {
      UniqueBlock b;
      b.values.reserve(SMP_BLOCKSIZE);

      for (int i = 0; i < SMP_BLOCKSIZE; ++i) {
         int uc = block * SMP_BLOCKSIZE + i;
         UnicodeData &d = UnicodeData::get_codePoint_data(uc);
         b.values.append(d.propertyIndex);
      }

      int index = uniqueBlocks.indexOf(b);
      if (index == -1) {
         index   = uniqueBlocks.size();
         b.index = used;
         used    += SMP_BLOCKSIZE;
         uniqueBlocks.append(b);
      }

      blockMap.append(uniqueBlocks.at(index).index);
   }
   int smp_blocks = uniqueBlocks.size() - bmp_blocks;

   int bmp_block_data = bmp_blocks * BMP_BLOCKSIZE * sizeof(unsigned short);
   int bmp_trie = BMP_END / BMP_BLOCKSIZE * sizeof(unsigned short);
   int bmp_mem  = bmp_block_data + bmp_trie;

   qDebug("    %d unique blocks in BMP", bmp_blocks);
   qDebug("        block data uses: %d bytes", bmp_block_data);
   qDebug("        trie data uses : %d bytes", bmp_trie);

   int smp_block_data = smp_blocks * SMP_BLOCKSIZE * sizeof(unsigned short);
   int smp_trie = (SMP_END - BMP_END) / SMP_BLOCKSIZE * sizeof(unsigned short);
   int smp_mem  = smp_block_data + smp_trie;

   qDebug("    %d unique blocks in SMP", smp_blocks);
   qDebug("        block data uses: %d bytes", smp_block_data);
   qDebug("        trie data uses : %d bytes", smp_trie);

   Q_ASSERT(blockMap.last() + blockMap.size() < (1 << (sizeof(unsigned short) * 8)));

   QByteArray out;
   out += "static const unsigned short uc_property_trie[] = {\n";

   // first write the map
   out += "    // [0x0..0x" + QByteArray::number(BMP_END, 16) + ")";

   for (int i = 0; i < BMP_END / BMP_BLOCKSIZE; ++i) {
      if (! (i % 8)) {
         if (out.endsWith(' ')) {
            out.chop(1);
         }
         if (!((i * BMP_BLOCKSIZE) % 0x1000)) {
            out += "\n";
         }
         out += "\n    ";
      }

      out += QByteArray::number(blockMap.at(i) + blockMap.size());
      out += ", ";
   }

   if (out.endsWith(' ')) {
      out.chop(1);
   }

   out += "\n\n    // [0x" + QByteArray::number(BMP_END, 16) + "..0x" + QByteArray::number(SMP_END, 16) + ")\n";
   for (int i = BMP_END / BMP_BLOCKSIZE; i < blockMap.size(); ++i) {
      if (! (i % 8)) {
         if (out.endsWith(' ')) {
            out.chop(1);
         }

         if (! (i % (0x10000 / SMP_BLOCKSIZE))) {
            out += "\n";
         }
         out += "\n    ";
      }
      out += QByteArray::number(blockMap.at(i) + blockMap.size());
      out += ", ";
   }

   if (out.endsWith(' ')) {
      out.chop(1);
   }

   out += "\n";
   // write the data
   for (int i = 0; i < uniqueBlocks.size(); ++i) {
      if (out.endsWith(' ')) {
         out.chop(1);
      }

      out += "\n";
      const UniqueBlock &b = uniqueBlocks.at(i);

      for (int j = 0; j < b.values.size(); ++j) {
         if (! (j % 8)) {
            if (out.endsWith(' ')) {
               out.chop(1);
            }
            out += "\n    ";
         }
         out += QByteArray::number(b.values.at(j));
         out += ", ";
      }
   }

   if (out.endsWith(", ")) {
      out.chop(2);
   }
   out += "\n};\n\n";

   out += "#define GET_PROP_INDEX(ucs4) \\\n"
          "       (ucs4 < 0x" + QByteArray::number(BMP_END, 16) + " \\\n"
          "        ? (uc_property_trie[uc_property_trie[ucs4>>" + QByteArray::number(BMP_SHIFT) +
          "] + (ucs4 & 0x" + QByteArray::number(BMP_BLOCKSIZE - 1, 16) + ")]) \\\n"
          "        : (uc_property_trie[uc_property_trie[((ucs4 - 0x" + QByteArray::number(BMP_END, 16) +
          ")>>" + QByteArray::number(SMP_SHIFT) + ") + 0x" + QByteArray::number(BMP_END / BMP_BLOCKSIZE, 16) + "]"
          " + (ucs4 & 0x" + QByteArray::number(SMP_BLOCKSIZE - 1, 16) + ")]))\n\n"
          "#define GET_PROP_INDEX_UCS2(ucs2) \\\n"
          "       (uc_property_trie[uc_property_trie[ucs2>>" + QByteArray::number(BMP_SHIFT) +
          "] + (ucs2 & 0x" + QByteArray::number(BMP_BLOCKSIZE - 1, 16) + ")])\n\n";

   out += "static const Properties uc_properties[] = {";

   // keep in sync with the property declaration

   // trip 1
   for (int i = 0; i < uniqueProperties.size(); ++i) {
      const PropertyFlags &p = uniqueProperties.at(i);

      out += "\n    { ";

      out += QByteArray::number( p.category );
      out += ", ";

      out += QByteArray::number( p.direction );
      out += ", ";

      out += QByteArray::number( p.combiningClass );
      out += ", ";

      out += QByteArray::number( p.joining );
      out += ", ";

      out += QByteArray::number( p.digitValue );
      out += ", ";

      out += QByteArray::number( p.mirrorDiff );
      out += ", ";

      out += QByteArray::number( p.age );
      out += ", ";

      out += QByteArray::number( p.nfQuickCheck );
      out += ", ";

      out += QByteArray::number( p.graphemeBreakClass );
      out += ", ";
      out += QByteArray::number( p.wordBreakClass );
      out += ", ";
      out += QByteArray::number( p.sentenceBreakClass );
      out += ", ";
      out += QByteArray::number( p.lineBreakClass );
      out += ", ";

      out += QByteArray::number( p.script );
      out += " },";
   }

   if (out.endsWith(',')) {
      out.chop(1);
   }
   out += "\n};\n\n";

   // trip 2
   out += "char32_t uc_caseFold(char32_t value)\n";
   out += "{\n   ";

   int oldDiff = 0;
   int count   = 1;

   for (int uc = 0; uc <= QChar::LastValidCodePoint; ++uc) {
      UnicodeData &rowData = UnicodeData::get_codePoint_data(uc);
      PropertyFlags &flags = rowData.pFlags;

      char32_t retval = uc + flags.caseFoldDiff;

      if (uc == 74) {
         // very special case for Turkish

         out += "if (value < " + QByteArray::number(uc) + " && cs_isTurkishLocale) {\n";
         out += "       // letter I, ignore for all other locales\n";
         out += "       return value + " + QByteArray::number(oldDiff) + ";\n\n";
         out += "    } else ";

         oldDiff = flags.caseFoldDiff;

      } else if (flags.caseFoldSpecial) {
         out += "if (value == " + QByteArray::number(uc) + ") {\n";
         out += "      // special char\n";
         out += "      return 0;\n\n";
         if (count % 100 == 0) {
            out += "   }\n\n";
            out += "   ";

         } else {
            out += "   } else ";

         }

         ++count;
      } else if (flags.caseFoldDiff != oldDiff) {
         out += "if (value < " + QByteArray::number(uc) + ") {\n";
         out += "      return value + " + QByteArray::number(oldDiff) + ";\n\n";
         if (count % 100 == 0) {
            out += "   }\n\n";
            out += "   ";

         } else {
            out += "   } else ";

         }
         oldDiff = flags.caseFoldDiff;
         ++count;
      }
   }

   out += "{\n";
   out += "      return value;\n";
   out += "   }\n";
   out += "}\n\n";


   // trip 3
   out += "char32_t uc_lowerCase(char32_t value)\n";
   out += "{\n   ";

   oldDiff = 0;
   count   = 1;

   for (int uc = 0; uc <= QChar::LastValidCodePoint; ++uc) {
      UnicodeData &rowData = UnicodeData::get_codePoint_data(uc);
      PropertyFlags &flags = rowData.pFlags;

      char32_t retval = uc + flags.lowerCaseDiff;

      if (flags.lowerCaseSpecial) {
         out += "if (value == " + QByteArray::number(uc) + ") {\n";
         out += "      // special char\n";
         out += "      return 0;\n\n";
         out += "   } else ";

      } else if (flags.lowerCaseDiff != oldDiff) {
         out += "if (value < " + QByteArray::number(uc) + ") {\n";
         out += "      return value + " + QByteArray::number(oldDiff) + ";\n\n";
         if (count % 100 == 0) {
            out += "   }\n\n";
            out += "   ";
         } else {
            out += "   } else ";

         }
         oldDiff = flags.lowerCaseDiff;
         ++count;
      }
   }

   out += "{\n";
   out += "      return value;\n";
   out += "   }\n";
   out += "}\n\n";


   // trip 4
   out += "char32_t uc_titleCase(char32_t value)\n";
   out += "{\n   ";

   oldDiff = 0;
   count   = 1;

   for (int uc = 0; uc <= QChar::LastValidCodePoint; ++uc) {
      UnicodeData &rowData = UnicodeData::get_codePoint_data(uc);
      PropertyFlags &flags = rowData.pFlags;

      char32_t retval = uc + flags.titleCaseDiff;

      if (flags.titleCaseSpecial) {
         out += "if (value == " + QByteArray::number(uc) + ") {\n";
         out += "      // special char\n";
         out += "      return 0;\n\n";
         out += "   } else ";

      } else if (flags.titleCaseDiff != oldDiff) {
         out += "if (value < " + QByteArray::number(uc) + ") {\n";
         out += "      return value + " + QByteArray::number(oldDiff) + ";\n\n";
         if (count % 100 == 0) {
            out += "   }\n\n";
            out += "   ";

         } else {
            out += "   } else ";

         }

         oldDiff = flags.titleCaseDiff;
         ++count;
      }
   }

   out += "{\n";
   out += "      return value;\n";
   out += "   }\n";
   out += "}\n\n";

   // trip 5
   out += "char32_t uc_upperCase(char32_t value)\n";
   out += "{\n    ";

   oldDiff = 0;
   count   = 1;

   for (int uc = 0; uc <= QChar::LastValidCodePoint; ++uc) {
      UnicodeData &rowData = UnicodeData::get_codePoint_data(uc);
      PropertyFlags &flags = rowData.pFlags;

      char32_t retval = uc + flags.upperCaseDiff;

      if (flags.upperCaseSpecial) {
         out += "if (value == " + QByteArray::number(uc) + ") {\n";
         out += "      // special char\n";
         out += "      return 0;\n\n";
         if (count % 100 == 0) {
            out += "   }\n\n";
            out += "   ";

         } else {
            out += "   } else ";

         }

         ++count;
      } else if (flags.upperCaseDiff != oldDiff) {
         out += "if (value < " + QByteArray::number(uc) + ") {\n";
         out += "      return value + " + QByteArray::number(oldDiff) + ";\n\n";
         if (count % 100 == 0) {
            out += "   }\n\n";
            out += "   ";

         } else {
            out += "   } else ";

         }

         oldDiff = flags.upperCaseDiff;
         ++count;
      }
   }

   out += "{\n";
   out += "      return value;\n";
   out += "   }\n";
   out += "}\n\n";

   out += "static inline const Properties *qGetProp(uint ucs4) \n"
          "{\n"
          "    return uc_properties + GET_PROP_INDEX(ucs4);\n"
          "}\n"
          "\n"
          "static inline const Properties *qGetProp(ushort ucs2) \n"
          "{\n"
          "    return uc_properties + GET_PROP_INDEX_UCS2(ucs2);\n"
          "}\n"
          "\n"
          "Q_CORE_EXPORT const Properties * properties(uint ucs4) \n"
          "{\n"
          "    return qGetProp(ucs4);\n"
          "}\n"
          "\n"
          "Q_CORE_EXPORT const Properties * properties(ushort ucs2) \n"
          "{\n"
          "    return qGetProp(ucs2);\n"
          "}\n\n";

   out += "Q_CORE_EXPORT GraphemeBreakClass graphemeBreakClass(uint ucs4) \n"
          "{\n"
          "    return static_cast<GraphemeBreakClass>(qGetProp(ucs4)->graphemeBreakClass);\n"
          "}\n"
          "\n"
          "Q_CORE_EXPORT WordBreakClass wordBreakClass(uint ucs4) \n"
          "{\n"
          "    return static_cast<WordBreakClass>(qGetProp(ucs4)->wordBreakClass);\n"
          "}\n"
          "\n"
          "Q_CORE_EXPORT SentenceBreakClass sentenceBreakClass(uint ucs4) \n"
          "{\n"
          "    return static_cast<SentenceBreakClass>(qGetProp(ucs4)->sentenceBreakClass);\n"
          "}\n"
          "\n"
          "Q_CORE_EXPORT LineBreakClass lineBreakClass(uint ucs4) \n"
          "{\n"
          "    return static_cast<LineBreakClass>(qGetProp(ucs4)->lineBreakClass);\n"
          "}\n"
          "\n";

   return out;
}

static QByteArray create_SpecialCaseTables()
{
   qDebug("createSpecialCase Functions");

   QByteArray out;

   // trip 1
   out += "const char32_t *uc_caseFoldSpecial(char32_t value)\n";
   out += "{\n    ";

   for (int uc = 0; uc <= QChar::LastValidCodePoint; ++uc) {
      UnicodeData &rowData = UnicodeData::get_codePoint_data(uc);
      PropertyFlags &flags = rowData.pFlags;

      if (flags.caseFoldSpecial) {
         out += "if (value == " + QByteArray::number(uc) + ") {\n";
         out += "      return U\"";

         for (auto item : rowData.pFlags.specialFlags->caseFoldList) {
            out += "\\U" + QByteArray::number(item, 16).rightJustified(8, '0');
         }

         out += "\";\n\n";
         out += "    } else ";
      }
   }

   out += "{\n";
   out += "      return nullptr;\n";
   out += "   }\n";
   out += "}\n\n";


   // trip 2
   out += "const char32_t *uc_lowerCaseSpecial(char32_t value)\n";
   out += "{\n    ";

   for (int uc = 0; uc <= QChar::LastValidCodePoint; ++uc) {
      UnicodeData &rowData = UnicodeData::get_codePoint_data(uc);
      PropertyFlags &flags = rowData.pFlags;

      if (flags.lowerCaseSpecial) {
         out += "if (value == " + QByteArray::number(uc) + ") {\n";
         out += "      return U\"";

         for (auto item : rowData.pFlags.specialFlags->lowerCaseList) {
            out += "\\U" + QByteArray::number(item, 16).rightJustified(8, '0');
         }

         out += "\";\n\n";
         out += "    } else ";
      }
   }

   out += "{\n";
   out += "      return nullptr;\n";
   out += "   }\n";
   out += "}\n\n";


   // trip 3
   out += "const char32_t *uc_titleCaseSpecial(char32_t value)\n";
   out += "{\n    ";

   for (int uc = 0; uc <= QChar::LastValidCodePoint; ++uc) {
      UnicodeData &rowData = UnicodeData::get_codePoint_data(uc);
      PropertyFlags &flags = rowData.pFlags;

     if (flags.titleCaseSpecial) {
         out += "if (value == " + QByteArray::number(uc) + ") {\n";
         out += "      return U\"";

         for (auto item : rowData.pFlags.specialFlags->titleCaseList) {
            out += "\\U" + QByteArray::number(item, 16).rightJustified(8, '0');
         }

         out += "\";\n\n";
         out += "    } else ";
      }
   }

   out += "{\n";
   out += "      return nullptr;\n";
   out += "   }\n";
   out += "}\n\n";


   // trip 4
   out += "const char32_t *uc_upperCaseSpecial(char32_t value)\n";
   out += "{\n    ";

   for (int uc = 0; uc <= QChar::LastValidCodePoint; ++uc) {
      UnicodeData &rowData = UnicodeData::get_codePoint_data(uc);
      PropertyFlags &flags = rowData.pFlags;

     if (flags.upperCaseSpecial) {
         out += "if (value == " + QByteArray::number(uc) + ") {\n";
         out += "      return U\"";

         for (auto item : rowData.pFlags.specialFlags->upperCaseList) {
            out += "\\U" + QByteArray::number(item, 16).rightJustified(8, '0');
         }

         out += "\";\n\n";
         out += "    } else ";
      }
   }

   out += "{\n";
   out += "      return nullptr;\n";
   out += "   }\n";
   out += "}\n\n";

   return out;
}

static std::pair<QByteArray, QByteArray> createCompositionInfo()
{
   qDebug("createCompositionInfo: highestComposedCharacter = 0x%x", highestComposedCharacter);

   const int BMP_BLOCKSIZE = 16;
   const int BMP_SHIFT     = 4;
   const int BMP_END       = 0x3400;       // start of Han
   const int SMP_END       = 0x30000;
   const int SMP_BLOCKSIZE = 256;
   const int SMP_SHIFT     = 8;

   if (SMP_END <= highestComposedCharacter) {
      qFatal("End of table smaller than highest composed character 0x%x", highestComposedCharacter);
   }

   QVector<char32_t> decompositionList;
   int tableIndex = 0;

   QList<UniqueBlock> uniqueBlocks;
   QVector<int> blockMap;
   int used = 0;

   for (int block = 0; block < BMP_END / BMP_BLOCKSIZE; ++block) {
      UniqueBlock b;
      b.values.reserve(BMP_BLOCKSIZE);

      for (int i = 0; i < BMP_BLOCKSIZE; ++i) {
         int uc = block * BMP_BLOCKSIZE + i;
         UnicodeData &d = UnicodeData::get_codePoint_data(uc);

         if (! d.decomposition.isEmpty()) {
            int utf16Length = 0;
            decompositionList.append(0);

            for (int j = 0; j < d.decomposition.size(); ++j) {
               char32_t code = d.decomposition.at(j);

               decompositionList.append(code);
               utf16Length++;
            }

            decompositionList[tableIndex] = d.decompositionType + (utf16Length << 8);
            b.values.append(tableIndex);
            tableIndex += utf16Length + 1;

         } else {
            b.values.append(0xffff);
         }
      }

      int index = uniqueBlocks.indexOf(b);
      if (index == -1) {
         index   = uniqueBlocks.size();
         b.index = used;
         used += BMP_BLOCKSIZE;
         uniqueBlocks.append(b);
      }

      blockMap.append(uniqueBlocks.at(index).index);
   }
   int bmp_blocks = uniqueBlocks.size();

   for (int block = BMP_END / SMP_BLOCKSIZE; block < SMP_END / SMP_BLOCKSIZE; ++block) {
      UniqueBlock b;
      b.values.reserve(SMP_BLOCKSIZE);

      for (int i = 0; i < SMP_BLOCKSIZE; ++i) {
         int uc = block * SMP_BLOCKSIZE + i;
         UnicodeData &d = UnicodeData::get_codePoint_data(uc);

         if (! d.decomposition.isEmpty()) {
            int utf16Length = 0;
            decompositionList.append(0);

            for (int j = 0; j < d.decomposition.size(); ++j) {
               char32_t code = d.decomposition.at(j);

               decompositionList.append(code);
               utf16Length++;
            }

            decompositionList[tableIndex] = d.decompositionType + (utf16Length << 8);
            b.values.append(tableIndex);
            tableIndex += utf16Length + 1;

         } else {
            b.values.append(0xffff);
         }
      }

      int index = uniqueBlocks.indexOf(b);
      if (index == -1) {
         index   = uniqueBlocks.size();
         b.index = used;
         used    += SMP_BLOCKSIZE;
         uniqueBlocks.append(b);
      }
      blockMap.append(uniqueBlocks.at(index).index);
   }
   int smp_blocks = uniqueBlocks.size() - bmp_blocks;

   // if the condition below does not hold anymore, modify our decomposition code
   Q_ASSERT(tableIndex < 0xffff);

   int bmp_block_data = bmp_blocks * BMP_BLOCKSIZE * sizeof(unsigned short);
   int bmp_trie = BMP_END / BMP_BLOCKSIZE * sizeof(unsigned short);
   int bmp_mem = bmp_block_data + bmp_trie;
   qDebug("    %d unique blocks in BMP", bmp_blocks);
   qDebug("        block data uses: %d bytes", bmp_block_data);
   qDebug("        trie data uses : %d bytes", bmp_trie);

   int smp_block_data = smp_blocks * SMP_BLOCKSIZE * sizeof(unsigned short);
   int smp_trie = (SMP_END - BMP_END) / SMP_BLOCKSIZE * sizeof(unsigned short);
   int smp_mem = smp_block_data + smp_trie;
   qDebug("    %d unique blocks in SMP", smp_blocks);
   qDebug("        block data uses: %d bytes", smp_block_data);
   qDebug("        trie data uses : %d bytes", smp_trie);

   int decomposition_data = decompositionList.size() * 2;
   qDebug("        decomposition data uses : %d bytes", decomposition_data);
   qDebug("    memory usage: %d bytes\n", bmp_mem + smp_mem + decomposition_data);

   Q_ASSERT(blockMap.last() + blockMap.size() < (1 << (sizeof(unsigned short) * 8)));

   QByteArray out_cpp;
   QByteArray out_h;

   out_cpp += "const unsigned short uc_decomposition_trie[] = {\n";

   // first write the map
   out_cpp += "    // 0 - 0x" + QByteArray::number(BMP_END, 16);

   for (int i = 0; i < BMP_END / BMP_BLOCKSIZE; ++i) {
      if (!(i % 8)) {
         if (out_cpp.endsWith(' ')) {
            out_cpp.chop(1);
         }
         if (!((i * BMP_BLOCKSIZE) % 0x1000)) {
            out_cpp += "\n";
         }
         out_cpp += "\n    ";
      }
      out_cpp += QByteArray::number(blockMap.at(i) + blockMap.size());
      out_cpp += ", ";
   }

   if (out_cpp.endsWith(' '))  {
      out_cpp.chop(1);
   }

   out_cpp += "\n\n    // 0x" + QByteArray::number(BMP_END, 16) + " - 0x" + QByteArray::number(SMP_END, 16) + "\n";

   for (int i = BMP_END / BMP_BLOCKSIZE; i < blockMap.size(); ++i) {
      if (! (i % 8)) {
         if (out_cpp.endsWith(' ')) {
            out_cpp.chop(1);
         }

         if (! (i % (0x10000 / SMP_BLOCKSIZE))) {
            out_cpp += "\n";
         }
         out_cpp += "\n    ";
      }

      out_cpp += QByteArray::number(blockMap.at(i) + blockMap.size());
      out_cpp += ", ";
   }

   if (out_cpp.endsWith(' ')) {
      out_cpp.chop(1);
   }

   out_cpp += "\n";

   // write the data
   for (int i = 0; i < uniqueBlocks.size(); ++i) {
      if (out_cpp.endsWith(' ')) {
         out_cpp.chop(1);
      }

      out_cpp += "\n";
      const UniqueBlock &b = uniqueBlocks.at(i);

      for (int j = 0; j < b.values.size(); ++j) {
         if (! (j % 8)) {
            if (out_cpp.endsWith(' ')) {
               out_cpp.chop(1);
            }
            out_cpp += "\n    ";
         }
         out_cpp += "0x" + QByteArray::number(b.values.at(j), 16);
         out_cpp += ", ";
      }
   }

   if (out_cpp.endsWith(' ')) {
      out_cpp.chop(2);
   }
   out_cpp += "\n};\n\n";

   out_h += "#define GET_DECOMPOSITION_INDEX(ucs4) \\\n"
          "       (ucs4 < 0x" + QByteArray::number(BMP_END, 16) + " \\\n"
          "        ? (QUnicodeTables::uc_decomposition_trie[QUnicodeTables::uc_decomposition_trie[ucs4 >> " + QByteArray::number(BMP_SHIFT) +
          "] + (ucs4 & 0x" + QByteArray::number(BMP_BLOCKSIZE - 1, 16) + ")]) \\\n"
          "        : (ucs4 < 0x" + QByteArray::number(SMP_END, 16) + " \\\n"
          "        ? QUnicodeTables::uc_decomposition_trie[QUnicodeTables::uc_decomposition_trie[((ucs4 - 0x" + QByteArray::number(BMP_END, 16) +
          ") >> " + QByteArray::number(SMP_SHIFT) + ") + 0x" + QByteArray::number(BMP_END / BMP_BLOCKSIZE, 16) + "] + \\\n"
          "        (ucs4 & 0x" + QByteArray::number(SMP_BLOCKSIZE - 1, 16) + ")] : 0xffff))\n\n";

   out_cpp += "const char32_t uc_decomposition_map[] = {";

   for (int i = 0; i < decompositionList.size(); ++i) {
      if (! (i % 8)) {
         if (out_cpp.endsWith(' ')) {
            out_cpp.chop(1);
         }
         out_cpp += "\n    ";
      }

      out_cpp += "0x" + QByteArray::number(decompositionList.at(i), 16);
      out_cpp += ", ";
   }

   if (out_cpp.endsWith(' ')) {
      out_cpp.chop(2);
   }

   out_cpp += "\n};\n\n";

   return { out_cpp, out_h };
}

static std::pair<QByteArray, QByteArray> createLigatureInfo()
{
   qDebug("createLigatureInfo: numLigatures = %d, highestLigature = 0x%x", numLigatures, highestLigature);

   const int BMP_BLOCKSIZE = 32;
   const int BMP_SHIFT     = 5;
   const int BMP_END       = 0x3100;
   const int SMP_END       = 0x12000;
   const int SMP_BLOCKSIZE = 256;
   const int SMP_SHIFT     = 8;

   if (SMP_END <= highestLigature) {
      qFatal("end of table smaller than highest ligature character 0x%x", highestLigature);
   }

   QList<char32_t> ligatures;
   int tableIndex = 0;

   QList<UniqueBlock> uniqueBlocks;
   QVector<int> blockMap;
   int used = 0;

   for (int block = 0; block < BMP_END / BMP_BLOCKSIZE; ++block) {
      UniqueBlock b;
      b.values.reserve(BMP_BLOCKSIZE);

      for (int i = 0; i < BMP_BLOCKSIZE; ++i) {
         int uc = block * BMP_BLOCKSIZE + i;
         QList<Ligature> list = ligatureHashes.value(uc);

         if (list.isEmpty()) {
            b.values.append(0xffff);

         } else {
            // ligaturers must be sorted
            std::sort(list.begin(), list.end());

            ligatures.append(list.size());

            for (int j = 0; j < list.size(); ++j) {
               ligatures.append(list.at(j).u1);
               ligatures.append(list.at(j).ligature);
            }

            b.values.append(tableIndex);
            tableIndex += 2 * list.size() + 1;
         }
      }

      int index = uniqueBlocks.indexOf(b);
      if (index == -1) {
         index   = uniqueBlocks.size();
         b.index = used;
         used    += BMP_BLOCKSIZE;
         uniqueBlocks.append(b);
      }
      blockMap.append(uniqueBlocks.at(index).index);
   }
   int bmp_blocks = uniqueBlocks.size();

   for (int block = BMP_END / SMP_BLOCKSIZE; block < SMP_END / SMP_BLOCKSIZE; ++block) {
      UniqueBlock b;
      b.values.reserve(SMP_BLOCKSIZE);

      for (int i = 0; i < SMP_BLOCKSIZE; ++i) {

         int uc = block*SMP_BLOCKSIZE + i;
         QList<Ligature> list = ligatureHashes.value(uc);

         if (! list.isEmpty()) {
            b.values.append(0xffff);

         } else {
            // ligaturers must be sorted

             std::sort(list.begin(), list.end());
             ligatures.append(list.size());

             for (int j = 0; j < list.size(); ++j) {
                 ligatures.append( list.at(j).u1 );
                 ligatures.append( list.at(j).ligature);
             }

             b.values.append(tableIndex);
             tableIndex += 2 * list.size() + 1;
         }
      }

      int index = uniqueBlocks.indexOf(b);

      if (index == -1) {
         index   = uniqueBlocks.size();
         b.index = used;
         used    += SMP_BLOCKSIZE;
         uniqueBlocks.append(b);
      }
      blockMap.append(uniqueBlocks.at(index).index);
   }

   int smp_blocks = uniqueBlocks.size() - bmp_blocks;

   // if the condition below doesn't hold anymore we need to modify our composition code
   Q_ASSERT(tableIndex < 0xffff);

   int bmp_block_data = bmp_blocks * BMP_BLOCKSIZE * sizeof(unsigned short);
   int bmp_trie = BMP_END / BMP_BLOCKSIZE * sizeof(unsigned short);
   int bmp_mem  = bmp_block_data + bmp_trie;
   qDebug("    %d unique blocks in BMP", bmp_blocks);
   qDebug("        block data uses: %d bytes", bmp_block_data);
   qDebug("        trie data uses : %d bytes", bmp_trie);

   int smp_block_data = smp_blocks * SMP_BLOCKSIZE * sizeof(unsigned short);
   int smp_trie = (SMP_END - BMP_END) / SMP_BLOCKSIZE * sizeof(unsigned short);
   int smp_mem  = smp_block_data + smp_trie;
   qDebug("    %d unique blocks in SMP", smp_blocks);
   qDebug("        block data uses: %d bytes", smp_block_data);
   qDebug("        trie data uses : %d bytes", smp_trie);

   int ligature_data = ligatures.size() * 2;
   qDebug("        ligature data uses : %d bytes", ligature_data);
   qDebug("    memory usage: %d bytes\n", bmp_mem + smp_mem + ligature_data);

   Q_ASSERT(blockMap.last() + blockMap.size() < (1 << (sizeof(unsigned short) * 8)));

   QByteArray out_cpp;
   QByteArray out_h;

   out_cpp += "const unsigned short uc_ligature_trie[] = {\n";

   // first write the map
   out_cpp += "    // 0 - 0x" + QByteArray::number(BMP_END, 16);

   for (int i = 0; i < BMP_END / BMP_BLOCKSIZE; ++i) {
      if (! (i % 8)) {
         if (out_cpp.endsWith(' ')) {
            out_cpp.chop(1);
         }

         if (! ((i * BMP_BLOCKSIZE) % 0x1000)) {
            out_cpp += "\n";
         }

         out_cpp += "\n    ";
      }

      out_cpp += QByteArray::number(blockMap.at(i) + blockMap.size());
      out_cpp += ", ";
   }

   if (out_cpp.endsWith(' ')) {
      out_cpp.chop(1);
   }

   out_cpp += "\n\n    // 0x" + QByteArray::number(BMP_END, 16) + " - 0x" + QByteArray::number(SMP_END, 16) + "\n";

   for (int i = BMP_END / BMP_BLOCKSIZE; i < blockMap.size(); ++i) {
      if (! (i % 8)) {
         if (out_cpp.endsWith(' ')) {
            out_cpp.chop(1);
         }

         if (!(i % (0x10000 / SMP_BLOCKSIZE))) {
            out_cpp += "\n";
         }
         out_cpp += "\n    ";
      }
      out_cpp += QByteArray::number(blockMap.at(i) + blockMap.size());
      out_cpp += ", ";
   }

   if (out_cpp.endsWith(' ')) {
      out_cpp.chop(1);
   }

   out_cpp += "\n";

   // write the data
   for (int i = 0; i < uniqueBlocks.size(); ++i) {
      if (out_cpp.endsWith(' ')) {
         out_cpp.chop(1);
      }

      out_cpp += "\n";
      const UniqueBlock &b = uniqueBlocks.at(i);

      for (int j = 0; j < b.values.size(); ++j) {
         if (! (j % 8)) {
            if (out_cpp.endsWith(' ')) {
               out_cpp.chop(1);
            }

            out_cpp += "\n    ";
         }

         out_cpp += "0x" + QByteArray::number(b.values.at(j), 16);
         out_cpp += ", ";
      }
   }

   if (out_cpp.endsWith(' ')) {
      out_cpp.chop(2);
   }
   out_cpp += "\n};\n\n";

   out_h += "   #define GET_LIGATURE_INDEX(ucs4) \\\n"
          "       (ucs4 < 0x" + QByteArray::number(BMP_END, 16) + " \\\n"
          "        ? (QUnicodeTables::uc_ligature_trie[QUnicodeTables::uc_ligature_trie[ucs4 >> " + QByteArray::number(BMP_SHIFT) +
          "] + (ucs4 & 0x" + QByteArray::number(BMP_BLOCKSIZE - 1, 16) + ")]) \\\n"
          "        : (ucs4 < 0x" + QByteArray::number(SMP_END, 16) + " \\\n"
          "        ? QUnicodeTables::uc_ligature_trie[QUnicodeTables::uc_ligature_trie[((ucs4 - 0x" + QByteArray::number(BMP_END, 16) +
          ") >> " + QByteArray::number(SMP_SHIFT) + ") + 0x" + QByteArray::number(BMP_END / BMP_BLOCKSIZE, 16) + "] + \\\n"
          "        (ucs4 & 0x" + QByteArray::number(SMP_BLOCKSIZE - 1, 16) + ")] : 0xffff))\n\n";

   out_cpp += "const char32_t uc_ligature_map[] = {";

   for (int i = 0; i < ligatures.size(); ++i) {
      if (! (i % 8)) {
         if (out_cpp.endsWith(' ')) {
            out_cpp.chop(1);
         }
         out_cpp += "\n    ";
      }
      out_cpp += "0x" + QByteArray::number(ligatures.at(i), 16);
      out_cpp += ", ";
   }

   if (out_cpp.endsWith(' ')) {
      out_cpp.chop(2);
   }

   out_cpp += "\n};\n\n";

   return { out_cpp, out_h };
}

QByteArray createCasingInfo()
{
   QByteArray out;

   out += "struct CasingInfo {\n"
          "    uint codePoint : 16;\n"
          "    uint flags : 8;\n"
          "    uint offset : 8;\n"
          "};\n\n";

   return out;
}


int main(int, char **)
{
   qDebug("\nGenerating Unicode Tables - Begin Proccesing");

   initAgeMap();
   initCategoryMap();
   initDecompositionMap();
   initDirectionMap();
   initJoiningMap();
   initGraphemeBreak();
   initWordBreak();
   initSentenceBreak();
   initLineBreak();
   initScriptMap();

   readUnicodeData();
   readBidiMirroring();
   readArabicShaping();
   readDerivedAge();
   readDerivedNormalizationProps();
   readSpecialCasing();
   readCaseFolding();
   // readBlocks();
   readScripts();
   readGraphemeBreak();
   readWordBreak();
   readSentenceBreak();
   readLineBreak();

   computeUniqueProperties();
   QByteArray property_tables    = create_PropertyTables();
   QByteArray specialCase_tables = create_SpecialCaseTables();

   std::pair<QByteArray, QByteArray> compositions             = createCompositionInfo();
   std::pair<QByteArray, QByteArray> ligatures                = createLigatureInfo();
   std::pair<QByteArray, QByteArray> normalizationCorrections = createNormalizationCorrections();

   QByteArray header =
      "/***********************************************************************\n"
      "*\n"
      "* Copyright (c) 2012-2024 Barbara Geller\n"
      "* Copyright (c) 2012-2024 Ansel Sermersheim\n"
      "*\n"
      "* Copyright (c) 2015 The Qt Company Ltd.\n"
      "* Copyright (c) 2012-2016 Digia Plc and/or its subsidiary(-ies).\n"
      "* Copyright (c) 2008-2012 Nokia Corporation and/or its subsidiary(-ies).\n"
      "*\n"
      "* This file is part of CopperSpice.\n"
      "*\n"
      "* CopperSpice is free software. You can redistribute it and/or\n"
      "* modify it under the terms of the GNU Lesser General Public License\n"
      "* version 2.1 as published by the Free Software Foundation.\n"
      "*\n"
      "* CopperSpice is distributed in the hope that it will be useful,\n"
      "* but WITHOUT ANY WARRANTY; without even the implied warranty of\n"
      "* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n"
      "*\n"
      "* http://www.gnu.org/licenses/\n"
      "*\n"
      "***********************************************************************/\n\n";

   QByteArray note = "/* File autogenerated from the Unicode " DATA_VERSION_S " database. Do not edit */\n\n";

   // file one
   QFile f("qunicodetables.cpp");
   f.open(QFile::WriteOnly | QFile::Truncate);

   f.write(header);
   f.write(note);

   f.write("#include \"qunicodetables_p.h\"\n\n");
   f.write("namespace QUnicodeTables {\n\n");

   f.write("bool cs_isTurkishLocale = false;\n\n");

   f.write(property_tables);
   f.write("\n");
   f.write(specialCase_tables);
   f.write("\n");

   f.write(compositions.first);
   f.write(ligatures.first);

   f.write(normalizationCorrections.first);
   f.write("} // namespace QUnicodeTables\n\n");
   f.close();


   // file two
   f.setFileName("qunicodetables_p.h");
   f.open(QFile::WriteOnly | QFile::Truncate);

   f.write(header);
   f.write(note);

   f.write("#ifndef QUNICODETABLES_P_H\n"
           "#define QUNICODETABLES_P_H\n\n"
           "#include <qchar.h>\n\n");

   f.write("#define UNICODE_DATA_VERSION " DATA_VERSION_STR "\n\n");
   f.write("namespace QUnicodeTables {\n\n");

   f.write("extern bool cs_isTurkishLocale;\n\n");

   f.write(normalizationCorrections.second);
   f.write(property_string);

   f.write(compositions.second);
   f.write(ligatures.second);

   f.write(grapheme_break_class_string);
   f.write(word_break_class_string);
   f.write(sentence_break_class_string);
   f.write(line_break_class_string);
   f.write(breakMethods);

   f.write("}  // namespace \n\n");
   f.write(hangulConstants);

   f.write("#endif\n");
   f.close();

   qDebug() << "maxMirroredDiff  = " << hex << maxMirroredDiff;
   qDebug("\nGenerating Unicode Tables - Completed");

   return 0;
}
