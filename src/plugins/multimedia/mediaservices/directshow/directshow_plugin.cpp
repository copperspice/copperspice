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

#include <directshow_plugin.h>

#include <qstring.h>
#include <qdebug.h>
#include <qfile.h>

#include <dshow.h>

#ifdef QMEDIA_DIRECTSHOW_PLAYER
#include <directshowplayerservice.h>
#endif

#ifdef QMEDIA_DIRECTSHOW_CAMERA

#include <dsvideodevicecontrol.h>
#include <dscameraservice.h>

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
      CoInitialize(NULL);
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

QMediaServiceProviderHint::Features DSServicePlugin::supportedFeatures(
   const QByteArray &service) const
{
   if (service == Q_MEDIASERVICE_MEDIAPLAYER) {
      return QMediaServiceProviderHint::StreamPlayback | QMediaServiceProviderHint::VideoSurface;
   } else {
      return QMediaServiceProviderHint::Features();
   }
}

QByteArray DSServicePlugin::defaultDevice(const QByteArray &service) const
{
#ifdef QMEDIA_DIRECTSHOW_CAMERA
   if (service == Q_MEDIASERVICE_CAMERA) {
      const QList<DSVideoDeviceInfo> &devs = DSVideoDeviceControl::availableDevices();
      if (!devs.isEmpty()) {
         return devs.first().first;
      }
   }
#endif

   return QByteArray();
}

QList<QByteArray> DSServicePlugin::devices(const QByteArray &service) const
{
   QList<QByteArray> result;

#ifdef QMEDIA_DIRECTSHOW_CAMERA
   if (service == Q_MEDIASERVICE_CAMERA) {
      const QList<DSVideoDeviceInfo> &devs = DSVideoDeviceControl::availableDevices();

      for (const DSVideoDeviceInfo &info : devs) {
         result.append(info.first);
      }
   }
#endif

   return result;
}

QString DSServicePlugin::deviceDescription(const QByteArray &service, const QByteArray &device)
{
#ifdef QMEDIA_DIRECTSHOW_CAMERA
   if (service == Q_MEDIASERVICE_CAMERA) {
      const QList<DSVideoDeviceInfo> &devs = DSVideoDeviceControl::availableDevices();
      Q_FOREACH (const DSVideoDeviceInfo &info, devs) {
         if (info.first == device) {
            return info.second;
         }
      }
   }
#endif

   return QString();
}
