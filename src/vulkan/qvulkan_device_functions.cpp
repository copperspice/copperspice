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

VkResult QVulkanDeviceFunctions::vkBeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo *pBeginInfo)
{
   return m_dld.vkBeginCommandBuffer(commandBuffer, pBeginInfo);
}

VkResult QVulkanDeviceFunctions::vkBindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory memory, VkDeviceSize memoryOffset)
{
   return m_dld.vkBindBufferMemory(device, buffer, memory, memoryOffset);
}

VkResult QVulkanDeviceFunctions::vkBindImageMemory(VkDevice device, VkImage image, VkDeviceMemory memory, VkDeviceSize memoryOffset)
{
   return m_dld.vkBindImageMemory(device, image, memory, memoryOffset);
}

void QVulkanDeviceFunctions::vkCmdBindDescriptorSets(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
      VkPipelineLayout layout, uint32_t firstSet, uint32_t descriptorSetCount, const VkDescriptorSet *pDescriptorSets,
      uint32_t dynamicOffsetCount, const uint32_t *pDynamicOffsets)
{
   m_dld.vkCmdBindDescriptorSets(commandBuffer, pipelineBindPoint, layout, firstSet, descriptorSetCount,
      pDescriptorSets, dynamicOffsetCount, pDynamicOffsets);
}

void QVulkanDeviceFunctions::vkCmdBindIndexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkIndexType indexType)
{
   m_dld.vkCmdBindIndexBuffer(commandBuffer, buffer, offset, indexType);
}

void QVulkanDeviceFunctions::vkCmdBindPipeline(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
      VkPipeline pipeline)
{
   m_dld.vkCmdBindPipeline(commandBuffer, pipelineBindPoint, pipeline);
}

void QVulkanDeviceFunctions::vkCmdBindVertexBuffers(VkCommandBuffer commandBuffer, uint32_t firstBinding,
      uint32_t bindingCount, const VkBuffer *pBuffers, const VkDeviceSize *pOffsets)
{
   m_dld.vkCmdBindVertexBuffers(commandBuffer, firstBinding, bindingCount, pBuffers, pOffsets);
}

void QVulkanDeviceFunctions::vkCmdDispatch(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY,
      uint32_t groupCountZ)
{
   m_dld.vkCmdDispatch(commandBuffer, groupCountX, groupCountY, groupCountZ);
}

void QVulkanDeviceFunctions::vkCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset)
{
   m_dld.vkCmdDispatchIndirect(commandBuffer, buffer, offset);
}

void QVulkanDeviceFunctions::vkCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount,
      uint32_t firstVertex, uint32_t firstInstance)
{
   m_dld.vkCmdDraw(commandBuffer, vertexCount, instanceCount, firstVertex, firstInstance);
}

void QVulkanDeviceFunctions::vkCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount,
      uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance)
{
   m_dld.vkCmdDrawIndexed(commandBuffer, indexCount, instanceCount, firstIndex, vertexOffset, firstInstance);
}

void QVulkanDeviceFunctions::vkCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
      uint32_t drawCount, uint32_t stride)
{
   m_dld.vkCmdDrawIndexedIndirect(commandBuffer, buffer, offset, drawCount, stride);
}

void QVulkanDeviceFunctions::vkCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset,
      uint32_t drawCount, uint32_t stride)
{
   m_dld.vkCmdDrawIndirect(commandBuffer, buffer, offset, drawCount, stride);
}

void QVulkanDeviceFunctions::vkCmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBufferCount,
      const VkCommandBuffer *pCommandBuffers)
{
   m_dld.vkCmdExecuteCommands(commandBuffer, commandBufferCount,pCommandBuffers);
}

VkResult QVulkanDeviceFunctions::vkEndCommandBuffer(VkCommandBuffer commandBuffer)
{
   return m_dld.vkEndCommandBuffer(commandBuffer);
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

VkResult QVulkanDeviceFunctions::vkResetCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferResetFlags flags)
{
   return m_dld.vkResetCommandBuffer(commandBuffer, flags);
}
