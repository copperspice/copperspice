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

#ifndef QPAINTER_P_H
#define QPAINTER_P_H

#include <QtGui/qbrush.h>
#include <QtGui/qfont.h>
#include <QtGui/qpen.h>
#include <QtGui/qregion.h>
#include <QtGui/qmatrix.h>
#include <QtGui/qpainter.h>
#include <QtGui/qpainterpath.h>
#include <QtGui/qpaintengine.h>

#include <qpen_p.h>

class QRawFont;
class QPaintEngine;
class QEmulationPaintEngine;
class QPaintEngineEx;

struct QFixedPoint;
struct QTLWExtra;

struct DataPtrContainer {
   void *ptr;
};

inline const void *data_ptr(const QTransform &t)
{
   return (DataPtrContainer *) &t;
}

inline bool qtransform_fast_equals(const QTransform &a, const QTransform &b)
{
   return data_ptr(a) == data_ptr(b);
}

// QPen inline functions
inline QPen::DataPtr &data_ptr(const QPen &p)
{
   return const_cast<QPen &>(p).data_ptr();
}

inline bool qpen_fast_equals(const QPen &a, const QPen &b)
{
   return data_ptr(a) == data_ptr(b);
}

inline QBrush qpen_brush(const QPen &p)
{
   return data_ptr(p)->brush;
}

inline qreal qpen_widthf(const QPen &p)
{
   return data_ptr(p)->width;
}

inline Qt::PenStyle qpen_style(const QPen &p)
{
   return data_ptr(p)->style;
}

inline Qt::PenCapStyle qpen_capStyle(const QPen &p)
{
   return data_ptr(p)->capStyle;
}
inline Qt::PenJoinStyle qpen_joinStyle(const QPen &p)
{
   return data_ptr(p)->joinStyle;
}

// QBrush inline functions...
inline QBrush::DataPtr &data_ptr(const QBrush &p)
{
   return const_cast<QBrush &>(p).data_ptr();
}

inline bool qbrush_fast_equals(const QBrush &a, const QBrush &b)
{
   return data_ptr(a) == data_ptr(b);
}

inline Qt::BrushStyle qbrush_style(const QBrush &b)
{
   return data_ptr(b)->style;
}

inline const QColor &qbrush_color(const QBrush &b)
{
   return data_ptr(b)->color;
}

inline bool qbrush_has_transform(const QBrush &b)
{
   return data_ptr(b)->transform.type() > QTransform::TxNone;
}

class QPainterClipInfo
{
 public:
   enum ClipType { RegionClip, PathClip, RectClip, RectFClip };

   QPainterClipInfo(const QPainterPath &p, Qt::ClipOperation op, const QTransform &m) :
      clipType(PathClip), matrix(m), operation(op), path(p) { }

   QPainterClipInfo(const QRegion &r, Qt::ClipOperation op, const QTransform &m) :
      clipType(RegionClip), matrix(m), operation(op), region(r) { }

   QPainterClipInfo(const QRect &r, Qt::ClipOperation op, const QTransform &m) :
      clipType(RectClip), matrix(m), operation(op), rect(r) { }

   QPainterClipInfo(const QRectF &r, Qt::ClipOperation op, const QTransform &m) :
      clipType(RectFClip), matrix(m), operation(op), rectf(r) { }

   ClipType clipType;
   QTransform matrix;
   Qt::ClipOperation operation;
   QPainterPath path;
   QRegion region;
   QRect rect;
   QRectF rectf;

   // ###
   //     union {
   //         QRegionData *d;
   //         QPainterPathPrivate *pathData;

   //         struct {
   //             int x, y, w, h;
   //         } rectData;
   //         struct {
   //             qreal x, y, w, h;
   //         } rectFData;
   //     };

};


class Q_GUI_EXPORT QPainterState : public QPaintEngineState
{
 public:
   QPainterState();
   QPainterState(const QPainterState *s);
   virtual ~QPainterState();

   void init(QPainter *p);

   QPointF brushOrigin;
   QFont font;
   QFont deviceFont;
   QPen pen;
   QBrush brush;
   QBrush bgBrush;                       // background brush
   QRegion clipRegion;
   QPainterPath clipPath;
   Qt::ClipOperation clipOperation;
   QPainter::RenderHints renderHints;
   QList<QPainterClipInfo> clipInfo;     // ### Make me smaller and faster to copy around...
   QTransform worldMatrix;               // World transformation matrix, not window and viewport
   QTransform matrix;                    // Complete transformation matrix,
   QTransform redirectionMatrix;
   int wx, wy, ww, wh;                   // window rectangle
   int vx, vy, vw, vh;                   // viewport rectangle
   qreal opacity;

   uint WxF: 1;                          // World transformation
   uint VxF: 1;                          // View transformation
   uint clipEnabled: 1;

   Qt::BGMode bgMode;
   QPainter *painter;
   Qt::LayoutDirection layoutDirection;
   QPainter::CompositionMode composition_mode;
   uint emulationSpecifier;
   uint changeFlags;
};

struct QPainterDummyState {
   QFont font;
   QPen pen;
   QBrush brush;
   QTransform transform;
};


class QPainterPrivate
{
   Q_DECLARE_PUBLIC(QPainter)

 public:
   QPainterPrivate(QPainter *painter)
      : q_ptr(painter), d_ptrs(0), state(0), dummyState(0), txinv(0), inDestructor(false), d_ptrs_size(0),
        refcount(1), device(0), original_device(0), helper_device(0), engine(0), emulationEngine(0),
        extended(0) {
   }

   ~QPainterPrivate();

   QPainter *q_ptr;
   QPainterPrivate **d_ptrs;

   QPainterState *state;
   QVector<QPainterState *> states;

   mutable QPainterDummyState *dummyState;

   QTransform invMatrix;
   uint txinv: 1;
   uint inDestructor : 1;
   uint d_ptrs_size;
   uint refcount;

   enum DrawOperation { StrokeDraw        = 0x1,
      FillDraw          = 0x2,
      StrokeAndFillDraw = 0x3
   };

   QPainterDummyState *fakeState() const {
      if (!dummyState) {
         dummyState = new QPainterDummyState();
      }
      return dummyState;
   }

   void updateEmulationSpecifier(QPainterState *s);
   void updateStateImpl(QPainterState *state);
   void updateState(QPainterState *state);

   void draw_helper(const QPainterPath &path, DrawOperation operation = StrokeAndFillDraw);
   void drawStretchedGradient(const QPainterPath &path, DrawOperation operation);
   void drawOpaqueBackground(const QPainterPath &path, DrawOperation operation);
   void drawTextItem(const QPointF &p, const QTextItem &_ti, QTextEngine *textEngine);

   void drawGlyphs(const quint32 *glyphArray, QFixedPoint *positionArray, int glyphCount,
      QFontEngine *fontEngine, bool overline = false, bool underline = false, bool strikeOut = false);

   void updateMatrix();
   void updateInvMatrix();
   void checkEmulation();

   static QPainterPrivate *get(QPainter *painter) {
      return painter->d_ptr.data();
   }

   QTransform viewTransform() const;
   qreal effectiveDevicePixelRatio() const;
   QTransform hidpiScaleTransform() const;

   static bool attachPainterPrivate(QPainter *q, QPaintDevice *pdev);
   void detachPainterPrivate(QPainter *q);

   QPaintDevice *device;
   QPaintDevice *original_device;
   QPaintDevice *helper_device;
   QPaintEngine *engine;
   QEmulationPaintEngine *emulationEngine;
   QPaintEngineEx *extended;
   QBrush colorBrush;          // for fill with solid color
};

Q_GUI_EXPORT void qt_draw_helper(QPainterPrivate *p, const QPainterPath &path, QPainterPrivate::DrawOperation operation);

QString qt_generate_brush_key(const QBrush &brush);

inline bool qt_pen_is_cosmetic(const QPen &pen, QPainter::RenderHints hints)
{
   (void) hints;
   return pen.isCosmetic() || (const_cast<QPen &>(pen).data_ptr()->defaultWidth);
}

#endif
