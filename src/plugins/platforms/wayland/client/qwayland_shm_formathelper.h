/***********************************************************************
*
* Copyright (c) 2012-2025 Barbara Geller
* Copyright (c) 2012-2025 Ansel Sermersheim
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

#ifndef QWAYLAND_SHM_FORMATHELPER_H
#define QWAYLAND_SHM_FORMATHELPER_H

#include <qimage.h>
#include <qvector.h>

#include <wayland-client-protocol.h>

class QWaylandShmFormatHelper
{
 public:
   static inline wl_shm_format fromQImageFormat(QImage::Format format);
   static inline QImage::Format fromWaylandShmFormat(wl_shm_format format);
   static inline QVector<wl_shm_format> supportedWaylandFormats();

 private:
   struct Array {
      Array(const size_t size, const wl_shm_format *data)
         : size(size), data(data)
      { }

      const size_t size;
      const wl_shm_format *data;
   };

   static const Array getData() {
      static wl_shm_format formats_array[] = {
         wl_shm_format(INT_MIN),    //Format_Invalid,
         wl_shm_format(INT_MIN),    //Format_Mono,
         wl_shm_format(INT_MIN),    //Format_MonoLSB,
         wl_shm_format(INT_MIN),    //Format_Indexed8,
         WL_SHM_FORMAT_XRGB8888,    //Format_RGB32,
         WL_SHM_FORMAT_ARGB8888,    //Format_ARGB32,
         WL_SHM_FORMAT_ARGB8888,    //Format_ARGB32_Premultiplied,
         WL_SHM_FORMAT_RGB565,      //Format_RGB16,
         wl_shm_format(INT_MIN),    //Format_ARGB8565_Premultiplied,
         wl_shm_format(INT_MIN),    //Format_RGB666,
         wl_shm_format(INT_MIN),    //Format_ARGB6666_Premultiplied,
         WL_SHM_FORMAT_XRGB1555,    //Format_RGB555,
         wl_shm_format(INT_MIN),    //Format_ARGB8555_Premultiplied,
         WL_SHM_FORMAT_RGB888,      //Format_RGB888,
         WL_SHM_FORMAT_XRGB4444,    //Format_RGB444,
         WL_SHM_FORMAT_ARGB4444,    //Format_ARGB4444_Premultiplied,
         WL_SHM_FORMAT_XBGR8888,    //Format_RGBX8888,
         WL_SHM_FORMAT_ABGR8888,    //Format_RGBA8888,
         WL_SHM_FORMAT_ABGR8888,    //Format_RGBA8888_Premultiplied,
         WL_SHM_FORMAT_XBGR2101010, //Format_BGR30,
         WL_SHM_FORMAT_ARGB2101010, //Format_A2BGR30_Premultiplied,
         WL_SHM_FORMAT_XRGB2101010, //Format_RGB30,
         WL_SHM_FORMAT_ARGB2101010, //Format_A2RGB30_Premultiplied,
         WL_SHM_FORMAT_C8,          //Format_Alpha8,
         WL_SHM_FORMAT_C8           //Format_Grayscale8,
      };

      const size_t size = sizeof(formats_array) / sizeof(*formats_array);

      return Array(size, formats_array);
   }
};

wl_shm_format QWaylandShmFormatHelper::fromQImageFormat(QImage::Format format)
{
   Array array = getData();

   if (array.size <= size_t(format)) {
      return wl_shm_format(INT_MIN);
   }

   return array.data[format];
}

QImage::Format QWaylandShmFormatHelper::fromWaylandShmFormat(wl_shm_format format)
{
   Array array = getData();

   for (size_t i = 0; i < array.size; i++) {
      if (array.data[i] == format) {
         return QImage::Format(i);
      }
   }

   return QImage::Format_Invalid;
}

QVector < wl_shm_format > QWaylandShmFormatHelper::supportedWaylandFormats()
{
   QVector<wl_shm_format> retFormats;
   Array array = getData();

   for (size_t i = 0; i < array.size; i++) {
      if (int(array.data[i]) != INT_MIN && ! retFormats.contains(array.data[i])) {
         retFormats.append(array.data[i]);
      }
   }

   return retFormats;
}

#endif
