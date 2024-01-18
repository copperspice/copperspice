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

#include <camera_resourcepolicy.h>
#include <qdebug.h>
#include <qset.h>

#ifdef HAVE_RESOURCE_POLICY
#include <policy/resource.h>
#include <policy/resources.h>
#include <policy/resource-set.h>
#endif

// #define DEBUG_RESOURCE_POLICY

CamerabinResourcePolicy::CamerabinResourcePolicy(QObject *parent)
   : QObject(parent), m_resourceSet(NoResources), m_releasingResources(false), m_canCapture(false)
{
#ifdef HAVE_RESOURCE_POLICY
   //loaded resource set is also kept requested for image and video capture sets
   m_resource = new ResourcePolicy::ResourceSet("camera");
   m_resource->setAlwaysReply();
   m_resource->initAndConnect();

   connect(m_resource, &ResourcePolicy::ResourceSet::resourcesGranted,         this, &CamerabinResourcePolicy::handleResourcesGranted);
   connect(m_resource, &ResourcePolicy::ResourceSet::resourcesDenied,          this, &CamerabinResourcePolicy::resourcesDenied);
   connect(m_resource, &ResourcePolicy::ResourceSet::lostResources,            this, &CamerabinResourcePolicy::handleResourcesLost);
   connect(m_resource, &ResourcePolicy::ResourceSet::resourcesReleased,        this, &CamerabinResourcePolicy::handleResourcesReleased);
   connect(m_resource, &ResourcePolicy::ResourceSet::resourcesBecameAvailable, this, &CamerabinResourcePolicy::resourcesAvailable);
   connect(m_resource, &ResourcePolicy::ResourceSet::updateOK,                 this, &CamerabinResourcePolicy::updateCanCapture);

#endif
}

CamerabinResourcePolicy::~CamerabinResourcePolicy()
{
#ifdef HAVE_RESOURCE_POLICY
   // ensure the resources are released
   if (m_resourceSet != NoResources) {
      setResourceSet(NoResources);
   }

   // do not delete the resource set until resources are released
   if (m_releasingResources) {
      m_resource->connect(m_resource, SIGNAL(resourcesReleased()),
                          SLOT(deleteLater()));
   } else {
      delete m_resource;
      m_resource = 0;
   }
#endif
}

CamerabinResourcePolicy::ResourceSet CamerabinResourcePolicy::resourceSet() const
{
   return m_resourceSet;
}

void CamerabinResourcePolicy::setResourceSet(CamerabinResourcePolicy::ResourceSet set)
{
   CamerabinResourcePolicy::ResourceSet oldSet = m_resourceSet;
   m_resourceSet = set;

#ifdef DEBUG_RESOURCE_POLICY
   qDebug() << Q_FUNC_INFO << set;
#endif

#ifdef HAVE_RESOURCE_POLICY
   QSet<ResourcePolicy::ResourceType> requestedTypes;

   switch (set) {
      case NoResources:
         break;

      case LoadedResources:
         requestedTypes << ResourcePolicy::LensCoverType //to detect lens cover is opened/closed
                        << ResourcePolicy::VideoRecorderType; //to open camera device
         break;

      case ImageCaptureResources:
         requestedTypes << ResourcePolicy::LensCoverType
                        << ResourcePolicy::VideoPlaybackType
                        << ResourcePolicy::VideoRecorderType
                        << ResourcePolicy::LedsType;
         break;

      case VideoCaptureResources:
         requestedTypes << ResourcePolicy::LensCoverType
                        << ResourcePolicy::VideoPlaybackType
                        << ResourcePolicy::VideoRecorderType
                        << ResourcePolicy::AudioPlaybackType
                        << ResourcePolicy::AudioRecorderType
                        << ResourcePolicy::LedsType;
         break;
   }

   QSet<ResourcePolicy::ResourceType> currentTypes;
   for (ResourcePolicy::Resource *resource : m_resource->resources()) {
      currentTypes << resource->type();
   }

   for (ResourcePolicy::ResourceType resourceType : currentTypes - requestedTypes) {
      m_resource->deleteResource(resourceType);
   }

   for (ResourcePolicy::ResourceType resourceType : requestedTypes - currentTypes) {
      if (resourceType == ResourcePolicy::LensCoverType) {
         ResourcePolicy::LensCoverResource *lensCoverResource = new ResourcePolicy::LensCoverResource;
         lensCoverResource->setOptional(true);
         m_resource->addResourceObject(lensCoverResource);

      } else if (resourceType == ResourcePolicy::AudioPlaybackType) {
         ResourcePolicy::Resource *resource = new ResourcePolicy::AudioResource;
         resource->setOptional(true);
         m_resource->addResourceObject(resource);

      } else if (resourceType == ResourcePolicy::AudioRecorderType) {
         ResourcePolicy::Resource *resource = new ResourcePolicy::AudioRecorderResource;
         resource->setOptional(true);
         m_resource->addResourceObject(resource);

      } else {
         m_resource->addResource(resourceType);
      }
   }

   m_resource->update();
   if (set != NoResources) {
      m_resource->acquire();
   } else {
      if (oldSet != NoResources) {
         m_releasingResources = true;
         m_resource->release();
      }
   }
#else
   (void) oldSet;
   updateCanCapture();
#endif
}

bool CamerabinResourcePolicy::isResourcesGranted() const
{
#ifdef HAVE_RESOURCE_POLICY
   for (ResourcePolicy::Resource *resource : m_resource->resources())
      if (! resource->isOptional() && !resource->isGranted()) {
         return false;
      }
#endif
   return true;
}

void CamerabinResourcePolicy::handleResourcesLost()
{
   updateCanCapture();
   emit resourcesLost();
}

void CamerabinResourcePolicy::handleResourcesGranted()
{
   updateCanCapture();
   emit resourcesGranted();
}

void CamerabinResourcePolicy::handleResourcesReleased()
{
#ifdef HAVE_RESOURCE_POLICY
#ifdef DEBUG_RESOURCE_POLICY
   qDebug() << Q_FUNC_INFO;
#endif
   m_releasingResources = false;
#endif
   updateCanCapture();
}

void CamerabinResourcePolicy::resourcesAvailable()
{
#ifdef HAVE_RESOURCE_POLICY
   if (m_resourceSet != NoResources) {
      m_resource->acquire();
   }
#endif
}

bool CamerabinResourcePolicy::canCapture() const
{
   return m_canCapture;
}

void CamerabinResourcePolicy::updateCanCapture()
{
   const bool wasAbleToRecord = m_canCapture;
   m_canCapture = (m_resourceSet == VideoCaptureResources) || (m_resourceSet == ImageCaptureResources);

#ifdef HAVE_RESOURCE_POLICY
   for (ResourcePolicy::Resource *resource : m_resource->resources()) {
      if (resource->type() != ResourcePolicy::LensCoverType) {
         m_canCapture = m_canCapture && resource->isGranted();
      }
   }
#endif
   if (wasAbleToRecord != m_canCapture) {
      emit canCaptureChanged();
   }
}

