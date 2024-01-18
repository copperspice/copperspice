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

#ifndef AVFCAMERAUTILITY_H
#define AVFCAMERAUTILITY_H

#include <qsysinfo.h>
#include <qglobal.h>
#include <qvector.h>
#include <qdebug.h>
#include <qsize.h>
#include <qpair.h>

#include <AVFoundation/AVFoundation.h>

class AVFConfigurationLock
{
 public:
   explicit AVFConfigurationLock(AVCaptureDevice *captureDevice)
      : m_captureDevice(captureDevice), m_locked(false) {
      Q_ASSERT(m_captureDevice);
      NSError *error = nil;
      m_locked = [m_captureDevice lockForConfiguration: &error];
   }

   AVFConfigurationLock(const AVFConfigurationLock &) = delete;
   AVFConfigurationLock &operator=(const AVFConfigurationLock &) = delete;

   ~AVFConfigurationLock() {
      if (m_locked) {
         [m_captureDevice unlockForConfiguration];
      }
   }

   operator bool() const {
      return m_locked;
   }

 private:
   AVCaptureDevice *m_captureDevice;
   bool m_locked;
};

struct AVFObjectDeleter {
   void operator()(NSObject *obj) const {
      if (obj != nullptr) {
         [obj release];
      }
   }
};

template<class T>
class AVFScopedPointer : public QScopedPointer<NSObject, AVFObjectDeleter>
{
 public:
   AVFScopedPointer() = default;

   explicit AVFScopedPointer(T *ptr)
      : QScopedPointer<NSObject, AVFObjectDeleter>(ptr)
   {
   }

   operator T *() const {
      return data();
   }

   T *data() const {
      return static_cast<T *>(QScopedPointer<NSObject, AVFObjectDeleter>::data());
   }

   T *take() {
      return static_cast<T *>(QScopedPointer<NSObject, AVFObjectDeleter>::take());
   }
};

template<>
class AVFScopedPointer<dispatch_queue_t>
{
 public:
   AVFScopedPointer()
      : m_queue(nullptr)
   {
   }

   explicit AVFScopedPointer(dispatch_queue_t q)
      : m_queue(q) {
   }

   AVFScopedPointer(const AVFScopedPointer &) = delete;
   AVFScopedPointer &operator=(const AVFScopedPointer &) = delete;

   ~AVFScopedPointer() {
      if (m_queue) {
         dispatch_release(m_queue);
      }
   }

   operator dispatch_queue_t() const {
      // Quite handy operator to enable Obj-C messages: [ptr someMethod];
      return m_queue;
   }

   dispatch_queue_t data() const {
      return m_queue;
   }

   void reset(dispatch_queue_t q = nullptr) {
      if (m_queue) {
         dispatch_release(m_queue);
      }

      m_queue = q;
   }

 private:
   dispatch_queue_t m_queue;
};

inline QSysInfo::MacVersion qt_OS_limit(QSysInfo::MacVersion osxVersion, QSysInfo::MacVersion iosVersion)
{
   (void) iosVersion;
   return osxVersion;
}

typedef QPair<qreal, qreal> AVFPSRange;
AVFPSRange qt_connection_framerates(AVCaptureConnection *videoConnection);

QVector<AVCaptureDeviceFormat *> qt_unique_device_formats(AVCaptureDevice *captureDevice, FourCharCode preferredFormat);
QSize qt_device_format_resolution(AVCaptureDeviceFormat *format);
QSize qt_device_format_high_resolution(AVCaptureDeviceFormat *format);
QSize qt_device_format_pixel_aspect_ratio(AVCaptureDeviceFormat *format);
QVector<AVFPSRange> qt_device_format_framerates(AVCaptureDeviceFormat *format);

AVCaptureDeviceFormat *qt_find_best_resolution_match(AVCaptureDevice *captureDevice, const QSize &res, FourCharCode preferredFormat);

AVCaptureDeviceFormat *qt_find_best_framerate_match(AVCaptureDevice *captureDevice, FourCharCode preferredFormat, Float64 fps);

AVFrameRateRange *qt_find_supported_framerate_range(AVCaptureDeviceFormat *format, Float64 fps);

bool qt_formats_are_equal(AVCaptureDeviceFormat *f1, AVCaptureDeviceFormat *f2);
bool qt_set_active_format(AVCaptureDevice *captureDevice, AVCaptureDeviceFormat *format, bool preserveFps);

AVFPSRange qt_current_framerates(AVCaptureDevice *captureDevice, AVCaptureConnection *videoConnection);
void qt_set_framerate_limits(AVCaptureDevice *captureDevice, AVCaptureConnection *videoConnection, qreal minFPS, qreal maxFPS);

#endif
