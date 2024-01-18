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

#include "directshowvideorenderercontrol.h"

#include "videosurfacefilter.h"

#ifdef HAVE_EVR
#include "evrcustompresenter.h"
#endif

#include <qabstractvideosurface.h>

DirectShowVideoRendererControl::DirectShowVideoRendererControl(DirectShowEventLoop *loop, QObject *parent)
   : QVideoRendererControl(parent), m_loop(loop), m_surface(nullptr), m_filter(nullptr)
#ifdef HAVE_EVR
   , m_evrPresenter(nullptr)
#endif
{
}

DirectShowVideoRendererControl::~DirectShowVideoRendererControl()
{
#ifdef HAVE_EVR
   if (m_evrPresenter) {
      m_evrPresenter->setSurface(nullptr);
      m_evrPresenter->Release();
   }
#endif

   if (m_filter) {
      m_filter->Release();
   }
}

QAbstractVideoSurface *DirectShowVideoRendererControl::surface() const
{
   return m_surface;
}

void DirectShowVideoRendererControl::setSurface(QAbstractVideoSurface *surface)
{
   if (m_surface == surface) {
      return;
   }

#ifdef HAVE_EVR
   if (m_evrPresenter) {
      m_evrPresenter->setSurface(nullptr);
      m_evrPresenter->Release();
      m_evrPresenter = nullptr;
   }
#endif

   if (m_filter) {
      m_filter->Release();
      m_filter = nullptr;
   }

   m_surface = surface;

   if (m_surface) {

#ifdef HAVE_EVR
      m_filter = com_new<IBaseFilter>(clsid_EnhancedVideoRenderer);
      m_evrPresenter = new EVRCustomPresenter(m_surface);

      if (! m_evrPresenter->isValid() || !qt_evr_setCustomPresenter(m_filter, m_evrPresenter)) {
         m_filter->Release();
         m_filter = nullptr;

         m_evrPresenter->Release();
         m_evrPresenter = nullptr;
      }

      if (! m_filter)
#endif
      {
         m_filter = new VideoSurfaceFilter(m_surface, m_loop);
      }
   }

   emit filterChanged();
}

IBaseFilter *DirectShowVideoRendererControl::filter()
{
   return m_filter;
}
