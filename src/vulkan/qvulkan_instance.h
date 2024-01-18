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

#ifndef QVULKAN_INSTANCE_H
#define QVULKAN_INSTANCE_H

#include <qglobal.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qversionnumber.h>
#include <qwindow.h>

#include <vulkan/vulkan.hpp>

class QVulkanDeviceFunctions;
class QVulkanFunctions;

// equivalent to VkExtensionProperties, with a QString instead of char array
struct QVulkanExtensionProperties
{
   uint32_t extensionVersion;

   QString extensionName;
};
using QExtension [[deprecated("Replace with QVulkanExtensionProperties")]] = QVulkanExtensionProperties;

struct QVulkanLayerProperties
{
   uint32_t specVersion;
   uint32_t implementationVersion;

   QString layerName;
   QString description;
};
using QVulkanLayer [[deprecated("Replace with QVulkanLayerProperties")]] = QVulkanLayerProperties;

template <typename T>
using QDynamicUniqueHandle = vk::UniqueHandle<T, vk::DispatchLoaderDynamic>;

template <typename T, typename U>
QDynamicUniqueHandle<T> cs_makeDynamicUnique(U object, const vk::DispatchLoaderDynamic &dld)
{
   return QDynamicUniqueHandle<T>(object, typename vk::UniqueHandleTraits<T, vk::DispatchLoaderDynamic>::deleter(nullptr, dld));
}

class Q_VULKAN_EXPORT QVulkanInstance
{
 public:
   using DebugFilter = std::function<bool (VkDebugReportFlagsEXT, VkDebugReportObjectTypeEXT, uint64_t, size_t,
      int32_t, const char *, const char *)>;

   enum InstanceOptions : uint32_t {
      NoDebugOutputRedirect = 0x0001
   };

   using InstanceFlags = QFlags<InstanceOptions>;
   using Flags [[deprecated("Replace with QVulkanInstance::InstanceFlags")]] = InstanceFlags;

   QVulkanInstance();
   ~QVulkanInstance();

   QVersionNumber apiVersion() const;
   bool create();
   void destroy();
   QVulkanDeviceFunctions *deviceFunctions(VkDevice device);

   VkResult errorCode() const;
   QStringList extensions() const;

   InstanceFlags flags() const;
   QVulkanFunctions *functions() const;
   PFN_vkVoidFunction getInstanceProcAddr(const char *name);

   uint32_t installDebugOutputFilter(QVulkanInstance::DebugFilter filter);
   bool isValid() const;
   QStringList layers() const;

   void presentAboutToBeQueued(QWindow *window);
   void presentQueued(QWindow *window);

   void removeDebugOutputFilter(uint32_t filter);
   void resetDeviceFunctions(VkDevice device);

   void setApiVersion(const QVersionNumber &vulkanVersion);
   void setExtensions(const QStringList &extensions);
   void setFlags(InstanceFlags flags);
   void setLayers(const QStringList &layers);
   void setVkInstance(VkInstance existingVkInstance);

   QVector<QVulkanExtensionProperties> supportedExtensions() const;
   QVector<QVulkanLayerProperties> supportedLayers() const;
   QVersionNumber supportedApiVersion() const;

   VkInstance vkInstance() const;

   const vk::Instance &apiInstance() const {
      return m_vkInstance.get();
   }

   const vk::DynamicLoader &dynamicLoader() const {
      return m_dl;
   }

   const vk::DispatchLoaderDynamic &dispatchLoader() const {
      return m_dld;
   }

   static VkSurfaceKHR surfaceForWindow(QWindow *window);

 private:
   QSet<QString> supportedExtensionSet() const;
   QSet<QString> supportedLayerSet() const;
   static QStringList filterStringList(QStringList input, QSet<QString> validStrings);

   static VkBool32 debugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType,
      uint64_t object, size_t location, int32_t messageCode, const char *pLayerPrefix, const char *pMessage, void *pUserData);

   uint32_t m_nextDebugFilterId;
   VkResult m_errorCode;
   InstanceFlags m_flags;
   QVersionNumber m_apiVersion;

   QStringList m_layers;
   QStringList m_extensions;

   vk::DynamicLoader m_dl;
   vk::DispatchLoaderDynamic m_dld;

   QDynamicUniqueHandle<vk::Instance> m_vkInstance;
   QDynamicUniqueHandle<vk::DebugReportCallbackEXT> m_debugCallback;
   QVector< std::pair<uint32_t,DebugFilter> > m_debugFilters;

   QMap<VkDevice, std::shared_ptr<QVulkanDeviceFunctions>> m_deviceFunctions;
   mutable std::shared_ptr<QVulkanFunctions> m_functions;
};

#endif
