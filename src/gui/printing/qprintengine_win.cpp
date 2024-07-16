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

#ifndef QT_NO_PRINTER

#include <qprinter_p.h>
#include <qprintengine_win_p.h>

#include <qplatform_printplugin.h>
#include <qplatform_printersupport.h>
#include <qplatform_pixmap.h>
#include <qt_windows.h>

#include <qbitmap.h>
#include <qdebug.h>
#include <qvector.h>
#include <qpicture.h>

#include <qfont_p.h>
#include <qfontengine_p.h>
#include <qpainter_p.h>
#include <qpicture_p.h>
#include <qpixmap_raster_p.h>

#include <limits.h>

Q_GUI_EXPORT HBITMAP qt_pixmapToWinHBITMAP(const QPixmap &p, int hbitmapFormat = 0);
extern QPainterPath qt_regionToPath(const QRegion &region);
extern QMarginsF qt_convertMargins(const QMarginsF &margins, QPageLayout::Unit fromUnits, QPageLayout::Unit toUnits);

static void draw_text_item_win(const QPointF &_pos, const QTextItemInt &ti, HDC hdc,
      const QTransform &xform, const QPointF &topLeft);

enum HBitmapFormat {
   HBitmapNoAlpha,
   HBitmapPremultipliedAlpha,
   HBitmapAlpha
};

QWin32PrintEngine::QWin32PrintEngine(QPrinter::PrinterMode mode)
   : QAlphaPaintEngine(*(new QWin32PrintEnginePrivate),
     PaintEngineFeatures(PrimitiveTransform
           | PixmapTransform
           | PerspectiveTransform
           | PainterPaths
           | Antialiasing
           | PaintOutsidePaintEvent))
{
   Q_D(QWin32PrintEngine);
   d->mode = mode;
   QPlatformPrinterSupport *ps = QPlatformPrinterSupportPlugin::get();

   if (ps) {
      d->m_printDevice = ps->createDefaultPrintDevice();
   }

   d->m_pageLayout.setPageSize(d->m_printDevice.defaultPageSize());
   d->initialize();
}

bool QWin32PrintEngine::begin(QPaintDevice *pdev)
{
   Q_D(QWin32PrintEngine);

   QAlphaPaintEngine::begin(pdev);
   if (! continueCall()) {
      return true;
   }

   if (d->reinit) {
      d->resetDC();
      d->reinit = false;
   }

   // set default colors, etc
   bool ok = d->state == QPrinter::Idle;

   if (! d->hdc) {
      return false;
   }

   d->devMode->dmCopies = d->num_copies;

   DOCINFO di;
   memset(&di, 0, sizeof(DOCINFO));

   di.cbSize = sizeof(DOCINFO);

   std::wstring tmpDoc(d->docName.toStdWString());

   if (d->docName.isEmpty()) {
      tmpDoc = L"document1";
   }

   di.lpszDocName = &tmpDoc[0];

   std::wstring tmpFile;

   if (d->printToFile && ! d->fileName.isEmpty()) {
      tmpFile = d->fileName.toStdWString();
   }

   if (d->printToFile && d->fileName.isEmpty()) {
      tmpFile = L"FILE:";
   }

   di.lpszOutput = &tmpFile[0];

   if (ok && StartDoc(d->hdc, &di) == SP_ERROR) {
      qErrnoWarning("QWin32PrintEngine::begin: StartDoc failed");
      ok = false;
   }

   if (StartPage(d->hdc) <= 0) {
      qErrnoWarning("QWin32PrintEngine::begin: StartPage failed");
      ok = false;
   }

   if (!ok) {
      d->state = QPrinter::Idle;
   } else {
      d->state = QPrinter::Active;
   }

   d->matrix    = QTransform();
   d->has_pen   = true;
   d->pen       = QColor(Qt::black);
   d->has_brush = false;

   d->complex_xform = false;

   updateMatrix(d->matrix);

   if (! ok) {
      cleanUp();
   }

   return ok;
}

bool QWin32PrintEngine::end()
{
   Q_D(QWin32PrintEngine);

   if (d->hdc) {
      if (d->state == QPrinter::Aborted) {
         cleanUp();
         AbortDoc(d->hdc);
         return true;
      }
   }

   QAlphaPaintEngine::end();
   if (!continueCall()) {
      return true;
   }

   if (d->hdc) {
      EndPage(d->hdc);                 // printing done
      EndDoc(d->hdc);
   }

   d->state = QPrinter::Idle;
   d->reinit = true;
   return true;
}

bool QWin32PrintEngine::newPage()
{
   Q_D(QWin32PrintEngine);
   Q_ASSERT(isActive());

   Q_ASSERT(d->hdc);

   flushAndInit();

   bool transparent = GetBkMode(d->hdc) == TRANSPARENT;

   if (EndPage(d->hdc) <= 0) {
      qErrnoWarning("QWin32PrintEngine::newPage: EndPage failed");
      return false;
   }

   if (d->reinit) {
      if (! d->resetDC()) {
         qErrnoWarning("QWin32PrintEngine::newPage: ResetDC failed");
         return false;
      }
      d->reinit = false;
   }

   if (StartPage(d->hdc) <= 0) {
      qErrnoWarning("Win32PrintEngine::newPage: StartPage failed");
      return false;
   }

   SetTextAlign(d->hdc, TA_BASELINE);
   if (transparent) {
      SetBkMode(d->hdc, TRANSPARENT);
   }

   return true;
}

bool QWin32PrintEngine::abort()
{
   // do nothing loop.
   return false;
}

void QWin32PrintEngine::drawTextItem(const QPointF &p, const QTextItem &textItem)
{
   Q_D(const QWin32PrintEngine);

   QAlphaPaintEngine::drawTextItem(p, textItem);
   if (!continueCall()) {
      return;
   }

   const QTextItemInt &ti = static_cast<const QTextItemInt &>(textItem);

   QRgb brushColor = state->pen().brush().color().rgb();
   bool fallBack = state->pen().brush().style() != Qt::SolidPattern
      || qAlpha(brushColor) != 0xff
      || d->txop >= QTransform::TxProject
      || ti.fontEngine->type() != QFontEngine::Win
      || ! d->embed_fonts;

   if (! fallBack) {
        const QVariantMap userData = ti.fontEngine->userData().toMap();

      const QVariant hFontV   = userData.value("hFont");
      const QVariant logFontV = userData.value("logFont");

      if (hFontV.canConvert<HFONT>() && logFontV.canConvert<LOGFONT>()) {
         const HFONT hfont = hFontV.value<HFONT>();
         const LOGFONT logFont = logFontV.value<LOGFONT>();

         // Try selecting the font to see if we get a substitution font
         SelectObject(d->hdc, hfont);

         if (GetDeviceCaps(d->hdc, TECHNOLOGY) != DT_CHARSTREAM) {
            std::wstring tmp(64, L'\0');
            GetTextFace(d->hdc, 64, &tmp[0]);

            fallBack = QString::fromStdWString(tmp) != QString::fromStdWString(std::wstring(logFont.lfFaceName));
         }
      }
   }

   if (fallBack) {
      QPaintEngine::drawTextItem(p, textItem);
      return;
   }

   COLORREF cf = RGB(qRed(brushColor), qGreen(brushColor), qBlue(brushColor));
   SelectObject(d->hdc, CreateSolidBrush(cf));
   SelectObject(d->hdc, CreatePen(PS_SOLID, 1, cf));
   SetTextColor(d->hdc, cf);

   draw_text_item_win(p, ti, d->hdc, d->matrix, QPointF(0.0, 0.0));
   DeleteObject(SelectObject(d->hdc, GetStockObject(HOLLOW_BRUSH)));
   DeleteObject(SelectObject(d->hdc, GetStockObject(BLACK_PEN)));
}

int QWin32PrintEngine::metric(QPaintDevice::PaintDeviceMetric m) const
{
   Q_D(const QWin32PrintEngine);

   if (!d->hdc) {
      return 0;
   }

   int val;
   int res = d->resolution;

   switch (m) {
      case QPaintDevice::PdmWidth:

         val = d->m_paintRectPixels.width();

         break;

      case QPaintDevice::PdmHeight:
         val = d->m_paintRectPixels.height();
         break;

      case QPaintDevice::PdmDpiX:
         val = res;
         break;

      case QPaintDevice::PdmDpiY:
         val = res;
         break;

      case QPaintDevice::PdmPhysicalDpiX:
         val = GetDeviceCaps(d->hdc, LOGPIXELSX);
         break;

      case QPaintDevice::PdmPhysicalDpiY:
         val = GetDeviceCaps(d->hdc, LOGPIXELSY);
         break;

      case QPaintDevice::PdmWidthMM:

         val = d->m_paintSizeMM.width();
         break;

      case QPaintDevice::PdmHeightMM:
         val = d->m_paintSizeMM.height();
         break;

      case QPaintDevice::PdmNumColors: {
         int bpp = GetDeviceCaps(d->hdc, BITSPIXEL);
         if (bpp == 32) {
            val = INT_MAX;
         } else if (bpp <= 8) {
            val = GetDeviceCaps(d->hdc, NUMCOLORS);
         } else {
            val = 1 << (bpp * GetDeviceCaps(d->hdc, PLANES));
         }
      }
      break;

      case QPaintDevice::PdmDepth:
         val = GetDeviceCaps(d->hdc, PLANES);
         break;

      case QPaintDevice::PdmDevicePixelRatio:
         val = 1;
         break;

      case QPaintDevice::PdmDevicePixelRatioScaled:
         val = 1 * QPaintDevice::devicePixelRatioFScale();
         break;

      default:
         qWarning("QPrinter::metric() Invalid metric command");
         return 0;
   }

   return val;
}

void QWin32PrintEngine::updateState(const QPaintEngineState & state)
{
   Q_D(QWin32PrintEngine);

   QAlphaPaintEngine::updateState(state);
   if (!continueCall()) {
      return;
   }

   if (state.state() & DirtyTransform) {
      updateMatrix(state.transform());
   }

   if (state.state() & DirtyPen) {
      d->pen = state.pen();
      d->has_pen = d->pen.style() != Qt::NoPen && d->pen.isSolid();
   }

   if (state.state() & DirtyBrush) {
      QBrush brush = state.brush();
      d->has_brush = brush.style() == Qt::SolidPattern;
      d->brush_color = brush.color();
   }

   if (state.state() & DirtyClipEnabled) {
      if (state.isClipEnabled()) {
         updateClipPath(painter()->clipPath(), Qt::ReplaceClip);
      } else {
         updateClipPath(QPainterPath(), Qt::NoClip);
      }
   }

   if (state.state() & DirtyClipPath) {
      updateClipPath(state.clipPath(), state.clipOperation());
   }

   if (state.state() & DirtyClipRegion) {
      QRegion clipRegion = state.clipRegion();
      QPainterPath clipPath = qt_regionToPath(clipRegion);
      updateClipPath(clipPath, state.clipOperation());
   }
}

void QWin32PrintEngine::updateClipPath(const QPainterPath & clipPath, Qt::ClipOperation op)
{
   Q_D(QWin32PrintEngine);

   bool doclip = true;
   if (op == Qt::NoClip) {
      SelectClipRgn(d->hdc, nullptr);
      doclip = false;
   }

   if (doclip) {
      QPainterPath xformed = clipPath * d->matrix;

      if (xformed.isEmpty()) {

         HRGN empty = CreateRectRgn(-0x1000000, -0x1000000, -0x0fffffff, -0x0ffffff);
         SelectClipRgn(d->hdc, empty);
         DeleteObject(empty);
      } else {
         d->composeGdiPath(xformed);
         const int ops[] = {
            -1,         // Qt::NoClip, covered above
            RGN_COPY,   // Qt::ReplaceClip
            RGN_AND,    // Qt::IntersectClip
            RGN_OR      // Qt::UniteClip
            };
         Q_ASSERT(op > 0 && unsigned(op) <= sizeof(ops) / sizeof(int));
         SelectClipPath(d->hdc, ops[op]);
      }
   }

   QPainterPath aclip = qt_regionToPath(alphaClipping());
   if (!aclip.isEmpty()) {
      QTransform tx(d->stretch_x, 0, 0, d->stretch_y, d->origin_x, d->origin_y);
      d->composeGdiPath(tx.map(aclip));
      SelectClipPath(d->hdc, RGN_DIFF);
   }
}

void QWin32PrintEngine::updateMatrix(const QTransform & m) {
   Q_D(QWin32PrintEngine);

   QTransform stretch(d->stretch_x, 0, 0, d->stretch_y, d->origin_x, d->origin_y);
   d->painterMatrix = m;
   d->matrix = d->painterMatrix * stretch;
   d->txop = d->matrix.type();
   d->complex_xform = (d->txop > QTransform::TxScale);
}

void QWin32PrintEngine::drawPixmap(const QRectF & targetRect, const QPixmap & originalPixmap, const QRectF & sourceRect) {
   Q_D(QWin32PrintEngine);

   QAlphaPaintEngine::drawPixmap(targetRect, originalPixmap, sourceRect);
   if (!continueCall()) {
      return;
   }

   const int tileSize = 2048;

   QRectF r = targetRect;
   QRectF sr = sourceRect;

   QPixmap pixmap = originalPixmap;
   if (sr.size() != pixmap.size()) {
      pixmap = pixmap.copy(sr.toRect());
   }

   qreal scaleX = 1.0f;
   qreal scaleY = 1.0f;

   QTransform scaleMatrix = QTransform::fromScale(r.width() / pixmap.width(), r.height() / pixmap.height());
   QTransform adapted = QPixmap::trueMatrix(d->painterMatrix * scaleMatrix,
         pixmap.width(), pixmap.height());

   qreal xform_offset_x = adapted.dx();
   qreal xform_offset_y = adapted.dy();

   if (d->complex_xform) {
      pixmap = pixmap.transformed(adapted);
      scaleX = d->stretch_x;
      scaleY = d->stretch_y;
   } else {
      scaleX = d->stretch_x * (r.width() / pixmap.width()) * d->painterMatrix.m11();
      scaleY = d->stretch_y * (r.height() / pixmap.height()) * d->painterMatrix.m22();
   }

   QPointF topLeft = r.topLeft() * d->painterMatrix;
   int tx = int(topLeft.x() * d->stretch_x + d->origin_x);
   int ty = int(topLeft.y() * d->stretch_y + d->origin_y);
   int tw = qAbs(int(pixmap.width() * scaleX));
   int th = qAbs(int(pixmap.height() * scaleY));

   xform_offset_x *= d->stretch_x;
   xform_offset_y *= d->stretch_y;

   int dc_state = SaveDC(d->hdc);

   int tilesw = pixmap.width() / tileSize;
   int tilesh = pixmap.height() / tileSize;
   ++tilesw;
   ++tilesh;

   int txinc = tileSize * scaleX;
   int tyinc = tileSize * scaleY;

   for (int y = 0; y < tilesh; ++y) {
      int tposy = ty + (y * tyinc);
      int imgh = tileSize;
      int height = tyinc;
      if (y == (tilesh - 1)) {
         imgh = pixmap.height() - (y * tileSize);
         height = (th - (y * tyinc));
      }
      for (int x = 0; x < tilesw; ++x) {
         int tposx = tx + (x * txinc);
         int imgw = tileSize;
         int width = txinc;
         if (x == (tilesw - 1)) {
            imgw = pixmap.width() - (x * tileSize);
            width = (tw - (x * txinc));
         }

         QImage img(QSize(imgw, imgh), QImage::Format_RGB32);
         img.fill(Qt::white);
         QPainter painter(&img);
         painter.drawPixmap(0, 0, pixmap, tileSize * x, tileSize * y, imgw, imgh);
         QPixmap p = QPixmap::fromImage(img);
         HBITMAP hbitmap = qt_pixmapToWinHBITMAP(p, HBitmapNoAlpha);
         HDC display_dc = GetDC(nullptr);
         HDC hbitmap_hdc = CreateCompatibleDC(display_dc);
         HGDIOBJ null_bitmap = SelectObject(hbitmap_hdc, hbitmap);

         ReleaseDC(nullptr, display_dc);

         if (!StretchBlt(d->hdc, qRound(tposx - xform_offset_x), qRound(tposy - xform_offset_y), width, height,
               hbitmap_hdc, 0, 0, p.width(), p.height(), SRCCOPY)) {
            qErrnoWarning("QWin32PrintEngine::drawPixmap, StretchBlt failed");
         }

         SelectObject(hbitmap_hdc, null_bitmap);
         DeleteObject(hbitmap);
         DeleteDC(hbitmap_hdc);
      }
   }

   RestoreDC(d->hdc, dc_state);
}

void QWin32PrintEngine::drawTiledPixmap(const QRectF & r, const QPixmap & pm, const QPointF & pos)
{
   Q_D(QWin32PrintEngine);

   QAlphaPaintEngine::drawTiledPixmap(r, pm, pos);
   if (!continueCall()) {
      return;
   }

   if (d->complex_xform || !pos.isNull()) {
      QPaintEngine::drawTiledPixmap(r, pm, pos);
   } else {
      int dc_state = SaveDC(d->hdc);

      HDC display_dc = GetDC(nullptr);
      HBITMAP hbitmap = qt_pixmapToWinHBITMAP(pm, HBitmapNoAlpha);
      HDC hbitmap_hdc = CreateCompatibleDC(display_dc);
      HGDIOBJ null_bitmap = SelectObject(hbitmap_hdc, hbitmap);

      ReleaseDC(nullptr, display_dc);

      QRectF trect = d->painterMatrix.mapRect(r);
      int tx = int(trect.left() * d->stretch_x + d->origin_x);
      int ty = int(trect.top() * d->stretch_y + d->origin_y);

      int xtiles = int(trect.width() / pm.width()) + 1;
      int ytiles = int(trect.height() / pm.height()) + 1;
      int xinc = int(pm.width() * d->stretch_x);
      int yinc = int(pm.height() * d->stretch_y);

      for (int y = 0; y < ytiles; ++y) {
         int ity = ty + (yinc * y);
         int ith = pm.height();
         if (y == (ytiles - 1)) {
            ith = int(trect.height() - (pm.height() * y));
         }

         for (int x = 0; x < xtiles; ++x) {
            int itx = tx + (xinc * x);
            int itw = pm.width();
            if (x == (xtiles - 1)) {
               itw = int(trect.width() - (pm.width() * x));
            }

            if (!StretchBlt(d->hdc, itx, ity, int(itw * d->stretch_x), int(ith * d->stretch_y),
                  hbitmap_hdc, 0, 0, itw, ith, SRCCOPY)) {
               qErrnoWarning("QWin32PrintEngine::drawPixmap, StretchBlt failed");
            }

         }
      }

      SelectObject(hbitmap_hdc, null_bitmap);
      DeleteObject(hbitmap);
      DeleteDC(hbitmap_hdc);

      RestoreDC(d->hdc, dc_state);
   }
}

void QWin32PrintEnginePrivate::composeGdiPath(const QPainterPath & path)
{
   if (!BeginPath(hdc)) {
      qErrnoWarning("QWin32PrintEnginePrivate::drawPath: BeginPath failed");
   }

   // Drawing the subpaths
   int start = -1;
   for (int i = 0; i < path.elementCount(); ++i) {
      const QPainterPath::Element &elm = path.elementAt(i);
      switch (elm.type) {
         case QPainterPath::MoveToElement:
            if (start >= 0
               && path.elementAt(start).x == path.elementAt(i - 1).x
               && path.elementAt(start).y == path.elementAt(i - 1).y) {
               CloseFigure(hdc);
            }
            start = i;
            MoveToEx(hdc, qRound(elm.x), qRound(elm.y), nullptr);
            break;
         case QPainterPath::LineToElement:
            LineTo(hdc, qRound(elm.x), qRound(elm.y));
            break;
         case QPainterPath::CurveToElement: {
            POINT pts[3] = {
               { qRound(elm.x), qRound(elm.y) },
               { qRound(path.elementAt(i + 1).x), qRound(path.elementAt(i + 1).y) },
               { qRound(path.elementAt(i + 2).x), qRound(path.elementAt(i + 2).y) }
            };
            i += 2;
            PolyBezierTo(hdc, pts, 3);
            break;
         }
         default:
            qFatal("QWin32PaintEngine::drawPath: Unhandled type: %d", elm.type);
      }
   }

   if (start >= 0 && path.elementAt(start).x == path.elementAt(path.elementCount() - 1).x
         && path.elementAt(start).y == path.elementAt(path.elementCount() - 1).y) {
      CloseFigure(hdc);
   }

   if (!EndPath(hdc)) {
      qErrnoWarning("QWin32PaintEngine::drawPath: EndPath failed");
   }

   SetPolyFillMode(hdc, path.fillRule() == Qt::WindingFill ? WINDING : ALTERNATE);
}

void QWin32PrintEnginePrivate::fillPath_dev(const QPainterPath & path, const QColor & color)
{
   composeGdiPath(path);

   HBRUSH brush = CreateSolidBrush(RGB(color.red(), color.green(), color.blue()));
   HGDIOBJ old_brush = SelectObject(hdc, brush);
   FillPath(hdc);
   DeleteObject(SelectObject(hdc, old_brush));
}

void QWin32PrintEnginePrivate::strokePath_dev(const QPainterPath & path, const QColor & color, qreal penWidth)
{
   composeGdiPath(path);
   LOGBRUSH brush;
   brush.lbStyle = BS_SOLID;
   brush.lbColor = RGB(color.red(), color.green(), color.blue());
   DWORD capStyle = PS_ENDCAP_SQUARE;
   DWORD joinStyle = PS_JOIN_BEVEL;
   if (pen.capStyle() == Qt::FlatCap) {
      capStyle = PS_ENDCAP_FLAT;
   } else if (pen.capStyle() == Qt::RoundCap) {
      capStyle = PS_ENDCAP_ROUND;
   }

   if (pen.joinStyle() == Qt::MiterJoin) {
      joinStyle = PS_JOIN_MITER;
   } else if (pen.joinStyle() == Qt::RoundJoin) {
      joinStyle = PS_JOIN_ROUND;
   }

   HPEN pen = ExtCreatePen(PS_GEOMETRIC | PS_SOLID | capStyle | joinStyle,
         (penWidth == 0) ? 1 : penWidth, &brush, 0, nullptr);

   HGDIOBJ old_pen = SelectObject(hdc, pen);
   StrokePath(hdc);
   DeleteObject(SelectObject(hdc, old_pen));
}

void QWin32PrintEnginePrivate::fillPath(const QPainterPath & path, const QColor & color)
{
   fillPath_dev(path * matrix, color);
}

void QWin32PrintEnginePrivate::strokePath(const QPainterPath & path, const QColor & color)
{
   Q_Q(QWin32PrintEngine);
   QPainterPathStroker stroker;

   if (pen.style() == Qt::CustomDashLine) {
      stroker.setDashPattern(pen.dashPattern());
      stroker.setDashOffset(pen.dashOffset());
   } else {
      stroker.setDashPattern(pen.style());
   }
   stroker.setCapStyle(pen.capStyle());
   stroker.setJoinStyle(pen.joinStyle());
   stroker.setMiterLimit(pen.miterLimit());

   QPainterPath stroke;
   qreal width = pen.widthF();
   bool cosmetic = qt_pen_is_cosmetic(pen, q->state->renderHints());
   if (pen.style() == Qt::SolidLine && (cosmetic || matrix.type() < QTransform::TxScale)) {
      strokePath_dev(path * matrix, color, width);
   } else {
      stroker.setWidth(width);
      if (cosmetic) {
         stroke = stroker.createStroke(path * matrix);
      } else {
         stroke = stroker.createStroke(path) * painterMatrix;
         QTransform stretch(stretch_x, 0, 0, stretch_y, origin_x, origin_y);
         stroke = stroke * stretch;
      }

      if (stroke.isEmpty()) {
         return;
      }

      fillPath_dev(stroke, color);
   }
}

void QWin32PrintEngine::drawPath(const QPainterPath & path)
{
   Q_D(QWin32PrintEngine);

   QAlphaPaintEngine::drawPath(path);
   if (!continueCall()) {
      return;
   }

   if (d->has_brush) {
      d->fillPath(path, d->brush_color);
   }

   if (d->has_pen) {
      d->strokePath(path, d->pen.color());
   }
}

void QWin32PrintEngine::drawPolygon(const QPointF * points, int pointCount, PolygonDrawMode mode)
{
   QAlphaPaintEngine::drawPolygon(points, pointCount, mode);

   if (!continueCall()) {
      return;
   }

   Q_ASSERT(pointCount > 1);

   QPainterPath path(points[0]);

   for (int i = 1; i < pointCount; ++i) {
      path.lineTo(points[i]);
   }

   Q_D(QWin32PrintEngine);

   bool has_brush = d->has_brush;

   if (mode == PolylineMode) {
      d->has_brush = false;   // No brush for polylines
   } else {
      path.closeSubpath();   // polygons are should always be closed.
   }

   drawPath(path);
   d->has_brush = has_brush;
}

QWin32PrintEnginePrivate::~QWin32PrintEnginePrivate()
{
   release();
}

void QWin32PrintEnginePrivate::initialize()
{
   release();

   Q_ASSERT(!hPrinter);
   Q_ASSERT(!hdc);
   Q_ASSERT(!devMode);
   Q_ASSERT(!pInfo);

   if (!m_printDevice.isValid()) {
      return;
   }

   txop = QTransform::TxNone;

   QString printerName = m_printDevice.id();
   bool ok = OpenPrinter(&printerName.toStdWString()[0], (LPHANDLE)&hPrinter, nullptr);

   if (! ok) {
      qErrnoWarning("QWin32PrintEngine::initialize: OpenPrinter failed");
      return;
   }

   // Fetch the PRINTER_INFO_2 with DEVMODE data containing the printer settings.
   DWORD infoSize, numBytes;
   GetPrinter(hPrinter, 2, nullptr, 0, &infoSize);

   hMem  = GlobalAlloc(GHND, infoSize);
   pInfo = (PRINTER_INFO_2 *) GlobalLock(hMem);
   ok = GetPrinter(hPrinter, 2, (LPBYTE)pInfo, infoSize, &numBytes);

   if (! ok) {
      qErrnoWarning("QWin32PrintEngine::initialize: GetPrinter failed");
      release();

      return;
   }

   devMode = pInfo->pDevMode;

   std::wstring tmpName(printerName.toStdWString());

   if (! devMode) {
      // Allocate the required buffer
      LONG result = DocumentProperties(nullptr, hPrinter, &tmpName[0], nullptr, nullptr, 0);
      devMode = (DEVMODE *) malloc(result);
      ownsDevMode = true;

      // Get the default DevMode
      result = DocumentProperties(nullptr, hPrinter, &tmpName[0], devMode, nullptr, DM_OUT_BUFFER);
      if (result != IDOK) {
         qErrnoWarning("QWin32PrintEngine::initialize: Failed to obtain devMode");

         free(devMode);
         devMode = nullptr;
         ownsDevMode = false;
      }
   }

   hdc = CreateDC(nullptr, &tmpName[0], nullptr, devMode);

   if (! hdc) {
      qErrnoWarning("QWin32PrintEngine::initialize: CreateDC failed");
      release();
      return;
   }

   Q_ASSERT(hPrinter);
   Q_ASSERT(pInfo);

   initHDC();
   if (devMode) {
      num_copies = devMode->dmCopies;
      devMode->dmCollate = DMCOLLATE_TRUE;
      updatePageLayout();
   }
}

void QWin32PrintEnginePrivate::initHDC()
{
   Q_ASSERT(hdc);

   HDC display_dc = GetDC(nullptr);
   dpi_x = GetDeviceCaps(hdc, LOGPIXELSX);
   dpi_y = GetDeviceCaps(hdc, LOGPIXELSY);
   dpi_display = GetDeviceCaps(display_dc, LOGPIXELSY);
   ReleaseDC(nullptr, display_dc);

   if (dpi_display == 0) {
      qWarning("QWin32PrintEngine::metric() GetDeviceCaps() failed, this might be a driver problem");
      dpi_display = 96; // Reasonable default
   }

   switch (mode) {
      case QPrinter::ScreenResolution:
         resolution = dpi_display;
         stretch_x = dpi_x / double(dpi_display);
         stretch_y = dpi_y / double(dpi_display);
         break;
      case QPrinter::PrinterResolution:
      case QPrinter::HighResolution:
         resolution = dpi_y;
         stretch_x = 1;
         stretch_y = 1;
         break;
      default:
         break;
   }

   updateMetrics();
}

void QWin32PrintEnginePrivate::release()
{
   if (globalDevMode) {
      // Devmode comes from print dialog
      GlobalUnlock(globalDevMode);

   } else if (hMem) {
      // devMode is a part of the same memory block as pInfo so one free is enough...
      GlobalUnlock(hMem);
      GlobalFree(hMem);
   }

   if (hPrinter) {
      ClosePrinter(hPrinter);
   }

   if (hdc) {
      DeleteDC(hdc);
   }

   // Check if devMode was allocated separately from pInfo / hMem.
   if (ownsDevMode) {
      free(devMode);
   }

   hdc      = nullptr;
   hPrinter = nullptr;
   pInfo    = nullptr;
   hMem     = nullptr;
   devMode  = nullptr;

   ownsDevMode = false;
}

void QWin32PrintEnginePrivate::doReinit()
{
   if (state == QPrinter::Active) {
      reinit = true;

   } else {
      resetDC();
      reinit = false;
   }
}

bool QWin32PrintEnginePrivate::resetDC()
{
   if (! hdc) {
      qWarning() << "resetDC() was called with an invalid hdc";
      return false;
   }

   const HDC oldHdc = hdc;
   const HDC hdc = ResetDC(oldHdc, devMode);

   if (!hdc) {
      const int lastError = GetLastError();
      qErrnoWarning(lastError, "ResetDC() on %p failed (%d)", oldHdc, lastError);
   }
   return hdc != nullptr;
}

static int indexOfId(const QList<QPrint::InputSlot> &inputSlots, QPrint::InputSlotId id)
{
   for (int i = 0; i < inputSlots.size(); ++i) {
      if (inputSlots.at(i).id == id) {
         return i;
      }
   }

   return -1;
}

static int indexOfWindowsId(const QList<QPrint::InputSlot> &inputSlots, int windowsId)
{
   for (int i = 0; i < inputSlots.size(); ++i) {
      if (inputSlots.at(i).windowsId == windowsId) {
         return i;
      }
   }

   return -1;
}

void QWin32PrintEngine::setProperty(PrintEnginePropertyKey key, const QVariant & value)
{
   Q_D(QWin32PrintEngine);

   switch (key) {
      case PPK_PageRect:
         break;
      case PPK_PaperRect:
         break;
      case PPK_PaperSources:
         break;
      case PPK_SupportsMultipleCopies:
         break;
      case PPK_SupportedResolutions:
         break;
      case PPK_CustomBase:
         break;
      case PPK_PageOrder:
         break;
      case PPK_PrinterProgram:
         break;
      case PPK_SelectionOption:
         break;

      // The following keys are properties and settings that are supported by the Windows PrintEngine
      case PPK_FontEmbedding:
         d->embed_fonts = value.toBool();
         break;

      case PPK_CollateCopies: {
         if (!d->devMode) {
            break;
         }
         d->devMode->dmCollate = value.toBool() ? DMCOLLATE_TRUE : DMCOLLATE_FALSE;
         d->doReinit();
      }
      break;

      case PPK_ColorMode: {
         if (!d->devMode) {
            break;
         }
         d->devMode->dmColor = (value.toInt() == QPrinter::Color) ? DMCOLOR_COLOR : DMCOLOR_MONOCHROME;
         d->doReinit();
      }
      break;

      case PPK_Creator:
         d->m_creator = value.toString();
         break;

      case PPK_DocumentName:
         if (isActive()) {
            qWarning("QWin32PrintEngine::setProperty() Unable to change document name while printing is active");
            return;
         }
         d->docName = value.toString();
         break;

      case PPK_Duplex: {
         if (!d->devMode) {
            break;
         }

         QPrint::DuplexMode mode = QPrint::DuplexMode(value.toInt());
         if (mode == property(PPK_Duplex).toInt() || !d->m_printDevice.supportedDuplexModes().contains(mode)) {
            break;
         }

         switch (mode) {
            case QPrinter::DuplexNone:
               d->devMode->dmDuplex = DMDUP_SIMPLEX;
               break;
            case QPrinter::DuplexAuto:
               d->devMode->dmDuplex = d->m_pageLayout.orientation() == QPageLayout::Landscape ? DMDUP_HORIZONTAL : DMDUP_VERTICAL;
               break;
            case QPrinter::DuplexLongSide:
               d->devMode->dmDuplex = DMDUP_VERTICAL;
               break;
            case QPrinter::DuplexShortSide:
               d->devMode->dmDuplex = DMDUP_HORIZONTAL;
               break;
            default:
               break;
         }
         d->doReinit();
         break;
      }

      case PPK_FullPage:
         if (value.toBool()) {
            d->m_pageLayout.setMode(QPageLayout::FullPageMode);
         } else {
            d->m_pageLayout.setMode(QPageLayout::StandardMode);
         }
         d->updateMetrics();

         break;

      case PPK_CopyCount:
      case PPK_NumberOfCopies:
         if (!d->devMode) {
            break;
         }
         d->num_copies = value.toInt();
         d->devMode->dmCopies = d->num_copies;
         d->doReinit();
         break;

      case PPK_Orientation: {
         if (!d->devMode) {
            break;
         }

         QPageLayout::Orientation orientation = QPageLayout::Orientation(value.toInt());
         d->devMode->dmOrientation = orientation == QPageLayout::Landscape ? DMORIENT_LANDSCAPE : DMORIENT_PORTRAIT;
         d->m_pageLayout.setOrientation(orientation);
         d->updateMetrics();

         d->doReinit();

         break;
      }

      case PPK_OutputFileName:
         if (isActive()) {
            qWarning("QWin32PrintEngine::setProperty() Unable to change filename while printing");
         } else {
            d->fileName = value.toString();
            d->printToFile = !value.toString().isEmpty();
         }
         break;

      case PPK_PageSize: {
         if (!d->devMode) {
            break;
         }
         const QPageSize pageSize = QPageSize(QPageSize::PageSizeId(value.toInt()));
         if (pageSize.isValid()) {
            d->setPageSize(pageSize);
            d->doReinit();
         }
         break;
      }

      case PPK_PaperName: {
         if (!d->devMode) {
            break;
         }
         // Get the named page size from the printer if supported
         const QPageSize pageSize = d->m_printDevice.supportedPageSize(value.toString());
         if (pageSize.isValid()) {
            d->setPageSize(pageSize);
            d->doReinit();

         }
         break;
      }

      case PPK_PaperSource: {
         if (!d->devMode) {
            break;
         }
         const QList<QPrint::InputSlot> inputSlots = d->m_printDevice.supportedInputSlots();
         const int paperSource = value.toInt();
         const int index = paperSource >= DMBIN_USER ?
            indexOfWindowsId(inputSlots, paperSource) : indexOfId(inputSlots, QPrint::InputSlotId(paperSource));
         d->devMode->dmDefaultSource = index >= 0 ? inputSlots.at(index).windowsId : DMBIN_AUTO;
         d->doReinit();

         break;
      }

      case PPK_PrinterName: {
         QString id = value.toString();
         QPlatformPrinterSupport *ps = QPlatformPrinterSupportPlugin::get();
         if (!ps) {
            return;
         }
         QPrintDevice printDevice = ps->createPrintDevice(id.isEmpty() ? ps->defaultPrintDeviceId() : id);
         if (printDevice.isValid()) {
            d->m_printDevice = printDevice;
            // TODO Do we need to check if the page size is valid on new printer?
            d->initialize();
         }
         break;
      }

      case PPK_Resolution: {
         d->resolution = value.toInt();

         d->stretch_x = d->dpi_x / double(d->resolution);
         d->stretch_y = d->dpi_y / double(d->resolution);
         d->updateMetrics();

         break;
      }

      case PPK_WindowsPageSize: {
         if (!d->devMode) {
            break;
         }
         const QPageSize pageSize = QPageSize(QPageSize::id(value.toInt()));
         if (pageSize.isValid()) {
            d->setPageSize(pageSize);
            d->doReinit();

            break;
         }


         break;
      }

      case PPK_CustomPaperSize: {
         if (!d->devMode) {
            break;
         }
         const QPageSize pageSize = QPageSize(value.toSizeF(), QPageSize::Unit::Point);
         if (pageSize.isValid()) {
            d->setPageSize(pageSize);
            d->doReinit();

         }
         break;
      }

      case PPK_PageMargins: {
         QList<QVariant> margins(value.toList());
         Q_ASSERT(margins.size() == 4);
         d->m_pageLayout.setUnits(QPageSize::Unit::Point);
         d->m_pageLayout.setMargins(QMarginsF(margins.at(0).toReal(), margins.at(1).toReal(),
               margins.at(2).toReal(), margins.at(3).toReal()));
         d->updateMetrics();

         break;
      }

      case PPK_QPageSize: {
         if (!d->devMode) {
            break;
         }

         // Get the page size from the printer if supported
         const QPageSize pageSize = value.value<QPageSize>();
         if (pageSize.isValid()) {
            d->setPageSize(pageSize);
            d->doReinit();

         }
         break;
      }

      case PPK_QPageMargins: {
         QPair<QMarginsF, QPageLayout::Unit> pair = value.value<QPair<QMarginsF, QPageLayout::Unit>>();
         d->m_pageLayout.setUnits(pair.second);
         d->m_pageLayout.setMargins(pair.first);
         d->updateMetrics();

         break;
      }

      case PPK_QPageLayout: {
         QPageLayout pageLayout = value.value<QPageLayout>();
         if (pageLayout.isValid() && d->m_printDevice.isValidPageLayout(pageLayout, d->resolution)) {
            setProperty(PPK_QPageSize, QVariant::fromValue(pageLayout.pageSize()));
            setProperty(PPK_FullPage, pageLayout.mode() == QPageLayout::FullPageMode);
            setProperty(PPK_Orientation, QVariant::fromValue(pageLayout.orientation()));
            d->m_pageLayout.setUnits(pageLayout.units());
            d->m_pageLayout.setMargins(pageLayout.margins());
            d->updateMetrics();

         }
         break;
      }

      // No default so that compiler will complain if new keys added and not handled in this engine
   }
}

QVariant QWin32PrintEngine::property(PrintEnginePropertyKey key) const
{
   Q_D(const QWin32PrintEngine);

   QVariant value;
   switch (key) {

      // The following keys are settings that are unsupported by the Windows PrintEngine
      // Return sensible default values to ensure consistent behavior across platforms
      case PPK_PageOrder:
         value = QPrinter::FirstPageFirst;
         break;
      case PPK_PrinterProgram:
         value = QString();
         break;
      case PPK_SelectionOption:
         value = QString();
         break;
         // The following keys are properties and settings that are supported by the Windows PrintEngine
      case PPK_FontEmbedding:
         value = d->embed_fonts;
         break;
      case PPK_CollateCopies:
         if (!d->devMode) {
            value = false;
         } else {
            value = d->devMode->dmCollate == DMCOLLATE_TRUE;
         }
         break;

      case PPK_ColorMode: {
         if (!d->devMode) {
            value = QPrinter::Color;
         } else {
            value = (d->devMode->dmColor == DMCOLOR_COLOR) ? QPrinter::Color : QPrinter::GrayScale;
         }
      }
         break;

      case PPK_Creator:
         value = d->m_creator;
         break;
      case PPK_DocumentName:
         value = d->docName;
         break;

      case PPK_Duplex: {
         if (!d->devMode) {
            value = QPrinter::DuplexNone;
         } else {
            switch (d->devMode->dmDuplex) {
               case DMDUP_VERTICAL:
                  value = QPrinter::DuplexLongSide;
                  break;
               case DMDUP_HORIZONTAL:
                  value = QPrinter::DuplexShortSide;
                  break;
               case DMDUP_SIMPLEX:
               default:
                  value = QPrinter::DuplexNone;
                  break;
            }
         }
         break;
      }
      case PPK_FullPage:
         value =  d->m_pageLayout.mode() == QPageLayout::FullPageMode;
         break;

      case PPK_CopyCount:
         value = d->num_copies;
         break;

      case PPK_SupportsMultipleCopies:
         value = true;
         break;

      case PPK_NumberOfCopies:
         value = 1;
         break;

      case PPK_Orientation:
         value = d->m_pageLayout.orientation();
         break;

      case PPK_OutputFileName:
         value = d->fileName;
         break;

      case PPK_PageRect:
         // PageRect is returned in device pixels
         value = d->m_pageLayout.paintRectPixels(d->resolution);
         break;

      case PPK_PageSize:
         value = d->m_pageLayout.pageSize().id();
         break;

      case PPK_PaperRect:
         // PaperRect is returned in device pixels
         value = d->m_pageLayout.fullRectPixels(d->resolution);
         break;

      case PPK_PaperName:
         value = d->m_pageLayout.pageSize().name();
         break;

      case PPK_PaperSource:
         if (!d->devMode) {
            value = d->m_printDevice.defaultInputSlot().id;
         } else {
            if (d->devMode->dmDefaultSource >= DMBIN_USER) {
               value = int(d->devMode->dmDefaultSource);
            } else {
               const QList<QPrint::InputSlot> inputSlots = d->m_printDevice.supportedInputSlots();
               const int index = indexOfWindowsId(inputSlots, d->devMode->dmDefaultSource);
               value = index >= 0 ? inputSlots.at(index).id : QPrint::Auto;
            }
         }
         break;

      case PPK_PrinterName:
         value = d->m_printDevice.id();
         break;

      case PPK_Resolution:
         if (d->resolution || d->m_printDevice.isValid()) {
            value = d->resolution;
         }
         break;

      case PPK_SupportedResolutions: {
         QList<QVariant> list;
         for (int resolution : d->m_printDevice.supportedResolutions()) {
            list << resolution;
         }
         value = list;
         break;
      }

      case PPK_WindowsPageSize:
         value = d->m_pageLayout.pageSize().windowsId();
         break;

      case PPK_PaperSources: {


         QList<QVariant> out;
         for (const QPrint::InputSlot inputSlot : d->m_printDevice.supportedInputSlots()) {
            out << QVariant(inputSlot.id == QPrint::CustomInputSlot ? inputSlot.windowsId : int(inputSlot.id));
         }
         value = out;
         break;
      }


      case PPK_CustomPaperSize:
         value = d->m_pageLayout.fullRectPoints().size();
         break;

      case PPK_PageMargins: {
         QList<QVariant> list;
         QMarginsF margins = d->m_pageLayout.margins(QPageSize::Unit::Point);
         list << margins.left() << margins.top() << margins.right() << margins.bottom();
         value = list;
         break;
      }

      case PPK_QPageSize:
         value.setValue(d->m_pageLayout.pageSize());
         break;

      case PPK_QPageMargins: {
         QPair<QMarginsF, QPageLayout::Unit> pair = qMakePair(d->m_pageLayout.margins(), d->m_pageLayout.units());
         value.setValue(pair);
         break;
      }

      case PPK_QPageLayout:
         value.setValue(d->m_pageLayout);
         break;
         // Do nothing
      case PPK_CustomBase:
         break;
   }
   return value;
}

QPrinter::PrinterState QWin32PrintEngine::printerState() const {
   return d_func()->state;
}

HDC QWin32PrintEngine::getDC() const {
   return d_func()->hdc;
}

void QWin32PrintEngine::releaseDC(HDC) const {

}

HGLOBAL *QWin32PrintEngine::createGlobalDevNames() {
   Q_D(QWin32PrintEngine);
   int size = sizeof(DEVNAMES) + d->m_printDevice.id().length() * 2 + 2;
   HGLOBAL *hGlobal = (HGLOBAL *) GlobalAlloc(GMEM_MOVEABLE, size);
   DEVNAMES *dn = (DEVNAMES *) GlobalLock(hGlobal);

   dn->wDriverOffset = 0;
   dn->wDeviceOffset = sizeof(DEVNAMES) / sizeof(wchar_t);
   dn->wOutputOffset = 0;

   QString16 tmp = d->m_printDevice.id().toUtf16();
   memcpy((ushort *)dn + dn->wDeviceOffset, tmp.constData(), tmp.size_storage() * 2 + 2);

   dn->wDefault = 0;
   GlobalUnlock(hGlobal);

   return hGlobal;
}

void QWin32PrintEngine::setGlobalDevMode(HGLOBAL globalDevNames, HGLOBAL globalDevMode) {
   Q_D(QWin32PrintEngine);

   if (globalDevNames) {
      DEVNAMES *dn = (DEVNAMES *) GlobalLock(globalDevNames);

      QString id = QString::fromStdWString(std::wstring((wchar_t *)(dn) + dn->wDeviceOffset));
      QPlatformPrinterSupport *ps = QPlatformPrinterSupportPlugin::get();

      if (ps) {
         d->m_printDevice = ps->createPrintDevice(id.isEmpty() ? ps->defaultPrintDeviceId() : id);
      }

      GlobalUnlock(globalDevNames);
   }

   if (globalDevMode) {
      DEVMODE *dm = (DEVMODE *) GlobalLock(globalDevMode);
      d->release();
      d->globalDevMode = globalDevMode;

      if (d->ownsDevMode) {
         free(d->devMode);
         d->ownsDevMode = false;
      }

      d->devMode = dm;

      std::wstring tmp = d->m_printDevice.id().toStdWString();
      d->hdc = CreateDC(nullptr, &tmp[0], nullptr, dm);

      d->num_copies = d->devMode->dmCopies;
      d->updatePageLayout();

      if (! OpenPrinter(&tmp[0], &d->hPrinter, nullptr)) {
         qWarning("QWin32PrintEngine::setGlobalDevMode() OpenPrinter failed after reading DEVMODE");
      }
   }

   if (d->hdc) {
      d->initHDC();
   }
}

HGLOBAL QWin32PrintEngine::globalDevMode() {
   Q_D(QWin32PrintEngine);
   return d->globalDevMode;
}

void QWin32PrintEnginePrivate::setPageSize(const QPageSize & pageSize) {
   if (!pageSize.isValid()) {
      return;
   }

   Q_ASSERT(devMode);

   // Use the printer page size if supported
   const QPageSize printerPageSize = m_printDevice.supportedPageSize(pageSize);
   const QPageSize usePageSize = printerPageSize.isValid() ? printerPageSize : pageSize;

   const QMarginsF printable = m_printDevice.printableMargins(usePageSize, m_pageLayout.orientation(), resolution);
   m_pageLayout.setPageSize(usePageSize, qt_convertMargins(printable, QPageSize::Unit::Point, m_pageLayout.units()));

   // Setup if Windows custom size, i.e. not a known Windows ID
   if (printerPageSize.isValid()) {
      has_custom_paper_size = false;
      devMode->dmPaperSize = m_pageLayout.pageSize().windowsId();
      devMode->dmFields &= ~(DM_PAPERLENGTH | DM_PAPERWIDTH);
      devMode->dmPaperWidth = 0;
      devMode->dmPaperLength = 0;
   } else {
      devMode->dmPaperSize = DMPAPER_USER;
      devMode->dmFields |= DM_PAPERLENGTH | DM_PAPERWIDTH;
      // Size in tenths of a millimeter
      const QSizeF sizeMM = m_pageLayout.pageSize().size(QPageSize::Millimeter);
      devMode->dmPaperWidth = qRound(sizeMM.width() * 10.0);
      devMode->dmPaperLength = qRound(sizeMM.height() * 10.0);
   }
   updateMetrics();
}

// Update the page layout after any changes made to devMode
void QWin32PrintEnginePrivate::updatePageLayout() {
   Q_ASSERT(devMode);

   // Update orientation first as is needed to obtain printable margins when changing page size
   m_pageLayout.setOrientation(devMode->dmOrientation == DMORIENT_LANDSCAPE ? QPageLayout::Landscape : QPageLayout::Portrait);
   if (devMode->dmPaperSize >= DMPAPER_LAST) {
      // Is a custom size
      QPageSize pageSize = QPageSize(QSizeF(devMode->dmPaperWidth / 10.0f, devMode->dmPaperLength / 10.0f),
                                     QPageSize::Millimeter);
      setPageSize(pageSize);
   } else {
      // Is a supported size
      setPageSize(QPageSize(QPageSize::id(devMode->dmPaperSize)));
   }
   updateMetrics();
}

// Update the cached page paint metrics whenever page layout is changed
void QWin32PrintEnginePrivate::updateMetrics() {
   m_paintRectPixels = m_pageLayout.paintRectPixels(resolution);

   QSizeF sizeMM = m_pageLayout.paintRect(QPageSize::Unit::Millimeter).size();
   m_paintSizeMM = QSize(qRound(sizeMM.width()), qRound(sizeMM.height()));

   // Calculate the origin using the physical device pixels, not our paint pixels
   // Origin is defined as User Margins - Device Margins
   QMarginsF margins = m_pageLayout.margins(QPageSize::Unit::Millimeter) / 25.4;
   origin_x = qRound(margins.left() * dpi_x) - GetDeviceCaps(hdc, PHYSICALOFFSETX);
   origin_y = qRound(margins.top() * dpi_y)  - GetDeviceCaps(hdc, PHYSICALOFFSETY);
}

static void draw_text_item_win(const QPointF & pos, const QTextItemInt & ti, HDC hdc,
      const QTransform & xform, const QPointF & topLeft)
{
   QPointF baseline_pos = xform.inverted().map(xform.map(pos) - topLeft);

   SetTextAlign(hdc, TA_BASELINE);
   SetBkMode(hdc, TRANSPARENT);

   const bool has_kerning = ti.f && ti.f->kerning();

   HFONT hfont = nullptr;

   if (ti.fontEngine->type() == QFontEngine::Win) {
      const QVariantMap userData = ti.fontEngine->userData().toMap();
      const QVariant hfontV = userData.value("hFont");
      const QVariant ttfV = userData.value("trueType");

      if (ttfV.toBool() && hfontV.canConvert<HFONT>()) {
         hfont = hfontV.value<HFONT>();
      }
   }

   if (! hfont) {
      hfont = (HFONT)GetStockObject(ANSI_VAR_FONT);
   }

   HGDIOBJ old_font     = SelectObject(hdc, hfont);
   unsigned int options = ETO_GLYPH_INDEX;
   QGlyphLayout glyphs  = ti.glyphs;

   bool fast = ! has_kerning && ! (ti.flags & QTextItem::RightToLeft);

   for (int i = 0; fast && i < glyphs.numGlyphs; i++) {
      if (glyphs.offsets[i].x != 0 || glyphs.offsets[i].y != 0 || glyphs.justifications[i].space_18d6 != 0
          || glyphs.attributes[i].dontPrint) {
         fast = false;
         break;
      }
   }

   // Scale, rotate and translate here.
   XFORM win_xform;
   win_xform.eM11 = xform.m11();
   win_xform.eM12 = xform.m12();
   win_xform.eM21 = xform.m21();
   win_xform.eM22 = xform.m22();
   win_xform.eDx  = xform.dx();
   win_xform.eDy  = xform.dy();

   SetGraphicsMode(hdc, GM_ADVANCED);
   SetWorldTransform(hdc, &win_xform);

   if (fast) {
      // fast path
      QVarLengthArray<wchar_t> g(glyphs.numGlyphs);

      for (int i = 0; i < glyphs.numGlyphs; ++i) {
         g[i] = glyphs.glyphs[i];
      }

      ExtTextOut(hdc, qRound(baseline_pos.x() + glyphs.offsets[0].x.toReal()),
            qRound(baseline_pos.y() + glyphs.offsets[0].y.toReal()),
            options, nullptr, g.constData(), glyphs.numGlyphs, nullptr);

   } else {
      QVarLengthArray<QFixedPoint> positions;
      QVarLengthArray<glyph_t> _glyphs;

      QTransform matrix = QTransform::fromTranslate(baseline_pos.x(), baseline_pos.y());
      ti.fontEngine->getGlyphPositions(ti.glyphs, matrix, ti.flags, _glyphs, positions);

      if (_glyphs.size() == 0) {
         SelectObject(hdc, old_font);
         return;
      }

      bool outputEntireItem = _glyphs.size() > 0;

      if (outputEntireItem) {
         options |= ETO_PDY;
         QVarLengthArray<INT> glyphDistances(_glyphs.size() * 2);
         QVarLengthArray<wchar_t> g(_glyphs.size());

         for (int i = 0; i < _glyphs.size() - 1; ++i) {
            glyphDistances[i * 2] = qRound(positions[i + 1].x) - qRound(positions[i].x);
            glyphDistances[i * 2 + 1] = qRound(positions[i + 1].y) - qRound(positions[i].y);
            g[i] = _glyphs[i];
         }

         glyphDistances[(_glyphs.size() - 1) * 2] = 0;
         glyphDistances[(_glyphs.size() - 1) * 2 + 1] = 0;
         g[_glyphs.size() - 1] = _glyphs[_glyphs.size() - 1];

         ExtTextOut(hdc, qRound(positions[0].x), qRound(positions[0].y), options, nullptr,
               g.constData(), _glyphs.size(), glyphDistances.data());

      } else {
         int i = 0;

         while (i < _glyphs.size()) {
            wchar_t g = _glyphs[i];

            ExtTextOut(hdc, qRound(positions[i].x), qRound(positions[i].y), options, nullptr, &g, 1, nullptr);
            ++i;
         }
      }
   }

   win_xform.eM11 = win_xform.eM22 = 1.0;
   win_xform.eM12 = win_xform.eM21 = win_xform.eDx = win_xform.eDy = 0.0;
   SetWorldTransform(hdc, &win_xform);

   SelectObject(hdc, old_font);
}

CS_DECLARE_METATYPE(HFONT)
CS_DECLARE_METATYPE(LOGFONT)

#endif // QT_NO_PRINTER
