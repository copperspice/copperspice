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

#include <qvulkan_device_functions.h>

QVulkanDeviceFunctions::QVulkanDeviceFunctions(vk::Instance instance, vk::Device device, vk::DispatchLoaderDynamic dld)
   : m_dld(std::move(dld))
{
   m_dld.init(instance, device);
}

VkResult QVulkanDeviceFunctions::vkAllocateCommandBuffers(VkDevice device, const VkCommandBufferAllocateInfo *pAllocateInfo,
      VkCommandBuffer *pCommandBuffers)
{
   return m_dld.vkAllocateCommandBuffers(device, pAllocateInfo, pCommandBuffers);
}

VkResult QVulkanDeviceFunctions::vkAllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo *pAllocateInfo,
      VkDescriptorSet *pDescriptorSets)
{
   return m_dld.vkAllocateDescriptorSets(device, pAllocateInfo, pDescriptorSets);
}

VkResult QVulkanDeviceFunctions::vkAllocateMemory(VkDevice device, const VkMemoryAllocateInfo *pAllocateInfo,
      const VkAllocationCallbacks *pAllocator, VkDeviceMemory *pMemory)
{
   return m_dld.vkAllocateMemory(device, pAllocateInfo, pAllocator, pMemory);
}

void QVulkanDeviceFunctions::vkFreeCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t commandBufferCount,
      const VkCommandBuffer *pCommandBuffers)
{
   m_dld.vkFreeCommandBuffers(device, commandPool, commandBufferCount, pCommandBuffers);
}

VkResult QVulkanDeviceFunctions::vkFreeDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool, uint32_t descriptorSetCount,
      const VkDescriptorSet *pDescriptorSets)
{
   return m_dld.vkFreeDescriptorSets(device, descriptorPool, descriptorSetCount, pDescriptorSets);
}

void QVulkanDeviceFunctions::vkFreeMemory(VkDevice device, VkDeviceMemory memory, const VkAllocationCallbacks *pAllocator)
{
   m_dld.vkFreeMemory(device, memory, pAllocator);
}
