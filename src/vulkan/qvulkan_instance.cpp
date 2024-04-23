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

#include <qvulkan_instance.h>
#include <qvulkan_functions.h>
#include <qvulkan_device_functions.h>
#include <qvulkan_window.h>

#include <qapplication.h>
#include <qplatform_window.h>

QVulkanInstance::QVulkanInstance()
   : m_errorCode(VK_SUCCESS)
{
   PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = m_dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
   m_dld.init(vkGetInstanceProcAddr);
}

QVulkanInstance::~QVulkanInstance()
{
}

QVersionNumber QVulkanInstance::apiVersion() const
{
   return m_apiVersion;
}

static const QStringList s_requiredExtensions = {
   "VK_KHR_surface",
   "VK_KHR_win32_surface",
   "VK_KHR_xcb_surface",
   "VK_EXT_debug_report"
};

bool QVulkanInstance::create()
{
   for (const auto &item : s_requiredExtensions) {
      if (! m_extensions.contains(item)) {
         m_extensions.append(item);
      }
   }

   m_extensions = filterStringList(std::move(m_extensions), supportedExtensionSet());
   m_layers     = filterStringList(std::move(m_layers), supportedLayerSet());

   QVector<const char *> layerPointers;
   for (const auto &layerString : m_layers) {
      layerPointers.append(layerString.constData());
   }

   QVector<const char *> extensionPointers;
   for (const auto &extensionString : m_extensions) {
      extensionPointers.append(extensionString.constData());
   }

   QString appName = QApplication::instance()->applicationName();

   vk::InstanceCreateInfo createInfo;
   createInfo.enabledLayerCount       = layerPointers.size();
   createInfo.ppEnabledLayerNames     = layerPointers.constData();
   createInfo.enabledExtensionCount   = extensionPointers.size();
   createInfo.ppEnabledExtensionNames = extensionPointers.constData();

   vk::ApplicationInfo applicationInfo;
   applicationInfo.pApplicationName   = appName.constData();
   applicationInfo.applicationVersion = 0;
   applicationInfo.pEngineName        = "CsVulkan";
   applicationInfo.engineVersion      = 0;
   applicationInfo.apiVersion         = 0;

   createInfo.pApplicationInfo = &applicationInfo;

   m_debugCallback.reset();

   vk::Instance instance;
   vk::Result result = vk::createInstance(&createInfo, nullptr, &instance, m_dld);

   if (result != vk::Result::eSuccess) {
      m_errorCode = static_cast<VkResult>(result);
      return false;
   }

   m_vkInstance = cs_makeDynamicUnique<vk::Instance>(instance, m_dld);

   m_dld.init(*m_vkInstance);

   if (! m_flags.testFlag(NoDebugOutputRedirect)) {
      vk::DebugReportCallbackCreateInfoEXT debugCreateInfo;

      debugCreateInfo.pfnCallback = &debugCallback;
      debugCreateInfo.flags       = vk::DebugReportFlagBitsEXT::eError | vk::DebugReportFlagBitsEXT::eWarning;
      debugCreateInfo.pUserData   = this;

      m_debugCallback = m_vkInstance->createDebugReportCallbackEXTUnique(debugCreateInfo, nullptr, m_dld);
   }

   return true;
}

static QString debugFlagsToString(VkDebugReportFlagsEXT flags)
{
   // default to error message type
   QString retval = "error";

   if (flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) {
      // do nothing

   } else if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT) {
      retval = "warning";

   } else if (flags & VK_DEBUG_REPORT_DEBUG_BIT_EXT) {
      retval = "debug";

   } else if (flags & VK_DEBUG_REPORT_WARNING_BIT_EXT) {
      retval = "warning";

   } else if (flags & VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT) {
      retval = "performance warning";

   } else if (flags & VK_DEBUG_REPORT_INFORMATION_BIT_EXT) {
      retval = "information";

   }

   return retval;
}

VkBool32 QVulkanInstance::debugCallback(VkDebugReportFlagsEXT flags, VkDebugReportObjectTypeEXT objectType, uint64_t object,
   size_t location, int32_t messageCode, const char *pLayerPrefix, const char *pMessage, void *pUserData)
{
   (void) flags;
   QVulkanInstance *self = static_cast<QVulkanInstance*>(pUserData);

   for (auto &filter: self->m_debugFilters) {
      if (filter.second(flags, objectType, object, location, messageCode, pLayerPrefix, pMessage)) {
         return VK_FALSE;
      }
   }

   QString errorType = debugFlagsToString(flags);
   QString errorMessage;

   if (objectType == VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT) {
      errorMessage = QString("Vulkan %1: object type = unknown, location = %2, messageCode = %3, layer = %4, message = %5")
         .formatArg(errorType).formatArg(location).formatArg(messageCode).formatArg(QString::fromUtf8(pLayerPrefix))
         .formatArg(QString::fromUtf8(pMessage));

   } else {
      vk::DebugReportObjectTypeEXT type = static_cast<vk::DebugReportObjectTypeEXT>(objectType);
      QString typeString = QString::fromStdString(to_string(type));

      errorMessage = QString("Vulkan %1: object type = %2, object = %3, location = %4, messageCode = %5, layer = %6, message = %7")
         .formatArg(errorType).formatArg(typeString).formatArg(object).formatArg(location).formatArg(messageCode)
         .formatArg(QString::fromUtf8(pLayerPrefix)).formatArg(QString::fromUtf8(pMessage));
   }

   qWarning("%s", errorMessage.constData());

   return VK_FALSE;
}

void QVulkanInstance::destroy()
{
   m_errorCode = VK_SUCCESS;
   m_debugCallback.reset();
   m_vkInstance.reset();
}

QVulkanDeviceFunctions *QVulkanInstance::deviceFunctions(VkDevice device)
{
   auto iter = m_deviceFunctions.find(device);

   if (iter == m_deviceFunctions.end()) {
      std::shared_ptr<QVulkanDeviceFunctions> tmp(new QVulkanDeviceFunctions(*m_vkInstance, device, m_dld));
      QVulkanDeviceFunctions *retval = tmp.get();

      m_deviceFunctions.insert(device, std::move(tmp));

      return retval;
   }

   return iter->get();
}

QStringList QVulkanInstance::extensions() const
{
   return m_extensions;
}

VkResult QVulkanInstance::errorCode() const
{
   return m_errorCode;
}

QVulkanInstance::InstanceFlags QVulkanInstance::flags() const
{
   return m_flags;
}

QStringList QVulkanInstance::filterStringList(QStringList input, QSet<QString> validStrings)
{
   auto iter = input.begin();

   while (iter != input.end()) {
      if (validStrings.contains(*iter)) {
         ++iter;

      } else {
         iter = input.erase(iter);

      }
   }

   return input;
}

QVulkanFunctions *QVulkanInstance::functions() const
{
   if (m_functions == nullptr) {
      m_functions.reset(new QVulkanFunctions(*m_vkInstance, m_dld));
   }

   return m_functions.get();
}

PFN_vkVoidFunction QVulkanInstance::getInstanceProcAddr(const char *name)
{
   return m_vkInstance->getProcAddr(name, m_dld);
}

uint32_t QVulkanInstance::installDebugOutputFilter(QVulkanInstance::DebugFilter filter)
{
   uint32_t filterId = m_nextDebugFilterId;
   ++m_nextDebugFilterId;

   m_debugFilters.append({filterId, filter});

   return filterId;
}

bool QVulkanInstance::isValid() const
{
   return static_cast<bool>(m_vkInstance);
}

QStringList QVulkanInstance::layers() const
{
   return m_layers;
}

void QVulkanInstance::presentAboutToBeQueued(QWindow *window)
{
   (void) window;

#if defined(Q_OS_WIN)
   // windows platform specific code

#elif defined(Q_OS_UNIX)
   // unix platform specific code

#endif

}

void QVulkanInstance::presentQueued(QWindow *window)
{
   window->handle()->syncIfNeeded();
}

void QVulkanInstance::removeDebugOutputFilter(uint32_t filterId)
{
   auto iter = m_debugFilters.begin();

   while (iter != m_debugFilters.end()) {
      if (iter->first == filterId) {
         iter = m_debugFilters.erase(iter);

      } else {
         ++iter;

      }
   }
}

void QVulkanInstance::resetDeviceFunctions(VkDevice device)
{
   m_deviceFunctions.remove(device);
}

void QVulkanInstance::setApiVersion(const QVersionNumber &version)
{
   m_apiVersion = version;
}

void QVulkanInstance::setFlags(QVulkanInstance::InstanceFlags flags)
{
   m_flags = flags;
}

void QVulkanInstance::setExtensions(const QStringList &extensions)
{
   m_extensions = extensions;
}

void QVulkanInstance::setLayers(const QStringList &layers)
{
   m_layers = layers;
}

void QVulkanInstance::setVkInstance(VkInstance existingVkInstance)
{
   m_vkInstance = cs_makeDynamicUnique<vk::Instance>(existingVkInstance, m_dld);
}

QVersionNumber QVulkanInstance::supportedApiVersion() const {
   static const auto enumerateFunctionPointer =
         reinterpret_cast<PFN_vkEnumerateInstanceVersion>(vkGetInstanceProcAddr(nullptr, "vkEnumerateInstanceVersion"));

   QVersionNumber retval = QVersionNumber(1, 0, 0);

   if (enumerateFunctionPointer == nullptr) {
      return retval;
   }

   uint32_t apiVersion;
   VkResult result = enumerateFunctionPointer(&apiVersion);

   if (result != VK_SUCCESS) {
      return retval;
   }

   retval = QVersionNumber(VK_VERSION_MAJOR(apiVersion), VK_VERSION_MINOR(apiVersion), VK_VERSION_PATCH(apiVersion));

   return retval;
}

QVector<QVulkanExtensionProperties> QVulkanInstance::supportedExtensions() const
{
   uint32_t numExtensions;
   vk::Result result;
   QVector<QVulkanExtensionProperties> retval;
   QVector<vk::ExtensionProperties> properties;

   // can not use dynamic loader before instance is loaded
   result = vk::enumerateInstanceExtensionProperties(nullptr, &numExtensions, properties.data());

   if (result != vk::Result::eSuccess) {
      return retval;
   }

   properties.resize(numExtensions);
   result = vk::enumerateInstanceExtensionProperties(nullptr, &numExtensions, properties.data());

   if (result != vk::Result::eSuccess) {
      return retval;
   }

   for (const auto &item: properties) {
      retval.append({item.specVersion, QString::fromUtf8(item.extensionName)});
   }

   return retval;
}

QSet<QString> QVulkanInstance::supportedExtensionSet() const
{
   QSet<QString> retval;
   QVector<QVulkanExtensionProperties> extensions = supportedExtensions();

   for (auto &item: extensions) {
      retval.insert(std::move(item.extensionName));
   }

   return retval;
}

QVector<QVulkanLayerProperties> QVulkanInstance::supportedLayers() const
{
   uint32_t numLayers;
   vk::Result result;
   QVector<QVulkanLayerProperties> retval;
   QVector<vk::LayerProperties> properties;

   // can not use dynamic loader before instance is loaded
   result = vk::enumerateInstanceLayerProperties(&numLayers, properties.data());

   if (result != vk::Result::eSuccess) {
      return retval;
   }

   properties.resize(numLayers);
   result = vk::enumerateInstanceLayerProperties(&numLayers, properties.data());

   if (result != vk::Result::eSuccess) {
      return retval;
   }

   for (const auto &item: properties) {
      retval.append({item.specVersion, item.implementationVersion,
            QString::fromUtf8(item.layerName), QString::fromUtf8(item.description)});
   }

   return retval;
}

QSet<QString> QVulkanInstance::supportedLayerSet() const
{
   QSet<QString> retval;
   QVector<QVulkanLayerProperties> layers = supportedLayers();

   for (auto &item : layers) {
      retval.insert(std::move(item.layerName));
   }

   return retval;
}

VkSurfaceKHR QVulkanInstance::surfaceForWindow(QWindow *window)
{
   QVulkanWindow *tmp = dynamic_cast<QVulkanWindow*>(window);

   if (tmp == nullptr) {
      return nullptr;
   }

   return tmp->vulkanSurface();
}

VkInstance QVulkanInstance::vkInstance() const
{
   return *m_vkInstance;
}
