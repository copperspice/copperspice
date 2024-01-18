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

#include <qdebug.h>
#include <qfile.h>
#include <qelapsedtimer.h>

#include <dsvideo_devicecontrol.h>
#include <dscamera_session.h>

#include <tchar.h>
#include <dshow.h>
#include <objbase.h>
#include <initguid.h>
#include <ocidl.h>
#include <string.h>

extern const CLSID CLSID_VideoInputDeviceCategory;

static QList<DSVideoDeviceInfo> *deviceList()
{
   static QList<DSVideoDeviceInfo> retval;
   return &retval;
}

DSVideoDeviceControl::DSVideoDeviceControl(QObject *parent)
   : QVideoDeviceSelectorControl(parent)
{
   m_session = dynamic_cast<DSCameraSession *>(parent);
   selected = 0;
}

int DSVideoDeviceControl::deviceCount() const
{
   updateDevices();
   return deviceList()->count();
}

QString DSVideoDeviceControl::deviceName(int index) const
{
   updateDevices();

   if (index >= 0 && index <= deviceList()->count()) {
      return QString::fromUtf8(deviceList()->at(index).first.constData());
   }

   return QString();
}

QString DSVideoDeviceControl::deviceDescription(int index) const
{
   updateDevices();

   if (index >= 0 && index <= deviceList()->count()) {
      return deviceList()->at(index).second;
   }

   return QString();
}

int DSVideoDeviceControl::defaultDevice() const
{
   return 0;
}

int DSVideoDeviceControl::selectedDevice() const
{
   return selected;
}

void DSVideoDeviceControl::setSelectedDevice(int index)
{
   updateDevices();

   if (index >= 0 && index < deviceList()->count()) {
      if (m_session) {
         QString device = deviceList()->at(index).first;

         if (device.startsWith("ds:")) {
            device.remove(0, 3);
         }

         m_session->setDevice(device);
      }

      selected = index;
   }
}

const QList<DSVideoDeviceInfo> &DSVideoDeviceControl::availableDevices()
{
   updateDevices();
   return *deviceList();
}

void DSVideoDeviceControl::updateDevices()
{
   static QElapsedTimer timer;
   if (timer.isValid() && timer.elapsed() < 500) { // ms
      return;
   }

   deviceList()->clear();

   ICreateDevEnum *pDevEnum = nullptr;
   IEnumMoniker *pEnum = nullptr;

   // Create the System device enumerator
   HRESULT hr = CoCreateInstance(CLSID_SystemDeviceEnum, nullptr,
                                 CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, reinterpret_cast<void **>(&pDevEnum));

   if (SUCCEEDED(hr)) {
      // Create the enumerator for the video capture category
      hr = pDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnum, 0);

      if (S_OK == hr) {
         pEnum->Reset();

         // find all video capture devices
         IMoniker *pMoniker = nullptr;
         IMalloc *mallocInterface = nullptr;
         CoGetMalloc(1, (LPMALLOC *)&mallocInterface);

         while (pEnum->Next(1, &pMoniker, nullptr) == S_OK) {
            BSTR strName = nullptr;
            hr = pMoniker->GetDisplayName(nullptr, nullptr, &strName);

            if (SUCCEEDED(hr)) {
               std::wstring tmp(strName);
               QString output = QString::fromStdWString(tmp);

               mallocInterface->Free(strName);

               DSVideoDeviceInfo devInfo;
               devInfo.first = output;

               IPropertyBag *pPropBag;
               hr = pMoniker->BindToStorage(nullptr, nullptr, IID_IPropertyBag, (void **)(&pPropBag));

               if (SUCCEEDED(hr)) {
                  // Find the description
                  VARIANT varName;
                  varName.vt = VT_BSTR;
                  hr = pPropBag->Read(L"FriendlyName", &varName, nullptr);

                  if (SUCCEEDED(hr)) {
                     std::wstring tmp(varName.bstrVal);
                     output = QString::fromStdWString(tmp);
                  }

                  pPropBag->Release();
               }

               devInfo.second = output;

               deviceList()->append(devInfo);
            }

            pMoniker->Release();
         }

         mallocInterface->Release();
         pEnum->Release();
      }

      pDevEnum->Release();
   }

   timer.restart();
}

