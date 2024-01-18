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

#ifndef QVULKAN_FUNCTIONS_H
#define QVULKAN_FUNCTIONS_H

#include <qglobal.h>

#include <vulkan/vulkan.hpp>

class Q_VULKAN_EXPORT QVulkanFunctions
{
 public:
   VkResult vkCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo *pCreateInfo,
      const VkAllocationCallbacks *pAllocator, VkDevice *pDevice);

   VkResult vkEnumerateDeviceExtensionProperties(VkPhysicalDevice physicalDevice, const char *pLayerName,
      uint32_t *pPropertyCount, VkExtensionProperties *pProperties);

   VkResult vkEnumerateDeviceLayerProperties(VkPhysicalDevice physicalDevice, uint32_t *pPropertyCount,
      VkLayerProperties *pProperties);

   VkResult vkEnumerateInstanceExtensionProperties(const char *pLayerName, uint32_t *pPropertyCount,
      VkExtensionProperties *pProperties);

   VkResult vkEnumerateInstanceLayerProperties(uint32_t *pPropertyCount, VkLayerProperties *pProperties);

   VkResult vkEnumeratePhysicalDevices(VkInstance instance, uint32_t *pPhysicalDeviceCount, VkPhysicalDevice *pPhysicalDevices);

   PFN_vkVoidFunction vkGetDeviceProcAddr(VkDevice device, const char *pName);

   void vkGetPhysicalDeviceFeatures(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures *pFeatures);
   void vkGetPhysicalDeviceFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format, VkFormatProperties *pFormatProperties);

   VkResult vkGetPhysicalDeviceImageFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format,
      VkImageType type, VkImageTiling tiling, VkImageUsageFlags usage, VkImageCreateFlags flags,
      VkImageFormatProperties *pImageFormatProperties);

   void vkGetPhysicalDeviceMemoryProperties(VkPhysicalDevice physicalDevice, VkPhysicalDeviceMemoryProperties *pMemoryProperties);
   void vkGetPhysicalDeviceProperties(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties *pProperties);

   void vkGetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice,
      uint32_t *pQueueFamilyPropertyCount, VkQueueFamilyProperties *pQueueFamilyProperties);

   void vkGetPhysicalDeviceSparseImageFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format,
      VkImageType type, VkSampleCountFlagBits samples, VkImageUsageFlags usage, VkImageTiling tiling,
      uint32_t *pPropertyCount, VkSparseImageFormatProperties *pProperties);

 private:
   QVulkanFunctions(vk::Instance instance, vk::DispatchLoaderDynamic dld);

   vk::DispatchLoaderDynamic m_dld;

   friend class QVulkanInstance;
};

#endif
