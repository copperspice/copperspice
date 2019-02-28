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

#include <qharfbuzz_gui_p.h>

#include <qstring.h>
#include <qvector.h>
#include <qfontengine_p.h>

// Font routines
static hb_bool_t cs_font_get_glyph(hb_font_t *, void *font_data, hb_codepoint_t unicode, hb_codepoint_t,
   hb_codepoint_t *glyph, void *)
{
   QFontEngine *fe = static_cast<QFontEngine *>(font_data);
   Q_ASSERT(fe);

   *glyph = fe->glyphIndex(unicode);

   return true;
}

static hb_position_t cs_font_get_glyph_h_advance(hb_font_t *font, void *font_data, hb_codepoint_t glyph, void *)
{
   QFontEngine *fe = static_cast<QFontEngine *>(font_data);
   Q_ASSERT(fe);

   QFixed advance;

   QGlyphLayout g;
   g.numGlyphs = 1;
   g.glyphs = &glyph;
   g.advances = &advance;

   fe->recalcAdvances(&g, QFontEngine::ShaperFlags(cs_font_get_use_design_metrics(font)));

   return advance.value();
}

static hb_position_t cs_font_get_glyph_v_advance(hb_font_t *font, void *font_Data,
   hb_codepoint_t glyph, void *user_data)
{
   qCritical("cs_font_get_glyph_v_advance: vertical writing is not supported");
   return 0;
}

static hb_bool_t cs_font_get_glyph_h_origin(hb_font_t *font, void *font_data,
   hb_codepoint_t, hb_position_t *x, hb_position_t *y, void *user_data)
{
   // always work in the horizontal coordinates
   return true;
}

static hb_bool_t cs_font_get_glyph_v_origin(hb_font_t *font, void *font_data,
   hb_codepoint_t glyph, hb_position_t *x, hb_position_t *y, void *user_data)
{
   qCritical("cs_font_get_glyph_v_origin: vertical writing is not supported");
   return false;
}

static hb_position_t cs_font_get_glyph_h_kerning(hb_font_t *font, void *font_data,
   hb_codepoint_t first_glyph, hb_codepoint_t second_glyph, void *)
{
   QFontEngine *fe = static_cast<QFontEngine *>(font_data);
   Q_ASSERT(fe);

   glyph_t glyphs[2] = { first_glyph, second_glyph };
   QFixed advance;

   QGlyphLayout g;
   g.numGlyphs = 2;
   g.glyphs = glyphs;
   g.advances = &advance;

   fe->doKerning(&g, QFontEngine::ShaperFlags(cs_font_get_use_design_metrics(font)));

   return advance.value();
}

static hb_position_t cs_font_get_glyph_v_kerning(hb_font_t *, void *, hb_codepoint_t, hb_codepoint_t, void *)
{
   qCritical("hb_qt_get_glyph_v_kerning: vertical writing is not supported");
   return 0;
}

static hb_bool_t cs_font_get_glyph_extents(hb_font_t *, void *font_data,
   hb_codepoint_t glyph, hb_glyph_extents_t *extents, void *)
{
   QFontEngine *fe = static_cast<QFontEngine *>(font_data);
   Q_ASSERT(fe);

   glyph_metrics_t gm = fe->boundingBox(glyph);

   extents->x_bearing = gm.x.value();
   extents->y_bearing = gm.y.value();
   extents->width = gm.width.value();
   extents->height = gm.height.value();

   return true;
}

static hb_bool_t cs_font_get_glyph_contour_point(hb_font_t *, void *font_data, hb_codepoint_t glyph,
   unsigned int point_index, hb_position_t *x, hb_position_t *y, void *)
{
   QFontEngine *fe = static_cast<QFontEngine *>(font_data);
   Q_ASSERT(fe);

   QFixed xpos, ypos;
   quint32 numPoints = 1;

   if (fe->getPointInOutline(glyph, 0, point_index, &xpos, &ypos, &numPoints) == 0) {
      *x = xpos.value();
      *y = ypos.value();
      return true;
   }

   *x = *y = 0;
   return false;
}

static hb_bool_t cs_font_get_glyph_name(hb_font_t *font, void *font_data,
   hb_codepoint_t glyph, char *name, unsigned int size, void *user_data)
{
   qCritical("hb_qt_font_get_glyph_name: not implemented");

   if (size) {
      *name = '\0';
   }

   return false;
}

static hb_bool_t cs_font_get_glyph_from_name(hb_font_t *font, void *font_data, const char *name, int len,
   hb_codepoint_t *glyph, void *user_data)
{
   qCritical("hb_qt_font_get_glyph_from_name: not implemented");

   *glyph = 0;
   return false;
}

static hb_user_data_key_t internal_useDesignMetricsKey;

void cs_font_set_use_design_metrics(hb_font_t *font, uint value)
{
   hb_font_set_user_data(font, &internal_useDesignMetricsKey, (void *)quintptr(value), NULL, true);
}

uint cs_font_get_use_design_metrics(hb_font_t *font)
{
   return quintptr(hb_font_get_user_data(font, &internal_useDesignMetricsKey));
}

struct cs_hb_font_funcs_t {

   cs_hb_font_funcs_t() {
      funcs = hb_font_funcs_create();

      hb_font_funcs_set_glyph_h_advance_func(funcs,     cs_font_get_glyph_h_advance, NULL, NULL);
      hb_font_funcs_set_glyph_v_advance_func(funcs,     cs_font_get_glyph_v_advance, NULL, NULL);

      hb_font_funcs_set_glyph_h_origin_func(funcs,      cs_font_get_glyph_h_origin, NULL, NULL);
      hb_font_funcs_set_glyph_v_origin_func(funcs,      cs_font_get_glyph_v_origin, NULL, NULL);

      hb_font_funcs_set_glyph_h_kerning_func(funcs,     cs_font_get_glyph_h_kerning, NULL, NULL);
      hb_font_funcs_set_glyph_v_kerning_func(funcs,     cs_font_get_glyph_v_kerning, NULL, NULL);
      hb_font_funcs_set_glyph_extents_func(funcs,       cs_font_get_glyph_extents, NULL, NULL);

      hb_font_funcs_set_glyph_contour_point_func(funcs, cs_font_get_glyph_contour_point, NULL, NULL);
      hb_font_funcs_set_glyph_name_func(funcs,          cs_font_get_glyph_name, NULL, NULL);
      hb_font_funcs_set_glyph_from_name_func(funcs,     cs_font_get_glyph_from_name, NULL, NULL);
   }

   ~cs_hb_font_funcs_t() {
      hb_font_funcs_destroy(funcs);
   }

   hb_font_funcs_t *funcs;
};

hb_font_funcs_t *cs_get_font_funcs()
{
   static cs_hb_font_funcs_t retval;
   return retval.funcs;
}

// **

static hb_blob_t *internal_hb_reference_table(hb_face_t * /*face*/, hb_tag_t tag, void *user_data)
{
   QFontEngine::FaceData *data = static_cast<QFontEngine::FaceData *>(user_data);
   Q_ASSERT(data);

   qt_get_font_table_func_ptr funcPtr = data->font_table_func_ptr;
   Q_ASSERT(funcPtr);

   uint length = 0;
   if (! funcPtr(data->user_data, tag, 0, &length)) {
      return hb_blob_get_empty();
   }

   char *buffer = static_cast<char *>(malloc(length));
   Q_CHECK_PTR(buffer);

   if (! funcPtr(data->user_data, tag, reinterpret_cast<uchar *>(buffer), &length)) {
      length = 0;
   }

   return hb_blob_create(const_cast<const char *>(buffer), length, HB_MEMORY_MODE_READONLY, buffer, free);
}

static inline hb_face_t *internal_hb_face_create(QFontEngine *fe)
{
   QFontEngine::FaceData *data = static_cast<QFontEngine::FaceData *>(malloc(sizeof(QFontEngine::FaceData)));
   Q_CHECK_PTR(data);

   data->user_data           = fe->faceData.user_data;
   data->font_table_func_ptr = fe->faceData.font_table_func_ptr;

   hb_face_t *face = hb_face_create_for_tables(internal_hb_reference_table, (void *)data, free);
   if (hb_face_is_immutable(face)) {
      hb_face_destroy(face);
      return NULL;
   }

   hb_face_set_index(face, fe->faceId().index);
   hb_face_set_upem(face,  fe->emSquareSize().truncate());

   return face;
}

static void internal_hb_face_release(void *user_data)
{
   if (user_data) {
      hb_face_destroy(static_cast<hb_face_t *>(user_data));
   }
}

hb_face_t *cs_face_get_for_engine(QFontEngine *fe)
{
   Q_ASSERT(fe && fe->type() != QFontEngine::Multi);

   if (! fe->face_) {
      fe->face_ = internal_hb_face_create(fe);

      if (! fe->face_) {
         return NULL;
      }

      fe->face_destroy_func_ptr = internal_hb_face_release;
   }

   return static_cast<hb_face_t *>(fe->face_);
}

static inline hb_font_t *internal_hb_font_create(QFontEngine *fe)
{
   hb_face_t *face = cs_face_get_for_engine(fe);

   if (! face) {
      return NULL;
   }

   hb_font_t *font = hb_font_create(face);

   if (hb_font_is_immutable(font)) {
      hb_font_destroy(font);
      return NULL;
   }

   const int y_ppem = fe->fontDef.pixelSize;
   const int x_ppem = (fe->fontDef.pixelSize * fe->fontDef.stretch) / 100;

   hb_font_set_funcs(font, cs_get_font_funcs(), (void *)fe, NULL);

#ifdef Q_OS_MAC
   hb_font_set_scale(font, QFixed(x_ppem).value(),  QFixed(y_ppem).value());
#else
   hb_font_set_scale(font, QFixed(x_ppem).value(), -QFixed(y_ppem).value());
#endif

   hb_font_set_ppem(font, x_ppem, y_ppem);

   return font;
}

static void internal_hb_font_release(void *user_data)
{
   if (user_data) {
      hb_font_destroy(static_cast<hb_font_t *>(user_data));
   }
}

hb_font_t *cs_font_get_for_engine(QFontEngine *fe)
{
   Q_ASSERT(fe && fe->type() != QFontEngine::Multi);

   if (! fe->font_) {
      fe->font_ = internal_hb_font_create(fe);

      if (fe->font_) {
         return NULL;
      }

      fe->font_destroy_func_ptr = internal_hb_font_release;
   }

   return static_cast<hb_font_t *>(fe->font_);
}
