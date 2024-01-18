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

/*
 * Copyright (C) 2004, 2005 Daniel M. Duley
 *
 * Imlib2 is (C) Carsten Haitzler and various contributors. The MMX code
 * is by Willem Monsuwe <willem@stack.nl>. All other modifications are
 * (C) Daniel M. Duley.
*/

#include <qimagescale_p.h>
#include <qdrawhelper_p.h>
#include <qimage.h>
#include <qcolor.h>

namespace QImageScale {

const unsigned int **qimageCalcYPoints(const unsigned int *src, int sw, int sh, int dh);
int *qimageCalcXPoints(int sw, int dw);
int *qimageCalcApoints(int s, int d, int up);
QImageScaleInfo *qimageFreeScaleInfo(QImageScaleInfo *isi);

QImageScaleInfo *qimageCalcScaleInfo(const QImage &img, int sw, int sh,
   int dw, int dh, char aa);
}

using namespace QImageScale;


const unsigned int **QImageScale::qimageCalcYPoints(const unsigned int *src,
   int sw, int sh, int dh)
{
   const unsigned int **p;
   int j = 0, rv = 0;
   qint64 val, inc;

   if (dh < 0) {
      dh = -dh;
      rv = 1;
   }
   p = new const unsigned int *[dh + 1];

   int up = qAbs(dh) >= sh;
   val = up ? 0x8000 * sh / dh - 0x8000 : 0;
   inc = (((qint64)sh) << 16) / dh;
   for (int i = 0; i < dh; i++) {
      p[j++] = src + qMax(0LL, val >> 16) * sw;
      val += inc;
   }
   if (rv) {
      for (int i = dh / 2; --i >= 0; ) {
         const unsigned int *tmp = p[i];
         p[i] = p[dh - i - 1];
         p[dh - i - 1] = tmp;
      }
   }
   return (p);
}

int *QImageScale::qimageCalcXPoints(int sw, int dw)
{
   int *p, j = 0, rv = 0;
   qint64 val, inc;

   if (dw < 0) {
      dw = -dw;
      rv = 1;
   }
   p = new int[dw + 1];

   int up = qAbs(dw) >= sw;
   val = up ? 0x8000 * sw / dw - 0x8000 : 0;
   inc = (((qint64)sw) << 16) / dw;
   for (int i = 0; i < dw; i++) {
      p[j++] = qMax(0LL, val >> 16);
      val += inc;
   }

   if (rv) {
      for (int i = dw / 2; --i >= 0; ) {
         int tmp = p[i];
         p[i] = p[dw - i - 1];
         p[dw - i - 1] = tmp;
      }
   }
   return p;
}

int *QImageScale::qimageCalcApoints(int s, int d, int up)
{
   int *p, j = 0, rv = 0;

   if (d < 0) {
      rv = 1;
      d = -d;
   }
   p = new int[d];

   if (up) {
      /* scaling up */
      qint64 val = 0x8000 * s / d - 0x8000;
      qint64 inc = (((qint64)s) << 16) / d;
      for (int i = 0; i < d; i++) {
         int pos = val >> 16;
         if (pos < 0) {
            p[j++] = 0;
         } else if (pos >= (s - 1)) {
            p[j++] = 0;
         } else {
            p[j++] = (val >> 8) - ((val >> 8) & 0xffffff00);
         }
         val += inc;
      }
   } else {
      /* scaling down */
      qint64 val = 0;
      qint64 inc = (((qint64)s) << 16) / d;
      int Cp = (((d << 14) + s - 1) / s);
      for (int i = 0; i < d; i++) {
         int ap = ((0x10000 - (val & 0xffff)) * Cp) >> 16;
         p[j] = ap | (Cp << 16);
         j++;
         val += inc;
      }
   }
   if (rv) {
      int tmp;
      for (int i = d / 2; --i >= 0; ) {
         tmp = p[i];
         p[i] = p[d - i - 1];
         p[d - i - 1] = tmp;
      }
   }
   return p;
}

QImageScaleInfo *QImageScale::qimageFreeScaleInfo(QImageScaleInfo *isi)
{
   if (isi) {
      delete[] isi->xpoints;
      delete[] isi->ypoints;
      delete[] isi->xapoints;
      delete[] isi->yapoints;
      delete isi;
   }

   return nullptr;
}

QImageScaleInfo *QImageScale::qimageCalcScaleInfo(const QImage &img,
   int sw, int sh,
   int dw, int dh, char aa)
{
   QImageScaleInfo *isi;
   int scw, sch;

   scw = dw * qint64(img.width()) / sw;
   sch = dh * qint64(img.height()) / sh;

   isi = new QImageScaleInfo;
   if (!isi) {
      return nullptr;
   }
   memset(isi, 0, sizeof(QImageScaleInfo));

   isi->xup_yup = (qAbs(dw) >= sw) + ((qAbs(dh) >= sh) << 1);

   isi->xpoints = qimageCalcXPoints(img.width(), scw);
   if (!isi->xpoints) {
      return (qimageFreeScaleInfo(isi));
   }

   isi->ypoints = qimageCalcYPoints((const unsigned int *)img.scanLine(0),
         img.bytesPerLine() / 4, img.height(), sch);
   if (!isi->ypoints) {
      return (qimageFreeScaleInfo(isi));
   }

   if (aa) {
      isi->xapoints = qimageCalcApoints(img.width(), scw, isi->xup_yup & 1);
      if (!isi->xapoints) {
         return (qimageFreeScaleInfo(isi));
      }
      isi->yapoints = qimageCalcApoints(img.height(), sch, isi->xup_yup & 2);
      if (!isi->yapoints) {
         return (qimageFreeScaleInfo(isi));
      }
   }
   return (isi);
}

static void qt_qimageScaleAARGBA_up_x_down_y(QImageScaleInfo *isi, unsigned int *dest,
   int dw, int dh, int dow, int sow);

static void qt_qimageScaleAARGBA_down_x_up_y(QImageScaleInfo *isi, unsigned int *dest,
   int dw, int dh, int dow, int sow);

static void qt_qimageScaleAARGBA_down_xy(QImageScaleInfo *isi, unsigned int *dest,
   int dw, int dh, int dow, int sow);

#if defined(QT_COMPILER_SUPPORTS_SSE4_1)

template <bool RGB>
void qt_qimageScaleAARGBA_up_x_down_y_sse4(QImageScaleInfo *isi, unsigned int *dest,
   int dw, int dh, int dow, int sow);

template <bool RGB>
void qt_qimageScaleAARGBA_down_x_up_y_sse4(QImageScaleInfo *isi, unsigned int *dest,
   int dw, int dh, int dow, int sow);

template <bool RGB>
void qt_qimageScaleAARGBA_down_xy_sse4(QImageScaleInfo *isi, unsigned int *dest,
   int dw, int dh, int dow, int sow);

#endif

static void qt_qimageScaleAARGBA_up_xy(QImageScaleInfo *isi, unsigned int *dest,
   int dw, int dh, int dow, int sow)
{
   const unsigned int **ypoints = isi->ypoints;
   int *xpoints = isi->xpoints;
   int *xapoints = isi->xapoints;
   int *yapoints = isi->yapoints;

   /* go through every scanline in the output buffer */
   for (int y = 0; y < dh; y++) {
      /* calculate the source line scan from */
      const unsigned int *sptr = ypoints[y];
      unsigned int *dptr = dest + (y * dow);
      const int yap = yapoints[y];
      if (yap > 0) {
         for (int x = 0; x < dw; x++) {
            const unsigned int *pix = sptr + xpoints[x];
            const int xap = xapoints[x];
            if (xap > 0) {
               *dptr = interpolate_4_pixels(pix[0], pix[1], pix[sow], pix[sow + 1], xap, yap);
            } else {
               *dptr = INTERPOLATE_PIXEL_256(pix[0], 256 - yap, pix[sow], yap);
            }
            dptr++;
         }
      } else {
         for (int x = 0; x < dw; x++) {
            const unsigned int *pix = sptr + xpoints[x];
            const int xap = xapoints[x];
            if (xap > 0) {
               *dptr = INTERPOLATE_PIXEL_256(pix[0], 256 - xap, pix[1], xap);
            } else {
               *dptr = pix[0];
            }
            dptr++;
         }
      }
   }
}

/* scale by area sampling */
static void qt_qimageScaleAARGBA(QImageScaleInfo *isi, unsigned int *dest,
   int dw, int dh, int dow, int sow)
{
   /* scaling up both ways */
   if (isi->xup_yup == 3) {
      qt_qimageScaleAARGBA_up_xy(isi, dest, dw, dh, dow, sow);
   }

   // if we are scaling down vertically
   else if (isi->xup_yup == 1) {

#ifdef QT_COMPILER_SUPPORTS_SSE4_1
      if (qCpuHasFeature(SSE4_1)) {
         qt_qimageScaleAARGBA_up_x_down_y_sse4<false>(isi, dest, dw, dh, dow, sow);
      } else
#endif

         qt_qimageScaleAARGBA_up_x_down_y(isi, dest, dw, dh, dow, sow);
   }

   // if we are scaling down horizontally
   else if (isi->xup_yup == 2) {

#ifdef QT_COMPILER_SUPPORTS_SSE4_1
      if (qCpuHasFeature(SSE4_1)) {
         qt_qimageScaleAARGBA_down_x_up_y_sse4<false>(isi, dest, dw, dh, dow, sow);
      } else
#endif
         qt_qimageScaleAARGBA_down_x_up_y(isi, dest, dw, dh, dow, sow);
   }

   // if we are scaling down horizontally & vertically
   else {

#ifdef QT_COMPILER_SUPPORTS_SSE4_1
      if (qCpuHasFeature(SSE4_1)) {
         qt_qimageScaleAARGBA_down_xy_sse4<false>(isi, dest, dw, dh, dow, sow);
      } else
#endif
         qt_qimageScaleAARGBA_down_xy(isi, dest, dw, dh, dow, sow);
   }
}

static inline void qt_qimageScaleAARGBA_helper(const unsigned int *pix, int xyap, int Cxy,
      int step, int &r, int &g, int &b, int &a)
{
   r = qRed(*pix)   * xyap;
   g = qGreen(*pix) * xyap;
   b = qBlue(*pix)  * xyap;
   a = qAlpha(*pix) * xyap;
   int j;

   for (j = (1 << 14) - xyap; j > Cxy; j -= Cxy) {
      pix += step;
      r += qRed(*pix)   * Cxy;
      g += qGreen(*pix) * Cxy;
      b += qBlue(*pix)  * Cxy;
      a += qAlpha(*pix) * Cxy;
   }

   pix += step;
   r += qRed(*pix)   * j;
   g += qGreen(*pix) * j;
   b += qBlue(*pix)  * j;
   a += qAlpha(*pix) * j;
}

static void qt_qimageScaleAARGBA_up_x_down_y(QImageScaleInfo *isi, unsigned int *dest,
   int dw, int dh, int dow, int sow)
{
   const unsigned int **ypoints = isi->ypoints;
   int *xpoints = isi->xpoints;
   int *xapoints = isi->xapoints;
   int *yapoints = isi->yapoints;

   /* go through every scanline in the output buffer */
   for (int y = 0; y < dh; y++) {
      int Cy = yapoints[y] >> 16;
      int yap = yapoints[y] & 0xffff;

      unsigned int *dptr = dest + (y * dow);
      for (int x = 0; x < dw; x++) {
         const unsigned int *sptr = ypoints[y] + xpoints[x];
         int r, g, b, a;
         qt_qimageScaleAARGBA_helper(sptr, yap, Cy, sow, r, g, b, a);

         int xap = xapoints[x];
         if (xap > 0) {
            int rr, gg, bb, aa;
            qt_qimageScaleAARGBA_helper(sptr + 1, yap, Cy, sow, rr, gg, bb, aa);

            r = r * (256 - xap);
            g = g * (256 - xap);
            b = b * (256 - xap);
            a = a * (256 - xap);
            r = (r + (rr * xap)) >> 8;
            g = (g + (gg * xap)) >> 8;
            b = (b + (bb * xap)) >> 8;
            a = (a + (aa * xap)) >> 8;
         }
         *dptr++ = qRgba(r >> 14, g >> 14, b >> 14, a >> 14);
      }
   }
}
static void qt_qimageScaleAARGBA_down_x_up_y(QImageScaleInfo *isi, unsigned int *dest,
   int dw, int dh, int dow, int sow)
{
   const unsigned int **ypoints = isi->ypoints;
   int *xpoints = isi->xpoints;
   int *xapoints = isi->xapoints;
   int *yapoints = isi->yapoints;

   /* go through every scanline in the output buffer */
   for (int y = 0; y < dh; y++) {
      unsigned int *dptr = dest + (y * dow);
      for (int x = 0; x < dw; x++) {
         int Cx = xapoints[x] >> 16;
         int xap = xapoints[x] & 0xffff;

         const unsigned int *sptr = ypoints[y] + xpoints[x];
         int r, g, b, a;
         qt_qimageScaleAARGBA_helper(sptr, xap, Cx, 1, r, g, b, a);

         int yap = yapoints[y];
         if (yap > 0) {
            int rr, gg, bb, aa;
            qt_qimageScaleAARGBA_helper(sptr + sow, xap, Cx, 1, rr, gg, bb, aa);

            r = r * (256 - yap);
            g = g * (256 - yap);
            b = b * (256 - yap);
            a = a * (256 - yap);
            r = (r + (rr * yap)) >> 8;
            g = (g + (gg * yap)) >> 8;
            b = (b + (bb * yap)) >> 8;
            a = (a + (aa * yap)) >> 8;
         }
         *dptr = qRgba(r >> 14, g >> 14, b >> 14, a >> 14);
         dptr++;
      }
   }
}

static void qt_qimageScaleAARGBA_down_xy(QImageScaleInfo *isi, unsigned int *dest,
   int dw, int dh, int dow, int sow)
{
   const unsigned int **ypoints = isi->ypoints;
   int *xpoints = isi->xpoints;
   int *xapoints = isi->xapoints;
   int *yapoints = isi->yapoints;

   for (int y = 0; y < dh; y++) {
      int Cy = (yapoints[y]) >> 16;
      int yap = (yapoints[y]) & 0xffff;

      unsigned int *dptr = dest + (y * dow);
      for (int x = 0; x < dw; x++) {
         int Cx = xapoints[x] >> 16;
         int xap = xapoints[x] & 0xffff;

         const unsigned int *sptr = ypoints[y] + xpoints[x];
         int rx, gx, bx, ax;
         qt_qimageScaleAARGBA_helper(sptr, xap, Cx, 1, rx, gx, bx, ax);

         int r = ((rx >> 4) * yap);
         int g = ((gx >> 4) * yap);
         int b = ((bx >> 4) * yap);
         int a = ((ax >> 4) * yap);

         int j;
         for (j = (1 << 14) - yap; j > Cy; j -= Cy) {
            sptr += sow;
            qt_qimageScaleAARGBA_helper(sptr, xap, Cx, 1, rx, gx, bx, ax);
            r += ((rx >> 4) * Cy);
            g += ((gx >> 4) * Cy);
            b += ((bx >> 4) * Cy);
            a += ((ax >> 4) * Cy);
         }
         sptr += sow;
         qt_qimageScaleAARGBA_helper(sptr, xap, Cx, 1, rx, gx, bx, ax);

         r += ((rx >> 4) * j);
         g += ((gx >> 4) * j);
         b += ((bx >> 4) * j);
         a += ((ax >> 4) * j);

         *dptr = qRgba(r >> 24, g >> 24, b >> 24, a >> 24);
         dptr++;
      }
   }
}

static void qt_qimageScaleAARGB_up_x_down_y(QImageScaleInfo *isi, unsigned int *dest,
   int dw, int dh, int dow, int sow);

static void qt_qimageScaleAARGB_down_x_up_y(QImageScaleInfo *isi, unsigned int *dest,
   int dw, int dh, int dow, int sow);

static void qt_qimageScaleAARGB_down_xy(QImageScaleInfo *isi, unsigned int *dest,
   int dw, int dh, int dow, int sow);


/* scale by area sampling - IGNORE the ALPHA byte*/
static void qt_qimageScaleAARGB(QImageScaleInfo *isi, unsigned int *dest,
   int dw, int dh, int dow, int sow)
{
   /* scaling up both ways */
   if (isi->xup_yup == 3) {
      qt_qimageScaleAARGBA_up_xy(isi, dest, dw, dh, dow, sow);
   }

   // if we are scaling down vertically
   else if (isi->xup_yup == 1) {

#ifdef QT_COMPILER_SUPPORTS_SSE4_1
      if (qCpuHasFeature(SSE4_1)) {
         qt_qimageScaleAARGBA_up_x_down_y_sse4<true>(isi, dest, dw, dh, dow, sow);
      } else
#endif
         qt_qimageScaleAARGB_up_x_down_y(isi, dest, dw, dh, dow, sow);
   }

   // if we are scaling down horizontally
   else if (isi->xup_yup == 2) {

#ifdef QT_COMPILER_SUPPORTS_SSE4_1
      if (qCpuHasFeature(SSE4_1)) {
         qt_qimageScaleAARGBA_down_x_up_y_sse4<true>(isi, dest, dw, dh, dow, sow);
      } else
#endif
         qt_qimageScaleAARGB_down_x_up_y(isi, dest, dw, dh, dow, sow);
   }

   // if we are scaling down horizontally & vertically
   else {

#ifdef QT_COMPILER_SUPPORTS_SSE4_1
      if (qCpuHasFeature(SSE4_1)) {
         qt_qimageScaleAARGBA_down_xy_sse4<true>(isi, dest, dw, dh, dow, sow);
      } else
#endif
         qt_qimageScaleAARGB_down_xy(isi, dest, dw, dh, dow, sow);
   }
}

static inline void qt_qimageScaleAARGB_helper(const unsigned int *pix, int xyap, int Cxy,
      int step, int &r, int &g, int &b)
{
   r = qRed(*pix)   * xyap;
   g = qGreen(*pix) * xyap;
   b = qBlue(*pix)  * xyap;
   int j;
   for (j = (1 << 14) - xyap; j > Cxy; j -= Cxy) {
      pix += step;
      r += qRed(*pix)   * Cxy;
      g += qGreen(*pix) * Cxy;
      b += qBlue(*pix)  * Cxy;
   }
   pix += step;
   r += qRed(*pix)   * j;
   g += qGreen(*pix) * j;
   b += qBlue(*pix)  * j;
}

static void qt_qimageScaleAARGB_up_x_down_y(QImageScaleInfo *isi, unsigned int *dest,
   int dw, int dh, int dow, int sow)
{
   const unsigned int **ypoints = isi->ypoints;
   int *xpoints = isi->xpoints;
   int *xapoints = isi->xapoints;
   int *yapoints = isi->yapoints;

   /* go through every scanline in the output buffer */
   for (int y = 0; y < dh; y++) {
      int Cy = yapoints[y] >> 16;
      int yap = yapoints[y] & 0xffff;

      unsigned int *dptr = dest + (y * dow);
      for (int x = 0; x < dw; x++) {
         const unsigned int *sptr = ypoints[y] + xpoints[x];
         int r, g, b;
         qt_qimageScaleAARGB_helper(sptr, yap, Cy, sow, r, g, b);

         int xap = xapoints[x];
         if (xap > 0) {
            int rr, bb, gg;
            qt_qimageScaleAARGB_helper(sptr + 1, yap, Cy, sow, rr, gg, bb);

            r = r * (256 - xap);
            g = g * (256 - xap);
            b = b * (256 - xap);
            r = (r + (rr * xap)) >> 8;
            g = (g + (gg * xap)) >> 8;
            b = (b + (bb * xap)) >> 8;
         }
         *dptr++ = qRgb(r >> 14, g >> 14, b >> 14);
      }
   }
}

static void qt_qimageScaleAARGB_down_x_up_y(QImageScaleInfo *isi, unsigned int *dest,
   int dw, int dh, int dow, int sow)
{
   const unsigned int **ypoints = isi->ypoints;
   int *xpoints = isi->xpoints;
   int *xapoints = isi->xapoints;
   int *yapoints = isi->yapoints;

   /* go through every scanline in the output buffer */
   for (int y = 0; y < dh; y++) {
      unsigned int *dptr = dest + (y * dow);
      for (int x = 0; x < dw; x++) {
         int Cx = xapoints[x] >> 16;
         int xap = xapoints[x] & 0xffff;

         const unsigned int *sptr = ypoints[y] + xpoints[x];
         int r, g, b;
         qt_qimageScaleAARGB_helper(sptr, xap, Cx, 1, r, g, b);

         int yap = yapoints[y];
         if (yap > 0) {
            int rr, bb, gg;
            qt_qimageScaleAARGB_helper(sptr + sow, xap, Cx, 1, rr, gg, bb);

            r = r * (256 - yap);
            g = g * (256 - yap);
            b = b * (256 - yap);
            r = (r + (rr * yap)) >> 8;
            g = (g + (gg * yap)) >> 8;
            b = (b + (bb * yap)) >> 8;
         }
         *dptr++ = qRgb(r >> 14, g >> 14, b >> 14);
      }
   }
}

static void qt_qimageScaleAARGB_down_xy(QImageScaleInfo *isi, unsigned int *dest,
   int dw, int dh, int dow, int sow)
{
   const unsigned int **ypoints = isi->ypoints;
   int *xpoints = isi->xpoints;
   int *xapoints = isi->xapoints;
   int *yapoints = isi->yapoints;

   for (int y = 0; y < dh; y++) {
      int Cy = yapoints[y] >> 16;
      int yap = yapoints[y] & 0xffff;

      unsigned int *dptr = dest + (y * dow);
      for (int x = 0; x < dw; x++) {
         int Cx = xapoints[x] >> 16;
         int xap = xapoints[x] & 0xffff;

         const unsigned int *sptr = ypoints[y] + xpoints[x];
         int rx, gx, bx;
         qt_qimageScaleAARGB_helper(sptr, xap, Cx, 1, rx, gx, bx);

         int r = (rx >> 4) * yap;
         int g = (gx >> 4) * yap;
         int b = (bx >> 4) * yap;

         int j;
         for (j = (1 << 14) - yap; j > Cy; j -= Cy) {
            sptr += sow;
            qt_qimageScaleAARGB_helper(sptr, xap, Cx, 1, rx, gx, bx);

            r += (rx >> 4) * Cy;
            g += (gx >> 4) * Cy;
            b += (bx >> 4) * Cy;
         }
         sptr += sow;
         qt_qimageScaleAARGB_helper(sptr, xap, Cx, 1, rx, gx, bx);

         r += (rx >> 4) * j;
         g += (gx >> 4) * j;
         b += (bx >> 4) * j;

         *dptr = qRgb(r >> 24, g >> 24, b >> 24);
         dptr++;
      }
   }
}

QImage qSmoothScaleImage(const QImage &src, int dw, int dh)
{
   QImage buffer;
   if (src.isNull() || dw <= 0 || dh <= 0) {
      return buffer;
   }

   int w = src.width();
   int h = src.height();
   QImageScaleInfo *scaleinfo =
      qimageCalcScaleInfo(src, w, h, dw, dh, true);
   if (!scaleinfo) {
      return buffer;
   }

   buffer = QImage(dw, dh, src.format());
   if (buffer.isNull()) {
      qWarning("QImage::qSmoothScaleImage() Out of memory");
      qimageFreeScaleInfo(scaleinfo);
      return QImage();
   }

   if (src.hasAlphaChannel())
      qt_qimageScaleAARGBA(scaleinfo, (unsigned int *)buffer.scanLine(0),
         dw, dh, dw, src.bytesPerLine() / 4);
   else
      qt_qimageScaleAARGB(scaleinfo, (unsigned int *)buffer.scanLine(0),
         dw, dh, dw, src.bytesPerLine() / 4);

   qimageFreeScaleInfo(scaleinfo);
   return buffer;
}

