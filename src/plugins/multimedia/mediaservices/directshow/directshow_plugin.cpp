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

#include <directshow_plugin.h>

#include <qstring.h>
#include <qdebug.h>
#include <qfile.h>

#include <dshow.h>

#ifdef QMEDIA_DIRECTSHOW_PLAYER
#include <directshowplayerservice.h>
#endif

#ifdef QMEDIA_DIRECTSHOW_CAMERA

#include <dsvideo_devicecontrol.h>
#include <dscamera_service.h>

extern const CLSID CLSID_VideoInputDeviceCategory;

#ifndef _STRSAFE_H_INCLUDED_
#include <tchar.h>
#endif

#include <initguid.h>
#include <objbase.h>

/*
#ifdef Q_CC_MSVC
#  pragma comment(lib, "strmiids.lib")
#  pragma comment(lib, "ole32.lib")
#endif
*/

#include <windows.h>
#include <ocidl.h>
#endif

CS_PLUGIN_REGISTER(DSServicePlugin)

static int g_refCount = 0;

void addRefCount()
{
   if (++g_refCount == 1) {
      CoInitialize(nullptr);
   }
}

void releaseRefCount()
{
   if (--g_refCount == 0) {
      CoUninitialize();
   }
}

QMediaService *DSServicePlugin::create(QString const &key)
{

#ifdef QMEDIA_DIRECTSHOW_CAMERA

   if (key == Q_MEDIASERVICE_CAMERA) {
      addRefCount();
      return new DSCameraService;
   }
#endif

#ifdef QMEDIA_DIRECTSHOW_PLAYER
   if (key == Q_MEDIASERVICE_MEDIAPLAYER) {
      addRefCount();
      return new DirectShowPlayerService;
   }
#endif

   return nullptr;
}

void DSServicePlugin::release(QMediaService *service)
{
   delete service;
   releaseRefCount();
}

QMediaServiceProviderHint::Features DSServicePlugin::supportedFeatures(const QString &service) const
{
   if (service == Q_MEDIASERVICE_MEDIAPLAYER) {
      return QMediaServiceProviderHint::StreamPlayback | QMediaServiceProviderHint::VideoSurface;
   } else {
      return QMediaServiceProviderHint::Features();
   }
}

QString DSServicePlugin::defaultDevice(const QString &service) const
{

#ifdef QMEDIA_DIRECTSHOW_CAMERA
   if (service == Q_MEDIASERVICE_CAMERA) {
      const QList<DSVideoDeviceInfo> &devs = DSVideoDeviceControl::availableDevices();

      if (! devs.isEmpty()) {
         return devs.first().first;
      }
   }
#else
   (void) service;
#endif

   return QString ();
}

QList<QString> DSServicePlugin::devices(const QString &service) const
{
   QList<QString > result;

#ifdef QMEDIA_DIRECTSHOW_CAMERA
   if (service == Q_MEDIASERVICE_CAMERA) {
      const QList<DSVideoDeviceInfo> &devs = DSVideoDeviceControl::availableDevices();

      for (const DSVideoDeviceInfo &item : devs) {
         result.append(item.first);
      }
   }
#else
   (void) service;
#endif

   return result;
}

QString DSServicePlugin::deviceDescription(const QString &service, const QString  &device)
{
#ifdef QMEDIA_DIRECTSHOW_CAMERA
   if (service == Q_MEDIASERVICE_CAMERA) {
      const QList<DSVideoDeviceInfo> &devs = DSVideoDeviceControl::availableDevices();

      for (const DSVideoDeviceInfo &item : devs) {
         if (item.first == device) {
            return item.second;
         }
      }
   }

#else
   (void) service;
   (void) device;
#endif

   return QString();
}
