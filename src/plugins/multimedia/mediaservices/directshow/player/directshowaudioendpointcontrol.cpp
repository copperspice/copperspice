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

#include <directshowaudioendpointcontrol.h>

#include <dsplayer_global.h>
#include <directshowplayerservice.h>

DirectShowAudioEndpointControl::DirectShowAudioEndpointControl(
   DirectShowPlayerService *service, QObject *parent)
   : QAudioOutputSelectorControl(parent), m_service(service),
     m_bindContext(nullptr), m_deviceEnumerator(nullptr)
{
   if (CreateBindCtx(0, &m_bindContext) == S_OK) {
      m_deviceEnumerator = com_new<ICreateDevEnum>(CLSID_SystemDeviceEnum);

      updateEndpoints();

      setActiveOutput(m_defaultEndpoint);
   }
}

DirectShowAudioEndpointControl::~DirectShowAudioEndpointControl()
{
   for (IMoniker *moniker : m_devices) {
      moniker->Release();
   }

   if (m_bindContext) {
      m_bindContext->Release();
   }

   if (m_deviceEnumerator) {
      m_deviceEnumerator->Release();
   }
}

QList<QString> DirectShowAudioEndpointControl::availableOutputs() const
{
   return m_devices.keys();
}

QString DirectShowAudioEndpointControl::outputDescription(const QString &name) const
{
#ifdef __IPropertyBag_INTERFACE_DEFINED__
   QString description;

   if (IMoniker *moniker = m_devices.value(name, nullptr)) {
      IPropertyBag *propertyBag = nullptr;

      if (SUCCEEDED(moniker->BindToStorage(nullptr, nullptr, IID_IPropertyBag, reinterpret_cast<void **>(&propertyBag)))) {
         VARIANT name;
         VariantInit(&name);

         if (SUCCEEDED(propertyBag->Read(L"FriendlyName", &name, nullptr))) {
            description = QString::fromStdWString(std::wstring(name.bstrVal));
         }

         VariantClear(&name);
         propertyBag->Release();
      }
   }

   return description;
#else
   return name.section('\\', -1);
#endif
}

QString DirectShowAudioEndpointControl::defaultOutput() const
{
   return m_defaultEndpoint;
}

QString DirectShowAudioEndpointControl::activeOutput() const
{
   return m_activeEndpoint;
}

void DirectShowAudioEndpointControl::setActiveOutput(const QString &name)
{
   if (m_activeEndpoint == name) {
      return;
   }

   if (IMoniker *moniker = m_devices.value(name, nullptr)) {
      IBaseFilter *filter = nullptr;

      if (moniker->BindToObject(m_bindContext, nullptr, IID_IBaseFilter, reinterpret_cast<void **>(&filter)) == S_OK) {
         m_service->setAudioOutput(filter);

         filter->Release();
      }
   }
}

void DirectShowAudioEndpointControl::updateEndpoints()
{
   IMalloc *oleMalloc = nullptr;

   if (m_deviceEnumerator && CoGetMalloc(1, &oleMalloc) == S_OK) {
      IEnumMoniker *monikers = nullptr;

      if (m_deviceEnumerator->CreateClassEnumerator(CLSID_AudioRendererCategory, &monikers, 0) == S_OK) {

         for (IMoniker *moniker = nullptr; monikers->Next(1, &moniker, nullptr) == S_OK; moniker->Release()) {
            OLECHAR *string = nullptr;

            if (moniker->GetDisplayName(m_bindContext, nullptr, &string) == S_OK) {
               QString deviceId = QString::fromStdWString(std::wstring(string));
               oleMalloc->Free(string);

               moniker->AddRef();
               m_devices.insert(deviceId, moniker);

               if (m_defaultEndpoint.isEmpty() || deviceId.endsWith("Default DirectSound Device")) {
                  m_defaultEndpoint = deviceId;
               }
            }
         }
         monikers->Release();
      }
      oleMalloc->Release();
   }
}
