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

#include <qvulkan_functions.h>

QVulkanFunctions::QVulkanFunctions(vk::Instance instance, vk::DispatchLoaderDynamic dld)
   : m_dld(dld)
{
   m_dld.init(instance);
}

VkResult QVulkanFunctions::vkCreateDevice(VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo *pCreateInfo,
   const VkAllocationCallbacks *pAllocator, VkDevice *pDevice)
{
   return m_dld.vkCreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice);
}

VkResult QVulkanFunctions::vkEnumerateDeviceExtensionProperties(VkPhysicalDevice physicalDevice, const char *pLayerName,
   uint32_t *pPropertyCount, VkExtensionProperties *pProperties)
{
   return m_dld.vkEnumerateDeviceExtensionProperties(physicalDevice, pLayerName, pPropertyCount, pProperties);
}

VkResult QVulkanFunctions::vkEnumerateInstanceExtensionProperties(const char *pLayerName, uint32_t *pPropertyCount,
   VkExtensionProperties *pProperties)
{
   return m_dld.vkEnumerateInstanceExtensionProperties(pLayerName, pPropertyCount, pProperties);
}

VkResult QVulkanFunctions::vkEnumerateDeviceLayerProperties(VkPhysicalDevice physicalDevice, uint32_t *pPropertyCount,
   VkLayerProperties *pProperties)
{
   return m_dld.vkEnumerateDeviceLayerProperties(physicalDevice, pPropertyCount, pProperties);
}

VkResult QVulkanFunctions::vkEnumerateInstanceLayerProperties(uint32_t *pPropertyCount, VkLayerProperties *pProperties)
{
   return m_dld.vkEnumerateInstanceLayerProperties(pPropertyCount, pProperties);
}

VkResult QVulkanFunctions::vkEnumeratePhysicalDevices(VkInstance instance, uint32_t *pPhysicalDeviceCount,
   VkPhysicalDevice *pPhysicalDevices)
{
   return m_dld.vkEnumeratePhysicalDevices(instance, pPhysicalDeviceCount, pPhysicalDevices);
}

PFN_vkVoidFunction QVulkanFunctions::vkGetDeviceProcAddr(VkDevice device, const char *pName)
{
   return m_dld.vkGetDeviceProcAddr(device, pName);
}

void QVulkanFunctions::vkGetPhysicalDeviceFeatures(VkPhysicalDevice physicalDevice, VkPhysicalDeviceFeatures *pFeatures)
{
   m_dld.vkGetPhysicalDeviceFeatures(physicalDevice, pFeatures);
}

void QVulkanFunctions::vkGetPhysicalDeviceFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format,
   VkFormatProperties *pFormatProperties)
{
   m_dld.vkGetPhysicalDeviceFormatProperties(physicalDevice, format, pFormatProperties);
}

VkResult QVulkanFunctions::vkGetPhysicalDeviceImageFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format,
   VkImageType type, VkImageTiling tiling, VkImageUsageFlags usage, VkImageCreateFlags flags,
   VkImageFormatProperties *pImageFormatProperties)
{
   return m_dld.vkGetPhysicalDeviceImageFormatProperties(physicalDevice, format, type, tiling, usage, flags,
      pImageFormatProperties);
}

void QVulkanFunctions::vkGetPhysicalDeviceProperties(VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties *pProperties)
{
   m_dld.vkGetPhysicalDeviceProperties(physicalDevice, pProperties);
}

void QVulkanFunctions::vkGetPhysicalDeviceMemoryProperties(VkPhysicalDevice physicalDevice,
   VkPhysicalDeviceMemoryProperties *pMemoryProperties)
{
   m_dld.vkGetPhysicalDeviceMemoryProperties(physicalDevice, pMemoryProperties);
}

void QVulkanFunctions::vkGetPhysicalDeviceQueueFamilyProperties(VkPhysicalDevice physicalDevice,
   uint32_t *pQueueFamilyPropertyCount, VkQueueFamilyProperties *pQueueFamilyProperties)
{
   m_dld.vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
}

void QVulkanFunctions::vkGetPhysicalDeviceSparseImageFormatProperties(VkPhysicalDevice physicalDevice, VkFormat format,
   VkImageType type, VkSampleCountFlagBits samples, VkImageUsageFlags usage, VkImageTiling tiling,
   uint32_t *pPropertyCount, VkSparseImageFormatProperties *pProperties)
{
   m_dld.vkGetPhysicalDeviceSparseImageFormatProperties(physicalDevice, format, type, samples,
      usage, tiling, pPropertyCount, pProperties);
}
