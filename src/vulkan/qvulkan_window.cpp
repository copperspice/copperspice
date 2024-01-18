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

#include <qvulkan_window.h>

#include <qmatrix4x4.h>
#include <qvulkan_device_functions.h>
#include <qvulkan_functions.h>
#include <qplatform_window.h>
#include <qplatform_screen.h>

#if defined(Q_OS_UNIX)
#include <xcb/xcb.h>
#endif

namespace {

template <typename T, typename Func, typename... ExtraArgs, template<typename...> typename Container_T>
QVector<std::invoke_result_t<Func, T>> map_vector(const Container_T<T, ExtraArgs...> &data, Func f)
{
   QVector<std::invoke_result_t<Func, T>> retval;
   retval.reserve(data.size());

   for (const auto &item : data) {
      retval.push_back( f(item) );
   }

   return retval;
}

template <typename T, typename Func>
QVector<T> filter_vector(QVector<T> data, Func f)
{
   data.erase(std::remove_if(data.begin(), data.end(),
         [&f] (const T &item) { return ! f(item); }), data.end());

   return data;
}

template <typename T, typename Flags>
QVector<T> filter_sort_queues(QVector<T> data, Flags f)
{
   auto retval = filter_vector(std::move(data),
         [f] (auto &item) {
            auto &[properties, index] = item;
            (void) index;
            return (properties.queueFlags & f) != Flags{};
         });

   std::stable_sort(retval.begin(), retval.end(),
         [] (auto &a, auto &b) {
            return uint32_t(a.first.queueFlags) < uint32_t(b.first.queueFlags);
         });

   return retval;
}

}

QVulkanWindow::QVulkanWindow(QWindow *parent)
   : QWindow(parent), m_isValid(false), m_concurrentFrameCount(MAX_CONCURRENT_FRAME_COUNT), m_currentFrame(0),
     m_physicalDeviceIndex(0), m_requestedSampleCount(1), m_singleDevice(false), m_sampleCount(vk::SampleCountFlagBits::e1)
{
   setSurfaceType(QSurface::VulkanSurface);
}

QVulkanWindow::~QVulkanWindow() = default;

bool QVulkanWindow::initialize()
{
   auto instance = vulkanInstance();

   if (! instance) {
      qWarning("QVulkanWindow::initialize() Called without a valid instance");
      return false;
   }

   if (m_renderer == nullptr) {
      m_renderer.reset(createRenderer());
   }

   if (! populatePhysicalDevices()) {
      qWarning("QVulkanWindow::initialize() Unable to detect a physical device");
      return false;
   }

   if ((m_physicalDeviceIndex < 0) || (m_physicalDeviceIndex >= m_physicalDeviceProperties.size())) {
      m_physicalDeviceIndex = 0;
   }

   if (m_renderer != nullptr) {
      m_renderer->preInitResources();
   }

   if (! m_surface && ! createSurface()) {
      qWarning("QVulkanWindow::initialize() Unable to create surface");
   }

   auto supported = supportedDeviceExtensions();

   QStringList deviceExtensions;

   for (const QString &item : m_requestedDeviceExtensions) {
      for (const auto &ext : supported) {
         if (item == ext.extensionName) {
            deviceExtensions.append(item);
            break;
         }
      }
   }

   auto &physicalDevice = m_physicalDevices[m_physicalDeviceIndex];

   // load queues
   auto queues = map_vector(physicalDevice.getQueueFamilyProperties(),
         [id = 0] (auto item) mutable {
            if (item.queueFlags & (vk::QueueFlagBits::eGraphics | vk::QueueFlagBits::eCompute)) {
               // any queue that supports graphics or compute is required to support transfer by the spec
               item.queueFlags |= vk::QueueFlagBits::eTransfer;
            }

            return std::make_pair(std::move(item), id++);
         });

   auto graphicsQueues = filter_sort_queues(queues, vk::QueueFlagBits::eGraphics);
   auto transferQueues = filter_sort_queues(std::move(queues), vk::QueueFlagBits::eTransfer);

   if (graphicsQueues.empty()) {
      qWarning("QVulkanWindow::initialize() Unable to select a graphics queue");
      return false;
   }

   if (transferQueues.empty()) {
      qWarning("QVulkanWindow::initialize() Unable to select a transfer queue");
      return false;
   }

   try {
      std::tie(m_graphicsDevice, m_graphicsQueues) =
            createLogicalDevice(graphicsQueues.front(), deviceExtensions);

      m_graphicsCommandQueueFamily = graphicsQueues.front().second;
      m_deviceFunctions = instance->deviceFunctions(m_graphicsDevice.get());

      if (graphicsQueues.front() == transferQueues.front()) {
         // unified graphics and transfer queue
         m_singleDevice = true;

      } else {
         // seperate graphics and transfer queues
         std::tie(m_transferDevice, m_transferQueues) =
               createLogicalDevice(transferQueues.front(), deviceExtensions);

         m_transferCommandQueueFamily = transferQueues.front().second;
      }

   } catch (vk::DeviceLostError &err) {
      m_graphicsDevice.reset();
      m_graphicsQueues.clear();
      m_transferDevice.reset();
      m_transferQueues.clear();
      m_physicalDevices.clear();
      m_physicalDeviceProperties.clear();
      return false;

   } catch (vk::SystemError &err) {
      qWarning("QVulkanWindow::initialize() unable to create device: %s", err.what());
      return false;

   }

   vk::CommandPoolCreateInfo poolCreateInfo;

   poolCreateInfo.queueFamilyIndex = m_graphicsCommandQueueFamily;
   m_graphicsPool = m_graphicsDevice->createCommandPoolUnique(poolCreateInfo, nullptr, m_deviceFunctions->dynamicLoader());

   vk::PhysicalDeviceMemoryProperties memoryProperties = physicalDevice.getMemoryProperties(m_deviceFunctions->dynamicLoader());

   uint32_t i = 0;
   std::optional<uint32_t> hostMemIndex;
   std::optional<uint32_t> hostCachedMemIndex;
   std::optional<uint32_t> deviceMemIndex;

   const auto hostBits = vk::MemoryPropertyFlagBits::eHostVisible
         | vk::MemoryPropertyFlagBits::eHostCoherent;

   const auto hostCachedBits = vk::MemoryPropertyFlagBits::eHostVisible
         | vk::MemoryPropertyFlagBits::eHostCoherent;

   const auto deviceBits = vk::MemoryPropertyFlagBits::eDeviceLocal;

   for (const auto &memoryType : memoryProperties.memoryTypes) {

      if (! hostMemIndex.has_value() && ((memoryType.propertyFlags & hostBits) == hostBits)) {
         hostMemIndex = i;
      }

      if (! hostCachedMemIndex.has_value() && ((memoryType.propertyFlags & hostCachedBits) == hostCachedBits)) {
         hostCachedMemIndex = i;
      }

      if (! deviceMemIndex.has_value() && ((memoryType.propertyFlags & deviceBits) == deviceBits)) {
         deviceMemIndex = i;
      }

      ++i;
   }

   // default colorspace and format
   m_colorSpace  = vk::ColorSpaceKHR::eSrgbNonlinear;
   m_colorFormat = vk::Format::eR8G8B8A8Unorm;

   auto surfaceFormats = physicalDevice.getSurfaceFormatsKHR(m_surface.get());

   // use first available format for the surface, if there is one
   for (const auto &item : surfaceFormats) {
      if (item.format != vk::Format::eUndefined) {
         m_colorFormat = item.format;
         m_colorSpace  = item.colorSpace;
         break;
      }
   }

   // find the optimal depth stencil format
   std::array preferredFormatList = { vk::Format::eD24UnormS8Uint, vk::Format::eD16UnormS8Uint, vk::Format::eD32SfloatS8Uint };

   m_depthFormat = vk::Format::eUndefined;

   for (auto item : preferredFormatList) {
      auto formatProperties = physicalDevice.getFormatProperties(item, m_deviceFunctions->dynamicLoader());

      if (formatProperties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eDepthStencilAttachment) {
         m_depthFormat = item;
         break;
      }
   }

   if (! populateRenderPass()) {
      return false;
   }

   if (m_renderer != nullptr) {
      m_renderer->initResources();
   }

   requestUpdate();

   return true;
}

std::pair<vk::UniqueHandle<vk::Device, vk::DispatchLoaderDynamic>, QVector<vk::Queue>>
QVulkanWindow::createLogicalDevice(std::pair<const vk::QueueFamilyProperties &, uint32_t> deviceProperties, QStringList extensions)
{
   auto instance = vulkanInstance();
   vk::UniqueHandle<vk::Device, vk::DispatchLoaderDynamic> device;

   auto &physicalDevice   = m_physicalDevices[m_physicalDeviceIndex];
   auto &[properties, id] = deviceProperties;
   const uint32_t count   = properties.queueCount;

   QVector<float> priorities(count, 1.0f);

   vk::DeviceQueueCreateInfo createInfo({}, id, count, priorities.data());

   QVector<const char *> extensionPointers;
   for (const auto &extensionString : extensions) {
      extensionPointers.append(extensionString.constData());
   }

   if (properties.queueFlags & vk::QueueFlagBits::eGraphics) {
      extensionPointers.push_back("VK_KHR_swapchain");
   }

   device = physicalDevice.createDeviceUnique(vk::DeviceCreateInfo(
         {}, 1, &createInfo, 0, nullptr, extensionPointers.size(), extensionPointers.data()),
         nullptr, instance->dispatchLoader());

   QVector<vk::Queue> queueVector;
   queueVector.reserve(count);

   for (uint32_t i = 0; i < count; ++i) {
      queueVector.push_back(device->getQueue(id, i));
   }

   return {std::move(device), std::move(queueVector)};
}

std::pair<QDynamicUniqueHandle<vk::Image>, QDynamicUniqueHandle<vk::DeviceMemory>>
QVulkanWindow::createTransientImage(vk::ImageCreateFlags imageFlags, vk::ImageUsageFlags usageFlags, vk::Format imageFormat,
      uint32_t imageWidth, uint32_t imageHeight)
{
   auto instance        = vulkanInstance();
   auto &physicalDevice = m_physicalDevices[m_physicalDeviceIndex];
   std::pair<QDynamicUniqueHandle<vk::Image>, QDynamicUniqueHandle<vk::DeviceMemory>> retval;

   vk::ImageCreateInfo createInfo;

   createInfo.flags         = imageFlags;
   createInfo.format        = imageFormat;
   createInfo.imageType     = vk::ImageType::e2D;
   createInfo.extent.width  = imageWidth;
   createInfo.extent.height = imageHeight;
   createInfo.extent.depth  = 1;
   createInfo.mipLevels     = 1;
   createInfo.arrayLayers   = 1;
   createInfo.samples       = m_sampleCount;
   createInfo.tiling        = vk::ImageTiling::eOptimal;
   createInfo.usage         = usageFlags | vk::ImageUsageFlagBits::eTransientAttachment;

   retval.first = m_graphicsDevice->createImageUnique(createInfo, nullptr, instance->dispatchLoader());

   auto memoryRequirements = m_graphicsDevice->getImageMemoryRequirements(retval.first.get());
   auto memoryInfo         = physicalDevice.getMemoryProperties();

   uint32_t memoryIndex;
   for (memoryIndex = 0; memoryIndex < memoryInfo.memoryTypeCount; ++memoryIndex) {
      if (memoryRequirements.memoryTypeBits & (1 << memoryIndex)) {
         break;
      }
   }

   if (memoryIndex == memoryInfo.memoryTypeCount) {
      qWarning("QVulkanWindow::createTransientImage() Unable to find a valid memory type");
      return retval;
   }

   vk::MemoryAllocateInfo allocateInfo;
   allocateInfo.allocationSize  = memoryRequirements.size;
   allocateInfo.memoryTypeIndex = memoryIndex;

   retval.second = m_graphicsDevice->allocateMemoryUnique(allocateInfo, nullptr, instance->dispatchLoader());

   m_graphicsDevice->bindImageMemory(retval.first.get(), retval.second.get(), 0);

   return retval;
}

bool QVulkanWindow::createSurface() const
{
#if defined(Q_OS_WIN)
   // windows platform specific code

   vk::Win32SurfaceCreateInfoKHR createInfo;

   createInfo.hinstance = GetModuleHandle(nullptr);
   createInfo.hwnd = *reinterpret_cast<HWND*>(handle()->nativeHandle());

   m_surface = vulkanInstance()->apiInstance().createWin32SurfaceKHRUnique(createInfo, nullptr, vulkanInstance()->dispatchLoader());

#elif defined(Q_OS_UNIX)
   // unix platform specific code

   vk::XcbSurfaceCreateInfoKHR createInfo;

   createInfo.connection = reinterpret_cast<xcb_connection_t*>(handle()->screen()->nativeHandle());
   createInfo.window     = *reinterpret_cast<xcb_window_t*>(handle()->nativeHandle());

   m_surface = vulkanInstance()->apiInstance().createXcbSurfaceKHRUnique(createInfo, nullptr, vulkanInstance()->dispatchLoader());

#endif

   return true;
}

bool QVulkanWindow::populatePhysicalDevices() const
{
   QVector<VkPhysicalDeviceProperties> properties;
   QVector<vk::PhysicalDevice> devices;

   if (! m_physicalDevices.empty()) {
      // already populated
      return true;
   }

   QVulkanInstance *instance = vulkanInstance();
   if (instance == nullptr) {
      qWarning("QVulkanWindow: Unable to retrieve a physical device before an instance is created");
      return false;
   }

   try {
      auto tmp = instance->apiInstance().enumeratePhysicalDevices(instance->dispatchLoader());

      for (auto & item : tmp) {
         devices.append(std::move(item));
      }

   } catch (vk::SystemError &err) {
      qWarning("QVulkanWindow: Call to enumeratePhysicalDevices() failed, %s", err.what());
      return false;
   }

   for (auto &item : devices) {
      properties.append(item.getProperties(instance->dispatchLoader()));
   }

   m_physicalDevices = devices;
   m_physicalDeviceProperties = properties;

   return true;
}

bool QVulkanWindow::populateRenderPass() const
{
   if (m_renderPass) {
      // already populated
      return true;
   }

   bool multisampleEnabled = false;

   if (m_requestedSampleCount > 1) {
      multisampleEnabled = true;
   }

   QVector<vk::AttachmentDescription> attachments;

   attachments.append(vk::AttachmentDescription(
      vk::AttachmentDescriptionFlagBits{}, m_colorFormat, vk::SampleCountFlagBits::e1,
      vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore,
      vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare,
      vk::ImageLayout::eUndefined, vk::ImageLayout::ePresentSrcKHR));

   attachments.append(vk::AttachmentDescription(
      vk::AttachmentDescriptionFlagBits{}, m_depthFormat, m_sampleCount,
      vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eDontCare,
      vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eDontCare,
      vk::ImageLayout::eUndefined,  vk::ImageLayout::eDepthStencilAttachmentOptimal));

   uint32_t colorId = 0;

   if (multisampleEnabled) {
      attachments.append(vk::AttachmentDescription(
         vk::AttachmentDescriptionFlagBits{}, m_colorFormat, m_sampleCount,
         vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore,
         vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare,
         vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal));
      colorId = 2;
   }

   vk::AttachmentReference colorAttachment(colorId, vk::ImageLayout::eColorAttachmentOptimal);
   vk::AttachmentReference depthAttachment(1, vk::ImageLayout::eDepthStencilAttachmentOptimal);
   vk::AttachmentReference multisampleAttachment(0, vk::ImageLayout::eColorAttachmentOptimal);

   vk::SubpassDescription subPass;
   subPass.pipelineBindPoint       = vk::PipelineBindPoint::eGraphics;
   subPass.colorAttachmentCount    = 1;
   subPass.pColorAttachments       = &colorAttachment;
   subPass.pDepthStencilAttachment = &depthAttachment;

   if (multisampleEnabled) {
      subPass.pResolveAttachments = &multisampleAttachment;
   }

   vk::RenderPassCreateInfo passInfo;
   passInfo.attachmentCount = attachments.size();
   passInfo.pAttachments    = attachments.data();
   passInfo.subpassCount    = 1;
   passInfo.pSubpasses      = &subPass;

   auto pass = m_deviceFunctions->device().createRenderPassUnique(passInfo, nullptr, m_deviceFunctions->dynamicLoader());

   if (! pass) {
      qWarning("QVulkanWindow: Unable to create render pass");
      return false;
   }

   m_renderPass = std::move(pass);

   return true;
}

bool QVulkanWindow::recreateSwapChain()
{
   // trigger a recreation of all resources
   m_swapChainImageSize = QSize();

   if (m_renderer) {
      m_renderer->releaseSwapChainResources();
   }

   return populateSwapChain();
}

bool QVulkanWindow::populateSwapChain()
{
   m_swapChainImageSize = size() * devicePixelRatio();

   if (m_swapChainImageSize.isEmpty()) {
      return false;
   }

   try {
      m_deviceFunctions->device().waitIdle();

   } catch (vk::OutOfDateKHRError &err) {
      return false;

   } catch (vk::DeviceLostError &err) {
      return false;

   } catch (vk::SystemError &err) {
      return false;

   }

   uint32_t numBuffers  = 3;
   auto &physicalDevice = m_physicalDevices[m_physicalDeviceIndex];

   if (! physicalDevice.getSurfaceSupportKHR(m_graphicsCommandQueueFamily, m_surface.get())) {
      return false;
   }

   vk::SurfaceCapabilitiesKHR capabilities;

   try {
      capabilities = physicalDevice.getSurfaceCapabilitiesKHR(m_surface.get());

   } catch (vk::OutOfDateKHRError &err) {
      return false;

   } catch (vk::DeviceLostError &err) {
      return false;

   } catch (vk::SystemError &err) {
      return false;

   }

   if (capabilities.maxImageCount != 0) {
      numBuffers = std::min(numBuffers, capabilities.maxImageCount);
   }

   numBuffers = std::max(numBuffers, capabilities.minImageCount);
   m_frameData.resize(numBuffers);

   for (auto &item : m_frameData) {
      item.frameFence     = m_graphicsDevice->createFence(vk::FenceCreateInfo{});
      item.imageFence     = m_graphicsDevice->createFence(vk::FenceCreateInfo{});
      item.frameSemaphore = m_graphicsDevice->createSemaphore(vk::SemaphoreCreateInfo{});
      item.imageSemaphore = m_graphicsDevice->createSemaphore(vk::SemaphoreCreateInfo{});
   }

   auto vk_size = capabilities.currentExtent;

   // compute Vulkan extent
   if (vk_size.width == 0xFFFFFFFF) {
      if (vk_size.height != 0xFFFFFFFF) {
         qWarning("QVulkanWindow::populateSwapChain() Unable to set extent, height was valid but width was invalid");
         return false;
      }

      vk_size.width  = m_swapChainImageSize.width();
      vk_size.height = m_swapChainImageSize.height();
   }

   // save extent back to QSize
   m_swapChainImageSize = QSize(vk_size.width, vk_size.height);

   vk::SwapchainCreateInfoKHR swapchainInfo;

   swapchainInfo.surface          = m_surface.get();
   swapchainInfo.minImageCount    = numBuffers;
   swapchainInfo.imageFormat      = m_colorFormat;
   swapchainInfo.imageColorSpace  = m_colorSpace;
   swapchainInfo.imageExtent      = vk_size;
   swapchainInfo.imageArrayLayers = 1;
   swapchainInfo.imageUsage       = vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferDst;
   swapchainInfo.imageSharingMode = vk::SharingMode::eExclusive;
   swapchainInfo.preTransform     = capabilities.currentTransform;
   swapchainInfo.compositeAlpha   = vk::CompositeAlphaFlagBitsKHR::eOpaque;
   swapchainInfo.presentMode      = vk::PresentModeKHR::eFifo;
   swapchainInfo.clipped          = true;
   swapchainInfo.oldSwapchain     = m_swapchain.get();

   try {
      m_swapchain    = m_graphicsDevice->createSwapchainKHRUnique(swapchainInfo, nullptr, m_deviceFunctions->dynamicLoader());
      auto vk_images = m_deviceFunctions->device().getSwapchainImagesKHR(m_swapchain.get());

      auto createFramebuffer =
            [this, vk_size](auto vk_image) {
               auto view = m_deviceFunctions->device().createImageViewUnique(
                     vk::ImageViewCreateInfo{
                     { }, vk_image, vk::ImageViewType::e2D, m_colorFormat,
                     {vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eG,
                     vk::ComponentSwizzle::eB, vk::ComponentSwizzle::eA},
                     {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1} },
                     nullptr, m_deviceFunctions->dynamicLoader());

               auto depthPair = createTransientImage(vk::ImageCreateFlags{},
                     vk::ImageUsageFlagBits::eDepthStencilAttachment, m_depthFormat, vk_size.width, vk_size.height);

               auto depthView = m_deviceFunctions->device().createImageViewUnique(
                     vk::ImageViewCreateInfo{
                     { }, depthPair.first.get(), vk::ImageViewType::e2D, m_depthFormat,
                     {vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eG,
                     vk::ComponentSwizzle::eB, vk::ComponentSwizzle::eA},
                     {vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil, 0, 1, 0, 1} },
                     nullptr, m_deviceFunctions->dynamicLoader());

               decltype(depthPair) multisamplePair;
               decltype(depthView) multisampleView;

               std::vector<vk::ImageView> tmp_images;
               tmp_images.push_back(*view);
               tmp_images.push_back(*depthView);

               if (m_requestedSampleCount > 1) {
                  multisamplePair = createTransientImage(vk::ImageCreateFlags{},
                        vk::ImageUsageFlagBits::eColorAttachment, m_colorFormat, vk_size.width, vk_size.height);

                  multisampleView = m_deviceFunctions->device().createImageViewUnique(
                     vk::ImageViewCreateInfo{
                     { }, multisamplePair.first.get(), vk::ImageViewType::e2D, m_colorFormat,
                     {vk::ComponentSwizzle::eR, vk::ComponentSwizzle::eG,
                     vk::ComponentSwizzle::eB, vk::ComponentSwizzle::eA},
                     {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1} },
                     nullptr, m_deviceFunctions->dynamicLoader());


                  tmp_images.push_back(*multisampleView);
               }

               auto framebuffer = m_deviceFunctions->device().createFramebufferUnique(vk::FramebufferCreateInfo(
                     {}, m_renderPass.get(), uint32_t(tmp_images.size()), tmp_images.data(),
                     m_swapChainImageSize.width(), m_swapChainImageSize.height(), 1),
                     nullptr, m_deviceFunctions->dynamicLoader());

               return std::make_tuple(std::move(vk_image), std::move(view), std::move(framebuffer),
                     std::move(depthPair.first), std::move(depthPair.second), std::move(depthView),
                     std::move(multisamplePair.first), std::move(multisamplePair.second), std::move(multisampleView));
            };

      m_framebuffers = map_vector(vk_images, createFramebuffer);

   } catch (vk::SystemError &err) {
      qWarning("QVulkanWindow::populateSwapChain() Unable to create device: %s", err.what());
      return false;

   }

   if (m_renderer != nullptr) {
      m_renderer->initSwapChainResources();
   }

   return true;
}

QVector<VkPhysicalDeviceProperties> QVulkanWindow::availablePhysicalDevices()
{
   if (! m_physicalDeviceProperties.empty()) {
      return m_physicalDeviceProperties;
   }

   if (! populatePhysicalDevices()) {
      return QVector<VkPhysicalDeviceProperties>();
   }

   return m_physicalDeviceProperties;
}

bool QVulkanWindow::event(QEvent *e)
{
   switch(e->type()) {
      case QEvent::UpdateRequest:
         startFrame();
         break;

      default:
         break;
   }

   return QWindow::event(e);
}

void QVulkanWindow::startFrame()
{
   // ensure we have a working swapchain
   if (! m_swapchain || (size() * devicePixelRatio() != m_swapChainImageSize)) {
      if (! recreateSwapChain()) {
         return;
      }
   }

   auto frameData = m_frameData.begin() + m_currentFrame;

   if (frameData->frameFenceActive) {
      // make sure previous operations on this frame have completed

      vk::Result result = m_deviceFunctions->device().waitForFences(1, &frameData->frameFence, true,
            std::numeric_limits<uint64_t>::max());
      if (result != vk::Result::eSuccess) {
         return;
      }

      result = m_deviceFunctions->device().resetFences(1, &frameData->frameFence);
      if (result != vk::Result::eSuccess) {
         return;
      }

      frameData->frameFenceActive = false;

   }

   if (! frameData->imageAcquired) {

      // get next image
      auto imageResult = m_deviceFunctions->device().acquireNextImageKHR
         (m_swapchain.get(), std::numeric_limits<uint64_t>::max(), frameData->imageSemaphore, frameData->frameFence);

      switch (imageResult.result) {
         case vk::Result::eSuccess:
         case vk::Result::eSuboptimalKHR:
            frameData->imageSemaphoreActive = true;
            frameData->imageAcquired        = true;
            frameData->frameFenceActive     = true;
            m_imageIndex                    = imageResult.value;
            break;

         case vk::Result::eErrorOutOfDateKHR:
            // stale swapchain, regenerate and try again next frame
            recreateSwapChain();
            requestUpdate();
            break;

         case vk::Result::eErrorDeviceLost:
            handleDeviceLost();
            break;

         default:
            // some other error occurred, drop this frame
            requestUpdate();
            break;

      }
   }

   if (frameData->imageFenceActive)  {
      // make sure previous operations on this image have completed
      vk::Result result = m_deviceFunctions->device().waitForFences(1, &frameData->imageFence, true, std::numeric_limits<uint64_t>::max());
      if (result != vk::Result::eSuccess) {
         return;
      }

      result = m_deviceFunctions->device().resetFences(1, &frameData->imageFence);
      if (result != vk::Result::eSuccess) {
         return;
      }

      frameData->imageFenceActive = false;
   }

   vk::CommandBufferAllocateInfo allocateInfo;
   allocateInfo.commandPool        = m_graphicsPool.get();
   allocateInfo.level              = vk::CommandBufferLevel::ePrimary;
   allocateInfo.commandBufferCount = 1;

   try {
      auto commandBufferList   = m_deviceFunctions->device().allocateCommandBuffersUnique(allocateInfo, m_deviceFunctions->dynamicLoader());
      frameData->commandBuffer = std::move(commandBufferList[0]);

      frameData->commandBuffer.value()->begin(vk::CommandBufferBeginInfo{});

   } catch (vk::DeviceLostError &err) {
      handleDeviceLost();
      return;

   } catch (vk::SystemError &err) {
      return;

   }

   if (m_renderer) {
      m_renderer->startNextFrame();

   } else {
      // no renderer available, clear the images
      vk::ClearValue clearColor[3];
      vk::ClearColorValue clearBlack{std::array{0.0f, 0.0f, 0.0f, 1.0f}};
      vk::ClearDepthStencilValue clearDepth{1.0f, 0};

      clearColor[0].color        = clearBlack;
      clearColor[1].depthStencil = clearDepth;
      clearColor[2].color        = clearBlack;

      vk::RenderPassBeginInfo renderPassInfo;
      renderPassInfo.renderPass               = m_renderPass.get();
      renderPassInfo.framebuffer              = std::get<2>(m_framebuffers[m_currentFrame]).get();
      renderPassInfo.renderArea.extent.width  = m_swapChainImageSize.width();
      renderPassInfo.renderArea.extent.height = m_swapChainImageSize.height();
      renderPassInfo.pClearValues             = clearColor;
      renderPassInfo.clearValueCount          = 2;

      if (m_requestedSampleCount > 1) {
         renderPassInfo.clearValueCount = 3;
      }

      frameData->commandBuffer.value()->beginRenderPass(renderPassInfo, vk::SubpassContents::eInline,  m_deviceFunctions->dynamicLoader());
      frameData->commandBuffer.value()->endRenderPass(m_deviceFunctions->dynamicLoader());

      endFrame();
   }
}

void QVulkanWindow::endFrame()
{
   auto frameData = m_frameData.begin() + m_currentFrame;
   frameData->commandBuffer.value()->end();

   vk::SubmitInfo submitInfo;
   submitInfo.commandBufferCount   = 1;
   submitInfo.signalSemaphoreCount = 1;
   submitInfo.pCommandBuffers      = &(frameData->commandBuffer.value().get());
   submitInfo.pSignalSemaphores    = &(frameData->frameSemaphore);

   vk::PipelineStageFlags pipelineFlags = vk::PipelineStageFlagBits::eColorAttachmentOutput;
   submitInfo.pWaitDstStageMask         = &pipelineFlags;

   if (frameData->imageSemaphoreActive) {
      submitInfo.waitSemaphoreCount = 1;
      submitInfo.pWaitSemaphores    = &(frameData->imageSemaphore);
   }

   vk::Result result = m_graphicsQueues.first().submit(1, &submitInfo, frameData->imageFence);

   if (result == vk::Result::eErrorDeviceLost) {
      handleDeviceLost();
      return;

   } else if (result != vk::Result::eSuccess) {
      return;

   }

   frameData->imageSemaphoreActive = false;
   frameData->imageFenceActive     = true;

   vk::PresentInfoKHR presentInfo;
   presentInfo.swapchainCount     = 1;
   presentInfo.waitSemaphoreCount = 1;
   presentInfo.pSwapchains        = &(m_swapchain.get());
   presentInfo.pImageIndices      = &m_imageIndex;
   presentInfo.pWaitSemaphores    = &(frameData->frameSemaphore);

   try {
      vulkanInstance()->presentAboutToBeQueued(this);
      (void) m_graphicsQueues.first().presentKHR(presentInfo);

   } catch (vk::OutOfDateKHRError &err) {
      recreateSwapChain();
      return;

   } catch (vk::DeviceLostError &err) {
      handleDeviceLost();
      return;

   } catch (vk::SystemError &err) {
      return;

   }

   frameData->imageAcquired = false;
   vulkanInstance()->presentQueued(this);
   m_currentFrame = (m_currentFrame + 1) % m_frameData.size();
}

void QVulkanWindow::exposeEvent(QExposeEvent *)
{
   if (isExposed()) {
      if (m_graphicsQueues.isEmpty()) {
         initialize();
      }

   } else {
      if (m_renderer) {
         m_renderer->releaseSwapChainResources();
         m_renderer->releaseResources();
      }

   }
}

bool QVulkanWindow::handleDeviceLost()
{
   if (m_renderer != nullptr) {
      m_renderer->logicalDeviceLost();
      m_renderer->releaseSwapChainResources();
      m_renderer->releaseResources();
   }

   return recreateSwapChain();
}

QMatrix4x4 QVulkanWindow::clipCorrectionMatrix()
{
   static const QMatrix4x4 retval = {1.0,  0.0,  0.0,  0.0,
                                     0.0, -1.0,  0.0,  0.0,
                                     0.0,  0.0,  0.5,  0.5,
                                     0.0,  0.0,  0.0,  1.0};

   return retval;
}

VkFormat QVulkanWindow::colorFormat() const
{
   return VkFormat(m_colorFormat);
}

int QVulkanWindow::concurrentFrameCount() const
{
   return m_concurrentFrameCount;
}

QVulkanWindowRenderer *QVulkanWindow::createRenderer()
{
   return nullptr;
}

VkCommandBuffer QVulkanWindow::currentCommandBuffer() const
{
   if (m_currentFrame >= m_frameData.size()) {
      return nullptr;
   }

   return m_frameData[m_currentFrame].commandBuffer.value().get();
}

int QVulkanWindow::currentFrame() const
{
   return m_currentFrame;
}

VkFramebuffer QVulkanWindow::currentFramebuffer() const
{
   if (m_currentFrame >= m_framebuffers.size()) {
      return nullptr;
   }

   return std::get<2>(m_framebuffers[m_currentFrame]).get();
}

int QVulkanWindow::currentSwapChainImageIndex() const
{
   return m_imageIndex;
}

VkRenderPass QVulkanWindow::defaultRenderPass() const
{
   if (! populateRenderPass()) {
      return nullptr;
   }

   return m_renderPass.get();
}

VkFormat QVulkanWindow::depthStencilFormat() const
{
   return VkFormat(m_depthFormat);
}

VkImage QVulkanWindow::depthStencilImage() const
{
   if (m_imageIndex >= m_framebuffers.size()) {
      return nullptr;
   }

   return std::get<3>(m_framebuffers[m_imageIndex]).get();
}

VkImageView QVulkanWindow::depthStencilImageView() const
{
   if (m_imageIndex >= m_framebuffers.size()) {
      return nullptr;
   }

   return std::get<5>(m_framebuffers[m_imageIndex]).get();
}

VkDevice QVulkanWindow::device() const
{
   return m_graphicsDevice.get();
}

uint32_t QVulkanWindow::deviceLocalMemoryIndex() const
{
   auto &physicalDevice = m_physicalDevices[m_physicalDeviceIndex];
   auto memoryInfo      = physicalDevice.getMemoryProperties();
   uint32_t memoryIndex;
   uint32_t retval = 0;

   const auto flags  = vk::MemoryPropertyFlagBits::eDeviceLocal;

   for (memoryIndex = 0; memoryIndex < memoryInfo.memoryTypeCount; ++memoryIndex) {
      if ((memoryInfo.memoryTypes[memoryIndex].propertyFlags & flags) == flags) {
         retval = memoryIndex;
         break;
      }
   }

   return retval;
}

QVulkanWindow::VulkanFlags QVulkanWindow::flags() const
{
   return m_vulkanFlags;
}

void QVulkanWindow::frameReady()
{
   endFrame();
}

VkCommandPool QVulkanWindow::graphicsCommandPool() const
{
   return VkCommandPool(m_graphicsPool.get());
}

VkQueue QVulkanWindow::graphicsQueue() const
{
   if (m_graphicsQueues.empty()) {
      return nullptr;
   }

   return VkQueue(m_graphicsQueues.first());
}

uint32_t QVulkanWindow::graphicsQueueFamilyIndex() const
{
   return m_graphicsCommandQueueFamily;
}

uint32_t QVulkanWindow::hostVisibleMemoryIndex() const
{
   auto &physicalDevice = m_physicalDevices[m_physicalDeviceIndex];
   auto memoryInfo      = physicalDevice.getMemoryProperties();
   uint32_t memoryIndex;
   uint32_t retval = 0;

   const auto desiredFlags  = vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible
      | vk::MemoryPropertyFlagBits::eHostCached;
   const auto requiredFlags = vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible;

   for (memoryIndex = 0; memoryIndex < memoryInfo.memoryTypeCount; ++memoryIndex) {
      if ((memoryInfo.memoryTypes[memoryIndex].propertyFlags & desiredFlags) == desiredFlags) {
         // found a memory type with all desired flags
         retval = memoryIndex;
         break;

      } else if ((memoryInfo.memoryTypes[memoryIndex].propertyFlags & requiredFlags) == requiredFlags) {
         // found a memory type with required flags, store and keep looking
         retval = memoryIndex;

      }

   }

   return retval;
}

bool QVulkanWindow::isValid() const
{
   return m_isValid;
}

VkImage QVulkanWindow::msaaColorImage(int idx) const
{
   if (idx >= m_framebuffers.size()) {
      return nullptr;
   }

   return std::get<6>(m_framebuffers[idx]).get();
}

VkImageView QVulkanWindow::msaaColorImageView(int idx) const
{
   if (idx >= m_framebuffers.size()) {
      return nullptr;
   }

   return std::get<8>(m_framebuffers[idx]).get();
}

VkPhysicalDevice QVulkanWindow::physicalDevice() const
{
   if (! populatePhysicalDevices()) {
      return nullptr;
   }

   if ((m_physicalDeviceIndex < 0) || (m_physicalDeviceIndex >= m_physicalDevices.size())) {
      qWarning("QVulkanWindow::physicalDevice() Index %d is out of range", m_physicalDeviceIndex);
      return nullptr;
   }

   return m_physicalDevices[m_physicalDeviceIndex];
}

const VkPhysicalDeviceProperties *QVulkanWindow::physicalDeviceProperties() const
{
   if (! populatePhysicalDevices()) {
      return nullptr;
   }

   if ((m_physicalDeviceIndex < 0) || (m_physicalDeviceIndex >= m_physicalDeviceProperties.size())) {
      qWarning("QVulkanWindow::physicalDeviceProperties() Index %d is out of range", m_physicalDeviceIndex);
      return nullptr;
   }

   return &(m_physicalDeviceProperties[m_physicalDeviceIndex]);
}

VkSampleCountFlagBits QVulkanWindow::sampleCountFlagBits() const
{
   return static_cast<VkSampleCountFlagBits>(m_sampleCount);
}

void QVulkanWindow::setDeviceExtensions(const QStringList &extensions)
{
   m_requestedDeviceExtensions = extensions;
}

void QVulkanWindow::setFlags(QVulkanWindow::VulkanFlags flags)
{
   m_vulkanFlags = flags;
}

void QVulkanWindow::setPhysicalDeviceIndex(int idx)
{
   auto deviceList = availablePhysicalDevices();

   if (idx >= 0 && idx < deviceList.size()) {
      m_physicalDeviceIndex = idx;
   }
}

void QVulkanWindow::setPreferredColorFormats(const QVector<VkFormat> &formats)
{
   m_requestedFormats = formats;
}

void QVulkanWindow::setSampleCount(int sampleCount)
{
   m_requestedSampleCount = sampleCount;

   if (m_requestedSampleCount < 1) {
      m_requestedSampleCount = 1;
   }

   switch (m_requestedSampleCount) {
      case 1:
         m_sampleCount = vk::SampleCountFlagBits::e1;
         break;

      case 2:
         m_sampleCount = vk::SampleCountFlagBits::e2;
         break;

      case 4:
         m_sampleCount = vk::SampleCountFlagBits::e4;
         break;

      case 8:
         m_sampleCount = vk::SampleCountFlagBits::e8;
         break;

      case 16:
         m_sampleCount = vk::SampleCountFlagBits::e16;
         break;

      case 32:
         m_sampleCount = vk::SampleCountFlagBits::e32;
         break;

      case 64:
         m_sampleCount = vk::SampleCountFlagBits::e64;
         break;
   }
}

QVector<QVulkanExtensionProperties> QVulkanWindow::supportedDeviceExtensions()
{
   QVector<QVulkanExtensionProperties> retval;

   if (! populatePhysicalDevices()) {
      return retval;
   }

   QVector<VkExtensionProperties> extensionProperties;

   auto &physicalDevice      = m_physicalDevices[m_physicalDeviceIndex];
   uint32_t extensionCount   = 0;
   QVulkanInstance *instance = vulkanInstance();
   QVulkanFunctions *f       = instance->functions();

   VkResult result = f->vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, nullptr);

   if (result != VK_SUCCESS) {
      return retval;
   }

   extensionProperties.resize(extensionCount);

   result = f->vkEnumerateDeviceExtensionProperties(physicalDevice, nullptr, &extensionCount, extensionProperties.data());

   if (result != VK_SUCCESS) {
      return retval;
   }

   for (const auto &item : extensionProperties) {
      retval.append(QVulkanExtensionProperties{item.specVersion, QString::fromUtf8(item.extensionName)});
   }

   return retval;
}

QVector<int> QVulkanWindow::supportedSampleCounts()
{
   QVector<int> retval;

   if (! populatePhysicalDevices()) {
      qWarning("QVulkanWindow::supportedSampleCounts() Unable to detect a physical device");
      return retval;
   }


   auto &physicalDevice = m_physicalDevices[m_physicalDeviceIndex];
   auto properties = physicalDevice.getProperties();
   static const QVector<std::pair<vk::SampleCountFlagBits, int>> supportedSampleBits =
      { {vk::SampleCountFlagBits::e1, 1},
        {vk::SampleCountFlagBits::e2, 2},
        {vk::SampleCountFlagBits::e4, 4},
        {vk::SampleCountFlagBits::e8, 8},
        {vk::SampleCountFlagBits::e16, 16},
        {vk::SampleCountFlagBits::e32, 32},
        {vk::SampleCountFlagBits::e64, 64},
      };

   for (const auto &item : supportedSampleBits) {
      if (((properties.limits.framebufferColorSampleCounts & item.first) == item.first) &&
          ((properties.limits.framebufferDepthSampleCounts & item.first) == item.first) &&
          ((properties.limits.framebufferStencilSampleCounts & item.first) == item.first)) {

         retval.append(item.second);
      }
   }

   return retval;
}

VkSurfaceKHR QVulkanWindow::vulkanSurface() const {

   if (! m_surface) {
      createSurface();
   }

   return m_surface.get();
}

VkImage QVulkanWindow::swapChainImage(int idx) const
{
   if (idx >= m_framebuffers.size()) {
      return nullptr;
   }

   return std::get<0>(m_framebuffers[idx]);
}

int QVulkanWindow::swapChainImageCount() const
{
   return m_framebuffers.count();
}

QSize QVulkanWindow::swapChainImageSize() const
{
   return m_swapChainImageSize;
}

VkImageView QVulkanWindow::swapChainImageView(int idx) const
{
   if (idx >= m_framebuffers.size()) {
      return nullptr;
   }

   return std::get<1>(m_framebuffers[idx]).get();
}
