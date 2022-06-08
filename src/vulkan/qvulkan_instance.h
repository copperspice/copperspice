/***********************************************************************
*
* Copyright (c) 2012-2022 Barbara Geller
* Copyright (c) 2012-2022 Ansel Sermersheim
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

#include <qstring.h>
#include <qglobal.h>

#include <vulkan/vulkan.hpp>

class QVulkanFunctions;

// Equivalent to VkExtensionProperties, with a QString instead of char array
struct QVulkanExtensionProperties
{
   QString extensionName;
   uint32_t extensionVersion;
};
using QExtension [[deprecated("Replace with QVulkanExtensionProperties")]] = QVulkanExtensionProperties;

template<typename T>
using QDynamicUniqueHandle = vk::UniqueHandle<T, vk::DispatchLoaderDynamic>;

template<typename T, typename U>
QDynamicUniqueHandle<T> cs_makeDynamicUnique(U object, const vk::DispatchLoaderDynamic& dld)
{
   return QDynamicUniqueHandle<T>(object, typename vk::UniqueHandleTraits<T, vk::DispatchLoaderDynamic>::deleter(nullptr, dld));
}

class Q_VULKAN_EXPORT QVulkanInstance
{
 public:
   QVulkanInstance();
   ~QVulkanInstance();

   QVulkanFunctions *functions() const;
   VkInstance vkInstance() const;

 private:
   vk::DynamicLoader m_dl;
   vk::DispatchLoaderDynamic m_dld;

   QDynamicUniqueHandle<vk::Instance> m_vkInstance;
   VkResult m_errorCode;
   mutable std::shared_ptr<QVulkanFunctions> m_functions;
};

#endif
