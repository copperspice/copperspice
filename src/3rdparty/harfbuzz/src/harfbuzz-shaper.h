/*
 * Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies)
 *
 * This is part of HarfBuzz, an OpenType Layout engine library.
 *
 * Permission is hereby granted, without written agreement and without
 * license or royalty fees, to use, copy, modify, and distribute this
 * software and its documentation for any purpose, provided that the
 * above copyright notice and the following two paragraphs appear in
 * all copies of this software.
 *
 * IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE TO ANY PARTY FOR
 * DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES
 * ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN
 * IF THE COPYRIGHT HOLDER HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 *
 * THE COPYRIGHT HOLDER SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING,
 * BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
 * FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS
 * ON AN "AS IS" BASIS, AND THE COPYRIGHT HOLDER HAS NO OBLIGATION TO
 * PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.
 */

#ifndef HARFBUZZ_SHAPER_H
#define HARFBUZZ_SHAPER_H

#include "harfbuzz-global.h"
#include "harfbuzz-gdef.h"
#include "harfbuzz-gpos.h"
#include "harfbuzz-gsub.h"
#include "harfbuzz-external.h"
#include "harfbuzz-stream-private.h"

HB_BEGIN_HEADER

/*
   using anything else than signed or unsigned for bitfields in C is non standard,
   but accepted by almost all compilers. And it gives a significant reduction in
   memory consumption as HB_CharAttributes and HB_GlyphAttributes will not have
   a 4 byte alignment
*/
#ifdef  __xlC__
typedef unsigned hb_bitfield;
#else
typedef hb_uint8 hb_bitfield;
#endif

typedef enum {
        HB_Script_Unknown,
        HB_Script_Inherited,
        HB_Script_Common,

        HB_Script_Latin,
        HB_Script_Greek,
        HB_Script_Cyrillic,
        HB_Script_Armenian,
        HB_Script_Hebrew,
        HB_Script_Arabic,
        HB_Script_Syriac,
        HB_Script_Thaana,
        HB_Script_Devanagari,
        HB_Script_Bengali,
        HB_Script_Gurmukhi,
        HB_Script_Gujarati,
        HB_Script_Oriya,
        HB_Script_Tamil,
        HB_Script_Telugu,
        HB_Script_Kannada,
        HB_Script_Malayalam,
        HB_Script_Sinhala,
        HB_Script_Thai,
        HB_Script_Lao,
        HB_Script_Tibetan,
        HB_Script_Myanmar,
        HB_Script_Georgian,
        HB_Script_Hangul,
        HB_Script_Ethiopic,
        HB_Script_Cherokee,
        HB_Script_CanadianAboriginal,
        HB_Script_Ogham,
        HB_Script_Runic,
        HB_Script_Khmer,

        HB_Script_Mongolian,
        HB_Script_Hiragana,
        HB_Script_Katakana,
        HB_Script_Bopomofo,
        HB_Script_Han,
        HB_Script_Yi,
        HB_Script_OldItalic,
        HB_Script_Gothic,
        HB_Script_Deseret,
        HB_Script_Tagalog,
        HB_Script_Hanunoo,
        HB_Script_Buhid,
        HB_Script_Tagbanwa,
        HB_Script_Coptic,

        // Unicode 4.0 additions
        HB_Script_Limbu,
        HB_Script_TaiLe,
        HB_Script_LinearB,
        HB_Script_Ugaritic,
        HB_Script_Shavian,
        HB_Script_Osmanya,
        HB_Script_Cypriot,
        HB_Script_Braille,

        // Unicode 4.1 additions
        HB_Script_Buginese,
        HB_Script_NewTaiLue,
        HB_Script_Glagolitic,
        HB_Script_Tifinagh,
        HB_Script_SylotiNagri,
        HB_Script_OldPersian,
        HB_Script_Kharoshthi,

        // Unicode 5.0 additions
        HB_Script_Balinese,
        HB_Script_Cuneiform,
        HB_Script_Phoenician,
        HB_Script_PhagsPa,
        HB_Script_Nko,

        // Unicode 5.1 additions
        HB_Script_Sundanese,
        HB_Script_Lepcha,
        HB_Script_OlChiki,
        HB_Script_Vai,
        HB_Script_Saurashtra,
        HB_Script_KayahLi,
        HB_Script_Rejang,
        HB_Script_Lycian,
        HB_Script_Carian,
        HB_Script_Lydian,
        HB_Script_Cham,

        // Unicode 5.2 additions
        HB_Script_TaiTham,
        HB_Script_TaiViet,
        HB_Script_Avestan,
        HB_Script_EgyptianHieroglyphs,
        HB_Script_Samaritan,
        HB_Script_Lisu,
        HB_Script_Bamum,
        HB_Script_Javanese,
        HB_Script_MeeteiMayek,
        HB_Script_ImperialAramaic,
        HB_Script_OldSouthArabian,
        HB_Script_InscriptionalParthian,
        HB_Script_InscriptionalPahlavi,
        HB_Script_OldTurkic,
        HB_Script_Kaithi,

        // Unicode 6.0 additions
        HB_Script_Batak,
        HB_Script_Brahmi,
        HB_Script_Mandaic,

        // Unicode 6.1 additions
        HB_Script_Chakma,
        HB_Script_MeroiticCursive,
        HB_Script_MeroiticHieroglyphs,
        HB_Script_Miao,
        HB_Script_Sharada,
        HB_Script_SoraSompeng,
        HB_Script_Takri,

        // Unicode 7.0 additions
        HB_Script_CaucasianAlbanian,
        HB_Script_BassaVah,
        HB_Script_Duployan,
        HB_Script_Elbasan,
        HB_Script_Grantha,
        HB_Script_PahawhHmong,
        HB_Script_Khojki,
        HB_Script_LinearA,
        HB_Script_Mahajani,
        HB_Script_Manichaean,
        HB_Script_MendeKikakui,
        HB_Script_Modi,
        HB_Script_Mro,
        HB_Script_OldNorthArabian,
        HB_Script_Nabataean,
        HB_Script_Palmyrene,
        HB_Script_PauCinHau,
        HB_Script_OldPermic,
        HB_Script_PsalterPahlavi,
        HB_Script_Siddham,
        HB_Script_Khudawadi,
        HB_Script_Tirhuta,
	HB_Script_WarangCiti,

        // Unicode 8.0 additions
        HB_Script_Ahom,
        HB_Script_AnatolianHieroglyphs,
        HB_Script_Hatran,
        HB_Script_Multani,
        HB_Script_OldHungarian,
	HB_Script_SignWriting,

	// Unicode 9.0 additions
	HB_Script_Adlam,
	HB_Script_Bhaiksuki,
	HB_Script_Marchen,
	HB_Script_Newa,
	HB_Script_Osage,
	HB_Script_Tangut,

	// Unicode 10.0 additions
	HB_Script_MasaramGondi,
	HB_Script_Nushu,
	HB_Script_Soyombo,
	HB_Script_ZanabazarSquare,

	// Unicode 11.0 additions
	HB_Script_HanafiRohingya,
	HB_Script_OldSogdian,
	HB_Script_Dogra,
	HB_Script_GunjalaGondi,
	HB_Script_Makasar,
	HB_Script_Medefaidrin,

	HB_ScriptCount
} HB_Script;

typedef struct
{
    hb_uint32 pos;
    hb_uint32 length;
    HB_Script script;
    hb_uint8 bidiLevel;
} HB_ScriptItem;

typedef enum {
    HB_NoBreak,
    HB_SoftHyphen,
    HB_Break,
    HB_ForcedBreak
} HB_LineBreakType;


typedef struct {
    /*HB_LineBreakType*/ hb_bitfield lineBreakType  :2;
    /*HB_Bool*/ hb_bitfield whiteSpace              :1;     /* A unicode whitespace character, except NBSP, ZWNBSP */
    /*HB_Bool*/ hb_bitfield charStop                :1;     /* Valid cursor position (for left/right arrow) */
    /*HB_Bool*/ hb_bitfield wordBoundary            :1;
    /*HB_Bool*/ hb_bitfield sentenceBoundary        :1;
    hb_bitfield unused                              :2;
} HB_CharAttributes;

void HB_GetCharAttributes(const HB_UChar16 *string, hb_uint32 stringLength,
                          const HB_ScriptItem *items, hb_uint32 numItems,
                          HB_CharAttributes *attributes);

/* requires HB_GetCharAttributes to be called before */
void HB_GetWordBoundaries(const HB_UChar16 *string, hb_uint32 stringLength,
                          const HB_ScriptItem *items, hb_uint32 numItems,
                          HB_CharAttributes *attributes);

/* requires HB_GetCharAttributes to be called before */
void HB_GetSentenceBoundaries(const HB_UChar16 *string, hb_uint32 stringLength,
                          const HB_ScriptItem *items, hb_uint32 numItems,
                          HB_CharAttributes *attributes);


typedef enum {
    HB_LeftToRight = 0,
    HB_RightToLeft = 1
} HB_StringToGlyphsFlags;

typedef enum {
    HB_ShaperFlag_Default = 0,
    HB_ShaperFlag_NoKerning = 1,
    HB_ShaperFlag_UseDesignMetrics = 2
} HB_ShaperFlag;

/*
   highest value means highest priority for justification. Justification is done by first inserting kashidas
   starting with the highest priority positions, then stretching spaces, afterwards extending inter char
   spacing, and last spacing between arabic words.
   NoJustification is for example set for arabic where no Kashida can be inserted or for diacritics.
*/
typedef enum {
    HB_NoJustification= 0,   /* Justification can't be applied after this glyph */
    HB_Arabic_Space   = 1,   /* This glyph represents a space inside arabic text */
    HB_Character      = 2,   /* Inter-character justification point follows this glyph */
    HB_Space          = 4,   /* This glyph represents a blank outside an Arabic run */
    HB_Arabic_Normal  = 7,   /* Normal Middle-Of-Word glyph that connects to the right (begin) */
    HB_Arabic_Waw     = 8,   /* Next character is final form of Waw/Ain/Qaf/Fa */
    HB_Arabic_BaRa    = 9,   /* Next two chars are Ba + Ra/Ya/AlefMaksura */
    HB_Arabic_Alef    = 10,  /* Next character is final form of Alef/Tah/Lam/Kaf/Gaf */
    HB_Arabic_HaaDal  = 11,  /* Next character is final form of Haa/Dal/Taa Marbutah */
    HB_Arabic_Seen    = 12,  /* Initial or Medial form Of Seen/Sad */
    HB_Arabic_Kashida = 13   /* Kashida(U+640) in middle of word */
} HB_JustificationClass;

// This structure is binary compatible with Uniscribe's SCRIPT_VISATTR. Would be nice to keep it like that
typedef struct {
    hb_bitfield justification   :4;  /* Justification class */
    hb_bitfield clusterStart    :1;  /* First glyph of representation of cluster */
    hb_bitfield mark            :1;  /* needs to be positioned around base char */
    hb_bitfield zeroWidth       :1;  /* ZWJ, ZWNJ etc, with no width */
    hb_bitfield dontPrint       :1;
    hb_bitfield combiningClass  :8;
} HB_GlyphAttributes;

typedef struct HB_FaceRec_ {
    HB_Bool isSymbolFont;

    HB_GDEF gdef;
    HB_GSUB gsub;
    HB_GPOS gpos;
    HB_Bool supported_scripts[HB_ScriptCount];
    HB_Buffer buffer;
    HB_Script current_script;
    int current_flags; /* HB_ShaperFlags */
    HB_Bool has_opentype_kerning;
    HB_Bool glyphs_substituted;
    HB_GlyphAttributes *tmpAttributes;
    unsigned int *tmpLogClusters;
    int length;
    int orig_nglyphs;
} HB_FaceRec;

typedef HB_Error (*HB_GetFontTableFunc)(void *font, HB_Tag tag, HB_Byte *buffer, HB_UInt *length);

HB_Face HB_NewFace(void *font, HB_GetFontTableFunc tableFunc);
void HB_FreeFace(HB_Face face);

typedef struct {
    HB_Fixed x, y;
    HB_Fixed width, height;
    HB_Fixed xOffset, yOffset;
} HB_GlyphMetrics;

typedef enum {
    HB_FontAscent,
    HB_FontDescent
} HB_FontMetric;

typedef struct {
    HB_Bool  (*convertStringToGlyphIndices)(HB_Font font, const HB_UChar16 *string, hb_uint32 length, HB_Glyph *glyphs, hb_uint32 *numGlyphs, HB_Bool rightToLeft);
    void     (*getGlyphAdvances)(HB_Font font, const HB_Glyph *glyphs, hb_uint32 numGlyphs, HB_Fixed *advances, int flags /*HB_ShaperFlag*/);
    HB_Bool  (*canRender)(HB_Font font, const HB_UChar16 *string, hb_uint32 length);
    /* implementation needs to make sure to load a scaled glyph, so /no/ FT_LOAD_NO_SCALE */
    HB_Error (*getPointInOutline)(HB_Font font, HB_Glyph glyph, int flags /*HB_ShaperFlag*/, hb_uint32 point, HB_Fixed *xpos, HB_Fixed *ypos, hb_uint32 *nPoints);
    void     (*getGlyphMetrics)(HB_Font font, HB_Glyph glyph, HB_GlyphMetrics *metrics);
    HB_Fixed (*getFontMetric)(HB_Font font, HB_FontMetric metric);
} HB_FontClass;

typedef struct HB_Font_ {
    const HB_FontClass *klass;

    /* Metrics */
    HB_UShort x_ppem, y_ppem;
    HB_16Dot16 x_scale, y_scale;

    void *userData;
} HB_FontRec;

typedef struct HB_ShaperItem_ HB_ShaperItem;

struct HB_ShaperItem_ {
    const HB_UChar16 *string;               /* input: the Unicode UTF16 text to be shaped */
    hb_uint32 stringLength;                 /* input: the length of the input in 16-bit words */
    HB_ScriptItem item;                     /* input: the current run to be shaped: a run of text all in the same script that is a substring of <string> */
    HB_Font font;                           /* input: the font: scale, units and function pointers supplying glyph indices and metrics */
    HB_Face face;                           /* input: the shaper state; current script, access to the OpenType tables , etc. */
    int shaperFlags;                        /* input (unused) should be set to 0; intended to support flags defined in HB_ShaperFlag */
    HB_Bool glyphIndicesPresent;            /* input: true if the <glyphs> array contains glyph indices ready to be shaped */
    hb_uint32 initialGlyphCount;            /* input: if glyphIndicesPresent is true, the number of glyph indices in the <glyphs> array */

    hb_uint32 num_glyphs;                   /* input: capacity of output arrays <glyphs>, <attributes>, <advances>, <offsets>, and <log_clusters>; */
                                            /* output: required capacity (may be larger than actual capacity) */

    HB_Glyph *glyphs;                       /* output: <num_glyphs> indices of shaped glyphs */
    HB_GlyphAttributes *attributes;         /* output: <num_glyphs> glyph attributes */
    HB_Fixed *advances;                     /* output: <num_glyphs> advances */
    HB_FixedPoint *offsets;                 /* output: <num_glyphs> offsets */
    unsigned short *log_clusters;           /* output: for each output glyph, the index in the input of the start of its logical cluster */

    /* internal */
    HB_Bool kerning_applied;                /* output: true if kerning was applied by the shaper */
};

HB_Bool HB_ShapeItem(HB_ShaperItem *item);

HB_END_HEADER

#endif
