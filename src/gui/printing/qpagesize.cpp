/***********************************************************************
*
* Copyright (c) 2012-2020 Barbara Geller
* Copyright (c) 2012-2020 Ansel Sermersheim
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

#include <qpagesize.h>

#include <qcoreapplication.h>
#include <qdebug.h>
#include <qpoint.h>
#include <qrect.h>
#include <qsize.h>
#include <qstring.h>

// Define the Windows DMPAPER sizes for use in the look-up table
// See http://msdn.microsoft.com/en-us/library/windows/desktop/dd319099.aspx

enum WindowsDmPaper {
   DMPAPER_NONE                            =   0,  // Not a DMPAPER, use for sizes without a DMPAPER value
   DMPAPER_LETTER                          =   1,
   DMPAPER_LETTERSMALL                     =   2,
   DMPAPER_TABLOID                         =   3,
   DMPAPER_LEDGER                          =   4,
   DMPAPER_LEGAL                           =   5,
   DMPAPER_STATEMENT                       =   6,
   DMPAPER_EXECUTIVE                       =   7,
   DMPAPER_A3                              =   8,
   DMPAPER_A4                              =   9,
   DMPAPER_A4SMALL                         =  10,
   DMPAPER_A5                              =  11,
   DMPAPER_B4                              =  12,
   DMPAPER_B5                              =  13,
   DMPAPER_FOLIO                           =  14,
   DMPAPER_QUARTO                          =  15,
   DMPAPER_10X14                           =  16,
   DMPAPER_11X17                           =  17,
   DMPAPER_NOTE                            =  18,
   DMPAPER_ENV_9                           =  19,
   DMPAPER_ENV_10                          =  20,
   DMPAPER_ENV_11                          =  21,
   DMPAPER_ENV_12                          =  22,
   DMPAPER_ENV_14                          =  23,
   DMPAPER_CSHEET                          =  24,
   DMPAPER_DSHEET                          =  25,
   DMPAPER_ESHEET                          =  26,
   DMPAPER_ENV_DL                          =  27,
   DMPAPER_ENV_C5                          =  28,
   DMPAPER_ENV_C3                          =  29,
   DMPAPER_ENV_C4                          =  30,
   DMPAPER_ENV_C6                          =  31,
   DMPAPER_ENV_C65                         =  32,
   DMPAPER_ENV_B4                          =  33,
   DMPAPER_ENV_B5                          =  34,
   DMPAPER_ENV_B6                          =  35,
   DMPAPER_ENV_ITALY                       =  36,
   DMPAPER_ENV_MONARCH                     =  37,
   DMPAPER_ENV_PERSONAL                    =  38,
   DMPAPER_FANFOLD_US                      =  39,
   DMPAPER_FANFOLD_STD_GERMAN              =  40,
   DMPAPER_FANFOLD_LGL_GERMAN              =  41,
   DMPAPER_ISO_B4                          =  42,
   DMPAPER_JAPANESE_POSTCARD               =  43,
   DMPAPER_9X11                            =  44,
   DMPAPER_10X11                           =  45,
   DMPAPER_15X11                           =  46,
   DMPAPER_ENV_INVITE                      =  47,
   DMPAPER_RESERVED_48                     =  48,
   DMPAPER_RESERVED_49                     =  49,
   DMPAPER_LETTER_EXTRA                    =  50,
   DMPAPER_LEGAL_EXTRA                     =  51,
   DMPAPER_TABLOID_EXTRA                   =  52,
   DMPAPER_A4_EXTRA                        =  53,
   DMPAPER_LETTER_TRANSVERSE               =  54,
   DMPAPER_A4_TRANSVERSE                   =  55,
   DMPAPER_LETTER_EXTRA_TRANSVERSE         =  56,
   DMPAPER_A_PLUS                          =  57,
   DMPAPER_B_PLUS                          =  58,
   DMPAPER_LETTER_PLUS                     =  59,
   DMPAPER_A4_PLUS                         =  60,
   DMPAPER_A5_TRANSVERSE                   =  61,
   DMPAPER_B5_TRANSVERSE                   =  62,
   DMPAPER_A3_EXTRA                        =  63,
   DMPAPER_A5_EXTRA                        =  64,
   DMPAPER_B5_EXTRA                        =  65,
   DMPAPER_A2                              =  66,
   DMPAPER_A3_TRANSVERSE                   =  67,
   DMPAPER_A3_EXTRA_TRANSVERSE             =  68,
   DMPAPER_DBL_JAPANESE_POSTCARD           =  69,
   DMPAPER_A6                              =  70,
   DMPAPER_JENV_KAKU2                      =  71,
   DMPAPER_JENV_KAKU3                      =  72,
   DMPAPER_JENV_CHOU3                      =  73,
   DMPAPER_JENV_CHOU4                      =  74,
   DMPAPER_LETTER_ROTATED                  =  75,
   DMPAPER_A3_ROTATED                      =  76,
   DMPAPER_A4_ROTATED                      =  77,
   DMPAPER_A5_ROTATED                      =  78,
   DMPAPER_B4_JIS_ROTATED                  =  79,
   DMPAPER_B5_JIS_ROTATED                  =  80,
   DMPAPER_JAPANESE_POSTCARD_ROTATED       =  81,
   DMPAPER_DBL_JAPANESE_POSTCARD_ROTATED   =  82,
   DMPAPER_A6_ROTATED                      =  83,
   DMPAPER_JENV_KAKU2_ROTATED              =  84,
   DMPAPER_JENV_KAKU3_ROTATED              =  85,
   DMPAPER_JENV_CHOU3_ROTATED              =  86,
   DMPAPER_JENV_CHOU4_ROTATED              =  87,
   DMPAPER_B6_JIS                          =  88,
   DMPAPER_B6_JIS_ROTATED                  =  89,
   DMPAPER_12X11                           =  90,
   DMPAPER_JENV_YOU4                       =  91,
   DMPAPER_JENV_YOU4_ROTATED               =  92,
   DMPAPER_P16K                            =  93,
   DMPAPER_P32K                            =  94,
   DMPAPER_P32KBIG                         =  95,
   DMPAPER_PENV_1                          =  96,
   DMPAPER_PENV_2                          =  97,
   DMPAPER_PENV_3                          =  98,
   DMPAPER_PENV_4                          =  99,
   DMPAPER_PENV_5                          = 100,
   DMPAPER_PENV_6                          = 101,
   DMPAPER_PENV_7                          = 102,
   DMPAPER_PENV_8                          = 103,
   DMPAPER_PENV_9                          = 104,
   DMPAPER_PENV_10                         = 105,
   DMPAPER_P16K_ROTATED                    = 106,
   DMPAPER_P32K_ROTATED                    = 107,
   DMPAPER_P32KBIG_ROTATED                 = 108,
   DMPAPER_PENV_1_ROTATED                  = 109,
   DMPAPER_PENV_2_ROTATED                  = 110,
   DMPAPER_PENV_3_ROTATED                  = 111,
   DMPAPER_PENV_4_ROTATED                  = 112,
   DMPAPER_PENV_5_ROTATED                  = 113,
   DMPAPER_PENV_6_ROTATED                  = 114,
   DMPAPER_PENV_7_ROTATED                  = 115,
   DMPAPER_PENV_8_ROTATED                  = 116,
   DMPAPER_PENV_9_ROTATED                  = 117,
   DMPAPER_PENV_10_ROTATED                 = 118,
   DMPAPER_LAST                            = DMPAPER_PENV_10_ROTATED,
   DMPAPER_USER                            = 256
};

// Conversion table for historic page size values that we don't support.
// These are deprecated in PPD and strongly discouraged from being used,
// so convert them to usable page sizes to support older print devices.
// The paper source orientation will be handled in the QPrintMedia class,
// we're only concerned about the standard size in QPageSize.
// _ROTATED = 90 degrees or QPageLayout::Landscape
// _TRANSVERSE = 180 degrees or QPageLayout::ReversePortrait

static const int qt_windowsConversion[][2] = {
   {DMPAPER_11X17,                         DMPAPER_TABLOID},  // = DMPAPER_LEDGER rotated
   {DMPAPER_A3_EXTRA_TRANSVERSE,           DMPAPER_A3_EXTRA},
   {DMPAPER_A3_ROTATED,                    DMPAPER_A3},
   {DMPAPER_A3_TRANSVERSE,                 DMPAPER_A3},
   {DMPAPER_A4_ROTATED,                    DMPAPER_A4},
   {DMPAPER_A4_TRANSVERSE,                 DMPAPER_A4},
   {DMPAPER_A5_ROTATED,                    DMPAPER_A5},
   {DMPAPER_A5_TRANSVERSE,                 DMPAPER_A5},
   {DMPAPER_A6_ROTATED,                    DMPAPER_A6},
   {DMPAPER_B4_JIS_ROTATED,                DMPAPER_B4},
   {DMPAPER_B5_JIS_ROTATED,                DMPAPER_B5},
   {DMPAPER_B5_TRANSVERSE,                 DMPAPER_B5},
   {DMPAPER_B6_JIS_ROTATED,                DMPAPER_B6_JIS},
   {DMPAPER_DBL_JAPANESE_POSTCARD_ROTATED, DMPAPER_DBL_JAPANESE_POSTCARD},
   {DMPAPER_JAPANESE_POSTCARD_ROTATED,     DMPAPER_JAPANESE_POSTCARD},
   {DMPAPER_JENV_CHOU3_ROTATED,            DMPAPER_JENV_CHOU3},
   {DMPAPER_JENV_CHOU4_ROTATED,            DMPAPER_JENV_CHOU4},
   {DMPAPER_JENV_KAKU2_ROTATED,            DMPAPER_JENV_KAKU2},
   {DMPAPER_JENV_KAKU3_ROTATED,            DMPAPER_JENV_KAKU3},
   {DMPAPER_JENV_YOU4_ROTATED,             DMPAPER_JENV_YOU4},
   {DMPAPER_LETTER_EXTRA_TRANSVERSE,       DMPAPER_LETTER_EXTRA},
   {DMPAPER_LETTER_ROTATED,                DMPAPER_LETTER},
   {DMPAPER_LETTER_TRANSVERSE,             DMPAPER_LETTER},
   {DMPAPER_P16K_ROTATED,                  DMPAPER_P16K},
   {DMPAPER_P32K_ROTATED,                  DMPAPER_P32K},
   {DMPAPER_P32KBIG_ROTATED,               DMPAPER_P32KBIG},
   {DMPAPER_PENV_1_ROTATED,                DMPAPER_PENV_1},
   {DMPAPER_PENV_2_ROTATED,                DMPAPER_PENV_2},
   {DMPAPER_PENV_3_ROTATED,                DMPAPER_PENV_3},
   {DMPAPER_PENV_4_ROTATED,                DMPAPER_PENV_4},
   {DMPAPER_PENV_5_ROTATED,                DMPAPER_PENV_5},
   {DMPAPER_PENV_6_ROTATED,                DMPAPER_PENV_6},
   {DMPAPER_PENV_7_ROTATED,                DMPAPER_PENV_7},
   {DMPAPER_PENV_8_ROTATED,                DMPAPER_PENV_8},
   {DMPAPER_PENV_9_ROTATED,                DMPAPER_PENV_9},
   {DMPAPER_PENV_10_ROTATED,               DMPAPER_PENV_10}  // Is = DMPAPER_LAST, use as loop terminator
};

static const int windowsConversionCount = int(sizeof(qt_windowsConversion) / sizeof(qt_windowsConversion[0]));

// Standard sizes data
struct StandardPageSize {
   QPageSize::PageSizeId id;
   int windowsId;                  // Windows DMPAPER value
   QPageSize::Unit definitionUnits;    // Standard definition size, e.g. ISO uses mm, ANSI uses inches
   int widthPoints;
   int heightPoints;
   qreal widthMillimeters;
   qreal heightMillimeters;
   qreal widthInches;
   qreal heightInches;
   const char *mediaOption;  // PPD standard mediaOption ID
};

// Standard page sizes taken from the Postscript PPD Standard v4.3
// See http://partners.adobe.com/public/developer/en/ps/5003.PPD_Spec_v4.3.pdf
// Excludes all Transverse and Rotated sizes
// NB! This table needs to be in sync with QPageSize::PageSizeId
static const StandardPageSize qt_pageSizes[] = {

   // Existing Qt sizes including ISO, US, ANSI and other standards
   {QPageSize::A4,   DMPAPER_A4,   QPageSize::Millimeter,    595,  842,      210,  297,       8.27,  11.69,    "A4"},
   {QPageSize::B5,   DMPAPER_NONE,   QPageSize::Millimeter,    499,  709,      176,  250,       6.9,   9.8,    "ISOB5"},
   {QPageSize::Letter,   DMPAPER_LETTER,   QPageSize::Inch,    612,  792,      215.9,  279.4,       8.5,  11,    "Letter"},
   {QPageSize::Legal,   DMPAPER_LEGAL,   QPageSize::Inch,    612, 1008,      215.9,  355.6,       8.5,  14,    "Legal"},
   {QPageSize::Executive,   DMPAPER_NONE,   QPageSize::Inch,    540,  720,      190.5,  254,       7.5,  10,    "Executive.7.5x10in"},                                       // Qt size differs from Postscript / Windows
   {QPageSize::A0,   DMPAPER_NONE,   QPageSize::Millimeter,   2384, 3370,      841, 1189,      33.11,  46.81,    "A0"},
   {QPageSize::A1,   DMPAPER_NONE,   QPageSize::Millimeter,   1684, 2384,      594,  841,      23.39,  33.11,    "A1"},
   {QPageSize::A2,   DMPAPER_A2,   QPageSize::Millimeter,   1191, 1684,      420,  594,      16.54,  23.39,    "A2"},
   {QPageSize::A3,   DMPAPER_A3,   QPageSize::Millimeter,    842, 1191,      297,  420,      11.69,  16.54,    "A3"},
   {QPageSize::A5,   DMPAPER_A5,   QPageSize::Millimeter,    420,  595,      148,  210,       5.83,   8.27,    "A5"},
   {QPageSize::A6,   DMPAPER_A6,   QPageSize::Millimeter,    297,  420,      105,  148,       4.13,   5.83,    "A6"},
   {QPageSize::A7,   DMPAPER_NONE,   QPageSize::Millimeter,    210,  297,       74,  105,       2.91,   4.13,    "A7"},
   {QPageSize::A8,   DMPAPER_NONE,   QPageSize::Millimeter,    148,  210,       52,   74,       2.05,   2.91,    "A8"},
   {QPageSize::A9,   DMPAPER_NONE,   QPageSize::Millimeter,    105,  148,       37,   52,       1.46,   2.05,    "A9"},
   {QPageSize::B0,   DMPAPER_NONE,   QPageSize::Millimeter,   2835, 4008,     1000, 1414,      39.37,  55.67,    "ISOB0"},
   {QPageSize::B1,   DMPAPER_NONE,   QPageSize::Millimeter,   2004, 2835,      707, 1000,      27.83,  39.37,    "ISOB1"},
   {QPageSize::B10,   DMPAPER_NONE,   QPageSize::Millimeter,     88,  125,       31,   44,       1.22,   1.73,    "ISOB10"},
   {QPageSize::B2,   DMPAPER_NONE,   QPageSize::Millimeter,   1417, 2004,      500,  707,      19.68,  27.83,    "ISOB2"},
   {QPageSize::B3,   DMPAPER_NONE,   QPageSize::Millimeter,   1001, 1417,      353,  500,      13.9,  19.68,    "ISOB3"},
   {QPageSize::B4,   DMPAPER_ISO_B4,   QPageSize::Millimeter,    709, 1001,      250,  353,       9.84,  13.9,    "ISOB4"},
   {QPageSize::B6,   DMPAPER_NONE,   QPageSize::Millimeter,    354,  499,      125,  176,       4.92,   6.93,    "ISOB6"},
   {QPageSize::B7,   DMPAPER_NONE,   QPageSize::Millimeter,    249,  354,       88,  125,       3.46,   4.92,    "ISOB7"},
   {QPageSize::B8,   DMPAPER_NONE,   QPageSize::Millimeter,    176,  249,       62,   88,       2.44,   3.46,    "ISOB8"},
   {QPageSize::B9,   DMPAPER_NONE,   QPageSize::Millimeter,    125,  176,       44,   62,       1.73,   2.44,    "ISOB9"},
   {QPageSize::C5E,   DMPAPER_ENV_C5,   QPageSize::Millimeter,    459,  649,      162,  229,       6.38,   9.02,    "EnvC5"},
   {QPageSize::Comm10E,   DMPAPER_ENV_10,   QPageSize::Inch,    297,  684,      104.8,  241.3,       4.12,   9.5,    "Env10"},
   {QPageSize::DLE,   DMPAPER_ENV_DL,   QPageSize::Millimeter,    312,  624,      110,  220,       4.33,   8.66,    "EnvDL"},
   {QPageSize::Folio,   DMPAPER_NONE,   QPageSize::Millimeter,    595,  935,      210,  330,       8.27,  13,    "Folio"},
   {QPageSize::Ledger,   DMPAPER_LEDGER,   QPageSize::Inch,   1224,  792,      431.8,  279.4,      17,  11,    "Ledger"},
   {QPageSize::Tabloid,   DMPAPER_TABLOID,   QPageSize::Inch,    792, 1224,      279.4,  431.8,      11,  17,    "Tabloid"},
   {QPageSize::Custom,   DMPAPER_USER,   QPageSize::Millimeter,     -1,   -1,       -1.,   -1,      -1,  -1,    "Custom"},                                       // Special case to keep in sync with QPageSize::PageSizeId

   // ISO Standard Sizes
   {QPageSize::A10,   DMPAPER_NONE,   QPageSize::Millimeter,     73,  105,       26,  37,       1.02,   1.46,    "A10"},
   {QPageSize::A3Extra,   DMPAPER_A3_EXTRA,   QPageSize::Millimeter,    913, 1262,      322,  445,      12.67,  17.52,    "A3Extra"},
   {QPageSize::A4Extra,   DMPAPER_A4_EXTRA,   QPageSize::Millimeter,    667,  914,      235.5,  322.3,       9.27,  12.69,    "A4Extra"},
   {QPageSize::A4Plus,   DMPAPER_A4_PLUS,   QPageSize::Millimeter,    595,  936,      210,  330,       8.27,  13,    "A4Plus"},
   {QPageSize::A4Small,   DMPAPER_A4SMALL,   QPageSize::Millimeter,    595,  842,      210,  297,       8.27,  11.69,    "A4Small"},
   {QPageSize::A5Extra,   DMPAPER_A5_EXTRA,   QPageSize::Millimeter,    492,  668,      174,  235,       6.85,   9.25,    "A5Extra"},
   {QPageSize::B5Extra,   DMPAPER_B5_EXTRA,   QPageSize::Millimeter,    570,  782,      201,  276,       7.9,  10.8,    "ISOB5Extra"},

   // JIS Standard Sizes
   {QPageSize::JisB0,   DMPAPER_NONE,   QPageSize::Millimeter,   2920, 4127,     1030, 1456,      40.55,  57.32,    "B0"},
   {QPageSize::JisB1,   DMPAPER_NONE,   QPageSize::Millimeter,   2064, 2920,      728, 1030,      28.66,  40.55,    "B1"},
   {QPageSize::JisB2,   DMPAPER_NONE,   QPageSize::Millimeter,   1460, 2064,      515,  728,      20.28,  28.66,    "B2"},
   {QPageSize::JisB3,   DMPAPER_NONE,   QPageSize::Millimeter,   1032, 1460,      364,  515,      14.33,  20.28,    "B3"},
   {QPageSize::JisB4,   DMPAPER_B4,   QPageSize::Millimeter,    729, 1032,      257,  364,      10.12,  14.33,    "B4"},
   {QPageSize::JisB5,   DMPAPER_B5,   QPageSize::Millimeter,    516,  729,      182,  257,       7.17,  10.12,    "B5"},
   {QPageSize::JisB6,   DMPAPER_B6_JIS,   QPageSize::Millimeter,    363,  516,      128,  182,       5.04,   7.17,    "B6"},
   {QPageSize::JisB7,   DMPAPER_NONE,   QPageSize::Millimeter,    258,  363,       91,  128,       3.58,   5.04,    "B7"},
   {QPageSize::JisB8,   DMPAPER_NONE,   QPageSize::Millimeter,    181,  258,       64,   91,       2.52,   3.58,    "B8"},
   {QPageSize::JisB9,   DMPAPER_NONE,   QPageSize::Millimeter,    127,  181,       45,   64,       1.77,   2.52,    "B9"},
   {QPageSize::JisB10,   DMPAPER_NONE,   QPageSize::Millimeter,     91,  127,       32,   45,       1.26,   1.77,    "B10"},

   // ANSI / US Standard sizes
   {QPageSize::AnsiC,   DMPAPER_NONE,   QPageSize::Inch,   1224, 1584,      431.8,  558.8,      17,  22,    "AnsiC"},
   {QPageSize::AnsiD,   DMPAPER_NONE,   QPageSize::Inch,   1584, 2448,      558.8,  863.6,      22,  34,    "AnsiD"},
   {QPageSize::AnsiE,   DMPAPER_NONE,   QPageSize::Inch,   2448, 3168,      863.6, 1118,      34,  44,    "AnsiE"},
   {QPageSize::LegalExtra,   DMPAPER_LEGAL_EXTRA,   QPageSize::Inch,    684, 1080,      241.3,  381,       9.5,  15,    "LegalExtra"},
   {QPageSize::LetterExtra,   DMPAPER_LETTER_EXTRA,   QPageSize::Inch,    684,  864,      241.3,  304.8,       9.5,  12,    "LetterExtra"},
   {QPageSize::LetterPlus,   DMPAPER_LETTER_PLUS,   QPageSize::Inch,    612,  914,      215.9,  322.3,       8.5,  12.69,    "LetterPlus"},
   {QPageSize::LetterSmall,   DMPAPER_LETTERSMALL,   QPageSize::Inch,    612,  792,      215.9,  279.4,       8.5,  11,    "LetterSmall"},
   {QPageSize::TabloidExtra,   DMPAPER_TABLOID_EXTRA,   QPageSize::Inch,    864, 1296,      304.8,  457.2,      12,  18,    "TabloidExtra"},

   // Architectural sizes
   {QPageSize::ArchA,   DMPAPER_NONE,   QPageSize::Inch,    648,  864,      228.6,  304.8,       9,  12,    "ARCHA"},
   {QPageSize::ArchB,   DMPAPER_NONE,   QPageSize::Inch,    864, 1296,      304.8,  457.2,      12,  18,    "ARCHB"},
   {QPageSize::ArchC,   DMPAPER_CSHEET,   QPageSize::Inch,   1296, 1728,      457.2,  609.6,      18,  24,    "ARCHC"},
   {QPageSize::ArchD,   DMPAPER_DSHEET,   QPageSize::Inch,   1728, 2592,      609.6,  914.4,      24,  36,    "ARCHD"},
   {QPageSize::ArchE,   DMPAPER_ESHEET,   QPageSize::Inch,   2592, 3456,      914.4, 1219,      36,  48,    "ARCHE"},

   // Inch-based Sizes
   {QPageSize::Imperial7x9,   DMPAPER_NONE,   QPageSize::Inch,    504,  648,      177.8,  228.6,       7,   9,    "7x9"},
   {QPageSize::Imperial8x10,   DMPAPER_NONE,   QPageSize::Inch,    576,  720,      203.2,  254,       8,  10,    "8x10"},
   {QPageSize::Imperial9x11,   DMPAPER_9X11,   QPageSize::Inch,    648,  792,      228.6,  279.4,       9,  11,    "9x11"},
   {QPageSize::Imperial9x12,   DMPAPER_NONE,   QPageSize::Inch,    648,  864,      228.6,  304.8,       9,  12,    "9x12"},
   {QPageSize::Imperial10x11,   DMPAPER_10X11,   QPageSize::Inch,    720,  792,      254,  279.4,      10,  11,    "10x11"},
   {QPageSize::Imperial10x13,   DMPAPER_NONE,   QPageSize::Inch,    720,  936,      254,  330.2,      10,  13,    "10x13"},
   {QPageSize::Imperial10x14,   DMPAPER_10X14,   QPageSize::Inch,    720, 1008,      254,  355.6,      10,  14,    "10x14"},
   {QPageSize::Imperial12x11,   DMPAPER_12X11,   QPageSize::Inch,    864,  792,      304.8,  279.4,      12,  11,    "12x11"},
   {QPageSize::Imperial15x11,   DMPAPER_15X11,   QPageSize::Inch,   1080,  792,      381,  279.4,      15,  11,    "15x11"},

   // Other Page Sizes
   {QPageSize::ExecutiveStandard,   DMPAPER_EXECUTIVE,   QPageSize::Inch,    522,  756,      184.2,  266.7,       7.25,  10.5,    "Executive"},                       // Qt size differs from Postscript / Windows
   {QPageSize::Note,   DMPAPER_NOTE,   QPageSize::Inch,    612,  792,      215.9,  279.4,       8.5,  11,    "Note"},
   {QPageSize::Quarto,   DMPAPER_QUARTO,   QPageSize::Inch,    610,  780,      215.9,  275.1,       8.5,  10.83,    "Quarto"},
   {QPageSize::Statement,   DMPAPER_STATEMENT,   QPageSize::Inch,    396,  612,      139.7,  215.9,       5.5,   8.5,    "Statement"},
   {QPageSize::SuperA,   DMPAPER_A_PLUS,   QPageSize::Millimeter,    643, 1009,      227,  356,       8.94,  14,    "SuperA"},
   {QPageSize::SuperB,   DMPAPER_B_PLUS,   QPageSize::Millimeter,    864, 1380,      305,  487,      12,  19.17,    "SuperB"},
   {QPageSize::Postcard,   DMPAPER_JAPANESE_POSTCARD,   QPageSize::Millimeter,    284,  419,      100,  148,       3.94,   5.83,    "Postcard"},
   {QPageSize::DoublePostcard,   DMPAPER_DBL_JAPANESE_POSTCARD,   QPageSize::Millimeter,    567,  419,      200,  148,       7.87,   5.83,    "DoublePostcard"},
   {QPageSize::Prc16K,   DMPAPER_P16K,   QPageSize::Millimeter,    414,  610,      146,  215,       5.75,   8.5,    "PRC16K"},
   {QPageSize::Prc32K,   DMPAPER_P32K,   QPageSize::Millimeter,    275,  428,       97,  151,       3.82,   5.95,    "PRC32K"},
   {QPageSize::Prc32KBig,   DMPAPER_P32KBIG,   QPageSize::Millimeter,    275,  428,       97,  151,       3.82,   5.95,    "PRC32KBig"},

   // Fan Fold Sizes
   {QPageSize::FanFoldUS,   DMPAPER_FANFOLD_US,   QPageSize::Inch,   1071,  792,      377.8,  279.4,      14.875, 11,    "FanFoldUS"},
   {QPageSize::FanFoldGerman,   DMPAPER_FANFOLD_STD_GERMAN,   QPageSize::Inch,    612,  864,      215.9,  304.8,       8.5,  12,    "FanFoldGerman"},
   {QPageSize::FanFoldGermanLegal,   DMPAPER_FANFOLD_LGL_GERMAN,   QPageSize::Inch,    612,  936,      215.9,  330,       8.5,  13,    "FanFoldGermanLegal"},

   // ISO Envelopes
   {QPageSize::EnvelopeB4,   DMPAPER_ENV_B4,   QPageSize::Millimeter,    708, 1001,      250,  353,       9.84,  13.9,    "EnvISOB4"},
   {QPageSize::EnvelopeB5,   DMPAPER_ENV_B5,   QPageSize::Millimeter,    499,  709,      176,  250,       6.9,   9.8,    "EnvISOB5"},
   {QPageSize::EnvelopeB6,   DMPAPER_ENV_B6,   QPageSize::Millimeter,    499,  354,      176,  125,       6.9,   4.9,    "EnvISOB6"},
   {QPageSize::EnvelopeC0,   DMPAPER_NONE,   QPageSize::Millimeter,   2599, 3676,      917,  1297,      36.1,  51.06,    "EnvC0"},
   {QPageSize::EnvelopeC1,   DMPAPER_NONE,   QPageSize::Millimeter,   1837, 2599,      648,  917,      25.51,  36.1,    "EnvC1"},
   {QPageSize::EnvelopeC2,   DMPAPER_NONE,   QPageSize::Millimeter,   1298, 1837,      458,  648,      18.03,  25.51,    "EnvC2"},
   {QPageSize::EnvelopeC3,   DMPAPER_ENV_C3,   QPageSize::Millimeter,    918, 1296,      324,  458,      12.75,  18.03,    "EnvC3"},
   {QPageSize::EnvelopeC4,   DMPAPER_ENV_C4,   QPageSize::Millimeter,    649,  918,      229,  324,       9.02,  12.75,    "EnvC4"},
   {QPageSize::EnvelopeC6,   DMPAPER_ENV_C6,   QPageSize::Millimeter,    323,  459,      114,  162,       4.49,   6.38,    "EnvC6"},
   {QPageSize::EnvelopeC65,   DMPAPER_ENV_C65,   QPageSize::Millimeter,    324,  648,      114,  229,       4.5,   9,    "EnvC65"},
   {QPageSize::EnvelopeC7,   DMPAPER_NONE,   QPageSize::Millimeter,    230,  323,       81,  114,       3.19,   4.49,    "EnvC7"},

   // US Envelopes
   {QPageSize::Envelope9,   DMPAPER_ENV_9,   QPageSize::Inch,    279,  639,       98.4,  225.4,       3.875,  8.875,   "Env9"},
   {QPageSize::Envelope11,   DMPAPER_ENV_11,   QPageSize::Inch,    324,  747,      114.3,  263.5,       4.5,  10.375,   "Env11"},
   {QPageSize::Envelope12,   DMPAPER_ENV_12,   QPageSize::Inch,    342,  792,      120.7,  279.4,       4.75,  11,    "Env12"},
   {QPageSize::Envelope14,   DMPAPER_ENV_14,   QPageSize::Inch,    360,  828,      127,  292.1,       5,  11.5,    "Env14"},
   {QPageSize::EnvelopeMonarch,   DMPAPER_ENV_MONARCH,   QPageSize::Inch,    279,  540,       98.43, 190.5,       3.875,  7.5,    "EnvMonarch"},
   {QPageSize::EnvelopePersonal,   DMPAPER_ENV_PERSONAL,   QPageSize::Inch,    261,  468,       92.08, 165.1,       3.625,  6.5,    "EnvPersonal"},

   // Other Envelopes
   {QPageSize::EnvelopeChou3,   DMPAPER_JENV_CHOU3,   QPageSize::Millimeter,    340,  666,      120,  235,       4.72,   9.25,    "EnvChou3"},
   {QPageSize::EnvelopeChou4,   DMPAPER_JENV_CHOU4,   QPageSize::Millimeter,    255,  581,       90,  205,       3.54,   8,    "EnvChou4"},
   {QPageSize::EnvelopeInvite,  DMPAPER_ENV_INVITE,   QPageSize::Millimeter,    624,  624,      220,  220,       8.66,   8.66,    "EnvInvite"},
   {QPageSize::EnvelopeItalian, DMPAPER_ENV_ITALY,   QPageSize::Millimeter,    312,  652,      110,  230,       4.33,   9,    "EnvItalian"},
   {QPageSize::EnvelopeKaku2,   DMPAPER_JENV_KAKU2,   QPageSize::Millimeter,    680,  941,      240,  332,       9.45,  13,    "EnvKaku2"},
   {QPageSize::EnvelopeKaku3,   DMPAPER_JENV_KAKU3,   QPageSize::Millimeter,    612,  785,      216,  277,       8.5,  10.9,    "EnvKaku3"},
   {QPageSize::EnvelopePrc1,   DMPAPER_PENV_1,   QPageSize::Millimeter,    289,  468,      102,  165,       4,   6.5,    "EnvPRC1"},
   {QPageSize::EnvelopePrc2,   DMPAPER_PENV_2,   QPageSize::Millimeter,    289,  499,      102,  176,       4,   6.9,    "EnvPRC2"},
   {QPageSize::EnvelopePrc3,   DMPAPER_PENV_3,   QPageSize::Millimeter,    354,  499,      125,  176,       4.9,   6.9,    "EnvPRC3"},
   {QPageSize::EnvelopePrc4,   DMPAPER_PENV_4,   QPageSize::Millimeter,    312,  590,      110,  208,       4.33,   8.2,    "EnvPRC4"},
   {QPageSize::EnvelopePrc5,   DMPAPER_PENV_5,   QPageSize::Millimeter,    312,  624,      110,  220,       4.33,   8.66,    "EnvPRC5"},
   {QPageSize::EnvelopePrc6,   DMPAPER_PENV_6,   QPageSize::Millimeter,    340,  652,      120,  230,       4.7,   9,    "EnvPRC6"},
   {QPageSize::EnvelopePrc7,   DMPAPER_PENV_7,   QPageSize::Millimeter,    454,  652,      160,  230,       6.3,   9,    "EnvPRC7"},
   {QPageSize::EnvelopePrc8,   DMPAPER_PENV_8,   QPageSize::Millimeter,    340,  876,      120,  309,       4.7,  12.2,    "EnvPRC8"},
   {QPageSize::EnvelopePrc9,   DMPAPER_PENV_9,   QPageSize::Millimeter,    649,  918,      229,  324,       9,  12.75,    "EnvPRC9"},
   {QPageSize::EnvelopePrc10,  DMPAPER_PENV_10,   QPageSize::Millimeter,    918, 1298,      324,  458,      12.75,  18,    "EnvPRC10"},
   {QPageSize::EnvelopeYou4,   DMPAPER_JENV_YOU4,   QPageSize::Millimeter,    298,  666,      105,  235,       4.13,   9.25,    "EnvYou4"}
};

static const int pageSizesCount = int(sizeof(qt_pageSizes) / sizeof(qt_pageSizes[0]));
static_assert(pageSizesCount == QPageSize::LastPageSize + 1, "Page size array mismatch");

// Return key name for PageSize
static QString qt_keyForPageSizeId(QPageSize::PageSizeId id)
{
   return QString::fromLatin1(qt_pageSizes[id].mediaOption);
}

// Return id name for PPD Key
static QPageSize::PageSizeId qt_idForPpdKey(const QString &ppdKey, QSize *match = 0)
{
   if (ppdKey.isEmpty()) {
      return QPageSize::Custom;
   }
   QString key = ppdKey;
   // Remove any Rotated or Tranverse modifiers
   if (key.endsWith(QLatin1String("Rotated"))) {
      key.chop(7);
   } else if (key.endsWith(QLatin1String(".Transverse"))) {
      key.chop(11);
   }
   for (int i = 0; i <= int(QPageSize::LastPageSize); ++i) {
      if (QString::fromLatin1(qt_pageSizes[i].mediaOption) == key) {
         if (match) {
            *match = QSize(qt_pageSizes[i].widthPoints, qt_pageSizes[i].heightPoints);
         }
         return qt_pageSizes[i].id;
      }
   }
   return QPageSize::Custom;
}

// Return id name for Windows ID
static QPageSize::PageSizeId qt_idForWindowsID(int windowsId, QSize *match = 0)
{
   // If outside known values then is Custom
   if (windowsId <= DMPAPER_NONE || windowsId > DMPAPER_LAST) {
      return QPageSize::Custom;
   }
   // Check if one of the unsupported values, convert to valid value if is
   for (int i = 0; i < windowsConversionCount; ++i) {
      if (qt_windowsConversion[i][0] == windowsId) {
         windowsId = qt_windowsConversion[i][1];
         break;
      }
   }
   // Look for the value in our supported size table
   for (int i = 0; i <= int(QPageSize::LastPageSize); ++i) {
      if (qt_pageSizes[i].windowsId == windowsId) {
         if (match) {
            *match = QSize(qt_pageSizes[i].widthPoints, qt_pageSizes[i].heightPoints);
         }
         return qt_pageSizes[i].id;
      }
   }
   // Otherwise is Custom
   return QPageSize::Custom;
}

// Return key name for custom size
static QString qt_keyForCustomSize(const QSizeF &size, QPageSize::Unit units)
{
   // PPD custom format
   QString key = "Custom.%1x%2%3";
   QString abbrev;

   switch (units) {
      case QPageSize::Unit::Millimeter:
         abbrev = "mm";
         break;

      case QPageSize::Unit::Point:
         break;

      case QPageSize::Unit::Inch:
         abbrev = "in";
         break;

      case QPageSize::Unit::Pica:
         abbrev = "pc";
         break;

      case QPageSize::Unit::Didot:
         abbrev = "DD";
         break;

      case QPageSize::Unit::Cicero:
         abbrev = "CC";
         break;

      default:
         abbrev = "Unknown";
         break;
   }

   // assumes size is already max 2 decimal places

   return key.formatArg(size.width()).formatArg(size.height()).formatArg(abbrev);
}

// Return localized name for custom size
static QString qt_nameForCustomSize(const QSizeF &size, QPageSize::Unit units)
{
   QString name;

   switch (units) {
      case QPageSize::Unit::Millimeter:
         //: Custom size name in millimeters
         name = QCoreApplication::translate("QPageSize", "Custom (%1mm x %2mm)");
         break;

      case QPageSize::Unit::Point:
         //: Custom size name in points
         name = QCoreApplication::translate("QPageSize", "Custom (%1pt x %2pt)");
         break;

      case QPageSize::Unit::Inch:
         //: Custom size name in inches
         name = QCoreApplication::translate("QPageSize", "Custom (%1in x %2in)");
         break;

      case QPageSize::Unit::Pica:
         //: Custom size name in picas
         name = QCoreApplication::translate("QPageSize", "Custom (%1pc x %2pc)");
         break;

      case QPageSize::Unit::Didot:
         //: Custom size name in didots
         name = QCoreApplication::translate("QPageSize", "Custom (%1DD x %2DD)");
         break;

      case QPageSize::Unit::Cicero:
         //: Custom size name in ciceros
         name = QCoreApplication::translate("QPageSize", "Custom (%1CC x %2CC)");
         break;

      default:
         name = QCoreApplication::translate("QPageSize", "Unknown (%1 x %2)");
         break;
   }

   // assumes size is already max 2 decimal places

   return name.formatArg(size.width()).formatArg(size.height());
}

// Multiplier for converting units to points.
static qreal qt_pointMultiplier(QPageSize::Unit unit)
{
   switch (unit) {
      case QPageSize::Unit::Millimeter:
         return 2.83464566929;

      case QPageSize::Unit::Point:
         return 1.0;

      case QPageSize::Unit::Inch:
         return 72.0;

      case QPageSize::Unit::Pica:
         return 12;

      case QPageSize::Unit::Didot:
         return 1.065826771;

      case QPageSize::Unit::Cicero:
         return 12.789921252;

      default:
         return 1.0;
   }
}

// Multiplier for converting pixels to points.
Q_GUI_EXPORT qreal qt_pixelMultiplier(int resolution)
{
   return resolution <= 0 ? 1.0 : 72.0 / resolution;
}

static QSizeF qt_definitionSize(QPageSize::PageSizeId pageSizeId)
{
   QPageSize::Unit units = qt_pageSizes[pageSizeId].definitionUnits;

   if (units == QPageSize::Millimeter) {
      return QSizeF(qt_pageSizes[pageSizeId].widthMillimeters, qt_pageSizes[pageSizeId].heightMillimeters);
   }

   Q_ASSERT(units == QPageSize::Inch);  // We currently only support definitions in mm or inches
   return QSizeF(qt_pageSizes[pageSizeId].widthInches, qt_pageSizes[pageSizeId].heightInches);
}

static QSizeF qt_convertUnits(const QSizeF &size, QPageSize::Unit fromUnits, QPageSize::Unit toUnits)
{
   if (! size.isValid()) {
      return QSizeF();
   }

   // If the units are the same or the size is 0, then don't need to convert
   if (fromUnits == toUnits || (qFuzzyIsNull(size.width()) && qFuzzyIsNull(size.height()))) {
      return size;
   }

   QSizeF newSize = size;
   // First convert to points
   if (fromUnits != QPageSize::Unit::Point) {
      const qreal multiplier = qt_pointMultiplier(fromUnits);
      newSize = newSize * multiplier;
   }

   // Then convert from points to required units
   const qreal multiplier = qt_pointMultiplier(toUnits);
   // Try force to 2 decimal places for consistency
   const int width = qRound(newSize.width() * 100 / multiplier);
   const int height = qRound(newSize.height() * 100 / multiplier);
   return QSizeF(width / 100.0, height / 100.0);
}

static QSize qt_convertUnitsToPoints(const QSizeF &size, QPageSize::Unit units)
{
   if (!size.isValid()) {
      return QSize();
   }
   return QSizeF(size * qt_pointMultiplier(units)).toSize();
}

static QSize qt_convertPointsToPixels(const QSize &size, int resolution)
{
   if (!size.isValid() || resolution <= 0) {
      return QSize();
   }
   const qreal multiplier = qt_pixelMultiplier(resolution);
   return QSize(qRound(size.width() / multiplier), qRound(size.height() / multiplier));
}

static QSizeF qt_convertPointsToUnits(const QSize &size, QPageSize::Unit units)
{
   if (!size.isValid()) {
      return QSizeF();
   }

   const qreal multiplier = qt_pointMultiplier(units);

   // Try force to 2 decimal places for consistency
   const int width  = qRound(size.width() * 100 / multiplier);
   const int height = qRound(size.height() * 100 / multiplier);

   return QSizeF(width / 100.0, height / 100.0);
}

static QSizeF qt_unitSize(QPageSize::PageSizeId pageSizeId, QPageSize::Unit units)
{
   switch (units) {
      case QPageSize::Unit::Millimeter:
         return QSizeF(qt_pageSizes[pageSizeId].widthMillimeters, qt_pageSizes[pageSizeId].heightMillimeters);

      case QPageSize::Unit::Point:
         return QSizeF(qt_pageSizes[pageSizeId].widthPoints, qt_pageSizes[pageSizeId].heightPoints);

      case QPageSize::Unit::Inch:
         return QSizeF(qt_pageSizes[pageSizeId].widthInches, qt_pageSizes[pageSizeId].heightInches);

      case QPageSize::Unit::Pica:
      case QPageSize::Unit::Didot:
      case QPageSize::Unit::Cicero:
         return qt_convertPointsToUnits(QSize(qt_pageSizes[pageSizeId].widthPoints,
                  qt_pageSizes[pageSizeId].heightPoints), units);

      default:
         return QSizeF();
   }
}

// Find matching standard page size for point size
static QPageSize::PageSizeId qt_idForPointSize(const QSize &size, QPageSize::SizeMatchPolicy matchPolicy, QSize *match)
{
   if (! size.isValid()) {
      return QPageSize::Custom;
   }

   // Try exact match in portrait layout
   for (int i = 0; i <= int(QPageSize::LastPageSize); ++i) {
      if (size.width() == qt_pageSizes[i].widthPoints && size.height() == qt_pageSizes[i].heightPoints) {
         if (match) {
            *match = QSize(qt_pageSizes[i].widthPoints, qt_pageSizes[i].heightPoints);
         }
         return qt_pageSizes[i].id;
      }
   }

   // If no exact match only try fuzzy if asked
   if (matchPolicy != QPageSize::ExactMatch) {
      // Set up the fuzzy tolerance
      // TODO Use ISO standard tolerance based on page size?
      const int tolerance = 3; // = approx 1mm
      const int minWidth = size.width() - tolerance;
      const int maxWidth = size.width() + tolerance;
      const int minHeight = size.height() - tolerance;
      const int maxHeight = size.height() + tolerance;

      // First try fuzzy match in portrait layout
      for (int i = 0; i <= QPageSize::LastPageSize; ++i) {
         const int width = qt_pageSizes[i].widthPoints;
         const int height = qt_pageSizes[i].heightPoints;
         if (width >= minWidth && width <= maxWidth && height >= minHeight && height <= maxHeight) {
            if (match) {
               *match = QSize(qt_pageSizes[i].widthPoints, qt_pageSizes[i].heightPoints);
            }
            return qt_pageSizes[i].id;
         }
      }

      // If FuzzyOrientationMatch then try rotated sizes
      if (matchPolicy == QPageSize::FuzzyOrientationMatch) {
         // First try exact match in landscape layout
         for (int i = 0; i <= QPageSize::LastPageSize; ++i) {
            if (size.width() == qt_pageSizes[i].heightPoints && size.height() == qt_pageSizes[i].widthPoints) {
               if (match) {
                  *match = QSize(qt_pageSizes[i].widthPoints, qt_pageSizes[i].heightPoints);
               }
               return qt_pageSizes[i].id;
            }
         }

         // Then try fuzzy match in landscape layout
         for (int i = 0; i <= QPageSize::LastPageSize; ++i) {
            const int width = qt_pageSizes[i].heightPoints;
            const int height = qt_pageSizes[i].widthPoints;
            if (width >= minWidth && width <= maxWidth && height >= minHeight && height <= maxHeight) {
               if (match) {
                  *match = QSize(qt_pageSizes[i].widthPoints, qt_pageSizes[i].heightPoints);
               }
               return qt_pageSizes[i].id;
            }
         }
      }
   }

   if (match) {
      *match = size;
   }
   // Otherwise no match so Custom
   return QPageSize::Custom;
}

// Find matching standard page size for point size
static QPageSize::PageSizeId qt_idForSize(const QSizeF &size, QPageSize::Unit units,
   QPageSize::SizeMatchPolicy matchPolicy, QSize *match)
{
   if (!size.isValid()) {
      return QPageSize::Custom;
   }

   // Try exact match if units are the same
   if (units == QPageSize::Millimeter) {
      for (int i = 0; i <= QPageSize::LastPageSize; ++i) {
         if (size.width() == qt_pageSizes[i].widthMillimeters && size.height() == qt_pageSizes[i].heightMillimeters) {
            if (match) {
               *match = QSize(qt_pageSizes[i].widthPoints, qt_pageSizes[i].heightPoints);
            }
            return qt_pageSizes[i].id;
         }
      }

   } else if (units == QPageSize::Unit::Inch) {
      for (int i = 0; i <= QPageSize::PageSizeId::LastPageSize; ++i) {
         if (size.width() == qt_pageSizes[i].widthInches && size.height() == qt_pageSizes[i].heightInches) {
            if (match) {
               *match = QSize(qt_pageSizes[i].widthPoints, qt_pageSizes[i].heightPoints);
            }
            return qt_pageSizes[i].id;
         }
      }

   } else if (units == QPageSize::Unit::Point) {
      for (int i = 0; i <= QPageSize::PageSizeId::LastPageSize; ++i) {
         if (size.width() == qt_pageSizes[i].widthPoints && size.height() == qt_pageSizes[i].heightPoints) {
            if (match) {
               *match = QSize(qt_pageSizes[i].widthPoints, qt_pageSizes[i].heightPoints);
            }
            return qt_pageSizes[i].id;
         }
      }
   }

   // If no exact match then convert to points and try match those
   QSize points = qt_convertUnitsToPoints(size, units);
   return qt_idForPointSize(points, matchPolicy, match);
}

class QPageSizePrivate : public QSharedData
{
 public:
   QPageSizePrivate();
   explicit QPageSizePrivate(QPageSize::PageSizeId pageSizeId);
   QPageSizePrivate(const QSize &pointSize,
      const QString &name,
      QPageSize::SizeMatchPolicy matchPolicy);
   QPageSizePrivate(const QSizeF &size, QPageSize::Unit units,
      const QString &name,
      QPageSize::SizeMatchPolicy matchPolicy);
   QPageSizePrivate(const QString &key, const QSize &size, const QString &name);
   QPageSizePrivate(int windowsId, const QSize &pointSize, const QString &name);
   ~QPageSizePrivate();

   bool operator==(const QPageSizePrivate &other) const;
   bool isEquivalentTo(const QPageSizePrivate &other) const;

   bool isValid() const;

   QSizeF size(QPageSize::Unit units) const;
   QSize sizePixels(int resolution) const;

 private:
   friend class QPageSize;

   void init(QPageSize::PageSizeId id, const QString &name);
   void init(const QSize &size, const QString &name);
   void init(const QSizeF &size, QPageSize::Unit units, const QString &name);

   QString m_key;
   QPageSize::PageSizeId m_id;
   QSize m_pointSize;
   QString m_name;
   int m_windowsId;
   QSizeF m_size;
   QPageSize::Unit m_units;
};

QPageSizePrivate::QPageSizePrivate()
   : m_id(QPageSize::Custom),
     m_windowsId(0),
     m_units(QPageSize::Unit::Point)
{
}

QPageSizePrivate::QPageSizePrivate(QPageSize::PageSizeId pageSizeId)
   : m_id(QPageSize::Custom), m_windowsId(0), m_units(QPageSize::Unit::Point)
{
   if (pageSizeId >= QPageSize::PageSizeId(0) && pageSizeId <= QPageSize::PageSizeId::LastPageSize) {
      init(pageSizeId, QString());
   }
}

QPageSizePrivate::QPageSizePrivate(const QSize &pointSize, const QString &name, QPageSize::SizeMatchPolicy matchPolicy)
   : m_id(QPageSize::Custom),
     m_windowsId(0),
     m_units(QPageSize::Unit::Point)
{
   if (pointSize.isValid()) {
      QPageSize::PageSizeId id = qt_idForPointSize(pointSize, matchPolicy, 0);
      id == QPageSize::Custom ? init(pointSize, name) : init(id, name);
   }
}

QPageSizePrivate::QPageSizePrivate(const QSizeF &size, QPageSize::Unit units,
   const QString &name, QPageSize::SizeMatchPolicy matchPolicy)
   : m_id(QPageSize::Custom),
     m_windowsId(0),
     m_units(QPageSize::Unit::Point)
{
   if (size.isValid()) {
      QPageSize::PageSizeId id = qt_idForSize(size, units, matchPolicy, 0);
      id == QPageSize::Custom ? init(size, units, name) : init(id, name);
   }
}

QPageSizePrivate::QPageSizePrivate(const QString &key, const QSize &pointSize, const QString &name)
   : m_id(QPageSize::Custom),
     m_windowsId(0),
     m_units(QPageSize::Unit::Point)
{
   if (!key.isEmpty() && pointSize.isValid()) {
      QPageSize::PageSizeId id = qt_idForPpdKey(key, 0);
      // If not a known PPD key, check if size is a standard PPD size
      if (id == QPageSize::Custom) {
         id = qt_idForPointSize(pointSize, QPageSize::FuzzyMatch, 0);
      }
      id == QPageSize::Custom ? init(pointSize, name) : init(id, name);
      m_key = key;
   }
}

QPageSizePrivate::QPageSizePrivate(int windowsId, const QSize &pointSize, const QString &name)
   : m_id(QPageSize::Custom),
     m_windowsId(0),
     m_units(QPageSize::Unit::Point)
{
   if (windowsId > 0 && pointSize.isValid()) {
      QPageSize::PageSizeId id = qt_idForWindowsID(windowsId, 0);
      // If not a known Windows ID, check if size is a standard PPD size
      if (id == QPageSize::Custom) {
         id = qt_idForPointSize(pointSize, QPageSize::FuzzyMatch, 0);
      }
      id == QPageSize::Custom ? init(pointSize, name) : init(id, name);
      m_windowsId = windowsId;
   }
}

QPageSizePrivate::~QPageSizePrivate()
{
}

// Init a standard PageSizeId
void QPageSizePrivate::init(QPageSize::PageSizeId id, const QString &name)
{
   m_id = id;
   m_size = qt_definitionSize(id);
   m_units = qt_pageSizes[id].definitionUnits;
   m_key = qt_keyForPageSizeId(id);
   m_name = name.isEmpty() ? QPageSize::name(id) : name;
   m_windowsId = qt_pageSizes[id].windowsId;
   m_pointSize = QSize(qt_pageSizes[id].widthPoints, qt_pageSizes[id].heightPoints);
}

// Init a point size
void QPageSizePrivate::init(const QSize &size, const QString &name)
{
   m_id = QPageSize::Custom;
   m_size = size;
   m_units = QPageSize::Unit::Point;
   m_key = qt_keyForCustomSize(m_size, m_units);
   m_name = name.isEmpty() ? qt_nameForCustomSize(m_size, m_units) : name;
   m_windowsId = 0;
   m_pointSize = size;
}

// Init a unit size
void QPageSizePrivate::init(const QSizeF &size, QPageSize::Unit units, const QString &name)
{
   m_id = QPageSize::Custom;
   m_size = size;
   m_units = units;
   m_key = qt_keyForCustomSize(m_size, m_units);
   if (name.isEmpty()) {
      m_name = qt_nameForCustomSize(m_size, m_units);
   } else {
      m_name = name;
   }
   m_windowsId = 0;
   m_pointSize = qt_convertUnitsToPoints(m_size, m_units);
}

bool QPageSizePrivate::operator==(const QPageSizePrivate &other) const
{
   return m_size == other.m_size
      && m_units == other.m_units
      && m_key == other.m_key
      && m_name == other.m_name;
}

bool QPageSizePrivate::isEquivalentTo(const QPageSizePrivate &other) const
{
   return m_pointSize == other.m_pointSize;
}

bool QPageSizePrivate::isValid() const
{
   return m_pointSize.isValid() && !m_key.isEmpty() && !m_name.isEmpty();
}

QSizeF QPageSizePrivate::size(QPageSize::Unit units) const
{
   // If want units we've stored in, we already have them
   if (units == m_units) {
      return m_size;
   }

   // If want points we already have them
   if (units == QPageSize::Unit::Point) {
      return QSizeF(m_pointSize.width(), m_pointSize.height());
   }

   // If a custom size do a conversion
   if (m_id == QPageSize::Custom) {
      return qt_convertUnits(m_size, m_units, units);
   }

   // Otherwise use the standard sizes
   return qt_unitSize(m_id, units);
}

QSize QPageSizePrivate::sizePixels(int resolution) const
{
   return qt_convertPointsToPixels(m_pointSize, resolution);;
}

QPageSize::QPageSize()
   : d(new QPageSizePrivate())
{
}

QPageSize::QPageSize(PageSizeId pageSize)
   : d(new QPageSizePrivate(pageSize))
{
}

QPageSize::QPageSize(const QSize &pointSize, const QString &name, SizeMatchPolicy matchPolicy)
   : d(new QPageSizePrivate(pointSize, name, matchPolicy))
{
}

QPageSize::QPageSize(const QSizeF &size, Unit units,
   const QString &name, SizeMatchPolicy matchPolicy)
   : d(new QPageSizePrivate(size, units, name, matchPolicy))
{
}

/*!
    \internal

    Create page with given key, size and name, for use by printer plugin.
*/

QPageSize::QPageSize(const QString &key, const QSize &pointSize, const QString &name)
   : d(new QPageSizePrivate(key, pointSize, name))
{
}

/*!
    \internal

    Create page with given windows ID, size and name, for use by printer plugin.
*/

QPageSize::QPageSize(int windowsId, const QSize &pointSize, const QString &name)
   : d(new QPageSizePrivate(windowsId, pointSize, name))
{
}

/*!
    \internal

    Create page with given private backend
*/

QPageSize::QPageSize(QPageSizePrivate &dd)
   : d(&dd)
{
}

/*!
    Copy constructor, copies \a other to this.
*/

QPageSize::QPageSize(const QPageSize &other)
   : d(other.d)
{
}

/*!
    Destroys the page.
*/

QPageSize::~QPageSize()
{
}

/*!
    Assignment operator, assigns \a other to this.
*/

QPageSize &QPageSize::operator=(const QPageSize &other)
{
   d = other.d;
   return *this;
}

/*!
    \fn void QPageSize::swap(QPageSize &other)

    Swaps this QPageSize with \a other. This function is very fast and
    never fails.
*/

/*!
    \fn QPageSize &QPageSize::operator=(QPageSize &&other)

    Move-assigns \a other to this QPageSize instance, transferring the
    ownership of the managed pointer to this instance.
*/

/*!
    \relates QPageSize

    Returns \c true if page size \a lhs is equal to page size \a rhs,
    i.e. if the page sizes have the same attributes. Current
    attributes are size and name.
*/

bool operator==(const QPageSize &lhs, const QPageSize &rhs)
{
   return lhs.d == rhs.d || *lhs.d == *rhs.d;
}
/*!
    \fn bool operator!=(const QPageSize &lhs, const QPageSize &rhs)
    \relates QPageSize

    Returns \c true if page size \a lhs is unequal to page size \a
    rhs, i.e. if the page size has different attributes. Current
    attributes are size and name.
*/

/*!
    Returns \c true if this page is equivalent to the \a other page, i.e. if the
    page has the same size regardless of other attributes like name.
*/

bool QPageSize::isEquivalentTo(const QPageSize &other) const
{
   if (d == other.d) {
      return true;
   }
   return d && other.d && d->isEquivalentTo(*other.d);
}

/*!
    Returns \c true if this page size is valid.

    The page size may be invalid if created with an invalid PageSizeId, or a
    negative or invalid QSize or QSizeF, or the null constructor.
*/

bool QPageSize::isValid() const
{
   return d && d->isValid();
}

/*!
    Returns the unique key of the page size.

    By default this is the PPD standard mediaOption keyword for the page size,
    or the PPD custom format key.  If the QPageSize instance was obtained from
    a print device then this will be the key provided by the print device and
    may differ from the standard key.

    If the QPageSize is invalid then the key will be an empty string.

    This key should never be shown to end users, it is an internal key only.
    For a human-readable name use name().

    \sa name()
*/

QString QPageSize::key() const
{
   return isValid() ? d->m_key : QString();
}

/*!
    Returns a localized human-readable name for the page size.

    If the QPageSize instance was obtained from a print device then the name
    used is that provided by the print device.  Note that a print device may
    not support the current default locale language.

    If the QPageSize is invalid then the name will be an empty string.
*/

QString QPageSize::name() const
{
   return isValid() ? d->m_name : QString();
}

/*!
    Returns the standard QPageSize::PageSizeId of the page, or QPageSize::Custom.

    If the QPageSize is invalid then the ID will be QPageSize::Custom.
*/

QPageSize::PageSizeId QPageSize::id() const
{
   return isValid() ? d->m_id : Custom;
}

/*!
    Returns the Windows DMPAPER enum value for the page size.

    Not all valid PPD page sizes have a Windows equivalent, in which case 0
    will be returned.

    If the QPageSize is invalid then the Windows ID will be 0.

    \sa id()
*/

int QPageSize::windowsId() const
{
   if (!isValid()) {
      return 0;
   }
   return d->m_windowsId > 0 ? d->m_windowsId : windowsId(d->m_id);
}

/*!
    Returns the definition size of the page size.

    For a standard page size this will be the size as defined in the relevant
    standard, i.e. ISO A4 will be defined in millimeters while ANSI Letter will
    be defined in inches.

    For a custom page size this will be the original size used to create the
    page size object.

    If the QPageSize is invalid then the QSizeF will be invalid.

    \sa definitionUnits()
*/

QSizeF QPageSize::definitionSize() const
{
   return isValid() ? d->m_size : QSizeF();
}

/*!
    Returns the definition units of the page size.

    For a standard page size this will be the units as defined in the relevant
    standard, i.e. ISO A4 will be defined in millimeters while ANSI Letter will
    be defined in inches.

    For a custom page size this will be the original units used to create the
    page size object.

    If the QPageSize is invalid then the QPageSize::Unit will be invalid.

    \sa definitionSize()
*/

QPageSize::Unit QPageSize::definitionUnits() const
{
   return isValid() ? d->m_units : Unit(-1);
}

/*!
    Returns the size of the page in the required \a units.

    If the QPageSize is invalid then the QSizeF will be invalid.
*/

QSizeF QPageSize::size(Unit units) const
{
   return isValid() ? d->size(units) : QSize();
}

/*!
    Returns the size of the page in Postscript Points (1/72 of an inch).

    If the QPageSize is invalid then the QSize will be invalid.
*/

QSize QPageSize::sizePoints() const
{
   return isValid() ? d->m_pointSize : QSize();
}

/*!
    Returns the size of the page in Device Pixels at the given \a resolution.

    If the QPageSize is invalid then the QSize will be invalid.
*/

QSize QPageSize::sizePixels(int resolution) const
{
   return isValid() ? d->sizePixels(resolution) : QSize();
}

/*!
    Returns the page rectangle in the required \a units.

    If the QPageSize is invalid then the QRect will be invalid.
*/

QRectF QPageSize::rect(Unit units) const
{
   return isValid() ? QRectF(QPointF(0, 0), d->size(units)) : QRectF();
}

/*!
    Returns the page rectangle in Postscript Points (1/72 of an inch).

    If the QPageSize is invalid then the QRect will be invalid.
*/

QRect QPageSize::rectPoints() const
{
   return isValid() ? QRect(QPoint(0, 0), d->m_pointSize) : QRect();
}

/*!
    Returns the page rectangle in Device Pixels at the given \a resolution.

    If the QPageSize is invalid then the QRect will be invalid.
*/

QRect QPageSize::rectPixels(int resolution) const
{
   return isValid() ? QRect(QPoint(0, 0), d->sizePixels(resolution)) : QRect();
}

// Statics

/*!
    Returns the PPD mediaOption keyword of the standard \a pageSizeId.

    If the QPageSize is invalid then the key will be empty.
*/

QString QPageSize::key(PageSizeId pageSizeId)
{
   if (pageSizeId < PageSizeId(0) || pageSizeId > LastPageSize) {
      return QString();
   }
   return QString::fromUtf8(qt_pageSizes[pageSizeId].mediaOption);
}

static QString msgImperialPageSizeInch(int width, int height)
{
   //: Page size in 'Inch'.
   return QCoreApplication::translate("QPageSize", "%1 x %2 in").formatArg(width).formatArg(height);
}

/*!
    Returns the localized name of the standard \a pageSizeId.

    If the QPageSize is invalid then the name will be empty.
*/

QString QPageSize::name(PageSizeId pageSizeId)
{
   if (pageSizeId < PageSizeId(0) || pageSizeId > LastPageSize) {
      return QString();
   }

   switch (pageSizeId) {
      case A0:
         return QCoreApplication::translate("QPageSize", "A0");
      case A1:
         return QCoreApplication::translate("QPageSize", "A1");
      case A2:
         return QCoreApplication::translate("QPageSize", "A2");
      case A3:
         return QCoreApplication::translate("QPageSize", "A3");
      case A4:
         return QCoreApplication::translate("QPageSize", "A4");
      case A5:
         return QCoreApplication::translate("QPageSize", "A5");
      case A6:
         return QCoreApplication::translate("QPageSize", "A6");
      case A7:
         return QCoreApplication::translate("QPageSize", "A7");
      case A8:
         return QCoreApplication::translate("QPageSize", "A8");
      case A9:
         return QCoreApplication::translate("QPageSize", "A9");
      case A10:
         return QCoreApplication::translate("QPageSize", "A10");
      case B0:
         return QCoreApplication::translate("QPageSize", "B0");
      case B1:
         return QCoreApplication::translate("QPageSize", "B1");
      case B2:
         return QCoreApplication::translate("QPageSize", "B2");
      case B3:
         return QCoreApplication::translate("QPageSize", "B3");
      case B4:
         return QCoreApplication::translate("QPageSize", "B4");
      case B5:
         return QCoreApplication::translate("QPageSize", "B5");
      case B6:
         return QCoreApplication::translate("QPageSize", "B6");
      case B7:
         return QCoreApplication::translate("QPageSize", "B7");
      case B8:
         return QCoreApplication::translate("QPageSize", "B8");
      case B9:
         return QCoreApplication::translate("QPageSize", "B9");
      case B10:
         return QCoreApplication::translate("QPageSize", "B10");
      case Executive:
         return QCoreApplication::translate("QPageSize", "Executive (7.5 x 10 in)");
      case ExecutiveStandard:
         return QCoreApplication::translate("QPageSize", "Executive (7.25 x 10.5 in)");
      case Folio:
         return QCoreApplication::translate("QPageSize", "Folio (8.27 x 13 in)");
      case Legal:
         return QCoreApplication::translate("QPageSize", "Legal");
      case Letter:
         return QCoreApplication::translate("QPageSize", "Letter / ANSI A");
      case Tabloid:
         return QCoreApplication::translate("QPageSize", "Tabloid / ANSI B");
      case Ledger:
         return QCoreApplication::translate("QPageSize", "Ledger / ANSI B");
      case Custom:
         return QCoreApplication::translate("QPageSize", "Custom");
      case A3Extra:
         return QCoreApplication::translate("QPageSize", "A3 Extra");
      case A4Extra:
         return QCoreApplication::translate("QPageSize", "A4 Extra");
      case A4Plus:
         return QCoreApplication::translate("QPageSize", "A4 Plus");
      case A4Small:
         return QCoreApplication::translate("QPageSize", "A4 Small");
      case A5Extra:
         return QCoreApplication::translate("QPageSize", "A5 Extra");
      case B5Extra:
         return QCoreApplication::translate("QPageSize", "B5 Extra");
      case JisB0:
         return QCoreApplication::translate("QPageSize", "JIS B0");
      case JisB1:
         return QCoreApplication::translate("QPageSize", "JIS B1");
      case JisB2:
         return QCoreApplication::translate("QPageSize", "JIS B2");
      case JisB3:
         return QCoreApplication::translate("QPageSize", "JIS B3");
      case JisB4:
         return QCoreApplication::translate("QPageSize", "JIS B4");
      case JisB5:
         return QCoreApplication::translate("QPageSize", "JIS B5");
      case JisB6:
         return QCoreApplication::translate("QPageSize", "JIS B6");
      case JisB7:
         return QCoreApplication::translate("QPageSize", "JIS B7");
      case JisB8:
         return QCoreApplication::translate("QPageSize", "JIS B8");
      case JisB9:
         return QCoreApplication::translate("QPageSize", "JIS B9");
      case JisB10:
         return QCoreApplication::translate("QPageSize", "JIS B10");
      case AnsiC:
         return QCoreApplication::translate("QPageSize", "ANSI C");
      case AnsiD:
         return QCoreApplication::translate("QPageSize", "ANSI D");
      case AnsiE:
         return QCoreApplication::translate("QPageSize", "ANSI E");
      case LegalExtra:
         return QCoreApplication::translate("QPageSize", "Legal Extra");
      case LetterExtra:
         return QCoreApplication::translate("QPageSize", "Letter Extra");
      case LetterPlus:
         return QCoreApplication::translate("QPageSize", "Letter Plus");
      case LetterSmall:
         return QCoreApplication::translate("QPageSize", "Letter Small");
      case TabloidExtra:
         return QCoreApplication::translate("QPageSize", "Tabloid Extra");
      case ArchA:
         return QCoreApplication::translate("QPageSize", "Architect A");
      case ArchB:
         return QCoreApplication::translate("QPageSize", "Architect B");
      case ArchC:
         return QCoreApplication::translate("QPageSize", "Architect C");
      case ArchD:
         return QCoreApplication::translate("QPageSize", "Architect D");
      case ArchE:
         return QCoreApplication::translate("QPageSize", "Architect E");
      case Imperial7x9:
         return msgImperialPageSizeInch(7, 9);
      case Imperial8x10:
         return msgImperialPageSizeInch(8, 10);
      case Imperial9x11:
         return msgImperialPageSizeInch(9, 11);
      case Imperial9x12:
         return msgImperialPageSizeInch(9, 12);
      case Imperial10x11:
         return msgImperialPageSizeInch(10, 11);
      case Imperial10x13:
         return msgImperialPageSizeInch(10, 13);
      case Imperial10x14:
         return msgImperialPageSizeInch(10, 14);
      case Imperial12x11:
         return msgImperialPageSizeInch(12, 11);
      case Imperial15x11:
         return msgImperialPageSizeInch(15, 11);
      case Note:
         return QCoreApplication::translate("QPageSize", "Note");
      case Quarto:
         return QCoreApplication::translate("QPageSize", "Quarto");
      case Statement:
         return QCoreApplication::translate("QPageSize", "Statement");
      case SuperA:
         return QCoreApplication::translate("QPageSize", "Super A");
      case SuperB:
         return QCoreApplication::translate("QPageSize", "Super B");
      case Postcard:
         return QCoreApplication::translate("QPageSize", "Postcard");
      case DoublePostcard:
         return QCoreApplication::translate("QPageSize", "Double Postcard");
      case Prc16K:
         return QCoreApplication::translate("QPageSize", "PRC 16K");
      case Prc32K:
         return QCoreApplication::translate("QPageSize", "PRC 32K");
      case Prc32KBig:
         return QCoreApplication::translate("QPageSize", "PRC 32K Big");
      case FanFoldUS:
         return QCoreApplication::translate("QPageSize", "Fan-fold US (14.875 x 11 in)");
      case FanFoldGerman:
         return QCoreApplication::translate("QPageSize", "Fan-fold German (8.5 x 12 in)");
      case FanFoldGermanLegal:
         return QCoreApplication::translate("QPageSize", "Fan-fold German Legal (8.5 x 13 in)");
      case EnvelopeB4:
         return QCoreApplication::translate("QPageSize", "Envelope B4");
      case EnvelopeB5:
         return QCoreApplication::translate("QPageSize", "Envelope B5");
      case EnvelopeB6:
         return QCoreApplication::translate("QPageSize", "Envelope B6");
      case EnvelopeC0:
         return QCoreApplication::translate("QPageSize", "Envelope C0");
      case EnvelopeC1:
         return QCoreApplication::translate("QPageSize", "Envelope C1");
      case EnvelopeC2:
         return QCoreApplication::translate("QPageSize", "Envelope C2");
      case EnvelopeC3:
         return QCoreApplication::translate("QPageSize", "Envelope C3");
      case EnvelopeC4:
         return QCoreApplication::translate("QPageSize", "Envelope C4");
      case EnvelopeC5: // C5E
         return QCoreApplication::translate("QPageSize", "Envelope C5");
      case EnvelopeC6:
         return QCoreApplication::translate("QPageSize", "Envelope C6");
      case EnvelopeC65:
         return QCoreApplication::translate("QPageSize", "Envelope C65");
      case EnvelopeC7:
         return QCoreApplication::translate("QPageSize", "Envelope C7");
      case EnvelopeDL: // DLE:
         return QCoreApplication::translate("QPageSize", "Envelope DL");
      case Envelope9:
         return QCoreApplication::translate("QPageSize", "Envelope US 9");
      case Envelope10: // Comm10E
         return QCoreApplication::translate("QPageSize", "Envelope US 10");
      case Envelope11:
         return QCoreApplication::translate("QPageSize", "Envelope US 11");
      case Envelope12:
         return QCoreApplication::translate("QPageSize", "Envelope US 12");
      case Envelope14:
         return QCoreApplication::translate("QPageSize", "Envelope US 14");
      case EnvelopeMonarch:
         return QCoreApplication::translate("QPageSize", "Envelope Monarch");
      case EnvelopePersonal:
         return QCoreApplication::translate("QPageSize", "Envelope Personal");
      case EnvelopeChou3:
         return QCoreApplication::translate("QPageSize", "Envelope Chou 3");
      case EnvelopeChou4:
         return QCoreApplication::translate("QPageSize", "Envelope Chou 4");
      case EnvelopeInvite:
         return QCoreApplication::translate("QPageSize", "Envelope Invite");
      case EnvelopeItalian:
         return QCoreApplication::translate("QPageSize", "Envelope Italian");
      case EnvelopeKaku2:
         return QCoreApplication::translate("QPageSize", "Envelope Kaku 2");
      case EnvelopeKaku3:
         return QCoreApplication::translate("QPageSize", "Envelope Kaku 3");
      case EnvelopePrc1:
         return QCoreApplication::translate("QPageSize", "Envelope PRC 1");
      case EnvelopePrc2:
         return QCoreApplication::translate("QPageSize", "Envelope PRC 2");
      case EnvelopePrc3:
         return QCoreApplication::translate("QPageSize", "Envelope PRC 3");
      case EnvelopePrc4:
         return QCoreApplication::translate("QPageSize", "Envelope PRC 4");
      case EnvelopePrc5:
         return QCoreApplication::translate("QPageSize", "Envelope PRC 5");
      case EnvelopePrc6:
         return QCoreApplication::translate("QPageSize", "Envelope PRC 6");
      case EnvelopePrc7:
         return QCoreApplication::translate("QPageSize", "Envelope PRC 7");
      case EnvelopePrc8:
         return QCoreApplication::translate("QPageSize", "Envelope PRC 8");
      case EnvelopePrc9:
         return QCoreApplication::translate("QPageSize", "Envelope PRC 9");
      case EnvelopePrc10:
         return QCoreApplication::translate("QPageSize", "Envelope PRC 10");
      case EnvelopeYou4:
         return QCoreApplication::translate("QPageSize", "Envelope You 4");
   }
   return QString();
}

/*!
    Returns the standard QPageSize::PageSizeId of the given \a pointSize in
    points using the given \a matchPolicy.

    If using FuzzyMatch then the point size of the PageSizeId returned may not
    exactly match the \a pointSize you passed in. You should call
    QPageSize::sizePoints() using the returned PageSizeId to find out the actual
    point size of the PageSizeId before using it in any calculations.
*/

QPageSize::PageSizeId QPageSize::id(const QSize &pointSize, SizeMatchPolicy matchPolicy)
{
   return qt_idForPointSize(pointSize, matchPolicy, 0);
}

/*!
    Returns the standard QPageSize::PageSizeId of the given \a size in \a units
    using the given \a matchPolicy.

    If using FuzzyMatch then the unit size of the PageSizeId returned may not
    exactly match the \a size you passed in. You should call
    QPageSize::size() using the returned PageSizeId to find out the actual
    unit size of the PageSizeId before using it in any calculations.
*/

QPageSize::PageSizeId QPageSize::id(const QSizeF &size, Unit units,
   SizeMatchPolicy matchPolicy)
{
   return qt_idForSize(size, units, matchPolicy, 0);
}

/*!
    Returns the PageSizeId for the given Windows DMPAPER enum value \a windowsId.

    If there is no matching PageSizeId then QPageSize::Custom is returned.
*/

QPageSize::PageSizeId QPageSize::id(int windowsId)
{
   return qt_idForWindowsID(windowsId);
}

/*!
    Returns the Windows DMPAPER enum value of the standard \a pageSizeId.

    Not all valid PPD page sizes have a Windows equivalent, in which case 0
    will be returned.
*/

int QPageSize::windowsId(PageSizeId pageSizeId)
{
   return qt_pageSizes[pageSizeId].windowsId;
}

/*!
    Returns the definition size of the standard \a pageSizeId.

    To obtain the definition units, call QPageSize::definitionUnits().
*/

QSizeF QPageSize::definitionSize(PageSizeId pageSizeId)
{
   if (pageSizeId == Custom) {
      return QSizeF();
   }
   return qt_definitionSize(pageSizeId);
}

/*!
    Returns the definition units of the standard \a pageSizeId.

    To obtain the definition size, call QPageSize::definitionSize().
*/

QPageSize::Unit QPageSize::definitionUnits(PageSizeId pageSizeId)
{
   if (pageSizeId == Custom) {
      return Unit(-1);
   }
   return qt_pageSizes[pageSizeId].definitionUnits;
}

/*!
    Returns the size of the standard \a pageSizeId in the requested \a units.
*/

QSizeF QPageSize::size(PageSizeId pageSizeId, Unit units)
{
   if (pageSizeId == Custom) {
      return QSizeF();
   }
   return qt_unitSize(pageSizeId, units);
}

/*!
    Returns the size of the standard \a pageSizeId in Points.
*/

QSize QPageSize::sizePoints(PageSizeId pageSizeId)
{
   if (pageSizeId == Custom) {
      return QSize();
   }
   return QSize(qt_pageSizes[pageSizeId].widthPoints, qt_pageSizes[pageSizeId].heightPoints);
}

/*!
    Returns the size of the standard \a pageSizeId in Device Pixels
    for the given \a resolution.
*/

QSize QPageSize::sizePixels(PageSizeId pageSizeId, int resolution)
{
   if (pageSizeId == Custom) {
      return QSize();
   }
   return qt_convertPointsToPixels(sizePoints(pageSizeId), resolution);
}

QDebug operator<<(QDebug dbg, const QPageSize &pageSize)
{
   QDebugStateSaver saver(dbg);
   dbg.nospace();
   dbg.noquote();
   dbg << "QPageSize(";
   if (pageSize.isValid()) {
      dbg << '"' << pageSize.name() << "\", key=\"" << pageSize.key()
         << "\", " << pageSize.sizePoints().width() << 'x'
         << pageSize.sizePoints().height() << "pt, id=" << pageSize.id();
   } else {
      dbg.nospace() << "QPageSize()";
   }
   dbg << ')';
   return dbg;
}


