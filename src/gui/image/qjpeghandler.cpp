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

#include <qjpeghandler_p.h>
#include <qimage.h>
#include <qvariant.h>
#include <qvector.h>
#include <qbuffer.h>
#include <qmath.h>
#include <qsimd_p.h>

#include <stdio.h>      // jpeglib needs this to be pre-included
#include <setjmp.h>

#ifdef FAR
#undef FAR
#endif

// including jpeglib.h seems to be a little messy
extern "C" {
   // mingw includes rpcndr.h but does not define boolean
#if defined(Q_OS_WIN) && defined(Q_CC_GNU)
#   if defined(__RPCNDR_H__) && !defined(boolean)
   typedef unsigned char boolean;
#       define HAVE_BOOLEAN
#   endif
#endif

#define XMD_H           // shut JPEGlib up
#if defined(Q_OS_UNIXWARE)
#  define HAVE_BOOLEAN  // libjpeg under Unixware seems to need this
#endif
#include <jpeglib.h>
#ifdef const
#  undef const          // remove crazy C hackery in jconfig.h
#endif
}

Q_GUI_EXPORT void qt_convert_rgb888_to_rgb32(quint32 *dst, const uchar *src, int len);

typedef void (*Rgb888ToRgb32Converter)(quint32 *dst, const uchar *src, int len);

struct my_error_mgr : public jpeg_error_mgr {
   jmp_buf setjmp_buffer;
};

extern "C" {

   static void my_error_exit (j_common_ptr cinfo)
   {
      my_error_mgr *myerr = (my_error_mgr *) cinfo->err;
      char buffer[JMSG_LENGTH_MAX];
      (*cinfo->err->format_message)(cinfo, buffer);
      qWarning("%s", buffer);
      longjmp(myerr->setjmp_buffer, 1);
   }

   static void my_output_message(j_common_ptr cinfo)
   {
      char buffer[JMSG_LENGTH_MAX];
      (*cinfo->err->format_message)(cinfo, buffer);
      qWarning("%s", buffer);
   }

}

static const int max_buf = 4096;

struct my_jpeg_source_mgr : public jpeg_source_mgr {
   // Nothing dynamic - cannot rely on destruction over longjump
   QIODevice *device;
   JOCTET buffer[max_buf];
   const QBuffer *memDevice;

 public:
   my_jpeg_source_mgr(QIODevice *device);
};


extern "C" {

   static void qt_init_source(j_decompress_ptr)
   {
   }

   static boolean qt_fill_input_buffer(j_decompress_ptr cinfo)
   {
      my_jpeg_source_mgr *src = (my_jpeg_source_mgr *)cinfo->src;
      qint64 num_read = 0;
      if (src->memDevice) {
         src->next_input_byte = (const JOCTET *)(src->memDevice->data().constData() + src->memDevice->pos());
         num_read = src->memDevice->data().size() - src->memDevice->pos();
         src->device->seek(src->memDevice->data().size());
      } else {
         src->next_input_byte = src->buffer;
         num_read = src->device->read((char *)src->buffer, max_buf);
      }
      if (num_read <= 0) {
         // Insert a fake EOI marker - as per jpeglib recommendation
         src->next_input_byte = src->buffer;
         src->buffer[0] = (JOCTET) 0xFF;
         src->buffer[1] = (JOCTET) JPEG_EOI;
         src->bytes_in_buffer = 2;
      } else {
         src->bytes_in_buffer = num_read;
      }
      return TRUE;
   }

   static void qt_skip_input_data(j_decompress_ptr cinfo, long num_bytes)
   {
      my_jpeg_source_mgr *src = (my_jpeg_source_mgr *)cinfo->src;

      // `dumb' implementation from jpeglib

      /* Just a dumb implementation for now.  Could use fseek() except
       * it doesn't work on pipes.  Not clear that being smart is worth
       * any trouble anyway --- large skips are infrequent.
       */
      if (num_bytes > 0) {
         while (num_bytes > (long) src->bytes_in_buffer) {  // Should not happen in case of memDevice
            num_bytes -= (long) src->bytes_in_buffer;
            (void) qt_fill_input_buffer(cinfo);
            /* note we assume that qt_fill_input_buffer will never return false,
            * so suspension need not be handled.
            */
         }
         src->next_input_byte += (size_t) num_bytes;
         src->bytes_in_buffer -= (size_t) num_bytes;
      }
   }

   static void qt_term_source(j_decompress_ptr cinfo)
   {
      my_jpeg_source_mgr *src = (my_jpeg_source_mgr *)cinfo->src;
      if (! src->device->isSequential()) {
         src->device->seek(src->device->pos() - src->bytes_in_buffer);
      }
   }
}


inline my_jpeg_source_mgr::my_jpeg_source_mgr(QIODevice *device)
{
   jpeg_source_mgr::init_source = qt_init_source;
   jpeg_source_mgr::fill_input_buffer = qt_fill_input_buffer;
   jpeg_source_mgr::skip_input_data = qt_skip_input_data;
   jpeg_source_mgr::resync_to_restart = jpeg_resync_to_restart;
   jpeg_source_mgr::term_source = qt_term_source;
   this->device = device;
   memDevice = qobject_cast<QBuffer *>(device);
   bytes_in_buffer = 0;
   next_input_byte = buffer;
}

inline static bool read_jpeg_size(int &w, int &h, j_decompress_ptr cinfo)
{
   (void) jpeg_calc_output_dimensions(cinfo);

   w = cinfo->output_width;
   h = cinfo->output_height;

   return true;
}

#define HIGH_QUALITY_THRESHOLD 50

inline static bool read_jpeg_format(QImage::Format &format, j_decompress_ptr cinfo)
{

   bool result = true;
   switch (cinfo->output_components) {
      case 1:
         format = QImage::Format_Grayscale8;
         break;
      case 3:
      case 4:
         format = QImage::Format_RGB32;
         break;
      default:
         result = false;
         break;
   }

   cinfo->output_scanline = cinfo->output_height;

   return result;
}

static bool ensureValidImage(QImage *dest, struct jpeg_decompress_struct *info, const QSize &size)
{
   QImage::Format format;

   switch (info->output_components) {
      case 1:
         format = QImage::Format_Grayscale8;
         break;
      case 3:
      case 4:
         format = QImage::Format_RGB32;
         break;
      default:
         return false; // unsupported format
   }

   if (dest->size() != size || dest->format() != format) {
      *dest = QImage(size, format);
   }

   return ! dest->isNull();
}

static bool read_jpeg_image(QImage *outImage,
   QSize scaledSize, QRect scaledClipRect,
   QRect clipRect, volatile int inQuality,
   Rgb888ToRgb32Converter converter,
   j_decompress_ptr info, struct my_error_mgr *err  )
{
   if (!setjmp(err->setjmp_buffer)) {
      // -1 means default quality.
      int quality = inQuality;

      if (quality < 0) {
         quality = 75;
      }

      // If possible, merge the scaledClipRect into either scaledSize
      // or clipRect to avoid doing a separate scaled clipping pass.
      // Best results are achieved by clipping before scaling, not after.
      if (!scaledClipRect.isEmpty()) {
         if (scaledSize.isEmpty() && clipRect.isEmpty()) {
            // No clipping or scaling before final clip.
            clipRect = scaledClipRect;
            scaledClipRect = QRect();
         } else if (scaledSize.isEmpty()) {
            // Clipping, but no scaling: combine the clip regions.
            scaledClipRect.translate(clipRect.topLeft());
            clipRect = scaledClipRect.intersected(clipRect);
            scaledClipRect = QRect();
         } else if (clipRect.isEmpty()) {
            // No clipping, but scaling: if we can map back to an
            // integer pixel boundary, then clip before scaling.
            if ((info->image_width % scaledSize.width()) == 0 &&
               (info->image_height % scaledSize.height()) == 0) {
               int x = scaledClipRect.x() * info->image_width /
                  scaledSize.width();
               int y = scaledClipRect.y() * info->image_height /
                  scaledSize.height();
               int width = (scaledClipRect.right() + 1) *
                  info->image_width / scaledSize.width() - x;
               int height = (scaledClipRect.bottom() + 1) *
                  info->image_height / scaledSize.height() - y;
               clipRect = QRect(x, y, width, height);
               scaledSize = scaledClipRect.size();
               scaledClipRect = QRect();
            }
         } else {
            // Clipping and scaling: too difficult to figure out,
            // and not a likely use case, so do it the long way.
         }
      }

      // Determine the scale factor to pass to libjpeg for quick downscaling.
      if (!scaledSize.isEmpty() && info->image_width && info->image_height) {
         if (clipRect.isEmpty()) {
            double f = qMin(double(info->image_width) / scaledSize.width(),
                  double(info->image_height) / scaledSize.height());

            // libjpeg supports M/8 scaling with M=[1,16]. All downscaling factors
            // are a speed improvement, but upscaling during decode is slower.
            info->scale_num   = qBound(1, qCeil(8 / f), 8);
            info->scale_denom = 8;

         } else {
            info->scale_denom =
               qMin(clipRect.width() / scaledSize.width(),
                  clipRect.height() / scaledSize.height());

            if (info->scale_denom < 2) {
               info->scale_denom = 1;
            } else if (info->scale_denom < 4) {
               info->scale_denom = 2;
            } else if (info->scale_denom < 8) {
               info->scale_denom = 4;
            } else {
               info->scale_denom = 8;
            }

            info->scale_num = 1;

            // Correct the scale factor so that we clip accurately.
            // It is recommended that the clip rectangle be aligned
            // on an 8-pixel boundary for best performance.
            while (info->scale_denom > 1 &&
               ((clipRect.x() % info->scale_denom) != 0 ||
                  (clipRect.y() % info->scale_denom) != 0 ||
                  (clipRect.width() % info->scale_denom) != 0 ||
                  (clipRect.height() % info->scale_denom) != 0)) {
               info->scale_denom /= 2;
            }
         }
      }

      // If high quality not required, use fast decompression
      if ( quality < HIGH_QUALITY_THRESHOLD ) {
         info->dct_method = JDCT_IFAST;
         info->do_fancy_upsampling = FALSE;
      }

      (void) jpeg_calc_output_dimensions(info);

      // Determine the clip region to extract.
      QRect imageRect(0, 0, info->output_width, info->output_height);
      QRect clip;
      if (clipRect.isEmpty()) {
         clip = imageRect;
      } else if (info->scale_denom == info->scale_num) {
         clip = clipRect.intersected(imageRect);
      } else {
         // The scale factor was corrected above to ensure that
         // we don't miss pixels when we scale the clip rectangle.
         clip = QRect(clipRect.x() / int(info->scale_denom),
               clipRect.y() / int(info->scale_denom),
               clipRect.width() / int(info->scale_denom),
               clipRect.height() / int(info->scale_denom));
         clip = clip.intersected(imageRect);
      }

      // Allocate memory for the clipped QImage.
      if (!ensureValidImage(outImage, info, clip.size())) {
         longjmp(err->setjmp_buffer, 1);
      }

      // Avoid memcpy() overhead if grayscale with no clipping.
      bool quickGray = (info->output_components == 1 &&
            clip == imageRect);
      if (!quickGray) {
         // Ask the jpeg library to allocate a temporary row.
         // The library will automatically delete it for us later.
         // The libjpeg docs say we should do this before calling
         // jpeg_start_decompress().  We can't use "new" here
         // because we are inside the setjmp() block and an error
         // in the jpeg input stream would cause a memory leak.
         JSAMPARRAY rows = (info->mem->alloc_sarray)
            ((j_common_ptr)info, JPOOL_IMAGE,
               info->output_width * info->output_components, 1);

         (void) jpeg_start_decompress(info);

         while (info->output_scanline < info->output_height) {
            int y = int(info->output_scanline) - clip.y();
            if (y >= clip.height()) {
               break;   // We've read the entire clip region, so abort.
            }

            (void) jpeg_read_scanlines(info, rows, 1);

            if (y < 0) {
               continue;   // Haven't reached the starting line yet.
            }

            if (info->output_components == 3) {
               uchar *in = rows[0] + clip.x() * 3;
               QRgb *out = (QRgb *)outImage->scanLine(y);
               converter(out, in, clip.width());

            } else if (info->out_color_space == JCS_CMYK) {
               // Convert CMYK->RGB.
               uchar *in = rows[0] + clip.x() * 4;
               QRgb *out = (QRgb *)outImage->scanLine(y);
               for (int i = 0; i < clip.width(); ++i) {
                  int k = in[3];
                  *out++ = qRgb(k * in[0] / 255, k * in[1] / 255,
                        k * in[2] / 255);
                  in += 4;
               }
            } else if (info->output_components == 1) {
               // Grayscale.
               memcpy(outImage->scanLine(y),
                  rows[0] + clip.x(), clip.width());
            }
         }
      } else {
         // Load unclipped grayscale data directly into the QImage.
         (void) jpeg_start_decompress(info);
         while (info->output_scanline < info->output_height) {
            uchar *row = outImage->scanLine(info->output_scanline);
            (void) jpeg_read_scanlines(info, &row, 1);
         }
      }

      if (info->output_scanline == info->output_height) {
         (void) jpeg_finish_decompress(info);
      }

      if (info->density_unit == 1) {
         outImage->setDotsPerMeterX(int(100. * info->X_density / 2.54));
         outImage->setDotsPerMeterY(int(100. * info->Y_density / 2.54));
      } else if (info->density_unit == 2) {
         outImage->setDotsPerMeterX(int(100. * info->X_density));
         outImage->setDotsPerMeterY(int(100. * info->Y_density));
      }

      if (scaledSize.isValid() && scaledSize != clip.size()) {
         *outImage = outImage->scaled(scaledSize, Qt::IgnoreAspectRatio,
               quality >= HIGH_QUALITY_THRESHOLD ? Qt::SmoothTransformation : Qt::FastTransformation);
      }

      if (!scaledClipRect.isEmpty()) {
         *outImage = outImage->copy(scaledClipRect);
      }
      return !outImage->isNull();
   } else {
      return false;
   }
}

struct my_jpeg_destination_mgr : public jpeg_destination_mgr {
   // Nothing dynamic - cannot rely on destruction over longjump
   QIODevice *device;
   JOCTET buffer[max_buf];

 public:
   my_jpeg_destination_mgr(QIODevice *);
};


extern "C" {
   static void qt_init_destination(j_compress_ptr)
   {
   }

   static boolean qt_empty_output_buffer(j_compress_ptr cinfo)
   {
      my_jpeg_destination_mgr *dest = (my_jpeg_destination_mgr *)cinfo->dest;

      int written = dest->device->write((char *)dest->buffer, max_buf);
      if (written == -1) {
         (*cinfo->err->error_exit)((j_common_ptr)cinfo);
      }

      dest->next_output_byte = dest->buffer;
      dest->free_in_buffer = max_buf;

      return TRUE;
   }

   static void qt_term_destination(j_compress_ptr cinfo)
   {
      my_jpeg_destination_mgr *dest = (my_jpeg_destination_mgr *)cinfo->dest;
      qint64 n = max_buf - dest->free_in_buffer;

      qint64 written = dest->device->write((char *)dest->buffer, n);
      if (written == -1) {
         (*cinfo->err->error_exit)((j_common_ptr)cinfo);
      }
   }

}

inline my_jpeg_destination_mgr::my_jpeg_destination_mgr(QIODevice *device)
{
   jpeg_destination_mgr::init_destination = qt_init_destination;
   jpeg_destination_mgr::empty_output_buffer = qt_empty_output_buffer;
   jpeg_destination_mgr::term_destination = qt_term_destination;
   this->device = device;
   next_output_byte = buffer;
   free_in_buffer = max_buf;
}

static inline void set_text(const QImage &image, j_compress_ptr cinfo, const QString &description)
{
   QMap<QString, QString> text;
   for (const QString &key : image.textKeys()) {
      if (!key.isEmpty()) {
         text.insert(key, image.text(key));
      }
   }

   for (const QString &pair : description.split(QLatin1String("\n\n"))) {
      int index = pair.indexOf(QLatin1Char(':'));
      if (index >= 0 && pair.indexOf(QLatin1Char(' ')) < index) {
         QString s = pair.simplified();
         if (!s.isEmpty()) {
            text.insert(QLatin1String("Description"), s);
         }
      } else {
         QString key = pair.left(index);
         if (!key.simplified().isEmpty()) {
            text.insert(key, pair.mid(index + 2).simplified());
         }
      }
   }
   if (text.isEmpty()) {
      return;
   }

   for (QMap<QString, QString>::const_iterator it = text.constBegin(); it != text.constEnd(); ++it) {
      QByteArray comment = it.key().toLatin1();
      if (!comment.isEmpty()) {
         comment += ": ";
      }
      comment += it.value().toLatin1();
      if (comment.length() > 65530) {
         comment.truncate(65530);
      }
      jpeg_write_marker(cinfo, JPEG_COM, (const JOCTET *)comment.constData(), comment.size());
   }
}

static bool write_jpeg_image(const QImage &image, QIODevice *device, volatile int sourceQuality,
   const QString &description, bool optimize, bool progressive)
{
   bool success = false;
   const QVector<QRgb> cmap = image.colorTable();

   if (image.format() == QImage::Format_Invalid || image.format() == QImage::Format_Alpha8) {
      return false;
   }
   struct jpeg_compress_struct cinfo;
   JSAMPROW row_pointer[1];
   row_pointer[0] = 0;

   struct my_jpeg_destination_mgr *iod_dest = new my_jpeg_destination_mgr(device);
   struct my_error_mgr jerr;

   cinfo.err = jpeg_std_error(&jerr);
   jerr.error_exit = my_error_exit;
   jerr.output_message = my_output_message;

   if (!setjmp(jerr.setjmp_buffer)) {
      // WARNING:
      // this if loop is inside a setjmp/longjmp branch
      // do not create C++ temporaries here because the destructor may never be called
      // if you allocate memory, make sure that you can free it (row_pointer[0])
      jpeg_create_compress(&cinfo);

      cinfo.dest = iod_dest;

      cinfo.image_width = image.width();
      cinfo.image_height = image.height();

      bool gray = false;
      switch (image.format()) {
         case QImage::Format_Mono:
         case QImage::Format_MonoLSB:
         case QImage::Format_Indexed8:
            gray = true;
            for (int i = image.colorCount(); gray && i; i--) {
               gray = gray & qIsGray(cmap[i - 1]);
            }
            cinfo.input_components = gray ? 1 : 3;
            cinfo.in_color_space = gray ? JCS_GRAYSCALE : JCS_RGB;
            break;

         case QImage::Format_Grayscale8:
            gray = true;
            cinfo.input_components = 1;
            cinfo.in_color_space = JCS_GRAYSCALE;
            break;

         default:
            cinfo.input_components = 3;
            cinfo.in_color_space = JCS_RGB;
      }

      jpeg_set_defaults(&cinfo);

      qreal diffInch = qAbs(image.dotsPerMeterX() * 2.54 / 100. - qRound(image.dotsPerMeterX() * 2.54 / 100.))
         + qAbs(image.dotsPerMeterY() * 2.54 / 100. - qRound(image.dotsPerMeterY() * 2.54 / 100.));
      qreal diffCm = (qAbs(image.dotsPerMeterX() / 100. - qRound(image.dotsPerMeterX() / 100.))
            + qAbs(image.dotsPerMeterY() / 100. - qRound(image.dotsPerMeterY() / 100.))) * 2.54;
      if (diffInch < diffCm) {
         cinfo.density_unit = 1; // dots/inch
         cinfo.X_density = qRound(image.dotsPerMeterX() * 2.54 / 100.);
         cinfo.Y_density = qRound(image.dotsPerMeterY() * 2.54 / 100.);
      } else {
         cinfo.density_unit = 2; // dots/cm
         cinfo.X_density = (image.dotsPerMeterX() + 50) / 100;
         cinfo.Y_density = (image.dotsPerMeterY() + 50) / 100;
      }

      if (optimize) {
         cinfo.optimize_coding = true;
      }

      if (progressive) {
         jpeg_simple_progression(&cinfo);
      }

      int quality = sourceQuality >= 0 ? qMin(sourceQuality, 100) : 75;
      jpeg_set_quality(&cinfo, quality, TRUE /* limit to baseline-JPEG values */);
      jpeg_start_compress(&cinfo, TRUE);
      set_text(image, &cinfo, description);

      row_pointer[0] = new uchar[cinfo.image_width * cinfo.input_components];
      int w = cinfo.image_width;

      while (cinfo.next_scanline < cinfo.image_height) {
         uchar *row = row_pointer[0];

         switch (image.format()) {
            case QImage::Format_Mono:
            case QImage::Format_MonoLSB:
               if (gray) {
                  const uchar *data = image.constScanLine(cinfo.next_scanline);
                  if (image.format() == QImage::Format_MonoLSB) {
                     for (int i = 0; i < w; i++) {
                        bool bit = !!(*(data + (i >> 3)) & (1 << (i & 7)));
                        row[i] = qRed(cmap[bit]);
                     }
                  } else {
                     for (int i = 0; i < w; i++) {
                        bool bit = !!(*(data + (i >> 3)) & (1 << (7 - (i & 7))));
                        row[i] = qRed(cmap[bit]);
                     }
                  }
               } else {
                  const uchar *data = image.constScanLine(cinfo.next_scanline);
                  if (image.format() == QImage::Format_MonoLSB) {
                     for (int i = 0; i < w; i++) {
                        bool bit = !!(*(data + (i >> 3)) & (1 << (i & 7)));
                        *row++ = qRed(cmap[bit]);
                        *row++ = qGreen(cmap[bit]);
                        *row++ = qBlue(cmap[bit]);
                     }
                  } else {
                     for (int i = 0; i < w; i++) {
                        bool bit = !!(*(data + (i >> 3)) & (1 << (7 - (i & 7))));
                        *row++ = qRed(cmap[bit]);
                        *row++ = qGreen(cmap[bit]);
                        *row++ = qBlue(cmap[bit]);
                     }
                  }
               }
               break;
            case QImage::Format_Indexed8:
               if (gray) {
                  const uchar *pix = image.constScanLine(cinfo.next_scanline);
                  for (int i = 0; i < w; i++) {
                     *row = qRed(cmap[*pix]);
                     ++row;
                     ++pix;
                  }
               } else {
                  const uchar *pix = image.constScanLine(cinfo.next_scanline);
                  for (int i = 0; i < w; i++) {
                     *row++ = qRed(cmap[*pix]);
                     *row++ = qGreen(cmap[*pix]);
                     *row++ = qBlue(cmap[*pix]);
                     ++pix;
                  }
               }
               break;
            case QImage::Format_Grayscale8:
               memcpy(row, image.constScanLine(cinfo.next_scanline), w);
               break;
            case QImage::Format_RGB888:
               memcpy(row, image.constScanLine(cinfo.next_scanline), w * 3);
               break;
            case QImage::Format_RGB32:
            case QImage::Format_ARGB32:
            case QImage::Format_ARGB32_Premultiplied: {
               const QRgb *rgb = (const QRgb *)image.constScanLine(cinfo.next_scanline);
               for (int i = 0; i < w; i++) {
                  *row++ = qRed(*rgb);
                  *row++ = qGreen(*rgb);
                  *row++ = qBlue(*rgb);
                  ++rgb;
               }
            }
            break;
            default: {
               // (Testing shows that this way is actually faster than converting to RGB888 + memcpy)
               QImage rowImg = image.copy(0, cinfo.next_scanline, w, 1).convertToFormat(QImage::Format_RGB32);
               const QRgb *rgb = (const QRgb *)rowImg.constScanLine(0);
               for (int i = 0; i < w; i++) {
                  *row++ = qRed(*rgb);
                  *row++ = qGreen(*rgb);
                  *row++ = qBlue(*rgb);
                  ++rgb;
               }
            }
            break;
         }
         jpeg_write_scanlines(&cinfo, row_pointer, 1);
      }

      jpeg_finish_compress(&cinfo);
      jpeg_destroy_compress(&cinfo);
      success = true;
   } else {
      jpeg_destroy_compress(&cinfo);
      success = false;
   }

   delete iod_dest;
   delete [] row_pointer[0];
   return success;
}

class QJpegHandlerPrivate
{
 public:
   enum State {
      Ready,
      ReadHeader,
      ReadingEnd,
      Error
   };

   QJpegHandlerPrivate(QJpegHandler *qq)
      : quality(75), transformation(QImageIOHandler::TransformationNone), iod_src(0),
        rgb888ToRgb32ConverterPtr(qt_convert_rgb888_to_rgb32), state(Ready), optimize(false), progressive(false), q(qq)
   {}

   ~QJpegHandlerPrivate() {
      if (iod_src) {
         jpeg_destroy_decompress(&info);
         delete iod_src;
         iod_src = 0;
      }
   }

   bool readJpegHeader(QIODevice *);
   bool read(QImage *image);

   int quality;
   QImageIOHandler::Transformations transformation;
   QVariant size;
   QImage::Format format;
   QSize scaledSize;
   QRect scaledClipRect;
   QRect clipRect;
   QString description;
   QStringList readTexts;
   struct jpeg_decompress_struct info;
   struct my_jpeg_source_mgr *iod_src;
   struct my_error_mgr err;

   Rgb888ToRgb32Converter rgb888ToRgb32ConverterPtr;
   State state;

   bool optimize;
   bool progressive;
   QJpegHandler *q;
};

static bool readExifHeader(QDataStream &stream)
{
   char prefix[6];
   if (stream.readRawData(prefix, sizeof(prefix)) != sizeof(prefix)) {
      return false;
   }
   static const char exifMagic[6] = {'E', 'x', 'i', 'f', 0, 0};
   return memcmp(prefix, exifMagic, 6) == 0;
}
/*
 * Returns -1 on error
 * Returns 0 if no Exif orientation was found
 * Returns 1 orientation is horizontal (normal)
 * Returns 2 mirror horizontal
 * Returns 3 rotate 180
 * Returns 4 mirror vertical
 * Returns 5 mirror horizontal and rotate 270 CCW
 * Returns 6 rotate 90 CW
 * Returns 7 mirror horizontal and rotate 90 CW
 * Returns 8 rotate 270 CW
 */
static int getExifOrientation(QByteArray &exifData)
{
   // Current EXIF version (2.3) says there can be at most 5 IFDs,
   // byte we allow for 10 so we're able to deal with future extensions.
   const int maxIfdCount = 10;

   QDataStream stream(&exifData, QIODevice::ReadOnly);

   if (!readExifHeader(stream)) {
      return -1;
   }

   quint16 val;
   quint32 offset;
   const qint64 headerStart = 6;   // the EXIF header has a constant size
   Q_ASSERT(headerStart == stream.device()->pos());

   // read byte order marker
   stream >> val;
   if (val == 0x4949) { // 'II' == Intel
      stream.setByteOrder(QDataStream::LittleEndian);
   } else if (val == 0x4d4d) { // 'MM' == Motorola
      stream.setByteOrder(QDataStream::BigEndian);
   } else {
      return -1;   // unknown byte order
   }

   // confirm byte order
   stream >> val;
   if (val != 0x2a) {
      return -1;
   }

   stream >> offset;

   // read IFD
   for (int n = 0; n < maxIfdCount; ++n) {
      quint16 numEntries;

      const qint64 bytesToSkip = offset - (stream.device()->pos() - headerStart);
      if (bytesToSkip < 0 || (offset + headerStart >= exifData.size())) {
         // disallow going backwards, though it's permitted in the spec
         return -1;
      } else if (bytesToSkip != 0) {
         // seek to the IFD
         if (!stream.device()->seek(offset + headerStart)) {
            return -1;
         }
      }

      stream >> numEntries;

      for (; numEntries > 0 && stream.status() == QDataStream::Ok; --numEntries) {
         quint16 tag;
         quint16 type;
         quint32 components;
         quint16 value;
         quint16 dummy;

         stream >> tag >> type >> components >> value >> dummy;
         if (tag == 0x0112) { // Tag Exif.Image.Orientation
            if (components != 1) {
               return -1;
            }
            if (type != 3) { // we are expecting it to be an unsigned short
               return -1;
            }
            if (value < 1 || value > 8) { // check for valid range
               return -1;
            }

            // It is possible to include the orientation multiple times.
            // Right now the first value is returned.
            return value;
         }
      }

      // read offset to next IFD
      stream >> offset;
      if (stream.status() != QDataStream::Ok) {
         return -1;
      }
      if (offset == 0) { // this is the last IFD
         return 0;   // No Exif orientation was found
      }
   }

   // too many IFDs
   return -1;
}

static QImageIOHandler::Transformations exif2Qt(int exifOrientation)
{
   switch (exifOrientation) {
      case 1: // normal
         return QImageIOHandler::TransformationNone;
      case 2: // mirror horizontal
         return QImageIOHandler::TransformationMirror;
      case 3: // rotate 180
         return QImageIOHandler::TransformationRotate180;
      case 4: // mirror vertical
         return QImageIOHandler::TransformationFlip;
      case 5: // mirror horizontal and rotate 270 CW
         return QImageIOHandler::TransformationFlipAndRotate90;
      case 6: // rotate 90 CW
         return QImageIOHandler::TransformationRotate90;
      case 7: // mirror horizontal and rotate 90 CW
         return QImageIOHandler::TransformationMirrorAndRotate90;
      case 8: // rotate 270 CW
         return QImageIOHandler::TransformationRotate270;
   }
   qWarning("Invalid EXIF orientation");
   return QImageIOHandler::TransformationNone;
}

/*!
    \internal
*/
bool QJpegHandlerPrivate::readJpegHeader(QIODevice *device)
{
   if (state == Ready) {
      state = Error;
      iod_src = new my_jpeg_source_mgr(device);


      info.err = jpeg_std_error(&err);
      err.error_exit = my_error_exit;
      err.output_message = my_output_message;

      jpeg_create_decompress(&info);
      info.src = iod_src;

      if (!setjmp(err.setjmp_buffer)) {
         jpeg_save_markers(&info, JPEG_COM, 0xFFFF);
         jpeg_save_markers(&info, JPEG_APP0 + 1, 0xFFFF); // Exif uses APP1 marker
         (void) jpeg_read_header(&info, TRUE);

         int width = 0;
         int height = 0;
         read_jpeg_size(width, height, &info);
         size = QSize(width, height);

         format = QImage::Format_Invalid;
         read_jpeg_format(format, &info);
         QByteArray exifData;
         for (jpeg_saved_marker_ptr marker = info.marker_list; marker != NULL; marker = marker->next) {
            if (marker->marker == JPEG_COM) {
               QString key, value;
               QString s = QString::fromLatin1((const char *)marker->data, marker->data_length);
               int index = s.indexOf(QLatin1String(": "));
               if (index == -1 || s.indexOf(QLatin1Char(' ')) < index) {
                  key = QLatin1String("Description");
                  value = s;
               } else {
                  key = s.left(index);
                  value = s.mid(index + 2);
               }
               if (!description.isEmpty()) {
                  description += QLatin1String("\n\n");
               }
               description += key + QLatin1String(": ") + value.simplified();
               readTexts.append(key);
               readTexts.append(value);
            } else if (marker->marker == JPEG_APP0 + 1) {
               exifData.append((const char *)marker->data, marker->data_length);
            }
         }
         if (!exifData.isEmpty()) {
            int exifOrientation = getExifOrientation(exifData);
            if (exifOrientation > 0) {
               transformation = exif2Qt(exifOrientation);
            }
         }
         state = ReadHeader;
         return true;
      } else {
         return false;
      }
   } else if (state == Error) {
      return false;
   }

   return true;
}

bool QJpegHandlerPrivate::read(QImage *image)
{
   if (state == Ready) {
      readJpegHeader(q->device());
   }

   if (state == ReadHeader) {
      bool success = read_jpeg_image(image, scaledSize, scaledClipRect, clipRect, quality, rgb888ToRgb32ConverterPtr, &info, &err);
      if (success) {
         for (int i = 0; i < readTexts.size() - 1; i += 2) {
            image->setText(readTexts.at(i), readTexts.at(i + 1));
         }
         state = ReadingEnd;
         return true;
      }

      state = Error;
   }

   return false;

}

Q_GUI_EXPORT void qt_convert_rgb888_to_rgb32_neon(quint32 *dst, const uchar *src, int len);
Q_GUI_EXPORT void qt_convert_rgb888_to_rgb32_ssse3(quint32 *dst, const uchar *src, int len);
extern "C" void qt_convert_rgb888_to_rgb32_mips_dspr2_asm(quint32 *dst, const uchar *src, int len);
QJpegHandler::QJpegHandler()
   : d(new QJpegHandlerPrivate(this))
{

#if defined(__ARM_NEON__)
   // from qimage_neon.cpp
   if (qCpuHasFeature(NEON)) {
      d->rgb888ToRgb32ConverterPtr = qt_convert_rgb888_to_rgb32_neon;
   }
#endif

#if defined(QT_COMPILER_SUPPORTS_SSSE3)
   // from qimage_ssse3.cpps
   if (qCpuHasFeature(SSSE3)) {
      d->rgb888ToRgb32ConverterPtr = qt_convert_rgb888_to_rgb32_ssse3;
   }
#endif

#if defined(QT_COMPILER_SUPPORTS_MIPS_DSPR2)
   if (qCpuHasFeature(DSPR2)) {
      d->rgb888ToRgb32ConverterPtr = qt_convert_rgb888_to_rgb32_mips_dspr2_asm;
   }
#endif
}

QJpegHandler::~QJpegHandler()
{
   delete d;
}

bool QJpegHandler::canRead() const
{
   if (d->state == QJpegHandlerPrivate::Ready && !canRead(device())) {
      return false;
   }

   if (d->state != QJpegHandlerPrivate::Error && d->state != QJpegHandlerPrivate::ReadingEnd) {
      setFormat("jpeg");
      return true;
   }

   return false;
}

bool QJpegHandler::canRead(QIODevice *device)
{
   if (!device) {
      qWarning("QJpegHandler::canRead() called with no device");
      return false;
   }

   char buffer[2];
   if (device->peek(buffer, 2) != 2) {
      return false;
   }
   return uchar(buffer[0]) == 0xff && uchar(buffer[1]) == 0xd8;
}

bool QJpegHandler::read(QImage *image)
{
   if (! canRead()) {
      return false;
   }
   return d->read(image);
}

extern void qt_imageTransform(QImage &src, QImageIOHandler::Transformations orient);
bool QJpegHandler::write(const QImage &image)
{
   if (d->transformation != QImageIOHandler::TransformationNone) {
      // We don't support writing EXIF headers so apply the transform to the data.
      QImage img = image;
      qt_imageTransform(img, d->transformation);
      return write_jpeg_image(img, device(), d->quality, d->description, d->optimize, d->progressive);
   }
   return write_jpeg_image(image, device(), d->quality, d->description, d->optimize, d->progressive);
}

bool QJpegHandler::supportsOption(ImageOption option) const
{
   return option == Quality
      || option == ScaledSize
      || option == ScaledClipRect
      || option == ClipRect
      || option == Description
      || option == Size
      || option == ImageFormat
      || option == OptimizedWrite
      || option == ProgressiveScanWrite
      || option == ImageTransformation;
}

QVariant QJpegHandler::option(ImageOption option) const
{
   switch (option) {
      case Quality:
         return d->quality;
      case ScaledSize:
         return d->scaledSize;
      case ScaledClipRect:
         return d->scaledClipRect;
      case ClipRect:
         return d->clipRect;
      case Description:
         d->readJpegHeader(device());
         return d->description;
      case Size:
         d->readJpegHeader(device());
         return d->size;
      case ImageFormat:
         d->readJpegHeader(device());
         return d->format;
      case OptimizedWrite:
         return d->optimize;
      case ProgressiveScanWrite:
         return d->progressive;
      case ImageTransformation:
         d->readJpegHeader(device());
         return int(d->transformation);
      default:
         break;
   }
   return QVariant();
}

void QJpegHandler::setOption(ImageOption option, const QVariant &value)
{
   switch (option) {
      case Quality:
         d->quality = value.toInt();
         break;
      case ScaledSize:
         d->scaledSize = value.toSize();
         break;
      case ScaledClipRect:
         d->scaledClipRect = value.toRect();
         break;
      case ClipRect:
         d->clipRect = value.toRect();
         break;
      case Description:
         d->description = value.toString();
         break;
      case OptimizedWrite:
         d->optimize = value.toBool();
         break;
      case ProgressiveScanWrite:
         d->progressive = value.toBool();
         break;
      case ImageTransformation: {
         int transformation = value.toInt();
         if (transformation > 0 && transformation < 8) {
            d->transformation = QImageIOHandler::Transformations(transformation);
         }
      }
      default:
         break;
   }
}

QByteArray QJpegHandler::name() const
{
   return "jpeg";
}


