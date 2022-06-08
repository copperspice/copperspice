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

#include <qvulkan_instance.h>
#include <qvulkan_functions.h>

#include <qapplication.h>

QVulkanInstance::QVulkanInstance()
   : m_errorCode(VK_SUCCESS)
{
   PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = m_dl.getProcAddress<PFN_vkGetInstanceProcAddr>("vkGetInstanceProcAddr");
   m_dld.init(vkGetInstanceProcAddr);
}

QVulkanInstance::~QVulkanInstance()
{
}

QVulkanFunctions *QVulkanInstance::functions() const
{
   if (m_functions == nullptr) {
      m_functions.reset(new QVulkanFunctions(*m_vkInstance, m_dld));
   }

   return m_functions.get();
}

VkInstance QVulkanInstance::vkInstance() const
{
   return *m_vkInstance;
}
