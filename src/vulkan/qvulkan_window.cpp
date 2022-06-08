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


QVulkanWindow::QVulkanWindow(QWindow* parent)
   : QWindow(parent), m_concurrentFrameCount(MAX_CONCURRENT_FRAME_COUNT), m_currentFrame(0),
     m_physicalDeviceIndex(0), m_requestedSampleCount(1)
{
   setSurfaceType(QSurface::VulkanSurface);
}

QVulkanWindow::~QVulkanWindow() = default;


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

int QVulkanWindow::currentFrame() const
{
   return m_currentFrame;
}

QVulkanWindow::VulkanFlags QVulkanWindow::flags() const
{
   return m_vulkanFlags;
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
