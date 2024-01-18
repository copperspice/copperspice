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

#ifndef QRasterDefs_P_H
#define QRasterDefs_P_H


typedef struct QT_FT_Vector_ {
   int  x;
   int  y;

} QT_FT_Vector;

typedef struct  QT_FT_BBox_ {
   int  xMin, yMin;
   int  xMax, yMax;

} QT_FT_BBox;

typedef enum  QT_FT_Pixel_Mode_ {
   QT_FT_PIXEL_MODE_NONE = 0,
   QT_FT_PIXEL_MODE_MONO,
   QT_FT_PIXEL_MODE_GRAY,
   QT_FT_PIXEL_MODE_GRAY2,
   QT_FT_PIXEL_MODE_GRAY4,
   QT_FT_PIXEL_MODE_LCD,
   QT_FT_PIXEL_MODE_LCD_V,

   QT_FT_PIXEL_MODE_MAX      /* do not remove */

} QT_FT_Pixel_Mode;

#define qt_ft_pixel_mode_none   QT_FT_PIXEL_MODE_NONE
#define qt_ft_pixel_mode_mono   QT_FT_PIXEL_MODE_MONO
#define qt_ft_pixel_mode_grays  QT_FT_PIXEL_MODE_GRAY
#define qt_ft_pixel_mode_pal2   QT_FT_PIXEL_MODE_GRAY2
#define qt_ft_pixel_mode_pal4   QT_FT_PIXEL_MODE_GRAY4


typedef struct  QT_FT_Bitmap_ {
   int             rows;
   int             width;
   int             pitch;
   unsigned char  *buffer;
   short           num_grays;
   char            pixel_mode;
   char            palette_mode;
   void           *palette;

} QT_FT_Bitmap;

typedef struct  QT_FT_Outline_ {
   int      n_contours;       /* number of contours in glyph        */
   int      n_points;         /* number of points in the glyph      */

   QT_FT_Vector  *points;     /* the outline's points               */
   char     *tags;            /* the points flags                   */
   int      *contours;        /* the contour end points             */

   int         flags;         /* outline masks                      */

} QT_FT_Outline;


#define QT_FT_OUTLINE_NONE             0x0
#define QT_FT_OUTLINE_OWNER            0x1
#define QT_FT_OUTLINE_EVEN_ODD_FILL    0x2
#define QT_FT_OUTLINE_REVERSE_FILL     0x4
#define QT_FT_OUTLINE_IGNORE_DROPOUTS  0x8

#define QT_FT_OUTLINE_HIGH_PRECISION   0x100
#define QT_FT_OUTLINE_SINGLE_PASS      0x200

#define qt_ft_outline_none             QT_FT_OUTLINE_NONE
#define qt_ft_outline_owner            QT_FT_OUTLINE_OWNER
#define qt_ft_outline_even_odd_fill    QT_FT_OUTLINE_EVEN_ODD_FILL
#define qt_ft_outline_reverse_fill     QT_FT_OUTLINE_REVERSE_FILL
#define qt_ft_outline_ignore_dropouts  QT_FT_OUTLINE_IGNORE_DROPOUTS
#define qt_ft_outline_high_precision   QT_FT_OUTLINE_HIGH_PRECISION
#define qt_ft_outline_single_pass      QT_FT_OUTLINE_SINGLE_PASS

#define QT_FT_CURVE_TAG( flag )  ( flag & 3 )

#define QT_FT_CURVE_TAG_ON           1
#define QT_FT_CURVE_TAG_CONIC        0
#define QT_FT_CURVE_TAG_CUBIC        2

#define QT_FT_CURVE_TAG_TOUCH_X      8  /* reserved for the TrueType hinter */
#define QT_FT_CURVE_TAG_TOUCH_Y     16  /* reserved for the TrueType hinter */

#define QT_FT_CURVE_TAG_TOUCH_BOTH  ( QT_FT_CURVE_TAG_TOUCH_X | \
                                   QT_FT_CURVE_TAG_TOUCH_Y )

#define  QT_FT_Curve_Tag_On       QT_FT_CURVE_TAG_ON
#define  QT_FT_Curve_Tag_Conic    QT_FT_CURVE_TAG_CONIC
#define  QT_FT_Curve_Tag_Cubic    QT_FT_CURVE_TAG_CUBIC
#define  QT_FT_Curve_Tag_Touch_X  QT_FT_CURVE_TAG_TOUCH_X
#define  QT_FT_Curve_Tag_Touch_Y  QT_FT_CURVE_TAG_TOUCH_Y


typedef int
(*QT_FT_Outline_MoveToFunc)( QT_FT_Vector  *to, void *user );

#define QT_FT_Outline_MoveTo_Func  QT_FT_Outline_MoveToFunc


typedef int
(*QT_FT_Outline_LineToFunc)( QT_FT_Vector  *to, void *user );

#define  QT_FT_Outline_LineTo_Func  QT_FT_Outline_LineToFunc


typedef int
(*QT_FT_Outline_ConicToFunc)( QT_FT_Vector *control, QT_FT_Vector *to, void *user );

#define  QT_FT_Outline_ConicTo_Func  QT_FT_Outline_ConicToFunc

typedef int
(*QT_FT_Outline_CubicToFunc)( QT_FT_Vector  *control1,
   QT_FT_Vector  *control2,
   QT_FT_Vector  *to,
   void *user );

#define  QT_FT_Outline_CubicTo_Func  QT_FT_Outline_CubicToFunc


typedef struct  QT_FT_Outline_Funcs_ {
   QT_FT_Outline_MoveToFunc   move_to;
   QT_FT_Outline_LineToFunc   line_to;
   QT_FT_Outline_ConicToFunc  conic_to;
   QT_FT_Outline_CubicToFunc  cubic_to;

   int  shift;
   int  delta;

} QT_FT_Outline_Funcs;


#ifndef QT_FT_IMAGE_TAG
#define QT_FT_IMAGE_TAG( value, _x1, _x2, _x3, _x4 )  \
          value = ( ( (unsigned long)_x1 << 24 ) | \
                    ( (unsigned long)_x2 << 16 ) | \
                    ( (unsigned long)_x3 << 8  ) | \
                      (unsigned long)_x4         )
#endif


typedef enum  QT_FT_Glyph_Format_ {
   QT_FT_IMAGE_TAG( QT_FT_GLYPH_FORMAT_NONE, 0, 0, 0, 0 ),

   QT_FT_IMAGE_TAG( QT_FT_GLYPH_FORMAT_COMPOSITE, 'c', 'o', 'm', 'p' ),
   QT_FT_IMAGE_TAG( QT_FT_GLYPH_FORMAT_BITMAP,    'b', 'i', 't', 's' ),
   QT_FT_IMAGE_TAG( QT_FT_GLYPH_FORMAT_OUTLINE,   'o', 'u', 't', 'l' ),
   QT_FT_IMAGE_TAG( QT_FT_GLYPH_FORMAT_PLOTTER,   'p', 'l', 'o', 't' )

} QT_FT_Glyph_Format;


#define qt_ft_glyph_format_none       QT_FT_GLYPH_FORMAT_NONE
#define qt_ft_glyph_format_composite  QT_FT_GLYPH_FORMAT_COMPOSITE
#define qt_ft_glyph_format_bitmap     QT_FT_GLYPH_FORMAT_BITMAP
#define qt_ft_glyph_format_outline    QT_FT_GLYPH_FORMAT_OUTLINE
#define qt_ft_glyph_format_plotter    QT_FT_GLYPH_FORMAT_PLOTTER

typedef struct TRaster_ *QT_FT_Raster;

typedef struct  QT_FT_Span_ {
   short x;
   unsigned short len;
   short y;
   unsigned char coverage;
} QT_FT_Span;


typedef void (*QT_FT_SpanFunc)(int count, const QT_FT_Span *spans, void *worker);

#define QT_FT_Raster_Span_Func   QT_FT_SpanFunc

typedef int (*QT_FT_Raster_BitTest_Func)(int y, int x, void *user );
typedef void (*QT_FT_Raster_BitSet_Func)(int y, int x, void *user );


#define QT_FT_RASTER_FLAG_DEFAULT  0x0
#define QT_FT_RASTER_FLAG_AA       0x1
#define QT_FT_RASTER_FLAG_DIRECT   0x2
#define QT_FT_RASTER_FLAG_CLIP     0x4

/* deprecated */
#define qt_ft_raster_flag_default  QT_FT_RASTER_FLAG_DEFAULT
#define qt_ft_raster_flag_aa       QT_FT_RASTER_FLAG_AA
#define qt_ft_raster_flag_direct   QT_FT_RASTER_FLAG_DIRECT
#define qt_ft_raster_flag_clip     QT_FT_RASTER_FLAG_CLIP


typedef struct  QT_FT_Raster_Params_ {
   QT_FT_Bitmap  *target;
   void *source;
   int flags;
   QT_FT_SpanFunc gray_spans;
   QT_FT_SpanFunc black_spans;
   QT_FT_Raster_BitTest_Func bit_test;     /* doesn't work! */
   QT_FT_Raster_BitSet_Func  bit_set;      /* doesn't work! */
   void *user;
   QT_FT_BBox clip_box;
   int skip_spans;

} QT_FT_Raster_Params;


typedef int
(*QT_FT_Raster_NewFunc)( QT_FT_Raster  *raster );

#define  QT_FT_Raster_New_Func    QT_FT_Raster_NewFunc

typedef void
(*QT_FT_Raster_DoneFunc)( QT_FT_Raster  raster );

#define  QT_FT_Raster_Done_Func   QT_FT_Raster_DoneFunc

typedef void
(*QT_FT_Raster_ResetFunc)( QT_FT_Raster raster, unsigned char *pool_base, unsigned long  pool_size );

#define  QT_FT_Raster_Reset_Func   QT_FT_Raster_ResetFunc

typedef int
(*QT_FT_Raster_SetModeFunc)( QT_FT_Raster raster, unsigned long mode, void *args );

#define  QT_FT_Raster_Set_Mode_Func  QT_FT_Raster_SetModeFunc

typedef int
(*QT_FT_Raster_RenderFunc)( QT_FT_Raster raster, QT_FT_Raster_Params  *params );

#define  QT_FT_Raster_Render_Func    QT_FT_Raster_RenderFunc


typedef struct  QT_FT_Raster_Funcs_ {
   QT_FT_Glyph_Format         glyph_format;
   QT_FT_Raster_NewFunc       raster_new;
   QT_FT_Raster_ResetFunc     raster_reset;
   QT_FT_Raster_SetModeFunc   raster_set_mode;
   QT_FT_Raster_RenderFunc    raster_render;
   QT_FT_Raster_DoneFunc      raster_done;

} QT_FT_Raster_Funcs;

#endif