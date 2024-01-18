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

#include <qvulkan_device_functions.h>

QVulkanDeviceFunctions::QVulkanDeviceFunctions(vk::Instance instance, vk::Device device, vk::DispatchLoaderDynamic dld)
   : m_device(device), m_dld(std::move(dld))
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

void QVulkanDeviceFunctions::vkCmdBeginQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query, VkQueryControlFlags flags)
{
   m_dld.vkCmdBeginQuery(commandBuffer, queryPool, query, flags);
}

void QVulkanDeviceFunctions::vkCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBeginInfo,
      VkSubpassContents contents)
{
   m_dld.vkCmdBeginRenderPass(commandBuffer, pRenderPassBeginInfo, contents);
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

void QVulkanDeviceFunctions::vkCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
      VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageBlit *pRegions, VkFilter filter)
{
   m_dld.vkCmdBlitImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions, filter);
}

void QVulkanDeviceFunctions::vkCmdClearAttachments(VkCommandBuffer commandBuffer, uint32_t attachmentCount,
      const VkClearAttachment *pAttachments, uint32_t rectCount, const VkClearRect *pRects)
{
   m_dld.vkCmdClearAttachments(commandBuffer, attachmentCount, pAttachments, rectCount, pRects);
}

void QVulkanDeviceFunctions::vkCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
      const VkClearColorValue *pColor, uint32_t rangeCount, const VkImageSubresourceRange *pRanges)
{
   m_dld.vkCmdClearColorImage(commandBuffer, image, imageLayout, pColor, rangeCount, pRanges);
}

void QVulkanDeviceFunctions::vkCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
      const VkClearDepthStencilValue *pDepthStencil, uint32_t rangeCount, const VkImageSubresourceRange *pRanges)
{
   m_dld.vkCmdClearDepthStencilImage(commandBuffer, image, imageLayout, pDepthStencil, rangeCount, pRanges);
}

void QVulkanDeviceFunctions::vkCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer,
      uint32_t regionCount, const VkBufferCopy *pRegions)
{
   m_dld.vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, regionCount, pRegions);
}

void QVulkanDeviceFunctions::vkCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
      VkImageLayout dstImageLayout, uint32_t regionCount, const VkBufferImageCopy *pRegions)
{
   m_dld.vkCmdCopyBufferToImage(commandBuffer, srcBuffer, dstImage, dstImageLayout, regionCount, pRegions);
}

void QVulkanDeviceFunctions::vkCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
      VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageCopy *pRegions)
{
   m_dld.vkCmdCopyImage(commandBuffer, srcImage, srcImageLayout, dstImage,
      dstImageLayout, regionCount, pRegions);
}

void QVulkanDeviceFunctions::vkCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
      VkBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy *pRegions)
{
   m_dld.vkCmdCopyImageToBuffer(commandBuffer, srcImage, srcImageLayout, dstBuffer, regionCount, pRegions);
}

void QVulkanDeviceFunctions::vkCmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery,
      uint32_t queryCount, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize stride, VkQueryResultFlags flags)
{
   m_dld.vkCmdCopyQueryPoolResults(commandBuffer, queryPool, firstQuery, queryCount, dstBuffer,
      dstOffset, stride, flags);
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

void QVulkanDeviceFunctions::vkCmdEndQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query)
{
   m_dld.vkCmdEndQuery(commandBuffer, queryPool, query);
}

void QVulkanDeviceFunctions::vkCmdEndRenderPass(VkCommandBuffer commandBuffer)
{
   m_dld.vkCmdEndRenderPass(commandBuffer);
}

void QVulkanDeviceFunctions::vkCmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBufferCount,
      const VkCommandBuffer *pCommandBuffers)
{
   m_dld.vkCmdExecuteCommands(commandBuffer, commandBufferCount,pCommandBuffers);
}

void QVulkanDeviceFunctions::vkCmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize size,
   uint32_t data)
{
   m_dld.vkCmdFillBuffer(commandBuffer, dstBuffer, dstOffset, size, data);
}

void QVulkanDeviceFunctions::vkCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents)
{
   m_dld.vkCmdNextSubpass( commandBuffer, contents);
}

void QVulkanDeviceFunctions::vkCmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask,
      VkPipelineStageFlags dstStageMask, VkDependencyFlags dependencyFlags, uint32_t memoryBarrierCount,
      const VkMemoryBarrier *pMemoryBarriers, uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier *pBufferMemoryBarriers,
      uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier *pImageMemoryBarriers)
{
   m_dld.vkCmdPipelineBarrier(commandBuffer, srcStageMask, dstStageMask, dependencyFlags, memoryBarrierCount, pMemoryBarriers,
      bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
}

void QVulkanDeviceFunctions::vkCmdPushConstants(VkCommandBuffer commandBuffer, VkPipelineLayout layout, VkShaderStageFlags stageFlags,
      uint32_t offset, uint32_t size, const void *pValues)
{
   m_dld.vkCmdPushConstants(commandBuffer, layout, stageFlags, offset, size, pValues);
}

void QVulkanDeviceFunctions::vkCmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask)
{
   m_dld.vkCmdResetEvent(commandBuffer, event, stageMask);
}

void QVulkanDeviceFunctions::vkCmdResetQueryPool(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount)
{
   m_dld.vkCmdResetQueryPool(commandBuffer, queryPool, firstQuery, queryCount);
}

void QVulkanDeviceFunctions::vkCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
      VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageResolve *pRegions)
{
   m_dld.vkCmdResolveImage(commandBuffer, srcImage, srcImageLayout, dstImage, dstImageLayout, regionCount, pRegions);
}

void QVulkanDeviceFunctions::vkCmdSetBlendConstants(VkCommandBuffer commandBuffer, const float blendConstants[4])
{
   m_dld.vkCmdSetBlendConstants(commandBuffer, blendConstants);
}

void QVulkanDeviceFunctions::vkCmdSetDepthBias(VkCommandBuffer commandBuffer, float depthBiasConstantFactor, float depthBiasClamp,
      float depthBiasSlopeFactor)
{
   m_dld.vkCmdSetDepthBias(commandBuffer, depthBiasConstantFactor, depthBiasClamp, depthBiasSlopeFactor);
}

void QVulkanDeviceFunctions::vkCmdSetDepthBounds(VkCommandBuffer commandBuffer, float minDepthBounds, float maxDepthBounds)
{
   m_dld.vkCmdSetDepthBounds(commandBuffer, minDepthBounds, maxDepthBounds);
}

void QVulkanDeviceFunctions::vkCmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask)
{
   m_dld.vkCmdSetEvent(commandBuffer, event, stageMask);
}

void QVulkanDeviceFunctions::vkCmdSetLineWidth(VkCommandBuffer commandBuffer, float lineWidth)
{
   m_dld.vkCmdSetLineWidth(commandBuffer, lineWidth);
}

void QVulkanDeviceFunctions::vkCmdSetScissor(VkCommandBuffer commandBuffer, uint32_t firstScissor, uint32_t scissorCount,
      const VkRect2D *pScissors)
{
   m_dld.vkCmdSetScissor(commandBuffer, firstScissor, scissorCount, pScissors);
}

void QVulkanDeviceFunctions::vkCmdSetStencilCompareMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t compareMask)
{
   m_dld.vkCmdSetStencilCompareMask(commandBuffer, faceMask, compareMask);
}

void QVulkanDeviceFunctions::vkCmdSetStencilReference(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t referenceId)
{
   m_dld.vkCmdSetStencilReference(commandBuffer, faceMask, referenceId);
}

void QVulkanDeviceFunctions::vkCmdSetStencilWriteMask(VkCommandBuffer commandBuffer, VkStencilFaceFlags faceMask, uint32_t writeMask)
{
   m_dld.vkCmdSetStencilWriteMask(commandBuffer, faceMask, writeMask);
}

void QVulkanDeviceFunctions::vkCmdSetViewport(VkCommandBuffer commandBuffer, uint32_t firstViewport, uint32_t viewportCount,
      const VkViewport *pViewports)
{
   m_dld.vkCmdSetViewport(commandBuffer, firstViewport, viewportCount, pViewports);
}

void QVulkanDeviceFunctions::vkCmdUpdateBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize dataSize,
      const void *pData)
{
   m_dld.vkCmdUpdateBuffer(commandBuffer, dstBuffer, dstOffset, dataSize, pData);
}

void QVulkanDeviceFunctions::vkCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent *pEvents,
      VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, uint32_t memoryBarrierCount,
      const VkMemoryBarrier *pMemoryBarriers, uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier *pBufferMemoryBarriers,
      uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier *pImageMemoryBarriers)
{
   m_dld.vkCmdWaitEvents(commandBuffer, eventCount, pEvents, srcStageMask, dstStageMask, memoryBarrierCount, pMemoryBarriers,
      bufferMemoryBarrierCount, pBufferMemoryBarriers, imageMemoryBarrierCount, pImageMemoryBarriers);
}

void QVulkanDeviceFunctions::vkCmdWriteTimestamp(VkCommandBuffer commandBuffer, VkPipelineStageFlagBits pipelineStage,
      VkQueryPool queryPool, uint32_t query)
{
   m_dld.vkCmdWriteTimestamp(commandBuffer, pipelineStage, queryPool, query);
}

VkResult QVulkanDeviceFunctions::vkCreateBuffer(VkDevice device, const VkBufferCreateInfo *pCreateInfo,
      const VkAllocationCallbacks *pAllocator, VkBuffer *pBuffer)
{
   return m_dld.vkCreateBuffer(device, pCreateInfo, pAllocator, pBuffer);
}

VkResult QVulkanDeviceFunctions::vkCreateBufferView(VkDevice device, const VkBufferViewCreateInfo *pCreateInfo,
      const VkAllocationCallbacks *pAllocator, VkBufferView *pView)
{
   return m_dld.vkCreateBufferView(device, pCreateInfo, pAllocator, pView);
}

VkResult QVulkanDeviceFunctions::vkCreateCommandPool(VkDevice device, const VkCommandPoolCreateInfo *pCreateInfo,
      const VkAllocationCallbacks *pAllocator, VkCommandPool *pCommandPool)
{
   return m_dld.vkCreateCommandPool(device, pCreateInfo, pAllocator, pCommandPool);
}

VkResult QVulkanDeviceFunctions::vkCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
      const VkComputePipelineCreateInfo *pCreateInfos, const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines)
{
   return m_dld.vkCreateComputePipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);
}

VkResult QVulkanDeviceFunctions::vkCreateDescriptorPool(VkDevice device, const VkDescriptorPoolCreateInfo *pCreateInfo,
      const VkAllocationCallbacks *pAllocator, VkDescriptorPool *pDescriptorPool)
{
   return m_dld.vkCreateDescriptorPool(device, pCreateInfo, pAllocator, pDescriptorPool);
}

VkResult QVulkanDeviceFunctions::vkCreateDescriptorSetLayout(VkDevice device, const VkDescriptorSetLayoutCreateInfo *pCreateInfo,
      const VkAllocationCallbacks *pAllocator, VkDescriptorSetLayout *pSetLayout)
{
   return m_dld.vkCreateDescriptorSetLayout(device, pCreateInfo, pAllocator, pSetLayout);
}

VkResult QVulkanDeviceFunctions::vkCreateEvent(VkDevice device, const VkEventCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator,
      VkEvent *pEvent)
{
   return m_dld.vkCreateEvent(device, pCreateInfo, pAllocator, pEvent);
}

VkResult QVulkanDeviceFunctions::vkCreateFence(VkDevice device, const VkFenceCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator,
   VkFence *pFence)
{
   return m_dld.vkCreateFence(device, pCreateInfo, pAllocator, pFence);
}

VkResult QVulkanDeviceFunctions::vkCreateFramebuffer(VkDevice device, const VkFramebufferCreateInfo *pCreateInfo,
      const VkAllocationCallbacks *pAllocator, VkFramebuffer *pFramebuffer)
{
   return m_dld.vkCreateFramebuffer(device, pCreateInfo, pAllocator, pFramebuffer);
}

VkResult QVulkanDeviceFunctions::vkCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
      const VkGraphicsPipelineCreateInfo *pCreateInfos, const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines)
{
   return m_dld.vkCreateGraphicsPipelines(device, pipelineCache, createInfoCount, pCreateInfos, pAllocator, pPipelines);
}

VkResult QVulkanDeviceFunctions::vkCreateImage(VkDevice device, const VkImageCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator,
      VkImage *pImage)
{
   return m_dld.vkCreateImage(device, pCreateInfo, pAllocator, pImage);
}

VkResult QVulkanDeviceFunctions::vkCreateImageView(VkDevice device, const VkImageViewCreateInfo *pCreateInfo,
      const VkAllocationCallbacks *pAllocator, VkImageView *pView)
{
   return m_dld.vkCreateImageView(device, pCreateInfo, pAllocator, pView);
}

VkResult QVulkanDeviceFunctions::vkCreatePipelineCache(VkDevice device, const VkPipelineCacheCreateInfo *pCreateInfo,
      const VkAllocationCallbacks *pAllocator, VkPipelineCache *pPipelineCache)
{
   return m_dld.vkCreatePipelineCache(device, pCreateInfo, pAllocator, pPipelineCache);
}

VkResult QVulkanDeviceFunctions::vkCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo *pCreateInfo,
      const VkAllocationCallbacks *pAllocator, VkPipelineLayout *pPipelineLayout)
{
   return m_dld.vkCreatePipelineLayout(device, pCreateInfo, pAllocator, pPipelineLayout);
}

VkResult QVulkanDeviceFunctions::vkCreateQueryPool(VkDevice device, const VkQueryPoolCreateInfo *pCreateInfo,
      const VkAllocationCallbacks *pAllocator, VkQueryPool *pQueryPool)
{
   return m_dld.vkCreateQueryPool(device, pCreateInfo, pAllocator, pQueryPool);
}

VkResult QVulkanDeviceFunctions::vkCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo *pCreateInfo,
      const VkAllocationCallbacks *pAllocator, VkRenderPass *pRenderPass)
{
   return m_dld.vkCreateRenderPass(device, pCreateInfo, pAllocator, pRenderPass);
}

VkResult QVulkanDeviceFunctions::vkCreateSampler(VkDevice device, const VkSamplerCreateInfo *pCreateInfo,
      const VkAllocationCallbacks *pAllocator, VkSampler *pSampler)
{
   return m_dld.vkCreateSampler(device, pCreateInfo, pAllocator, pSampler);
}

VkResult QVulkanDeviceFunctions::vkCreateSemaphore(VkDevice device, const VkSemaphoreCreateInfo *pCreateInfo,
      const VkAllocationCallbacks *pAllocator, VkSemaphore *pSemaphore)
{
   return m_dld.vkCreateSemaphore(device, pCreateInfo, pAllocator, pSemaphore);
}

VkResult QVulkanDeviceFunctions::vkCreateShaderModule(VkDevice device, const VkShaderModuleCreateInfo *pCreateInfo,
      const VkAllocationCallbacks *pAllocator, VkShaderModule *pShaderModule)
{
   return m_dld.vkCreateShaderModule(device, pCreateInfo, pAllocator, pShaderModule);
}

void QVulkanDeviceFunctions::vkDestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks *pAllocator)
{
   m_dld.vkDestroyBuffer(device, buffer, pAllocator);
}

void QVulkanDeviceFunctions::vkDestroyBufferView(VkDevice device, VkBufferView bufferView, const VkAllocationCallbacks *pAllocator)
{
   m_dld.vkDestroyBufferView(device, bufferView, pAllocator);
}

void QVulkanDeviceFunctions::vkDestroyCommandPool(VkDevice device, VkCommandPool commandPool, const VkAllocationCallbacks *pAllocator)
{
   m_dld.vkDestroyCommandPool(device, commandPool, pAllocator);
}

void QVulkanDeviceFunctions::vkDestroyDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool, const VkAllocationCallbacks *pAllocator)
{
   m_dld.vkDestroyDescriptorPool(device, descriptorPool, pAllocator);
}

void QVulkanDeviceFunctions::vkDestroyDescriptorSetLayout(VkDevice device, VkDescriptorSetLayout descriptorSetLayout,
      const VkAllocationCallbacks *pAllocator)
{
   m_dld.vkDestroyDescriptorSetLayout(device, descriptorSetLayout, pAllocator);
}

void QVulkanDeviceFunctions::vkDestroyDevice(VkDevice device, const VkAllocationCallbacks *pAllocator)
{
   m_dld.vkDestroyDevice(device, pAllocator);
}

void QVulkanDeviceFunctions::vkDestroyEvent(VkDevice device, VkEvent event, const VkAllocationCallbacks *pAllocator)
{
   m_dld.vkDestroyEvent(device, event, pAllocator);
}

void QVulkanDeviceFunctions::vkDestroyFence(VkDevice device, VkFence fence, const VkAllocationCallbacks *pAllocator)
{
   m_dld.vkDestroyFence(device, fence, pAllocator);
}

void QVulkanDeviceFunctions::vkDestroyFramebuffer(VkDevice device, VkFramebuffer framebuffer, const VkAllocationCallbacks *pAllocator)
{
   m_dld.vkDestroyFramebuffer(device, framebuffer, pAllocator);
}

void QVulkanDeviceFunctions::vkDestroyImage(VkDevice device, VkImage image, const VkAllocationCallbacks *pAllocator)
{
   m_dld.vkDestroyImage(device, image, pAllocator);
}

void QVulkanDeviceFunctions::vkDestroyImageView(VkDevice device, VkImageView imageView, const VkAllocationCallbacks *pAllocator)
{
   m_dld.vkDestroyImageView(device, imageView, pAllocator);
}

void QVulkanDeviceFunctions::vkDestroyPipeline(VkDevice device, VkPipeline pipeline, const VkAllocationCallbacks *pAllocator)
{
   m_dld.vkDestroyPipeline(device, pipeline, pAllocator);
}

void QVulkanDeviceFunctions::vkDestroyPipelineCache(VkDevice device, VkPipelineCache pipelineCache, const VkAllocationCallbacks *pAllocator)
{
   m_dld.vkDestroyPipelineCache(device,  pipelineCache, pAllocator);
}

void QVulkanDeviceFunctions::vkDestroyPipelineLayout(VkDevice device, VkPipelineLayout pipelineLayout, const VkAllocationCallbacks *pAllocator)
{
   m_dld.vkDestroyPipelineLayout(device, pipelineLayout, pAllocator);
}

void QVulkanDeviceFunctions::vkDestroyQueryPool(VkDevice device, VkQueryPool queryPool, const VkAllocationCallbacks *pAllocator)
{
   m_dld.vkDestroyQueryPool(device, queryPool, pAllocator);
}

void QVulkanDeviceFunctions::vkDestroyRenderPass(VkDevice device, VkRenderPass renderPass, const VkAllocationCallbacks *pAllocator)
{
   m_dld.vkDestroyRenderPass(device, renderPass, pAllocator);
}

void QVulkanDeviceFunctions::vkDestroySampler(VkDevice device, VkSampler sampler, const VkAllocationCallbacks *pAllocator)
{
   m_dld.vkDestroySampler(device, sampler, pAllocator);
}

void QVulkanDeviceFunctions::vkDestroySemaphore(VkDevice device, VkSemaphore semaphore, const VkAllocationCallbacks *pAllocator)
{
   m_dld.vkDestroySemaphore(device, semaphore, pAllocator);
}

void QVulkanDeviceFunctions::vkDestroyShaderModule(VkDevice device, VkShaderModule shaderModule, const VkAllocationCallbacks *pAllocator)
{
   m_dld.vkDestroyShaderModule(device, shaderModule, pAllocator);
}

VkResult QVulkanDeviceFunctions::vkDeviceWaitIdle(VkDevice device)
{
   return m_dld.vkDeviceWaitIdle(device);
}

VkResult QVulkanDeviceFunctions::vkEndCommandBuffer(VkCommandBuffer commandBuffer)
{
   return m_dld.vkEndCommandBuffer(commandBuffer);
}

VkResult QVulkanDeviceFunctions::vkFlushMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount, const VkMappedMemoryRange *pMemoryRanges)
{
   return m_dld.vkFlushMappedMemoryRanges(device, memoryRangeCount, pMemoryRanges);
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

void QVulkanDeviceFunctions::vkGetBufferMemoryRequirements(VkDevice device, VkBuffer buffer, VkMemoryRequirements *pMemoryRequirements)
{
   m_dld.vkGetBufferMemoryRequirements(device, buffer, pMemoryRequirements);
}

void QVulkanDeviceFunctions::vkGetDeviceMemoryCommitment(VkDevice device, VkDeviceMemory memory, VkDeviceSize *pCommittedMemoryInBytes)
{
   m_dld.vkGetDeviceMemoryCommitment(device, memory, pCommittedMemoryInBytes);
}

void QVulkanDeviceFunctions::vkGetDeviceQueue(VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue *pQueue)
{
   m_dld.vkGetDeviceQueue(device, queueFamilyIndex, queueIndex, pQueue);
}

VkResult QVulkanDeviceFunctions::vkGetEventStatus(VkDevice device, VkEvent event)
{
   return m_dld.vkGetEventStatus(device, event);
}

VkResult QVulkanDeviceFunctions::vkGetFenceStatus(VkDevice device, VkFence fence)
{
   return m_dld.vkGetFenceStatus(device, fence);
}

void QVulkanDeviceFunctions::vkGetImageMemoryRequirements(VkDevice device, VkImage image, VkMemoryRequirements *pMemoryRequirements)
{
   m_dld.vkGetImageMemoryRequirements(device, image, pMemoryRequirements);
}

void QVulkanDeviceFunctions::vkGetImageSparseMemoryRequirements(VkDevice device, VkImage image, uint32_t *pSparseMemoryRequirementCount,
      VkSparseImageMemoryRequirements *pSparseMemoryRequirements)
{
   m_dld.vkGetImageSparseMemoryRequirements(device, image, pSparseMemoryRequirementCount, pSparseMemoryRequirements);
}

void QVulkanDeviceFunctions::vkGetImageSubresourceLayout(VkDevice device, VkImage image, const VkImageSubresource *pSubresource,
      VkSubresourceLayout *pLayout)
{
   m_dld.vkGetImageSubresourceLayout(device, image, pSubresource, pLayout);
}

VkResult QVulkanDeviceFunctions::vkGetPipelineCacheData(VkDevice device, VkPipelineCache pipelineCache, size_t *pDataSize, void *pData)
{
   return m_dld.vkGetPipelineCacheData(device, pipelineCache, pDataSize, pData);
}

VkResult QVulkanDeviceFunctions::vkGetQueryPoolResults(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount,
      size_t dataSize, void *pData, VkDeviceSize stride, VkQueryResultFlags flags)
{
   return m_dld.vkGetQueryPoolResults(device, queryPool, firstQuery, queryCount, dataSize, pData, stride, flags);
}

void QVulkanDeviceFunctions::vkGetRenderAreaGranularity(VkDevice device, VkRenderPass renderPass, VkExtent2D *pGranularity)
{
   m_dld.vkGetRenderAreaGranularity(device, renderPass, pGranularity);
}

VkResult QVulkanDeviceFunctions::vkInvalidateMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount,
      const VkMappedMemoryRange *pMemoryRanges)
{
   return m_dld.vkInvalidateMappedMemoryRanges(device, memoryRangeCount, pMemoryRanges);
}

VkResult QVulkanDeviceFunctions::vkMapMemory(VkDevice device, VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size,
      VkMemoryMapFlags flags, void **ppData)
{
   return m_dld.vkMapMemory(device, memory, offset, size, flags, ppData);
}

VkResult QVulkanDeviceFunctions::vkMergePipelineCaches(VkDevice device, VkPipelineCache dstCache, uint32_t srcCacheCount,
      const VkPipelineCache *pSrcCaches)
{
   return m_dld.vkMergePipelineCaches(device, dstCache, srcCacheCount, pSrcCaches);
}

VkResult QVulkanDeviceFunctions::vkQueueBindSparse(VkQueue queue, uint32_t bindInfoCount, const VkBindSparseInfo *pBindInfo, VkFence fence)
{
   return m_dld.vkQueueBindSparse(queue, bindInfoCount, pBindInfo, fence);
}

VkResult QVulkanDeviceFunctions::vkQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo *pSubmits, VkFence fence)
{
   return m_dld.vkQueueSubmit(queue, submitCount, pSubmits, fence);
}

VkResult QVulkanDeviceFunctions::vkQueueWaitIdle(VkQueue queue)
{
   return m_dld.vkQueueWaitIdle(queue);
}

VkResult QVulkanDeviceFunctions::vkResetCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferResetFlags flags)
{
   return m_dld.vkResetCommandBuffer(commandBuffer, flags);
}

VkResult QVulkanDeviceFunctions::vkResetCommandPool(VkDevice device, VkCommandPool commandPool, VkCommandPoolResetFlags flags)
{
   return m_dld.vkResetCommandPool(device, commandPool, flags);
}

VkResult QVulkanDeviceFunctions::vkResetDescriptorPool(VkDevice device, VkDescriptorPool descriptorPool, VkDescriptorPoolResetFlags flags)
{
   return m_dld.vkResetDescriptorPool(device, descriptorPool, flags);
}

VkResult QVulkanDeviceFunctions::vkResetEvent(VkDevice device, VkEvent event)
{
   return m_dld.vkResetEvent(device, event);
}

VkResult QVulkanDeviceFunctions::vkResetFences(VkDevice device, uint32_t fenceCount, const VkFence *pFences)
{
   return m_dld.vkResetFences(device, fenceCount, pFences);
}

VkResult QVulkanDeviceFunctions::vkSetEvent(VkDevice device, VkEvent event)
{
   return m_dld.vkSetEvent(device, event);
}

void QVulkanDeviceFunctions::vkUnmapMemory(VkDevice device, VkDeviceMemory memory)
{
   m_dld.vkUnmapMemory(device, memory);
}

void QVulkanDeviceFunctions::vkUpdateDescriptorSets(VkDevice device, uint32_t descriptorWriteCount, const VkWriteDescriptorSet *pDescriptorWrites,
      uint32_t descriptorCopyCount, const VkCopyDescriptorSet *pDescriptorCopies)
{
   m_dld.vkUpdateDescriptorSets(device, descriptorWriteCount, pDescriptorWrites, descriptorCopyCount, pDescriptorCopies);
}

VkResult QVulkanDeviceFunctions::vkWaitForFences(VkDevice device, uint32_t fenceCount, const VkFence *pFences, VkBool32 waitAll, uint64_t timeout)
{
   return m_dld.vkWaitForFences(device, fenceCount, pFences, waitAll, timeout);
}
