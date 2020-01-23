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

#include "qxcbimage.h"
#include <QColor>
#include <qimage_p.h>
#include <qdrawhelper_p.h>

#ifdef XCB_USE_RENDER

#include <xcb/render.h>

// 'template' is used as a function argument name in xcb_renderutil.h
#define template template_param

// extern "C" is missing too
extern "C" {
#include <xcb/xcb_renderutil.h>
}

#undef template
#endif

QImage::Format qt_xcb_imageFormatForVisual(QXcbConnection *connection, uint8_t depth,
   const xcb_visualtype_t *visual)
{
   const xcb_format_t *format = connection->formatForDepth(depth);

   if (!visual || !format) {
      return QImage::Format_Invalid;
   }

   if (depth == 32 && format->bits_per_pixel == 32 && visual->red_mask == 0xff0000
      && visual->green_mask == 0xff00 && visual->blue_mask == 0xff) {
      return QImage::Format_ARGB32_Premultiplied;
   }

   if (depth == 30 && format->bits_per_pixel == 32 && visual->red_mask == 0x3ff
      && visual->green_mask == 0x0ffc00 && visual->blue_mask == 0x3ff00000) {
      return QImage::Format_BGR30;
   }

   if (depth == 30 && format->bits_per_pixel == 32 && visual->blue_mask == 0x3ff
      && visual->green_mask == 0x0ffc00 && visual->red_mask == 0x3ff00000) {
      return QImage::Format_RGB30;
   }

   if (depth == 24 && format->bits_per_pixel == 32 && visual->red_mask == 0xff0000
      && visual->green_mask == 0xff00 && visual->blue_mask == 0xff) {
      return QImage::Format_RGB32;
   }

   if (depth == 16 && format->bits_per_pixel == 16 && visual->red_mask == 0xf800
      && visual->green_mask == 0x7e0 && visual->blue_mask == 0x1f) {
      return QImage::Format_RGB16;
   }

   return QImage::Format_Invalid;
}

QPixmap qt_xcb_pixmapFromXPixmap(QXcbConnection *connection, xcb_pixmap_t pixmap,
   int width, int height, int depth,
   const xcb_visualtype_t *visual)
{
   xcb_connection_t *conn = connection->xcb_connection();

   xcb_get_image_cookie_t get_image_cookie =
      xcb_get_image_unchecked(conn, XCB_IMAGE_FORMAT_Z_PIXMAP, pixmap,
         0, 0, width, height, 0xffffffff);

   xcb_get_image_reply_t *image_reply =
      xcb_get_image_reply(conn, get_image_cookie, NULL);

   if (!image_reply) {
      return QPixmap();
   }

   uint8_t *data = xcb_get_image_data(image_reply);
   uint32_t length = xcb_get_image_data_length(image_reply);

   QPixmap result;

   QImage::Format format = qt_xcb_imageFormatForVisual(connection, depth, visual);
   if (format != QImage::Format_Invalid) {
      uint32_t bytes_per_line = length / height;
      QImage image(const_cast<uint8_t *>(data), width, height, bytes_per_line, format);
      uint8_t image_byte_order = connection->setup()->image_byte_order;

      // we may have to swap the byte order
      if ((QSysInfo::ByteOrder == QSysInfo::LittleEndian && image_byte_order == XCB_IMAGE_ORDER_MSB_FIRST)
         || (QSysInfo::ByteOrder == QSysInfo::BigEndian && image_byte_order == XCB_IMAGE_ORDER_LSB_FIRST)) {
         for (int i = 0; i < image.height(); i++) {
            switch (format) {
               case QImage::Format_RGB16: {
                  ushort *p = (ushort *)image.scanLine(i);
                  ushort *end = p + image.width();
                  while (p < end) {
                     *p = ((*p << 8) & 0xff00) | ((*p >> 8) & 0x00ff);
                     p++;
                  }
                  break;
               }
               case QImage::Format_RGB32: // fall-through
               case QImage::Format_ARGB32_Premultiplied: {
                  uint *p = (uint *)image.scanLine(i);
                  uint *end = p + image.width();
                  while (p < end) {
                     *p = ((*p << 24) & 0xff000000) | ((*p << 8) & 0x00ff0000)
                        | ((*p >> 8) & 0x0000ff00) | ((*p >> 24) & 0x000000ff);
                     p++;
                  }
                  break;
               }
               default:
                  Q_ASSERT(false);
            }
         }
      }

      // fix-up alpha channel
      if (format == QImage::Format_RGB32) {
         QRgb *p = (QRgb *)image.bits();
         for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
               p[x] |= 0xff000000;
            }
            p += bytes_per_line / 4;
         }
      } else if (format == QImage::Format_BGR30 || format == QImage::Format_RGB30) {
         QRgb *p = (QRgb *)image.bits();
         for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
               p[x] |= 0xc0000000;
            }
            p += bytes_per_line / 4;
         }
      }

      result = QPixmap::fromImage(image.copy());
   }

   free(image_reply);
   return result;
}

xcb_pixmap_t qt_xcb_XPixmapFromBitmap(QXcbScreen *screen, const QImage &image)
{
   xcb_connection_t *conn = screen->xcb_connection();
   QImage bitmap = image.convertToFormat(QImage::Format_MonoLSB);
   const QRgb c0 = QColor(Qt::black).rgb();
   const QRgb c1 = QColor(Qt::white).rgb();
   if (bitmap.color(0) == c0 && bitmap.color(1) == c1) {
      bitmap.invertPixels();
      bitmap.setColor(0, c1);
      bitmap.setColor(1, c0);
   }
   const int width = bitmap.width();
   const int height = bitmap.height();
   const int bytesPerLine = bitmap.bytesPerLine();
   int destLineSize = width / 8;
   if (width % 8) {
      ++destLineSize;
   }
   const uchar *map = bitmap.bits();
   uint8_t *buf = new uint8_t[height * destLineSize];
   for (int i = 0; i < height; i++) {
      memcpy(buf + (destLineSize * i), map + (bytesPerLine * i), destLineSize);
   }
   xcb_pixmap_t pm = xcb_create_pixmap_from_bitmap_data(conn, screen->root(), buf,
         width, height, 1, 0, 0, 0);
   delete[] buf;
   return pm;
}

xcb_cursor_t qt_xcb_createCursorXRender(QXcbScreen *screen, const QImage &image,
   const QPoint &spot)
{
#ifdef XCB_USE_RENDER
   xcb_connection_t *conn = screen->xcb_connection();
   const int w = image.width();
   const int h = image.height();
   xcb_generic_error_t *error = 0;
   xcb_render_query_pict_formats_cookie_t formatsCookie = xcb_render_query_pict_formats(conn);
   xcb_render_query_pict_formats_reply_t *formatsReply = xcb_render_query_pict_formats_reply(conn,
         formatsCookie,
         &error);
   if (!formatsReply || error) {
      qWarning("qt_xcb_createCursorXRender: query_pict_formats failed");
      free(formatsReply);
      free(error);
      return XCB_NONE;
   }
   xcb_render_pictforminfo_t *fmt = xcb_render_util_find_standard_format(formatsReply,
         XCB_PICT_STANDARD_ARGB_32);
   if (!fmt) {
      qWarning("qt_xcb_createCursorXRender: Failed to find format PICT_STANDARD_ARGB_32");
      free(formatsReply);
      return XCB_NONE;
   }

   QImage img = image.convertToFormat(QImage::Format_ARGB32_Premultiplied);
   xcb_image_t *xi = xcb_image_create(w, h, XCB_IMAGE_FORMAT_Z_PIXMAP,
         32, 32, 32, 32,
         QSysInfo::ByteOrder == QSysInfo::BigEndian ? XCB_IMAGE_ORDER_MSB_FIRST : XCB_IMAGE_ORDER_LSB_FIRST,
         XCB_IMAGE_ORDER_MSB_FIRST,
         0, 0, 0);
   if (!xi) {
      qWarning("qt_xcb_createCursorXRender: xcb_image_create failed");
      free(formatsReply);
      return XCB_NONE;
   }
   xi->data = (uint8_t *) malloc(xi->stride * h);
   if (!xi->data) {
      qWarning("qt_xcb_createCursorXRender: Failed to malloc() image data");
      xcb_image_destroy(xi);
      free(formatsReply);
      return XCB_NONE;
   }
   memcpy(xi->data, img.constBits(), img.byteCount());

   xcb_pixmap_t pix = xcb_generate_id(conn);
   xcb_create_pixmap(conn, 32, pix, screen->root(), w, h);

   xcb_render_picture_t pic = xcb_generate_id(conn);
   xcb_render_create_picture(conn, pic, pix, fmt->id, 0, 0);

   xcb_gcontext_t gc = xcb_generate_id(conn);
   xcb_create_gc(conn, gc, pix, 0, 0);
   xcb_image_put(conn, pix, gc, xi, 0, 0, 0);
   xcb_free_gc(conn, gc);

   xcb_cursor_t cursor = xcb_generate_id(conn);
   xcb_render_create_cursor(conn, cursor, pic, spot.x(), spot.y());

   free(xi->data);
   xcb_image_destroy(xi);
   xcb_render_free_picture(conn, pic);
   xcb_free_pixmap(conn, pix);
   free(formatsReply);
   return cursor;

#else
   return XCB_NONE;
#endif

}

