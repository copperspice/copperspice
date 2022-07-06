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

#include <qvulkan_window.h>

#include <qmatrix4x4.h>
#include <qvulkan_device_functions.h>
#include <qvulkan_functions.h>

namespace {

template <typename T, typename Func, template<typename> typename Container_T>
QVector<std::invoke_result_t<Func, T>> map_vector(const Container_T<T> &data, Func f)
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

};

QVulkanWindow::QVulkanWindow(QWindow *parent)
   : QWindow(parent), m_isValid(false), m_concurrentFrameCount(MAX_CONCURRENT_FRAME_COUNT), m_currentFrame(0),
     m_physicalDeviceIndex(0), m_requestedSampleCount(1)
{
   setSurfaceType(QSurface::VulkanSurface);
}

QVulkanWindow::~QVulkanWindow() = default;

bool QVulkanWindow::populatePhysicalDevices() const
{
   QVector<VkPhysicalDeviceProperties> properties;
   QVector<VkPhysicalDevice> devices;
   uint32_t deviceCount;

   if (! m_physicalDevices.empty()) {
      // already populated
      return true;
   }

   QVulkanInstance *instance = vulkanInstance();
   if (instance == nullptr) {
      qWarning("QVulkanWindow: Unable to retrieve a physical device before an instance is created");
      return false;
   }

   QVulkanFunctions *f = instance->functions();
   VkResult result = f->vkEnumeratePhysicalDevices(instance->vkInstance(), &deviceCount, nullptr);

   if (result != VK_SUCCESS || deviceCount < 1) {
      qWarning("QVulkanWindow: First call to vkEnumeratePhysicalDevices() failed");
      return false;
   }

   devices.resize(deviceCount);
   result = f->vkEnumeratePhysicalDevices(instance->vkInstance(), &deviceCount, devices.data());

   if (result != VK_SUCCESS || deviceCount == 0) {
      qWarning("QVulkanWindow: Second call to vkEnumeratePhysicalDevices() failed");
      return false;
   }

   properties.resize(deviceCount);
   for (uint32_t i = 0; i < deviceCount; ++i) {
      f->vkGetPhysicalDeviceProperties(devices[i], properties.data() + i);
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
                   vk::AttachmentLoadOp::eLoad, vk::AttachmentStoreOp::eStore,
                   vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare,
                   vk::ImageLayout::eUndefined, vk::ImageLayout::ePresentSrcKHR));

   attachments.append(vk::AttachmentDescription(
                   vk::AttachmentDescriptionFlagBits{}, m_depthFormat, vk::SampleCountFlagBits::e1,
                   vk::AttachmentLoadOp::eLoad, vk::AttachmentStoreOp::eStore,
                   vk::AttachmentLoadOp::eDontCare, vk::AttachmentStoreOp::eDontCare,
                   vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal));

   uint32_t colorId = 0;

   if (multisampleEnabled) {
      attachments.append(vk::AttachmentDescription(
                   vk::AttachmentDescriptionFlagBits{}, m_colorFormat, m_sampleCount,
                   vk::AttachmentLoadOp::eLoad, vk::AttachmentStoreOp::eStore,
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
   }

   m_renderPass = std::move(pass);

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

QMatrix4x4 QVulkanWindow::clipCorrectionMatrix()
{
   static const QMatrix4x4 retval = {1.0,  0.0,  0.0,  0.0,
                                     0.0, -1.0,  0.0,  0.0,
                                     0.0,  0.0,  0.5,  0.5,
                                     0.0,  0.0,  0.0,  1.0};

   return retval;
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
   if (m_currentFrame < m_commandbuffers.size()) {
      return nullptr;
   }

   return m_commandbuffers[m_currentFrame];
}

int QVulkanWindow::currentFrame() const
{
   return m_currentFrame;
}

VkFramebuffer QVulkanWindow::currentFramebuffer() const
{
   if (m_currentFrame < m_framebuffers.size()) {
      return nullptr;
   }

   return m_framebuffers[m_currentFrame];
}

VkRenderPass QVulkanWindow::defaultRenderPass() const
{
   if (! populateRenderPass()) {
      return nullptr;
   }

   return m_renderPass.get();
}

VkDevice QVulkanWindow::device() const
{
   return m_device;
}

QVulkanWindow::VulkanFlags QVulkanWindow::flags() const
{
   return m_vulkanFlags;
}

bool QVulkanWindow::isValid() const
{
   return m_isValid;
}

VkPhysicalDevice QVulkanWindow::physicalDevice() const
{
   if (! populatePhysicalDevices()) {
      return nullptr;
   }

   if ((m_physicalDeviceIndex < 0) || (m_physicalDeviceIndex >= m_physicalDevices.size())) {
      qWarning("QVulkanWindow::physicalDevice() Index %d is out of range", m_physicalDeviceIndex);
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

void QVulkanWindow::setSampleCount(int sampleCount)
{
   m_requestedSampleCount = sampleCount;

   if (m_requestedSampleCount < 1) {
      m_requestedSampleCount = 1;
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
