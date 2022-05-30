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

#ifndef QVULKAN_DEVICE_FUNCTIONS_H
#define QVULKAN_DEVICE_FUNCTIONS_H

#include <qglobal.h>

#include <vulkan/vulkan.hpp>

class Q_VULKAN_EXPORT QVulkanDeviceFunctions
{
 public:
   VkResult vkAllocateCommandBuffers(VkDevice device, const VkCommandBufferAllocateInfo *pAllocateInfo,
      VkCommandBuffer *pCommandBuffers);

   VkResult vkAllocateDescriptorSets(VkDevice device, const VkDescriptorSetAllocateInfo *pAllocateInfo,
      VkDescriptorSet *pDescriptorSets);

   VkResult vkAllocateMemory(VkDevice device, const VkMemoryAllocateInfo *pAllocateInfo,
      const VkAllocationCallbacks *pAllocator, VkDeviceMemory *pMemory);

   VkResult vkBeginCommandBuffer(VkCommandBuffer commandBuffer, const VkCommandBufferBeginInfo *pBeginInfo);
   VkResult vkBindBufferMemory(VkDevice device, VkBuffer buffer, VkDeviceMemory memory, VkDeviceSize memoryOffset);
   VkResult vkBindImageMemory(VkDevice device, VkImage image, VkDeviceMemory memory, VkDeviceSize memoryOffset);

   void vkCmdBeginQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query, VkQueryControlFlags flags);

   void vkCmdBeginRenderPass(VkCommandBuffer commandBuffer, const VkRenderPassBeginInfo *pRenderPassBeginInfo,
      VkSubpassContents contents);

   void vkCmdBindDescriptorSets(VkCommandBuffer commandBuffer, VkPipelineBindPoint pipelineBindPoint,
      VkPipelineLayout layout, uint32_t firstSet, uint32_t descriptorSetCount, const VkDescriptorSet *pDescriptorSets,
      uint32_t dynamicOffsetCount, const uint32_t *pDynamicOffsets);

   void vkCmdBindIndexBuffer(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, VkIndexType indexType);

   void vkCmdBindPipeline(VkCommandBuffer commandBuffer,
      VkPipelineBindPoint pipelineBindPoint, VkPipeline pipeline);

   void vkCmdBindVertexBuffers(VkCommandBuffer commandBuffer, uint32_t firstBinding,
      uint32_t bindingCount, const VkBuffer *pBuffers, const VkDeviceSize *pOffsets);

   void vkCmdBlitImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
      VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageBlit *pRegions, VkFilter filter);

   void vkCmdClearColorImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
      const VkClearColorValue *pColor, uint32_t rangeCount, const VkImageSubresourceRange *pRanges);

   void vkCmdClearDepthStencilImage(VkCommandBuffer commandBuffer, VkImage image, VkImageLayout imageLayout,
      const VkClearDepthStencilValue *pDepthStencil, uint32_t rangeCount, const VkImageSubresourceRange *pRanges);

   void vkCmdCopyBuffer(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkBuffer dstBuffer,
      uint32_t regionCount, const VkBufferCopy *pRegions);

   void vkCmdCopyBufferToImage(VkCommandBuffer commandBuffer, VkBuffer srcBuffer, VkImage dstImage,
      VkImageLayout dstImageLayout, uint32_t regionCount, const VkBufferImageCopy *pRegions);

   void vkCmdCopyImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
      VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageCopy *pRegions);

   void vkCmdCopyImageToBuffer(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
      VkBuffer dstBuffer, uint32_t regionCount, const VkBufferImageCopy *pRegions);

   void vkCmdCopyQueryPoolResults(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery,
      uint32_t queryCount, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize stride, VkQueryResultFlags flags);

   void vkCmdDispatch(VkCommandBuffer commandBuffer, uint32_t groupCountX, uint32_t groupCountY, uint32_t groupCountZ);

   void vkCmdDispatchIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset);

   void vkCmdDraw(VkCommandBuffer commandBuffer, uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex,
      uint32_t firstInstance);

   void vkCmdDrawIndexed(VkCommandBuffer commandBuffer, uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex,
      int32_t vertexOffset, uint32_t firstInstance);

   void vkCmdDrawIndexedIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount,
      uint32_t stride);

   void vkCmdDrawIndirect(VkCommandBuffer commandBuffer, VkBuffer buffer, VkDeviceSize offset, uint32_t drawCount,
      uint32_t stride);

   void vkCmdEndQuery(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t query);

   void vkCmdEndRenderPass(VkCommandBuffer commandBuffer);

   void vkCmdExecuteCommands(VkCommandBuffer commandBuffer, uint32_t commandBufferCount, const VkCommandBuffer *pCommandBuffers);

   void vkCmdFillBuffer(VkCommandBuffer commandBuffer, VkBuffer dstBuffer, VkDeviceSize dstOffset, VkDeviceSize size,
      uint32_t data);

   void vkCmdNextSubpass(VkCommandBuffer commandBuffer, VkSubpassContents contents);

   void vkCmdPipelineBarrier(VkCommandBuffer commandBuffer, VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask,
      VkDependencyFlags dependencyFlags, uint32_t memoryBarrierCount, const VkMemoryBarrier * pMemoryBarriers,
      uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier *pBufferMemoryBarriers,
      uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier *pImageMemoryBarriers);

   void vkCmdResetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask);

   void vkCmdResetQueryPool(VkCommandBuffer commandBuffer, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount);

   void vkCmdResolveImage(VkCommandBuffer commandBuffer, VkImage srcImage, VkImageLayout srcImageLayout,
      VkImage dstImage, VkImageLayout dstImageLayout, uint32_t regionCount, const VkImageResolve *pRegions);


   void vkCmdSetEvent(VkCommandBuffer commandBuffer, VkEvent event, VkPipelineStageFlags stageMask);

   void vkCmdWaitEvents(VkCommandBuffer commandBuffer, uint32_t eventCount, const VkEvent* pEvents,
      VkPipelineStageFlags srcStageMask, VkPipelineStageFlags dstStageMask, uint32_t memoryBarrierCount,
      const VkMemoryBarrier *pMemoryBarriers, uint32_t bufferMemoryBarrierCount, const VkBufferMemoryBarrier *pBufferMemoryBarriers,
      uint32_t imageMemoryBarrierCount, const VkImageMemoryBarrier *pImageMemoryBarriers);


   VkResult vkCreateBuffer(VkDevice device, const VkBufferCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator,
      VkBuffer *pBuffer);

   VkResult vkCreateBufferView(VkDevice device, const VkBufferViewCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator,
      VkBufferView *pView);

   VkResult vkCreateComputePipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
      const VkComputePipelineCreateInfo *pCreateInfos, const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines);


   VkResult vkCreateEvent(VkDevice device, const VkEventCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator,
      VkEvent *pEvent);

   VkResult vkCreateFence(VkDevice device, const VkFenceCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator,
      VkFence *pFence);

   VkResult vkCreateGraphicsPipelines(VkDevice device, VkPipelineCache pipelineCache, uint32_t createInfoCount,
      const VkGraphicsPipelineCreateInfo *pCreateInfos, const VkAllocationCallbacks *pAllocator, VkPipeline *pPipelines);

   VkResult vkCreateImage(VkDevice device, const VkImageCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator,
      VkImage *pImage);

   VkResult vkCreateImageView(VkDevice device, const VkImageViewCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator,
      VkImageView *pView);

   VkResult vkCreatePipelineCache(VkDevice device, const VkPipelineCacheCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator,
      VkPipelineCache *pPipelineCache);

   VkResult vkCreatePipelineLayout(VkDevice device, const VkPipelineLayoutCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator,
      VkPipelineLayout *pPipelineLayout);

   VkResult vkCreateQueryPool(VkDevice device, const VkQueryPoolCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator,
      VkQueryPool *pQueryPool);

   VkResult vkCreateRenderPass(VkDevice device, const VkRenderPassCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator,
      VkRenderPass *pRenderPass);

   VkResult vkCreateSemaphore(VkDevice device, const VkSemaphoreCreateInfo *pCreateInfo, const VkAllocationCallbacks *pAllocator,
      VkSemaphore *pSemaphore);

   void vkDestroyBuffer(VkDevice device, VkBuffer buffer, const VkAllocationCallbacks *pAllocator);
   void vkDestroyBufferView(VkDevice device, VkBufferView bufferView, const VkAllocationCallbacks *pAllocator);
   void vkDestroyEvent(VkDevice device, VkEvent event, const VkAllocationCallbacks *pAllocator);
   void vkDestroyFence(VkDevice device, VkFence fence, const VkAllocationCallbacks *pAllocator);
   void vkDestroyImage(VkDevice device, VkImage image, const VkAllocationCallbacks *pAllocator);
   void vkDestroyImageView(VkDevice device, VkImageView imageView, const VkAllocationCallbacks *pAllocator);
   void vkDestroyPipeline(VkDevice device, VkPipeline pipeline, const VkAllocationCallbacks *pAllocator);
   void vkDestroyPipelineCache(VkDevice device, VkPipelineCache pipelineCache, const VkAllocationCallbacks *pAllocator);
   void vkDestroyPipelineLayout(VkDevice device, VkPipelineLayout pipelineLayout, const VkAllocationCallbacks *pAllocator);
   void vkDestroyQueryPool(VkDevice device, VkQueryPool queryPool, const VkAllocationCallbacks *pAllocator);
   void vkDestroyRenderPass(VkDevice device, VkRenderPass renderPass, const VkAllocationCallbacks *pAllocator);
   void vkDestroySemaphore(VkDevice device, VkSemaphore semaphore, const VkAllocationCallbacks *pAllocator);
   VkResult vkDeviceWaitIdle(VkDevice device);
   VkResult vkEndCommandBuffer(VkCommandBuffer commandBuffer);
   VkResult vkFlushMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount, const VkMappedMemoryRange * pMemoryRanges);

   void vkFreeCommandBuffers(VkDevice device, VkCommandPool commandPool, uint32_t commandBufferCount,
      const VkCommandBuffer *pCommandBuffers);

   VkResult vkFreeDescriptorSets(VkDevice device, VkDescriptorPool descriptorPool, uint32_t descriptorSetCount,
      const VkDescriptorSet *pDescriptorSets);

   void vkFreeMemory(VkDevice device, VkDeviceMemory memory, const VkAllocationCallbacks *pAllocator);
   void vkGetBufferMemoryRequirements(VkDevice device, VkBuffer buffer, VkMemoryRequirements *pMemoryRequirements);
   void vkGetDeviceMemoryCommitment(VkDevice device, VkDeviceMemory memory, VkDeviceSize *pCommittedMemoryInBytes);
   void vkGetDeviceQueue(VkDevice device, uint32_t queueFamilyIndex, uint32_t queueIndex, VkQueue *pQueue);
   VkResult vkGetEventStatus(VkDevice device, VkEvent event);
   VkResult vkGetFenceStatus(VkDevice device, VkFence fence);
   void vkGetImageMemoryRequirements(VkDevice device, VkImage image, VkMemoryRequirements *pMemoryRequirements);

   void vkGetImageSparseMemoryRequirements(VkDevice device, VkImage image, uint32_t *pSparseMemoryRequirementCount,
      VkSparseImageMemoryRequirements *pSparseMemoryRequirements);

   void vkGetImageSubresourceLayout(VkDevice device, VkImage image, const VkImageSubresource *pSubresource,
      VkSubresourceLayout *pLayout);

   VkResult vkGetPipelineCacheData(VkDevice device, VkPipelineCache pipelineCache, size_t *pDataSize, void *pData);

   VkResult vkGetQueryPoolResults(VkDevice device, VkQueryPool queryPool, uint32_t firstQuery, uint32_t queryCount,
      size_t dataSize, void *pData, VkDeviceSize stride, VkQueryResultFlags flags);

   VkResult vkInvalidateMappedMemoryRanges(VkDevice device, uint32_t memoryRangeCount, const VkMappedMemoryRange *pMemoryRanges);

   VkResult vkMapMemory(VkDevice device, VkDeviceMemory memory, VkDeviceSize offset, VkDeviceSize size, VkMemoryMapFlags flags,
      void **ppData);

   VkResult vkMergePipelineCaches(VkDevice device, VkPipelineCache dstCache, uint32_t srcCacheCount, const VkPipelineCache *pSrcCaches);
   VkResult vkQueueBindSparse(VkQueue queue, uint32_t bindInfoCount, const VkBindSparseInfo *pBindInfo, VkFence fence);
   VkResult vkQueueSubmit(VkQueue queue, uint32_t submitCount, const VkSubmitInfo *pSubmits, VkFence fence);
   VkResult vkQueueWaitIdle(VkQueue queue);
   VkResult vkResetCommandBuffer(VkCommandBuffer commandBuffer, VkCommandBufferResetFlags flags);
   VkResult vkResetEvent(VkDevice device, VkEvent event);
   VkResult vkResetFences(VkDevice device, uint32_t fenceCount, const VkFence *pFences);
   VkResult vkSetEvent(VkDevice device, VkEvent event);

   void vkUnmapMemory(VkDevice device, VkDeviceMemory memory);

   VkResult vkWaitForFences(VkDevice device, uint32_t fenceCount, const VkFence *pFences, VkBool32 waitAll, uint64_t timeout);

 private:
   QVulkanDeviceFunctions(vk::Instance instance, vk::Device device, vk::DispatchLoaderDynamic dld);

   vk::DispatchLoaderDynamic m_dld;

};

#endif
