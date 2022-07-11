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

#ifndef QVULKAN_WINDOW_H
#define QVULKAN_WINDOW_H

#include <qwindow.h>
#include <qvulkan_instance.h>
#include <qvulkan_window_renderer.h>

#include <vulkan/vulkan.hpp>

class Q_VULKAN_EXPORT QVulkanWindow: public QWindow
{
   VULKAN_CS_OBJECT(QVulkanWindow)

 public:
   enum VulkanOptions : uint32_t {
      PersistentResources = 0x0001
   };

   using VulkanFlags = QFlags<VulkanOptions>;
   using Flags [[deprecated("Replace with QVulkanWindow::VulkanFlags")]] = VulkanFlags;

   static constexpr const int MAX_CONCURRENT_FRAME_COUNT = 10;

   QVulkanWindow(QWindow *parent = nullptr);
   ~QVulkanWindow();

   QVector<VkPhysicalDeviceProperties> availablePhysicalDevices();
   QMatrix4x4 clipCorrectionMatrix();
   int concurrentFrameCount() const;

   virtual QVulkanWindowRenderer *createRenderer();

   VkCommandBuffer currentCommandBuffer() const;
   int currentFrame() const;
   VkFramebuffer currentFramebuffer() const;
   VkRenderPass defaultRenderPass() const;
   VkDevice device() const;

   QVulkanWindow::VulkanFlags flags() const;
   bool isValid() const;
   VkPhysicalDevice physicalDevice() const;
   const VkPhysicalDeviceProperties *physicalDeviceProperties() const;
   void setDeviceExtensions(const QStringList &extensions);
   void setFlags(QVulkanWindow::VulkanFlags flags);
   void setPhysicalDeviceIndex(int idx);
   void setSampleCount(int sampleCount);
   QVector<QVulkanExtensionProperties> supportedDeviceExtensions();
   VkSurfaceKHR vulkanSurface() const;
   VkImage swapChainImage(int idx) const;
   int swapChainImageCount() const;
   QSize swapChainImageSize() const;
   VkImageView swapChainImageView(int idx) const;

 private:
   std::pair<vk::UniqueHandle<vk::Device, vk::DispatchLoaderDynamic>, QVector<vk::Queue>>
      createLogicalDevice(std::pair<const vk::QueueFamilyProperties &, uint32_t> deviceProperties, QStringList extensions);

   bool initialize();
   bool createSurface() const;
   bool populatePhysicalDevices() const;
   bool populateRenderPass() const;
   bool populateSwapChain();

   bool m_isValid;

   int m_concurrentFrameCount;
   int m_currentFrame;
   int m_physicalDeviceIndex;
   int m_requestedSampleCount;
   QStringList m_requestedDeviceExtensions;
   QSize m_swapChainImageSize;

   vk::PhysicalDevice m_physicalDevice;
   bool m_singleDevice;
   vk::UniqueHandle<vk::Device, vk::DispatchLoaderDynamic> m_graphicsDevice;
   vk::UniqueHandle<vk::Device, vk::DispatchLoaderDynamic> m_transferDevice;
   QVector<vk::Queue> m_graphicsQueues;
   QVector<vk::Queue> m_transferQueues;
   uint32_t m_graphicsCommandQueueFamily;
   uint32_t m_transferCommandQueueFamily;
   VulkanFlags m_vulkanFlags;
   mutable vk::UniqueHandle<vk::RenderPass, vk::DispatchLoaderDynamic> m_renderPass;
   vk::UniqueHandle<vk::CommandPool, vk::DispatchLoaderDynamic> m_graphicsPool;

   uint32_t m_hostVisibleIndex;

   vk::Format m_colorFormat;
   vk::Format m_depthFormat;
   vk::ColorSpaceKHR m_colorSpace;
   vk::SampleCountFlagBits m_sampleCount;

   QVector<VkFormat> m_requestedFormats;
   mutable QVector<VkPhysicalDevice> m_physicalDevices;
   mutable QVector<VkPhysicalDeviceProperties> m_physicalDeviceProperties;

   std::unique_ptr<QVulkanDeviceFunctions> m_deviceFunctions;

   QVector<vk::CommandBuffer> m_commandbuffers;
   QVector<std::tuple<vk::Image, QDynamicUniqueHandle<vk::ImageView>, QDynamicUniqueHandle<vk::Framebuffer>>> m_framebuffers;
   QDynamicUniqueHandle<vk::SwapchainKHR> m_swapchain;

   std::unique_ptr<QVulkanWindowRenderer> m_renderer;
   QDynamicUniqueHandle<vk::SurfaceKHR> m_surface;
};

#endif
